#ifndef UTILS_H
#define UTILS_H

#include "Common.h"
class Game;

#include <vector>
#include <unordered_set>
#include <random>
#include <algorithm>

Position findFirstEmptyGridCell(Game* game, Position startPos);

template <typename T>
void shuffle_vector(std::vector<T> & vec)
{
	static std::random_device rd;
	static std::mt19937 g(rd());
	std::shuffle(vec.begin(), vec.end(), g);
}

#include "Game.h"

#endif // UTILS_H
