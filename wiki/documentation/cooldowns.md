- Every unit has an action cooldown limiting the speed at which the unit can operate. A unit can only execute an action when it's action cooldown is 0.
- Each core has a spawn cooldown limiting the speed at which it can spawn new units. A core can only spawn a new unit when it's spawn cooldown is 0.

The cooldowns automatically count down by 1 every tick.

## Unit Action Cooldown

After any action, the unit’s action cooldown becomes base + something depending on how many gems it carries (bounded between 1 and max).

The value the action cooldown is reset to after a unit performed an action is calculated like this:
`max( 1, min( {the units maxActionCooldown from config}, {the units baseActionCooldown from config} +  {the units gems} / {the units balancePerCooldownStep from config} ) )`. Integer devision is used.

e.g.:

```text
- base = 3; max = 12; step = 15; gems = 0;     => actionCooldown gets reset to 3;
- base = 3; max = 12; step = 15; gems = 42;    => actionCooldown gets reset to 5;
- base = 3; max = 12; step = 15; gems = 99999; => actionCooldown gets reset to 12;
- base = 0; max = 1;  step = 0;  gems = 99999; => actionCooldown gets reset to 1; unit can immediately act again next tick.
```

## Core Spawn Cooldown

The core spawn cooldown is more straightforward, after a new unit was spawned, it is simply reset to a static config-defined value.
