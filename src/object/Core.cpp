#include "Core.h"

Core::Core(unsigned int id, unsigned int teamId, Position pos)
	: Object(id, pos, Config::getInstance().coreHp, ObjectType::Core), balance_(Config::getInstance().initialBalance), team_id_(teamId) {}
