solver :
	mpicxx ./main.cpp -std=c++2a -Wall -Wextra -O3 -fopenmp -o ./solver

clean :
	rm ./solver