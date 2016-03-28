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

#ifndef __TIZEN_SOCIAL_LIBSERVICE_PLUGIN_MESSAGE_H__
#define __TIZEN_SOCIAL_LIBSERVICE_PLUGIN_MESSAGE_H__

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum {
	PLUGIN_MESSAGE_TYPE_FUNCTION = 1,
	PLUGIN_MESSAGE_TYPE_CALLBACK = 2,
} plugin_message_type_e;

/*
#define PLUGIN_DATA_TYPE_INT		'i'
#define	PLUGIN_DATA_TYPE_LONG		'l'
*/

/* INT/LONG -> NUM (64bit) */
#define PLUGIN_DATA_TYPE_NUM		'n'
#define	PLUGIN_DATA_TYPE_STRING		's'
#define	PLUGIN_DATA_TYPE_BOOL		'b'
#define	PLUGIN_DATA_TYPE_ARRAY		'a'
#define	PLUGIN_DATA_TYPE_UNKNOWN	'u'

typedef char plugin_data_type;


/*
typedef enum {
	PM_TYPE_FALSE = 0,
	PM_TYPE_TRUE = 1,
} pmbool_type_e;
*/

typedef long long int pmnumber;
typedef char *pmstring;
typedef bool pmbool;

typedef enum {
	PLUGIN_MESSAGE_ELEMENT_CONTEXT_ID,
	PLUGIN_MESSAGE_ELEMENT_FUNCTION_NAME,
	PLUGIN_MESSAGE_ELEMENT_PARAMETER_MANDATORY,
	PLUGIN_MESSAGE_ELEMENT_PARAMETER_OPTIONAL,
	PLUGIN_MESSAGE_ELEMENT_REQUEST_ID,
	PLUGIN_MESSAGE_ELEMENT_MESSAGE_TYPE,
	PLUGIN_MESSAGE_ELEMENT_RESULT_CODE,
	PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE,
} plugin_message_element_e;

typedef struct _plugin_message_s *plugin_message_h;

typedef struct _plugin_message_array_s *plugin_message_array_h;

API
int plugin_message_create(plugin_message_h *message);

API
void plugin_message_destroy(plugin_message_h message);

API
int plugin_message_set_value_number(plugin_message_h message, plugin_message_element_e field, pmnumber value);

API
int plugin_message_get_value_number(plugin_message_h message, plugin_message_element_e field, pmnumber *value);

API
int plugin_message_set_value_string(plugin_message_h message, plugin_message_element_e field, const char *value);

API
int plugin_message_get_value_string(plugin_message_h message, plugin_message_element_e field, char **value);

API
int plugin_message_set_value_bool(plugin_message_h message, plugin_message_element_e field, bool value);

API
int plugin_message_get_value_bool(plugin_message_h message, plugin_message_element_e field, bool *value);


API
int plugin_message_set_param_number(plugin_message_h message, int param_index, pmnumber value);

API
int plugin_message_get_param_number(plugin_message_h message, int param_index, pmnumber *value);

API
int plugin_message_set_param_string(plugin_message_h message, int param_index, const char *value);

API
int plugin_message_get_param_string(plugin_message_h message, int param_index, char **value);

API
int plugin_message_set_param_bool(plugin_message_h message, int param_index, bool value);

API
int plugin_message_get_param_bool(plugin_message_h message, int param_index, bool *value);

API
int plugin_message_set_param_array(plugin_message_h message, int param_index, plugin_message_array_h value);

API
int plugin_message_get_param_array(plugin_message_h message, int param_index, plugin_message_array_h *value);


API
int plugin_message_set_opt_param_number(plugin_message_h message, int param_index, pmnumber value);

API
int plugin_message_get_opt_param_number(plugin_message_h message, int param_index, pmnumber *value);

API
int plugin_message_set_opt_param_string(plugin_message_h message, int param_index, const char *value);

API
int plugin_message_get_opt_param_string(plugin_message_h message, int param_index, char **value);

API
int plugin_message_set_opt_param_bool(plugin_message_h message, int param_index, bool value);

API
int plugin_message_get_opt_param_bool(plugin_message_h message, int param_index, bool *value);

API
int plugin_message_set_opt_param_array(plugin_message_h message, int param_index, plugin_message_array_h value);

API
int plugin_message_get_opt_param_array(plugin_message_h message, int param_index, plugin_message_array_h *value);


API
int plugin_message_serialize(plugin_message_h message, char **data);

API
int plugin_message_deserialize(const char *data, plugin_message_h *message);


API
int plugin_message_array_create(const plugin_data_type *type_string, plugin_message_array_h *array);

API
void plugin_message_array_destroy(plugin_message_array_h array);

API
int plugin_message_array_add_element(plugin_message_array_h array, ...);

API
int plugin_message_array_get_element(plugin_message_array_h array, int idx, ...);

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_SOCIAL_LIBSERVICE_PLUGIN_MESSAGE_H__ */
