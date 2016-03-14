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

#include <stdlib.h>
#include <stdio.h>
#include <tzplatform_config.h>

#include "service-adaptor.h"
#include "service-adaptor-push.h"
#include "service-adaptor-message.h"
#include "service-adaptor-type.h"
#include "service-adaptor-log.h"
#include "dbus-ipc.h"
#include "dbus-server.h"
#include "dbus-service-adaptor.h"
#include "dbus-push-adaptor.h"
#include "push-adaptor.h"
#include "message-adaptor.h"
#include "util/service_file_manager.h"

#include <gio/gio.h>


/*************************************************
 *               Type definition
 *************************************************/

/*#define PUSH_PLUGIN_PATH	"/usr/lib/push-adaptor/plugins"*/
#ifdef SA_PUSH_ON_DEMAND_ENABLE
#define SERVICE_ADAPTOR_SERVICE_FILE_PATH	tzplatform_mkpath(TZ_SYS_SHARE, "/service-adaptor/services/")
#define SERVICE_ADAPTOR_PUSH_ENABLED_PATH	tzplatform_mkpath(TZ_SYS_SHARE, "/service-adaptor/.push/")
#endif

#define PUSH_SERVICE_FILE_KEY_PLUGIN_URI	"PluginUri"
#define PUSH_SERVICE_FILE_KEY_APP_ID		"AppId"
#define PUSH_SERVICE_FILE_KEY_SESSION_INFO	"SessionInfo"
#define PUSH_SERVICE_FILE_KEY_BUS_TYPE		"BusType"
#define PUSH_SERVICE_FILE_KEY_BUS_NAME		"BusName"
#define PUSH_SERVICE_FILE_KEY_OBJ_PATH		"ObjectPath"
#define PUSH_SERVICE_FILE_KEY_INTERFACE		"Interface"
#define PUSH_SERVICE_FILE_KEY_METHOD		"Method"
#define PUSH_SERVICE_FILE_KEY_EXEC		"ExecPath"


/*************************************************
 *               Global valuable
 *************************************************/

static GList *g_push_activate_list = NULL;

/* static GMutex g_push_activate_mutex; */

static GMutex g_push_reconnect_mutex;


/*************************************************
 *               Internal function prototype
 *************************************************/

#ifdef SA_PUSH_ON_DEMAND_ENABLE
static push_activate_h _create_push_handle_by_service_file(service_file_h file);

static void _add_push_service_file(const char *service_file_name);

static void _remove_push_service_file(const char *service_file_name);

static void _load_all_push_service_file(void);

static void _unload_all_push_service_file(void);
#endif

static void push_data_dbus_activate_callback(const char *app_id, const char *session_info, long long int timestamp, const char *data, const char *message);


/*************************************************
 *               Public function prototype
 *************************************************/

service_adaptor_internal_error_code_e service_adaptor_ref_enabled_push_services(push_activate_h **services, int *services_len);


/*************************************************
 *               Internal function definition
 *************************************************/
