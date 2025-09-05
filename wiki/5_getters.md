# ----- GETTER FUNCTIONS -----

> GETTER FUNCTIONS are used to get information about the current game state. They are **highly useful** and it is suggested to utilize them, though if you don't want to you can just loop over all the data in `game.objects`.

> **TIP**: If you are unsure about how to use the getter functions and the function filtering pointers, you are provided with a `getter.c` file that uses them and provides some basic functions with them.

## `core_get_obj_from_id`

Get any object based on its id.
- `return`: The object or NULL if no such object exists.

```c
t_obj *core_get_obj_from_id(unsigned long id);
```

## `core_get_obj_from_pos`

Get any object based on its position.
- `return`: The object at the position or NULL if no such object exists.

```c
t_obj *core_get_obj_from_pos(t_pos pos);
```

## `core_get_objs_filter`

Get all objects matching a custom filtering condition.

- `condition`: Selection function pointer returning if the inputted object should be selected
- `return`: Null-terminated array of selected objects or NULL if no condition is provided or no objects match the condition.

```c
t_obj **core_get_objs_filter(bool (*condition)(const t_obj *));
```

> **WARNING**: You are responsibility to free the array returned by this function, but not the objects in it themselves, they are the same object instances as in the `game.objects` array.

## `core_get_obj_filter_first`

Get the first object matching a custom filtering condition. Which one it is is not defined and unpredictable.

- `condition`: Selection function filtering pointer returning if the inputted object should be selected
- `return`: The first object that matches the condition or NULL if no such object exists or no condition is provided.

```c
t_obj *core_get_obj_filter_first(bool (*condition)(const t_obj *));
```

> **TIP**: Why is this useful? - Because you won't need an array if you want any instance of an object, regardless of which one specifically. Like when getting your own core.

## `core_get_obj_filter_nearest`

Get the nearest object to a given position matching a custom filtering condition.

- `pos`: Position to search from
- `condition`: Selection function pointer returning if the inputted object should be selected
- `return`: The nearest object that matches the condition or NULL if no such object exists or no condition is provided.

```c
t_obj *core_get_obj_filter_nearest(t_pos pos, bool (*condition)(const t_obj *));
```

## `core_get_objs_filter_count(bool (*condition)(const t_obj *))

Get the count of objects that match a certain custom filtering condition.

- `condition`: Selection function filtering pointer returning if the inputted object should be counted
- `return`: Count of objects in the game that match condition.

```c
unsigned int core_get_objs_filter_count(bool (*condition)(const t_obj *));
```

---

## `core_get_unitConfig`

Get the unit config for a specific unit type.

- `type`: The type of unit to get the config for
- `return`: The unit config or NULL if no such unit type or unit config exists.

```c
t_unit_config *core_get_unitConfig(t_unit_type type);
```
