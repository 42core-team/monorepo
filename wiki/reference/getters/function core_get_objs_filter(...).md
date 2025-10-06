## URL

https://github.com/42core-team/monorepo/blob/dev/client_lib/src/public/get.c#L40

## Description

Get all objects matching a custom filtering condition.

> **WARNING**: You are responsibility to free the array returned by this function, but not the objects in it themselves, they are the same object instances as in the `game.objects` array.

## Signature

```c
t_obj **core_get_objs_filter(bool (*condition)(const t_obj *));
```

## Parameters

- `condition`: Selection function pointer returning if the inputted object should be selected
- `return`: Null-terminated array of selected objects or NULL if no condition is provided or no objects match the condition.

## Examples

## Related