#ifdef SA_PUSH_ON_DEMAND_ENABLE
static push_activate_h _create_push_handle_by_service_file(service_file_h file)
{
	service_adaptor_debug("<Start> %s", __FUNCTION__);
	push_activate_h handle = (push_activate_h) calloc(1, sizeof(push_activate_t));

	int ret = 0;
	char *values[9] = {NULL, };
	if (NULL != handle) {
		ret += service_file_get_string(file, SERVICE_FILE_SECTION_PUSH, PUSH_SERVICE_FILE_KEY_PLUGIN_URI, &values[0]);
		ret += service_file_get_string(file, SERVICE_FILE_SECTION_PUSH, PUSH_SERVICE_FILE_KEY_APP_ID, &values[1]);
		ret += service_file_get_string(file, SERVICE_FILE_SECTION_PUSH, PUSH_SERVICE_FILE_KEY_SESSION_INFO, &values[2]);
		ret += service_file_get_string(file, SERVICE_FILE_SECTION_BUS, PUSH_SERVICE_FILE_KEY_BUS_TYPE, &values[3]);
		ret += service_file_get_string(file, SERVICE_FILE_SECTION_BUS, PUSH_SERVICE_FILE_KEY_BUS_NAME, &values[4]);
		ret += service_file_get_string(file, SERVICE_FILE_SECTION_BUS, PUSH_SERVICE_FILE_KEY_OBJ_PATH, &values[5]);
		ret += service_file_get_string(file, SERVICE_FILE_SECTION_BUS, PUSH_SERVICE_FILE_KEY_INTERFACE, &values[6]);
		ret += service_file_get_string(file, SERVICE_FILE_SECTION_BUS, PUSH_SERVICE_FILE_KEY_METHOD, &values[7]);
		ret += service_file_get_string(file, SERVICE_FILE_SECTION_GENERAL, PUSH_SERVICE_FILE_KEY_EXEC, &values[8]);
	} else {
		service_adaptor_error("Memory allocation failed");
		return NULL;
	}

	GBusType bus_type = G_BUS_TYPE_NONE;
	if (NULL != values[3]) {
		if (0 == strcmp("session", values[3])) {
			bus_type = G_BUS_TYPE_SESSION;
		} else if (0 == strcmp("system", values[3])) {
			bus_type = G_BUS_TYPE_SYSTEM;
		} else {
			service_adaptor_debug("Invalid bus type");
			ret += -10;
		}
	}

	if (ret) {
		service_adaptor_debug("Failed config exist (%d)", ret);
		for (int i = 0; i < 9; i++) {
			free(values[i]);
		}
		free(handle);
		handle = NULL;
	} else {
		handle->plugin_uri = values[0];
		handle->app_id = values[1];
		handle->session_info = values[2];
		handle->bus_type = (int) bus_type;
		free(values[3]);
		handle->bus_name = values[4];
		handle->object_path = values[5];
		handle->interface = values[6];
		handle->method = values[7];
		handle->proxy = NULL;
		handle->exec = values[8];
	}

	service_adaptor_debug("<End> %s", __FUNCTION__);
	return handle;
}

static void _load_all_push_service_file()
{
	service_adaptor_debug("<Start> %s", __FUNCTION__);
	char **files = NULL;
	int len = 0;

	int ret = service_file_get_list(SERVICE_FILE_DIRECTORY_PUSH, &files, &len);
	if (ret) {
		service_adaptor_debug("service file load failed");
		return;
	}

/*	if ((NULL == files) || (0 >= len)) {
		service_adaptor_debug("There is no files");
		return;
	}*/

	service_adaptor_debug_func("<<<<<[LOCK] push-activate mutex");
	g_mutex_lock(&g_push_activate_mutex);
	for (int i = 0; i < len; i++) {
		service_file_h service_file = NULL;
		ret = service_file_load(SERVICE_FILE_DIRECTORY_PUSH, files[i], &service_file);

		if (0 == ret && NULL != service_file) {
			push_activate_h _data = NULL;
			_data = _create_push_handle_by_service_file(service_file);

			if (NULL != _data) {
				service_adaptor_debug("push service file load success");
				g_push_activate_list = g_list_append(g_push_activate_list, (void *)_data);
				_data->file_name = files[i];
			} else {
				service_adaptor_debug("Config data load failed");
				free(files[i]);
			}

		} else {
			service_adaptor_debug("File load failed");
			free(files[i]);
		}
		ret = service_file_unload(service_file);
	}
	g_mutex_unlock(&g_push_activate_mutex);
	service_adaptor_debug_func(">>>>>[UNLOCK] push-activate mutex");

	free(files);
	service_adaptor_debug("<End> %s", __FUNCTION__);
}


static void _unload_all_push_service_file(void)
{
	service_adaptor_debug("<Start> %s", __FUNCTION__);

	service_adaptor_debug_func("<<<<<[LOCK] push-activate mutex");
	g_mutex_lock(&g_push_activate_mutex);
	int len = g_list_length(g_push_activate_list);

	for (int i = 0; i < len; i++) {
		push_activate_h handle = (push_activate_h) g_list_nth_data(g_push_activate_list, i);

		free(handle->file_name);
		free(handle->plugin_uri);
		free(handle->app_id);
		free(handle->session_info);
		free(handle->bus_name);
		free(handle->object_path);
		free(handle->interface);
		free(handle->method);
		if (NULL != handle->proxy) {
			g_object_unref(((GDBusProxy *)(handle->proxy)));
		}
		free(handle->exec);
		free(handle);
	}

	g_list_free(g_push_activate_list);
	g_push_activate_list = NULL;
	g_mutex_unlock(&g_push_activate_mutex);
	service_adaptor_debug_func(">>>>>[UNLOCK] push-activate mutex");
	service_adaptor_debug("<End> %s", __FUNCTION__);
}


