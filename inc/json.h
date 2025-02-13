#ifndef JSON_H
# define JSON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* --- Basic Structs --- */

typedef enum e_json_type
{
	JSON_TYPE_NULL,
	JSON_TYPE_BOOL,
	JSON_TYPE_STRING,
	JSON_TYPE_NUMBER,
	JSON_TYPE_OBJECT,
	JSON_TYPE_ARRAY
} json_type;

typedef struct s_json_node
{
	char *key;
	json_type type;

	union
	{
		char *string;
		double number;
		struct s_json_node **array;
	};
}	json_node;

/* --- Functions --- */

json_node *string_to_json(char *string);					// Convert a JSON string into a JSON tree
char *json_to_string(json_node *json);						// Convert a JSON tree to a JSON string
char *json_to_formatted_string(json_node *json);			// Convert a JSON tree to a formatted JSON string
json_node *json_find(json_node *json, char *key);			// Find a node in the JSON tree by key (top-level only)
json_node *json_find_recursive(json_node *json, char *key);	// Find a node in the JSON tree by key (recursive)
void free_json(json_node *json);							// Free all memory allocated for the JSON tree
json_node *create_node(json_type type);						// Create a new, empty-initialized node

#endif // JSON_H
