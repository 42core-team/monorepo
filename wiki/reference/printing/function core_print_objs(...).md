## URL

https://github.com/42core-team/monorepo/blob/dev/client_lib/src/public/printing.c#L53

## Description

Prints multiple objects.

## Signature

```c
t_obj **core_print_objs(t_obj **objs);
```

## Parameters

- `objs`: The objects to print information about.
- `return`: The inputted objects array, so you can easily free in the same line as you print.

## Examples

```c
free(core_print_objs(core_get_objs_filter(ft_is_deposit)));
```

This example will get an array of all deposits using the getter system, print it, then free it, all in the same line. *Awesome!*

## Related
