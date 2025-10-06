## URL

https://github.com/42core-team/monorepo/blob/dev/client_lib/src/public/actions.c#L133

## Description

You can use `core_action_attack_obj` to be able to pass in the object directly into the attack action function. It will behave the same as calling the main attack action function on the target objects position.

## Signature

```c
void core_action_attack_obj(const t_obj *attacker, const t_obj *target);
```

## Parameters

- `attacker`: The unit that should attack
- `target`: The object the unit should attack

## Examples

```c
t_obj *target = ft_get_target();
t_obj *unit = ft_get_attacker_unit();

if (manhattan_distance(unit->pos, target->pos) <= 1)
{
	core_action_attackObj(unit, target);
}
```

## Related
