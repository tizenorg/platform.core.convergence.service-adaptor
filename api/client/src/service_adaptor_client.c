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

/******************************************************************************
 * Desc:
 * Created on: Oct, 2014
 * Auth: Yongjin Kim <youth.kim@samsung.com>
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

#include <glib.h>
#include <tizen.h>
/*#include <security-server.h>*/
/* #include <app.h> */
#include <app_common.h>
/* #include <app_manager.h> */

#include <dbus-server.h>

#include "service_adaptor_client.h"
#include "service_adaptor_client_plugin.h"
#include "service_adaptor_client_private.h"
#include "service_adaptor_client_type.h"
#include "service_adaptor_client_log.h"
#include "dbus_client.h"
#include "dbus_client_layer.h"

#include "util/service_adaptor_client_util.h"
/************************* private feature */
#include "private/service-adaptor-client.h"
/************************* private feature */

#define ERROR_MSG_MAX_LEN 2048

static __thread int last_error_code = 0;
static __thread char last_error_message[ERROR_MSG_MAX_LEN] = "";

/**
 * Work Queue about async functions
 */
GList *g_service_adaptor_task_queue = NULL;
GList *g_service_adaptor_signal_queue = NULL;

/**
 * Connection counter. It is needed to allow multiple connections from single process.
 */
static int connections_counter = 0;

static GMutex connections_counter_mutex;

static service_adaptor_h g_service_adaptor = NULL;

#ifdef __DEBUG_GLIB_ERROR
static void __glog_handler_cb(const gchar *log_domain,
		GLogLevelFlags log_level,
		const gchar *message,
		gpointer user_data)
{
	sac_error("============================================================");
	sac_error("============================================================");
	sac_error("================== Critical GLib Error =====================");
	sac_error("============================================================");
	sac_error("============================================================");
	sac_error("=== Log Domain : %s", log_domain);
	sac_error("=== Level : %d", (int)log_level);
	sac_error("=== Message : %s", message);
	sac_error("============================================================");
	sac_error("============================================================");
}

static void glog_handler_init()
{
	sac_info("glib log handler init : %d",
			(int)g_log_set_handler("GLib", G_LOG_LEVEL_CRITICAL, __glog_handler_cb, NULL));
}
#endif

/**	@brief	Adds Task in Queue
 *	@return	int
 *	@remarks :
 */
int _queue_add_task(int64_t _id,
						uint32_t _callback,
						void *_handle,
						void *user_data)
{
	FUNC_START();

	service_adaptor_task_h task = (service_adaptor_task_h) g_malloc0(sizeof(service_adaptor_task_s));

	if (NULL == task) {
		/* LCOV_EXCL_START */
		FUNC_STOP();
		return -1;
		/* LCOV_EXCL_STOP */
	}

	task->id = _id;
	task->callback = _callback;
	task->handle = _handle;
	task->user_data = user_data;

	g_service_adaptor_task_queue = g_list_append(g_service_adaptor_task_queue, task);

	FUNC_END();
	return 0;
}

/**	@brief	Adds Task in Queue
 *	@return	int
 *	@remarks :
 */
int _queue_del_task(service_adaptor_task_h _task)
{
	FUNC_START();
	service_adaptor_task_h target = NULL;

	for (GList *list = g_list_first(g_service_adaptor_task_queue); list != NULL; list = g_list_next(list)) {
		service_adaptor_task_h data = (service_adaptor_task_h) list->data;

		if ((NULL != data) && (_task == data)) {
			target = data;
			break;
		}
	}

	if (NULL != target) {
		g_service_adaptor_task_queue = g_list_remove(g_service_adaptor_task_queue, target);
		g_free(target);
		target = NULL;
	}

	FUNC_END();
	return 0;
}

/**	@brief	Adds Task in Queue
 *	@return	service_adaptor_task_h
 *	@remarks :
 */
service_adaptor_task_h _queue_get_task(int64_t id)
{
	FUNC_START();
	service_adaptor_task_h target = NULL;

	for (GList *list = g_list_first(g_service_adaptor_task_queue); list != NULL; list = g_list_next(list)) {
		service_adaptor_task_h data = (service_adaptor_task_h) list->data;

		if ((NULL != data) && (id == data->id)) {
			target = data;
			break;
		}
	}

	FUNC_END();
	return target;
}

