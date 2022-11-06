As a part of a parallel computing unit, we were required to implement a paper of our choice related to OpenMP or MPI.

The repository contains a breadth first search and 
simulated annealing based algorithm for solving the order/degree problem.
Both OpenMP and MPI are required.

## How to use
To build the program, run `make`, which requires `mpicxx`, and must have support for C++20.

To run on Windows, do:
```
mpiexec -n X ./main [graph file] -t Y
```
And on Linux:
```
mpirun -np X ./main [graph file] -t Y
```
Where X is the number of processes, Y is the number of threads, and [graph file] is the path to the file defining the graphs as an adjacency list. An example is given as `smallGraphBad.txt`.

Warning: X (the number of processes) must be less than or equal to the number of nodes in the graph.

## Example Output
### Console output
```
\> mpiexec -n 8 ./main .\largeGraphBad.txt -t 5
Process 6 will check from 192 to 223.
Process 5 will check from 160 to 191.
Process 3 will check from 96 to 127.
Process 7 will check from 224 to 255.
Process 4 will check from 128 to 159.
Process 0 will check from 0 to 31.
Process 1 will check from 32 to 63.
Process 2 will check from 64 to 95.
The original ASPL was 2.899908, and the diameter was 5.
Final minimum ASPL was 2.820113, and the diameter of this graph was 5.
```
### File output
For the input graph file, a new augmented graph file will be produced, with the same name bu the extension `.res.txt`.

## Originating Paper
This code implements, and is based on pseudo code from, the paper:
"A Method for Order/Degree Problem Based on Graph Symmetry and Simulated 
Annealing with MPI/OpenMP Parallelization"
Available at, as of 2022:
https://dl.acm.org/doi/pdf/10.1145/3293320.3293325
