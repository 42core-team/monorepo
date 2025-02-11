#ifndef UTILS_H
#define UTILS_H

#include "Common.h"
class Game;

#include <vector>
#include <unordered_set>
#include <random>

Position findFirstEmptyGridCell(Game* game, Position startPos);
template <typename T>
void shuffle_vector(std::vector<T> & vec);

#include "Game.h"

#endif // UTILS_H
