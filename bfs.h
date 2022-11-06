#pragma once
#include "./core.h"
#include "./Graph.h"

namespace APSP{
	//Returns the neigbours of the given vertex v in the graph
	const Array<Vertex>& getNeighbours(Vertex v, const Graph& graph){
		return graph.v[v.value];
	};

	/*
	* For every vertex in the frontier, updates the distance to all the neighbours if it needs to.
	* Returns the vertices that had their distance updated.
	*/
	void topDownStep(
		const Graph& graph,
		const Array<Vertex>& frontier,
		Array<Vertex>& next,
		Array<int>& distance
	){
		for(auto v : frontier){
			for(auto n : getNeighbours(v, graph)){
				//Only write to the distance if it was unevealuated (-1)
				if(distance[n.value] == -1){
					distance[n.value] = distance[v.value] + 1;
					//Store the value for the next frontier
					next.push_back(n);
				}
			}
		}
	};

	//Atomic read and update of the value v to newVal iff v == oldVal
	bool compareAndSwap(int* v, int oldVal, int newVal){
		return __sync_bool_compare_and_swap(v, oldVal, newVal);
	};

	/*
	* For every vertex in the frontier, updates the distance to all the neighbours if it needs to.
	* This is parallelised using OpenMP.
	* Returns the vertices that had their distance updated.
	*/
	void topDownStepPar(
		const Graph& graph,
		const Array<Vertex>& frontier,
		Array<Vertex>& next,
		Array<int>& distance
	){
	#pragma omp parallel for schedule(dynamic, 2)
		for(uint j = 0; j < frontier.size(); ++j){
			auto v = frontier[j];
			//Create a thread local array of next vertices
			Array<Vertex> local_next{};
			auto neighbours = getNeighbours(v, graph);
			for(uint i = 0; i < neighbours.size(); ++i){
				auto n = neighbours[i];
				//Only write to the distance if it was unevealuated (-1). This is done atomically
				if(compareAndSwap(
					&(distance[n.value]),
					-1,
					distance[v.value] + 1
				)){
					//Store the value for the next frontier
					local_next.push_back(n);
				}
			}

		//This next section is critical, as only a single thread can safely updated next at a time.
		#pragma omp critical
			{
				for(auto v : local_next){
					next.push_back(v);
				}
			}
		}
	};

	//Performs a parallelised breadth first search on the graph starting from the source
	Array<int> breadthFirstSearch(const Graph& graph, Vertex source){
		//The array of values to check this iteration
		Array<Vertex> frontier{source};
		//The array of values to check next iteration
		Array<Vertex> next{};
		auto distance = Array<int>(graph.v.size(), -1); //-1 means unevaluated
		distance[source.value] = 0; //This is the intial vertex
		while(frontier.size() != 0){
			topDownStepPar(graph, frontier, next, distance);
			//Update next to the frontier, and reset next
			std::swap(frontier, next);
			next.resize(0);
		}
		return distance;
	};
};