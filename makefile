main :
	mpicxx ./main.cpp -std=c++2a -Wall -Wextra -O3 -fopenmp -o ./main

clean :
	rm ./main