#include "core_lib_internal.h"

json_node *core_internal_encode_packet_actions(void)
{
	// Build actions array
	json_node *actions_arr = create_node(JSON_TYPE_ARRAY);
	actions_arr->array = malloc(sizeof(json_node *) * (actions.count + 1));

	for (unsigned i = 0; i < actions.count; i++)
	{
		t_action *a = &actions.list[i];
		json_node *obj = create_node(JSON_TYPE_OBJECT);

		// always allocate 4–6 slots; worst case we need 5 keys + NULL
		obj->array = malloc(sizeof(json_node *) * 6);
		int idx = 0;

		json_node *t = create_node(JSON_TYPE_STRING);
		t->key = strdup("type");

		switch (a->type)
		{
		case ACTION_CREATE:
			t->string = strdup("create");
			obj->array[idx++] = t;
			{
				size_t component_count = 0;
				while (a->data.create.components && a->data.create.components[component_count])
					component_count++;

				json_node *components = create_node(JSON_TYPE_ARRAY);
				components->key = strdup("components");
				components->array = malloc(sizeof(json_node *) * (component_count + 1));

				for (size_t j = 0; j < component_count; j++)
				{
					json_node *component = create_node(JSON_TYPE_STRING);
					component->string = strdup(a->data.create.components[j]);
					components->array[j] = component;
				}

				components->array[component_count] = NULL;
				obj->array[idx++] = components;
			}
			break;
		case ACTION_MOVE:
			t->string = strdup("move");
			obj->array[idx++] = t;
			// id, x, y
			{
				json_node *uid = create_node(JSON_TYPE_NUMBER);
				uid->key = strdup("unit_id");
				uid->number = clamp_ulong_for_json(a->data.move.id);
				obj->array[idx++] = uid;
				json_node *x = create_node(JSON_TYPE_NUMBER);
				x->key = strdup("x");
				x->number = clamp_ulong_for_json(a->data.move.pos.x);
				obj->array[idx++] = x;
				json_node *y = create_node(JSON_TYPE_NUMBER);
				y->key = strdup("y");
				y->number = clamp_ulong_for_json(a->data.move.pos.y);
				obj->array[idx++] = y;
			}
			break;
		case ACTION_ATTACK:
			t->string = strdup("attack");
			obj->array[idx++] = t;
			// same pattern…
			{
				json_node *uid = create_node(JSON_TYPE_NUMBER);
				uid->key = strdup("unit_id");
				uid->number = clamp_ulong_for_json(a->data.attack.id);
				obj->array[idx++] = uid;
				json_node *target_id = create_node(JSON_TYPE_NUMBER);
				target_id->key = strdup("target_id");
				target_id->number = clamp_ulong_for_json(a->data.attack.target_id);
				obj->array[idx++] = target_id;
			}
			break;
		case ACTION_TRANSFER:
			t->string = strdup("transfer_gems");
			obj->array[idx++] = t;
			{
				json_node *src = create_node(JSON_TYPE_NUMBER);
				src->key = strdup("source_id");
				src->number = clamp_ulong_for_json(a->data.transfer.source_id);
				obj->array[idx++] = src;
				json_node *amt = create_node(JSON_TYPE_NUMBER);
				amt->key = strdup("amount");
				amt->number = clamp_ulong_for_json(a->data.transfer.amount);
				obj->array[idx++] = amt;
				json_node *x = create_node(JSON_TYPE_NUMBER);
				x->key = strdup("x");
				x->number = clamp_ulong_for_json(a->data.transfer.target_pos.x);
				obj->array[idx++] = x;
				json_node *y = create_node(JSON_TYPE_NUMBER);
				y->key = strdup("y");
				y->number = clamp_ulong_for_json(a->data.transfer.target_pos.y);
				obj->array[idx++] = y;
			}
			break;
		}
		obj->array[idx] = NULL;
		actions_arr->array[i] = obj;
	}
	actions_arr->array[actions.count] = NULL;

	return actions_arr;
}
