/*
* Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
*
* Licensed under the Apache License, Version 2.0 (the License);
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an AS IS BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <glib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <glib-unix.h>
#include <glib-object.h>

#include "libservice_plugin_log.h"
#include <plugin_message.h>
#include <json-glib/json-glib.h>

/*	For debug */
/*#define CONSOLE_MODE */

#ifdef CONSOLE_MODE
#define PRINT_LOG printf
#else
#define PRINT_LOG LSP_LOG_debug
#endif

#define FUNC_START() do {PRINT_LOG("Start >>%s>>\n", __FUNCTION__); } while (0)
#define FUNC_END() do {PRINT_LOG("End <<%s<<\n", __FUNCTION__); } while (0)


#define PLUGIN_MESSAGE_ELEMENT_KEY_CONTEXT_ID		"ctx_id"
#define PLUGIN_MESSAGE_ELEMENT_KEY_FUNCTION_NAME	"func_id"
#define PLUGIN_MESSAGE_ELEMENT_KEY_PARAMETER_MANDATORY	"man_param"
#define PLUGIN_MESSAGE_ELEMENT_KEY_PARAMETER_OPTIONAL	"opt_param"
#define PLUGIN_MESSAGE_ELEMENT_KEY_REQUEST_ID		"req_id"
#define PLUGIN_MESSAGE_ELEMENT_KEY_MESSAGE_TYPE		"msg_id"
#define PLUGIN_MESSAGE_ELEMENT_KEY_RESULT_CODE		"rcode"
#define PLUGIN_MESSAGE_ELEMENT_KEY_RESULT_MESSAGE	"rmsg"


#define SUCCESS 0
#define FAIL -1

#define PLUGIN_PARAMETER_BUF_SIZE_MAX 10

#define JSON_EMPTY_NODE_FILL_NULL(node)		do {if (0 == json_node_get_value_type((node))) { \
							json_node_free((node)); \
							(node) = NULL; \
							(node) = json_node_new(JSON_NODE_NULL); } } while (0)


struct _plugin_message_s {
	JsonNode *context;
	JsonNode *function;
	JsonNode *parameter_mandatory;
	JsonNode *parameter_optional;
	JsonNode *request_id;
	JsonNode *message_type;
	JsonNode *rcode;
	JsonNode *rmsg;
};

struct _plugin_message_array_s {
	JsonArray *element;
	plugin_data_type *types;
};

int plugin_message_create(plugin_message_h *message)
{
	FUNC_START();
	if (NULL != message) {
		plugin_message_h _msg = (plugin_message_h) calloc(1, sizeof(struct _plugin_message_s));
		if (NULL != _msg) {
			_msg->context			= json_node_new(JSON_NODE_VALUE);
			_msg->function			= json_node_new(JSON_NODE_VALUE);
			_msg->parameter_mandatory	= json_node_new(JSON_NODE_OBJECT);
			_msg->parameter_optional	= json_node_new(JSON_NODE_OBJECT);
			_msg->request_id		= json_node_new(JSON_NODE_VALUE);
			_msg->message_type		= json_node_new(JSON_NODE_VALUE);
			_msg->rcode			= json_node_new(JSON_NODE_VALUE);
			_msg->rmsg			= json_node_new(JSON_NODE_VALUE);

			json_node_set_object(_msg->parameter_mandatory, json_object_new());
			json_node_set_object(_msg->parameter_optional, json_object_new());

			*message = _msg;

			FUNC_END();
			return SUCCESS;
		}
		FUNC_END();
	}
	return FAIL;
}

void plugin_message_destroy(plugin_message_h message)
{
	FUNC_START();
	if (NULL != message) {
		json_node_free(message->context);
		json_node_free(message->function);
		json_node_free(message->parameter_mandatory);
		json_node_free(message->parameter_optional);
		json_node_free(message->request_id);
		json_node_free(message->message_type);
		json_node_free(message->rcode);
		json_node_free(message->rmsg);

		free(message);
	}
	FUNC_END();
}

/*
int plugin_message_set_value_int(plugin_message_h message, plugin_message_element_e field, int value)
{
	if (NULL == message)
	{
		return FAIL;
	}

	switch (field)
	{
	case PLUGIN_MESSAGE_ELEMENT_CONTEXT_ID:
		json_node_set_int(message->context, (gint64)value);
		break;
	case PLUGIN_MESSAGE_ELEMENT_FUNCTION_NAME:
		json_node_set_int(message->function, (gint64)value);
		break;
	case PLUGIN_MESSAGE_ELEMENT_REQUEST_ID:
		json_node_set_int(message->request_id, (gint64)value);
		break;
	case PLUGIN_MESSAGE_ELEMENT_MESSAGE_TYPE:
		json_node_set_int(message->message_type, (gint64)value);
		break;
	case PLUGIN_MESSAGE_ELEMENT_RESULT_CODE:
		json_node_set_int(message->rcode, (gint64)value);
		break;
	case PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE:
		json_node_set_int(message->rmsg, (gint64)value);
		break;
	default:
		return FAIL;
	}

	return SUCCESS;
}

int plugin_message_get_value_int(plugin_message_h message, plugin_message_element_e field, int *value)
{
	if ((NULL == message) || (NULL == value))
	{
		return FAIL;
	}

	switch (field)
	{
	case PLUGIN_MESSAGE_ELEMENT_CONTEXT_ID:
		*value = (int)json_node_get_int(message->context);
		break;
	case PLUGIN_MESSAGE_ELEMENT_FUNCTION_NAME:
		*value = (int)json_node_get_int(message->function);
		break;
	case PLUGIN_MESSAGE_ELEMENT_REQUEST_ID:
		*value = (int)json_node_get_int(message->request_id);
		break;
	case PLUGIN_MESSAGE_ELEMENT_MESSAGE_TYPE:
		*value = (int)json_node_get_int(message->message_type);
		break;
	case PLUGIN_MESSAGE_ELEMENT_RESULT_CODE:
		*value = (int)json_node_get_int(message->rcode);
		break;
	case PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE:
		*value = (int)json_node_get_int(message->rmsg);
		break;
	default:
		return FAIL;
	}

	return SUCCESS;
}
*/

