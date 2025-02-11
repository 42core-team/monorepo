#include "Utils.h"

template <typename T>
void shuffle_vector(std::vector<T> & vec)
{
	static std::random_device rd;
	static std::mt19937 g(rd());
	std::shuffle(vec.begin(), vec.end(), g);
}
