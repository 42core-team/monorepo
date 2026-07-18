---
title: "⚙️ function core_action_travel(...)"
permalink: "core_action_travel"
sidebarTitle: "⚙️ action_travel()"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/c/client_lib/inc/core_lib.h#L147

## Description

Makes your unit move across the grid, choosing the most efficient path around obstacles to your goal.

One call makes the unit take one step. Call it again on later ticks to keep moving.

You decide what "efficient" means with a callback function. For every position, your callback returns:

- how costly that position is; and
- whether the unit may attack an object blocking that position.

The unit then does one of two things:

- If the next position is empty, it moves there.
- If an attackable object blocks the next position, it attacks it.

If the goal cannot be reached, the unit moves towards the reachable position closest to it. If the unit cannot get any closer, no action is added.

## Signature

```c
typedef struct s_travel_surface
{
	int weight;
	bool can_remove;
} t_travel_surface;

void core_action_travel(const t_obj *unit, t_pos pos,
						t_travel_surface (*get_surface)(t_pos, const t_obj *));
```

## `t_travel_surface`

Your callback returns one of these structs for every position on the grid.

- `weight`: The cost of entering the position. Travel prefers routes with a lower total cost. The starting position does not add to the cost.
- `can_remove`: Set this to `true` if an object on the position may be attacked. Set it to `false` for friendly units, your core, or anything else the route must not attack.

`can_remove` only matters when a position contains an object. An empty position is still walkable when `can_remove` is `false`.

## Parameters

- `unit`: One of your units. It must be ready to act (`action_cooldown <= 0`).
- `pos`: The goal. It may be anywhere inside the grid and does not need to be empty.
- `get_surface`: Your callback describing each grid position for this unit.

The callback receives both the position being checked and the unit that is travelling. It should only inspect the game and return a value. Do not add actions or change state inside it.

## Example

```c
static t_travel_surface get_travel_surface(t_pos pos, const t_obj *unit)
{
	t_obj *obstacle = core_get_obj_from_pos(pos);
	bool friendly = obstacle &&
		((obstacle->type == OBJ_UNIT && obstacle->s_unit.team_id == unit->s_unit.team_id) ||
		 (obstacle->type == OBJ_CORE && obstacle->s_core.team_id == unit->s_unit.team_id));

	return (t_travel_surface){
		.weight = obstacle ? 10 : 1,
		.can_remove = obstacle && !friendly,
	};
}

void move_towards(t_obj *unit, t_pos goal)
{
	core_action_travel(unit, goal, get_travel_surface);
}
```

This callback gives empty positions a cost of 1 and occupied positions a cost of 10. The unit will usually walk around an obstacle because open ground is cheaper. If going around is too expensive or impossible, it may attack a non-friendly obstacle and continue through that position after the obstacle is destroyed.

Friendly units and cores get `can_remove = false`, so they block the route and will not be attacked.

## When no action is added

Travel does nothing when:

- `unit` is `NULL` or is not a unit;
- the unit is still on cooldown;
- the goal is outside the grid;
- the unit is already at the goal;
- `get_surface` is `NULL`;
- no reachable position gets the unit closer to the goal; or
- the weights allow a loop whose total cost is negative.

## About weights

Use zero or positive weights unless you have a clear reason not to. Negative weights are allowed, but they can form a loop that gets cheaper every time it is repeated. There is then no cheapest route, so travel adds no action.

You do not have to use travel. If you want to write your own pathfinder or movement rules, call [`core_action_move`](reference/c/actions/core_action_move) and [`core_action_attack`](reference/c/actions/core_action_attack) directly.

## Related

- [⚙️ function core_action_move(...)](reference/c/actions/core_action_move)
- [⚙️ function core_action_attack(...)](reference/c/actions/core_action_attack)
- [🧩 struct s_obj](reference/c/objects/s_obj)
- [🧩 struct s_pos](reference/c/objects/s_pos)