/**	@brief	Clears Task in Queue
 *	@return	void
 *	@remarks :
 */
void _queue_clear_task()
{
	FUNC_START();
	if (NULL != g_service_adaptor_task_queue) {
		g_list_free(g_service_adaptor_task_queue);
	}
	g_service_adaptor_task_queue = NULL;
}

/* LCOV_EXCL_START */
/**     @brief  Adds Task in Queue
 *      @return service_adaptor_task_h
 *      @remarks :
 */
service_adaptor_task_h _signal_queue_get_task(int64_t _id)
{
	FUNC_START();
	service_adaptor_task_h target = NULL;

	for (GList *list = g_list_first(g_service_adaptor_signal_queue); list != NULL; list = g_list_next(list)) {
		service_adaptor_task_h data = (service_adaptor_task_h) list->data;

		if ((NULL != data) && (_id == data->id)) {
			target = data;
			break;
		}
	}

	FUNC_END();
	return target;
}
/* LCOV_EXCL_STOP */

/**     @brief  Clears Task in Queue
 *      @return void
 *      @remarks :
 */
void _signal_queue_clear_task()
{
	FUNC_START();
	if (NULL != g_service_adaptor_signal_queue) {
		g_list_free(g_service_adaptor_signal_queue); /* LCOV_EXCL_LINE */
	}
	g_service_adaptor_signal_queue = NULL;
	FUNC_END();
}

void _service_adaptor_set_last_result(int code, const char *message)
{
	FUNC_START();
	last_error_code = code;

	memset(last_error_message, 0x00, ERROR_MSG_MAX_LEN);

	if (NULL != message) {
		strncpy(last_error_message, message, ERROR_MSG_MAX_LEN-1);
	}
	FUNC_END();
}

int service_adaptor_get_last_result(int *err)
{
	FUNC_START();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	if (NULL == err) {
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}
	*err = last_error_code;

	FUNC_END();
	return ret;
}

int service_adaptor_get_last_error_message(char **message)
{
	FUNC_START();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	if (NULL == message) {
		ret = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	} else if (0 >= strlen(last_error_message)) {
		ret = SERVICE_ADAPTOR_ERROR_NO_DATA;
	} else {
		*message = strdup(last_error_message); /* LCOV_EXCL_LINE */
		ret = SERVICE_ADAPTOR_ERROR_NONE; /* LCOV_EXCL_LINE */
	}

	FUNC_END();
	return ret;
}

/**	@brief	Connects Service Adaptor
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_create(service_adaptor_h *handle)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_h service = NULL;

#ifdef __DEBUG_GLIB_ERROR
	glog_handler_init();
#endif

	g_mutex_lock(&connections_counter_mutex);

	if (NULL == handle) {
		g_mutex_unlock(&connections_counter_mutex);
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	if (0 < connections_counter) {
		g_mutex_unlock(&connections_counter_mutex); /* LCOV_EXCL_LINE */
		return SERVICE_ADAPTOR_ERROR_NOT_SUPPORTED; /* LCOV_EXCL_LINE */
	}

	service = (service_adaptor_h) calloc(1, sizeof(service_adaptor_s));

	if (NULL == service) {
		service_adaptor_set_last_result(SERVICE_ADAPTOR_ERROR_UNKNOWN, "Memory allocation failed"); /* LCOV_EXCL_LINE */
		g_mutex_unlock(&connections_counter_mutex); /* LCOV_EXCL_LINE */
		return SERVICE_ADAPTOR_ERROR_UNKNOWN; /* LCOV_EXCL_LINE */
	}

	int dbus_ret = _dbus_client_layer_init();

	if (0 == dbus_ret) {
		connections_counter = connections_counter + 1;
	}

	service_adaptor_error_s error;
	error.msg = NULL;

	ret = _dbus_connect_service_adaptor(&error);

	if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
		service_adaptor_set_last_result(error.code, error.msg); /* LCOV_EXCL_LINE */
		free(service); /* LCOV_EXCL_LINE */
		g_mutex_unlock(&connections_counter_mutex); /* LCOV_EXCL_LINE */
		free(error.msg); /* LCOV_EXCL_LINE */
		return ret; /* LCOV_EXCL_LINE */
	}

	service->on_signal = NULL;

	*handle = service;
	g_service_adaptor = service;

	g_mutex_unlock(&connections_counter_mutex);

	sac_api_end(ret);
	return ret;
}

