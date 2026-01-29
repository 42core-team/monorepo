#include "core_lib.h"
#include "core_lib_internal.h"

t_debug_data debug_data = {0};

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

void core_debug_addObjectInfo(const t_obj *obj, const char *info)
{
	if (!obj || !info) return;

	t_debug_entry *entry = core_static_findOrCreateEntry(obj->id);

	if (entry->info == NULL)
	{
		// First info for this object
		entry->info = strdup(info);
	}
	else
	{
		size_t old_len = strlen(entry->info);
		size_t new_len = strlen(info);
		entry->info = realloc(entry->info, old_len + new_len + 1);
		strcat(entry->info, info);
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