int plugin_message_set_value_number(plugin_message_h message, plugin_message_element_e field, pmnumber value)
{
	FUNC_START();
	if (NULL == message) {
		return FAIL;
	}

	switch (field) {
	case PLUGIN_MESSAGE_ELEMENT_CONTEXT_ID:
		json_node_set_int(message->context, (gint64)value);
		break;
	case PLUGIN_MESSAGE_ELEMENT_FUNCTION_NAME:
		json_node_set_int(message->function, (gint64)value);
		break;
	case PLUGIN_MESSAGE_ELEMENT_REQUEST_ID:
		json_node_set_int(message->request_id, (gint64)value);
		break;
	case PLUGIN_MESSAGE_ELEMENT_MESSAGE_TYPE:
		json_node_set_int(message->message_type, (gint64)value);
		break;
	case PLUGIN_MESSAGE_ELEMENT_RESULT_CODE:
		json_node_set_int(message->rcode, (gint64)value);
		break;
	case PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE:
		json_node_set_int(message->rmsg, (gint64)value);
		break;
	default:
		return FAIL;
	}

	FUNC_END();
	return SUCCESS;
}

int plugin_message_get_value_number(plugin_message_h message, plugin_message_element_e field, pmnumber *value)
{
	FUNC_START();
	if ((NULL == message) || (NULL == value)) {
		return FAIL;
	}

	switch (field) {
	case PLUGIN_MESSAGE_ELEMENT_CONTEXT_ID:
		*value = (long long int)json_node_get_int(message->context);
		break;
	case PLUGIN_MESSAGE_ELEMENT_FUNCTION_NAME:
		*value = (long long int)json_node_get_int(message->function);
		break;
	case PLUGIN_MESSAGE_ELEMENT_REQUEST_ID:
		*value = (long long int)json_node_get_int(message->request_id);
		break;
	case PLUGIN_MESSAGE_ELEMENT_MESSAGE_TYPE:
		*value = (long long int)json_node_get_int(message->message_type);
		break;
	case PLUGIN_MESSAGE_ELEMENT_RESULT_CODE:
		*value = (long long int)json_node_get_int(message->rcode);
		break;
	case PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE:
		*value = (long long int)json_node_get_int(message->rmsg);
		break;
	default:
		return FAIL;
	}

	FUNC_END();
	return SUCCESS;
}

int plugin_message_set_value_string(plugin_message_h message, plugin_message_element_e field, const char *value)
{
	FUNC_START();
	if (NULL == message) {
		return FAIL;
	}

	switch (field) {
	case PLUGIN_MESSAGE_ELEMENT_CONTEXT_ID:
		json_node_set_string(message->context, value);
		break;
	case PLUGIN_MESSAGE_ELEMENT_FUNCTION_NAME:
		json_node_set_string(message->function, value);
		break;
	case PLUGIN_MESSAGE_ELEMENT_REQUEST_ID:
		json_node_set_string(message->request_id, value);
		break;
	case PLUGIN_MESSAGE_ELEMENT_MESSAGE_TYPE:
		json_node_set_string(message->message_type, value);
		break;
	case PLUGIN_MESSAGE_ELEMENT_RESULT_CODE:
		json_node_set_string(message->rcode, value);
		break;
	case PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE:
		json_node_set_string(message->rmsg, value);
		break;
	default:
		return FAIL;
	}

	FUNC_END();
	return SUCCESS;
}

int plugin_message_get_value_string(plugin_message_h message, plugin_message_element_e field, char **value)
{
	FUNC_START();
	if ((NULL == message) || (NULL == value)) {
		return FAIL;
	}

	switch (field) {
	case PLUGIN_MESSAGE_ELEMENT_CONTEXT_ID:
		*value = (char *)json_node_dup_string(message->context);
		break;
	case PLUGIN_MESSAGE_ELEMENT_FUNCTION_NAME:
		*value = (char *)json_node_dup_string(message->function);
		break;
	case PLUGIN_MESSAGE_ELEMENT_REQUEST_ID:
		*value = (char *)json_node_dup_string(message->request_id);
		break;
	case PLUGIN_MESSAGE_ELEMENT_MESSAGE_TYPE:
		*value = (char *)json_node_dup_string(message->message_type);
		break;
	case PLUGIN_MESSAGE_ELEMENT_RESULT_CODE:
		*value = (char *)json_node_dup_string(message->rcode);
		break;
	case PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE:
		*value = (char *)json_node_dup_string(message->rmsg);
		break;
	default:
		return FAIL;
	}

	FUNC_END();
	return SUCCESS;
}

