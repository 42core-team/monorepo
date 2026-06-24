In this guide (30-minute-ish, please take your time), we'll go through your first bot that performs a multitude of actions. If you've already written a CORE bot before, you probably don't need to read this again.

---

## Before we start

If you haven't created a team & cloned your repo yet, look at & follow the [Getting Started Guide](README) first.

Before we get started writing code, please run `make` once to run the game, and then have a look at the game that was played in the visualizer. For more details on how to do those things, look at your bots README.
- Watch the game from start to finish & enjoy seeing your team be killed by the noob default opponent. 😂
- Have a look at all the different objects in the game and how they differ. Hover with your mouse and look at the tooltip to see what kind of data is associated with each object.
- Can you see the differently colored bars behind the objects? Do you understand what they mean?
- Can you figure out why some units move faster than others? When do units move?
- Open up the [game config](documentation/configs) and look at the values for the units that are in the game. Can you see how the config values influence what happens when a unit performs an action?
- You may also want to familiarize yourself with the hotkeys that the visualizer supports, which will be very handy later on.

The program we will be writing will not avoid action failures. You'll have to figure out how to avoid those yourself.

---

## The default program

Look into your bot's source directory - for C this is `my-CORE-bot/src/main.c`, for Go this is your `main.go` file.

You should currently see something roughly like this:

::: code-group labels=[C, Go]
```c
#include "bot.h"

void ft_on_tick(unsigned long tick);

int main(int argc, char **argv)
{
	return CORE_startGame("YOUR TEAM NAME HERE", argc, argv, ft_on_tick, false);
}

void ft_on_tick(unsigned long tick)
{
	printf("-----> [⚡️ TICK %ld 🔥]\n", tick);
}
```
```go
package main

import (
	"fmt"
	"os"
	"strconv"

	coregame "github.com/42core-team/go-client-lib"
	"github.com/42core-team/go-client-lib/game"
)

func main() {
	teamID, _ := strconv.Atoi(os.Args[1])
	cfg := coregame.DefaultBotConfig(teamID, "YOUR TEAM NAME HERE")
	bot, err := coregame.NewBot(cfg)
	if err != nil {
		fmt.Println("Error:", err)
		return
	}
	defer bot.Close()

	bot.Run(func(g *game.Game, b *coregame.Bot) {
		fmt.Printf("-----> [⚡️ TICK %d 🔥]\n", g.ElapsedTicks)
	})
}
```
:::

If you have some more basic code in there, we will now be writing that again in this guide.

- **C**: `#include "bot.h"` includes some util functions and the CORE Library, which we will use to interface with the game. The `main` function starts up the CORE Library Gameloop. The `ft_on_tick` function prints out the current tick.
- **Go**: The `main` function sets up the bot using `NewBot` and starts the game loop with `bot.Run`. The tick callback is passed as an inline function that receives both the game state and the bot instance.

