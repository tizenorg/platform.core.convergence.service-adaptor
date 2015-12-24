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

#include <service_adaptor_client.h>

#include "sal_log.h"
#include "sal_ipc.h"
#include "sal_types.h"

#include "sal_ipc_client.h"

#include "sal_client_types.h"
#include "sal_client_internal.h"

/*
#include "service_adaptor_errors.h"
#include "service_adaptor_internal.h"
#include "sal_service_adaptor.h"
#include "sal_service_task.h"
#include "sal_service_task_internal.h"
#include "sal_ipc_client.h"
#include "sal_ipc_client_core.h"
#include "sal_ipc_client_auth.h"

#include "sal_client_internal.h"
*/

/******************************************************************************
 * Global variables and defines
 ******************************************************************************/

static service_adaptor_h g_service_adaptor = NULL;

/******************************************************************************
 * Private interface
 ******************************************************************************/

static void __destroy_service_adaptor(service_adaptor_h handle);

static void __destroy_service_plugin(service_plugin_h handle);

/******************************************************************************
 * Private interface definition
 ******************************************************************************/

static void __destroy_service_adaptor(service_adaptor_h handle)
{
}

static void __destroy_service_plugin(service_plugin_h handle)
{
	if (handle) {
		if (handle->property)
			g_hash_table_unref(handle->property);
		handle->property = NULL;
		SAL_FREE(handle->uri);
		SAL_FREE(handle);
	}
}

/******************************************************************************
 * Public interface definition
 ******************************************************************************/

API int service_adaptor_connect()
{
	SAL_FN_CALL;

	g_service_adaptor = (service_adaptor_h) g_malloc0(sizeof(service_adaptor_s));
	RETV_IF(NULL == g_service_adaptor, SERVICE_ADAPTOR_ERROR_OUT_OF_MEMORY);

	char *uri = NULL;

	app_get_id(&uri);

	if (NULL == uri) {
		char path[1024] = {0,};
		int path_len = 0;

		path_len = readlink("/proc/self/exe", path, 1024);
		if (0 == path_len)
			uri = strdup("notfound");
		else
			uri = strndup(path, path_len);

		RETV_IF(NULL == uri, SERVICE_ADAPTOR_ERROR_OUT_OF_MEMORY);
	}

	g_service_adaptor->uri = uri;
	uri = NULL;

	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GList *plugins = NULL;

	ret = sal_ipc_client_init();
	TRY_IF(SAL_ERROR_NONE != ret, "ipc init failed : %d", ret);

	int pid = (int)getpid();
	ret = ipc_service_adaptor_connect(pid, uri, &plugins);
	TRY_IF(SAL_ERROR_NONE != ret, "API remote failed : %d", ret);

	g_service_adaptor->pid = pid;
	g_service_adaptor->plugins = plugins;

	return sal_client_return_ipc_ret(ret);

catch:
	SAL_FREE(g_service_adaptor->uri);
	SAL_FREE(g_service_adaptor);

	return sal_client_return_ipc_ret(ret);
}

API int service_adaptor_disconnect()
{
	SAL_FN_CALL;

	RETV_IF(NULL == g_service_adaptor, SERVICE_ADAPTOR_ERROR_INVALID_STATE);

	int ret = SERVICE_ADAPTOR_ERROR_NONE;
/*
	ret = service_task_disconnect();
	RETVM_IF(SERVICE_ADAPTOR_ERROR_NONE != ret, ret, "service_task_disconnect() Failed");
*/
	ret = ipc_service_adaptor_disconnect(g_service_adaptor->pid, g_service_adaptor->uri);
/*	RETV_IF(SERVICE_ADAPTOR_ERROR_NONE != ret, SERVICE_ADAPTOR_ERROR_UNKNOWN);*/
	SAL_DBG("disconnect ret : %d", ret);

	ret = sal_ipc_client_deinit();
/*	RETV_IF(SERVICE_ADAPTOR_ERROR_NONE != ret, SERVICE_ADAPTOR_ERROR_UNKNOWN);*/
	SAL_DBG("deinit ret : %d", ret);

	/* TODO: free memory in adaptor */
	__destroy_service_adaptor(g_service_adaptor);
	g_service_adaptor = NULL;

	return SERVICE_ADAPTOR_ERROR_NONE;
}