int plugin_message_set_value_bool(plugin_message_h message, plugin_message_element_e field, bool value)
{
	FUNC_START();
	if (NULL == message) {
		return FAIL;
	}

	switch (field) {
	case PLUGIN_MESSAGE_ELEMENT_CONTEXT_ID:
		json_node_set_boolean(message->context, value);
		break;
	case PLUGIN_MESSAGE_ELEMENT_FUNCTION_NAME:
		json_node_set_boolean(message->function, value);
		break;
	case PLUGIN_MESSAGE_ELEMENT_REQUEST_ID:
		json_node_set_boolean(message->request_id, value);
		break;
	case PLUGIN_MESSAGE_ELEMENT_MESSAGE_TYPE:
		json_node_set_boolean(message->message_type, value);
		break;
	case PLUGIN_MESSAGE_ELEMENT_RESULT_CODE:
		json_node_set_boolean(message->rcode, value);
		break;
	case PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE:
		json_node_set_boolean(message->rmsg, value);
		break;
	default:
		return FAIL;
	}

	FUNC_END();
	return SUCCESS;
}

int plugin_message_get_value_bool(plugin_message_h message, plugin_message_element_e field, bool *value)
{
	FUNC_START();
	if ((NULL == message) || (NULL == value)) {
		return FAIL;
	}

	switch (field) {
	case PLUGIN_MESSAGE_ELEMENT_CONTEXT_ID:
		*value = (bool)json_node_get_boolean(message->context);
		break;
	case PLUGIN_MESSAGE_ELEMENT_FUNCTION_NAME:
		*value = (bool)json_node_get_boolean(message->function);
		break;
	case PLUGIN_MESSAGE_ELEMENT_REQUEST_ID:
		*value = (bool)json_node_get_boolean(message->request_id);
		break;
	case PLUGIN_MESSAGE_ELEMENT_MESSAGE_TYPE:
		*value = (bool)json_node_get_boolean(message->message_type);
		break;
	case PLUGIN_MESSAGE_ELEMENT_RESULT_CODE:
		*value = (bool)json_node_get_boolean(message->rcode);
		break;
	case PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE:
		*value = (bool)json_node_get_boolean(message->rmsg);
		break;
	default:
		return FAIL;
	}

	FUNC_END();
	return SUCCESS;
}

/*
int _plugin_message_set_parameter_value_int(JsonNode *node, int param_index, int value)
{
	char index[PLUGIN_PARAMETER_BUF_SIZE_MAX+1] = {0, };
	if (0 > param_index)
	{
		return FAIL;
	}
	snprintf(index, PLUGIN_PARAMETER_BUF_SIZE_MAX, "%d", param_index);

	JsonObject *params;
	params = json_node_get_object(node);

	// release memory of legacy node

//	JsonNode *legacy_node = json_object_get_member(params, index);
//	if (NULL != legacy_node)
//	{
//		json_node_free(legacy_node);
//	}

	JsonNode *new_node = json_node_new(JSON_NODE_VALUE);
	json_node_set_int(new_node, (gint64)value);
	json_object_set_member(params, index, new_node);

	return SUCCESS;
}

int _plugin_message_get_parameter_value_int(JsonNode *node, int param_index, int *value)
{
	char index[PLUGIN_PARAMETER_BUF_SIZE_MAX+1] = {0, };
	if (0 > param_index)
	{
		return FAIL;
	}
	snprintf(index, PLUGIN_PARAMETER_BUF_SIZE_MAX, "%d", param_index);

	JsonObject *params;
	params = json_node_get_object(node);

	JsonNode *legacy_node = json_object_get_member(params, index);
	if (NULL != legacy_node)
	{
		*value = (int)json_node_get_int(legacy_node);
	}
	else
	{
		return FAIL;
	}

	return SUCCESS;
}
*/

int _plugin_message_set_parameter_value_number(JsonNode *node, int param_index, pmnumber value)
{
	FUNC_START();
	char index[PLUGIN_PARAMETER_BUF_SIZE_MAX+1] = {0, };
	if (0 > param_index) {
		return FAIL;
	}
	snprintf(index, PLUGIN_PARAMETER_BUF_SIZE_MAX, "%d", param_index);

	JsonObject *params;
	params = json_node_get_object(node);

	/* release memory of legacy node */
/*
	JsonNode *legacy_node = json_object_get_member(params, index);
	if (NULL != legacy_node)
	{
		json_node_free(legacy_node);
	}
*/
	JsonNode *new_node = json_node_new(JSON_NODE_VALUE);
	json_node_set_int(new_node, (gint64)value);
	json_object_set_member(params, index, new_node);

	FUNC_END();
	return SUCCESS;
}