static void _add_push_service_file(const char *service_file_name)
{
	service_adaptor_debug("<Start> %s", __FUNCTION__);

	service_adaptor_debug_func("<<<<<[LOCK] push-activate mutex");
	g_mutex_lock(&g_push_activate_mutex);

	char *file_name = strdup(service_file_name);

	if (NULL == file_name) {
		service_adaptor_debug("Memory allocation failed");
		return;
	}

	service_file_h service_file = NULL;
	int ret = service_file_load(SERVICE_FILE_DIRECTORY_PUSH, file_name, &service_file);

	if (0 == ret && NULL != service_file) {
		push_activate_h _data = NULL;
		_data = _create_push_handle_by_service_file(service_file);

		if (NULL != _data) {
			service_adaptor_debug("push service file load success");
			g_push_activate_list = g_list_append(g_push_activate_list, (void *)_data);
			_data->file_name = file_name;
		} else {
			service_adaptor_debug("Config data load failed");
			free(file_name);
		}

	} else {
		service_adaptor_debug("File load failed");
		free(file_name);
	}

	ret = service_file_unload(service_file);

	g_mutex_unlock(&g_push_activate_mutex);
	service_adaptor_debug_func(">>>>>[UNLOCK] push-activate mutex");
	service_adaptor_debug("<End> %s", __FUNCTION__);
}

static void _remove_push_service_file(const char *service_file)
{
	service_adaptor_debug("<Start> %s", __FUNCTION__);

	service_adaptor_debug_func("<<<<<[LOCK] push-activate mutex");
	g_mutex_lock(&g_push_activate_mutex);

	push_activate_h handle = NULL;

	int len = g_list_length(g_push_activate_list);

	for (int i = 0; i < len; i++) {
		push_activate_h _data = (push_activate_h) g_list_nth_data(g_push_activate_list, i);
		if (0 == strcmp(_data->file_name, service_file)) {
			handle = _data;
			break;
		}
	}

	if (NULL != handle) {
		g_push_activate_list = g_list_remove(g_push_activate_list, (void *) handle);

		free(handle->file_name);
		free(handle->plugin_uri);
		free(handle->app_id);
		free(handle->session_info);
		free(handle->bus_name);
		free(handle->object_path);
		free(handle->interface);
		free(handle->method);
		if (NULL != handle->proxy) {
			g_object_unref(((GDBusProxy *)(handle->proxy)));
		}
		free(handle->exec);
		free(handle);
	}

	g_mutex_unlock(&g_push_activate_mutex);
	service_adaptor_debug_func(">>>>>[UNLOCK] push-activate mutex");

	service_adaptor_debug("<End> %s", __FUNCTION__);
}
#endif


/***********************************************************
 * Push adaptor callback
 ***********************************************************/

