In this short 5-minute guide, we'll go through your first bot that performs a multitude of actions. If you've already written a Core bot before, you probably don't need to read this again.

---

## Before we start

If you haven't created a team & cloned your repo yet, look at & follow the [Getting Started Guide](../README) first.

Before we get started writing code, please run `make` once, and then have a look at the game that was played in the visualizer. For more details on how to do those things, look at your bots README.
- Have a look at all the different objects in the game and how they differ. Hover with your mouse and look at the tooltip to see what kind of data is associated with each object.
- Can you see the differently colored bars behind the objects? Do you understand what they mean?
- Can you figure out why some units move faster than others? When do units move?
- Watch the game run through entirely once & enjoy seeing your team be killed by the noob default opponent. 😂

The program we will be writing will not avoid action failures. You'll have to figure out how to avoid those yourself.

---

## The default program

Look into the `my-core-bot/src` directory - in there you will find the files you'll be working in. We can ignore all the files but the `main.c` file for now.

In the `main.c` file you should currently see something roughly like this:

```c
#include "bot.h"

void ft_on_tick(unsigned long tick);

int main(int argc, char **argv)
{
	return core_startGame("YOUR TEAM NAME HERE", argc, argv, ft_on_tick, false);
}

void ft_on_tick(unsigned long tick)
{
	printf("-----> [⚡️ TICK %ld 🔥]\n", tick);
}
```

If you have some more basic code in there, we will now be writing that again in this guide.