int _plugin_message_get_parameter_value_number(JsonNode *node, int param_index, pmnumber *value)
{
	FUNC_START();
	char index[PLUGIN_PARAMETER_BUF_SIZE_MAX+1] = {0, };
	if (0 > param_index) {
		return FAIL;
	}
	snprintf(index, PLUGIN_PARAMETER_BUF_SIZE_MAX, "%d", param_index);

	JsonObject *params;
	params = json_node_get_object(node);

	JsonNode *legacy_node = json_object_get_member(params, index);
	if (NULL != legacy_node) {
		*value = (long long int)json_node_get_int(legacy_node);
	} else {
		return FAIL;
	}

	FUNC_END();
	return SUCCESS;
}

int _plugin_message_set_parameter_value_string(JsonNode *node, int param_index, const char *value)
{
	FUNC_START();
	char index[PLUGIN_PARAMETER_BUF_SIZE_MAX+1] = {0, };
	if (0 > param_index) {
		return FAIL;
	}
	snprintf(index, PLUGIN_PARAMETER_BUF_SIZE_MAX, "%d", param_index);

	JsonObject *params;
	params = json_node_get_object(node);

	/* release memory of legacy node */
/*
	JsonNode *legacy_node = json_object_get_member(params, index);
	if (NULL != legacy_node)
	{
		json_node_free(legacy_node);
	}
*/
	JsonNode *new_node = json_node_new(JSON_NODE_VALUE);
	json_node_set_string(new_node, value);
	json_object_set_member(params, index, new_node);

	FUNC_END();
	return SUCCESS;
}

int _plugin_message_get_parameter_value_string(JsonNode *node, int param_index, char **value)
{
	FUNC_START();
	char index[PLUGIN_PARAMETER_BUF_SIZE_MAX+1] = {0, };
	if (0 > param_index) {
		return FAIL;
	}
	snprintf(index, PLUGIN_PARAMETER_BUF_SIZE_MAX, "%d", param_index);

	JsonObject *params;
	params = json_node_get_object(node);

	JsonNode *legacy_node = json_object_get_member(params, index);
	if (NULL != legacy_node) {
		*value = (char *)json_node_dup_string(legacy_node);
	} else {
		return FAIL;
	}

	FUNC_END();
	return SUCCESS;
}
int _plugin_message_set_parameter_value_bool(JsonNode *node, int param_index, bool value)
{
	FUNC_START();
	char index[PLUGIN_PARAMETER_BUF_SIZE_MAX+1] = {0, };
	if (0 > param_index) {
		return FAIL;
	}
	snprintf(index, PLUGIN_PARAMETER_BUF_SIZE_MAX, "%d", param_index);

	JsonObject *params;
	params = json_node_get_object(node);

	/* release memory of legacy node */
/*
	JsonNode *legacy_node = json_object_get_member(params, index);
	if (NULL != legacy_node)
	{
		json_node_free(legacy_node);
	}
*/
	JsonNode *new_node = json_node_new(JSON_NODE_VALUE);
	json_node_set_boolean(new_node, value);
	json_object_set_member(params, index, new_node);

	FUNC_END();
	return SUCCESS;
}

int _plugin_message_get_parameter_value_bool(JsonNode *node, int param_index, bool *value)
{
	FUNC_START();
	char index[PLUGIN_PARAMETER_BUF_SIZE_MAX+1] = {0, };
	if (0 > param_index) {
		return FAIL;
	}
	snprintf(index, PLUGIN_PARAMETER_BUF_SIZE_MAX, "%d", param_index);

	JsonObject *params;
	params = json_node_get_object(node);

	JsonNode *legacy_node = json_object_get_member(params, index);
	if (NULL != legacy_node) {
		*value = (bool)json_node_get_boolean(legacy_node);
	} else {
		return FAIL;
	}

	FUNC_END();
	return SUCCESS;
}

int _plugin_message_set_parameter_value_array(JsonNode *node, int param_index, plugin_message_array_h value)
{
	FUNC_START();
	char index[PLUGIN_PARAMETER_BUF_SIZE_MAX+1] = {0, };
	if (0 > param_index) {
		return FAIL;
	}
	snprintf(index, PLUGIN_PARAMETER_BUF_SIZE_MAX, "%d", param_index);

	JsonObject *params;
	params = json_node_get_object(node);

	/* release memory of legacy node */
/*
	JsonNode *legacy_node = json_object_get_member(params, index);
	if (NULL != legacy_node)
	{
		json_node_free(legacy_node);
	}
*/
	JsonNode *new_node = json_node_new(JSON_NODE_ARRAY);
	json_node_set_array(new_node, value->element);
	json_object_set_member(params, index, new_node);

	FUNC_END();
	return SUCCESS;
}

