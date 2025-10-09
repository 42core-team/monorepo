=====
HEADING="🔢 enum e_obj_type"
PERMALINK="e_obj_type"
SIDEBAR_HEADING="🔢 e_obj_type"
=====

## URL

https://github.com/42core-team/monorepo/blob/dev/client_lib/inc/core_lib.h#L16

## Description

Type of object

## Signature

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

## Parameters

- **Cores**: The namesake of CORE GAME. Your team's central hub. When yours is destroyed, you lose, when you destroy your opponents, you win. Also used to spawn new units if it has enough gems stored. There will only ever be one core per team.
- **Unit**: Your pawns, used to execute all the things you want to do in the game. Except for `core_action_createUnit`, all actions in the game are executed by units.
- **Deposits**: Gems encased in stone. Use a miner or another unit to mine it, making the gems drop as a gem pile. Will generally have significantly more gems stored than gem piles spawning normally.
- **Walls**: It's there to be in your way. Walk around it or mine through it.
- **Gem Piles**: Gems lying around on the floor.
- **Bombs**: Careful!

## Examples

```c
static bool is_deposit(const t_obj *obj)
{
	return (obj->type == OBJ_DEPOSIT);
}
static bool is_gems(const t_obj *obj)
{
	return (obj->type == OBJ_GEM_PILE);
}
static bool is_deposit_gems(const t_obj *obj)
{
	return (is_deposit(obj) || is_gems(obj));
}
```

## Related

- [🧩 struct s_obj](../objects/s_obj)