/**	@brief	Disconnects Service Adaptor
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_destroy(service_adaptor_h handle)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	if (NULL == handle) {
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	g_mutex_lock(&connections_counter_mutex);

	if (0 >= connections_counter) {
		connections_counter = 0; /* LCOV_EXCL_LINE */
		g_service_adaptor = NULL; /* LCOV_EXCL_LINE */
		g_mutex_unlock(&connections_counter_mutex); /* LCOV_EXCL_LINE */

		ret = SERVICE_ADAPTOR_ERROR_UNKNOWN; /* LCOV_EXCL_LINE */
		return ret; /* LCOV_EXCL_LINE */
	}

	if (NULL != handle) {
		free(handle);
		handle = NULL;
	}

	connections_counter = connections_counter - 1;

	if (0 == connections_counter) {
		_dbus_client_layer_deinit();
	}

	g_service_adaptor = NULL;
	g_mutex_unlock(&connections_counter_mutex);

	sac_api_end(ret);
	return ret;
}

int service_adaptor_foreach_plugin(service_adaptor_h handle,
						service_adaptor_plugin_cb callback,
						void *user_data)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	if (NULL == handle) {
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}
	if (NULL == callback) {
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	plugin_entry_t **plugin_uris = NULL;
	unsigned int plugins_len = 0;
	service_adaptor_error_s error;
	error.msg = NULL;
	ret = _dbus_get_plugin_list(&plugin_uris, &plugins_len, &error);

	if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
		service_adaptor_set_last_result(error.code, error.msg); /* LCOV_EXCL_LINE */
		free(error.msg); /* LCOV_EXCL_LINE */
	} else if ((NULL != plugin_uris) && (0 < plugins_len)) {
		int i;
		bool is_continue = true;
		for (i = 0; i < plugins_len; i++) {
			if (is_continue) {
				is_continue = callback(plugin_uris[i]->plugin_uri, plugin_uris[i]->installed_mask, user_data);
			}
			free(plugin_uris[i]->plugin_uri);
			free(plugin_uris[i]);
		}
	} else {
		ret = SERVICE_ADAPTOR_ERROR_NO_DATA;
	}
	free(plugin_uris);

	sac_api_end(ret);
	return ret;
}

int service_adaptor_create_plugin(service_adaptor_h handle,
						const char *plugin_uri,
						service_plugin_h *plugin)
{
	sac_info("API Called <%s>", __FUNCTION__);
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	if ((NULL == handle) || (NULL == plugin) || (NULL == plugin_uri)) {
		ret = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	} else {
		service_plugin_h _plugin = (service_plugin_h) calloc(1, sizeof(service_plugin_s));
		char *_plugin_uri = strdup(plugin_uri);
		if ((NULL != _plugin) && (NULL != _plugin_uri)) {
			_plugin->adaptor = handle;
			_plugin->service_handle_name = NULL;
			_plugin->optional_property = (void *)g_hash_table_new_full(g_str_hash, g_str_equal, free, free);

			_plugin->plugin_uri = _plugin_uri;
			_plugin->enabled_mask = 0;

			*plugin = _plugin;
		} else {
			free(_plugin); /* LCOV_EXCL_LINE */
			free(_plugin_uri); /* LCOV_EXCL_LINE */
			sac_error("Critical : Memory allocation failed."); /* LCOV_EXCL_LINE */
			ret = SERVICE_ADAPTOR_ERROR_UNKNOWN; /* LCOV_EXCL_LINE */
		}
	}

	sac_api_end(ret);
	return ret;
}

int service_plugin_destroy(service_plugin_h plugin)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	if (NULL == plugin) {
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	} else {
		plugin->adaptor = NULL;
		if (NULL != plugin->optional_property) {
			g_hash_table_destroy((GHashTable *)(plugin->optional_property));
			plugin->optional_property = NULL;
		}
		free(plugin->service_handle_name);
		free(plugin->plugin_uri);
		free(plugin);
	}
	sac_api_end(ret);
	return ret;
}

