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

		json_node *obj = create_node(JSON_TYPE_OBJECT);
		obj->array = malloc(sizeof(json_node *) * 4); // object_id, object_info, object_path, NULL

		// object_id
		json_node *id_node = create_node(JSON_TYPE_NUMBER);
		id_node->key = strdup("object_id");
		id_node->number = clamp_ulong_for_json(entry->object_id);
		obj->array[0] = id_node;

		// object_info
		json_node *info_node = create_node(JSON_TYPE_STRING);
		info_node->key = strdup("object_info");
		info_node->string = entry->info ? strdup(entry->info) : strdup("");
		obj->array[1] = info_node;

		// object_path
		if (entry->path)
		{
			json_node *path_node = create_node(JSON_TYPE_ARRAY);
			path_node->key = strdup("object_path");

			int path_len = 0;
			t_debug_path_node *curr = entry->path;
			while (curr)
			{
				curr = curr->next;
				path_len++;
			}

			path_node->array = malloc(sizeof(json_node *) * (path_len + 1));
			curr = entry->path;
			int idx = 0;
			while (curr)
			{
				json_node *pos_node = create_node(JSON_TYPE_OBJECT);
				pos_node->array = malloc(sizeof(json_node *) * 3); // x, y, NULL

				json_node *x_node = create_node(JSON_TYPE_NUMBER);
				x_node->key = strdup("x");
				x_node->number = curr->pos.x;
				pos_node->array[0] = x_node;

				json_node *y_node = create_node(JSON_TYPE_NUMBER);
				y_node->key = strdup("y");
				y_node->number = curr->pos.y;
				pos_node->array[1] = y_node;

				pos_node->array[2] = NULL;

				curr = curr->next;
				path_node->array[idx++] = pos_node;
			}
			path_node->array[path_len] = NULL;

			obj->array[2] = path_node;
		}
		else
		{
			obj->array[2] = NULL;
		}

		obj->array[3] = NULL;
		arr->array[i] = obj;
	}

	arr->array[debug_data.count] = NULL;
	return arr;
}
