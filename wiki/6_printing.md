> PRINT FUNCTIONS are used to print information about the game state to the console.

# Usage Example:

```c
free(core_print_objs(core_get_objs_filter(ft_is_deposit)));
```

This will get an array of all deposits using the getter system, print it, then free it, all in the same line. *Awesome!*

# Client Lib Functions & Structs

## function `core_print_obj`

Prints all information about the current game state of a given object.

- `obj`: The object to print information about

```c
void core_print_obj(t_obj *obj);
```

---

## function `core_print_objs`

Prints multiple objects.

- `objs`: The objects to print information about.
- `return`: The inputted objects array, so you can easily free in the same line as you print.

```c
t_obj **core_print_objs(t_obj **objs);
```

This returns the objects that were given to it as an input. See the usage example above to find out why this may be useful.

---

## function `core_print_config_unit`

Prints a selected unit config.

- `unit_type`: The type of unit to print the config for

```c
void core_print_config_unit(t_unit_type unit_type);
```

---

## function `core_print_config_game`

Prints the game config.

```c
void core_print_config_game(void);
```

---

## function `core_print_config`

Prints the entire game config and all unit configs

```c
void core_print_config(void);
```
