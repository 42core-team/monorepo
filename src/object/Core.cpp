#include "Core.h"

Core::Core(unsigned int id, unsigned int teamId, Position pos)
	: Object(id, teamId, pos, Config::getInstance().coreHp)
{
	balance_ = Config::getInstance().initialBalance;
	team_id_ = teamId;
};
