#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse_json.h"

char* json_formatter(const char* json_input)
{
	int len = strlen(json_input);
	int max_output_len = len * 2; // Estimate output size
	char *output = (char*)malloc(max_output_len);
	if (output == NULL) return NULL;

	int indent_level = 0;
	int in_string = 0;
	int i, j = 0;

	for (i = 0; i < len; i++) {
		char ch = json_input[i];

		if (ch == '\"') {
			// Check for escaped quotes
			int escape = 0;
			int k = i - 1;
			while (k >= 0 && json_input[k] == '\\') {
				escape = !escape;
				k--;
			}
			if (!escape) in_string = !in_string;
			output[j++] = ch;
		} else if (!in_string) {
			if (ch == '{' || ch == '[') {
				output[j++] = ch;
				output[j++] = '\n';
				indent_level++;
				for (int l = 0; l < indent_level; l++) output[j++] = '\t';
			} else if (ch == '}' || ch == ']') {
				output[j++] = '\n';
				indent_level--;
				for (int l = 0; l < indent_level; l++) output[j++] = '\t';
				output[j++] = ch;
			} else if (ch == ',') {
				output[j++] = ch;
				output[j++] = '\n';
				for (int l = 0; l < indent_level; l++) output[j++] = '\t';
			} else if (ch == ':') {
				output[j++] = ch;
				output[j++] = ' ';
			} else if (ch != ' ' && ch != '\n' && ch != '\t') {
				output[j++] = ch;
			}
		} else {
			output[j++] = ch;
		}

		// Resize output buffer if needed
		if (j >= max_output_len - 1) {
			max_output_len *= 2;
			char* temp = (char*)realloc(output, max_output_len);
			if (temp == NULL) {
				free(output);
				return NULL;
			}
			output = temp;
		}
	}
	output[j] = '\0';
	return output;
}
