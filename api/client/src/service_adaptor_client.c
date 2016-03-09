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
		FUNC_STOP();
		return -1;
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

/**     @brief  Adds Task in Queue
 *      @return int
 *      @remarks :
 */
int _signal_queue_add_task(int64_t _id,
						uint32_t _callback,
						void *_handle,
						void *user_data)
{
	FUNC_START();

	service_adaptor_task_h task = (service_adaptor_task_h) g_malloc0(sizeof(service_adaptor_task_s));

	if (NULL == task) {
		FUNC_STOP();
		return -1;
	}

	task->id = _id;
	task->callback = _callback;
	task->handle = _handle;
	task->user_data = user_data;

	g_service_adaptor_signal_queue = g_list_append(g_service_adaptor_signal_queue, task);

	FUNC_END();
	return 0;
}

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

/**     @brief  Adds Task in Queue
 *      @return int
 *      @remarks :
 */
int _signal_queue_del_task(service_adaptor_task_h _task)
{
	FUNC_START();
	service_adaptor_task_h target = NULL;

	for (GList *list = g_list_first(g_service_adaptor_signal_queue); list != NULL; list = g_list_next(list)) {
		service_adaptor_task_h data = (service_adaptor_task_h) list->data;

		if ((NULL != data) && (_task == data)) {
			target = data;
			break;
		}
	}

	if (NULL != target) {
		g_service_adaptor_signal_queue = g_list_remove(g_service_adaptor_signal_queue, target);
		g_free(target);
		target = NULL;
	}

	FUNC_END();
	return 0;
}

/**     @brief  Adds Task in Queue
 *      @return int
 *      @remarks :
 */
int _signal_queue_del_task_by_id(int id)
{
	FUNC_START();
	service_adaptor_task_h target = NULL;

	int i, len = g_list_length(g_service_adaptor_signal_queue);
	for (i = 0; i < len; i++) {
		for (GList *list = g_list_first(g_service_adaptor_signal_queue); list != NULL; list = g_list_next(list)) {
			service_adaptor_task_h data = (service_adaptor_task_h) list->data;

			if ((NULL != data) && (id == data->id)) {
				target = data;
				break;
			}
		}

		if (NULL != target) {
			g_service_adaptor_signal_queue = g_list_remove(g_service_adaptor_signal_queue, target);
			g_free(target);
		}
	}

	FUNC_END();
	return 0;
}

/**     @brief  Clears Task in Queue
 *      @return void
 *      @remarks :
 */
void _signal_queue_clear_task()
{
	FUNC_START();
	if (NULL != g_service_adaptor_signal_queue) {
		g_list_free(g_service_adaptor_signal_queue);
	}
	g_service_adaptor_signal_queue = NULL;
	FUNC_END();
}

int service_adaptor_check_handle_validate(service_adaptor_h handle)
{
	if ((NULL == handle) || (NULL == g_service_adaptor)) {
		sac_error("The handle is invalid <user(%p) adaptor(%p)>", handle, g_service_adaptor);
		return -1;
	} else if (handle != g_service_adaptor) {
		sac_error("The handle is invalid <user(%p) adaptor(%p)>", handle, g_service_adaptor);
		return -2;
	} else {
		return 0;
	}
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
		*message = strdup(last_error_message);
		ret = SERVICE_ADAPTOR_ERROR_NONE;
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
		g_mutex_unlock(&connections_counter_mutex);

		return SERVICE_ADAPTOR_ERROR_NOT_SUPPORTED;
	}

	service = (service_adaptor_h) calloc(1, sizeof(service_adaptor_s));

	if (NULL == service) {
		service_adaptor_set_last_result(SERVICE_ADAPTOR_ERROR_UNKNOWN, "Memory allocation failed");
		g_mutex_unlock(&connections_counter_mutex);
		return SERVICE_ADAPTOR_ERROR_UNKNOWN;
	}
/*
	int trd = 0;
	char fingerprint[50] = {0, };
	snprintf(fingerprint, 50, "%s/%d", SERVICE_ADAPTOR_START_KEY_PATH, getpid());
	sac_debug("Trigger open : %s", fingerprint);
	trd = open(fingerprint, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	if (0 > trd) {
		sac_error("Trigger open failed (%d)", trd);
		free(service);
		g_mutex_unlock(&connections_counter_mutex);
		return SERVICE_ADAPTOR_ERROR_INVALID_STATE;
	}
*/
	int dbus_ret = _dbus_client_layer_init();

	if (0 == dbus_ret) {
		connections_counter = connections_counter + 1;
	}

	service_adaptor_error_s error;
	error.msg = NULL;

	ret = _dbus_connect_service_adaptor(&error);

	if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
		service_adaptor_set_last_result(error.code, error.msg);
		free(service);
