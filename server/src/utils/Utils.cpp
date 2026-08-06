#include "Action.h"

#include <array>
#include <cctype>
#include <stdexcept>
#include <string>

void shuffle_actions_vector(std::vector<std::pair<std::unique_ptr<Action>, Core *>> &actions, uint64_t seed)
{
	std::mt19937_64 generator(seed);

	auto phase_of = [](ActionType t) -> int
	{
		switch (t)
		{
		case ActionType::TRANSFER_GEMS:
			return 0;
		case ActionType::ATTACK:
			return 1;
		case ActionType::MOVE:
			return 2;
		case ActionType::CREATE:
			return 3;
		default:
			throw std::runtime_error("Unknown ActionType");
		}
	};

	std::array<std::vector<std::pair<std::unique_ptr<Action>, Core *>>, 5> buckets;
	for (auto &ele : actions)
	{
		const int p = ele.first ? phase_of(ele.first->getActionType()) : 3;
		buckets[p].push_back(std::move(ele));
	}
	for (auto &bucket : buckets)
	{
		// Packet arrival order is nondeterministic; establish a stable order before applying seeded randomness.
		std::stable_sort(bucket.begin(), bucket.end(),
						 [](const auto &lhs, const auto &rhs)
						 {
							 if (!lhs.second) return false;
							 if (!rhs.second) return true;
							 return lhs.second->getTeamId() < rhs.second->getTeamId();
						 });
		std::shuffle(bucket.begin(), bucket.end(), generator);
	}
	actions.clear();
	for (auto &bucket : buckets)
	{
		for (auto &ele : bucket)
			actions.emplace_back(std::move(ele));
	}
}
