/*
 * Service Adaptor
 *
 * Copyright (c) 2014 - 2015 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <glib.h>

#include <app.h>

#include "service_adaptor_errors.h"
#include "service_adaptor_internal.h"
#include "sal_service_adaptor.h"
#include "sal_service_task.h"
#include "sal_service_task_internal.h"
#include "sal_ipc_client.h"
#include "sal_ipc_client_core.h"
#include "sal_ipc_client_auth.h"

//******************************************************************************
//* Global variables and defines
//******************************************************************************

/**
 * @brief Describes infromation about Adaptor Handle
 */
typedef struct _service_adaptor_s
{
	char *uri;

	GList *plugins;		/* char **plugins (uri) */
	GList *started_plugins;	/* service_plugin_h **started_plugins */
} service_adaptor_s;
//typedef struct _service_adaptor_s *service_adaptor_h;

/**
 * @brief Describes infromation about Plugin Handle
 */
typedef struct _service_plugin_s
{
	char *uri;
	GHashTable *property;
} service_plugin_s;

service_adaptor_h service_adaptor = NULL;

//******************************************************************************
//* Private interface
//******************************************************************************

//******************************************************************************
//* Private interface definition
//******************************************************************************

//******************************************************************************
//* Public interface definition
//******************************************************************************

API int service_adaptor_create(service_adaptor_h *service_adaptor)
{
	RETV_IF(NULL == service_adaptor, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	*service_adaptor = (service_adaptor_h) g_malloc0(sizeof(service_adaptor_s));

	return service_adaptor_connect();
}

API int service_adaptor_destroy(service_adaptor_h service_adaptor)
{
	RETV_IF(NULL == service_adaptor, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	return service_adaptor_disconnect();
}

API int service_adaptor_connect()
{
	SAL_FN_CALL;

	service_adaptor = (service_adaptor_h) g_malloc0(sizeof(service_adaptor_s));

	char *uri = NULL;

	app_get_id(&uri);

	if (NULL == uri)
	{
		char path[1024] = {0,};
		int path_len = 0;

		path_len = readlink("/proc/self/exe", path, 1024);
		RETV_IF(0 == path_len, SERVICE_ADAPTOR_ERROR_SYSTEM);

		uri = strndup(path, path_len);
	}

	service_adaptor->uri = strdup(uri);

	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	ret = sal_ipc_client_init();
	RETV_IF(SERVICE_ADAPTOR_ERROR_NONE != ret, SERVICE_ADAPTOR_ERROR_INTERNAL);

	GList *plugins = NULL;
	ret = ipc_service_adaptor_connect(uri, &plugins);
	RETV_IF(SERVICE_ADAPTOR_ERROR_NONE != ret, SERVICE_ADAPTOR_ERROR_INTERNAL);

	ret = service_task_connect();
	RETVM_IF(SERVICE_ADAPTOR_ERROR_NONE != ret, ret, "service_task_connect() Failed");

	service_adaptor->plugins = plugins;

	SAL_FREE(uri);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_adaptor_disconnect()
{
	SAL_FN_CALL;

	RETV_IF(NULL == service_adaptor, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	ret = service_task_disconnect();
	RETVM_IF(SERVICE_ADAPTOR_ERROR_NONE != ret, ret, "service_task_disconnect() Failed");

	ret = ipc_service_adaptor_disconnect(service_adaptor->uri);
	RETV_IF(SERVICE_ADAPTOR_ERROR_NONE != ret, SERVICE_ADAPTOR_ERROR_INTERNAL);

	ret = sal_ipc_client_deinit();
	RETV_IF(SERVICE_ADAPTOR_ERROR_NONE != ret, SERVICE_ADAPTOR_ERROR_INTERNAL);

	// TODO: free memory in adaptor
	SAL_FREE(service_adaptor);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_adaptor_foreach_plugin(service_adaptor_h service_adaptor, service_adaptor_plugin_cb callback, void *user_data)
{
	SAL_FN_CALL;

	RETV_IF(NULL == service_adaptor, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	return service_adaptor_foreach_service_plugin(callback, user_data);
}

API int service_adaptor_foreach_service_plugin(service_adaptor_plugin_cb callback, void *user_data)
{
	SAL_FN_CALL;

	RETV_IF(NULL == service_adaptor, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == callback, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	RETV_IF(0 == g_list_length(service_adaptor->plugins), SERVICE_ADAPTOR_ERROR_NO_DATA);

	for (GList *list = g_list_first(service_adaptor->plugins); list != NULL; list = list->next)
	{
		char * uri = (char *) list->data;

		bool ret = callback(uri, 0, user_data);
		RETV_IF(false == ret, SERVICE_ADAPTOR_ERROR_NONE);
	}

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_adaptor_get_last_result(int *err)
{
	SAL_FN_CALL;

	RETV_IF(NULL == service_adaptor, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == err, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_adaptor_get_last_error_message(char **message)
{
	SAL_FN_CALL;

	RETV_IF(NULL == service_adaptor, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == message, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	return SERVICE_ADAPTOR_ERROR_NO_DATA;
}

API int service_adaptor_create_plugin(service_adaptor_h service_adaptor, const char *plugin_uri, service_plugin_h *plugin)
{
	RETV_IF(NULL == service_adaptor, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	return service_plugin_create(plugin_uri, plugin);
}

API int service_plugin_create(const char *uri, service_plugin_h *plugin)
{
	SAL_FN_CALL;

	RETV_IF(NULL == service_adaptor, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == uri, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == plugin, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	int ret = ipc_service_plugin_create(uri);
	RETV_IF(SERVICE_ADAPTOR_ERROR_NONE != ret, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	service_plugin_h service_plugin = (service_plugin_h) g_malloc0(sizeof(service_plugin_s));

	service_plugin->uri = strdup(uri);
	service_plugin->property = g_hash_table_new(g_str_hash, g_str_equal);

	*plugin = service_plugin;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_plugin_destroy(service_plugin_h plugin)
{
	SAL_FN_CALL;

	RETV_IF(NULL == service_adaptor, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == plugin, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	int ret = ipc_service_plugin_destroy(plugin->uri);
	RETV_IF(SERVICE_ADAPTOR_ERROR_NONE != ret, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	// TODO: free memeory of plugin
	SAL_FREE(plugin);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_plugin_add_property(service_plugin_h plugin, const char *key, const char *value)
{
	RETV_IF(NULL == service_adaptor, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == plugin, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == value, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	g_hash_table_insert(plugin->property, g_strdup(key), g_strdup(value));

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_plugin_remove_property(service_plugin_h plugin, const char *key)
{
	RETV_IF(NULL == service_adaptor, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == plugin, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	g_hash_table_remove(plugin->property, key);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_plugin_get_property(service_plugin_h plugin, const char *key, char **value)
{
	RETV_IF(NULL == service_adaptor, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == plugin, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == value, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	if (0 < g_hash_table_size(plugin->property))
	{
		GHashTableIter iter;
		gpointer iter_key, iter_value;

		g_hash_table_iter_init(&iter, plugin->property);
		while (g_hash_table_iter_next(&iter, &iter_key, &iter_value))
		{
			if (strcmp(key, (char *) iter_key) == 0)
			{
				*value = g_strdup(iter_value);
				return SERVICE_ADAPTOR_ERROR_NONE;
			}
		}
	}

	return SERVICE_ADAPTOR_ERROR_NO_DATA;
}

API int service_plugin_login(service_plugin_h plugin, service_plugin_login_cb callback, void *user_data)
{
	SAL_FN_CALL;

	RETV_IF(NULL == service_adaptor, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == plugin, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == callback, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	// TODO: login this plugin via service adaptor (dbus)

	callback(SERVICE_ADAPTOR_ERROR_NONE, user_data);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_plugin_start(service_plugin_h plugin, int service_mask)
{
	RETV_IF(0 >= service_mask, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	return service_plugin_start_all(plugin);
}

API int service_plugin_start_all(service_plugin_h plugin)
{
	SAL_FN_CALL;

	RETV_IF(NULL == service_adaptor, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == plugin, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	// TODO: check this plugin via service adaptor (dbus), use or not use?, must be logined.

	service_adaptor->started_plugins = g_list_append(service_adaptor->started_plugins, plugin);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_plugin_stop(service_plugin_h plugin)
{
	SAL_FN_CALL;

	RETV_IF(NULL == service_adaptor, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == plugin, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	// TODO: notify to stop this plugin to service adaptor (dbus)

	for (GList *list = g_list_first(service_adaptor->started_plugins); list != NULL; list = list->next)
	{
		service_plugin_h service_plugin = (service_plugin_h) list->data;

		if (0 == strcmp(service_plugin->uri, plugin->uri))
		{
			service_adaptor->started_plugins = g_list_remove(service_adaptor->started_plugins, list);

			return SERVICE_ADAPTOR_ERROR_NONE;
		}
	}

	return SERVICE_ADAPTOR_ERROR_NONE;
}

int service_plugin_get_uri(service_plugin_h plugin, char **uri)
{
	SAL_FN_CALL;

	RETV_IF(NULL == service_adaptor, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == plugin, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == uri, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == g_list_find(service_adaptor->started_plugins, plugin), SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	*uri = strdup(plugin->uri);

	return SERVICE_ADAPTOR_ERROR_NONE;
}
