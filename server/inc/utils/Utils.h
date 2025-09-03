#ifndef UTILS_H
#define UTILS_H

#include "Board.h"
#include "Common.h"
#include "Config.h"

#include <algorithm>
#include <queue>
#include <random>
#include <unordered_set>
#include <vector>

Position findFirstEmptyGridCell(Position startPos);

std::string random_base32_seed();

template <typename T> void shuffle_vector(std::vector<T> &vec)
{
	static std::random_device rd;
	static std::mt19937 g(rd());
	std::shuffle(vec.begin(), vec.end(), g);
}

#endif // UTILS_H
