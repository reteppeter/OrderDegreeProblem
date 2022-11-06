#pragma once
#include <random>

//Get the next value from the random device
int getRandSeed(){
	static std::random_device randomDevice{};
	return (int)randomDevice();
};

struct Random{
	std::mt19937 gen;
	//Set up the distribution types we need
	std::uniform_int_distribution<> intDist{};
	std::uniform_int_distribution<> boolDist{0,1};
	std::uniform_real_distribution<> probDist{0,1};

	inline Random(int seed): gen(seed){};

	//Gets the next random value of the specified type 
	template<typename T> inline T next();

	//Generates the next double in the range [0,1].
	double inline nextProb(){ return probDist(gen); }
};

//Get the next random integer
template<> inline int Random::next<int>(){ return intDist(gen); };
//Get the next random bool
template<> inline bool Random::next<bool>(){ return boolDist(gen); };