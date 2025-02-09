#include "json.h"

/* --- STRING -> JSON --- */

static void skip_whitespace(const char **p)
{
	while (**p && isspace(**p))
		(*p)++;
}

static char *parse_string_literal(const char **p)
{
	if (**p != '"')
	{
		fprintf(stderr, "Expected '\"' at beginning of string literal.\n");
		exit(EXIT_FAILURE);
	}
	(*p)++;

	size_t bufsize = 64, len = 0;
	char *buffer = malloc(bufsize);
	if (!buffer)
	{
		fprintf(stderr, "Memory allocation failed in parse_string_literal.\n");
		exit(EXIT_FAILURE);
	}

	while (**p && **p != '"')
	{
		if (**p == '\\')
		{
			(*p)++;
			if (**p == '\0') break;
			switch (**p)
			{
				case '"':  buffer[len++] = '"'; break;
				case '\\': buffer[len++] = '\\'; break;
				case 'n':  buffer[len++] = '\n'; break;
				case 't':  buffer[len++] = '\t'; break;
				default:   buffer[len++] = **p; break;
			}
		}
		else
		{
			buffer[len++] = **p;
		}
		(*p)++;
		if (len >= bufsize - 1)
		{
			bufsize *= 2;
			buffer = realloc(buffer, bufsize);
			if (!buffer)
			{
				fprintf(stderr, "Memory reallocation failed in parse_string_literal.\n");
				exit(EXIT_FAILURE);
			}
		}
	}
	if (**p != '"')
	{
		fprintf(stderr, "Unterminated string literal.\n");
		exit(EXIT_FAILURE);
	}
	(*p)++;
	buffer[len] = '\0';
	return buffer;
}

static double parse_number_literal(const char **p)
{
	char *endptr;
	double num = strtod(*p, &endptr);
	if (endptr == *p)
	{
		fprintf(stderr, "Failed to parse number.\n");
		exit(EXIT_FAILURE);
	}
	*p = endptr;
	return num;
}

json_node* create_node(json_type type)
{
	json_node *node = malloc(sizeof(json_node));
	if (!node)
	{
		fprintf(stderr, "Memory allocation failed in create_node.\n");
		exit(EXIT_FAILURE);
	}
	node->type = type;
	node->key = NULL;

	switch (type)
	{
		case JSON_TYPE_OBJECT:
		case JSON_TYPE_ARRAY:
			node->array = NULL;
			break;
		case JSON_TYPE_STRING:
			node->string = NULL;
			break;
		case JSON_TYPE_NUMBER:
			node->number = 0;
			break;
		default:
			break;
	}

	return node;
}

static json_node* parse_value(const char **p);

static json_node* parse_object(const char **p)
{
	if (**p != '{')
	{
		fprintf(stderr, "Expected '{' at beginning of object.\n");
		exit(EXIT_FAILURE);
	}
	(*p)++;
	skip_whitespace(p);
	
	json_node *node = create_node(JSON_TYPE_OBJECT);
	size_t capacity = 4, count = 0;
	json_node **members = malloc(sizeof(json_node*) * capacity);
	if (!members)
	{
		fprintf(stderr, "Memory allocation failed for object members.\n");
		exit(EXIT_FAILURE);
	}
	skip_whitespace(p);
	
	if (**p == '}')
	{
		(*p)++;
		members[count] = NULL;
		node->array = members;
		return node;
	}
	
	while (1)
	{
		skip_whitespace(p);
		char *key = parse_string_literal(p);
		skip_whitespace(p);
		if (**p != ':')
		{
			fprintf(stderr, "Expected ':' after object key.\n");
			exit(EXIT_FAILURE);
		}
		(*p)++;
		skip_whitespace(p);
		json_node *value = parse_value(p);
		value->key = key;

		if (count >= capacity)
		{
			capacity *= 2;
			members = realloc(members, sizeof(json_node*) * capacity);
			if (!members)
			{
				fprintf(stderr, "Memory reallocation failed for object members.\n");
				exit(EXIT_FAILURE);
			}
		}
		members[count++] = value;
		skip_whitespace(p);
		if (**p == ',')
		{
			(*p)++;
			skip_whitespace(p);
		}
		else if (**p == '}')
		{
			(*p)++;
			break;
		}
		else
		{
			fprintf(stderr, "Expected ',' or '}' in object.\n");
			exit(EXIT_FAILURE);
		}
	}
	members = realloc(members, sizeof(json_node*) * (count + 1));
	if (!members)
	{
		fprintf(stderr, "Memory reallocation failed for object termination.\n");
		exit(EXIT_FAILURE);
	}
	members[count] = NULL;
	node->array = members;
	return node;
}

