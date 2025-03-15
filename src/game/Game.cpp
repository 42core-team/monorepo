#include "Game.h"

Game::Game(std::vector<unsigned int> team_ids) : teamCount_(team_ids.size()), nextObjectId_(1)
{
	GameConfig config = Config::getInstance();
	objects_.reserve(team_ids.size());
	std::vector<unsigned int> team_ids_double = team_ids;
	shuffle_vector(team_ids_double); // randomly assign core positions to ensure fairness
	for (unsigned int i = 0; i < team_ids.size(); ++i)
		objects_.push_back(std::make_unique<Core>(getNextObjectId(), team_ids_double[i], Config::getCorePosition(i)));
	JigsawWorldGenerator generator;
	generator.generateWorld(this);
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

	std::cout << "Game over!" << std::endl;
	json config = getConfig();
	replayEncoder_.includeConfig(config);
	json replay = replayEncoder_.getReplay();
	std::ofstream replayFile("replay_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + ".json");
	replayFile << replay.dump();
	replayFile.close();
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

		if (!action->execute(this, &core))
		{
			delete action;
			actions[i].first = nullptr;
		}
	}

	// 2. UPDATE OBJECTS

	for (auto it = objects_.begin(); it != objects_.end(); )
	{
		Object * obj = it->get();
		if (obj->getHP() <= 0)
		{
			if (obj->getType() == ObjectType::Unit)
				objects_.push_back(std::make_unique<Resource>(getNextObjectId(), obj->getPosition(), ((Unit *)obj)->getBalance())); // drop balance on death
			it = objects_.erase(it);
			teamCount_--;
			// TODO: handle game over (send message, disconnect bridge, decrease teamCount_)
		}
		else
		{
			obj->tick(tick);
			++it;
		}
	}

	// 3. SEND STATE

	sendState(actions, tick);
	visualizeGameState(tick);

	for (auto& action : actions)
	{
		if (action.first != nullptr)
			delete action.first;
	}
}

void Game::sendState(std::vector<std::pair<Action *, Core &>> actions, unsigned long long tick)
{
	json state;

	state["tick"] = tick;

	state["objects"] = json::array();
	for (auto& objPtr : objects_)
	{
		Object & obj = *objPtr;

		json o;

		o["id"] = obj.getId();
		o["type"] = (int)obj.getType();
		o["x"] = obj.getPosition().x;
		o["y"] = obj.getPosition().y;
		o["hp"] = obj.getHP();

		if (obj.getType() == ObjectType::Core)
		{
			o["teamId"] = ((Core &)obj).getTeamId();
			o["balance"] = ((Core &)obj).getBalance();
		}
		if (obj.getType() == ObjectType::Unit)
		{
			o["teamId"] = ((Unit &)obj).getTeamId();
			o["typeId"] = ((Unit &)obj).getTypeId();
			o["balance"] = ((Unit &)obj).getBalance();
			o["nextMoveOpp"] = ((Unit &)obj).getNextMoveOpp();
		}
		if (obj.getType() == ObjectType::Resource)
		{
			o["balance"] = ((Resource &)obj).getBalance();
		}

		state["objects"].push_back(o);
	}

	// append all actions that were executed without issues this turn
	state["actions"] = json::array();
	for (auto& action : actions)
	{
		if (action.first == nullptr)
			continue;

		state["actions"].push_back(action.first->encodeJSON());
	}

	replayEncoder_.addTickState(state);
	for (auto bridge : bridges_)
		bridge->sendMessage(state);
}
json Game::getConfig() const
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
	configJson["wallBuildCost"] = config.wallBuildCost;

	configJson["units"] = json::array();
	for (auto& unit : config.units)
	{
		json u;

		u["name"] = unit.name;
		u["cost"] = unit.cost;
		u["hp"] = unit.hp;
		u["speed"] = unit.speed;
		u["minSpeed"] = unit.minSpeed;

		u["damageCore"] = unit.damageCore;
		u["damageUnit"] = unit.damageUnit;
		u["damageResource"] = unit.damageResource;
		u["damageWall"] = unit.damageWall;
		u["attackType"] = (int)unit.attackType;
		u["attackReach"] = unit.attackReach;

		u["canBuild"] = unit.canBuild;

		configJson["units"].push_back(u);
	}

	return configJson;
}
void Game::sendConfig()
{
	json config = getConfig();

	for (auto bridge : bridges_)
	{
		bridge->sendMessage(config);
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
std::vector<Core> Game::getCores()
{
	std::vector<Core> cores;
	cores.reserve(teamCount_);
	for (auto& objPtr : objects_)
	{
		Object & obj = *objPtr;
		if (obj.getType() != ObjectType::Core)
			continue;
		Core & core = (Core &)obj;

		cores.push_back(core);
	}

	return cores;
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