void service_adaptor_push_adaptor_on_notification_received(push_adaptor_notification_data_h notification,
						void *user_data)
{
	service_adaptor_debug("Push adaptor notification callback");

	if ((NULL == notification) || (NULL == notification->session_info)) {
		service_adaptor_error("Could not get session info");
		return;
	}

	service_adaptor_info("(%s) - Data: %s\n MSG: %s\n Time:%lld", notification->session_info, notification->data, notification->msg, notification->time_stamp);

	uint32_t service_id = 0;

	service_adaptor_h service_adaptor = service_adaptor_get_handle();
	if (0 == strncmp(notification->session_info, "0", 1)) {
		service_id = 0;
	} else if (0 == strncmp(notification->session_info, "1", 1)) {
		service_id = 1;

		message_adaptor_h adaptor = service_adaptor_get_message_adaptor(service_adaptor);

		if (NULL == adaptor) {
			service_adaptor_error("Invalid Param");
			return;
		}

		for (GList * list = g_list_first(service_adaptor->service_list); list != NULL; list = g_list_next(list)) {
			int ret = 0;
			char *decode_msg = NULL;
			message_adaptor_error_code_h error_code = NULL;

			service_adaptor_service_context_h service = (service_adaptor_service_context_h) list->data;
			if ((NULL == service) || (NULL == service->message_context)) {
				continue;
			}

			message_adaptor_plugin_h plugin = message_adaptor_get_plugin_by_name(adaptor, service->plugin_uri);

			if (NULL == plugin) {
				service_adaptor_error("Could not find a plugin");
				continue;
			}

			ret = message_adaptor_decode_push_message(plugin, service->message_context, notification->data, &decode_msg, &error_code);

			if (MESSAGE_ADAPTOR_ERROR_NONE == ret) {
				free(notification->data);
				notification->data = decode_msg;
				break;
			}

			if (NULL != error_code) {
				message_adaptor_destroy_error_code(&error_code);
			}
		}

	} else if (0 == strncmp(notification->session_info, "2", 1)) {
		service_id = 2;
	} else {
		service_adaptor_error("Could not get session info");
		return;
	}

	service_adaptor_internal_error_t error;
	error.code = 0;
	error.msg = strdup("");


	dbus_push_data_callback(service_id, notification, &error, NULL);
	free(error.msg);

	push_data_dbus_activate_callback(notification->app_id, notification->session_info, notification->time_stamp, notification->data, notification->msg);
}

push_adaptor_h service_adaptor_get_push_adaptor(service_adaptor_h service_adaptor)
{
	service_adaptor_debug("Get push adaptor");

	if ((void *) NULL == service_adaptor) {
		service_adaptor_error("Invalid argument");
		return NULL;
	}

	return service_adaptor->push_handle;
}

push_adaptor_plugin_context_h service_adaptor_get_push_context(service_adaptor_h service_adaptor,
						const char *imsi,
						const char *app_id)
{
	service_adaptor_debug("Get push context");

	return NULL;
}

service_adaptor_internal_error_code_e service_adaptor_connect_push_plugin(service_adaptor_h service_adaptor,
						service_adaptor_service_context_h service,
						char *ret_msg)
{
	service_adaptor_debug("Connect to push plugin");

	if ((NULL == service_adaptor) || (NULL == service)) {
		service_adaptor_error("Invalid parameter");
		snprintf(ret_msg, 2048, "push plugin connect failed [%d]", (int)__LINE__);
		return SERVICE_ADAPTOR_INTERNAL_ERROR_INVALID_ARGUMENT;
	}

	push_adaptor_h adaptor = service_adaptor_get_push_adaptor(service_adaptor);
	push_adaptor_plugin_h plugin = push_adaptor_get_plugin_by_name(adaptor, service->plugin_uri);

	if (NULL == plugin) {
		service_adaptor_warning("Invalid plugin");
		snprintf(ret_msg, 2048, "push plugin connect failed [%d]", (int)__LINE__);
		return SERVICE_ADAPTOR_INTERNAL_ERROR_INVALID_ARGUMENT;
	} else if (NULL == service->context_info) {
		service_adaptor_warning("Invalid service->context_info");
		snprintf(ret_msg, 2048, "push plugin connect failed [%d]", (int)__LINE__);
		return SERVICE_ADAPTOR_INTERNAL_ERROR_INVALID_ARGUMENT;
	} else if (NULL == service->context_info->app_id) {
		service_adaptor_warning("Invalid app_id");
		snprintf(ret_msg, 2048, "push plugin connect failed [%d]", (int)__LINE__);
		return SERVICE_ADAPTOR_INTERNAL_ERROR_INVALID_ARGUMENT;
	}

	push_adaptor_plugin_context_h push_context = push_adaptor_create_plugin_context(plugin,
			service->plugin_uri, service->context_info->app_id);

	if (NULL == push_context) {
		service_adaptor_error("Could not create plugin context: %s", service->service_name);
		snprintf(ret_msg, 2048, "push plugin connect failed [%d]", (int)__LINE__);
		return SERVICE_ADAPTOR_INTERNAL_ERROR_CREATE;
	}
/*
	// Set server info
	int ret = 0;
	push_adaptor_error_code_h error = NULL;
	ret = push_adaptor_set_server_info(plugin, push_context, service->server_info, NULL, &error, NULL);
	if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
		service_adaptor_warning("Could not set push plugin server information: %d", ret);
		if (NULL != error) {
			service_adaptor_warning("[%lld] %s", error->code, error->msg);
		}
		push_adaptor_destroy_error_code(&error);
	}
*/
	push_adaptor_error_code_h error = NULL;
	SERVICE_ADAPTOR_API_TIME_CHECK_PAUSE();
	SERVICE_ADAPTOR_PLUGIN_API_TIME_CHECK_START(SA_TIME_CHECK_FLAG_PUSH);

	service_adaptor_debug_func("<<<<<[[[[[[LOCK] push-reconnect mutex");
	g_mutex_lock(&g_push_reconnect_mutex);

	push_adaptor_connect(plugin, push_context, &error);

	g_mutex_unlock(&g_push_reconnect_mutex);
	service_adaptor_debug_func(">>>>>[UNLOCK] push-reconnect mutex");

	SERVICE_ADAPTOR_PLUGIN_API_TIME_CHECK_PAUSE(SA_TIME_CHECK_FLAG_PUSH);
	SERVICE_ADAPTOR_API_TIME_CHECK_START();

	service->push_context = push_context;
	service->connected |= 0x1000000;
	push_adaptor_destroy_error_code(&error);
	service_adaptor_debug("Connected to push plugin");

	return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
}