int _plugin_message_get_parameter_value_array(JsonNode *node, int param_index, plugin_message_array_h *value)
{
	FUNC_START();
	char index[PLUGIN_PARAMETER_BUF_SIZE_MAX+1] = {0, };
	if (0 > param_index) {
		return FAIL;
	}
	snprintf(index, PLUGIN_PARAMETER_BUF_SIZE_MAX, "%d", param_index);

	JsonObject *params;
	params = json_node_get_object(node);


	JsonNode *legacy_node = json_object_get_member(params, index);
	if ((NULL != legacy_node) && (JSON_NODE_ARRAY == json_node_get_node_type(legacy_node))) {
		JsonArray *j_array = NULL;
		PRINT_LOG("duplacate json array\n");
		j_array = json_node_dup_array(legacy_node);

		if (NULL != j_array) {
			plugin_message_array_h _array = (plugin_message_array_h) calloc(1, sizeof(struct _plugin_message_array_s));
			if (NULL == _array) {
				PRINT_LOG("Memory allocation failed\n");
				return FAIL;
			}

			_array->element = j_array;

			PRINT_LOG("get object from json array\n");
			JsonObject *job = json_array_get_object_element(j_array, 0);

			int len = (int) json_object_get_size(job);

			PRINT_LOG("json object length : %d\n", len);
			_array->types = (plugin_data_type *) calloc((len + 1), sizeof(plugin_data_type));
			if (NULL == _array->types) {
				PRINT_LOG("Memory allocation failed\n");
				free(_array);
				return FAIL;
			}

			int i;
			JsonNode *iter_node;
			char idx[PLUGIN_PARAMETER_BUF_SIZE_MAX];

			for (i = 0; i < len; i++) {
				snprintf(idx, PLUGIN_PARAMETER_BUF_SIZE_MAX, "%d", i+1);

				iter_node = json_object_get_member(job, idx);
				if (JSON_NODE_VALUE == json_node_get_node_type(iter_node)) {
					GType value_type = json_node_get_value_type(iter_node);
					if (G_TYPE_INT64 == value_type) {
						PRINT_LOG("node type (long == int)\n");
						_array->types[i] = PLUGIN_DATA_TYPE_NUM;
					} else if (G_TYPE_BOOLEAN == value_type) {
						PRINT_LOG("node type (bool)\n");
						_array->types[i] = PLUGIN_DATA_TYPE_BOOL;
					} else if (G_TYPE_STRING == value_type) {
						PRINT_LOG("node type (string)\n");
						_array->types[i] = PLUGIN_DATA_TYPE_STRING;
					} else {
						PRINT_LOG("node type (unknown)\n");
						_array->types[i] = PLUGIN_DATA_TYPE_UNKNOWN;
					}
				} else {
					PRINT_LOG("node type (no value node)\n");
					_array->types[i] = PLUGIN_DATA_TYPE_UNKNOWN;
				}
			}

			*value = _array;

			FUNC_END();
			return SUCCESS;
		}
	}

	FUNC_END();
	return FAIL;
}
/*
int plugin_message_set_param_int(plugin_message_h message, int param_index, int value)
{
	if (NULL == message)
	{
		return FAIL;
	}

	return _plugin_message_set_parameter_value_int(message->parameter_mandatory, param_index, value);
}

int plugin_message_get_param_int(plugin_message_h message, int param_index, int *value)
{
	if ((NULL == message) || (NULL == value))
	{
		return FAIL;
	}

	return _plugin_message_get_parameter_value_int(message->parameter_mandatory, param_index, value);
}
*/

int plugin_message_set_param_number(plugin_message_h message, int param_index, pmnumber value)
{
	FUNC_START();
	if (NULL == message) {
		return FAIL;
	}

	FUNC_END();
	return _plugin_message_set_parameter_value_number(message->parameter_mandatory, param_index, value);
}

int plugin_message_get_param_number(plugin_message_h message, int param_index, pmnumber *value)
{
	FUNC_START();
	if ((NULL == message) || (NULL == value)) {
		return FAIL;
	}

	FUNC_END();
	return _plugin_message_get_parameter_value_number(message->parameter_mandatory, param_index, value);
}

int plugin_message_set_param_string(plugin_message_h message, int param_index, const char *value)
{
	FUNC_START();
	if ((NULL == message) || (NULL == value)) {
		return FAIL;
	}

	FUNC_END();
	return _plugin_message_set_parameter_value_string(message->parameter_mandatory, param_index, value);
}

int plugin_message_get_param_string(plugin_message_h message, int param_index, char **value)
{
	FUNC_START();
	if ((NULL == message) || (NULL == value)) {
		return FAIL;
	}

	FUNC_END();
	return _plugin_message_get_parameter_value_string(message->parameter_mandatory, param_index, value);
}

int plugin_message_set_param_bool(plugin_message_h message, int param_index, bool value)
{
	FUNC_START();
	if (NULL == message) {
		return FAIL;
	}

	FUNC_END();
	return _plugin_message_set_parameter_value_bool(message->parameter_mandatory, param_index, value);
}

int plugin_message_get_param_bool(plugin_message_h message, int param_index, bool *value)
{
	FUNC_START();
	if ((NULL == message) || (NULL == value)) {
		return FAIL;
	}

	FUNC_END();
	return _plugin_message_get_parameter_value_bool(message->parameter_mandatory, param_index, value);
}

int plugin_message_set_param_array(plugin_message_h message, int param_index, plugin_message_array_h value)
{
	FUNC_START();
	if ((NULL == message) || (NULL == value)) {
		return FAIL;
	}

	FUNC_END();
	return _plugin_message_set_parameter_value_array(message->parameter_mandatory, param_index, value);
}

