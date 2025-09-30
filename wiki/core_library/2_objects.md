Everything that can be on the game grid is an object.

There are never two objects in one grid position.

Each object has a unique id that never changes and never gets reused.

# Bomb Explosions

To start a bombs countdown, attack it.

The bombs countdown will then continue to decrement by 1 each tick. After a bombs countdown has ended, an explosion will trigger. The explosion will realistically flood out, stopping only at walls.

If an idle or still-counting-down bomb is within range of an explosion, it will explode immediately, creating a chain reaction.

If the bomb receives enough damage while its countdown is going down to reach 0 hp before its explosion, the bomb will be defused, destroying it without causing an explosion.

The algorithm used to determine whether a given grid position should be hit with explosion damage is a sort of reversed Bresenham / raycast type algorithm. From every possible position in range that could explode, we'll draw a metaphorical line between the center of that grid position and the center of the bomb. If this line crosses over any walls, the position won't explode, otherwise it will. If the line crosses over an edge of a wall exactly, that won't count as the wall overlapping.

The bomb reach is in addition the the tile the bomb is standing on, so a bomb reach of three would result in an explosion with a diameter of 7.

For more info & specifics, please check out [the server bomb code](https://github.com/42core-team/monorepo/blob/dev/server/src/object/Bomb.cpp) directly.

# Client Lib Functions & Structs

## struct [`t_obj`](https://github.com/42core-team/monorepo/blob/dev/client_lib/inc/core_lib.h#L45)

Game object structure representing all game objects.

- `type`: Type of the obj
- `state`: State of the obj
- `data`: Custom data, save whatever you want here, it will persist across ticks.
- `id`: The unique id of the obj
- `pos`: The position of the obj
- `hp`: The current healthpoints of the obj
---
- `s_core.team_id`: The id of the team that owns the core.
- `s_core.gems`: The current gems stored in the core.
- `s_core.spawn_cooldown`: Countdown to the next tick the core can spawn a unit, defined by core_spawn_cooldown in the config.
---
- `s_unit.unit_type`: Which type of unit this is.
- `s_unit.team_id`: The id of the team that owns the unit.
- `s_unit.gems`: The amount of gems the unit is carrying.
- `s_unit.action_cooldown`: Countdown to the next tick the unit can move, defined by it's action cooldown & how many gems it's carrying.
---
- `s_deposit_gems_pile.gems`: The amount of gems the deposit or gem pile contains.
---
- `s_bomb.countdown`: The time until the bomb will explode. **TAKE COVER!**

```c
typedef struct s_obj
{
	t_obj_type type;
	t_obj_state state;
	void *data;
	unsigned long id;
	t_pos pos;
	unsigned long hp;
	union
	{
		struct
		{
			unsigned long team_id;
			unsigned long gems;
			unsigned long spawn_cooldown;
		} s_core;
		struct
		{
			unsigned long unit_type;
			unsigned long team_id;
			unsigned long gems;
			unsigned long action_cooldown;
		} s_unit;
		struct
		{
			unsigned long gems;
		} s_deposit_gems_pile;
		struct
		{
			unsigned long countdown;
		} s_bomb;
	};
} t_obj;
```

Walls don't have any special properties so they don't need a unionized struct, deposits and gem piles share the same additional properties so they share the same unionized struct (`s_deposit_gems_pile`).

There is no limit to how many gems units or cores can hold.

> **TIP**: The `void *data` field is **especially powerful & useful**. The library will never touch this field, it's yours to mess with for whatever you want, and it can already safely be set when a unit is still uninitialized, so immediately after unit creation, in the same tick. It's generally used to store specific jobs, tasks or targets of the unit in the easiest way possible.

> **WARNING**: Objects that have `state` set to `STATE_UNINITIALIZED` can only be partially interacted with. See down at `t_obj_state`for specifics.

---

## struct [`t_obj_type`](https://github.com/42core-team/monorepo/blob/dev/client_lib/inc/core_lib.h#L16)

Type of object

```c
typedef enum e_obj_type
{
	OBJ_CORE,
	OBJ_UNIT,
	OBJ_DEPOSIT,
	OBJ_WALL,
	OBJ_GEM_PILE,
	OBJ_BOMB
} t_obj_type;
```

- **Cores**: The namesake of CORE GAME. Your team's central hub. When yours is destroyed, you lose, when you destroy your opponents, you win. Also used to spawn new units if it has enough gems stored. There will only ever be one core per team.
- **Unit**: Your pawns, used to execute all the things you want to do in the game. Except for `core_action_createUnit`, all actions in the game are executed by units.
- **Deposits**: Gems encased in stone. Use a miner or another unit to mine it, making the gems drop as a gem pile. Will generally have significantly more gems stored than gem piles spawning normally.
- **Walls**: It's there to be in your way. Walk around it or mine through it.
- **Gem Piles**: Gems lying around on the floor.
- **Bombs**: Careful!

---

## struct [`t_obj_state`](https://github.com/42core-team/monorepo/blob/dev/client_lib/inc/core_lib.h#L28)

Object state.

```c
typedef enum e_obj_state
{
    STATE_UNINITIALIZED = 1,
	STATE_ALIVE = 2,
	STATE_DEAD = 3
} t_obj_state;
```

Units will stay uninitialized during the same tick they were created. In the next tick, they will be full, normal units. Uninitialized objects should only have their type, state, team_id & unit_type read and their data field set.

> **WARNING**: Be careful to verify that a unit is alive before executing an action it. Otherwise, the action will fail.

> **TIP**: Why are uninitialized units even a thing? Because even though when you call `core_action_createUnit` the unit will have a delay of one tick until it can spawn, the memory position will still stay consistent. You can therefore already give the unit a specific task / job, set their data field, save it in your structs, whatever you want.

> **TIP**: Why are dead units a thing? So you can free the things you've allocated in your objects `void *data` field and other places before the program ends, preventing memory leaks. Similarly, you can, thanks to this, save pointers to the unit anywhere and they won't start dangling when the unit is killed. A dead unit is no longer considered anywhere else by the server, so you can walk and build on their corpses. 👀

---

## struct [`t_unit_type`](https://github.com/42core-team/monorepo/blob/dev/client_lib/inc/units.h#L5)

Type of unit. Which are available differs from event to event

> **TIP**: Type `UNIT_` and autocomplete will show you all the available units. There is an overview of all the unit types and their properties and settings in the config.

---

## struct [`t_pos`](https://github.com/42core-team/monorepo/blob/dev/client_lib/inc/core_lib.h#L36)

Position structure for 2D coordinates

- `x`: X coordinate
- `y`: Y coordinate

```c
typedef struct s_pos
{
	unsigned short x;
	unsigned short y;
} t_pos;
```

> Positions are 0-indexed, so position [0,0] and (assuming a gridSize of 20 in the config) [19, 19] are valid positions, but [-1,-1] and [20,20] are *not*.

> There can never be two objects at the same position. To check whether there is something at a given position, use `core_get_obj_from_pos()`. If it returns `NULL`, there's nothing there.
