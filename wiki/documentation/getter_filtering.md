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
// Simple filter: return true only if obj is a deposit
isDeposit := func(obj *game.Object) bool {
	return obj != nil && obj.Type == game.ObjectDeposit
}

pos := game.Position{X: 5, Y: 5} // search around this position

// Use the filter with the nearest-object getter
if nearest := core.NearestObject(pos, isDeposit); nearest != nil {
	fmt.Printf("Nearest deposit at (%d,%d)\n", nearest.Pos.X, nearest.Pos.Y)
}
```

:::

Here, the deposit check acts as the condition. The getter runs this check on every object in the game and returns the one closest to `(5,5)` that passes. This can be used with a large variety of different getter functions.

::: lang c

C has no lambda functions, so conditions are separate functions that you pass by name. To filter on something that changes, such as a team or a position, read it from the global `game` state inside the condition. See [`core_get_objs_filter`](reference/c/getters/core_get_objs_filter), [`core_get_objs_filter_count`](reference/c/getters/core_get_objs_filter_count) and [`core_get_obj_filter_nearest`](reference/c/getters/core_get_obj_filter_nearest).

:::

::: lang go

Conditions are ordinary Go functions, so they can be closures that capture values such as a team ID or a position. The same condition works with every filter getter:

```go
deposits := core.ObjectsFilter(isDeposit)
count := core.ObjectsFilterCount(isDeposit)
```

A nil predicate returns no results from `ObjectsFilter` and `NearestObject`. `ObjectsFilterCount(nil)` is the exception: it counts every object. See the [Go getter reference](reference/go/getters/GameGetters).

:::
