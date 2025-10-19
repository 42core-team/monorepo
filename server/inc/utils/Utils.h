#ifndef UTILS_H
#define UTILS_H

#include "Action.h"
#include "Board.h"
#include "Common.h"
#include "Config.h"

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

void shuffle_actions_vector(std::vector<std::pair<std::unique_ptr<Action>, Core *>> &actions);

#endif // UTILS_H
