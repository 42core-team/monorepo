# Connection Library API Documentation

This document provides documentation for the public API of the Connection Library.

## Table of Contents

### Enumerations

- [e_obj_type](#obj-type)
- [e_obj_state](#obj-state)
- [e_unit_type](#unitype)
- [e_build_type](#build-type)

### Data Structures

- [s_unit_config](#uniconfig)
- [s_obj](#obj)
- [s_config](#config)
- [s_game](#game)
- [s_pos](#pos)

### Functions

- [core_startGame](#core-startgame)
- [core_get_obj_from_id](#core-get-obj-from-id)
- [core_get_obj_from_pos](#core-get-obj-from-pos)
- [core_get_objs_customCondition](#core-get-objs-customcondition)
- [core_get_obj_customCondition_first](#core-get-obj-customcondition-first)
- [core_get_obj_customCondition_nearest](#core-get-obj-customcondition-nearest)
- [core_get_unitConfig](#core-get-unitconfig)
- [core_action_createUnit](#core-action-createunit)
- [core_action_move](#core-action-move)
- [core_action_attack](#core-action-attack)
- [core_action_transferMoney](#core-action-transfermoney)
- [core_action_build](#core-action-build)
- [core_print_object](#core-print-object)
- [core_print_objects](#core-print-objects)
- [core_print_config_unit](#core-print-config-unit)
- [core_print_config](#core-print-config)

## Enumerations

### e_obj_type

**Brief:** Type of object.

**Values:**

| Name | Value | Description |
|------|-------|-------------|
| `OBJ_CORE` | `-` | - |
| `OBJ_UNIT` | `-` | - |
| `OBJ_RESOURCE` | `-` | - |
| `OBJ_WALL` | `-` | - |
| `OBJ_MONEY` | `-` | - |
| `OBJ_BOMB` | `-` | - |

---

### e_obj_state

**Brief:** Object state.

**Description:** Uninitialized objects should only have their type, state, data, team_id & unit_type read and set.

**Values:**

| Name | Value | Description |
|------|-------|-------------|
| `STATE_UNINITIALIZED` | `1` | - |
| `STATE_ALIVE` | `2` | - |
| `STATE_DEAD` | `3` | - |

---

### e_unit_type

**Brief:** Type of unit.

**Values:**

| Name | Value | Description |
|------|-------|-------------|
| `UNIT_WARRIOR` | `0` | - |
| `UNIT_MINER` | `1` | - |
| `UNIT_CARRIER` | `2` | - |
| `UNIT_BUILDER` | `3` | - |
| `UNIT_BOMBERMAN` | `4` | - |

---

### e_build_type

**Brief:** Determines what a unit can build.

**Values:**

| Name | Value | Description |
|------|-------|-------------|
| `BUILD_TYPE_NONE` | `0` | - |
| `BUILD_TYPE_WALL` | `1` | - |
| `BUILD_TYPE_BOMB` | `2` | - |

---

## Data Structures

### s_unit_config

**Members:**

| Type | Name | Description |
|------|------|-------------|
| `char *` | `name` | The name of the unit. |
| `t_unit_type` | `unit_type` | The unit type of the unit. |
| `unsigned long` | `cost` | What the unit costs to create. |
| `unsigned long` | `hp` | How much healthpoints the unit has. |
| `unsigned long` | `dmg_core` | How much damage the unit deals to cores. |
| `unsigned long` | `dmg_unit` | How much damage the unit deals to units. |
| `unsigned long` | `dmg_resource` | How much damage the unit deals to resources. |
| `unsigned long` | `dmg_wall` | How much damage the unit deals to walls. |
| `t_build_type` | `build_type` | The units build type. |
| `unsigned long` | `speed` | The time a unit waits between moves if it is not carrying money. |
| `unsigned long` | `min_speed` | The minimum time a unit waits between moves. |
| `bool` | `can_build` | Whether the unit can build walls or bombs. |

---

### s_obj

**Brief:** Game object structure representing all entities in the game.

**Members:**

| Type | Name | Description |
|------|------|-------------|
| `t_obj_type` | `type` | Type of the obj. |
| `t_obj_state` | `state` | State of the obj. |
| `void *` | `data` | Custom data, save whatever you want here. |
| `unsigned long` | `id` | The unique id of the obj. |
| `t_pos` | `pos` | The position of the obj. |
| `unsigned long` | `hp` | The current healthpoints of the obj. |
| `unsigned long` | `team_id` | The id of the team that owns the core. |
| `unsigned long` | `balance` | The current balance of the core. |
| `structs_obj` | `s_core` | - |
| `unsigned long` | `unit_type` | Which type of unit this is. |
| `unsigned long` | `move_cooldown` | Countdown to the next tick the unit can move, defined by it's speed & how much it's carrying. |
| `structs_obj` | `s_unit` | - |
| `structs_obj` | `s_resource_money` | - |
| `unsigned long` | `countdown` | How much longer the bomb will take to explode. |
| `structs_obj` | `s_bomb` | - |
| `unions_obj` | `None` | - |

---

### s_config

**Brief:** Game configuration structure containing all game settings.

**Members:**

| Type | Name | Description |
|------|------|-------------|
| `unsigned long` | `height` | The height of the map. |
| `unsigned long` | `width` | The width of the map. |
| `unsigned long` | `tick_rate` | How many ticks there are in one second. |
| `unsigned long` | `idle_income` | How much idle income you get every second. |
| `unsigned long` | `idle_income_timeout` | How many ticks you get idle income. |
| `unsigned long` | `resource_hp` | How much healthpoints a resource has at the start of the game. |
| `unsigned long` | `resource_income` | How much income you get when you destroy a resource. |
| `unsigned long` | `core_hp` | How much healthpoints a core has at the start of the game. |
| `unsigned long` | `initial_balance` | How much money a team starts with. |
| `unsigned long` | `wall_hp` | How much healthpoints a wall has at the start of the game. |
| `unsigned long` | `wall_build_cost` | How much it costs for a builder to build a wall. |
| `unsigned long` | `bomb_hp` | How much healthpoints a bomb has. |
| `unsigned long` | `bomb_countdown` | How many ticks a bomb takes to explode after being thrown. |
| `unsigned long` | `bomb_throw_cost` | How much it costs to throw a bomb. |
| `unsigned long` | `bomb_reach` | How big the explosion of a bomb is. |
| `unsigned long` | `bomb_damage` | How much damage a bomb does to objects hit by its explosion. |
| `t_unit_config **` | `units` | List of all unit types that are available in the game. NULL-terminated. |

---

### s_game

**Brief:** Contains all the data about the game. Read it to your liking! Access it anywhere by typing

**Members:**

| Type | Name | Description |
|------|------|-------------|
| `unsigned long` | `elapsed_ticks` | The elapsed ticks since the game started. |
| `t_config` | `config` | The config contains base informations about the game that don't change like the map size and the unit types. |
| `unsigned long` | `my_team_id` | The id of the team that you are playing for. Saved in your cores team_id field. |
| `t_obj **` | `objects` | List of all objects (units, cores, resources, etc.) and their informations. NULL-terminated. |

---

### s_pos

**Brief:** Position structure for 2D coordinates.

**Members:**

| Type | Name | Description |
|------|------|-------------|
| `unsigned short` | `x` | X coordinate. |
| `unsigned short` | `y` | Y coordinate. |

---

## Functions

### core_startGame

```c
int core_startGame(const char *team_name, int argc, char **argv, void(*tick_callback)(unsigned long), bool debug)
```

**Brief:** Starts the connection lib up, initializes the game, connects to the server & starts the game.

**Parameters:**

- `team_name`: Name of your team
- `argc`: Argument count from main function
- `argv`: Arguments from main function
- `tick_callback`: Function that will be called every game tick once new server data is available.
- `debug`: Set to true to enable extensive logging.

**Returns:** 0 on success, another number on failure.

---

### core_get_obj_from_id

```c
t_obj * core_get_obj_from_id(unsigned long id)
```

**Brief:** GETTER FUNCTIONS are used to get information about the current game state.

**Description:** Get any object based on its id.

**Returns:** The object or NULL if no such object exists.

---

### core_get_obj_from_pos

```c
t_obj * core_get_obj_from_pos(t_pos pos)
```

**Brief:** Get any object based on its position.

**Returns:** The object at the position or NULL if no such object exists.

---

### core_get_objs_customCondition

```c
t_obj ** core_get_objs_customCondition(bool(*condition)(const t_obj *))
```

**Brief:** Get all objects matching a custom condition.

**Parameters:**

- `condition`: Selection function pointer returning if the inputted object should be selected

**Returns:** Null-terminated array of selected objects or NULL if no condition is provided or no objects match the condition.

---

### core_get_obj_customCondition_first

```c
t_obj * core_get_obj_customCondition_first(bool(*condition)(const t_obj *))
```

**Brief:** Get the first object matching a custom condition.

**Parameters:**

- `condition`: Selection function pointer returning if the inputted object should be selected

**Returns:** The first object that matches the condition or NULL if no such object exists or no condition is provided.

---

### core_get_obj_customCondition_nearest

```c
t_obj * core_get_obj_customCondition_nearest(t_pos pos, bool(*condition)(const t_obj *))
```

**Brief:** Get the nearest object to a given position matching a custom condition.

**Parameters:**

- `pos`: Position to search from
- `condition`: Selection function pointer returning if the inputted object should be selected

**Returns:** The nearest object that matches the condition or NULL if no such object exists or no condition is provided.

---

### core_get_unitConfig

```c
t_unit_config * core_get_unitConfig(t_unit_type type)
```

**Brief:** Get the unit config for a specific unit type.

**Parameters:**

- `type`: The type of unit to get the config for.

**Returns:** The unit config or NULL if no such unit type or unit config exists.

---

### core_action_createUnit

```c
t_obj * core_action_createUnit(t_unit_type unit_type)
```

**Brief:** Create a new unit of specified type.

**Description:** The unit will be uninitialized, meaning you can read & write only read its type, state, data, team_id & unit_type.

**Parameters:**

- `unit_type`: The type of unit to create.

**Returns:** A newly created, uninitialized unit object or NULL if the unit could not be created.

---

### core_action_move

```c
void core_action_move(const t_obj *unit, t_pos pos)
```

**Brief:** Moves a unit to a specific position.

**Description:** Units can only move one tile up, down, left or right; and only if their move_cooldown is 0.

**Parameters:**

- `unit`: The unit that should move.
- `pos`: The position where the unit should move to.

---

### core_action_attack

```c
void core_action_attack(const t_obj *attacker, t_pos pos)
```

**Brief:** Attacks a target position with a unit.

**Description:** Units can only attack one tile up, down, left or right; and only if their move_cooldown is 0.

**Parameters:**

- `attacker`: The unit that should attack.
- `pos`: The position where the unit should attack.

---

### core_action_transferMoney

```c
void core_action_transferMoney(const t_obj *source, t_pos target_pos, unsigned long amount)
```

**Brief:** Gives money to another object or drops it on the floor.

**Parameters:**

- `source`: The object that the money should be transferred from.
- `target_pos`: The position of the object to transfer the money to, or the non-occupied position where the money should be dropped.
- `amount`: The amount of money to transfer or drop.

---

### core_action_build

```c
void core_action_build(const t_obj *builder, t_pos pos)
```

**Brief:** Builds a new object.

**Description:** Units can only build one tile up, down, left or right.

**Parameters:**

- `builder`: The unit that should build a new object. What will be built depends on the buildType of the builder unit.
- `pos`: The position where the object should be built.

---

### core_print_object

```c
void core_print_object(const t_obj *obj)
```

**Brief:** Prints all information about the current game state of a given object.

**Parameters:**

- `obj`: The object to print information about.

---

### core_print_objects

```c
void core_print_objects(bool(*condition)(const t_obj *))
```

**Brief:** Prints all objects that match a custom condition.

**Parameters:**

- `condition`: Selection function pointer returning if the inputted object should be selected

---

### core_print_config_unit

```c
void core_print_config_unit(t_unit_type unit_type)
```

**Brief:** Prints a selected unit config.

**Parameters:**

- `unit_type`: The type of unit to print the config for.

---

### core_print_config

```c
void core_print_config(void)
```

**Brief:** Prints the entire game config and all unit configs.

---

