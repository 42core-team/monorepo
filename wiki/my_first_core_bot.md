In this guide (30-minute-ish, please take your time), we'll go through your first bot that performs a multitude of actions. If you've already written a Core bot before, you probably don't need to read this again.

---

## Before we start

If you haven't created a team & cloned your repo yet, look at & follow the [Getting Started Guide](README) first.

Before we get started writing code, please run `make` once to run the game, and then have a look at the game that was played in the visualizer. For more details on how to do those things, look at your bots README.
- Watch the game run through entirely once & enjoy seeing your team be killed by the noob default opponent. 😂
- Have a look at all the different objects in the game and how they differ. Hover with your mouse and look at the tooltip to see what kind of data is associated with each object.
- Can you see the differently colored bars behind the objects? Do you understand what they mean?
- Can you figure out why some units move faster than others? When do units move?
- Open up the [game config](documentation/configs) and look at the values for the units that are in the game. Can you see how the config values influence what happens when a unit performs an action?
- You may also want to familiarize yourself with the hotkeys that the visualizer supports, which will be very handy later on.

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

Before you continue, insert your name into the `core_startGame` function call. Replace "YOUR TEAM NAME HERE" with a name for your team. (Note: The name you set will only be shown locally, in website games your team will have the name that you set for your team there.)\
If you rerun the game and check out the visualizer, your core should now have the name you just set in a box next to it.

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

The [createUnit action](reference/actions/core_action_createUnit) takes the [type of unit](reference/objects/e_unit_type) you wish to create as an input. It then asks the server to create the unit. If all is right, then there will be a new unit spawned the next tick / the next time the `ft_on_tick` function gets executed. The unit does not spawn immediately in the same tick.

If your core (your central base) does not have enough gems however, no unit will be created. Later, we will collect more gems to be able to spawn more units. For now, this warrior is enough. Because your core starts with a few hundred gems, we are able to afford it.

To see how many gems your core holds, you can hover your mouse over it in the visualizer. To do this in code, you can get a reference to your core (you will learn how to do this soon), then check the gems property like this:

```c
t_obj *my_core = /**/;
printf("My core has %lu gems!\n", my_core->s_core.gems);
```

You can find out how many gems a unit costs to create by looking at the game config. Here's how you can do this in the code:

```c
printf("My warrior costs %lu gems!\n", core_get_unitConfig(UNIT_WARRIOR)->cost);
```

Currently there is an action error (almost) every tick because we try to create a new warrior every tick but don't have enough gems in our core. Knowing how to check for the amount of gems the core holds and how much money creating a warrior costs, can you find a way to stop the action error from occurring?

Doing this fix is up to you and won't be included in code blocks as this tutorial continues.

---

## Pathfinding to the opponents core

Let's move the unit to the opponent core and perform an attack action!

To do this, we'll first need a reference to our warrior. The game needs to know which specific unit should perform a certain action, not just that a unit that is a warrior should do a certain thing.

All objects are represented in the library as the `t_obj` struct. A `t_obj` could therefore represent your own or your opponents units, but also your core, a wall, a deposit or even a bomb. By using cmd+click in VSCode, you should be able to navigate to the `bot.h` file and from there to the `core_lib.h` file. Here you can see every struct and function provided to you by the core library, where you should also be able to find the `t_obj` struct. You don't every have to allocate a `t_obj` yourself, as the library already created every object currently in the game for you, you just need to look for it.

To get the `t_obj` struct of the warrior we created, let's have the Core library make us an array of all of our units that we can then loop over. Let's add this new utility function:

```c
bool ft_is_own_unit(const t_obj *obj)
{
	if (obj->type != OBJ_UNIT) return false;
	if (obj->s_unit.team_id != ft_get_core_own()->s_core.team_id) return false;
	return true;
}
```

This is a getter filtering function - it exactly matches the function signature required to be used with the Core libraries getter filtering functionality. For more info on that see the [getter filtering documentation](documentation/getter_filtering). The function takes a `t_obj *` as input, then returns false if the object isn't a unit, and if the object is not on our team, otherwise it returns true.

With it, we can now do the following to easily have an array of all our warriors created for us:

```c
void ft_on_tick(unsigned long tick)
{
	printf("-----> [⚡️ TICK %ld 🔥]\n", tick);

	core_action_createUnit(UNIT_WARRIOR);

	t_obj **my_units = core_get_objs_filter(ft_is_own_unit);
	// ... do something with the array of all my units
	free(my_units);
}
```

