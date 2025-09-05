# ----- OBJECTS -----

## `t_obj`

Game object structure representing all entities in the game

- `type`: Type of the obj
- `state`: State of the obj
- `data`: Custom data, save whatever you want here, it will persist across ticks.
- `id`: The unique id of the obj
- `pos`: The position of the obj
- `hp`: The current healthpoints of the obj
- `s_core.team_id`: The id of the team that owns the core.
- `s_core.gems`: The current gems stored in the core.
- `s_unit.unit_type`: Which type of unit this is.
- `s_unit.team_id`: The id of the team that owns the unit.
- `s_unit.gems`: The amount of gems the unit is carrying.
- `s_unit.action_cooldown`: Countdown to the next tick the unit can move, defined by it's action cooldown & how many gems it's carrying.
- `s_resource_gems_pile.gems`: The amount of gems the deposit or gem pile contains.
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
		} s_resource_gems_pile;
		struct
		{
			unsigned long countdown;
		} s_bomb;
	};
} t_obj;
```

> **TIP**: The `void *data` field is **especially powerful & useful**. The library will never touch this field, it's yours to mess with for whatever you want, and it can already safely be set when a unit is still uninitialized, so immediately after unit creation, in the same tick. It's generally used to store specific jobs, tasks or targets of the unit in the easiest way possible.

> **WARNING**: Objects that have `state` set to `STATE_UNINITIALIZED` can only be partially interacted with. Their id field, for example, is just filled with random bits. See down at `t_obj_state`for specifics.

## `t_obj_type`

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

- **Cores**: Your team's central hub. When your's is destroyed, you lose, when you destroy your opponents, you win. Also used to spawn new units if it has enough gems stored. And it is the namesake of CORE GAME. There will only ever be one core per team.
- **Unit**: Your pawns, used to execute all the things you want to do in the game. Except for core_action_createUnit, all actions in the game are executed by units.
- **Deposits**: Gems encased in stone. Use a miner or another unit to mine it, making the gems drop as a gem pile. Will generally have significantly more gems stored than gem piles spawning normally.
- **Walls**: It's there to be in your way. Walk around it or mine through it.
- **Gem Piles**: Gems lying around on the floor.
- **Bombs**: Careful!

## `t_obj_state`

Object state.

```c
typedef enum e_obj_state
{
    STATE_UNINITIALIZED = 1,
	STATE_ALIVE = 2,
	STATE_DEAD = 3
} t_obj_state;
```

> Uninitialized objects should only have their type, state, data, team_id & unit_type read and set.

> Be careful to verify that a unit is alive before executing an action it. Otherwise, the action will fail.

> **TIP**: Why are uninitialized units even a thing? Because even though when you call `core_action_createUnit` the unit will have a delay of one tick until it can spawn, the memory position will still stay consistent. You can therefore already give the unit a specific task / job, set their data field, save it in your structs, whatever you want.

> **TIP**: Why are dead units a thing? So you can free the things you've allocated in your objects `void *data` field and other places before the program ends, preventing memory leaks.

## `t_unit_type`

Type of unit. Which are available differs from event to event

> **TIP**: Type `UNIT_` and autocomplete will show you all the available units. There is an overview of all the unit types and their properties and settings in the config.

## `t_pos`

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
