In this guide, you will build a small bot that creates a custom unit and sends it towards the opponent. You will learn how unit design, game ticks, object searches, and travel fit together.

---

## Before you start

If you have not created a team and cloned your repository yet, follow the [Getting Started Guide](README) first. Run `make` once and watch the replay in the visualizer before changing anything. Pay attention to when units act, how cooldown bars change, and which objects block movement.

Open `my-core-bot/src/main.c`. Your starting bot may already resemble the finished example below; this guide explains why each part exists so you can change it deliberately instead of treating it as magic.

## Build your first unit

There are no fixed Warrior or Miner enums anymore. Open the **Unit Builder** for your event on the CORE website and assemble a unit from the components available there. The builder calculates its cost and final properties, checks the event's construction rules, and gives you the component IDs needed by the C API.

Read the [Unit Builder and property guide](documentation/unit_builder) while making the unit. Do not copy a component list from another event: component IDs, modifications, costs, limits, and validity rules can differ.

For this example, assume the builder offers a `combat` component:

```c
core_action_createUnit("Warrior", "combat", NULL);
```

The first argument is your unit's name. The remaining arguments are component IDs. The list must end with `NULL`:

```c
core_action_createUnit("Tank", "armor", "health", "health", NULL);
```

Repeated components are allowed only when the builder accepts the finished design. Passing `NULL` as the name asks the game to generate one.

The call requests a new unit. The unit does not appear in the current tick. If the request works, you will see the unit on a later tick. Creation can fail because the core is on spawn cooldown, lacks gems, has no free spawn position, or because the component design is invalid. See the [`core_action_createUnit` reference](reference/c/actions/core_action_createUnit) for all failure cases.

## The game loop

Every CORE bot starts the library and gives it a callback:

```c
void ft_on_tick(unsigned long tick);

int main(int argc, char **argv)
{
	return core_startGame("My CORE Bot", argc, argv, ft_on_tick, false);
}
```

`core_startGame` calls `ft_on_tick` whenever a new game state arrives. In that function, you read the current state and choose actions. Those actions happen between ticks. Then `game` is updated and your function is called again.

That boundary explains two important rules:

- A unit requested now is not available in `game.objects` until a later callback.
- Object pointers may change between ticks. Store an object's `id` instead of keeping a `t_obj *` for the next callback.

## Choose how the unit travels

`core_action_travel` makes your unit move across the grid, choosing the most efficient path around obstacles to your goal. One call adds one move or attack. You describe each position with a callback returning this struct:

```c
typedef struct s_travel_surface
{
	int weight;
	bool can_remove;
} t_travel_surface;
```

`weight` is the cost of entering a position. `can_remove` tells the pathfinder whether an object on that position may be attacked as part of the route. It does not make an empty position impassable.

Here is a useful first callback:

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
```

Here is what it means:

- `core_get_obj_from_pos(pos)` checks what is on the position.
- Friendly units and your own core cannot be removed, so they block the route.
- Empty positions cost 1. Occupied positions cost 10, so an open detour is preferred when it is cheaper.
- A non-friendly obstacle is removable. If it becomes the route's next step, travel queues an attack; after it is destroyed, later ticks can move through that position.

The callback should only inspect the game and return a value. Do not add actions or change state inside it. The full [`core_action_travel` reference](reference/c/actions/core_action_travel) explains unreachable goals and negative weights.

Travel is optional. If you want to write your own pathfinder or movement rules, use [`core_action_move`](reference/c/actions/core_action_move) to step into an empty position next to the unit and [`core_action_attack`](reference/c/actions/core_action_attack) to attack a target next to it.

## Put the bot together

```c
#include "bot.h"

#include <stdlib.h>

void ft_on_tick(unsigned long tick);

int main(int argc, char **argv)
{
	return core_startGame("My CORE Bot", argc, argv, ft_on_tick, false);
}

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

void ft_on_tick(unsigned long tick)
{
	(void)tick;

	t_obj *own_core = ft_get_core_own();
	if (own_core && own_core->s_core.spawn_cooldown == 0)
		core_action_createUnit("Warrior", "combat", NULL);

	t_obj *target = ft_get_core_opponent();
	t_obj **units = ft_get_units_own();
	for (size_t i = 0; target && units && units[i]; i++)
		core_action_travel(units[i], target->pos, get_travel_surface);

	free(units);
}
```

Replace `"combat"` with the component list produced by your event's Unit Builder.

The spawn-cooldown check avoids requests that cannot work yet. It does not check cost because the cost belongs to the design shown in the builder. Once the core runs out of gems, creation fails until your strategy earns more.

`ft_get_units_own()` returns a newly allocated, `NULL`-terminated array of pointers to the current objects. The objects belong to the library, but the array belongs to you, which is why the code frees the array and not its elements.

The loop calls travel once per unit. Units whose action cooldown is positive queue nothing; ready units each plan from the current state. Since every call queues at most one adjacent move or attack, repeating this on later ticks advances the route and reacts to changes on the board.

Run `make` and inspect the replay. If creation reports an invalid component, return to the Unit Builder: the tutorial's example component is not a promise about your event.

## Make it yours

The unit name and component list are available in `unit->s_unit.name` and the `NULL`-terminated `unit->s_unit.components` array. Use [`core_get_units_by_name`](reference/c/getters/core_get_units_by_name) when different designs need different jobs. For example, mining units can target deposits while combat units target the opponent.

Do not stop at one callback for every unit. Should a fragile carrier avoid enemies? Should a demolition unit prefer a short route through a wall? Should units avoid a crowded corridor? Give positions different weights to express those choices. If weights do not fit your strategy, use move and attack directly.

Continue with the [getter filtering guide](documentation/getter_filtering), [action execution order](documentation/action_execution_order), and the [Unit Builder property guide](documentation/unit_builder).