API int service_adaptor_create(service_adaptor_h *handle)
{
	RETV_IF(NULL == handle, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	if (NULL == g_service_adaptor) {
		/* TODO add handling for duplicated connect */
		int ret = service_adaptor_connect();
		if (ret)
			return ret;
	}
	*handle = g_service_adaptor;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_adaptor_destroy(service_adaptor_h handle)
{
	RETV_IF(NULL == handle, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	return service_adaptor_disconnect();
}


API int service_adaptor_foreach_plugin(service_adaptor_h handle, service_adaptor_plugin_cb callback, void *user_data)
{
	SAL_FN_CALL;

	RETV_IF(NULL == handle, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == callback, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	RETV_IF(0 == g_list_length(g_service_adaptor->plugins), SERVICE_ADAPTOR_ERROR_NO_DATA);

	SAL_FOREACH_GLIST(iter, g_service_adaptor->plugins) {
		char *_uri = (char *)iter->data;
		bool ret = callback(_uri, (SERVICE_PLUGIN_SERVICE_AUTH | SERVICE_PLUGIN_SERVICE_STORAGE), user_data);
		if (false == ret)
			break;
	}

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_adaptor_get_last_result(int *err)
{
	SAL_FN_CALL;

	RETV_IF(NULL == err, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	*err = sal_ipc_client_get_last_error();

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_adaptor_get_last_error_message(char **message)
{
	SAL_FN_CALL;

	RETV_IF(NULL == message, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	char *_msg = sal_ipc_client_get_last_message();
	RETV_IF(NULL == _msg, SERVICE_ADAPTOR_ERROR_NO_DATA);
	RETV_IF('\0' == _msg[0], SERVICE_ADAPTOR_ERROR_NO_DATA);

	char *ret = strdup(_msg);
	RETV_IF(NULL == ret, SERVICE_ADAPTOR_ERROR_OUT_OF_MEMORY);

	*message = ret;
	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_adaptor_create_plugin(service_adaptor_h adaptor, const char *plugin_uri, service_plugin_h *plugin)
{
	SAL_FN_CALL;

	RETV_IF(NULL == adaptor, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == plugin_uri, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == plugin, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	RETV_IF(NULL == g_service_adaptor, SERVICE_ADAPTOR_ERROR_INVALID_STATE);

/*	int ret = ipc_service_plugin_create(uri);
	RETV_IF(SERVICE_ADAPTOR_ERROR_NONE != ret, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
*/
	service_plugin_h service_plugin = NULL;
	char *_uri = NULL;
	GHashTable *_property = NULL;

	service_plugin = (service_plugin_h) g_malloc0(sizeof(service_plugin_s));
	TRY_IF(NULL == service_plugin, "Out of memory");

	_uri = strdup(plugin_uri);
	TRY_IF(NULL == service_plugin, "Out of memory");

	_property = g_hash_table_new(g_str_hash, g_str_equal);
	TRY_IF(NULL == _property, "Out of memory");

	service_plugin->uri = _uri;
	service_plugin->property = _property;
	*plugin = service_plugin;

	return SERVICE_ADAPTOR_ERROR_NONE;

catch:
	SAL_FREE(_uri);
	__destroy_service_plugin(service_plugin);
	service_plugin = NULL;

	return SERVICE_ADAPTOR_ERROR_OUT_OF_MEMORY;
}

API int service_plugin_destroy(service_plugin_h plugin)
{
	SAL_FN_CALL;

	RETV_IF(NULL == plugin, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == g_service_adaptor, SERVICE_ADAPTOR_ERROR_INVALID_STATE);

	/*int ret = ipc_service_plugin_stop(plugin->uri);
	SAL_DBG("plugin destroy ret : %d", ret);
	*/

	g_service_adaptor->plugins = g_list_remove(g_service_adaptor->plugins, (gpointer)plugin);
	__destroy_service_plugin(plugin);
	plugin = NULL;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_plugin_add_property(service_plugin_h plugin, const char *key, const char *value)
{
	RETV_IF(NULL == plugin, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == value, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	RETV_IF(NULL == g_service_adaptor, SERVICE_ADAPTOR_ERROR_INVALID_STATE);

	char *_key = g_strdup(key);
	TRY_IF(NULL == _key, "Out of memory");

	char *_value = g_strdup(value);
	TRY_IF(NULL == _value, "Out of memory");

	g_hash_table_insert(plugin->property, g_strdup(key), g_strdup(value));

	return SERVICE_ADAPTOR_ERROR_NONE;

catch:
	SAL_FREE(_key);
	SAL_FREE(_value);

	return SERVICE_ADAPTOR_ERROR_OUT_OF_MEMORY;
}

API int service_plugin_remove_property(service_plugin_h plugin, const char *key)
{
	RETV_IF(NULL == plugin, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	RETV_IF(NULL == g_service_adaptor, SERVICE_ADAPTOR_ERROR_INVALID_STATE);

	g_hash_table_remove(plugin->property, key);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_plugin_get_property(service_plugin_h plugin, const char *key, char **value)
{
	RETV_IF(NULL == plugin, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == value, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	RETV_IF(NULL == g_service_adaptor, SERVICE_ADAPTOR_ERROR_INVALID_STATE);

	char *find_data = NULL;
	find_data = (char *)g_hash_table_lookup(plugin->property, key);
	RETV_IF(NULL == find_data, SERVICE_ADAPTOR_ERROR_NO_DATA);

	char *_value = strdup(find_data);
	RETV_IF(NULL == _value, SERVICE_ADAPTOR_ERROR_OUT_OF_MEMORY);

	*value = _value;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

/*
API int service_plugin_login(service_plugin_h plugin, service_plugin_login_cb callback, void *user_data)
{
	SAL_FN_CALL;

	RETV_IF(NULL == g_service_adaptor, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == plugin, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == callback, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	// TODO: login this plugin via service adaptor (dbus)

	callback(SERVICE_ADAPTOR_ERROR_NONE, user_data);

	return SERVICE_ADAPTOR_ERROR_NONE;
}
*/

API int service_plugin_start(service_plugin_h plugin, int service_mask)
{
	SAL_FN_CALL;

	RETV_IF(NULL == plugin, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(0 >= service_mask, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	RETV_IF(NULL == g_service_adaptor, SERVICE_ADAPTOR_ERROR_INVALID_STATE);

	RETV_IF(NULL == g_service_adaptor->uri, SERVICE_ADAPTOR_ERROR_UNKNOWN);
	RETV_IF(NULL == plugin->uri, SERVICE_ADAPTOR_ERROR_UNKNOWN);

	char *plugin_handle = NULL;
	int ret = ipc_service_plugin_start(g_service_adaptor->pid,
			g_service_adaptor->uri, plugin->uri, &plugin_handle);

	if (!ret) {
		g_service_adaptor->started_plugins = g_list_append(g_service_adaptor->started_plugins, plugin);
		plugin->handle = plugin_handle;
	}

	return sal_client_return_ipc_ret(ret);
}

API int service_plugin_stop(service_plugin_h plugin)
{
	SAL_FN_CALL;

	RETV_IF(NULL == plugin, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	RETV_IF(NULL == g_service_adaptor, SERVICE_ADAPTOR_ERROR_INVALID_STATE);
	RETV_IF(NULL == plugin->handle, SERVICE_ADAPTOR_ERROR_INVALID_STATE);

	int ret = ipc_service_plugin_stop(plugin->handle);
	/* TODO add retry logic if ipc failed */
	SAL_ERR("ipc stop failed : %d", ret);

	SAL_FREE(plugin->handle);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

/*
int service_plugin_get_uri(service_plugin_h plugin, char **uri)
{
	SAL_FN_CALL;

	RETV_IF(NULL == g_service_adaptor, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == plugin, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == uri, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == g_list_find(g_service_adaptor->started_plugins, plugin), SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	*uri = strdup(plugin->uri);

	return SERVICE_ADAPTOR_ERROR_NONE;
}
*/
