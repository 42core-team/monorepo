## URL

https://github.com/42core-team/monorepo/blob/dev/client_lib/src/public/get.c#L40

## Description

Get all objects matching a custom filtering condition.

> **WARNING**: You are responsibility to **free the array** returned by this function, but not the objects in it themselves, they are the same object instances as in the `game.objects` array.

## Signature

```c
t_obj **core_get_objs_filter(bool (*condition)(const t_obj *));
```

## Parameters

- `bool (*condition)(const t_obj *)`: Selection function pointer returning if the inputted object should be selected

## Return

- `t_obj **`: Null-terminated array of selected objects or NULL if no condition is provided or no objects match the condition. **You are responsible to free the returned array.**

## Examples

```c
t_obj **warriors = core_get_objs_filter(ft_is_warrior);
for (int i = 0; warriors && warriors[i]; i++)
{
	// warrior move & attack logic
}
free(warriors);
```

## Related
