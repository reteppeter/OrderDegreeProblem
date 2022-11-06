#pragma once
#include "./core.h"
#include <algorithm>
#include "stdio.h"

namespace APSP{
	//Represents an edge in a graph. Assumed to always have the vertices sorted in ascending order.
	struct Edge{
		int first;
		int second;

		//Sets the two vetrices in ascending order
		void sort(){
			if(first > second){
				auto temp = first;
				first = second;
				second = temp;
			}
		};

		//Returns true if both edges have the same start and end points
		bool operator==(const Edge& other) const{
			return first == other.first and second == other.second;
		};
	};

	//Represents a vertex in the graph. Just to prevent math being performed when not intended.
	struct Vertex{
		int value;

		//Returns true if both vertices have the same value
		bool operator==(const Vertex& other) const{
			return value == other.value;
		};
	};

	//The graph, stores both edges and vertex neighbours
	struct Graph{
		Array<Edge> e;
		Array<Array<Vertex>> v;

		//Construct a graph with both the edges and vertex neighbours. These aren't verified.
		Graph(Array<Edge> e, Array<Array<Vertex>> v) : e(e), v(v){};

		//Construct a graph from only a set of edges. The vertex neighbours will be generated.
		Graph(Array<Edge> e) : e(e), v(){
			for(auto edge : e){
				//Vertices are only defined by their index, so we resize to the largest one seen
				while(edge.first >= (int)v.size()){
					v.push_back({});
				}
				v[edge.first].push_back({edge.second});

				while(edge.second >= (int)v.size()){
					v.push_back({});
				}
				v[edge.second].push_back({edge.first});
			}
		};

		//Prints all edges then vertices and connections
		void print() const{
			printf("Edges:\n");
			for(auto& edge : e){
				printf("(%d, %d)\n", edge.first, edge.second);
			}

			printf("\nVertices:\n");
			for(uint i = 0; i != v.size(); ++i){
				printf("%d: ", i);
				for(auto& neighbour : v[i]){
					printf("%d, ", neighbour.value);
				}
				printf("\n");
			}
		};
	};
}