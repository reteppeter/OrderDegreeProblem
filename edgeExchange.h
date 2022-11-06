#pragma once
#include "./core.h"
#include "./Graph.h"

#include "Random.h"
static Random randomGenerator{getRandSeed()};

namespace APSP{
	//Returns true if the edges a and b share a vertex
	bool duplicatedVertex(int a, int b, const Graph& edges){
		auto& A = edges.e[a];
		auto& B = edges.e[b];
		return (A.first == B.first || A.first == B.second || A.second == B.first || A.second == B.second);
	};

	//Given two edges will perform an exchange and return one of two chose valid swaps
	std::tuple<Edge, Edge> edgeExchange(Edge A, Edge B, bool swapType){
		auto x = B.second;
		B.second = A.second;
		if(swapType){
			A.second = x;
		} else{
			A.second = B.first;
			B.first = x;
		}

		A.sort();
		B.sort();

		return {A, B};
	};

	//Updates the neighbours of the current vertex v, removing the old neighbour oldVal and finding the new neighbour in the new edge
	void updateNeighbours(int v, int oldVal, const Edge& newEdge, Graph& graph){
		//Find the vertex that isn't v
		Vertex newVal{v != newEdge.first ? newEdge.first : newEdge.second};
		//Iterate throught the neighbours list to replace the old neighbour with the new.
		std::replace(graph.v[v].begin(), graph.v[v].end(), Vertex{oldVal}, newVal);
	};

	//Will augment the two edges by the chosen technique
	void edgeExchange2opt(int a, int b, Graph& graph, bool swapType){
		auto oA = graph.e[a];
		auto oB = graph.e[b];
		auto [nA, nB] = edgeExchange(oA, oB, swapType);
		//We know that the first value of A stayed in A
		updateNeighbours(oA.first, oA.second, nA, graph);
		updateNeighbours(oA.second, oA.first, nB, graph);
		//Determine where the first value of B was sent
		if(oB.first == nB.first || oB.first == nB.second){
			//It remaind in edge B
			updateNeighbours(oB.first, oB.second, nB, graph);
			updateNeighbours(oB.second, oB.first, nA, graph);
		} else{
			//It was moved to edge A
			updateNeighbours(oB.first, oB.second, nA, graph);
			updateNeighbours(oB.second, oB.first, nB, graph);
		}
		graph.e[a] = nA;
		graph.e[b] = nB;
	};

	//Assumes that the vertices making up each edge are sorted in ascending order
	bool isMultigraph(int a, int b, const Graph& graph, bool swapType){
		//A multigraph is when an edge is duplicated
		auto [A, B] = edgeExchange(graph.e[a], graph.e[b], swapType);

		for(auto& e : graph.e){
			if(e == A or e == B){ return true; }
		}

		return false;
	};

	/*
	* Augments the input graph into a new valid graph with two edges changed.
	* The edges swapped and the type of swap are returned
	* 
	* This is an implementation of the pseudo code of figure 4 in the first paper.
	*/
	auto edgeExchange(Graph& graph){
		struct Result{ int A; int B; bool swapType; };
		int A;
		int B;
		bool swapType = false;
		do{
			do{
				do{
					//Choose two random edges
					A = randomGenerator.next<int>() % graph.e.size();
					B = randomGenerator.next<int>() % graph.e.size();
				} while(A == B); //Choose again if the edges are the same
			} while(duplicatedVertex(A, B, graph)); //Choose again if the edges are incident on the same vertex
			swapType = randomGenerator.next<bool>(); //Choose a swapping method
		} while(isMultigraph(A, B, graph, swapType)); //Choose the two edges again if the swap would result in a multigraph
		edgeExchange2opt(A, B, graph, swapType); //Perform the exchange
		return Result{A, B, swapType};
	};
};