#include "Board.h"

Board::Board(unsigned int grid_width, unsigned int grid_height)
	: grid_width_(grid_width), grid_height_(grid_height)
{
	objects_.reserve(grid_width * grid_height);
	for (unsigned int i = 0; i < grid_width * grid_height; ++i)
		objects_.emplace_back(nullptr);
}

// @return true if object was removed successfully
bool Board::removeObjectById(unsigned int id)
{
	for (unsigned int i = 0; i < objects_.size(); ++i)
	{
		if (objects_[i] != nullptr && objects_[i]->getId() == id)
		{
			objects_[i] = nullptr;
			return true;
		}
	}
	return false;
}
// @return true if object was removed successfully
bool Board::removeObjectAtPos(const Position & pos)
{
	if (!pos.isValid(grid_width_, grid_height_))
		return false;
	unsigned int vecPos = gridPosToVecPos(pos);
	if (vecPos > grid_height_ * grid_width_ || objects_[vecPos] == nullptr)
		return false;
	objects_[vecPos] = nullptr;
	return true;
}

// @return nullptr if no object with given id
Object *Board::getObjectById(unsigned int id) const
{
	for (const auto &obj : objects_)
		if (obj != nullptr)
			if (obj->getId() == id)
				return obj.get();
	return nullptr;
}
// @return nullptr if no object at given position
Object *Board::getObjectAtPos(const Position & pos) const
{
	if (!pos.isValid(grid_width_, grid_height_))
		return nullptr;
	unsigned int vecPos = gridPosToVecPos(pos);
	if (vecPos >= objects_.size())
		return nullptr;
	return objects_[vecPos].get();
}
// @return nullptr if no core with given team id
Core *Board::getCoreByTeamId(unsigned int team_id) const
{
	for (const auto &obj : objects_)
		if (obj != nullptr)
			if (obj->getType() == ObjectType::Core && ((Core *)obj.get())->getTeamId() == team_id)
				return (Core *)obj.get();
	return nullptr;
}
Position Board::getObjectPositionById(unsigned int id) const
{
	for (unsigned int idx = 0; idx < objects_.size(); ++idx)
	{
		if (!objects_[idx]) 
			continue;
		if (objects_[idx]->getId() == id)
			return vecPosToGridPos(idx);
	}
	return Position(-1, -1);
}
int Board::getCoreCount()
{
	int count = 0;
	for (const auto &obj : objects_)
	{
		if (obj && obj->getType() == ObjectType::Core && obj->getHP() > 0)
			++count;
	}
	return count;
}

bool Board::moveObjectById(unsigned int id, const Position & newPos)
{
	if (!newPos.isValid(grid_width_, grid_height_))
		return false;
	unsigned int destIdx = gridPosToVecPos(newPos);
	if (destIdx >= objects_.size() || objects_[destIdx])
		return false;
	unsigned int srcIdx = std::numeric_limits<unsigned int>::max();
	for (unsigned int idx = 0; idx < objects_.size(); ++idx)
	{
		if (objects_[idx] && objects_[idx]->getId() == id)
		{
			srcIdx = idx;
			break;
		}
	}
	if (srcIdx == std::numeric_limits<unsigned int>::max())
		return false;

	objects_[destIdx] = std::move(objects_[srcIdx]);
	objects_[srcIdx] = nullptr;
	return true;
}

Position Board::vecPosToGridPos(unsigned int vecPos) const
{
	unsigned int x = vecPos % grid_width_;
	unsigned int y = vecPos / grid_width_;
	Position pos = Position(x, y);
	if (!pos.isValid(grid_width_, grid_height_))
		return Position(-1, -1);
	return pos;
}
unsigned int Board::gridPosToVecPos(const Position & gridPos) const
{
	if (!gridPos.isValid(grid_width_, grid_height_))
		return -1; // purposeful overflow, max val to indicate invalidity
	return gridPos.y * grid_width_ + gridPos.x;
}