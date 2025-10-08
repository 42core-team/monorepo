=====
HEADING="⚙️ function core_get_obj_from_id(...)"
PERMALINK="core_get_obj_from_id"
SIDEBAR_HEADING="⚙️ get_obj_from_id()"
=====

## URL

https://github.com/42core-team/monorepo/blob/dev/client_lib/src/public/get.c#L4

## Description

Get any object based on its id.

## Signature

```c
t_obj *core_get_obj_from_id(unsigned long id);
```

## Parameters

- `unsigned long id`: The id of the object the function should get

## Return

- `t_obj *`: The selected object with a matching id or NULL if the selection didn't match anything.

## Examples

```c
// we only need one carrier for now
int carrier_id = -1;

void ft_on_tick(unsigned long tick)
{
	if (carrier_id >= 0)
	{
		t_obj *carrier = core_get_obj_by_id(carrier_id);
```

## Related
