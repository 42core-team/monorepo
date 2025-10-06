## URL

https://github.com/42core-team/monorepo/blob/dev/client_lib/src/public/get.c#L92

## Description

Get the count of objects that match a certain custom filtering condition.

## Signature

```c
unsigned int core_get_objs_filter_count(bool (*condition)(const t_obj *));
```

## Parameters

- `condition`: Selection function filtering pointer returning if the inputted object should be counted
- `return`: Count of objects in the game that match condition.

## Examples

## Related
