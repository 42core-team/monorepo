#include "core_lib_internal.h"

char *core_internal_encode_action(void)
{
	json_node *arr = create_node(JSON_TYPE_ARRAY);
	arr->array = malloc(sizeof(json_node *) * (actions.count + 1));
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
				json_node *u = create_node(JSON_TYPE_NUMBER);
				u->key = strdup("unit_type");
				u->number = a->data.create.unit_type;
				obj->array[idx++] = u;
			}
			break;
		case ACTION_MOVE:
			t->string = strdup("move");
			obj->array[idx++] = t;
			// id, x, y
			{
				json_node *uid = create_node(JSON_TYPE_NUMBER);
				uid->key = strdup("unit_id");
				uid->number = a->data.move.id;
				obj->array[idx++] = uid;
				json_node *x = create_node(JSON_TYPE_NUMBER);
				x->key = strdup("x");
				x->number = a->data.move.pos.x;
				obj->array[idx++] = x;
				json_node *y = create_node(JSON_TYPE_NUMBER);
				y->key = strdup("y");
				y->number = a->data.move.pos.y;
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
				uid->number = a->data.attack.id;
				obj->array[idx++] = uid;
				json_node *x = create_node(JSON_TYPE_NUMBER);
				x->key = strdup("x");
				x->number = a->data.attack.pos.x;
				obj->array[idx++] = x;
				json_node *y = create_node(JSON_TYPE_NUMBER);
				y->key = strdup("y");
				y->number = a->data.attack.pos.y;
				obj->array[idx++] = y;
			}
			break;
		case ACTION_TRANSFER:
			t->string = strdup("transfer_gems");
			obj->array[idx++] = t;
			{
				json_node *src = create_node(JSON_TYPE_NUMBER);
				src->key = strdup("source_id");
				src->number = a->data.transfer.source_id;
				obj->array[idx++] = src;
				json_node *amt = create_node(JSON_TYPE_NUMBER);
				amt->key = strdup("amount");
				amt->number = a->data.transfer.amount;
				obj->array[idx++] = amt;
				json_node *x = create_node(JSON_TYPE_NUMBER);
				x->key = strdup("x");
				x->number = a->data.transfer.target_pos.x;
				obj->array[idx++] = x;
				json_node *y = create_node(JSON_TYPE_NUMBER);
				y->key = strdup("y");
				y->number = a->data.transfer.target_pos.y;
				obj->array[idx++] = y;
			}
			break;
		case ACTION_BUILD:
			t->string = strdup("build");
			obj->array[idx++] = t;
			{
				json_node *bid = create_node(JSON_TYPE_NUMBER);
				bid->key = strdup("unit_id");
				bid->number = a->data.build.builder_id;
				obj->array[idx++] = bid;
				json_node *x = create_node(JSON_TYPE_NUMBER);
				x->key = strdup("x");
				x->number = a->data.build.pos.x;
				obj->array[idx++] = x;
				json_node *y = create_node(JSON_TYPE_NUMBER);
				y->key = strdup("y");
				y->number = a->data.build.pos.y;
				obj->array[idx++] = y;
			}
			break;
		}
		obj->array[idx] = NULL;
		arr->array[i] = obj;
	}
	arr->array[actions.count] = NULL;

	json_node *root = create_node(JSON_TYPE_OBJECT);
	root->array = malloc(sizeof(json_node *) * 2);
	json_node *ka = create_node(JSON_TYPE_ARRAY);
	ka->key = strdup("actions");
	ka->array = arr->array;
	root->array[0] = ka;
	root->array[1] = NULL;

	char *out = json_to_string(root);
	free_json(root);
	free(arr);

	// append newline
	size_t L = strlen(out);
	out = realloc(out, L + 2);
	out[L] = '\n';
	out[L + 1] = '\0';
	return out;
}
