#pragma once
#include "./edgeExchange.h"
#include "./mpiWrapper.h"
#include "./bfs.h"
#include "./Graph.h"
#include "./core.h"

#include <numeric>

namespace APSP{
	//Checks if all distances are valid (none are -1)
	bool isValid(const Array<int>& dist){
		for(auto d : dist){ if(d == -1){ return false; } }
		return true;
	}

	//Finds the average shortest path length between all pairs of vertices, as well as the diameter.
	auto calculateASPL(const APSP::Graph& graph, int startVertex, int endVertex){
		//Struct so we can return multiple values
		struct Result{ double aspl; int diameter; };

		//Perform a test on the first vertex
		auto dists = APSP::breadthFirstSearch(graph, {startVertex});
		//If the graph is disconnected then not all pairs have paths
		if(!isValid(dists)){ return Result{INFINITY, 0}; }

		//The average for a vertex is the sum of its distance to all other vertices, so we subtract 1
		auto div = double(graph.v.size() - 1);

		//Intial values for what we are trying to find
		int diameter = *std::max_element(dists.begin(), dists.end());
		double total = std::accumulate(dists.begin(), dists.end(), 0) / div;

		//Add the average for a given vertex to all others to the total
		for(uint i = startVertex + 1; i != (uint)endVertex; ++i){
			dists = APSP::breadthFirstSearch(graph, {(int)i});
			//Add average distance to total
			total += std::accumulate(dists.begin(), dists.end(), 0) / div;
			//Only update the diameter if it is greater than our current
			int newDiameter = *std::max_element(dists.begin(), dists.end());
			diameter = diameter < newDiameter ? newDiameter : diameter;
		}
		return Result{total / graph.v.size(), diameter};
	};

	//Calculates the energy, which is just the ASPL. Distributed, must be called from each process
	double calculateEnergy(const Graph& graph, int startVertex, int endVertex){
		double newEnergy = calculateASPL(graph, startVertex, endVertex).aspl;
		double totalEnergy;
		//Sum results and distrbute them back to all other processes
		MPI_Allreduce(&newEnergy, &totalEnergy, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
		return totalEnergy;
	};

	//Calculate the Metropolis criterion
	double metropolis(double deltaE, double T){
		if(deltaE < 0){
			//If the energy is lower always take it
			return 1.0;
		} else{
			//Else take it based on the temperature
			return exp(-deltaE / T);
		}
	};

	//Finds a new layout of connections using SA and BFS
	Graph simulatedAnnealing(Graph graph, int rank, [[maybe_unused]] int size, int startVertex, int endVertex){
		/*
		* An implementation of the SA steps from page 3 of "A Method for
		* Order/Degree Problem Based on Graph Symmetry and Simulated Annealing
		* with MPI/OpenMP Parallelization".
		*/

		//(1) Set initialize parameters
		double energy = calculateEnergy(graph, startVertex, endVertex); //Calculate the intial energy.
		int energyMultiplier = graph.v.size() * (graph.v.size() - 1);
		double T = 100; //Start temperature
		double C = 0.22; //End temperature
		int I = 1; //Repetitions for cooling process
		int N = 1000; //Total number of calculations
		double alpha = pow(C / T, double(I) / N); //Scalling factor for T
		int iters = 0; //Current number of iterations

		for(;;){
			//(2) Generate next solution
			auto newGraph = graph;
			int edgeA;
			int edgeB;
			int swapTypeInt;
			if(rank == 0){
				//The exchange with verification only needs to be done in the root process
				auto [A, B, sT] = edgeExchange(newGraph);
				edgeA = A;
				edgeB = B;
				swapTypeInt = sT;
			}

			//Send the swapped edges and swap type to all other processes.
			//Alternatively, this could have been done using an int array of size 3.
			mpi::broadcast(&edgeA, 0);
			mpi::broadcast(&edgeB, 0);
			mpi::broadcast(&swapTypeInt, 0);

			//If the rank isn't zero we have to update the edges received above
			if(rank != 0){
				edgeExchange2opt(edgeA, edgeB, newGraph, bool(swapTypeInt));
			}

			//(3) Compute energy
			//Calculate and reduce the energy from each process
			double newEnergy = calculateEnergy(newGraph, startVertex, endVertex);

			double deltaE = energyMultiplier * (newEnergy - energy);

			//(4) Acceptance
			int accepted = 0;
			//Only calculate the random value in process 0
			if(rank == 0){
				accepted = metropolis(deltaE, T) >= randomGenerator.nextProb();
			}
			mpi::broadcast(&accepted, 0);
			
			if(accepted){
				//(5) Transition
				std::swap(graph, newGraph);
				energy = newEnergy;
			}

			//(6) Cooling cycle
			if(iters % I == 0){
				//(7) Cooling
				T *= alpha;
			}

			++iters;

			//(8) Terminal
			if(T <= C || iters == N){
				break;
			}
		}

		return graph;
	};
};