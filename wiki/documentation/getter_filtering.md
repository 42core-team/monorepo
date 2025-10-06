Filtering lets you pass in a custom function to some getter functions that decides whether an object should be included or not.

```c
// Simple filter: return true only if obj is a deposit and alive
static bool is_deposit(const t_obj *obj)
{
	return (obj->type == OBJ_DEPOSIT && obj->state == STATE_ALIVE);
}

void example(void)
{
	t_pos pos = {5, 5}; // search around this position

	// Use the filter with the nearest-object getter
	t_obj *nearest = core_get_obj_filter_nearest(pos, is_deposit);

	if (nearest) printf("Nearest deposit at (%d,%d)\n", nearest->pos.x, nearest->pos.y);
}
```

Here, `is_deposit` acts as the condition. The getter runs this check on every object in the game and returns the one closest to `(5,5)` that passes.