static json_node* parse_array(const char **p)
{
	if (**p != '[')
	{
		fprintf(stderr, "Expected '[' at beginning of array.\n");
		exit(EXIT_FAILURE);
	}
	(*p)++;
	skip_whitespace(p);
	
	json_node *node = create_node(JSON_TYPE_ARRAY);
	size_t capacity = 4, count = 0;
	json_node **elements = malloc(sizeof(json_node*) * capacity);
	if (!elements)
	{
		fprintf(stderr, "Memory allocation failed for array elements.\n");
		exit(EXIT_FAILURE);
	}
	skip_whitespace(p);
	
	if (**p == ']')
	{
		(*p)++;
		elements[count] = NULL;
		node->array = elements;
		return node;
	}
	
	while (1)
	{
		skip_whitespace(p);
		json_node *element = parse_value(p);
		if (count >= capacity)
		{
			capacity *= 2;
			elements = realloc(elements, sizeof(json_node*) * capacity);
			if (!elements)
			{
				fprintf(stderr, "Memory reallocation failed for array elements.\n");
				exit(EXIT_FAILURE);
			}
		}
		elements[count++] = element;
		skip_whitespace(p);
		if (**p == ',')
		{
			(*p)++;  // skip comma
			skip_whitespace(p);
		}
		else if (**p == ']')
		{
			(*p)++;
			break;
		}
		else
		{
			fprintf(stderr, "Expected ',' or ']' in array.\n");
			exit(EXIT_FAILURE);
		}
	}
	elements = realloc(elements, sizeof(json_node*) * (count + 1));
	if (!elements)
	{
		fprintf(stderr, "Memory reallocation failed for array termination.\n");
		exit(EXIT_FAILURE);
	}
	elements[count] = NULL;
	node->array = elements;
	return node;
}

static json_node* parse_value(const char **p)
{
	skip_whitespace(p);
	char c = **p;
	if (c == '{')
		return parse_object(p);
	else if (c == '[')
		return parse_array(p);
	else if (c == '"')
	{
		char *str = parse_string_literal(p);
		json_node *node = create_node(JSON_TYPE_STRING);
		node->string = str;
		return node;
	}
	else if (c == '-' || isdigit(c))
	{
		double num = parse_number_literal(p);
		json_node *node = create_node(JSON_TYPE_NUMBER);
		node->number = num;
		return node;
	}
	else if (strncmp(*p, "null", 4) == 0)
	{
		*p += 4;
		return create_node(JSON_TYPE_NULL);
	}
	else
	{
		fprintf(stderr, "Unexpected character '%c' while parsing JSON.\n", c);
		exit(EXIT_FAILURE);
	}
}

json_node *string_to_json(char *string)
{
	const char *p = string;
	json_node *root = parse_value(&p);
	skip_whitespace(&p);
	if (*p != '\0')
	{
		fprintf(stderr, "Extra characters after JSON value.\n");
		exit(EXIT_FAILURE);
	}
	return root;
}

/* --- JSON -> STRING --- */

typedef struct s_string_buffer
{
	char *data;
	size_t len;
	size_t capacity;
} StringBuffer;

static void sb_init(StringBuffer *sb)
{
	sb->capacity = 128;
	sb->len = 0;
	sb->data = malloc(sb->capacity);
	if (!sb->data)
	{
		fprintf(stderr, "Memory allocation failed for StringBuffer.\n");
		exit(EXIT_FAILURE);
	}
	sb->data[0] = '\0';
}

static void sb_append(StringBuffer *sb, const char *str)
{
	size_t str_len = strlen(str);
	while (sb->len + str_len + 1 > sb->capacity)
	{
		sb->capacity *= 2;
		sb->data = realloc(sb->data, sb->capacity);
		if (!sb->data)
		{
			fprintf(stderr, "Memory reallocation failed for StringBuffer.\n");
			exit(EXIT_FAILURE);
		}
	}
	memcpy(sb->data + sb->len, str, str_len + 1);
	sb->len += str_len;
}

static void sb_append_char(StringBuffer *sb, char c)
{
	if (sb->len + 2 > sb->capacity)
	{
		sb->capacity *= 2;
		sb->data = realloc(sb->data, sb->capacity);
		if (!sb->data)
		{
			fprintf(stderr, "Memory reallocation failed for StringBuffer.\n");
			exit(EXIT_FAILURE);
		}
	}
	sb->data[sb->len++] = c;
	sb->data[sb->len] = '\0';
}

static void sb_append_escaped(StringBuffer *sb, const char *str)
{
	sb_append_char(sb, '"');
	for (const char *p = str; *p; p++)
	{
		if (*p == '"' || *p == '\\')
		{
			sb_append_char(sb, '\\');
			sb_append_char(sb, *p);
		}
		else if (*p == '\n')
			sb_append(sb, "\\n");
		else if (*p == '\t')
			sb_append(sb, "\\t");
		else
			sb_append_char(sb, *p);
	}
	sb_append_char(sb, '"');
}

