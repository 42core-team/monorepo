Bombs can be built by units that have a t_build_type of "bomb". If there is no such unit in your event, you may not have to worry about being attacked with bombs.

They cost a certain amount of gems and the gems must be held by the unit that is building the bomb, not by the units core.

Bombs can only be built directly up, down, left or right of the builder unit.

💣💣💣💣💣💥

To start a bombs countdown, attack it.

The bombs countdown will then continue to decrement by 1 each tick. After a bombs countdown has ended, an explosion will trigger. The explosion will realistically flood out, stopping only at walls.

If an idle or still-counting-down bomb is within range of an explosion, it will explode immediately, creating a chain reaction.

If the bomb receives enough damage while its countdown is going down to reach 0 hp before its explosion, the bomb will be defused, destroying it without causing an explosion.

> The algorithm used to determine whether a given grid position should be hit with explosion damage is a sort of reversed Bresenham / raycast type algorithm. From every possible position in range that could explode, we'll draw a metaphorical line between the center of that grid position and the center of the bomb. If this line crosses over any walls, the position won't explode, otherwise it will. If the line crosses over an edge of a wall exactly, that won't count as the wall overlapping.

The bomb reach is in addition the the tile the bomb is standing on, so a bomb reach of three would result in an explosion with a diameter of 7.

For more info & specifics, please check out [the server bomb code](https://github.com/42core-team/monorepo/blob/dev/server/src/object/Bomb.cpp) directly.

## Example

// TODO: Example Bomb Code