service_adaptor_internal_error_code_e service_adaptor_disconnect_push_plugin(service_adaptor_h service_adaptor,
						service_adaptor_service_context_h service)
{
	service_adaptor_debug("Disconnect from push plugin");

	if (NULL != service) {
		if (service->push_context) {
			service_adaptor_debug("Input NULL to push_context");
			service->push_context = NULL;
		}
	}

	service_adaptor_debug("Disconnected from push plugin");

	return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
}

service_adaptor_internal_error_code_e service_adaptor_reconnect_push_plugin(service_adaptor_h service_adaptor,
						service_adaptor_service_context_h service)
{
	service_adaptor_info("Reconnect push plugin");

	if ((NULL != service) && (NULL != service->push_context)) {
		push_adaptor_h adaptor = service_adaptor_get_push_adaptor(service_adaptor);
		push_adaptor_plugin_h plugin = push_adaptor_get_plugin_by_name(adaptor, service->plugin_uri);
		if (NULL != plugin) {
			push_adaptor_error_code_h error = NULL;
			push_error_code_t push_ret;

			service_adaptor_debug_func("<<<<<[LOCK] push-reconnect mutex");
			g_mutex_lock(&g_push_reconnect_mutex);
			int connected = 0;
			push_ret = push_adaptor_is_connected(plugin, service->push_context, &connected);

			if (connected != PUSH_ADAPTOR_STATE_CONNECTED) {
				service_adaptor_debug("Disconnect push");

				push_ret = push_adaptor_disconnect(plugin, service->push_context, &error);
				service_adaptor_debug_func("Disconnect ret (%d)", push_ret);
				if (error) {
					service_adaptor_error("error : %lld, %s", (long long int) error->code, error->msg);
					push_adaptor_destroy_error_code(&error);
					error = NULL;
				}

				service_adaptor_debug("Connect push");
				push_ret = push_adaptor_connect(plugin, service->push_context, &error);
				service_adaptor_debug_func("Connect ret (%d)", push_ret);
			} else {
				service_adaptor_debug("Keep legacy connection");
			}
			g_mutex_unlock(&g_push_reconnect_mutex);
			service_adaptor_debug_func(">>>>>[UNLOCK] push-reconnect mutex");

			if (error) {
				service_adaptor_error("error : %lld, %s", (long long int) error->code, error->msg);
				push_adaptor_destroy_error_code(&error);
				error = NULL;
			}
			if (PUSH_ADAPTOR_ERROR_NONE != push_ret) {
				return SERVICE_ADAPTOR_INTERNAL_ERROR_ADAPTOR_INTERNAL;
			}
		} else {
			service_adaptor_warning("There is no push plugin (uri:%s)", service->plugin_uri);
			return SERVICE_ADAPTOR_INTERNAL_ERROR_ADAPTOR_INTERNAL;
		}
	} else {
		service_adaptor_warning("There is no push context (%p)(%p)", service, (service ? service->push_context : NULL));
		return SERVICE_ADAPTOR_INTERNAL_ERROR_ADAPTOR_INTERNAL;
	}

	service_adaptor_info("Reconnected push plugin");
	return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
}

