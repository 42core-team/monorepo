---
title: "Unit Builder"
permalink: "unit_builder"
sidebarTitle: "Unit Builder"
---

Units are built from components. Each component affect multiple properties. A property is one stat that defines the unit. You cannot choose properties directly, you can only choose components. To make this easy, we made the **Unit Builder**. Find it under your event on the CORE website. Name your unit, then drag the components of the unit into the unit assembly. You can see the properties that your chosen components will result in on the right side. Components can be stacked and their values are added together.

After that, scroll to the bottom, and copy the unit creation line into your code, which will look something like this:

```c
core_action_createUnit("Warrior", "combat", "health", NULL);
```

The first argument is the unit's name. Pass `NULL` to receive a generated name. Every following argument is a component ID from the builder, and the list **must end with `NULL`**. See the [C unit creation action](reference/c/actions/core_action_createUnit).

The Go equivalent uses a variadic component list and therefore needs no sentinel:

```go
bot.CreateUnit("Warrior", "combat", "health")
```

Pass an empty string as the name to receive a generated name. See [`Bot.CreateUnit`](reference/go/actions/CreateUnit).

After the unit spawns, its component IDs are in `unit->s_unit.components` and its finished properties are in `unit->s_unit.properties`. Its current health, carried gems, and cooldown are in `unit->hp`, `unit->s_unit.gems`, and `unit->s_unit.action_cooldown`.

In Go, read the same state from `unit.GetUnitData().Components`, `unit.GetUnitData().Properties`, `unit.Hp`, `unit.GetUnitData().Gems`, and `unit.GetUnitData().ActionCooldown`.

## Properties

The following properties exist and are modified by components you choose. Balance them wisely!

### `hp`

The unit's health when it spawns. `unit->hp` is the health it has left after taking damage.

A unit with 10 HP survives 9 unit damage. It is destroyed when its health reaches 0 or less.

Component changes are added together. If the default is 5 HP and two components each add 5, the unit spawns with 15 HP. The builder tells you if the final health breaks an event rule.

## `baseActionCooldown`

The base number of ticks a unit waits between actions - lower values therefore make a unit faster. This is effectively the speed property, but counterintuitively the higher the number the lower the speed.

Moving, attacking, and transferring gems reset `action_cooldown`. A positive cooldown means the unit must wait; at `0`
or below, it can queue an action. A successful action resets the cooldown to the base action cooldown plus a carried-gem
penalty defined by `gemsPerCooldownStep`:

<pre>
action cooldown = max(1, max(0, baseActionCooldown)
                         + floor(carried gems / max(1, gemsPerCooldownStep)))
</pre>

If a ready unit does not act, its cooldown continues into the negative range. Once it reaches the configured maximum standing cooldown, the unit forfeits that stored readiness and its cooldown resets as if it had acted.

## `gemsPerCooldownStep`

How many carried gems add one tick to the unit's action cooldown. Only complete groups count because integer division is used.

With `baseActionCooldown = 3` and `gemsPerCooldownStep = 15`:

<pre>
 0 gems -> cooldown 3
14 gems -> cooldown 3
15 gems -> cooldown 4
42 gems -> cooldown 5
</pre>

A larger value lets a loaded unit stay faster; a smaller value makes carrying gems slow it down sooner.

## `maxGems`

The most gems a unit can carry at once. If collecting a pile or receiving a transfer would exceed this limit, the unit is filled only to this limit.

## `damageReductionPercent`

The percentage removed from damage when this unit is attacked.

Reduced damage is rounded to the nearest whole number. A positive attack always deals at least 1 damage.

For example, 30% damage reduction turns 10 damage into 7. Even 100% reduction leaves 1 damage from a positive attack, so it cannot make a unit impossible to kill.

## `damageCore`

Damage dealt when this unit attacks an enemy core. It has no effect on attacks against units, deposits, or walls. Any unit, no matter the component assembly, is able to attack gem piles.

## `damageUnit`

Damage dealt when this unit attacks another unit. (Friendly fire works!). The target's `damageReductionPercent` is applied after this value is selected. It has no effect on attacks against cores, deposits, or walls. Any unit, no matter the component assembly, is able to attack gem piles.

## `damageObject`

Damage dealt when this unit attacks deposits and walls. It has no effect on attacks against cores and units. Any unit, no matter the component assembly, is able to attack gem piles.

## `postSpawnCoreCooldown`

The number of ticks the core must wait after creating this unit before it can create another unit. After a unit is spawned successfully, that unit's `postSpawnCoreCooldown` becomes `core->s_core.spawn_cooldown`, which then counts down once per tick. Once the cores spawn cooldown reached 0, it can spawn another unit.
