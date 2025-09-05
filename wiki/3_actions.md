# ----- ACTION FUNCTIONS -----

> ACTION FUNCTIONS are used to perform actions in the game, like creating units, moving them, attacking, etc. Their changes are applied between ticks.

> ACTION FUNCTIONS are the only way to interact with the game state. Use them to perform actions.

## Position limits

Multiple action functions, like move and build, only work up, down left or right of a units position. Here's an example based on the `core_action_move` function:

```
*123456
1......
2......
3......
4...u..
5......
6......
```

Say we have unit `u` at [4,4]. The move action will fail if you try to move it anywhere but [3,4], [4,3], [5,4], [4,5].

We still use absolute positions even if almost none of them are valid as converting to a relative position would be pain.

## `core_action_createUnit`

Create a new unit of specified type.
The unit will be uninitialized, meaning you can read only its type, state, team_id & unit_type; and write only its data. Next tick it will be a normal unit spawned next to your core.

- `unit_type`: The type of unit to create
- `return`: A newly created, uninitialized unit object or NULL if the unit could not be created.

```c
t_obj *core_action_createUnit(t_unit_type unit_type);
```

> **TIP**: Units are spawned as close to your core as possible - but if all positions directly next to your core are occupied, a flood fill algorithm will be used looking for the next empty space and the unit will be placed there. 

## `core_action_move`

Moves a unit to a specific position.
Units can only move one tile up, down, left or right; and only if their action_cooldown is 0.

- `unit`: The unit that should move
- `pos`: The position where the unit should move to

```c
void core_action_move(const t_obj *unit, t_pos pos);
```

## `core_action_attack`

Attacks a target position with a unit.
Units can only attack one tile up, down, left or right; and only if their action_cooldown is 0.

- `attacker`: The unit that should attack
- `pos`: The position where the unit should attack

```c
void core_action_attack(const t_obj *attacker, t_pos pos);
```

> This action is used to damage any object, and for destroying deposits, picking up gem piles & lighting a bombs fuse as well.

> **TIP**: There is friendly fire - you can damage your own units and core. So **BE CAREFUL!**

> **TIP**: Attacking a bomb is what starts its detonation countdown. *Keep calm and don't blow up!*

## `core_action_transferGems`

Gives gems to another object or drops it on the floor.

- `source`: The object that the gems should be transferred from. (e.g. Core / Unit)
- `target_pos`: The position of the object to transfer the gems to, or the non-occupied position where the gems pile should be dropped
- `amount`: The amount of gems to transfer or drop

```c
void core_action_transferGems(const t_obj *source, t_pos target_pos, unsigned long amount);
```

> **TIP**: But what if my core is surrounded by units? How will I get gems to and from it? -> The transferGems action will work back and forth between a unit and its core provided the unit is at *the closest possible unoccupied position* to its core. If the core is surrounded, the unit must simply get as close as possible for this action to work then, as determined by a floodfill algorithm and the manhattan distance.

## `core_action_build`

Builds a new object.
Objects can only be built one tile up, down, left or right from the builder; and only if their builders action_cooldown is 0.

- `builder`: The unit object that has a unit type that is able to build. Must have a `t_build_type`that is not `BUILD_TYPE_NONE` in the config.
- `pos`: The position to build the object at.

```c
void core_action_build(const t_obj *builder, t_pos pos);
```

The `t_build_type` of the builder unit in the config will determine what gets built (e.g. bomb or wall).