The library created an array of all of our units for us. We only need to remember to free it after using it! The getter filtering functions can be used to filter for any single or array of objects you may want to.

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

Remember to check that my_units is initialized - otherwise you'll segfault if you don't have any units and the [`core_get_objs_filter`](reference/getters/core_get_objs_filter) function returns NULL.

With our warrior found, we can now have it pathfind to the opponents core:

```c
if (unit->s_unit.unit_type == UNIT_WARRIOR)
{
	// we found our warrior

	t_obj *opponent_core = ft_get_core_opponent();

	core_action_pathfind(unit, opponent_core->pos);
}
```

`ft_get_core_opponent()` returns the core of the opponent. It is not a part of the Core Library, it is a utility function that is already a part of your bot in the `getters.c` file right next to the file we're in right now. It contains a few helpful getters that all utilize [getter filtering](documentation/getter_filtering). It may be useful to you. Another function in this file is `ft_get_core_own()`, which does exactly what it says tin. In fact, now that you understand how getter filtering works, if you wanted to, you could remove the previous `ft_is_own_unit` utility function entirely and use `t_obj **ft_get_units_own(void)` from the `getters.c` file to get all your units instead.

[`core_action_pathfind`](reference/actions/core_action_pathfind) is another action similar to [`core_action_createUnit`](reference/actions/core_action_createUnit), which tries to move a certain object to a certain position. It makes one move each time it's called, so since multiple moves across multiple ticks are needed to move an object to another location, keep calling the pathfind function each tick until it's reached its target, as we do here.\
If there is an object in the way (that isn't one of your teams units or your core), it will attack that object to get to its target. So if we target the position of an object that's not our unit our core, it will actually also attack it, making the function perfect for both moving and attacking.\
Note that this function is only a temporary util - you'll probably want a custom function that's better at navigating the game field at some point. Once you do, you can use [`core_action_move`](reference/actions/core_action_move) and [`core_action_attack`](reference/actions/core_action_attack) to move and attack units manually.

If you run the game again now, you should see your bot spawn a warrior and then see that warrior walk towards the opponent core.
The problem is that the gridmaster opponent does the exact same - so it's entirely random which one of you wins. So let's improve our logic and gain a leg up on our opponent!

---

## Attacking opponent units directly

The way we will win against our opponent is by not running past all of it's units - instead, out units will first attack the units of the opponents and only then target the opponent core. Since the gridmaster opponent clearly only targets our core directly, when we hit the opponents enemies, they won't hit back. Let's try it!

First of all, let's create another getter filtering util from the `getters.c` file, `ft_get_units_opponent_nearest`. It uses a new getter filtering function, [`core_get_obj_filter_nearest`](reference/getters/core_get_obj_filter_nearest), which takes a position argument to return the closest object to the inputted position that matches the inserted condition function.

You should open the `getter.c` file and understand how the `ft_get_units_opponent_nearest` function is implemented and how it works.

So, for each of our active warriors, let's use the `ft_get_units_opponent_nearest` function to get the closest opponent unit. If the function returns null (meaning there is no closest opponent unit, they're all dead), we will head for the opponent core, otherwise we'll make our way to killing that opponent unit.

Let's put it all together & then run:

```c
void ft_on_tick(unsigned long tick)
{
	printf("-----> [⚡️ TICK %ld 🔥]\n", tick);

	core_action_createUnit(UNIT_WARRIOR);

	t_obj **my_units = ft_get_units_own();
	
	for (size_t i = 0; my_units && my_units[i] != NULL; i++)
	{
		t_obj *unit = my_units[i];
		
		if (unit->s_unit.unit_type == UNIT_WARRIOR)
		{
			t_obj *opponent_core = ft_get_core_opponent();
			t_obj *nearest_enemy = ft_get_units_opponent_nearest(unit->pos);

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

To defeat more advanced opponents, we will need more gems than the idle income we get simply by waiting over time - we need to actually pick up the gems you have probably seen lying around in the form of gem piles yourself.

You may also have noticed there are deposit objects around. They are huge amounts of gems encased in stone, and they are the secret to earning tons of gems and overrunning your opponent with units.

Let's mine a few!

Firstly, let's spawn the miner unit. As you can see in the [config](documentation/configs), it does more damage to deposits in comparison to warriors. Using miners to mine deposits will therefore be a lot quicker.

First, we need to spawn miner units, so lets modify the unit spawning logic at the beginning:

```c
// getter filtering utility function
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

