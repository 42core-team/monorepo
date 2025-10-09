Bombs can be built by units that have a t_build_type of "bomb". If there is no such unit in your event, you may not have to worry about being attacked with bombs.

They cost a certain amount of gems and the gems must be held by the unit that is building the bomb, not by the units core.

Bombs can only be built directly up, down, left or right of the builder unit.

💣💣💣💣💣💥

To start a bombs countdown, attack it.

The bombs countdown will then continue to decrement by 1 each tick. After a bombs countdown has ended, an explosion will trigger. The explosion will realistically flood out, stopping only at walls.

If an idle or still-counting-down bomb is within range of an explosion, it will explode immediately, creating a chain reaction.

If the bomb receives enough damage while its countdown is going down to reach 0 hp before its explosion, the bomb will be defused, destroying it without causing an explosion.

> [!INFO]
> The algorithm used to determine whether a given grid position should be hit with explosion damage is a sort of reversed Bresenham / raycast type algorithm. From every possible position in range that could explode, we'll draw a metaphorical line between the center of that grid position and the center of the bomb. If this line crosses over any walls, the position won't explode, otherwise it will. If the line crosses over an edge of a wall exactly, that won't count as the wall overlapping.

The bomb reach is in addition the the tile the bomb is standing on, so a bomb reach of three would result in an explosion with a diameter of 7.

For more info & specifics, please check out [the server bomb code](https://github.com/42core-team/monorepo/blob/dev/server/src/object/Bomb.cpp) directly.

## Example

Warning: This code is not meant to be good, it's just to supposed to show off how bombs can work. That is to say, this code spawns a bomberman that ignites a bomb right next to its own core. Unfortunate. Tweaks are recommended.

```c
static bool is_bomberman(const t_obj *o)
{
	return (o->type == OBJ_UNIT && o->s_unit.team_id == game.my_team_id && o->s_unit.unit_type == UNIT_BOMBERMAN);
}

void spawn_and_update_bombermen(void)
{
	if (core_get_objs_filter_count(is_bomberman) == 0
		&& ft_get_core_own()->s_core.gems >= core_get_unitConfig(UNIT_BOMBERMAN)->cost)
		core_action_createUnit(UNIT_BOMBERMAN);

	t_obj **bombermen = core_get_objs_filter(is_bomberman);
	for (size_t i = 0; bombermen && bombermen[i]; i++)
	{
		t_obj *bomber = bombermen[i];
		if (bomber->s_unit.action_cooldown != 0) continue;

		// super-simple state machine: NULL = need to build, (void*)1 = built, waiting to light, (void*)2 = done

		if (bomber->data == NULL)
		{
			// 1. transfer bomb cost gems
			unsigned long missing = (game.config.bomb_throw_cost > bomber->s_unit.gems)
				? (game.config.bomb_throw_cost - bomber->s_unit.gems) : 0;
			if (missing > 0)
				core_action_transferGems(ft_get_core_own(), bomber->pos, missing);

			// 2. build bomb
			t_pos p = bomber->pos;
			t_pos adj[4] = { {p.x+1,p.y},{p.x-1,p.y},{p.x,p.y+1},{p.x,p.y-1} };
			for (int k = 0; k < 4; k++)
			{
				t_pos s = adj[k];
				if (s.x < game.config.gridSize && s.y < game.config.gridSize && core_get_obj_from_pos(s) == NULL) 
				{
					core_action_build(bomber, s);
					bomber->data = (void*)1; // build now, light next tick
					break;
				}
			}
			continue;
		}

		if (bomber->data == (void*)1)
		{
			// 3. find adjacent bomb and light it
			t_pos p = bomber->pos;
			t_pos adj[4] = { {p.x+1,p.y},{p.x-1,p.y},{p.x,p.y+1},{p.x,p.y-1} };
			for (int k = 0; k < 4; k++) {
				t_obj *maybe_bomb = core_get_obj_from_pos(adj[k]);
				if (maybe_bomb && maybe_bomb->type == OBJ_BOMB) {
					core_action_attack(bomber, maybe_bomb);
					bomber->data = (void*)2; // done
					break;
				}
			}
		}
	}
	free(bombermen);
}
```