The tick function / callback is called every time a [tick](https://www.reddit.com/r/explainlikeimfive/comments/4mn531/eli5_what_is_atick_in_gaming_development/) has passed. It will be called multiple times a second while the game is running.

Before you continue, insert your name into the startup call. Replace "YOUR TEAM NAME HERE" with a name for your team. (Note: The name you set will only be shown locally, in website games your team will have the name that you set for your team there.)\
If you rerun the game and check out the visualizer, your CORE should now have the name you just set in a box next to it.

---

## Spawning the first unit

Let's create a unit!

::: code-group labels=[C, Go]
```c
void ft_on_tick(unsigned long tick)
{
	printf("-----> [⚡️ TICK %ld 🔥]\n", tick);

	CORE_action_createUnit(UNIT_ACID_WARRIOR);
}
```
```go
bot.Run(func(g *game.Game, b *coregame.Bot) {
	fmt.Printf("-----> [⚡️ TICK %d 🔥]\n", g.ElapsedTicks)

	b.CreateUnit(game.UnitWarrior)
})
```
:::

The createUnit action ([C](reference/c/actions/core_action_createUnit) | [Go](reference/go/actions/CreateUnit)) takes the type of unit ([C](reference/c/objects/e_unit_type) | [Go](reference/go/objects/UnitType)) you wish to create as an input. It then asks the server to create the unit. If all is right, then there will be a new unit spawned the next tick / the next time the tick function gets executed. The unit does not spawn immediately in the same tick.

If your CORE (your central base) does not have enough gems however, no unit will be created. Later, we will collect more gems to be able to spawn more units. For now, this acid warrior is enough.

In your config, there is also a rock warrior, a scissors warrior, and a paper warrior. These, intuitively, work like the classic childrens game [rock paper scissors](https://en.wikipedia.org/wiki/Rock_paper_scissors). For example, the paper warrior does more damage to rock warriors and less damage to scissor warriors. The acid warrior is quite bad in comparison to any of these three even when they're used highly ineffectively. It is only included to help you get started easier when writing your first bot. You should switch to rps warriors at a later point.

Because your CORE starts with a few hundred gems, we are able to afford buying the acid warrior for now.

To see how many gems your CORE holds, you can hover your mouse over it in the visualizer. To do this in code, you can get a reference to your CORE (you will learn how to do this soon), then check the gems property like this:

::: code-group labels=[C, Go]
```c
t_obj *my_CORE = /**/;
printf("My CORE has %lu gems!\n", my_CORE->s_CORE.gems);
```
```go
myCore := g.MyCore()
coreData := myCore.GetCoreData()
fmt.Printf("My CORE has %d gems!\n", coreData.Gems)
```
:::

You can find out how many gems a unit costs to create by looking at the game config. Here's how you can do this in the code:

::: code-group labels=[C, Go]
```c
printf("My warrior costs %lu gems!\n", CORE_get_unitConfig(UNIT_ACID_WARRIOR)->cost);
```
```go
uconf := g.Config.GetUnitConfig(game.UnitWarrior)
fmt.Printf("My warrior costs %d gems!\n", uconf.Cost)
```
:::

Currently there is an action error (almost) every tick because we try to create a new warrior every tick but don't have enough gems in our CORE. Knowing how to check for the amount of gems the CORE holds and how much gems creating a warrior costs, can you find a way to stop the action error from occurring?

Doing this fix is up to you and won't be included in code blocks as this tutorial continues.

---

## Pathfinding to the opponent's CORE

Let's move the unit to the opponent CORE and perform an attack action!

To do this, we'll first need a reference to our warrior. The game needs to know which specific unit should perform a certain action, not just that a unit that is a warrior should do a certain thing.

All objects are represented as the object struct ([C: `t_obj`](reference/c/objects/s_obj) | [Go: `Object`](reference/go/objects/Object)). An object could therefore represent your own or your opponent's units, but also your CORE, a wall, a deposit or even a bomb. You don't ever have to allocate an object yourself, as the library already created every object currently in the game for you, you just need to look for it.

To get the object of the warrior we created, let's have the library make us a list of all of our units that we can then loop over. Let's add a filtering utility:

::: code-group labels=[C, Go]
```c
bool ft_is_own_unit(const t_obj *obj)
{
	if (obj->type != OBJ_UNIT) return false;
	if (obj->s_unit.team_id != ft_get_CORE_own()->s_CORE.team_id) return false;
	return true;
}
```
```go
// In Go, you can use inline predicates or the built-in TeamUnits() helper:
isOwnUnit := func(obj *game.Object) bool {
	if obj.Type != game.ObjectUnit {
		return false
	}
	data := obj.GetUnitData()
	if data == nil {
		return false
	}
	return data.TeamID == g.MyTeamID
}
```
:::

- **C**: This is a getter filtering function - it exactly matches the function signature required to be used with the CORE library's getter filtering functionality. For more info on that see the [getter filtering documentation](documentation/getter_filtering). The function takes a `t_obj *` as input, then returns false if the object isn't a unit, and if the object is not on our team, otherwise it returns true.
- **Go**: Filtering is done by passing predicate functions (closures) directly to getter methods like `ObjectsFilter()`. You can also use built-in helpers like `g.TeamUnits()` which returns all your team's units.

With it, we can now do the following to easily have a list of all our units created for us:

::: code-group labels=[C, Go]
```c
void ft_on_tick(unsigned long tick)
{
	printf("-----> [⚡️ TICK %ld 🔥]\n", tick);

	CORE_action_createUnit(UNIT_ACID_WARRIOR);

	t_obj **my_units = CORE_get_objs_filter(ft_is_own_unit);
	// ... do something with the array of all my units
	free(my_units);
}
```
```go
bot.Run(func(g *game.Game, b *coregame.Bot) {
	fmt.Printf("-----> [⚡️ TICK %d 🔥]\n", g.ElapsedTicks)

	b.CreateUnit(game.UnitWarrior)

	myUnits := g.TeamUnits()
	// ... do something with the slice of all my units
	// No need to free in Go - the garbage collector handles it!
})
```
:::

- **C**: The library created an array of all of our units for us. We only need to remember to free it after using it! The getter filtering functions can be used to filter for any single or array of objects you may want to.
- **Go**: The library created a slice of all of our units for us. Go's garbage collector manages memory automatically, so no need to free anything.

Now, let's loop through our units and find our warrior:

::: code-group labels=[C, Go]
```c
for (size_t i = 0; my_units && my_units[i] != NULL; i++)
{
	t_obj *unit = my_units[i];

	if (unit->s_unit.unit_type == UNIT_ACID_WARRIOR)
	{
		// we found our warrior
	}
}
```
```go
for _, unit := range myUnits {
	data := unit.GetUnitData()
	if data != nil && data.UnitType == game.UnitWarrior {
		// we found our warrior
	}
}
```
:::

- **C**: Remember to check that my_units is initialized - otherwise you'll segfault if you don't have any units and the [`CORE_get_objs_filter`](reference/c/getters/core_get_objs_filter) function returns NULL.
- **Go**: Ranging over a nil or empty slice is safe - no need for extra nil checks.

With our warrior found, we can now have it pathfind to the opponent's CORE:

::: code-group labels=[C, Go]
```c
if (unit->s_unit.unit_type == UNIT_ACID_WARRIOR)
{
	// we found our warrior

	t_obj *opponent_CORE = ft_get_CORE_opponent();

	CORE_action_pathfind(unit, opponent_CORE->pos);
}
```
```go
if data != nil && data.UnitType == game.UnitWarrior {
	// we found our warrior

	enemyCore := g.EnemyCore()

	if enemyCore != nil {
		nextPos := b.SimplePathfind(unit, enemyCore.Pos)
		b.Move(unit, nextPos)
	}
}
```
:::

- **C**: `ft_get_CORE_opponent()` returns the CORE of the opponent. It is not a part of the CORE Library, it is a utility function that is already a part of your bot in the `getters.c` file. Another function in this file is `ft_get_CORE_own()`, which does exactly what it says on the tin. [`CORE_action_pathfind`](reference/c/actions/core_action_pathfind) will determine the next move and execute it, attacking objects in its way.
- **Go**: `g.EnemyCore()` and `g.MyCore()` are built-in helper methods on the Game struct. [`SimplePathfind`](reference/go/actions/SimplePathfind) determines the next move and handles attacking objects in its way, but you need to call `Move` with the returned position yourself.

In fact, now that you understand how getter filtering works, if you wanted to, you could use the built-in helpers (**C**: `ft_get_units_own()` from the `getters.c` file | **Go**: `g.TeamUnits()`) to get all your units instead.

The pathfind function ([C](reference/c/actions/core_action_pathfind) | [Go](reference/go/actions/SimplePathfind)) is another action similar to the createUnit action, which tries to move a certain object to a certain position. It makes one move each time it's called, so since multiple moves across multiple ticks are needed to move an object to another location, keep calling the pathfind function each tick until it's reached its target.\
If there is an object in the way (that isn't one of your teams units or your CORE), it will attack that object to get to its target. So if we target the position of an object that's not our unit or our CORE, it will actually also attack it, making the function perfect for both moving and attacking.\
Note that this function is only a temporary util - you'll probably want a custom function that's better at navigating the game field at some point. Once you do, you can use the move action ([C](reference/c/actions/core_action_move) | [Go](reference/go/actions/Move)) and the attack action ([C](reference/c/actions/core_action_attack) | [Go](reference/go/actions/Attack)) to move and attack units manually.

If you run the game again now, you should see your bot spawn a warrior and then see that warrior walk towards the opponent CORE.
The problem is that the gridmaster opponent does the exact same - so it's entirely random which one of you wins. So let's improve our logic and gain a leg up on our opponent!

---

## Attacking opponent units directly

The way we will win against our opponent is by not running past all of its units - instead, our units will first attack the units of the opponent and only then target the opponent CORE. Since the gridmaster opponent clearly only targets our CORE directly, when we hit the opponent's enemies, they won't hit back. Let's try it!

First of all, let's find the nearest enemy unit. In C, you can use `ft_get_units_opponent_nearest` from the `getters.c` file. In Go, you can use `g.NearestObject` with a predicate. Both use [`NearestObject` / `CORE_get_obj_filter_nearest`](documentation/getter_filtering) which takes a position argument to return the closest object matching a condition.

So, for each of our active warriors, let's find the closest opponent unit. If no opponent units exist (they're all dead), we will head for the opponent CORE, otherwise we'll make our way to killing that opponent unit.

Let's put it all together & then run:

::: code-group labels=[C, Go]
```c
void ft_on_tick(unsigned long tick)
{
	printf("-----> [⚡️ TICK %ld 🔥]\n", tick);

	CORE_action_createUnit(UNIT_ACID_WARRIOR);

	t_obj **my_units = ft_get_units_own();

	for (size_t i = 0; my_units && my_units[i] != NULL; i++)
	{
		t_obj *unit = my_units[i];

		if (unit->s_unit.unit_type == UNIT_ACID_WARRIOR)
		{
			t_obj *opponent_CORE = ft_get_CORE_opponent();
			t_obj *nearest_enemy = ft_get_units_opponent_nearest(unit->pos);

			if (nearest_enemy)
				CORE_action_pathfind(unit, nearest_enemy->pos);
			else if (opponent_CORE)
				CORE_action_pathfind(unit, opponent_CORE->pos);
		}
	}

	free(my_units);
}
```
```go
bot.Run(func(g *game.Game, b *coregame.Bot) {
	fmt.Printf("-----> [⚡️ TICK %d 🔥]\n", g.ElapsedTicks)

	b.CreateUnit(game.UnitWarrior)

	myUnits := g.TeamUnits()

	for _, unit := range myUnits {
		data := unit.GetUnitData()
		if data == nil {
			continue
		}

		if data.UnitType == game.UnitWarrior {
			enemyCore := g.EnemyCore()
			nearestEnemy := g.NearestObject(unit.Pos, func(obj *game.Object) bool {
				d := obj.GetUnitData()
				return obj.Type == game.ObjectUnit && d != nil && d.TeamID != g.MyTeamID && d.TeamID != 0
			})

			if nearestEnemy != nil {
				nextPos := b.SimplePathfind(unit, nearestEnemy.Pos)
				b.Move(unit, nextPos)
			} else if enemyCore != nil {
				nextPos := b.SimplePathfind(unit, enemyCore.Pos)
				b.Move(unit, nextPos)
			}
		}
	}
})
```
:::

AWESOME! (You should have just won.)

---

## A second unit & mining gems

Finally, let's go a step further!

To defeat more advanced opponents, we will need more gems than the idle income we get simply by waiting over time - we need to actually pick up the gems you have probably seen lying around in the form of gem piles yourself. Like modern life, CORE game is about capitalism - if you have more gems, you're likely to crush your opponent.

You may also have noticed there are deposit objects around. They are huge amounts of gems encased in stone, and they are the secret to earning tons of gems and overrunning your opponent with units.

Let's mine a few!

Firstly, let's spawn the miner unit. As you can see in the [config](documentation/configs), it does more damage to deposits in comparison to warriors. Using miners to mine deposits will therefore be a lot quicker.

First, we need to spawn miner units, so let's modify the unit spawning logic at the beginning:

::: code-group labels=[C, Go]
```c
// getter filtering utility function
bool ft_is_miner(const t_obj *obj)
{
	if (obj->type != OBJ_UNIT) return false;
	if (obj->s_unit.unit_type != UNIT_MINER) return false;
	return true;
}

// ---

void ft_on_tick(unsigned long tick)
{
	printf("-----> [⚡️ TICK %ld 🔥]\n", tick);

	if (CORE_get_objs_filter_count(ft_is_miner) < 2)
		CORE_action_createUnit(UNIT_MINER);
	CORE_action_createUnit(UNIT_ACID_WARRIOR);
// ...
```
```go
bot.Run(func(g *game.Game, b *coregame.Bot) {
	fmt.Printf("-----> [⚡️ TICK %d 🔥]\n", g.ElapsedTicks)

	miners := g.ObjectsFilter(func(obj *game.Object) bool {
		data := obj.GetUnitData()
		return obj.Type == game.ObjectUnit && data != nil && data.TeamID == g.MyTeamID && data.UnitType == game.UnitMiner
	})
	if len(miners) < 2 {
		b.CreateUnit(game.UnitMiner)
	}
	b.CreateUnit(game.UnitWarrior)
// ...
```
:::

This will guarantee that there are always at least 2 miners, and we will start spawning warriors afterwards. In C, [`CORE_get_objs_filter_count`](reference/c/getters/core_get_objs_filter_count) is similar to previous getter functions, but it simply returns an integer. In Go, use `len(g.ObjectsFilter(...))` to count matching objects.

Thirdly, let's have each of our miners attack the nearest deposits that are closest to them to mine them. In C, you can use the helpfully provided `ft_get_deposit_nearest` function from `getters.c`. In Go, use `g.NearestObject` with a deposit predicate.

::: code-group labels=[C, Go]
```c
for (size_t i = 0; my_units && my_units[i] != NULL; i++)
{
	t_obj *unit = my_units[i];

	if (unit->s_unit.unit_type == UNIT_ACID_WARRIOR)
	{
		// ...
	}

	else if (unit->s_unit.unit_type == UNIT_MINER)
	{
		t_obj *nearest_deposit = ft_get_deposit_nearest(unit->pos);
		if (nearest_deposit)
			CORE_action_pathfind(unit, nearest_deposit->pos);
	}
}
```
```go
for _, unit := range myUnits {
	data := unit.GetUnitData()
	if data == nil {
		continue
	}

	if data.UnitType == game.UnitWarrior {
		// ...
	} else if data.UnitType == game.UnitMiner {
		nearestDeposit := g.NearestObject(unit.Pos, func(obj *game.Object) bool {
			return obj.Type == game.ObjectDeposit
		})
		if nearestDeposit != nil {
			nextPos := b.SimplePathfind(unit, nearestDeposit.Pos)
			b.Move(unit, nextPos)
		}
	}
}
```
:::

Run the program!
As you may have noticed, things aren't working quite as expected - the deposits are being mined, but our CORE is not gaining any gems. The gems just fall on the floor and our miner is not picking them up. The reason for this is that once a deposit was fully mined, it turns into a gem pile object with the same amount of gems. But since our miners aren't targeting gem piles, they just walk along.

If we don't have our miners target the nearest deposit, but the nearest deposit or gem pile, they will attack the gem pile after it has dropped from the destroyed deposit. The gems will therefore be picked up by the miner. In C, use the `ft_get_deposit_gems_nearest` function from `getters.c`. In Go, expand the predicate.

::: code-group labels=[C, Go]
```c
else if (unit->s_unit.unit_type == UNIT_MINER)
{
	t_obj *nearest_deposit_or_gem_pile = ft_get_deposit_gems_nearest(unit->pos);
	if (nearest_deposit_or_gem_pile)
		CORE_action_pathfind(unit, nearest_deposit_or_gem_pile->pos);
}
```
```go
} else if data.UnitType == game.UnitMiner {
	nearest := g.NearestObject(unit.Pos, func(obj *game.Object) bool {
		return obj.Type == game.ObjectDeposit || obj.Type == game.ObjectGemPile
	})
	if nearest != nil {
		nextPos := b.SimplePathfind(unit, nearest.Pos)
		b.Move(unit, nextPos)
	}
}
```
:::

Great! Now the gems are being mined & picked up correctly. Trouble is that they are being picked up by the miner unit, not the CORE. We just need to bring them to our CORE now, so the CORE can use them to spawn more units!

To do this, we can use yet another action, the transferGems action ([C](reference/c/actions/core_action_transferGems) | [Go](reference/go/actions/TransferGems)).

The logic itself isn't too tricky - we can just check whether the unit is holding any gems. If so, we should walk back to our CORE and transfer the gems to the CORE, and if not, we should go get some more by mining deposits or picking up normal gem piles.

::: code-group labels=[C, Go]
```c
else if (unit->s_unit.unit_type == UNIT_MINER)
{
	if (unit->s_unit.gems > 0)
	{
		t_obj *own_CORE = ft_get_CORE_own();
		CORE_action_pathfind(unit, own_CORE->pos);
		CORE_action_transferGems(unit, own_CORE->pos, unit->s_unit.gems);
	}
	else
	{
		t_obj *nearest_deposit_or_gem_pile = ft_get_deposit_gems_nearest(unit->pos);
		if (nearest_deposit_or_gem_pile)
			CORE_action_pathfind(unit, nearest_deposit_or_gem_pile->pos);
	}
}
```
```go
} else if data.UnitType == game.UnitMiner {
	if data.Gems != nil && *data.Gems > 0 {
		myCore := g.MyCore()
		if myCore != nil {
			nextPos := b.SimplePathfind(unit, myCore.Pos)
			b.Move(unit, nextPos)
			b.TransferGems(unit, myCore.Pos, *data.Gems)
		}
	} else {
		nearest := g.NearestObject(unit.Pos, func(obj *game.Object) bool {
			return obj.Type == game.ObjectDeposit || obj.Type == game.ObjectGemPile
		})
		if nearest != nil {
			nextPos := b.SimplePathfind(unit, nearest.Pos)
			b.Move(unit, nextPos)
		}
	}
}
```
:::

Awesome! It works!

I hope you enjoyed this tutorial.

---

## Final code result

Here's the entire code block we ended up with:

::: code-group labels=[C, Go]
```c
#include "bot.h"

void ft_on_tick(unsigned long tick);

int main(int argc, char **argv)
{
	return CORE_startGame("testiebestie", argc, argv, ft_on_tick, false);
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

	if (CORE_get_objs_filter_count(ft_is_miner) < 2)
		CORE_action_createUnit(UNIT_MINER);
	CORE_action_createUnit(UNIT_ACID_WARRIOR);

	t_obj **my_units = ft_get_units_own();

	for (size_t i = 0; my_units && my_units[i] != NULL; i++)
	{
		t_obj *unit = my_units[i];

		if (unit->s_unit.unit_type == UNIT_ACID_WARRIOR)
		{
			t_obj *opponent_CORE = ft_get_CORE_opponent();
			t_obj *nearest_enemy = ft_get_units_opponent_nearest(unit->pos);

			if (nearest_enemy)
				CORE_action_pathfind(unit, nearest_enemy->pos);
			else if (opponent_CORE)
				CORE_action_pathfind(unit, opponent_CORE->pos);
		}
		else if (unit->s_unit.unit_type == UNIT_MINER)
		{
			if (unit->s_unit.gems > 0)
			{
				t_obj *own_CORE = ft_get_CORE_own();
				CORE_action_pathfind(unit, own_CORE->pos);
				CORE_action_transferGems(unit, own_CORE->pos, unit->s_unit.gems);
			}
			else
			{
				t_obj *nearest_deposit_or_gem_pile = ft_get_deposit_gems_nearest(unit->pos);
				if (nearest_deposit_or_gem_pile)
					CORE_action_pathfind(unit, nearest_deposit_or_gem_pile->pos);
			}
		}
	}

	free(my_units);
}
```
```go
package main

import (
	"fmt"
	"os"
	"strconv"

	coregame "github.com/42core-team/go-client-lib"
	"github.com/42core-team/go-client-lib/game"
)

func main() {
	teamID, _ := strconv.Atoi(os.Args[1])
	cfg := coregame.DefaultBotConfig(teamID, "testiebestie")
	bot, err := coregame.NewBot(cfg)
	if err != nil {
		fmt.Println("Error:", err)
		return
	}
	defer bot.Close()

	bot.Run(func(g *game.Game, b *coregame.Bot) {
		fmt.Printf("-----> [⚡️ TICK %d 🔥]\n", g.ElapsedTicks)

		miners := g.ObjectsFilter(func(obj *game.Object) bool {
			data := obj.GetUnitData()
			return obj.Type == game.ObjectUnit && data != nil && data.TeamID == g.MyTeamID && data.UnitType == game.UnitMiner
		})
		if len(miners) < 2 {
			b.CreateUnit(game.UnitMiner)
		}
		b.CreateUnit(game.UnitWarrior)

		myUnits := g.TeamUnits()

		for _, unit := range myUnits {
			data := unit.GetUnitData()
			if data == nil {
				continue
			}

			if data.UnitType == game.UnitWarrior {
				enemyCore := g.EnemyCore()
				nearestEnemy := g.NearestObject(unit.Pos, func(obj *game.Object) bool {
					d := obj.GetUnitData()
					return obj.Type == game.ObjectUnit && d != nil && d.TeamID != g.MyTeamID && d.TeamID != 0
				})

				if nearestEnemy != nil {
					nextPos := b.SimplePathfind(unit, nearestEnemy.Pos)
					b.Move(unit, nextPos)
				} else if enemyCore != nil {
					nextPos := b.SimplePathfind(unit, enemyCore.Pos)
					b.Move(unit, nextPos)
				}
			} else if data.UnitType == game.UnitMiner {
				if data.Gems != nil && *data.Gems > 0 {
					myCore := g.MyCore()
					if myCore != nil {
						nextPos := b.SimplePathfind(unit, myCore.Pos)
						b.Move(unit, nextPos)
						b.TransferGems(unit, myCore.Pos, *data.Gems)
					}
				} else {
					nearest := g.NearestObject(unit.Pos, func(obj *game.Object) bool {
						return obj.Type == game.ObjectDeposit || obj.Type == game.ObjectGemPile
					})
					if nearest != nil {
						nextPos := b.SimplePathfind(unit, nearest.Pos)
						b.Move(unit, nextPos)
					}
				}
			}
		}
	})
}
```
:::

---

## What next?

- We fully missed out on the most powerful unit: The carrier. You may have noticed already that the miners walk a little slower when they are holding gems - that is because gems are quite heavy, so they struggle to carry them. But don't worry, the carrier is well-equipped to help you will all of your speed-carrying needs! The transfer gems action even works between two units, so carriers can take all the heavy gems off the miners' hands!
- There may be even more units in your config depending on the event you are in! Check them out to not miss out on powerful strategic possibilities. If you're not sure why you should use them, check out the [configs](documentation/configs) where you can read all the hard facts.
- The code currently has plenty of action failures, almost all of which are easily avoidable. Adding some code to get rid of some of the nasty red error lines will teach you a lot.
- How should we handle opponents that try to attack our units? Maybe we can figure out a way to set up a fleeing logic.
- Using CORE game's debug features, we can add custom text to the unit tooltips. This can be super useful to know what a unit's current job is in order to avoid confusion.
- What if an opponent warrior has reached our CORE, but all our warriors are near their CORE - can we keep a few warriors always close to our CORE as a defensive measure?

Before you get ahead of yourself, make sure to read the [Basics Guide](basics). Whatever you plan on doing, the wiki will be here to give info on every function, struct or concept you may be wondering about. Good luck!
