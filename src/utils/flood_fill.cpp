#include "Utils.h"

Position findFirstEmptyGridCell(Game* game, Position startPos) // bfs / flood fill
{
	Position errorPos = {static_cast<int>(Config::getInstance().width), static_cast<int>(Config::getInstance().height)};
	const int width  = Config::getInstance().width;
	const int height = Config::getInstance().height;
	
	if ((int)startPos.x < 0 || (int)startPos.x >= width || (int)startPos.y < 0 || (int)startPos.y >= height)
		return errorPos;
	
	std::queue<Position> q;
	std::unordered_set<int> visited;

	const int dx[] = {1, -1, 0, 0};
	const int dy[] = {0, 0, 1, -1};
	
	q.push(startPos);
	visited.insert(startPos.x * height + startPos.y); // hash function: x * height + y
	
	while (!q.empty())
	{
		Position cur = q.front();
		q.pop();

		if ((int)cur.x < 0 || (int)cur.x >= width || (int)cur.y < 0 || (int)cur.y >= height)
			continue;

		if (game->getObjectAtPos(cur) == nullptr)
			return cur;

		for (int i = 0; i < 4; i++)
		{
			Position next = {cur.x + dx[i], cur.y + dy[i]};
			int nextId = next.x * height + next.y;

			if ((int)next.x >= 0 && (int)next.x < width && (int)next.y >= 0 && (int)next.y < height &&
				visited.find(nextId) == visited.end())
			{
				q.push(next);
				visited.insert(nextId);
			}
		}
	}

	return errorPos;
}
