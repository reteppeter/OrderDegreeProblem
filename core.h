#pragma once

//Remove the need for std:: on string
#include <string>
using std::string;

//Aliasing std::vector as it isn't a mathematical vector
#include <vector>
template<typename T, typename A = std::allocator<T>>
using Array = std::vector<T, A>;

//Alias unsigned int to a more convenient name
using uint = unsigned int;

// Converts an enum to its underlying type
template <typename E>
constexpr auto underlying(E e){
	/* This function is very useful for converting enum classes to their
	* underlying type, which you can specify (in both C and C++). Only 
	* integral types are able to be used.
	* You can restrict this function using C++20 concepts as I do in my own
	* projects, but the form shown here is being added to the standard as
	* to_underlying in C++23.
	*/
	return static_cast<typename std::underlying_type<E>::type>(e);
};