int service_plugin_add_property(service_plugin_h handle,
						const char *key,
						const char *value)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	if ((NULL == handle) || (NULL == handle->optional_property)) {
		ret = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	} else if ((NULL == key) || (0 >= strlen(key))) {
		ret = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	} else if ((NULL == value) || (0 >= strlen(value))) {
		ret = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	} else {
		g_hash_table_insert((GHashTable *)(handle->optional_property), strdup(key), strdup(value));
	}

	sac_api_end(ret);
	return ret;
}

int service_plugin_remove_property(service_plugin_h handle,
						const char *key)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	if ((NULL == handle) || (NULL == handle->optional_property)) {
		ret = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	} else if ((NULL == key) || (0 >= strlen(key))) {
		ret = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	} else {
		g_hash_table_remove((GHashTable *)(handle->optional_property), key);
	}

	sac_api_end(ret);
	return ret;
}

int service_plugin_get_property(service_plugin_h handle,
						const char *key,
						char **value)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	if (NULL == handle) {
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	if ((NULL == key) || (0 >= strlen(key)) || (NULL == value)) {
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	void *property = (void *)g_hash_table_lookup((GHashTable *)(handle->optional_property), (gconstpointer *)key);
	if (NULL == property) {
		ret = SERVICE_ADAPTOR_ERROR_NO_DATA;
	} else {
		*value = strdup((char *) property);
	}
	sac_api_end(ret);
	return ret;
}

int service_plugin_start(service_plugin_h handle,
						int service_flag)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	int app_ret = 0;
	char *service_handle_name = NULL;
	char security_cookie[SECURITY_SERVER_COOKIE_BUFFER_SIZE] = {0, };

	char *tizen_app_id = NULL;
	char *type = NULL;

	if (NULL == handle) {
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	app_ret = app_get_id(&tizen_app_id);

	if (app_ret || (NULL == tizen_app_id)) {
		char executable_path[1000]; /* LCOV_EXCL_LINE */
		int executable_path_len = 0; /* LCOV_EXCL_LINE */
		executable_path_len = readlink("/proc/self/exe", executable_path, 1000); /* LCOV_EXCL_LINE */
		tizen_app_id = strndup(executable_path, executable_path_len); /* LCOV_EXCL_LINE */
		type = strdup("etc"); /* LCOV_EXCL_LINE */
		handle->app_type = CLIENT_APP_TYPE_ETC; /* LCOV_EXCL_LINE */
	} else {
		type = strdup("app");
		handle->app_type = CLIENT_APP_TYPE_APPLICATION;
	}

	if (NULL == tizen_app_id) {
		free(type); /* LCOV_EXCL_LINE */
		return SERVICE_ADAPTOR_ERROR_UNKNOWN; /* LCOV_EXCL_LINE */
	}

	service_handle_name = g_strconcat(tizen_app_id, "?type=", type, "&plugin=", handle->plugin_uri, NULL);

	free(tizen_app_id);
	free(type);

	if (NULL == service_handle_name) {
		sac_error("handle name get failed"); /* LCOV_EXCL_LINE */
		service_adaptor_set_last_result(SERVICE_ADAPTOR_ERROR_UNKNOWN, "handle name get failed"); /* LCOV_EXCL_LINE */
		return SERVICE_ADAPTOR_ERROR_UNKNOWN; /* LCOV_EXCL_LINE */
	}

	/* TODO replace to real cookie */
	snprintf(security_cookie, SECURITY_SERVER_COOKIE_BUFFER_SIZE, "%020d", (int)getpid());

	handle->service_handle_name = service_handle_name;
	sac_info("handle name :%s", handle->service_handle_name);
	sac_info("uri :%s mask : %d", handle->plugin_uri, handle->enabled_mask);

	service_adaptor_error_s error;
	error.msg = NULL;

	ret = _dbus_start_service(handle, service_flag, security_cookie, &error);

	if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
		service_adaptor_set_last_result(error.code, error.msg); /* LCOV_EXCL_LINE */
		free(error.msg); /* LCOV_EXCL_LINE */
	}

	sac_api_end(ret);
	return ret;
}

int service_plugin_stop(service_plugin_h handle)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	if (NULL == handle) {
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	sac_api_end(ret);
	return ret;
}
