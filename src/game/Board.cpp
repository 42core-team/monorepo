#include "Board.h"

Board::Board(unsigned int grid_width, unsigned int grid_height)
	: grid_width_(grid_width), grid_height_(grid_height)
{
	objects_.reserve(grid_width * grid_height);
}

// @brief Adds object to board, if no object already at pos
// @param `force` set to true will overwrite object if already present at pos
// @return true if object was added sucessfully
bool Board::addObject(const Object & object, bool force)
{
	unsigned int vecPos = gridPosToVecPos(object.getPosition());
	if (vecPos < 0 || (objects_[vecPos] != nullptr && !force))
		return false;
	objects_[vecPos] = std::make_unique<Object>(object);
	return true;
}
// @return true if object was removed successfully
bool Board::removeObjectById(unsigned int id)
{
	for (unsigned int i = 0; i < objects_.size(); ++i)
	{
		if (objects_[i] != nullptr && objects_[i]->getId() == id)
		{
			objects_[i].reset();
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
	if (vecPos < 0)
		return false;
	objects_[vecPos].reset();
	return true;
}

// @return nullptr if no object with given id
Object *Board::getObjectById(unsigned int id) const
{
	for (const auto &obj : objects_)
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
		if (obj->getType() == ObjectType::Core && ((Core *)obj.get())->getTeamId() == team_id)
			return (Core *)obj.get();
	return nullptr;
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
		return -1;
	return gridPos.y * grid_width_ + gridPos.x;
}