#include "Game.h"

Game::Game(std::vector<unsigned int> team_ids) : teamCount_(team_ids.size()), nextObjectId_(1)
{
	GameConfig config = Config::getInstance();
	objects_.reserve(team_ids.size());
	std::vector<unsigned int> team_ids_double = team_ids;
	shuffle_vector(team_ids_double); // randomly assign core positions to ensure fairness
	for (unsigned int i = 0; i < team_ids.size(); ++i)
		objects_.push_back(std::make_unique<Core>(nextObjectId_++, team_ids_double[i], Config::getCorePosition(i)));
	Logger::Log("Game created with " + std::to_string(team_ids.size()) + " teams.");
}
Game::~Game()
{
	for (auto bridge : bridges_)
		delete bridge;
}

void Game::addBridge(Bridge* bridge)
{
	bridges_.push_back(bridge);
}

void Game::run()
{
	sendConfig();

	using clock = std::chrono::steady_clock;
	unsigned int ticksPerSecond = Config::getInstance().tickRate;
	auto tickInterval = std::chrono::nanoseconds(1000000000 / ticksPerSecond);

	auto startTime = clock::now();
	unsigned long long tickCount = 0;

	int alivePlayers = teamCount_;

	while (alivePlayers > 1) // CORE GAMELOOP
	{
		auto now = clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(now - startTime);
		unsigned long long expectedTickCount = elapsed.count() / tickInterval.count();

		if (expectedTickCount > tickCount)
		{
			// Over one tick passed, we're lagging.
			if (expectedTickCount > tickCount + 1)
			{
				unsigned long skipped = expectedTickCount - tickCount - 1;
				Logger::Log(LogLevel::WARNING, "Processing delay: skipping " + std::to_string(skipped) + " tick(s).");
			}
			tickCount = expectedTickCount;
		}

		auto nextTickTime = startTime + (tickCount + 1) * tickInterval;
		std::this_thread::sleep_until(nextTickTime);

		tick(tickCount);
		sendState();

		alivePlayers = 0;
		for (const auto & objPtr : objects_)
		{
			const Object & obj = *objPtr;
			if (obj.getType() != ObjectType::Core)
				continue;
			if (obj.getHP() > 0)
				alivePlayers++;
		}

		tickCount++;
	}
}

void Game::tick(unsigned long long tick)
{
	// 1. COMPUTE ACTIONS

	std::vector<std::pair<Action *, Core &>> actions; // action, team id

	for (auto bridge : bridges_)
	{
		json msg;
		bridge->receiveMessage(msg);

		Core * core = getCore(bridge->getTeamId());

		if (!core)
			continue;
		if (core->getHP() <= 0)
			continue;

		std::vector<Action *> clientActions = Action::parseActions(msg);

		for (Action * action : clientActions)
			actions.emplace_back(action, *core);
	}

	shuffle_vector(actions); // shuffle action execution order to ensure fairness

	for (int i = 0; i < (int)actions.size(); i++)
	{
		Action * action = actions[i].first;
		Core & core = actions[i].second;

		if (action->getActionType() == ActionType::CREATE)
		{
			CreateAction * createAction = (CreateAction *)action;

			Position closestEmptyPos = findFirstEmptyGridCell(this, core.getPosition());
			if (!closestEmptyPos.isValid(Config::getInstance().width, Config::getInstance().height))
				continue;

			unsigned int unitType = createAction->getUnitTypeId();
			if (unitType >= Config::getInstance().units.size())
				continue;

			unsigned int unitCost = Config::getUnitConfig(unitType).cost;
			if (core.getBalance() < unitCost)
				continue;

			objects_.push_back(std::make_unique<Unit>(nextObjectId_++, core.getTeamId(), closestEmptyPos, unitType));
			core.setBalance(core.getBalance() - unitCost);
		}

		else if (action->getActionType() == ActionType::MOVE)
		{
			MoveAction * moveAction = (MoveAction *)action;
			Object * unitObj = getObject(moveAction->getUnitId());

			if (!unitObj || unitObj->getType() != ObjectType::Unit)
				continue;
			Unit * unit = (Unit *)unitObj;
			if (!unit->canTravel())
				continue;

			Position newPos = unit->getPosition() + moveAction->getDirection();
			if (!newPos.isValid(Config::getInstance().width, Config::getInstance().height))
				continue;

			Object * obj = getObjectAtPos(newPos);
			if (obj)
			{
				if (obj->getType() == ObjectType::Unit)
					obj->setHP(obj->getHP() - Config::getInstance().units[unit->getTypeId()].damageUnit);
				else if (obj->getType() == ObjectType::Core)
					obj->setHP(obj->getHP() - Config::getInstance().units[unit->getTypeId()].damageCore);
				else if (obj->getType() == ObjectType::Resource)
					((Resource *)obj)->getMined(unit);
				else if (obj->getType() == ObjectType::Wall)
					obj->setHP(obj->getHP() - Config::getInstance().units[unit->getTypeId()].damageWall);
			}
			else
			{
				unit->setPosition(newPos);
			}
		}

		delete action;
	}

	// 2. UPDATE OBJECTS

	for (auto it = objects_.begin(); it != objects_.end(); )
	{
		Object & obj = **it;
		if (obj.getHP() <= 0)
		{
			it = objects_.erase(it); 
			// TODO: handle game over (send message, disconnect bridge, decrease teamCount_)
		}
		else
		{
			obj.tick(tick);
			++it;
		}
	}

	visualizeGameState();
}
Object * Game::getObjectAtPos(Position pos)
{
	for (const auto & objPtr : objects_)
	{
		Object & obj = *objPtr;
		if (obj.getPosition() == pos)
			return &obj;
	}

	return nullptr;
}

