> ACTION FUNCTIONS are used to perform actions in the game, like creating units, moving them, attacking, etc. Their changes are applied between ticks.

> ACTION FUNCTIONS are the only way to interact with the game state. There is no point in setting the values of object structs locally, as they won't have an effect on the actual gamestate, only actions will.

If you are wondering why a certain action is failing, look through [its code in the server](https://github.com/42core-team/monorepo/tree/dev/server/src/action)! You can see everything that is validated about the action before it is executed there.

All actions from all teams are put into a list which order gets randomized before any actions get executed. That means if two actions conflict each other, a random decision is made, and everything is always 100% fair.

# Position limits

Multiple action functions, like move and build, only work up, down left or right of a units position. Here's an example based on the `core_action_move` function:

```text
*123456
1......
2......
3......
4...u..
5......
6......
```

Say we have unit `u` at [4,4]. The move action will fail if you try to move it anywhere but [3,4], [4,3], [5,4], [4,5].

We still use absolute positions even if almost none of them are valid as converting to a relative position would be annoying and unnecessary in most pathfinding-related scenarios.

# Action Cooldown

Every unit has an action cooldown limiting the speed at which the unit can operate. A unit can only execute an action when it's action cooldown is 0. The action cooldown automatically counts down by 1 every tick. After any action, the unit’s cooldown becomes base + something depending on how many gems it carries (bounded between 1 and max).

The value the action cooldown is reset to after a unit performed an action is calculated like this:
`max( 1, min( {the units maxActionCooldown from config}, {the units baseActionCooldown from config} +  {the units gems} / {the units balancePerCooldownStep from config} ) )`. Integer devision is used.

e.g.:

```text
- base = 3; max = 12; step = 15; gems = 0;     => actionCooldown gets reset to 3;
- base = 3; max = 12; step = 15; gems = 42;    => actionCooldown gets reset to 5;
- base = 3; max = 12; step = 15; gems = 99999; => actionCooldown gets reset to 12;
- base = 0; max = 1;  step = 0;  gems = 99999; => actionCooldown gets reset to 1; unit can immediately act again next tick.
```

# Client Lib Functions & Structs

## function [`core_action_createUnit`](https://github.com/42core-team/monorepo/blob/dev/client_lib/src/public/actions.c#L24)

Create a new unit of specified type.
The unit will be uninitialized. For more info on what that means, check out the objects page (t_obj_state section).

- `unit_type`: The type of unit to create
- `return`: A newly created, uninitialized unit object or NULL if the unit could not be created.

```c
t_obj *core_action_createUnit(t_unit_type unit_type);
```

> **TIP**: Units are spawned as close to your core as possible - but if all positions directly next to your core are occupied, a flood fill algorithm will be used looking for the next empty space and the unit will be placed there. 

**Usage Example (core_action_createUnit)**

```c
if (core_get_objs_filter_count(ft_is_own_team_warrior) < 3)
{
	core_action_createUnit(UNIT_WARRIOR);
}
```

---

## function [`core_action_move`](https://github.com/42core-team/monorepo/blob/dev/client_lib/src/public/actions.c#L59)

Moves a unit to a specific position.
Units can only move one tile up, down, left or right; and only if their action_cooldown is 0.

- `unit`: The unit that should move
- `pos`: The position where the unit should move to

```c
void core_action_move(const t_obj *unit, t_pos pos);
```

**Usage Example (core_action_move)**

```c
t_obj *target = ft_get_target();
t_obj *moving_unit = ft_get_moving_unit();

if (moving_unit->pos.x < target->pos.x)
{
	core_action_move(moving_unit, (t_pos){moving_unit->pos.x + 1, moving_unit->pos.y});
}
if (moving_unit->pos.y < target->pos.y)
{
	core_action_move(moving_unit, (t_pos){moving_unit->pos.x, moving_unit->pos.y + 1});
}
// ...
```

### alternative function [`core_action_moveTowards`](https://github.com/42core-team/monorepo/blob/dev/client_lib/src/public/actions.c#L67)


Alternatively, `core_action_moveTowards` is a very simple pathfinding logic implementation. It won't help you win, but it can be used to easily get started. When you call it, it will determine the next move to make and then immediately either move there or attack objects in its way, provided the objects in its way aren't your units or core.

```c
void core_action_moveTowards(const t_obj *unit, t_pos pos);
```

**Usage Example (core_action_moveTowards)**

```c
t_obj **own_team_warriors = core_get_objs_filter(ft_is_own_team_warrior);
for (int i = 0; own_team_warriors && own_team_warriors[i]; i++)
{
	core_action_moveTowards(own_team_warriors[i], ft_get_core_opponent()->pos);
}
```

---

## function [`core_action_attack`](https://github.com/42core-team/monorepo/blob/dev/client_lib/src/public/actions.c#L124)

Attacks a target position with a unit.
Units can only attack one tile up, down, left or right; and only if their action_cooldown is 0.

- `attacker`: The unit that should attack
- `pos`: The position where the unit should attack

```c
void core_action_attack(const t_obj *attacker, t_pos pos);
```

> This action is used to damage any object, and for destroying deposits, picking up gem piles & lighting a bomb's fuse as well.

> **TIP**: There is friendly fire - you can damage your own units and core. So **BE CAREFUL!**

> **TIP**: Attacking a bomb is what starts its detonation countdown. *Keep calm and don't blow up!*

**Usage Example (core_action_attack)**

```c
t_obj *target = ft_get_target();
t_obj *unit = ft_get_attacker_unit();

if (manhattan_distance(unit->pos, target->pos) <= 1)
{
	core_action_attack(unit, target->pos);
}
```

### alternative function [`core_action_attack_obj`](https://github.com/42core-team/monorepo/blob/dev/client_lib/src/public/actions.c#L133)

Alternatively, you can use `core_action_attack_obj` to be able to pass in the object directly into the attack action function. It will behave the same as calling the main attack action function on the target objects position.

```c
void core_action_attack_obj(const t_obj *attacker, const t_obj *target);
```

**Usage Example (core_action_attackObj)**

```c
t_obj *target = ft_get_target();
t_obj *unit = ft_get_attacker_unit();

if (manhattan_distance(unit->pos, target->pos) <= 1)
{
	core_action_attackObj(unit, target);
}
```

---

## function [`core_action_transferGems`](https://github.com/42core-team/monorepo/blob/dev/client_lib/src/public/actions.c#L139)

Gives gems to another object or drops it on the floor.

- `source`: The object that the gems should be transferred from. (e.g. Core / Unit)
- `target_pos`: The position of the object to transfer the gems to, or the non-occupied position where the gems pile should be dropped
- `amount`: The amount of gems to transfer or drop

```c
void core_action_transferGems(const t_obj *source, t_pos target_pos, unsigned long amount);
```

> **TIP**: If there is no object at the specified position, the gems will be dropped on the floor instead, creating a new gem pile object.

> **TIP**: But what if my core is surrounded by units? How will I get gems to and from it? -> The transferGems action will work back and forth between a unit and its core provided the unit is at *the closest possible unoccupied position* to its core in Manhattan distance. If the core is surrounded, the unit must simply get as close as possible for this action to work then, as determined by a floodfill algorithm and the manhattan distance.

> **TIP**: Unlike other action parameters, the server won't stop executing the transfer gems action if the gems amount parameter is set higher than the possible amount. That means you can pass `99999` or `-1` (the unsigned integer will underflow), even if the source object is only holding `42` gems, to transfer the maximum possible amount of gems anyways.

Alternatively, you can use [`core_action_transferGems_toObj`](https://github.com/42core-team/monorepo/blob/dev/client_lib/src/public/actions.c#L149) to be able to pass in the object directly into the transfer gems action function. It will behave the same as calling the main transfer gems action function on the target objects position.

```c
void core_action_transferGems_toObj(const t_obj *source, t_obj *target, unsigned long amount);
```

**Usage Example (core_action_transferGems_toObj)**

```c
t_obj *nearest_deposit = ft_get_deposit_nearest(obj->pos);
if (nearest_deposit && obj->s_unit.gems <= 0)
	core_action_moveTowards(obj, nearest_deposit->pos);
else
{
	core_action_moveTowards(obj, ft_get_core_own()->pos);
	core_action_transferGems_toObj(obj, ft_get_core_own(), -1);
}
```

---

## function [`core_action_build`](https://github.com/42core-team/monorepo/blob/dev/client_lib/src/public/actions.c#L155)

Builds a new object.
Objects can only be built one tile up, down, left or right from the builder; and only if their builders action_cooldown is 0.

- `builder`: The unit object that has a unit type that is able to build. Must have a `t_build_type`that is not `BUILD_TYPE_NONE` in the config.
- `pos`: The position to build the object at.

```c
void core_action_build(const t_obj *builder, t_pos pos);
```

The `t_build_type` of the builder unit in the config will determine what gets built (e.g. bomb or wall).

**Usage Example (core_action_build)**

```c
t_obj *unit = ft_get_wall_builder_unit();
if (unit && unit->s_unit.gems >= game.config.wall_build_cost)
{
	core_action_build(unit, (t_pos){ unit->pos.x + 1, unit->pos.y });
}
```