This will guarantee that there is always at least 2 miners, and we will start spawning warriors afterwards. [`core_get_objs_filter_count`](reference/getters/core_get_objs_filter_count) is similar to previous getter functions, but it simply returns an integer - the amount of objects in the game that match the custom filtering condition.

Thirdly, lets have each of our miners attack the nearest deposits that are closest to them to mine them. To do this, we can use the helpfully provided `ft_get_deposit_nearest` function from `getters.c`.

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
		t_obj *nearest_deposit = ft_get_deposit_nearest(unit->pos);
		if (nearest_deposit)
			core_action_pathfind(unit, nearest_deposit->pos);
	}
}
```

Run the program!
As you may have noticed, things aren't working quite as expected - the deposits are being mined, but our core is not gaining any gems. The gems just fall on the floor and our miner is not picking them up. The reason for this is that once a deposit was fully mined, it turns into a gem pile object with the same amount of gems. But since our miners aren't targeting gem piles, they just walk along.

If we don't have our miners target the nearest deposit, but the nearest deposit or gem pile, they will attack the gem pile after it has dropped from the destroyed deposit. The gems will therefore be picked up by the miner. We can do this using the `ft_get_deposit_gems_nearest` function also provided in `getters.c`.

```c
else if (unit->s_unit.unit_type == UNIT_MINER)
{
	t_obj *nearest_deposit_or_gem_pile = ft_get_deposit_gems_nearest(unit->pos);
	if (nearest_deposit_or_gem_pile)
		core_action_pathfind(unit, nearest_deposit_or_gem_pile->pos);
}
```

Great! Now the gems are being mined & picked up correctly. Trouble is that they are being picked up by the miner unit, not the core. We just need to bring them to our core now, so the core can can use them to spawn more units!

To do this, we can use yet another action, [`core_action_transferGems`](reference/actions/core_action_transferGems).

The logic itself isn't too tricky - we can just check whether the unit is holding any gems, very similar to how we previously checked whether our core had any gems. If so, we should walk back to our core and transfer the gems to the core, and if not, we should go get some more by mining deposits or picking up normal gem piles.

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
		t_obj *nearest_deposit_or_gem_pile = ft_get_deposit_gems_nearest(unit->pos);
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
	return core_startGame("testiebestie", argc, argv, ft_on_tick, false);
}

// getter filtering utility function
bool ft_is_miner(const t_obj *obj)
{
	if (obj->type != OBJ_UNIT) return false;
	if (obj->s_unit.unit_type != UNIT_MINER) return false;
	return true;
}

void ft_on_tick(unsigned long tick)
{
	printf("-----> [⚡️ TICK %ld 🔥]\n", tick);

	if (core_get_objs_filter_count(ft_is_miner) < 2)
		core_action_createUnit(UNIT_MINER);
	core_action_createUnit(UNIT_WARRIOR);

	t_obj **my_units = ft_get_units_own();
	
	for (size_t i = 0; my_units && my_units[i] != NULL; i++)
	{
		t_obj *unit = my_units[i];
		
		if (unit->s_unit.unit_type == UNIT_WARRIOR)
		{
			t_obj *opponent_core = ft_get_core_opponent();
			t_obj *nearest_enemy = ft_get_units_opponent_nearest(unit->pos);

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
				t_obj *nearest_deposit_or_gem_pile = ft_get_deposit_gems_nearest(unit->pos);
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

- We fully missed out on the most powerful unit: The carrier. You may have noticed already that the miners walk a little slower when they are holding gems - that is because gems are quite heavy, so they struggle to carry them. But don't worry, the carrier is well-equipped to help you will all of your quick-carrying needs!
- There may be even more units in your config depending on the event you are in! Check them out to not miss out on powerful strategic possibilities. If you're not sure why you should use them, check out the [configs](documentation/configs) where you can read all the hard facts.
- The code currently has plenty of action failures, almost all of which are easily avoidable. Adding some code to get rid of some of the nasty red error lines will teach you a lot.
- How should we handle opponents that try to attack our units? Maybe we can figure out a way to set up a fleeing logic.
- Using core game's debug features, we can add custom text to the unit tooltips. This can be super useful to know what a unit's current job is in order to avoid confusion.
- What if an opponent warrior has reached our core, but all our warriors are near their core - can we keep a few warriors always close to our core as a defensive measure?

Before you get ahead of yourself, make sure to read the [Basics Guide](basics). Whatever you plan on doing, the wiki will be here to give info on every function, struct or concept you may be wondering about. Good luck!