- `#include "bot.h"` includes some util functions and the Core Library, which we will use to interface with the game.
- The `main` function starts up the Core Library Gameloop. There's no need for you to touch it.
- The `ft_on_tick` function prints out the current tick. It is called every time a [tick](https://www.reddit.com/r/explainlikeimfive/comments/4mn531/eli5_what_is_atick_in_gaming_development/) has passed. This function will be called multiple times a second while the game is running.

---

## Spawning the first unit

Let's create a unit!

```c
void ft_on_tick(unsigned long tick)
{
	printf("-----> [⚡️ TICK %ld 🔥]\n", tick);

	core_action_createUnit(UNIT_WARRIOR);
}
```

The [createUnit action](../reference/actions/core_action_createUnit) takes the [type of unit](../reference/objects/e_unit_type) you wish to create as an input. It then asks the server to create the unit. If all is right, then there will be a new unit spawned the next tick / the next time the `ft_on_tick` function gets executed.

If your core (your central base) does not have enough gems however, no unit will be created. Later, we will collect more gems to be able to spawn more units. For now, this warrior is enough.

---

## Pathfinding to the opponents core

Let's move the unit to the opponent core and perform an attack action!

To do this, we'll first need a reference to our warrior. The game needs to know which specific unit should perform a certain action, not just that a unit that is a warrior should do a certain thing.

To do this, let's have the Core library make us an array of all of our units that we can then loop over. Let's add this new util:

```c
bool ft_is_own_unit(const t_obj *obj)
{
	if (obj->type != OBJ_UNIT) return false;
	if (obj->s_unit.team_id != ft_get_core_own()->s_core.team_id) return false;
	return true;
}
```

This is a getter filtering function - it exactly matches the function signature required to be used with the Core libraries getter filtering functionality. For more info on that see [the getter filtering documentation](getter_filtering).

With it, we can now do the following to easily have an array of all our warriors created for us:

```c
void ft_on_tick(unsigned long tick)
{
	printf("-----> [⚡️ TICK %ld 🔥]\n", tick);

	core_action_createUnit(UNIT_WARRIOR);

	t_obj **my_units = core_get_objs_filter(ft_is_own_unit);
	// ... do something
	free(my_units);
}
```

The library created an array of all of our units for us. We only need to remember to free it after using it!

Now, let's loop through our units and find out warrior:

```c
for (size_t i = 0; my_units && my_units[i] != NULL; i++)
{
	t_obj *unit = my_units[i];
	
	if (unit->s_unit.unit_type == UNIT_WARRIOR)
	{
		// we found our warrior
	}
}
```

Remember to check that my_units is initialized - otherwise you'll segfault if you don't have any units and the [`core_get_objs_filter`](../reference/getters/core_get_objs_filter) function returns NULL.

With our warrior found, we can now have it pathfind to the opponents core:

```c
if (unit->s_unit.unit_type == UNIT_WARRIOR)
{
	t_obj *opponent_core = ft_get_core_opponent();

	core_action_pathfind(unit, opponent_core->pos);
}
```

`ft_get_core_opponent()` returns the core of the opponent. It is not a part of the Core Library, it is a utility function that is already a part of your bot in the `getters.c` file right next to the file we're in right now. It contains a few helpful getters that all utilize [getter filtering](getter_filtering). It may be useful to you.

[`core_action_pathfind`](../reference/actions/core_action_pathfind) is another action similar to [`core_action_createUnit`](../reference/actions/core_action_createUnit), which tries to move a certain object to a certain position. It makes one move each time it's called, so since multiple moves across multiple ticks are needed to move an object to another location, keep calling the pathfind function until it's reached its target, as we do here.\
If there is an object in the way (that isn't one of your teams units or your core), it will attack that object to get to its target. So if we target an object that's not our unit our core, it will actually also attack it, making the function perfect for both moving and attacking.\
Note that this function is only a temporary util - you'll probably want a custom function that's better at navigating the game field at some point. Once you do, you can use [`core_action_move`](../reference/actions/core_action_move) and [`core_action_attack`](../reference/actions/core_action_attack) to move and attack units manually.

If you run the game again now, you should see your bot spawn a warrior and then see that warrior walk towards the opponent core.
The problem is that the gridmaster opponent does the exact same - so it's entirely random which one of you wins. So let's improve our logic and gain a leg up on our opponent!

---

## Attacking opponent units directly

The way we will win against our opponent is by not running past all of it's units - instead, we shall attack them before going for the core. Since the opponent clearly only targets our core directly, when we hit the opponents enemies, they won't hit back. Let's try it!

First of all, let's create another getter filtering util:

```c
bool ft_is_enemy_unit(const t_obj *obj)
{
	if (obj->type != OBJ_UNIT) return false;
	if (obj->s_unit.team_id == ft_get_core_own()->s_core.team_id) return false;
	return true;
}
```

And then let's use it to find the nearest enemy unit and attack it! To do this, we'll use another Core library function, [`core_get_obj_filter_nearest`](../reference/getters/core_get_obj_filter_nearest):

```c
t_obj *nearest_enemy = core_get_obj_filter_nearest(unit->pos, ft_is_enemy_unit);
```

This function is similar to [`core_get_objs_filter`](../reference/getters/core_get_objs_filter), but instead of returning multiple objects in an array, it puts together the same list, then only returns the closest object to an inputted position. Perfect for our goals!

Let's put it all together & then run:

```c
bool ft_is_own_unit(const t_obj *obj)
{
	if (obj->type != OBJ_UNIT) return false;
	if (obj->s_unit.team_id != ft_get_core_own()->s_core.team_id) return false;
	return true;
}

bool ft_is_enemy_unit(const t_obj *obj)
{
	if (obj->type != OBJ_UNIT) return false;
	if (obj->s_unit.team_id == ft_get_core_own()->s_core.team_id) return false;
	return true;
}

void ft_on_tick(unsigned long tick)
{
	printf("-----> [⚡️ TICK %ld 🔥]\n", tick);

	core_action_createUnit(UNIT_WARRIOR);

	t_obj **my_units = core_get_objs_filter(ft_is_own_unit);
	
	for (size_t i = 0; my_units && my_units[i] != NULL; i++)
	{
		t_obj *unit = my_units[i];
		
		if (unit->s_unit.unit_type == UNIT_WARRIOR)
		{
			t_obj *opponent_core = ft_get_core_opponent();
			t_obj *nearest_enemy = core_get_obj_filter_nearest(unit->pos, ft_is_enemy_unit);

			if (nearest_enemy)
				core_action_pathfind(unit, nearest_enemy->pos);
			else if (opponent_core)
				core_action_pathfind(unit, opponent_core->pos);
		}
	}

	free(my_units);
}
```

AWESOME! (You should have just won.)

---

## A second unit & mining gems

Finally, let's go a step further!

To defeat more advanced opponents, we will need more money that the idle income we get simply by waiting over time - we need to actually pick up the gems you have probably seen lying around ourselves.

You may have noticed there are these deposit objects around. They are gems encased in stone, and they are the secret to earning tons of gems and overrunning your opponent with units.

Let's mine a few!

Firstly, let's spawn the miner unit. As you can see in the [config](../configs), it does more damage to deposits in comparison to warriors. Using miners to mine deposits will therefore be a lot quicker.

First, we need to spawn miner units, so lets modify the unit spawning logic at the beginning:

```c
bool ft_is_miner(const t_obj *obj)
{
	if (obj->type != OBJ_UNIT) return false;
	if (obj->s_unit.unit_type != UNIT_MINER) return false;
	return true;
}
```

```c
void ft_on_tick(unsigned long tick)
{
	printf("-----> [⚡️ TICK %ld 🔥]\n", tick);

	if (core_get_objs_filter_count(ft_is_miner) < 2)
		core_action_createUnit(UNIT_MINER);
	core_action_createUnit(UNIT_WARRIOR);
// ...
```

This will guarantee that there is always at least 2 miners, and start spawning warriors afterwards. [`core_get_objs_filter_count`](../reference/getters/core_get_objs_filter_count) is similar to previous getter functions, but it simply returns an integer - the amount of units in the game that match the custom filtering condition.

Secondly, we need a new getter filtering util:

```c
bool ft_is_deposit(const t_obj *obj)
{
	return (obj->type == OBJ_DEPOSIT);
}
```

Thirdly, lets have each of our miners attack the nearest deposits that are closest to them to mine them:

```c
for (size_t i = 0; my_units && my_units[i] != NULL; i++)
{
	t_obj *unit = my_units[i];
	
	if (unit->s_unit.unit_type == UNIT_WARRIOR)
	{
		// ...
	}

	else if (unit->s_unit.unit_type == UNIT_MINER)
	{
		t_obj *nearest_deposit = core_get_obj_filter_nearest(unit->pos, ft_is_deposit);
		if (nearest_deposit)
			core_action_pathfind(unit, nearest_deposit->pos);
	}
}
```

Run the program!
As you may have noticed, things aren't working quite as expected - the deposits are being mined, but our core is not gaining any gems. The gems just fall on the floor and our miner is not picking them up. Let's fix that!

Let's remove the `ft_is_deposit` util, and instead use this:

```c
bool ft_is_deposit_or_gem_pile(const t_obj *obj)
{
	return (obj->type == OBJ_DEPOSIT || obj->type == OBJ_GEM_PILE);
}
```

If we now don't have our miners target the nearest deposit, but the nearest deposit or gem pile, they will attack the gem pile after it has dropped from the destroyed deposit. The gems will therefore be picked up by the miner.

```c
else if (unit->s_unit.unit_type == UNIT_MINER)
{
	t_obj *nearest_deposit_or_gem_pile = core_get_obj_filter_nearest(unit->pos, ft_is_deposit_or_gem_pile);
	if (nearest_deposit_or_gem_pile)
		core_action_pathfind(unit, nearest_deposit_or_gem_pile->pos);
}
```

Great! Now the gems are being mined & picked up correctly. Trouble is that they are being picked up by the miner unit, not the core. We just need to bring them to our core now, so the core can can use them to spawn more units!

To do this, we can use yet another action, [`core_action_transferGems`](../reference/actions/core_action_transferGems).

The logic itself isn't too tricky - we can just check whether the unit is holding any gems. If so, we should walk back to our core and transfer the gems to the core, and if not, we should go get some more by mining deposits.

```c
else if (unit->s_unit.unit_type == UNIT_MINER)
{
	if (unit->s_unit.gems > 0)
	{
		t_obj *own_core = ft_get_core_own();
		core_action_pathfind(unit, own_core->pos);
		core_action_transferGems(unit, own_core->pos, unit->s_unit.gems);
	}
	else
	{
		t_obj *nearest_deposit_or_gem_pile = core_get_obj_filter_nearest(unit->pos, ft_is_deposit_or_gem_pile);
		if (nearest_deposit_or_gem_pile)
			core_action_pathfind(unit, nearest_deposit_or_gem_pile->pos);
	}
}
```

Awesome! It works!

I hope you enjoyed this tutorial.

---

## Final code result

Here's the entire code block we ended up with:

```c
#include "bot.h"

void ft_on_tick(unsigned long tick);

int main(int argc, char **argv)
{
	return core_startGame("YOUR TEAM NAME HERE", argc, argv, ft_on_tick, false);
}

// Getter filtering utils
bool ft_is_own_unit(const t_obj *obj)
{
	if (obj->type != OBJ_UNIT) return false;
	if (obj->s_unit.team_id != ft_get_core_own()->s_core.team_id) return false;
	return true;
}
bool ft_is_enemy_unit(const t_obj *obj)
{
	if (obj->type != OBJ_UNIT) return false;
	if (obj->s_unit.team_id == ft_get_core_own()->s_core.team_id) return false;
	return true;
}
bool ft_is_miner(const t_obj *obj)
{
	if (obj->type != OBJ_UNIT) return false;
	if (obj->s_unit.unit_type != UNIT_MINER) return false;
	return true;
}
bool ft_is_deposit_or_gem_pile(const t_obj *obj)
{
	return (obj->type == OBJ_DEPOSIT || obj->type == OBJ_GEM_PILE);
}

// This function gets called once every game tick
void ft_on_tick(unsigned long tick)
{
	printf("-----> [⚡️ TICK %ld 🔥]\n", tick);

	// Spawn new units
	if (core_get_objs_filter_count(ft_is_miner) < 2)
		core_action_createUnit(UNIT_MINER);
	core_action_createUnit(UNIT_WARRIOR);

	// Move existing units
	t_obj **my_units = core_get_objs_filter(ft_is_own_unit);
	for (size_t i = 0; my_units && my_units[i] != NULL; i++)
	{
		t_obj *unit = my_units[i];
		
		if (unit->s_unit.unit_type == UNIT_WARRIOR)
		{
			t_obj *opponent_core = ft_get_core_opponent();
			t_obj *nearest_enemy = core_get_obj_filter_nearest(unit->pos, ft_is_enemy_unit);

			if (nearest_enemy)
				core_action_pathfind(unit, nearest_enemy->pos);
			else if (opponent_core)
				core_action_pathfind(unit, opponent_core->pos);
		}

		else if (unit->s_unit.unit_type == UNIT_MINER)
		{
			if (unit->s_unit.gems > 0)
			{
				t_obj *own_core = ft_get_core_own();
				core_action_pathfind(unit, own_core->pos);
				core_action_transferGems(unit, own_core->pos, unit->s_unit.gems);
			}
			else
			{
				t_obj *nearest_deposit_or_gem_pile = core_get_obj_filter_nearest(unit->pos, ft_is_deposit_or_gem_pile);
				if (nearest_deposit_or_gem_pile)
					core_action_pathfind(unit, nearest_deposit_or_gem_pile->pos);
			}
		}
	}

	free(my_units);
}
```

---

## What next?

- We fully missed out on the most powerful unit: The carrier. And there may be even more units in your config depending on the event you are in! Check them out to not miss out on powerful strategic possibilities.
- How should we handle opponents that try to attack our units? Maybe we can figure out a way to set up a fleeing logic.
- What if an opponent warrior has reached our core, but all our warriors are near their core - can we keep a few warriors always close to our core as a defensive measure?
