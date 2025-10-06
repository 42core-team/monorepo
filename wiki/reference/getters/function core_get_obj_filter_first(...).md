## URL

https://github.com/42core-team/monorepo/blob/dev/client_lib/src/public/get.c#L63

## Description

Get the first object matching a custom filtering condition. Which one it is is not defined and unpredictable, but most likely the object matching the condition with the smallest id.

> **TIP**: Why is this useful? - Because you won't need an array if you want any instance of an object, regardless of which one specifically. Like when getting your own core or another one-of-a-kind object.

## Signature

```c
t_obj *core_get_obj_filter_first(bool (*condition)(const t_obj *));
```

## Parameters

- `bool (*condition)(const t_obj *)`: Selection function filtering pointer returning if the inputted object should be selected

## Return

- `t_obj *`: The first object that matches the condition or NULL if no such object exists or no condition is provided.

## Examples

## Related
