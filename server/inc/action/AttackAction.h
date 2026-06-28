#ifndef ATTACK_ACTION_H
#define ATTACK_ACTION_H

#include "Action.h"
#include "Board.h"
#include "Common.h"
#include "Deposit.h"
#include "GemPile.h"
#include "Stats.h"

#include <vector>

class Unit;

#include "json.hpp"
using json = nlohmann::ordered_json;

class AttackAction : public Action
{
  public:
	AttackAction(json msg);
	unsigned int getUnitId() const { return unit_id_; }
	unsigned int getTargetId() const { return target_id_; }

	std::string execute(Core *core);
	void decodeJSON(json msg);
	json encodeJSON();

  private:
	unsigned int unit_id_;
	unsigned int target_id_;
};

#endif // ATTACK_ACTION_H
