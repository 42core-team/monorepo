#include "core_lib_internal.h"

char *core_internal_encode_packet(void)
{
	// Build debug data array
	json_node *debug_arr = core_internal_encode_packet_debugData();
	json_node *actions_arr = core_internal_encode_packet_actions();

	// Build root object with actions and debug_data
	json_node *root = create_node(JSON_TYPE_OBJECT);
	root->array = malloc(sizeof(json_node *) * 3); // actions, debug_data, NULL

	json_node *ka = create_node(JSON_TYPE_ARRAY);
	ka->key = strdup("actions");
	ka->array = actions_arr->array;
	root->array[0] = ka;

	json_node *kd = create_node(JSON_TYPE_ARRAY);
	kd->key = strdup("debug_data");
	kd->array = debug_arr->array;
	root->array[1] = kd;

	root->array[2] = NULL;

	char *out = json_to_string(root);
	free_json(root);
	free(actions_arr);
	free(debug_arr);

	// append newline
	size_t L = strlen(out);
	out = realloc(out, L + 2);
	out[L] = '\n';
	out[L + 1] = '\0';
	return out;
}
