//* This code is based on the paper:
//*
//* "A Method for Order/Degree Problem Based on Graph Symmetry and Simulated 
//* Annealing with MPI/OpenMP Parallelization"
//* Available at:
//* https://dl.acm.org/doi/pdf/10.1145/3293320.3293325

#include "./edgeExchange.h"
#include "./bfs.h"
#include "./Graph.h"
#include "./core.h"
#include "./mpiWrapper.h"
#include "./simulatedAnnealing.h"

#include <fstream>
#include <stdio.h>
#include <omp.h>

//The edge datatype for sending with MPI
MPI_Datatype mpiEdge;

//Initialises the value mpiEdge to a valid mpi datatype corresponding to APSP::Edge
void initMPIEdge(){
	//Define the Edge stuct for MPI
	MPI_Datatype type[2] = {MPI_INT, MPI_INT};
	int blocklen[2] = {1, 1};
	MPI_Aint disp[2];

	//Find offsets using an instance
	APSP::Edge testEdge{};
	MPI_Get_address(&testEdge.first, &disp[0]);
	MPI_Get_address(&testEdge.second, &disp[1]);

	//Make offsets in struct relative
	disp[1] = disp[1] - disp[0];
	disp[0] = 0;

	//Create type for edge
	MPI_Type_create_struct(2, blocklen, disp, type, &mpiEdge);
	MPI_Type_commit(&mpiEdge);
};



//To build: mpicxx ./main.cpp -std=c++2a -Wall -Wextra -O3 -fopenmp -o ./main
//To run: mpirun -np 2 ./main ./smallGraphBad.txt -t 2
int main(int argc, char** argv){
	mpi::init(argc, argv);
	auto [rank, size] = mpi::Comm::info();
	initMPIEdge();

	//Number of threads per process
	int threads = 1;
	string path = "";

	//Check for command line arguments
	if(argc == 2){
		path = string(argv[1]);
	} else if(argc == 4){
		//Check if thread count is specified. As we only support two inputs, we can check both orderings explicitly.
		if(argv[1][0] == '-'){
			threads = atoi(argv[2]);
			path = string(argv[3]);
		} else{
			path = string(argv[1]);
			threads = atoi(argv[3]);
		}
	} else{
		//Only print error in process 0
		if(rank == 0){
			printf("Invalid arguments. \"<filepath>\" must be present and optionaly and \"-t threadCount\"\n");
		}
		return -2;
	}

	//A valid path must be given
	if(path.size() <= 0){
		if(rank == 0){
			printf("Invalid path given\n");
		}
		return -1;
	}

	//Set the number of threads per process
	omp_set_num_threads(threads);

	//Total number of edges. Calculated in process 0 and then distributs so arrays can be resized.
	int edgeCount;
	//Edges that represent the graph
	Array<APSP::Edge> edges;

	//Only load the edges of the graph in process 0, then distribute this to the other processes
	if(rank == 0){
		std::ifstream infile(path);
		int a;
		int b;
		//File is assumed to be of format: "startVertex endVertex", with a new line between each edge
		while(infile >> a >> b){
			edges.push_back({a, b});
		}

		//Edge count must be sent first so the dynamically allocated array can be resized
		edgeCount = (int)edges.size();
		mpi::broadcast(&edgeCount, 0);
		mpi::broadcast(edges.data(), edgeCount, mpiEdge, 0);
	} else{
		mpi::broadcast(&edgeCount, 0);
		edges.resize(edgeCount);
		mpi::broadcast(edges.data(), edgeCount, mpiEdge, 0);
	}

	APSP::Graph graph{edges};
	//Keep a copy of the original to compare against at the end
	auto originalGraph = graph;

	//Range of vertices for this process
	int startVertex;
	int endVertex;

	//Calculate a range off vertics each program is reponsible for checking
	//Gives each process an equal number of vertices, and any remaining to process 0
	if(rank == 0){
		int width = graph.v.size() / size;
		int offset = graph.v.size() - width * (size - 1);
		mpi::broadcast(&offset, 0);
		mpi::broadcast(&width, 0);
		startVertex = 0;
		endVertex = offset;
	} else{
		int offset;
		int width;
		mpi::broadcast(&offset, 0);
		mpi::broadcast(&width, 0);
		startVertex = width * (rank - 1) + offset;
		endVertex = startVertex + width;
	}

	printf("Process %d will check from %d to %d.\n", rank, startVertex, endVertex - 1);

	//Run simulated anneling
	auto finalGraph = APSP::simulatedAnnealing(graph, rank, size, startVertex, endVertex);

	if(rank == 0){
		auto [origAspl, origDiam] = calculateASPL(originalGraph, 0, originalGraph.v.size());
		printf("The original ASPL was %f, and the diameter was %d.\n", origAspl, origDiam);
		auto [aspl, diam] = calculateASPL(finalGraph, 0, finalGraph.v.size());
		printf("Final minimum ASPL was %f, and the diameter of this graph was %d.\n", aspl, diam);

		//Save to file with derived filename
		auto outPath = path;
		if(outPath.size() > 4){
			//Only strip the file type if it had one
			if(outPath[outPath.size() - 4] == '.'){
				outPath.resize(outPath.size() - 4);
			}
		}
		outPath += ".res.txt";
		std::ofstream outfile(outPath);
		for(auto e: finalGraph.e){
			outfile << e.first << ' ' << e.second << '\n';
		}
	}
	mpi::finalise();
	return 0;
};