Filtering lets you pass in a custom function to some getter functions that decides whether an object should be included or not.

::: code-group labels=[C, Go]
```c
// Simple filter: return true only if obj is a deposit
static bool is_deposit(const t_obj *obj)
{
	return (obj->type == OBJ_DEPOSIT);
}

void example(void)
{
	t_pos pos = {5, 5}; // search around this position

	// Use the filter with the nearest-object getter
	t_obj *nearest = core_get_obj_filter_nearest(pos, is_deposit);

	if (nearest) printf("Nearest deposit at (%d,%d)\n", nearest->pos.x, nearest->pos.y);
}
```
```go
func example(g *game.Game) {
	pos := game.Position{X: 5, Y: 5} // search around this position

	// Use an inline predicate with the nearest-object getter
	nearest := g.NearestObject(pos, func(obj *game.Object) bool {
		return obj.Type == game.ObjectDeposit
	})

	if nearest != nil {
		fmt.Printf("Nearest deposit at (%d,%d)\n", nearest.Pos.X, nearest.Pos.Y)
	}
}
```
:::

- **C**: Here, `is_deposit` acts as the condition. The getter runs this check on every object in the game and returns the one closest to `(5,5)` that passes. This can be used with a large variety of different getter functions.
- **Go**: You pass predicate functions (closures) directly as arguments. The getter runs this check on every object in the game and returns the one closest to `(5,5)` that passes. This can be used with `ObjectsFilter()`, `NearestObject()`, and other getter methods.
