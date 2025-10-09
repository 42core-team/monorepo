=====
HEADING="⚙️ function core_get_objs_filter_count(...)"
PERMALINK="core_get_objs_filter_count"
SIDEBAR_HEADING="⚙️ get_objs_filter_count()"
=====

## URL

https://github.com/42core-team/monorepo/blob/dev/client_lib/src/public/get.c#L90

## Description

Get the count of objects that match a [custom filtering condition](../../documentation/getter_filtering).

## Signature

```c
unsigned int core_get_objs_filter_count(bool (*condition)(const t_obj *));
```

## Parameters

- `bool (*condition)(const t_obj *)`: Selection function filtering pointer returning if the inputted object should be counted

## Return

- `unsigned int`: Count of objects in the game that match the condition.

## Examples

```c
bool ft_is_warrior(const t_obj *obj)
{
	return (obj->type == OBJ_UNIT && obj->s_unit.unit_type == UNIT_WARRIOR);
}
// ...
printf("Warrior Count: %d\n", core_get_objs_filter_count(ft_is_warrior));

```

```c
int unitToCheck = 0;
long unsigned int teamToCheck = 0;
bool ft_get_count_of_unitToCheck(const t_obj *obj)
{
	return (obj->type == OBJ_UNIT && obj->state == STATE_ALIVE && (int)obj->s_unit.unit_type == unitToCheck && obj->s_unit.team_id == teamToCheck);
}
int get_unit_count(int unitType, int teamId)
{
	unitToCheck = unitType;
	teamToCheck = teamId;
	return (int)core_get_objs_filter_count(ft_get_count_of_unitToCheck);
}
```

## Related

- [🧩 struct s_obj](../objects/s_obj)
- [Getter filtering Documentation](../../documentation/getter_filtering)
