#ifndef UTILS_H
#define UTILS_H

#include "Common.h"
#include "Config.h"
#include "Board.h"

#include <vector>
#include <unordered_set>
#include <random>
#include <algorithm>
#include <queue>

Position findFirstEmptyGridCell(Position startPos);

template <typename T>
void shuffle_vector(std::vector<T> & vec)
{
	static std::random_device rd;
	static std::mt19937 g(rd());
	std::shuffle(vec.begin(), vec.end(), g);
}

#endif // UTILS_H
