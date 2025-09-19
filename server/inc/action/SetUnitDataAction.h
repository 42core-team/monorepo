#ifndef SET_UNIT_DATA_ACTION_H
#define SET_UNIT_DATA_ACTION_H

#include <list>

#include "Action.h"
#include "Board.h"
#include "Bomb.h"
#include "Common.h"
#include "Deposit.h"
#include "GemPile.h"
#include "Stats.h"

#include <vector>

class Unit;

#include "json.hpp"
using json = nlohmann::ordered_json;

class SetUnitDataAction : public Action
{
  public:
	SetUnitDataAction(json msg);
	std::list<Position> getPath() const { return path_; }

	std::string execute(Core *core);
	void decodeJSON(json msg);
	json encodeJSON();

  private:
	std::list<Position> path_;
	unsigned int unit_id_;
};

#endif // SET_UNIT_DATA_ACTION_H
