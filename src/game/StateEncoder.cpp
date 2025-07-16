#include "Game.h"

json Game::encodeState(std::vector<std::pair<std::unique_ptr<Action>, Core *>> &actions, unsigned long long tick)
{
	json state;

	state["tick"] = tick;

	state["objects"] = json::array();
	for (const Object & obj : Board::instance())
	{
		json o;

		o["id"] = obj.getId();
		o["type"] = (int)obj.getType();
		o["x"] = Board::instance().getObjectPositionById(obj.getId()).x;
		o["y"] = Board::instance().getObjectPositionById(obj.getId()).y;
		o["hp"] = obj.getHP();

		if (obj.getType() == ObjectType::Core)
		{
			o["teamId"] = ((Core &)obj).getTeamId();
			o["balance"] = ((Core &)obj).getBalance();
		}
		if (obj.getType() == ObjectType::Unit)
		{
			o["teamId"] = ((Unit &)obj).getTeamId();
			o["unit_type"] = ((Unit &)obj).getUnitType();
			o["balance"] = ((Unit &)obj).getBalance();
			o["moveCooldown"] = ((Unit &)obj).getMoveCooldown();
		}
		if (obj.getType() == ObjectType::Resource || obj.getType() == ObjectType::Money)
		{
			o["balance"] = ((Resource &)obj).getBalance();
		}
		if (obj.getType() == ObjectType::Bomb)
		{
			o["countdown"] = ((Bomb &)obj).getCountdown();
		}

		state["objects"].push_back(o);
	}

	// append all actions that were executed without issues this turn
	state["actions"] = json::array();
	for (auto &action : actions)
	{
		if (action.first == nullptr)
			continue;

		state["actions"].push_back(action.first->encodeJSON());
	}

	return state;
}
