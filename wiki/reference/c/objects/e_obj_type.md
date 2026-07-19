---
title: "🔢 enum e_obj_type"
permalink: "e_obj_type"
sidebarTitle: "🔢 e_obj_type"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/c/client_lib/inc/core_lib.h#L16

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
	OBJ_GEM_PILE
} t_obj_type;
```

## Parameters

- **Cores**: The namesake of CORE GAME. Your team's central hub. When yours is destroyed, you lose, when you destroy your opponents, you win. Also used to spawn new units if it has enough gems stored. There will only ever be one core per team.
- **Unit**: Component-built troops that move, attack, and carry gems.
- **Deposits**: Gems encased in stone. Destroying one turns it into a gem pile.
- **Walls**: Obstacles you must walk around or destroy.
- **Gem Piles**: Gems lying around on the floor.

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

- [🧩 struct s_obj](reference/c/objects/s_obj)