void Game::sendState()
{
	json state;

	state["cores"] = json::array();
	for (auto& objPtr : objects_)
	{
		Object & obj = *objPtr;
		if (obj.getType() != ObjectType::Core)
			continue;
		Core & core = (Core &)obj;

		json c;

		c["id"] = core.getId();
		c["teamId"] = core.getTeamId();
		c["x"] = core.getPosition().x;
		c["y"] = core.getPosition().y;
		c["hp"] = core.getHP();
		c["balance"] = core.getBalance();

		state["cores"].push_back(c);
	}

	state["units"] = json::array();
	for (auto& objPtr : objects_)
	{
		Object & obj = *objPtr;
		if (obj.getType() != ObjectType::Unit)
			continue;
		Unit & unit = (Unit &)obj;

		json u;

		u["id"] = unit.getId();
		u["teamId"] = unit.getTeamId();
		u["x"] = unit.getPosition().x;
		u["y"] = unit.getPosition().y;
		u["hp"] = unit.getHP();
		u["type"] = unit.getTypeId();
		u["balance"] = unit.getBalance();

		state["units"].push_back(u);
	}

	state["resources"] = json::array();
	for (auto& objPtr : objects_)
	{
		Object & obj = *objPtr;
		if (obj.getType() != ObjectType::Resource)
			continue;
		Resource & resource = (Resource &)obj;

		json r;

		r["id"] = resource.getId();
		r["x"] = resource.getPosition().x;
		r["y"] = resource.getPosition().y;
		r["hp"] = resource.getHP();

		state["resources"].push_back(r);
	}

	state["walls"] = json::array();
	for (auto& objPtr : objects_)
	{
		Object & obj = *objPtr;
		if (obj.getType() != ObjectType::Wall)
			continue;
		Wall & wall = (Wall &)obj;

		json w;

		w["id"] = wall.getId();
		w["x"] = wall.getPosition().x;
		w["y"] = wall.getPosition().y;
		w["hp"] = wall.getHP();

		state["walls"].push_back(w);
	}
	
	for (auto bridge : bridges_)
	{
		bridge->sendMessage(state);
	}
}
void Game::sendConfig()
{
	GameConfig config = Config::getInstance();

	json configJson;

	configJson["width"] = config.width;
	configJson["height"] = config.height;
	configJson["tickRate"] = config.tickRate;

	configJson["idleIncome"] = config.idleIncome;
	configJson["idleIncomeTimeOut"] = config.idleIncomeTimeOut;

	configJson["resourceHp"] = config.resourceHp;
	configJson["resourceIncome"] = config.resourceIncome;

	configJson["coreHp"] = config.coreHp;
	configJson["initialBalance"] = config.initialBalance;

	configJson["wallHp"] = config.wallHp;

	configJson["units"] = json::array();
	for (auto& unit : config.units)
	{
		json u;

		u["name"] = unit.name;
		u["cost"] = unit.cost;
		u["hp"] = unit.hp;
		u["speed"] = unit.speed;

		u["damageCore"] = unit.damageCore;
		u["damageUnit"] = unit.damageUnit;
		u["damageResource"] = unit.damageResource;
		u["damageWall"] = unit.damageWall;

		configJson["units"].push_back(u);
	}

	for (auto bridge : bridges_)
	{
		bridge->sendMessage(configJson);
	}
}

Core * Game::getCore(unsigned int teamId)
{
	for (auto& objPtr : objects_)
	{
		Object & obj = *objPtr;
		if (obj.getType() != ObjectType::Core)
			continue;
		Core & core = (Core &)obj;

		if (core.getTeamId() == teamId)
			return &core;
	}

	return nullptr;
}

Object * Game::getObject(unsigned int id)
{
	for (auto& objPtr : objects_)
	{
		Object & obj = *objPtr;

		if (obj.getId() == id)
			return &obj;
	}

	return nullptr;
}
