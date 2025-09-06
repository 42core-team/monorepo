# Printing

> PRINT FUNCTIONS are used to print information about the game state to the console.

## Usage Example:

```C
free(core_print_objs(core_get_objs_filter(ft_is_deposit)));
```

This will allocate an array of all deposits, print it, then free it, all in the same line. *Awesome!*

## `core_print_obj`

Prints all information about the current game state of a given object.

- `obj`: The object to print information about

```c
void core_print_obj(t_obj *obj);
```

## `core_print_objs`

Prints multiple objects.

- `objs`: The objects to print information about.
- `return`: The inputted objects array, so you can easily free in the same line as you print.

```c
t_obj **core_print_objs(t_obj **objs);
```

## `core_print_config_unit`

Prints a selected unit config.

- `unit_type`: The type of unit to print the config for

```c
void core_print_config_unit(t_unit_type unit_type);
```

## `core_print_config_game`

Prints the game config.

```c
void core_print_config_game(void);
```

## `core_print_config`

Prints the entire game config and all unit configs

```c
void core_print_config(void);
```
