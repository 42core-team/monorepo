#include "core_lib.h"
#include "core_lib_internal.h"

t_debug_data debug_data = {0};

/**
 * Locate the debug entry for the given object ID, creating and initializing one if it does not exist.
 *
 * @param object_id Identifier of the object whose debug entry is requested.
 * @returns Pointer to the persistent `t_debug_entry` for `object_id`. The entry will have `info` and `path` initialized to NULL if newly created. Note: the pointer may be invalidated by future operations that reallocate the entries array. 
 */
static t_debug_entry *core_static_findOrCreateEntry(unsigned long object_id)
{
	for (unsigned int i = 0; i < debug_data.count; i++)
	{
		if (debug_data.entries[i].object_id == object_id) return &debug_data.entries[i];
	}

	// Need to create new entry
	if (debug_data.entries == NULL)
	{
		debug_data.capacity = 8;
		debug_data.entries = malloc(sizeof(t_debug_entry) * debug_data.capacity);
	}
	else if (debug_data.count >= debug_data.capacity)
	{
		debug_data.capacity *= 2;
		debug_data.entries = realloc(debug_data.entries, sizeof(t_debug_entry) * debug_data.capacity);
	}

	// Initialize new entry
	t_debug_entry *entry = &debug_data.entries[debug_data.count++];
	entry->object_id = object_id;
	entry->info = NULL;
	entry->path = NULL;

	return entry;
}

/**
 * Append formatted informational text to the debug entry for the given object.
 *
 * Formats a printf-style string and appends it to the object's debug info, creating
 * a debug entry if necessary. If `obj` or `format` is NULL, or if formatting/allocation
 * fails, the function returns without modifying state.
 *
 * @param obj Object whose debug entry will receive the appended text; must be non-NULL.
 * @param format printf-style format string describing the text to append, followed by
 *        matching variadic arguments.
 */
void core_debug_addObjectInfo(const t_obj *obj, const char *format, ...)
{
	if (!obj || !format) return;

	t_debug_entry *entry = core_static_findOrCreateEntry(obj->id);
	if (!entry) return;

	// format the new info string
	va_list ap;
	va_start(ap, format);

	va_list ap2;
	va_copy(ap2, ap);

	int needed = vsnprintf(NULL, 0, format, ap);
	va_end(ap);

	if (needed < 0)
	{
		va_end(ap2);
		return;
	}

	char *msg = (char *)malloc((size_t)needed + 1);
	if (!msg)
	{
		va_end(ap2);
		return;
	}

	vsnprintf(msg, (size_t)needed + 1, format, ap2);
	va_end(ap2);

	// append to debug info
	if (entry->info == NULL)
	{
		entry->info = msg;
	}
	else
	{
		size_t old_len = strlen(entry->info);
		size_t new_len = strlen(msg);
		entry->info = realloc(entry->info, old_len + new_len + 1);
		strcat(entry->info, msg);
		free(msg);
	}
}

void core_debug_addObjectPathStep(const t_obj *unit, t_pos pos)
{
	if (!unit) return;

	t_debug_entry *entry = core_static_findOrCreateEntry(unit->id);

	// Create new path node
	t_debug_path_node *new_node = malloc(sizeof(t_debug_path_node));
	new_node->pos = pos;
	new_node->next = NULL;

	// Append to path
	if (entry->path == NULL)
	{
		entry->path = new_node;
	}
	else
	{
		t_debug_path_node *current = entry->path;
		while (current->next)
		{
			current = current->next;
		}
		current->next = new_node;
	}
}