/*
		close(trd);
		remove(fingerprint);
*/
		g_mutex_unlock(&connections_counter_mutex);
		free(error.msg);
		return ret;
	}

	service->on_signal = NULL;

/*	service->plugins = NULL; */
/*	service->plugin_count = 0; */

	*handle = service;
	g_service_adaptor = service;
/*
	close(trd);
	remove(fingerprint);
*/
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
		connections_counter = 0;
		g_service_adaptor = NULL;
		g_mutex_unlock(&connections_counter_mutex);

		ret = SERVICE_ADAPTOR_ERROR_UNKNOWN;
		return ret;
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

/**	@brief	Connects Service Adaptor
 *	@return	int
 *	@remarks :
 */
int service_adaptor_set_state_changed_cb(service_adaptor_h handle,
						service_adaptor_signal_cb callback,
						void *user_data)
{
	FUNC_START();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	if (NULL == handle) {
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	handle->on_signal = callback;
	_signal_queue_add_task(SIGNAL_SERVICE_ADAPTOR, (uint32_t) callback, handle, user_data);

	FUNC_END();
	return ret;
}

int service_adaptor_unset_state_changed_cb(service_adaptor_h handle)
{
	FUNC_START();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	if (NULL == handle) {
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	handle->on_signal = NULL;

	FUNC_END();
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
		service_adaptor_set_last_result(error.code, error.msg);
		free(error.msg);
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
			free(_plugin);
			free(_plugin_uri);
			sac_error("Critical : Memory allocation failed.");
			ret = SERVICE_ADAPTOR_ERROR_UNKNOWN;
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
/*
	GHashTableIter iter;
	gpointer iter_key;
	gpointer iter_value;
	g_hash_table_iter_init(&iter, (GHashTable *)(handle->optional_property));
	while (g_hash_table_iter_next(&iter, &iter_key, &iter_value)) {
		if (NULL != iter_key) {
			if (0 == strcmp((char *)iter_key, key)) {
				ret = SERVICE_ADAPTOR_ERROR_NONE;
				*value = strdup((char *)iter_value);
				break;
			}
		}
	}
*/

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
		char executable_path[1000];
		int executable_path_len = 0;
		executable_path_len = readlink("/proc/self/exe", executable_path, 1000);
		tizen_app_id = strndup(executable_path, executable_path_len);
		type = strdup("etc");
		handle->app_type = CLIENT_APP_TYPE_ETC;
	} else {
		type = strdup("app");
		handle->app_type = CLIENT_APP_TYPE_APPLICATION;
	}

	if (NULL == tizen_app_id) {
		free(type);
		return SERVICE_ADAPTOR_ERROR_UNKNOWN;
	}

	service_handle_name = g_strconcat(tizen_app_id, "?type=", type, "&plugin=", handle->plugin_uri, NULL);

	free(tizen_app_id);
	free(type);

	if (NULL == service_handle_name) {
		sac_error("handle name get failed");
		service_adaptor_set_last_result(SERVICE_ADAPTOR_ERROR_UNKNOWN, "handle name get failed");
		return SERVICE_ADAPTOR_ERROR_UNKNOWN;
	}

	/* TODO replace to real cookie */
	snprintf(security_cookie, SECURITY_SERVER_COOKIE_BUFFER_SIZE, "%020d", (int)getpid());
/*
	int sec_ret = security_server_request_cookie(security_cookie, SECURITY_SERVER_COOKIE_BUFFER_SIZE);
	sac_debug_func("security_cookie : %s (%d)", security_cookie, sec_ret);

	if (('\0' == security_cookie[0]) || sec_ret) {
		sac_error("cookie get failed");
		service_adaptor_set_last_result(SERVICE_ADAPTOR_ERROR_UNKNOWN, "security cookie get failed");
		free(service_handle_name);
		return SERVICE_ADAPTOR_ERROR_UNKNOWN;
	}
*/

	handle->service_handle_name = service_handle_name;
	sac_info("handle name :%s", handle->service_handle_name);
	sac_info("uri :%s mask : %d", handle->plugin_uri, handle->enabled_mask);

	service_adaptor_error_s error;
	error.msg = NULL;

	ret = _dbus_start_service(handle, service_flag, security_cookie, &error);

	if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
		service_adaptor_set_last_result(error.code, error.msg);
		free(error.msg);
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


int service_plugin_is_login_required(service_plugin_h plugin,
						bool *required)
{
	sac_api_start();
	sac_check_param_null(plugin, "plugin");
	sac_check_param_null(required, "required");

	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error = {0ULL, NULL};
	bool _required = false;

	ret = _dbus_is_login_required(plugin, &_required, &error);
	if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
		service_adaptor_set_last_result(error.code, error.msg);
		free(error.msg);
	}

	sac_api_end(ret);
	return ret;
}

int service_plugin_request_login(service_plugin_h plugin,
						service_plugin_login_cb callback,
						void *user_data)
{
	sac_api_start();
	sac_check_param_null(plugin, "plugin");
	sac_check_param_null(callback, "callback");

	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error = {0ULL, NULL};

	ret = _dbus_request_login(plugin, (void *)callback, user_data, &error);
	if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
		service_adaptor_set_last_result(error.code, error.msg);
		free(error.msg);
	}

	sac_api_end(ret);
	return ret;
}