push_adaptor_h service_adaptor_create_push()
{
	push_adaptor_h push_adaptor = push_adaptor_create(PUSH_PLUGIN_PATH);

	if ((void *) NULL == push_adaptor) {
		service_adaptor_error("Could not create push adaptor");
		return NULL;
	}

	service_adaptor_debug("Push adaptor created");

#ifdef SA_PUSH_ON_DEMAND_ENABLE
	_load_all_push_service_file();
#endif

	return push_adaptor;
}

void service_adaptor_destroy_push(push_adaptor_h push_handle)
{
	if ((void *) NULL == push_handle) {
		service_adaptor_error("Could not create push adaptor");
		return;
	}

	push_adaptor_destroy(push_handle);

	service_adaptor_debug("Push adaptor destroied");

#ifdef SA_PUSH_ON_DEMAND_ENABLE
	_unload_all_push_service_file();
#endif
}


push_adaptor_listener_h service_adaptor_register_push_listener(push_adaptor_h push_adaptor)
{
	if ((void *) NULL == push_adaptor) {
		service_adaptor_error("Could not create push adaptor");
		return NULL;
	}

	push_adaptor_listener_h push_listener =
			(push_adaptor_listener_h) malloc(sizeof(push_adaptor_listener_t));

	if ((void *) NULL == push_listener) {
		service_adaptor_error("Could not create push listener");
		return NULL;
	}

	push_listener->_on_notification_received = service_adaptor_push_adaptor_on_notification_received;

	push_adaptor_register_listener(push_adaptor, push_listener);
	service_adaptor_debug("Push adaptor listener created");

	return push_listener;
}

service_adaptor_internal_error_code_e service_adaptor_push_register(const char *service_file, char **error_msg)
{
	service_adaptor_debug("<Start> %s", __FUNCTION__);
	service_adaptor_internal_error_code_e ret = SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;

	if (NULL == service_file) {
		*error_msg = strdup("Invalid service file name");
		return SERVICE_ADAPTOR_INTERNAL_ERROR_INVALID_ARGUMENT;
	}

#ifdef SA_PUSH_ON_DEMAND_ENABLE
	char *src_path = g_strconcat(SERVICE_ADAPTOR_SERVICE_FILE_PATH, service_file, NULL);
	char *dst_path = g_strconcat(SERVICE_ADAPTOR_PUSH_ENABLED_PATH, service_file, NULL);

	if ((NULL == src_path) || (NULL == dst_path)) {
		free(src_path);
		free(dst_path);
		*error_msg = strdup("Critical : Memory allocation failed");
		return SERVICE_ADAPTOR_INTERNAL_ERROR_ADAPTOR_INTERNAL;
	}

	int r = 0;
	if ((r = access(src_path, O_RDONLY))) {
		service_adaptor_error("File access failed (readonly)(%d): %s", r, service_file);
		*error_msg = strdup("File access failed (readonly)");
		ret = SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_FOUND;
	} else {
		remove(dst_path);
		_remove_push_service_file(service_file);
		if ((r = symlink(src_path, dst_path))) {
			service_adaptor_error("push register failed(%d): %s", r, service_file);
			*error_msg = strdup("Push register failed (symlink fail)");
			ret = SERVICE_ADAPTOR_INTERNAL_ERROR_ADAPTOR_INTERNAL;
		} else {
			_add_push_service_file(service_file);
			service_adaptor_info("push registered: %s", service_file);
		}
	}

	free(src_path);
	free(dst_path);
#endif

	service_adaptor_debug("<End> %s", __FUNCTION__);
	return ret;
}

