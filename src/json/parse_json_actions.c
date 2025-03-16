#include "parse_json.h"
#include "event_handler.h"
#include "con_lib.h"

void parse_json_actions(json_node *root)
{
	json_node *actions_node = json_find(root, "actions");
	if (actions_node && actions_node->type == JSON_TYPE_ARRAY && actions_node->array)
	{
		for (int i = 0; actions_node->array[i] != NULL; i++)
		{
			json_node *action = actions_node->array[i];
			json_node *type_node = json_find(action, "type");
			if (!type_node || type_node->type != JSON_TYPE_STRING)
				continue;
			
			if (strcmp(type_node->string, "build") == 0)
			{
				json_node *builder_node = json_find(action, "builder_id");
				json_node *x_node = json_find(action, "x");
				json_node *y_node = json_find(action, "y");
				if (builder_node && x_node && y_node)
				{
					t_pos pos;
					pos.x = (unsigned short)x_node->number;
					pos.y = (unsigned short)y_node->number;
					if (event_handler.on_unit_build)
						event_handler.on_unit_build(ft_get_obj_from_id((unsigned long)builder_node->number), pos, user_data);
				}
			}
			else if (strcmp(type_node->string, "transfer_money") == 0)
			{
				json_node *source_node = json_find(action, "source_id");
				json_node *target_node = json_find(action, "target_id");
				json_node *amount_node = json_find(action, "amount");
				if (source_node && target_node && amount_node)
					if (event_handler.on_unit_transfer_money)
						event_handler.on_unit_transfer_money(ft_get_obj_from_id((unsigned long)source_node->number),
																ft_get_obj_from_id((unsigned long)target_node->number),
																(unsigned long)amount_node->number,
																user_data);
			}
			else if (strcmp(type_node->string, "move") == 0)
			{
				// Only trigger unit attack if the move action indicates an attack.
				// Normal moves are handled by the on_object_pos_change event
				json_node *attacked_node = json_find(action, "attacked");
				if (attacked_node && attacked_node->type == JSON_TYPE_BOOL && attacked_node->number)
				{
					json_node *unit_node = json_find(action, "unit_id");
					if (unit_node && event_handler.on_unit_attack)
						event_handler.on_unit_attack(ft_get_obj_from_id((unsigned long)unit_node->number), 0, 0, user_data);
				}
			}
		}
	}
}
