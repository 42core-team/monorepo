---
title: "🧩 Unit Builder"
permalink: "unit_builder"
sidebarTitle: "🧩 Unit Builder"
---

Units are built from components. There is no fixed list of unit types. Open the **Unit Builder** for your event on the CORE website. It shows the available components, checks your design, calculates the unit's properties and cost, and gives you the component list for `core_action_createUnit`.

```c
core_action_createUnit("Warrior", "combat", "health", NULL);
```

The first argument is the unit's name. Pass `NULL` to receive a generated name. Every following argument is a component ID from the builder, and the list **must end with `NULL`**. Components may be repeated when the builder allows it.

A unit starts with the event's default properties and base cost. Each component changes those properties and adds to the cost:

<pre>
final property = default property + sum of component modifications
unit cost      = default unit cost + sum of component costs
</pre>

No unit is created when:

- it has too many components;
- a component ID does not exist;
- the design breaks one of the event's rules;
- the core does not have enough gems; or
- the core's spawn cooldown is still active.

These rules can change between events. Use your event's Unit Builder instead of copying another event's design.

After the unit spawns, its component IDs are in `unit->s_unit.components` and its finished properties are in `unit->s_unit.properties`. Its current health, carried gems, and cooldown are in `unit->hp`, `unit->s_unit.gems`, and `unit->s_unit.action_cooldown`.

## `hp`

The unit's health when it spawns. This is the value shown as the unit's full health bar; `unit->hp` is the health it has left after taking damage.

Health does not reduce damage or make attacks stronger. It only decides how much damage the unit can survive. A unit with 10 HP survives 9 damage. It is destroyed when its health reaches 0 or less.

Component changes are added together. If the default is 5 HP and two components each add 5, the unit spawns with 15 HP. The builder tells you if the final health breaks an event rule.

## `baseActionCooldown`

The base number of ticks a unit waits between actions. Moving, attacking, and transferring gems reset `action_cooldown`. A new unit also starts with this cooldown. The unit can act when `unit->s_unit.action_cooldown <= 0`.

The reset value also includes the carried-gem penalty described by `balancePerCooldownStep`:

<pre>
action cooldown = max(1, max(0, baseActionCooldown)
                         + floor(carried gems / max(1, balancePerCooldownStep)))
</pre>

The cooldown decreases by 1 every tick. If a ready unit does nothing, the value continues below zero. After it reaches the event's standing-cooldown limit, it is reset even though the unit did not act.

A negative cooldown only means "ready." It does not save time for the next action. When the unit acts, the old value is replaced with a new cooldown.

Lower values therefore make a unit act more frequently. The final cooldown is always at least 1, so even a zero or negative `baseActionCooldown` cannot make a unit perform two rounds of actions without a tick between them.

## `balancePerCooldownStep`

How many carried gems add one tick to the unit's action cooldown. Only complete groups count because integer division is used.

With `baseActionCooldown = 3` and `balancePerCooldownStep = 15`:

<pre>
 0 gems -> cooldown 3
14 gems -> cooldown 3
15 gems -> cooldown 4
42 gems -> cooldown 5
</pre>

A larger value lets a loaded unit stay faster; a smaller value makes carrying gems slow it down sooner. A final value of 0 or less is treated as 1 when the cooldown is calculated, avoiding division by zero but making every carried gem add one cooldown tick.

The gem penalty is checked while the cooldown counts down. If the unit gets rid of gems while waiting, its remaining cooldown can become shorter. Gems received during a cooldown do not make that cooldown longer. The larger penalty starts after the unit's next action.

## `maxBalance`

The most gems a unit can collect when it attacks a gem pile. If the whole pile would put the unit over this value, the unit is filled only to this limit. The unused part of the pile is not left on the ground.

This limit applies when collecting gem piles. A direct gem transfer into the unit is not limited by `maxBalance`.

Capacity and carrying speed are separate concerns: `maxBalance` controls how much the unit can collect, while `balancePerCooldownStep` controls how strongly its carried amount slows actions. A high-capacity unit without a suitable cooldown design may carry more gems but take much longer between moves.

`unit->s_unit.gems` contains the current amount, not this limit. Read the limit from `unit->s_unit.properties.max_balance` when your strategy needs to know how much free capacity remains.

## `damageReductionPercent`

The percentage removed from damage when this unit is attacked. It protects only the unit that owns the property; it does not protect the core or other friendly objects.

Only values from 0 to 100 are used. Reduced damage is rounded to the nearest whole number. A positive attack always deals at least 1 damage; an attack with 0 base damage stays at 0.

For example, 30% damage reduction turns 10 damage into 7. Even 100% reduction leaves 1 damage from a positive attack, so it cannot make a unit impossible to kill.

## `damageCore`

Damage dealt when this unit attacks an enemy core. It has no effect on attacks against units, deposits, walls, or gem piles.

This value is separate from the other damage properties. A unit can be good at destroying cores but bad at clearing the path to one. A value of 0 deals no damage, but the attack still uses the unit's action and resets its cooldown.

## `damageUnit`

Base damage dealt when this unit attacks another unit. The target's `damageReductionPercent` is applied after this value is selected.

For example, `damageUnit = 8` against a unit with 25% damage reduction deals 6 damage. This property does not improve attacks against cores, deposits, or walls. A strong hit can still be slow, so compare this value with the unit's action cooldown.

## `damageObject`

Damage dealt to deposits and walls. This is the mining and obstacle-clearing property; it does not add damage against units or cores.

Attacking a deposit reduces its health by this value. When the deposit reaches 0 HP, it becomes a gem pile containing the deposit's gems. Attacking that gem pile collects it, subject to the unit's `maxBalance`; the gem-pile interaction itself does not use `damageObject`.

A value of 0 cannot damage a deposit or wall, but the attempted attack still resets the action cooldown.

## `postSpawnCoreCooldown`

The number of ticks the creating core must wait before it can create another unit. After a unit is spawned successfully, that unit's final `postSpawnCoreCooldown` becomes `core->s_core.spawn_cooldown`, which then counts down once per tick.

The value comes from the new unit, but the countdown is stored on the core. It does not change how fast the unit acts. `baseActionCooldown` and `balancePerCooldownStep` control that.

Check `core->s_core.spawn_cooldown == 0` before requesting another unit when you want to avoid predictable spawn errors. The builder shows the final cooldown for the design you are about to create.