service_adaptor_internal_error_code_e service_adaptor_push_deregister(const char *service_file, char **error_msg)
{
	service_adaptor_debug("<Start> %s", __FUNCTION__);
	service_adaptor_internal_error_code_e ret = SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;

	if (NULL == service_file) {
		*error_msg = strdup("Invalid service file name");
		return SERVICE_ADAPTOR_INTERNAL_ERROR_INVALID_ARGUMENT;
	}

#ifdef SA_PUSH_ON_DEMAND_ENABLE
	char *dst_path = g_strconcat(SERVICE_ADAPTOR_PUSH_ENABLED_PATH, service_file, NULL);

	if (NULL == dst_path) {
		free(dst_path);
		*error_msg = strdup("Critical : Memory allocation failed");
		return SERVICE_ADAPTOR_INTERNAL_ERROR_ADAPTOR_INTERNAL;
	}

	remove(dst_path);
	_remove_push_service_file(service_file);
	service_adaptor_info("push deregistered: %s", service_file);

	free(dst_path);
#endif

	service_adaptor_debug("<End> %s", __FUNCTION__);
	return ret;
}

static void push_data_dbus_activate_callback(const char *app_id,
						const char *session_info,
						long long int timestamp,
						const char *push_data,
						const char *message)
{
	service_adaptor_debug("<Start> %s", __FUNCTION__);
	int len = g_list_length(g_push_activate_list);

	push_activate_h data = NULL;

	service_adaptor_debug("Registered push clitns (%d)", len);
	for (int i = 0; i < len; i++) {
		data = (push_activate_h) g_list_nth_data(g_push_activate_list, i);
		if ((0 == strcmp(app_id, data->app_id)) &&
				(0 == strcmp(session_info, data->session_info))) {
			service_adaptor_debug("Catched push client (%s)(%s)", app_id, session_info);
			dbus_send_to_push_with_activation(data->bus_type, data->bus_name,
					data->object_path, data->interface, data->method, &(data->proxy),
					timestamp, push_data, message);
		}
	}
	service_adaptor_debug("<End> %s", __FUNCTION__);
}

service_adaptor_internal_error_code_e service_adaptor_ref_enabled_push_services(push_activate_h **services, int *services_len)
{
	service_adaptor_internal_error_code_e ret = SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
	if ((NULL == services) || (NULL == services_len)) {
		return SERVICE_ADAPTOR_INTERNAL_ERROR_INVALID_ARGUMENT;
	}

	service_adaptor_debug("<Start> %s", __FUNCTION__);
	int len = g_list_length(g_push_activate_list);

	push_activate_h data = NULL;
	push_activate_h svc_data = NULL;
	GList *svc = NULL;

	service_adaptor_debug("Registered push clitns (%d)", len);
	for (int i = 0; i < len; i++) {
		data = (push_activate_h) g_list_nth_data(g_push_activate_list, i);

		bool exist = false;
		int jlen = g_list_length(svc);
		for (int j = 0; j < jlen; j++) {
			svc_data = (push_activate_h) g_list_nth_data(svc, j);
			if ((0 == strcmp(data->plugin_uri, svc_data->plugin_uri)) &&
					(0 == strcmp(data->app_id, svc_data->app_id))) {
				exist = true;
				break;
			}
		}
		if (false == exist) {
			svc = g_list_append(svc, (void *) data);
			service_adaptor_debug_func("Appended : plugin_uri(%s) app_id(%s)", data->plugin_uri, data->app_id);
		}
	}

	int res_len = g_list_length(svc);
	service_adaptor_debug("Enabled query res len(%d)", res_len);

	push_activate_h *acts = NULL;
	if (0 < res_len) {
		acts = (push_activate_h *) calloc(res_len, sizeof(push_activate_h));
		if (NULL != acts) {
			for (int k = 0; k < res_len; k++) {
				acts[k] = (push_activate_h) g_list_nth_data(svc, k);
			}
		} else {
			service_adaptor_error("Memory allocation failed");
			ret = SERVICE_ADAPTOR_INTERNAL_ERROR_ADAPTOR_INTERNAL;
		}
	} else {
		service_adaptor_debug("There is no enabled push service");
		ret = SERVICE_ADAPTOR_INTERNAL_ERROR_NO_DATA;
	}
	g_list_free(svc);

	*services = acts;
	*services_len = res_len;

	service_adaptor_debug("<End> %s", __FUNCTION__);
	return ret;
}
