## URL

https://github.com/42core-team/monorepo/blob/dev/client_lib/src/public/get.c#L72

## Description

Get the nearest object to a given position matching a custom filtering condition. Manhattan distance is used to determine distance. If two objects or more have the same distance, either could be picked.

## Signature

```c
t_obj *core_get_obj_filter_nearest(t_pos pos, bool (*condition)(const t_obj *));
```

## Parameters

- `t_pos pos`: Position to search from
- `bool (*condition)(const t_obj *)`: Selection function pointer returning if the inputted object should be selected

## Return

- `t_obj *`: The nearest object that matches the condition or NULL if no such object exists or no condition is provided.

## Examples

## Related