/************************* private feature */

/**	@brief	Connects Service Adaptor
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_connect(service_adaptor_h *handle,
						service_adaptor_signal_cb callback)
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
		sac_error("Handle already connected");
		g_mutex_unlock(&connections_counter_mutex);

		return SERVICE_ADAPTOR_ERROR_UNKNOWN;
	}

	service = (service_adaptor_h) calloc(1, sizeof(service_adaptor_s));

	if (NULL == service) {
		service_adaptor_set_last_result(SERVICE_ADAPTOR_ERROR_UNKNOWN, "Memory allocation failed");
		g_mutex_unlock(&connections_counter_mutex);
		return SERVICE_ADAPTOR_ERROR_UNKNOWN;
	}
/*
	int trd = 0;
	char fingerprint[50] = {0, };
	snprintf(fingerprint, 50, "%s/%d", SERVICE_ADAPTOR_START_KEY_PATH, getpid());
	sac_debug("Trigger open : %s", fingerprint);
	trd = open(fingerprint, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	if (0 > trd) {
		sac_error("Trigger open failed (%d)", trd);
		free(service);
		g_mutex_unlock(&connections_counter_mutex);
		return SERVICE_ADAPTOR_ERROR_INVALID_STATE;
	}
*/
	int dbus_ret = _dbus_client_layer_init();

	if (0 == dbus_ret) {
		sac_info("Proxy creation success");
	}

	service_adaptor_error_s error;
	error.msg = NULL;

	ret = _dbus_connect_service_adaptor(&error);

	if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
		service_adaptor_set_last_result(error.code, error.msg);
		free(service);
/*
		close(trd);
		remove(fingerprint);
*/
		g_mutex_unlock(&connections_counter_mutex);
		free(error.msg);
		return ret;
	}

	service->service_name = NULL;
	service->user_id = NULL;
	service->app_id = NULL;
	service->service_id = 0;
	service->imsi = NULL;
	service->on_signal = callback;
	service->plugin = NULL;
	g_mutex_init(&service->set_auth_mutex);

	*handle = service;
	g_service_adaptor = service;
	sac_info("Connects success handle (%p) instance (%p)", handle, service);
	connections_counter = connections_counter + 1;

	_signal_queue_add_task(SIGNAL_SERVICE_ADAPTOR, (uint32_t) callback, *handle, NULL);
/*
	close(trd);
	remove(fingerprint);
*/
	g_mutex_unlock(&connections_counter_mutex);

	sac_api_end(ret);
	return ret;
}