static void json_to_string_internal(json_node *node, StringBuffer *sb)
{
	char buffer[64];
	switch (node->type)
	{
		case JSON_TYPE_NULL:
			sb_append(sb, "null");
			break;
		case JSON_TYPE_NUMBER:
			snprintf(buffer, sizeof(buffer), "%g", node->number);
			sb_append(sb, buffer);
			break;
		case JSON_TYPE_STRING:
			sb_append_escaped(sb, node->string);
			break;
		case JSON_TYPE_OBJECT:
		{
			sb_append_char(sb, '{');
			if (node->array)
			{
				for (size_t i = 0; node->array[i] != NULL; i++)
				{
					if (i > 0)
						sb_append_char(sb, ',');
					sb_append_escaped(sb, node->array[i]->key);
					sb_append_char(sb, ':');
					json_to_string_internal(node->array[i], sb);
				}
			}
			sb_append_char(sb, '}');
			break;
		}
		case JSON_TYPE_ARRAY:
		{
			sb_append_char(sb, '[');
			if (node->array)
			{
				for (size_t i = 0; node->array[i] != NULL; i++)
				{
					if (i > 0)
						sb_append_char(sb, ',');
					json_to_string_internal(node->array[i], sb);
				}
			}
			sb_append_char(sb, ']');
			break;
		}
		default:
			break;
	}
}

char *json_to_string(json_node *json)
{
	StringBuffer sb;
	sb_init(&sb);
	json_to_string_internal(json, &sb);
	return sb.data;
}

static void sb_append_indent(StringBuffer *sb, int indent)
{
	for (int i = 0; i < indent; i++)
		sb_append(sb, "  ");
}

static void json_to_formatted_string_internal(json_node *node, StringBuffer *sb, int indent)
{
	char buffer[64];
	switch (node->type)
	{
		case JSON_TYPE_NULL:
			sb_append(sb, "null");
			break;
		case JSON_TYPE_NUMBER:
			snprintf(buffer, sizeof(buffer), "%g", node->number);
			sb_append(sb, buffer);
			break;
		case JSON_TYPE_STRING:
			sb_append_escaped(sb, node->string);
			break;
		case JSON_TYPE_OBJECT:
		{
			sb_append(sb, "{\n");
			if (node->array)
			{
				for (size_t i = 0; node->array[i] != NULL; i++)
				{
					sb_append_indent(sb, indent + 1);
					sb_append_escaped(sb, node->array[i]->key);
					sb_append(sb, ": ");
					json_to_formatted_string_internal(node->array[i], sb, indent + 1);
					if (node->array[i + 1] != NULL)
						sb_append(sb, ",");
					sb_append(sb, "\n");
				}
			}
			sb_append_indent(sb, indent);
			sb_append(sb, "}");
			break;
		}
		case JSON_TYPE_ARRAY:
		{
			sb_append(sb, "[\n");
			if (node->array)
			{
				for (size_t i = 0; node->array[i] != NULL; i++)
				{
					sb_append_indent(sb, indent + 1);
					json_to_formatted_string_internal(node->array[i], sb, indent + 1);
					if (node->array[i + 1] != NULL)
						sb_append(sb, ",");
					sb_append(sb, "\n");
				}
			}
			sb_append_indent(sb, indent);
			sb_append(sb, "]");
			break;
		}
		default:
			break;
	}
}

char *json_to_formatted_string(json_node *json)
{
	StringBuffer sb;
	sb_init(&sb);
	json_to_formatted_string_internal(json, &sb, 0);
	return sb.data;
}

/* --- JSON SEARCH --- */

json_node *json_find_recursive(json_node *json, char *key)
{
	if (!json)
		return NULL;
	if (json->key && strcmp(json->key, key) == 0)
		return json;
	if (json->type == JSON_TYPE_OBJECT && json->array)
	{
		for (size_t i = 0; json->array[i] != NULL; i++)
		{
			json_node *found = json_find(json->array[i], key);
			if (found)
				return found;
		}
	}
	else if (json->type == JSON_TYPE_ARRAY && json->array)
	{
		for (size_t i = 0; json->array[i] != NULL; i++)
		{
			json_node *found = json_find(json->array[i], key);
			if (found)
				return found;
		}
	}
	return NULL;
}

json_node *json_find(json_node *json, char *key)
{
	if (!json)
		return NULL;
	if (json->type == JSON_TYPE_OBJECT && json->array)
	{
		for (size_t i = 0; json->array[i] != NULL; i++)
		{
			if (json->array[i]->key && strcmp(json->array[i]->key, key) == 0)
				return json->array[i];
		}
	}
	else if (json->type == JSON_TYPE_ARRAY && json->array)
	{
		for (size_t i = 0; json->array[i] != NULL; i++)
		{
			if (json->array[i]->key && strcmp(json->array[i]->key, key) == 0)
				return json->array[i];
		}
	}
	return NULL;
}

/* --- FREE MEMORY --- */

void free_json(json_node *json)
{
	if (!json)
		return;
	if (json->key)
		free(json->key);
	
	switch (json->type)
	{
		case JSON_TYPE_STRING:
			if (json->string)
				free(json->string);
			break;
		case JSON_TYPE_OBJECT:
			if (json->array)
			{
				for (size_t i = 0; json->array[i] != NULL; i++)
					free_json(json->array[i]);
				free(json->array);
			}
			break;
		case JSON_TYPE_ARRAY:
			if (json->array)
			{
				for (size_t i = 0; json->array[i] != NULL; i++)
					free_json(json->array[i]);
				free(json->array);
			}
			break;
		default:
			break;
	}
	free(json);
}
