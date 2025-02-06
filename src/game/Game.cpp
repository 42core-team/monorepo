#include "Game.h"

#include <chrono>
#include <thread>

Game::Game(const GameConfig& config) : config_(config), nextEntityId_(1)
{
	cores_.push_back(Core(nextEntityId_++, 1, {1000, 1000}, config_.coreHP));
	cores_.push_back(Core(nextEntityId_++, 2, {9000, 9000}, config_.coreHP));
}

void Game::addBridge(Bridge* bridge)
{
	bridges_.push_back(bridge);
}

void Game::run() {
	using clock = std::chrono::steady_clock;
	auto tickDuration = std::chrono::milliseconds(1000 / config_.tickRate);
	
	while (!stop_server)
	{
		auto start = clock::now();
		
		// Process incoming messages from all bridges.
		for (auto bridge : bridges_) {
			json msg;
			while (bridge->receiveMessage(msg)) {
				// Assume the message has an "actions" array.
				if (msg.contains("actions")) {
					for (auto& actJson : msg["actions"]) {
						Action action = Action::fromJson(actJson);
						if (action.type == ActionType::Create) {
							// Expect data: {"teamId": X}
							unsigned int teamId = action.data["teamId"];
							// Create a new unit at the core position for that team.
							Position pos;
							for (auto& core : cores_) {
								if (core.getTeamId() == teamId) {
									pos = core.getPosition();
									break;
								}
							}
							units_.push_back(Unit(nextEntityId_++, teamId, pos, config_.unitHP));
						} else if (action.type == ActionType::Attack) {
							// Expect data: {"attackerId": X, "targetId": Y}
							unsigned int targetId = action.data["targetId"];
							// For simplicity, deal fixed damage (e.g. 100) to the target.
							for (auto& unit : units_) {
								if (unit.getId() == targetId) {
									unit.dealDamage(100);
									break;
								}
							}
							for (auto& core : cores_) {
								if (core.getId() == targetId) {
									core.dealDamage(100);
									break;
								}
							}
						} else if (action.type == ActionType::Travel) {
							unsigned int unitId = action.data["unitId"];
							int dx = action.data["dx"];
							int dy = action.data["dy"];
							for (auto& unit : units_) {
								if (unit.getId() == unitId) {
									unit.travel(dx, dy);
									break;
								}
							}
						}
					}
				}
			}
		}
		
		update();
		sendState();
		
		auto end = clock::now();
		std::this_thread::sleep_for(tickDuration - std::chrono::duration_cast<std::chrono::milliseconds>(end - start));
	}
}

void Game::update()
{
	// In a complete game, update positions, resolve collisions, spawn resources, etc.
}

void Game::sendState()
{
	json state;
	state["cores"] = json::array();
	for (auto& core : cores_) {
		json c;
		c["id"] = core.getId();
		c["teamId"] = core.getTeamId();
		Position pos = core.getPosition();
		c["position"] = { {"x", pos.x}, {"y", pos.y} };
		c["hp"] = core.getHP();
		state["cores"].push_back(c);
	}
	state["units"] = json::array();
	for (auto& unit : units_) {
		json u;
		u["id"] = unit.getId();
		u["teamId"] = unit.getTeamId();
		Position pos = unit.getPosition();
		u["position"] = { {"x", pos.x}, {"y", pos.y} };
		u["hp"] = unit.getHP();
		state["units"].push_back(u);
	}
	
	json msg;
	msg["type"] = "State";
	msg["data"] = state;
	
	// Broadcast the state to every connected bridge.
	for (auto bridge : bridges_) {
		bridge->sendMessage(msg);
	}
}