int plugin_message_get_param_array(plugin_message_h message, int param_index, plugin_message_array_h *value)
{
	FUNC_START();
	if ((NULL == message) || (NULL == value)) {
		return FAIL;
	}

	FUNC_END();
	return _plugin_message_get_parameter_value_array(message->parameter_mandatory, param_index, value);
}

/*
int plugin_message_set_opt_param_int(plugin_message_h message, int param_index, int value)
{
	FUNC_START();
	if (NULL == message)
	{
		return FAIL;
	}

	FUNC_END();
	return _plugin_message_set_parameter_value_int(message->parameter_optional, param_index, value);
}

int plugin_message_get_opt_param_int(plugin_message_h message, int param_index, int *value)
{
	FUNC_START();
	if ((NULL == message) || (NULL == value))
	{
		return FAIL;
	}

	FUNC_END();
	return _plugin_message_get_parameter_value_int(message->parameter_optional, param_index, value);
}
*/

int plugin_message_set_opt_param_number(plugin_message_h message, int param_index, pmnumber value)
{
	FUNC_START();
	if (NULL == message) {
		return FAIL;
	}

	FUNC_END();
	return _plugin_message_set_parameter_value_number(message->parameter_optional, param_index, value);
}

int plugin_message_get_opt_param_number(plugin_message_h message, int param_index, pmnumber *value)
{
	FUNC_START();
	if ((NULL == message) || (NULL == value)) {
		return FAIL;
	}

	FUNC_END();
	return _plugin_message_get_parameter_value_number(message->parameter_optional, param_index, value);
}

int plugin_message_set_opt_param_string(plugin_message_h message, int param_index, const char *value)
{
	FUNC_START();
	if ((NULL == message) || (NULL == value)) {
		return FAIL;
	}

	FUNC_END();
	return _plugin_message_set_parameter_value_string(message->parameter_optional, param_index, value);
}

int plugin_message_get_opt_param_string(plugin_message_h message, int param_index, char **value)
{
	FUNC_START();
	if ((NULL == message) || (NULL == value)) {
		return FAIL;
	}

	FUNC_END();
	return _plugin_message_get_parameter_value_string(message->parameter_optional, param_index, value);
}

int plugin_message_set_opt_param_bool(plugin_message_h message, int param_index, bool value)
{
	FUNC_START();
	if (NULL == message) {
		return FAIL;
	}

	FUNC_END();
	return _plugin_message_set_parameter_value_bool(message->parameter_optional, param_index, value);
}

int plugin_message_get_opt_param_bool(plugin_message_h message, int param_index, bool *value)
{
	FUNC_START();
	if ((NULL == message) || (NULL == value)) {
		return FAIL;
	}

	FUNC_END();
	return _plugin_message_get_parameter_value_bool(message->parameter_optional, param_index, value);
}

int plugin_message_set_opt_param_array(plugin_message_h message, int param_index, plugin_message_array_h value)
{
	FUNC_START();
	if ((NULL == message) || (NULL == value)) {
		return FAIL;
	}

	FUNC_END();
	return _plugin_message_set_parameter_value_array(message->parameter_optional, param_index, value);
}

int plugin_message_get_opt_param_array(plugin_message_h message, int param_index, plugin_message_array_h *value)
{
	FUNC_START();
	if ((NULL == message) || (NULL == value)) {
		return FAIL;
	}

	FUNC_END();
	return _plugin_message_get_parameter_value_array(message->parameter_optional, param_index, value);
}

char *_json_serialize_by_jnode(JsonNode *total_node)
{
	FUNC_START();
	JsonGenerator *gen = json_generator_new();
	json_generator_set_root(gen, total_node);

	char *t_data = NULL;
	gsize len = 0;
	t_data = json_generator_to_data(gen, &len);

	g_object_unref(gen);

	FUNC_END();
	return t_data;
/*	return t; */
}

JsonNode *_json_deserialize_by_data(const char *data)
{
	FUNC_START();
	JsonParser *parser = json_parser_new();
	json_parser_load_from_data(parser, data, strlen(data), NULL);

	PRINT_LOG("next\n");

	JsonNode *node = json_parser_get_root(parser);

	JsonNodeType nt = json_node_get_node_type(node);
	if (nt == JSON_NODE_OBJECT)
		PRINT_LOG("object type\n");
	if (nt == JSON_NODE_ARRAY)
		PRINT_LOG("array type\n");
	if (nt == JSON_NODE_VALUE)
		PRINT_LOG("value type\n");
	if (nt == JSON_NODE_NULL)
		PRINT_LOG("null type\n");

	FUNC_END();
	return node;
}