/**	@brief	Disconnects Service Adaptor
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_disconnect(service_adaptor_h handle)
{
	sac_api_start();
	if (NULL == handle) {
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	g_mutex_lock(&connections_counter_mutex);

	if (0 >= connections_counter) {
		connections_counter = 0;
		g_service_adaptor = NULL;
		g_mutex_unlock(&connections_counter_mutex);

		ret = SERVICE_ADAPTOR_ERROR_UNKNOWN;
		return ret;
	}

	service_adaptor_error_s error;
	error.msg = NULL;

	ret = _dbus_disconnect_service_adaptor(&error);
	if (ret) {
		sac_error("disconnect error : %s", error.msg);
		free(error.msg);
		error.msg = NULL;
	}

	if (NULL != handle) {
		__SAFE_FREE(handle->service_name);
		__SAFE_FREE(handle->user_id);

		__SAFE_FREE(handle->app_id);
		__SAFE_FREE(handle->imsi);

		if (NULL != handle->plugin) {
			__SAFE_FREE(handle->plugin->name);
		}

		__SAFE_FREE(handle->plugin);
		handle->on_signal = NULL;
		__SAFE_FREE(handle);
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

/**	@brief	Sets IMSI
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_set_imsi(service_adaptor_h handle,
						const char *imsi)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	if ((NULL == handle) || (NULL == imsi)) {
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	if (NULL != handle->imsi) {
		free(handle->imsi);
		handle->imsi = NULL;
	}

	handle->imsi = _safe_strdup(imsi);
	if (NULL == handle->imsi) {
		ret = SERVICE_ADAPTOR_ERROR_UNKNOWN;
	}

	sac_api_end(ret);
	return ret;
}

int service_adaptor_set_plugin(service_adaptor_h handle,
						const char *plugin_uri)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	if ((NULL == handle) || (NULL == plugin_uri)) {
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	service_adaptor_plugin_h plugin = NULL;
	plugin = (service_adaptor_plugin_h) calloc(1, sizeof(service_adaptor_plugin_s));
	char *_plugin_uri = strdup(plugin_uri);
	if ((NULL == plugin) || (NULL == _plugin_uri)) {
		free(plugin);
		free(_plugin_uri);
		return SERVICE_ADAPTOR_ERROR_UNKNOWN;
	}

	plugin->name = _plugin_uri;
	plugin->login = true;

	handle->plugin = plugin;

	sac_api_end(ret);
	return ret;
}

/**	@brief	Free plugins
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_free_plugins(service_adaptor_plugin_h * plugins,
						unsigned int plugins_len)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	if ((NULL == plugins) || (0U == plugins_len)) {
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	for (int i = 0; i < plugins_len; i++) {
		if (NULL == plugins[i]) {
			continue;
		}

		free(plugins[i]->name);
		free(plugins[i]);
	}

	free(plugins);
	plugins = NULL;

	sac_api_end(ret);
	return ret;
}

int service_adaptor_external_request(service_adaptor_h handle,
						int service_flag,
						const char *api_uri,
						bundle *in_params,
						bundle **out_params)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	service_adaptor_error_s error;
	error.msg = NULL;

	if ((NULL == handle) || (0 == service_flag) || (NULL == api_uri) || (NULL == in_params) || (NULL == out_params)) {
		service_adaptor_set_last_result(SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER,
				"Invalid Argument : Not null params");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	if (service_adaptor_check_handle_validate(handle)) {
		service_adaptor_set_last_result(SERVICE_ADAPTOR_ERROR_INVALID_STATE, "Invalid handle (Not connected handle)");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}
/*
	if (NULL == handle->service_name) {
		service_adaptor_set_last_result(SERVICE_ADAPTOR_ERROR_INVALID_STATE, "Invalid State");
		return SERVICE_ADAPTOR_ERROR_INVALID_STATE;
	}
*/

	unsigned char *input_str = NULL;
	unsigned char *output_str = NULL;
	int input_len = 0;
	int output_len = 0;

	ret = bundle_encode(in_params, &input_str, &input_len);

	ret = _dbus_external_request(sac_safe_add_string(handle->service_name), service_flag, api_uri, input_str, input_len, &output_str, &output_len, &error);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		service_adaptor_set_last_result(error.code, error.msg);
	} else {
		*out_params = bundle_decode(output_str, output_len);
		if (NULL == *out_params) {
			ret = SERVICE_ADAPTOR_ERROR_NO_DATA;
		}
	}

	sac_api_end(ret);
	return ret;
}

int service_adaptor_external_request_async(service_adaptor_h handle,
						int service_flag,
						const char *api_uri,
						bundle *in_params,
						service_adaptor_external_response_cb callback,
						void *user_data)
{
	service_adaptor_set_last_result(SERVICE_ADAPTOR_ERROR_NOT_SUPPORTED, "Not supported yet (TBD)");
	return SERVICE_ADAPTOR_ERROR_NOT_SUPPORTED;
}
/************************* private feature */
