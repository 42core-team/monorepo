#ifndef COMMON_H
#define COMMON_H

#include <cmath>

enum class MovementDirection
{
	UP,
	DOWN,
	LEFT,
	RIGHT
};

struct Position
{
	int x;
	int y;

	Position(int x, int y) : x(x), y(y) {}
	Position() : x(-1), y(-1) {} // purposeful overflow, max val to indicate invalidity

	bool isValid(int maxX, int maxY) const
	{
		return x < maxX && y < maxY;
	}

	bool operator==(const Position& other) const
	{
		return x == other.x && y == other.y;
	}

	Position operator+(const MovementDirection& dir) const
	{
		Position newPos = *this;
		switch (dir)
		{
			case MovementDirection::UP:
				newPos.y--;
				break;
			case MovementDirection::DOWN:
				newPos.y++;
				break;
			case MovementDirection::LEFT:
				newPos.x--;
				break;
			case MovementDirection::RIGHT:
				newPos.x++;
				break;
		}
		return newPos;
	}
	Position operator+(const Position& other) const
	{
		return {x + other.x, y + other.y};
	}
	Position operator+(int scalar) const
	{
		return {x + scalar, y + scalar};
	}
	Position operator-(const Position& other) const
	{
		return {x - other.x, y - other.y};
	}
	Position operator*(int scalar) const
	{
		return {x * scalar, y * scalar};
	}

	double distance(const Position& other) const
	{
		return std::abs(x - other.x) + std::abs(y - other.y);
	}
};

#endif // COMMON_H