int plugin_message_serialize(plugin_message_h message, char **data)
{
	FUNC_START();
	if ((NULL == message) || (NULL == data)) {
		return FAIL;
	}

	JsonObject *obj = json_object_new();

	JsonNode *context_id = json_node_copy(message->context);
	JsonNode *function_name = json_node_copy(message->function);
	JsonNode *parameter_mandatory = json_node_copy(message->parameter_mandatory);
	JsonNode *parameter_optional = json_node_copy(message->parameter_optional);
	JsonNode *request_id = json_node_copy(message->request_id);
	JsonNode *message_type = json_node_copy(message->message_type);
	JsonNode *rcode = json_node_copy(message->rcode);
	JsonNode *rmsg = json_node_copy(message->rmsg);

	JSON_EMPTY_NODE_FILL_NULL(context_id);
	JSON_EMPTY_NODE_FILL_NULL(function_name);
	JSON_EMPTY_NODE_FILL_NULL(parameter_mandatory);
	JSON_EMPTY_NODE_FILL_NULL(parameter_optional);
	JSON_EMPTY_NODE_FILL_NULL(request_id);
	JSON_EMPTY_NODE_FILL_NULL(message_type);
	JSON_EMPTY_NODE_FILL_NULL(rcode);
	JSON_EMPTY_NODE_FILL_NULL(rmsg);

	json_object_set_member(obj, PLUGIN_MESSAGE_ELEMENT_KEY_CONTEXT_ID, context_id);
	json_object_set_member(obj, PLUGIN_MESSAGE_ELEMENT_KEY_FUNCTION_NAME, function_name);
	json_object_set_member(obj, PLUGIN_MESSAGE_ELEMENT_KEY_PARAMETER_MANDATORY, parameter_mandatory);
	json_object_set_member(obj, PLUGIN_MESSAGE_ELEMENT_KEY_PARAMETER_OPTIONAL, parameter_optional);
	json_object_set_member(obj, PLUGIN_MESSAGE_ELEMENT_KEY_REQUEST_ID, request_id);
	json_object_set_member(obj, PLUGIN_MESSAGE_ELEMENT_KEY_MESSAGE_TYPE, message_type);
	json_object_set_member(obj, PLUGIN_MESSAGE_ELEMENT_KEY_RESULT_CODE, rcode);
	json_object_set_member(obj, PLUGIN_MESSAGE_ELEMENT_KEY_RESULT_MESSAGE, rmsg);

	JsonNode *node = json_node_new(JSON_NODE_OBJECT);
	json_node_take_object(node, obj);

	char *_data = NULL;
	_data = _json_serialize_by_jnode(node);
/*
	json_node_free(node);
	json_object_unref(obj);

	json_node_free(context_id);
	json_node_free(function_name);
	json_node_free(parameter_mandatory);
	json_node_free(parameter_optional);
	json_node_free(request_id);
	json_node_free(message_type);
	json_node_free(rcode);
	json_node_free(rmsg);
*/
	*data = _data;
	FUNC_END();
	return SUCCESS;
}

int plugin_message_deserialize(const char *data, plugin_message_h *message)
{
	FUNC_START();
	if ((NULL == message) || (NULL == data)) {
		return FAIL;
	}

	plugin_message_h _msg = (plugin_message_h) calloc(1, sizeof(struct _plugin_message_s));
	if (NULL == _msg) {
		return FAIL;
	}

	JsonNode *root_node = _json_deserialize_by_data(data);

	JsonObject *root_object = json_node_get_object(root_node);

	_msg->context			= json_object_dup_member(root_object, PLUGIN_MESSAGE_ELEMENT_KEY_CONTEXT_ID);
	_msg->function			= json_object_dup_member(root_object, PLUGIN_MESSAGE_ELEMENT_KEY_FUNCTION_NAME);
	_msg->parameter_mandatory	= json_object_dup_member(root_object, PLUGIN_MESSAGE_ELEMENT_KEY_PARAMETER_MANDATORY);
	_msg->parameter_optional	= json_object_dup_member(root_object, PLUGIN_MESSAGE_ELEMENT_KEY_PARAMETER_OPTIONAL);
	_msg->request_id		= json_object_dup_member(root_object, PLUGIN_MESSAGE_ELEMENT_KEY_REQUEST_ID);
	_msg->message_type		= json_object_dup_member(root_object, PLUGIN_MESSAGE_ELEMENT_KEY_MESSAGE_TYPE);
	_msg->rcode			= json_object_dup_member(root_object, PLUGIN_MESSAGE_ELEMENT_KEY_RESULT_CODE);
	_msg->rmsg			= json_object_dup_member(root_object, PLUGIN_MESSAGE_ELEMENT_KEY_RESULT_MESSAGE);

	*message = _msg;

	json_node_free(root_node);

	return SUCCESS;
}

int plugin_message_array_create(const plugin_data_type *type_string, plugin_message_array_h *array)
{
	FUNC_START();
	if ((NULL == type_string) || (NULL == array)) {
		return FAIL;
	}
	int i = 0, len = strlen(type_string);
	for (i = 0; i < len; i++) {
		switch (type_string[i]) {
		case PLUGIN_DATA_TYPE_NUM:
		case PLUGIN_DATA_TYPE_BOOL:
		case PLUGIN_DATA_TYPE_STRING:
			break;
		default:
			return FAIL;
		}
	}

	plugin_message_array_h _array = (plugin_message_array_h) calloc(1, sizeof(struct _plugin_message_array_s));
	plugin_data_type *_types = (plugin_data_type *) calloc(strlen(type_string)+1, sizeof(plugin_data_type));

	if ((NULL == _array) || (NULL == _types)) {
		free(_array);
		free(_types);
		return FAIL;
	}

	_array->element = json_array_new();

	_array->types = _types;
	strncpy(_array->types, type_string, strlen(type_string));

	*array = _array;

	FUNC_END();
	return SUCCESS;
}

