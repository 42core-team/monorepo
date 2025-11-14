#include "core_lib.h"
#include "core_lib_internal.h"

t_debug_data debug_data = {0};

// Helper to ensure newline at end of string
static void core_static_ensureNewline(char **str)
{
	if (!str || !*str) return;

	size_t len = strlen(*str);
	if (len == 0) return;

	if ((*str)[len - 1] != '\n')
	{
		*str = realloc(*str, len + 2);
		(*str)[len] = '\n';
		(*str)[len + 1] = '\0';
	}
}

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
		// Append to existing info
		core_static_ensureNewline(&entry->info);

		size_t old_len = strlen(entry->info);
		size_t new_len = strlen(info);
		entry->info = realloc(entry->info, old_len + new_len + 1);
		strcat(entry->info, info);
	}
}
