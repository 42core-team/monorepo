- Every unit has an action cooldown limiting the speed at which the unit can operate. A unit can only execute an action when its action cooldown is 0 or less.
- Each core has a spawn cooldown limiting the speed at which it can spawn new units. A core can only spawn a new unit when it's spawn cooldown is 0.

The cooldowns automatically count down by 1 every tick.

## Unit Action Cooldown

After any action, the unit's action cooldown becomes its `baseActionCooldown` plus something depending on how many gems it carries, with a minimum of 1.

The value the action cooldown is reset to after a unit performed an action is calculated like this:
`max(1, baseActionCooldown + gems / max(1, balancePerCooldownStep))`. Integer division is used.

If a unit is ready and does not act, its `action_cooldown` keeps counting below 0. Once the absolute negative cooldown is greater than `min(the unit's gem-adjusted action cooldown, maxUnitStandingCooldown)`, the server resets the cooldown without executing an action. This prevents a unit from standing ready forever.

e.g.:

```text
- base = 3; step = 15; gems = 0;     => actionCooldown gets reset to 3;
- base = 3; step = 15; gems = 42;    => actionCooldown gets reset to 5;
- base = 0; step = 0;  gems = 99999; => actionCooldown gets reset to 1; unit can immediately act again next tick.
```

## Core Spawn Cooldown

The core spawn cooldown is more straightforward, after a new unit was spawned, it is simply reset to a static config-defined value.

## Examples

::: code-group labels=[C, Go]
```c
int ft_util_actionCooldown(int baseActionCooldown, int balancePerCooldownStep, unsigned long gems)
{
	if (baseActionCooldown < 0) baseActionCooldown = 0;
	if (balancePerCooldownStep < 1) balancePerCooldownStep = 1;

	unsigned long ac = baseActionCooldown + gems / balancePerCooldownStep;
	return ac < 1 ? 1 : (int)ac;
}
```
```go
func predictActionCooldown(baseActionCooldown, balancePerCooldownStep int, gems uint) uint {
	if baseActionCooldown < 0 {
		baseActionCooldown = 0
	}
	if balancePerCooldownStep < 1 {
		balancePerCooldownStep = 1
	}
	ac := uint(baseActionCooldown) + gems/uint(balancePerCooldownStep)
	if ac < 1 {
		ac = 1
	}
	return ac
}
```
:::
