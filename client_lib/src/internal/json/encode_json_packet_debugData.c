#include "core_lib_internal.h"

// Declare external debug data
extern t_debug_data debug_data;

// Encode debug data as JSON array
json_node *core_internal_encode_packet_debugData(void)
{
	json_node *arr = create_node(JSON_TYPE_ARRAY);

	if (debug_data.count == 0)
	{
		// Empty array
		arr->array = malloc(sizeof(json_node *));
		arr->array[0] = NULL;
		return arr;
	}

	arr->array = malloc(sizeof(json_node *) * (debug_data.count + 1));

	for (unsigned int i = 0; i < debug_data.count; i++)
	{
		t_debug_entry *entry = &debug_data.entries[i];

		// Create debug entry object
		json_node *obj = create_node(JSON_TYPE_OBJECT);
		obj->array = malloc(sizeof(json_node *) * 3); // object_id, object_info, NULL

		// Add object_id
		json_node *id_node = create_node(JSON_TYPE_NUMBER);
		id_node->key = strdup("object_id");
		id_node->number = clamp_ulong_for_json(entry->object_id);
		obj->array[0] = id_node;

		// Add object_info (handle NULL case)
		json_node *info_node = create_node(JSON_TYPE_STRING);
		info_node->key = strdup("object_info");
		info_node->string = entry->info ? strdup(entry->info) : strdup("");
		obj->array[1] = info_node;

		obj->array[2] = NULL;
		arr->array[i] = obj;
	}

	arr->array[debug_data.count] = NULL;
	return arr;
}
