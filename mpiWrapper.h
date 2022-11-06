#pragma once
#include <mpi.h>
#include "./core.h"

namespace mpi{
	//Not exhaustive, just covering the ones we've used
	enum class Datatype: MPI_Datatype{
		Int = MPI_INT,
	};

	//The errors funtions like MPI_Send can return. These should be returned in the future, but the wrapper prevents most of these already.
	enum class Error: int{
		Success = MPI_SUCCESS,
		Comm = MPI_ERR_COMM,
		Type = MPI_ERR_TYPE,
		Count = MPI_ERR_COUNT,
		Tag = MPI_ERR_TAG,
		Rank = MPI_ERR_RANK
	};

	namespace Comm{
		//Enum of communication scopes
		enum class Comm: int{
			World = MPI_COMM_WORLD,
		};

		//Returns the number of processes.
		int size(const Comm comm = Comm::World){
			int size;
			MPI_Comm_size(underlying(comm), &size);
			return size;
		};

		//Returns the rank of this process
		int rank(const Comm comm = Comm::World){
			int rank;
			MPI_Comm_rank(underlying(comm), &rank);
			return rank;
		};

		//Returns the rank and size
		auto info(const Comm comm = Comm::World){
			struct Info{ const int rank = 0; const int size = 0; };
			return Info{rank(comm), size(comm)};
		}
	};

	//Set of compile time functions that map types to their MPI indicators
	template<typename T> constexpr Datatype typeToMPI();
	template<> constexpr Datatype typeToMPI<int>(){ return Datatype::Int; };

	//Intialise the MPI execution environment
	void init(int& argc, char**& argv){
		MPI_Init(&argc, &argv);
	};

	//Ends the MPI execution environment
	void finalise(){
		MPI_Finalize();
	};

	//Sets these values in the destination thread.
	template<typename T>
	void send(const T* source, int count, int destination, Comm::Comm comm = Comm::Comm::World){
		//Idealy this would take a reference
		MPI_Send(source, count, underlying(typeToMPI<T>()), destination, 0, underlying(comm));
	};

	//Sets this value in the destination thread.
	template<typename T>
	void send(const T* source, int destination, Comm::Comm comm = Comm::Comm::World){
		send(source, 1, destination, comm);
	};

	//Receives these values from sending process
	template<typename T>
	void receive(T* sink, int count, int sender, Comm::Comm comm = Comm::Comm::World){
		//This wrapper isn't supposed to be extensive, but it may make sense to return the status. 
		MPI_Recv(sink, count, underlying(typeToMPI<T>()), sender, 0, underlying(comm), MPI_STATUS_IGNORE);
	};

	//Receives this value from sending process
	template<typename T>
	void receive(T* sink, int sender, Comm::Comm comm = Comm::Comm::World){
		receive(sink, 1, sender, comm);
	};

	//Send these values to all other processes
	template<typename T>
	void broadcast(T* sink, int count, MPI_Datatype mpiType, int sender, Comm::Comm comm = Comm::Comm::World){
		MPI_Bcast(sink, count, mpiType, sender, underlying(comm));
	};

	//Send these values to all other processes
	template<typename T>
	void broadcast(T* sink, int count, int sender, Comm::Comm comm = Comm::Comm::World){
		broadcast(sink, count, underlying(typeToMPI<T>()), sender, comm);
	};

	//Send this value to all other processes
	template<typename T>
	void broadcast(T* sink, int sender, Comm::Comm comm = Comm::Comm::World){
		broadcast(sink, 1, sender, comm);
	};
};