You can ask a unit to perform as many actions you want in a given tick. You may think a problem could arise when you ask a unit to move to two different positions in one tick, or if you try to move a unit to an empty position while the opponent tries to move another unit to the same position in the same tick. What would happen in these cases?\
Here is how action functions are applied on the server side:
1. All action functions are collected for all connected clients.
2. They are sorted by type (in the order: transfer_gems, build, attack, move, create), then shuffled per-type. Whether a specific attack action instance is therefore executed before another attack action instance is random, but all move actions will always be executed after all the attack actions.
3. They are executed in that order.

Therefore, if you ask a unit to move to two different positions in one tick, it will be entirely random which will execute first. One of them will be executed, and one of them will fail with an error.

Same thing happens in the case where the opponent is involved. It would be entirely 50-50 random whether yours or your opponents unit ends up on the target position. Good luck!