void plugin_message_array_destroy(plugin_message_array_h array)
{
	FUNC_START();
	if (NULL != array) {
		json_array_unref(array->element);
		free(array->types);
		free(array);
	}
	FUNC_END();
}

int plugin_message_array_add_element(plugin_message_array_h array, ...)
{
	FUNC_START();
	if (NULL == array) {
		return FAIL;
	}

	plugin_data_type *types = array->types;

	int count = strlen(types);
	int i = 0;
	char index[PLUGIN_PARAMETER_BUF_SIZE_MAX+1] = {0, };

	JsonObject *obj = json_object_new();
	JsonNode *new_node;

	PRINT_LOG("[%d]count : %d\n", __LINE__, count);

	va_list vl;
	va_start(vl, array);

	for (i = 0; i < count; i++) {
		memset(index, 0, PLUGIN_PARAMETER_BUF_SIZE_MAX);
		snprintf(index, PLUGIN_PARAMETER_BUF_SIZE_MAX, "%d", i+1);
		PRINT_LOG("[%d]index : %s, type : %c\n", __LINE__, index, types[i]);

		switch (types[i]) {
/*
		case PLUGIN_DATA_TYPE_INT:
			new_node = json_node_new(JSON_NODE_VALUE);
			json_node_set_int(new_node, (gint64)va_arg(vl, int));
			json_object_set_member(obj, index, new_node);
			PRINT_LOG("[%d]index : %s\n", __LINE__, index);
			break;
*/
		case PLUGIN_DATA_TYPE_NUM:
			new_node = json_node_new(JSON_NODE_VALUE);
			json_node_set_int(new_node, (gint64)va_arg(vl, long long int));
			json_object_set_member(obj, index, new_node);
			PRINT_LOG("[%d]index : %s\n", __LINE__, index);
			break;
		case PLUGIN_DATA_TYPE_BOOL:
			new_node = json_node_new(JSON_NODE_VALUE);
			json_node_set_boolean(new_node, va_arg(vl, int) ? true : false);
			json_object_set_member(obj, index, new_node);
			PRINT_LOG("[%d]index : %s\n", __LINE__, index);
			break;
		case PLUGIN_DATA_TYPE_STRING:
			new_node = json_node_new(JSON_NODE_VALUE);
			json_node_set_string(new_node, va_arg(vl, char *));
			json_object_set_member(obj, index, new_node);
			PRINT_LOG("[%d]index : %s\n", __LINE__, index);
			break;
		default:
			new_node = json_node_new(JSON_NODE_NULL);
			json_object_set_member(obj, index, new_node);
			PRINT_LOG("[%d]index : %s <unknown dada, function fail>\n", __LINE__, index);
			va_end(vl);
			return FAIL;
		}
	}
	va_end(vl);

	PRINT_LOG("array length : %u\n", json_object_get_size(obj));
	json_array_add_object_element(array->element, obj);

	FUNC_END();
	return SUCCESS;
}

int plugin_message_array_get_element(plugin_message_array_h array, int idx, ...)
{
	FUNC_START();
	if ((NULL == array) || (1 > idx)) {
		return FAIL;
	}

	if (json_array_get_length(array->element) < idx) {
		return FAIL;
	}

	JsonObject *obj = json_array_get_object_element(array->element, idx-1);

	if (NULL == obj) {
		return FAIL;
	}

	plugin_data_type *types = array->types;

	int count = strlen(types);
	int i = 0;
	char index[PLUGIN_PARAMETER_BUF_SIZE_MAX+1] = {0, };

	JsonNode *new_node;

	PRINT_LOG("[%d]count : %d\n", __LINE__, count);

	va_list vl;
	va_start(vl, idx);

	for (i = 0; i < count; i++) {
		memset(index, 0, PLUGIN_PARAMETER_BUF_SIZE_MAX);
		snprintf(index, PLUGIN_PARAMETER_BUF_SIZE_MAX, "%d", i+1);
		PRINT_LOG("[%d]index : %s, type : %c\n", __LINE__, index, types[i]);
		new_node = json_object_get_member(obj, index);

		switch (types[i]) {
/*
		case PLUGIN_DATA_TYPE_INT:
			*(va_arg(vl, int *)) = (int) json_node_get_int(new_node);
			PRINT_LOG("[%d]index : %s\n", __LINE__, index);
			break;
*/
		case PLUGIN_DATA_TYPE_NUM:
			*(va_arg(vl, long long int *)) = (long long int) json_node_get_int(new_node);
			PRINT_LOG("[%d]index : %s\n", __LINE__, index);
			break;
		case PLUGIN_DATA_TYPE_BOOL:
			*(va_arg(vl, bool *)) = (bool) json_node_get_boolean(new_node);
			PRINT_LOG("[%d]index : %s\n", __LINE__, index);
			break;
		case PLUGIN_DATA_TYPE_STRING:
			*(va_arg(vl, char **)) = (char *) json_node_dup_string(new_node);
			PRINT_LOG("[%d]index : %s\n", __LINE__, index);
			break;
		default:
			PRINT_LOG("[%d]index : %s\n", __LINE__, index);
			break;
		}
	}
	va_end(vl);

	FUNC_END();
	return SUCCESS;
}

