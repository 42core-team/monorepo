#ifndef BOARD_H
#define BOARD_H

#include <vector>
#include <iterator>

#include "Object.h"
#include "Core.h"

// TODO: instead of clone, use builtin copy constructor functionality
// TODO: remove auto usage until very necessary, its just unclear
// TODO: can you somehow configure the board iterator to skip stuff conditionally, to only loop over objects with certain type for example?
// TODO: rework getnextobjectid to not have to be called manually from outside the function all the time

class Board
{
	public:
		Board(unsigned int grid_width, unsigned int grid_height);
		~Board() = default;

		template <typename T>
		bool			addObject(const T &object, bool force = false)
		{
			static_assert(std::is_base_of<Object, T>::value, "T must be a subclass of Object");
			unsigned int vecPos = gridPosToVecPos(object.getPosition());
			if (vecPos < 0 || (objects_[vecPos] != nullptr && !force))
				return false;
			objects_[vecPos] = std::make_unique<T>(object);
			return true;
		}
		bool			removeObjectById(unsigned int id);
		bool			removeObjectAtPos(const Position & pos);

		Object			*getObjectById(unsigned int id) const;
		Object			*getObjectAtPos(const Position & pos) const;
		Core			*getCoreByTeamId(unsigned int team_id) const;

		Position		vecPosToGridPos(unsigned int vecPos) const;
		unsigned int	gridPosToVecPos(const Position & gridPos) const;

		unsigned int	getNextObjectId() { return next_object_id_++; }

	private:
		std::vector<std::unique_ptr<Object>> objects_; // [ 5 | 3] is at index grid_width * 5 + 3

		unsigned int grid_width_;
		unsigned int grid_height_;

		unsigned int next_object_id_ = 0;


	// @brief Board Iterator that only exposes valid objects
	class Iterator
	{
		using VecIt = std::vector<std::unique_ptr<Object>>::iterator;
		VecIt current_, end_;

		void advance_to_valid()
		{
			while (current_ != end_ && !*current_)
				++current_;
		}

	public:
		Iterator(VecIt begin, VecIt end)
			: current_{begin}, end_{end} { advance_to_valid(); }

		Object &operator*() const { return *current_->get(); }
		Object *operator->() const { return  current_->get(); }

		Iterator& operator++() {
			++current_;
			advance_to_valid();
			return *this;
		}

		bool operator!=(const Iterator &other) const {
			return current_ != other.current_;
		}
	};

	public:
		Iterator begin()	{ return {objects_.begin(), objects_.end()}; }
		Iterator end()		{ return {objects_.end(), objects_.end()}; }
};

#endif // BOARD_H
