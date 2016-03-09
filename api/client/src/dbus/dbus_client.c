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
 * File: dbus-client.c
 * Desc:
 * Created on: Oct, 2014
 * Auth: Yongjin Kim <youth.kim@samsung.com>
 *
 *****************************************************************************/

#include <gio/gio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <glib-object.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <dbus-server.h>

#include "service_adaptor_client_type.h"
#include "service_adaptor_client_log.h"
#include "service_adaptor_client_private.h"
#include "dbus_client.h"
#include "dbus_client_storage.h"
#include "dbus_client_message.h"
#include "dbus_client_push.h"

#include "private/service-adaptor-client.h"
#include "util/service_adaptor_client_util.h"

/******************************************************************************
 * Global variables and defines
 ******************************************************************************/

/**
 * Service Adaptor D-Bus client thread data
 */
typedef struct {
	GMutex connection_mutex;	/* Mutex used to protect connection status data */
	GCond connection_cond;		/* Conditional variable used to signal that connection was established */
	int connection_cond_signaled;	/* Additional variable used to signal that connection was established */
} dbus_service_adaptor_client_thread_data_s;

/**
 * D-Bus client thread.
 */
static GThread *dbusClientThread = NULL;

/**
 * D-Bus client thread main loop context.
 */
static GMainContext *dbusClientMainContext = NULL;

/**
 * D-Bus client thread main loop.
 */
static GMainLoop *dbusClientMainLoop = NULL;

/**
 * Service Adaptor D-Bus bus watcher id.
 */
static guint watcher_id = 0;

/**
 * D-Bus connection to vService Channel
 */
static GDBusConnection *connection = NULL;

/**
 * D-Bus proxy to Service Adaptor Client interface
 */
static GDBusProxy *sac_interface_proxy = NULL;

/******************************************************************************
 * Private interface
 ******************************************************************************/

struct _internal_service_signal_data_s {
	uint64_t code;
	char *msg;
	service_adaptor_task_h task;
};

static void *_on_dbus_disappeared_cb(void *data);

/**
 * @brief D-Bus client thread function.
 *
 * D-Bus client thread function. It initialises all client D-Bus interfaces.
 * @param data Data passed to thread.
 * @return Result data from thread (always NULL in this implementation).
 */
static gpointer __dbus_service_adaptor_client_thread(gpointer data);

/**
 * @brief Initialises Service Adaptor D-Bus client internal structures.
 *
 * Initialises Service Adaptor D-Bus client internal structures.
 * @param thread_data Pointer to thread data used to signal that connection was successfully established.
 * @return 0 on success, error code when some structures could not be initialised.
 */
static int __dbus_connection_init(dbus_service_adaptor_client_thread_data_s *thread_data);

/**
 * @brief Deinitialises Service Adaptor D-Bus client internal structures.
 *
 * Deinitialises Service Adaptor D-Bus client internal structures.
 */
static void __dbus_connection_deinit();

/**
 * @brief Service Adaptor availability callback function.
 *
 * Service Adaptor availability callback function. Called when Service Adaptor appears at D-Bus bus.
 * @param connection D-Bus connection.
 * @param name The name being watched.
 * @param name_owner Unique name of the owner of the name being watched.
 * @param user_data User data passed to g_bus_watch_name().
 */
static void on_name_appeared(GDBusConnection *connection,
						const gchar *name,
						const gchar *name_owner,
						gpointer user_data);

/**
 * @brief Service Adaptor availability callback function.
 *
 * Service Adaptor availability callback function. Called when Service Adaptor disappears from D-Bus bus.
 * @param connection D-Bus connection.
 * @param name he name being watched.
 * @param user_data User data passed to g_bus_watch_name().
 */
static void on_name_vanished(GDBusConnection *connection,
						const gchar *name,
						gpointer user_data);

/**
 * @brief Service Adaptor signals handler.
 *
 * Service Adaptor signals handler. It is called when Service Adaptor sends signal over D-Bus.
 * @param proxy D-Bus proxy object.
 * @param sender_name The unique bus name of the remote caller.
 * @param signal_name Signal name.
 * @param parameters Signal parameters.
 * @param user_data The user_data gpointer.
 */
static void on_signal(GDBusProxy *proxy,
						gchar *sender_name,
						gchar *signal_name,
						GVariant *parameters,
						gpointer user_data);

/******************************************************************************
 * Private interface definition
 ******************************************************************************/

static gpointer __dbus_service_adaptor_client_thread(gpointer data)
{
	FUNC_START();
	int ret = 0;

	dbusClientMainContext = g_main_context_new();
	dbusClientMainLoop = g_main_loop_new(dbusClientMainContext, FALSE);
	g_main_context_push_thread_default(dbusClientMainContext);

	ret = __dbus_connection_init(data);

	if (0 == ret) {
		g_main_loop_run(dbusClientMainLoop);
		__dbus_connection_deinit();
	} else {
		/* Already released dbus setting (=__dbus_connection_deinit()) */
	}

	g_main_context_pop_thread_default(dbusClientMainContext);
	g_main_loop_unref(dbusClientMainLoop);
	dbusClientMainLoop = NULL;
	g_main_context_unref(dbusClientMainContext);
	dbusClientMainContext = NULL;

	FUNC_END();
	return data;
}

static int __dbus_connection_init(dbus_service_adaptor_client_thread_data_s *thread_data)
{
	FUNC_START();
	GError *error = NULL;

	if ((NULL != connection) || (sac_interface_proxy)) {
		FUNC_STOP();
		return -1;
	}

	connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);

	if (NULL == connection) {
		g_error_free(error);

		FUNC_STOP();
		return -1;
	} else {
		sac_interface_proxy = g_dbus_proxy_new_sync(connection,
				G_DBUS_PROXY_FLAGS_NONE,
				NULL,
				SERVICE_ADAPTOR_BUS_NAME,
				SERVICE_ADAPTOR_OBJECT_PATH,
				SERVICE_ADAPTOR_INTERFACE,
				NULL,
				&error);

		if (NULL == sac_interface_proxy) {
			g_error_free(error);

			g_object_unref(connection);
			connection = NULL;

			FUNC_STOP();
			return -1;
		}
	}

	watcher_id = g_bus_watch_name(G_BUS_TYPE_SYSTEM,
			SERVICE_ADAPTOR_BUS_NAME,
			G_BUS_NAME_WATCHER_FLAGS_NONE,
			on_name_appeared,
			on_name_vanished,
			thread_data,
			NULL);

	if (0 == watcher_id) {
		g_object_unref(sac_interface_proxy);
		sac_interface_proxy = NULL;

		g_object_unref(connection);
		connection = NULL;

		FUNC_STOP();
		return -1;
	}

	int res = g_signal_connect(sac_interface_proxy,
			"g-signal",
			G_CALLBACK(on_signal),
			NULL);

	if (0 == res) {
		g_object_unref(sac_interface_proxy);
		sac_interface_proxy = NULL;

		g_object_unref(connection);
		connection = NULL;

		g_bus_unwatch_name(watcher_id);
		watcher_id = 0;

		FUNC_STOP();
		return -1;
	}

	FUNC_END();
	return 0;
}

static void __dbus_connection_deinit()
{
	FUNC_START();
	if (NULL != sac_interface_proxy) {
		g_object_unref(sac_interface_proxy);
		sac_interface_proxy = NULL;
	}

	if (NULL != connection) {
		g_object_unref(connection);
		connection = NULL;
	}

	if (0 != watcher_id) {
		g_bus_unwatch_name(watcher_id);
		watcher_id = 0;
	}
}

static void on_name_appeared(GDBusConnection *connection,
						const gchar *name,
						const gchar *name_owner,
						gpointer user_data)
{
	FUNC_START();
	dbus_service_adaptor_client_thread_data_s *thread_data = (dbus_service_adaptor_client_thread_data_s *)user_data;

	if (NULL != thread_data) {
		g_mutex_lock(&thread_data->connection_mutex);
		thread_data->connection_cond_signaled = 1;
		g_cond_signal(&thread_data->connection_cond);
		g_mutex_unlock(&thread_data->connection_mutex);
	}
	FUNC_END();
}

static void on_name_vanished(GDBusConnection *connection,
						const gchar *name,
						gpointer user_data)
{
	FUNC_START();
	dbus_service_adaptor_client_thread_data_s *thread_data = (dbus_service_adaptor_client_thread_data_s *)user_data;

	if ((NULL != thread_data) && (thread_data->connection_cond_signaled)) { /* appeared -> vanished */
		if (0 != watcher_id) {
			g_bus_unwatch_name(watcher_id);
			watcher_id = 0;
		}
		pthread_t th;
		pthread_create(&th, NULL, _on_dbus_disappeared_cb, NULL);
	}
	FUNC_END();
}

static void _service_signal_emitter(void *data)
{
	FUNC_START();
	struct _internal_service_signal_data_s *signal_context =
		(struct _internal_service_signal_data_s *) data;

	service_adaptor_signal_cb callback = (service_adaptor_signal_cb) signal_context->task->callback;

	if (NULL != callback) {
		callback(signal_context->task->handle, (service_adaptor_signal_code_e) signal_context->code, signal_context->msg);/*, signal_context->task->user_data); */
	}

	g_free(signal_context->msg);
	g_free(signal_context);
	signal_context = NULL;

	g_thread_unref(g_thread_self());
	FUNC_END();
}

static void on_service_signal(GDBusProxy *proxy,
						gchar *sender_name,
						gchar *signal_name,
						GVariant *parameters,
						gpointer user_data)
{
	FUNC_START();

	if (0 == g_strcmp0(signal_name, DBUS_SERVICE_ADAPTOR_SIGNAL)) {
		service_adaptor_task_h task = _signal_queue_get_task(SIGNAL_SERVICE_ADAPTOR);

		if (NULL == task) {
			FUNC_STOP();
			return;
		}

		struct _internal_service_signal_data_s *signal_context =
			(struct _internal_service_signal_data_s *)g_malloc0(sizeof(struct _internal_service_signal_data_s));

		if (NULL == signal_context) {
			FUNC_STOP();
			return;
		} else {
			signal_context->msg = NULL;
			signal_context->task = NULL;
		}

		GVariant *call_result[2];
		call_result[0] = g_variant_get_child_value(parameters, 0);
		call_result[1] = g_variant_get_child_value(parameters, 1);

		signal_context->code = g_variant_get_uint64(call_result[0]);
		signal_context->msg = ipc_g_variant_dup_string(call_result[1]);
		signal_context->task = task;


		GError *worker_error = NULL;
		g_thread_try_new("service-adaptor-client-signal-emitter", (GThreadFunc) _service_signal_emitter,
				(void *) signal_context, &worker_error);

		g_clear_error(&worker_error);
		worker_error = NULL;
	}

	FUNC_END();
}

static void on_signal(GDBusProxy *proxy,
						gchar *sender_name,
						gchar *signal_name,
						GVariant *parameters,
						gpointer user_data)
{
	if (0 == strncmp(signal_name, DBUS_SERVICE_ADAPTOR, DBUS_NAME_LENGTH)) {
		on_service_signal(proxy,
				sender_name,
				signal_name,
				parameters,
				user_data);
	} else if (0 == strncmp(signal_name, DBUS_MESSAGE_ADAPTOR, DBUS_NAME_LENGTH)) {
		on_message_signal(proxy,
				sender_name,
				signal_name,
				parameters,
				user_data);
	} else if (0 == strncmp(signal_name, DBUS_STORAGE_ADAPTOR, DBUS_NAME_LENGTH)) {
		on_storage_signal(proxy,
				sender_name,
				signal_name,
				parameters,
				user_data);
	} else if (0 == strncmp(signal_name, DBUS_PUSH_ADAPTOR, DBUS_NAME_LENGTH)) {
		on_push_signal(proxy,
				sender_name,
				signal_name,
				parameters,
				user_data);
	}

}

/******************************************************************************
 * Public interface definition
 ******************************************************************************/

GDBusProxy *_dbus_get_sac_interface_proxy()
{
	return sac_interface_proxy;
}

/**	@brie
 *	@return	int
 *	@remarks :
 */
int _dbus_client_service_adaptor_init()
{
	FUNC_START();
#if !GLIB_CHECK_VERSION(2, 32, 0)
	g_thread_init(NULL);
#endif
#if !GLIB_CHECK_VERSION(2, 35, 0)
	g_type_init();
#endif

	if (NULL != dbusClientThread) {
		/* D-Bus client thread is already running */
		FUNC_STOP();
		return -1;
	}

	dbus_service_adaptor_client_thread_data_s *thread_data =
			(dbus_service_adaptor_client_thread_data_s *) calloc(1, sizeof(dbus_service_adaptor_client_thread_data_s));

	if (NULL == thread_data) {
		FUNC_STOP();
		return -1;
	}

	g_mutex_init(&thread_data->connection_mutex);
	g_cond_init(&thread_data->connection_cond);
	thread_data->connection_cond_signaled = 0;

	dbusClientThread = g_thread_new("Service Adaptor D-Bus Client Thread", __dbus_service_adaptor_client_thread, thread_data);

	gint64 timeout = g_get_monotonic_time() + 15 * G_TIME_SPAN_SECOND;
	g_mutex_lock(&thread_data->connection_mutex);
	while (!thread_data->connection_cond_signaled) {
		if (!g_cond_wait_until(&thread_data->connection_cond, &thread_data->connection_mutex, timeout)) {
			/* timeout */
			g_mutex_unlock(&thread_data->connection_mutex);

			FUNC_STOP();
			return -1;
		}
	}
	g_mutex_unlock(&thread_data->connection_mutex);

	FUNC_END();
	return 0;
}

/**	@brief
 *	@return	void
 *	@remarks :
 */
void _dbus_client_service_adaptor_deinit()
{
	FUNC_START();
	if (NULL != dbusClientMainLoop) {
		if (g_main_loop_is_running(dbusClientMainLoop)) {
			g_main_loop_quit(dbusClientMainLoop);
		}
	}

	if (NULL != dbusClientThread) {
		gpointer data = g_thread_join(dbusClientThread);
		if (NULL != data) {	/* thread passed data must be free here */
			free(data);
		}
		dbusClientThread = NULL;
	}

	if (NULL != dbusClientMainLoop) {
		g_main_loop_unref(dbusClientMainLoop);
		dbusClientMainLoop = NULL;
	}

	if (NULL != dbusClientMainContext) {
		g_main_context_pop_thread_default(dbusClientMainContext);
		g_main_context_unref(dbusClientMainContext);
		dbusClientMainContext = NULL;
	}

	_queue_clear_task();
	_signal_queue_clear_task();
	FUNC_END();
}

/**
 * Adds string into variant builder
 * @param builder Builder
 * @param data String to be added
 */
void __safe_g_variant_builder_add_string(GVariantBuilder *builder,
						const char *data)
{
	if (NULL == data) {
		g_variant_builder_add(builder, "s", "");
	} else {
		g_variant_builder_add(builder, "s", data);
	}
}

void __safe_g_variant_builder_add_array_string(GVariantBuilder *builder,
						const char *data)
{
	if (NULL == data) {
		g_variant_builder_add(builder, "(s)", "");
	} else {
		g_variant_builder_add(builder, "(s)", data);
	}
}

char *ipc_g_variant_dup_string(GVariant *string)
{
	char *ret = g_variant_dup_string(string, NULL);

	if (0 == strcmp(ret, "")) {
		free(ret);
		ret = NULL;
	}

	return ret;
}

/**	@brief
 *	@return	service_adaptor_error_s
 *	@remarks :
 */
int _dbus_connect_service_adaptor(service_adaptor_error_s *error)
{
	FUNC_START();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	ipc_check_proxy(sac_interface_proxy);

	char executable_path[1000] = {0, };
	int executable_path_len = 0;
	executable_path_len = readlink("/proc/self/exe", executable_path, 1000);

	if (executable_path_len <= 0)
		snprintf(executable_path, strlen("notfound") + 1, "%s", "notfound");

	char *client_info = g_strdup_printf("%s %d", executable_path, (int)getpid());

	sac_info("[DM] client info <%s>", client_info);
	GVariant *req = g_variant_new("("service_adaptor_essential_s_type")", client_info);
	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			DBUS_CONNECT_SERVICE_ADAPTOR_METHOD,
			req,
			G_DBUS_CALL_FLAGS_NONE,
			-1,
			NULL,
			&g_error);

	if (NULL == call_result) {
		error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;

		if (NULL != g_error) {
			error->msg = __SAFE_STRDUP(g_error->message);
			g_error_free(g_error);
		}
	} else {
		if (FALSE == g_variant_is_of_type(call_result, G_VARIANT_TYPE("(ts)"))) {
			error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->msg = strdup("D-Bus return type error");
			ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		} else {
			GVariant *call_result_struct[2];
			call_result_struct[0] = g_variant_get_child_value(call_result, 0);
			call_result_struct[1] = g_variant_get_child_value(call_result, 1);

			uint64_t remote_call_result = g_variant_get_uint64(call_result_struct[0]);

			if (SERVICE_ADAPTOR_ERROR_NONE != remote_call_result) {
				error->code = remote_call_result;
				error->msg = ipc_g_variant_dup_string(call_result_struct[1]);
				ret = SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED;
			}

			g_variant_unref(call_result_struct[0]);
			g_variant_unref(call_result_struct[1]);
		}
		g_variant_unref(call_result);
	}

/*	g_variant_unref(req);*/
	free(client_info);

	FUNC_END();
	return ret;
}

int _dbus_disconnect_service_adaptor(service_adaptor_error_s *error)
{
	FUNC_START();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	ipc_check_proxy(sac_interface_proxy);

	char executable_path[1000] = {0, };
	int executable_path_len = 0;
	executable_path_len = readlink("/proc/self/exe", executable_path, 1000);

	if (executable_path_len <= 0)
		snprintf(executable_path, strlen("notfound") + 1, "%s", "notfound");

	char *client_info = g_strdup_printf("%s %d", executable_path, (int)getpid());

	sac_info("[DM] client info <%s>", client_info);
	GVariant *req = g_variant_new("("service_adaptor_essential_s_type")", client_info);
	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			DBUS_DISCONNECT_SERVICE_ADAPTOR_METHOD,
			req,
			G_DBUS_CALL_FLAGS_NONE,
			-1,
			NULL,
			&g_error);

	if (NULL == call_result) {
		error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;

		if (NULL != g_error) {
			error->msg = __SAFE_STRDUP(g_error->message);
			g_error_free(g_error);
		}
	} else {
		if (FALSE == g_variant_is_of_type(call_result, G_VARIANT_TYPE("(ts)"))) {
			error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->msg = strdup("D-Bus return type error");
			ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		} else {
			GVariant *call_result_struct[2];
			call_result_struct[0] = g_variant_get_child_value(call_result, 0);
			call_result_struct[1] = g_variant_get_child_value(call_result, 1);

			uint64_t remote_call_result = g_variant_get_uint64(call_result_struct[0]);

			if (SERVICE_ADAPTOR_ERROR_NONE != remote_call_result) {
				error->code = remote_call_result;
				error->msg = ipc_g_variant_dup_string(call_result_struct[1]);
				ret = SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED;
			}

			g_variant_unref(call_result_struct[0]);
			g_variant_unref(call_result_struct[1]);
		}

		g_variant_unref(call_result);
	}

/*	g_variant_unref(req);*/
	free(client_info);

	FUNC_END();
	return ret;
}

int _dbus_get_plugin_list(plugin_entry_t ***plugin_list,
						unsigned int *plugins_len,
						service_adaptor_error_s *error)
{
	FUNC_START();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();

	ipc_check_proxy(sac_interface_proxy);

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			DBUS_GET_AUTH_PLUGIN_LIST_METHOD,
			g_variant_new("(s)",
					"temp_todo_remove"), /*TODO */
			G_DBUS_CALL_FLAGS_NONE,
			-1,
			NULL,
			&g_error);

	if (NULL == call_result) {
		error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;

		if (NULL != g_error) {
			error->msg = __SAFE_STRDUP(g_error->message);
			g_error_free(g_error);
		}
	} else {
		if (FALSE == g_variant_is_of_type(call_result, G_VARIANT_TYPE(MAKE_RETURN_TYPE(plugin_list_type)))) {
			error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->msg = strdup("D-Bus return type error");
			ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		} else {
			GVariant *call_result_struct[3];
			call_result_struct[0] = g_variant_get_child_value(call_result, 0);
			call_result_struct[1] = g_variant_get_child_value(call_result, 1);
			call_result_struct[2] = g_variant_get_child_value(call_result, 2);

			uint64_t remote_call_result = g_variant_get_uint64(call_result_struct[1]);

			if (SERVICE_ADAPTOR_ERROR_NONE != remote_call_result) {
				error->code = remote_call_result;
				error->msg = ipc_g_variant_dup_string(call_result_struct[2]);
				ret = SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED;
			} else {
				gsize list_count = g_variant_n_children(call_result_struct[0]);

				*plugins_len = (unsigned int) (list_count);
				plugin_entry_t **plugins = NULL;
				plugins = (plugin_entry_t **) calloc((*plugins_len), sizeof(plugin_entry_t *));

				if (NULL != plugins) {
					for (gsize i = 0; i < list_count; i++) {
						GVariant *plugin_info_struct[service_adaptor_plugin_s_type_length];
						GVariant *plugin_info_entry_v = g_variant_get_child_value(call_result_struct[0], i);

						for (size_t j = 0; j < service_adaptor_plugin_s_type_length; j++) {
							plugin_info_struct[j] = g_variant_get_child_value(plugin_info_entry_v, j);
						}

						plugins[i] = (plugin_entry_t *) calloc(1, sizeof(plugin_entry_t));

						int idx = 0;
						plugins[i]->plugin_uri = ipc_g_variant_dup_string(plugin_info_struct[idx++]);
						plugins[i]->installed_mask = g_variant_get_int32(plugin_info_struct[idx++]);

						for (size_t j = 0; j < service_adaptor_plugin_s_type_length; j++) {
							g_variant_unref(plugin_info_struct[j]);
						}
					}
					*plugin_list = plugins;
				} else {
					*plugins_len = (unsigned int) 0;
					ret = SERVICE_ADAPTOR_ERROR_UNKNOWN;
				}
			}

			g_variant_unref(call_result_struct[0]);
			g_variant_unref(call_result_struct[1]);
			g_variant_unref(call_result_struct[2]);
		}

		g_variant_unref(call_result);
	}

	FUNC_END();
	return ret;
}

int _dbus_is_login_required(service_plugin_h plugin,
						bool *required,
						service_adaptor_error_s *error)
{
	FUNC_START();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();

	ipc_check_proxy(sac_interface_proxy);

	GVariantBuilder *builder_in = NULL;
	_create_raw_data_from_plugin_property((void *)(plugin->optional_property), &builder_in);

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			DBUS_IS_AUTH_METHOD,
			g_variant_new("(" service_adaptor_is_auth_req_s_type ")",
					plugin->plugin_uri,
					builder_in),
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
			NULL,
			&g_error);

	bool is_auth = false;

	_ipc_get_complex_result(MAKE_RETURN_TYPE("(b)"),

		GVariant *res_info_struct = g_variant_get_child_value(call_result_struct[0], 0);
		is_auth = g_variant_get_boolean(res_info_struct);
		g_variant_unref(res_info_struct);
	);

	*required = !is_auth;

	return ret;
}

struct __login_request_context {
	void *callback;
	void *user_data;
};

static void __dbus_login_result_callback(GObject *source_object,
						GAsyncResult *res,
						gpointer user_data)
{

	service_adaptor_error_s error = {0LL, NULL};
	GError *g_error = NULL;

	GVariant *result_obj = g_dbus_proxy_call_finish(_dbus_get_sac_interface_proxy(), res, &g_error);
	int result = _ipc_get_simple_result(result_obj, g_error, &error);
	if (SERVICE_ADAPTOR_ERROR_NONE != result) {
		service_adaptor_set_last_result(error.code, error.msg);
	}

	struct __login_request_context *context = (struct __login_request_context *)user_data;
	service_plugin_login_cb callback = (service_plugin_login_cb)(context->callback);

	callback(result, context->user_data);

	free(context);
	if (g_error) {
		g_error_free(g_error);
	}
}

int _dbus_request_login(service_plugin_h plugin,
						void *callback,
						void *user_data,
						service_adaptor_error_s *error)
{
	FUNC_START();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();

	ipc_check_proxy(sac_interface_proxy);

	struct __login_request_context *context = (struct __login_request_context *)calloc(1, sizeof(struct __login_request_context));
	if (NULL == context) {
		error->code = SERVICE_ADAPTOR_ERROR_UNKNOWN;
		error->msg = strdup("Memory allocation failed");
		return SERVICE_ADAPTOR_ERROR_UNKNOWN;
	}
	context->callback = callback;
	context->user_data = user_data;

	GVariantBuilder *builder_in = NULL;
	_create_raw_data_from_plugin_property((void *)(plugin->optional_property), &builder_in);

	g_dbus_proxy_call(sac_interface_proxy,
			DBUS_JOIN_METHOD,
			g_variant_new("(" service_adaptor_join_req_s_type ")",
					plugin->plugin_uri,
					builder_in),
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
			NULL,
			__dbus_login_result_callback,
			(void *)context);

	return ret;
}

int _dbus_start_service(service_plugin_h plugin,
						int service_flag,
						const char *security_cookie,
						service_adaptor_error_s *error)
{
	FUNC_START();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();

	ipc_check_proxy(sac_interface_proxy);

	char *app_id = NULL;
	char *app_secret = NULL;
	char *user_id = NULL;
	char *user_password = "";

	if (NULL != plugin->optional_property) {
		app_id = (char *) g_hash_table_lookup((GHashTable *)plugin->optional_property, (gconstpointer) SERVICE_PLUGIN_PROPERTY_APP_KEY);
		app_secret = (char *) g_hash_table_lookup((GHashTable *)plugin->optional_property, (gconstpointer) SERVICE_PLUGIN_PROPERTY_APP_SECRET);
		user_id = (char *) g_hash_table_lookup((GHashTable *)plugin->optional_property, (gconstpointer) SERVICE_PLUGIN_PROPERTY_USER_ID);
	}

	GVariantBuilder *builder_in = g_variant_builder_new(G_VARIANT_TYPE("a(y)"));
	for (int k = 0; k < (SECURITY_SERVER_COOKIE_BUFFER_SIZE - 1); k++) {
		g_variant_builder_add(builder_in, "(y)", (guchar)security_cookie[k]);
	}

	GVariantBuilder *builder_property = NULL;
	_create_raw_data_from_plugin_property((void *)(plugin->optional_property), &builder_property);

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			DBUS_SET_AUTH_METHOD,
			g_variant_new("(" service_adaptor_set_auth_s_type ")",
					builder_in,
					builder_property,
					plugin->service_handle_name,
					plugin->plugin_uri,
					app_id ? app_id : "",
					app_secret ? app_secret : "",
					user_id ? user_id : "",
					user_password,
					service_flag),
			G_DBUS_CALL_FLAGS_NONE,
			-1,
			NULL,
			&g_error);

	g_variant_builder_unref(builder_in);

	if (NULL == call_result) {
		error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;

		if (NULL != g_error) {
			error->msg = __SAFE_STRDUP(g_error->message);
			g_error_free(g_error);
		}
	} else {
		if (FALSE == g_variant_is_of_type(call_result, G_VARIANT_TYPE("(ts)"))) {
			error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->msg = strdup("D-Bus return type error");
			ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		} else {
			GVariant *call_result_struct[2];
			call_result_struct[0] = g_variant_get_child_value(call_result, 0);
			call_result_struct[1] = g_variant_get_child_value(call_result, 1);

			uint64_t remote_call_result = g_variant_get_uint64(call_result_struct[0]);

			if (SERVICE_ADAPTOR_ERROR_NONE != remote_call_result) {
				error->code = remote_call_result;
				error->msg = ipc_g_variant_dup_string(call_result_struct[1]);
				ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			}

			g_variant_unref(call_result_struct[0]);
			g_variant_unref(call_result_struct[1]);
		}

		g_variant_unref(call_result);
	}

	return ret;
}

int _dbus_external_request(const char *service_name,
						int service_flag,
						const char *api_uri,
						unsigned char *input_str,
						int input_len,
						unsigned char **output_str,
						int *output_len,
						service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();

	ipc_check_proxy(sac_interface_proxy);

	GVariantBuilder *builder_in = g_variant_builder_new(G_VARIANT_TYPE(service_adaptor_raw_data_s_type));
	for (int k = 0; k < input_len; k++) {
		g_variant_builder_add(builder_in, "(y)", (guchar)input_str[k]);
	}

	sac_debug_func("input_str_len(%d)", input_len);
	/* sac_debug_func("input_str(%s)", input_str); */

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			PRIVATE_DBUS_EXTERNAL_REQ_METHOD,
			g_variant_new("(" private_service_adaptor_external_req_s_type ")",
					service_name,
					(int32_t) service_flag,
					api_uri,
					builder_in),
			G_DBUS_CALL_FLAGS_NONE,
			-1,
			NULL,
			&g_error);

	g_variant_builder_unref(builder_in);

	sac_debug("%s API sent", PRIVATE_DBUS_EXTERNAL_REQ_METHOD);

	if (NULL == call_result) {
		error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;

		if (NULL != g_error) {
			error->msg = __SAFE_STRDUP(g_error->message);
			g_error_free(g_error);
		}
	} else {
		if (FALSE == g_variant_is_of_type(call_result, G_VARIANT_TYPE(MAKE_RETURN_TYPE(service_adaptor_raw_data_s_type)))) {
			error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->msg = strdup("D-Bus return type error");
			ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		} else {
			FUNC_STEP();
			GVariant *call_result_struct[3];
			call_result_struct[0] = g_variant_get_child_value(call_result, 0);
			call_result_struct[1] = g_variant_get_child_value(call_result, 1);
			call_result_struct[2] = g_variant_get_child_value(call_result, 2);


			uint64_t remote_call_result = g_variant_get_uint64(call_result_struct[1]);

			if (SERVICE_ADAPTOR_ERROR_NONE != remote_call_result) {
				error->code = remote_call_result;
				error->msg = ipc_g_variant_dup_string(call_result_struct[2]);
				ret = SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED;
			} else {

				int raw_data_len = g_variant_n_children(call_result_struct[0]);
				unsigned char *raw_data = (unsigned char *) calloc((raw_data_len + 1), sizeof(unsigned char));

				if (NULL != raw_data) {
					for (int k = 0; k < raw_data_len; k++) {
						g_variant_get_child(call_result_struct[0], k, "(y)", &(raw_data[k]));
					}
					*output_str = raw_data;
					*output_len = raw_data_len;
				}
				sac_debug_func("output_str_len(%d)", raw_data_len);
				/* sac_debug_func("output_str(%s)", raw_data); */
			}

			g_variant_unref(call_result_struct[0]);
			g_variant_unref(call_result_struct[1]);
			g_variant_unref(call_result_struct[2]);
		}

		g_variant_unref(call_result);
	}

	return ret;
}

static void *_on_dbus_disappeared_cb(void *data)
{
	FUNC_START();
	__dbus_connection_deinit();
	service_adaptor_task_h task = _signal_queue_get_task(SIGNAL_SERVICE_ADAPTOR);

	if (NULL == task) {
		FUNC_STOP();
		return NULL;
	}

	struct _internal_service_signal_data_s *signal_context =
		(struct _internal_service_signal_data_s *)g_malloc0(sizeof(struct _internal_service_signal_data_s));

	if (NULL == signal_context) {
		FUNC_STOP();
		return NULL;
	} else {
		signal_context->msg = NULL;
		signal_context->task = NULL;
	}

	signal_context->code = (int64_t) SERVICE_ADAPTOR_SIGNAL_SHUTDOWN;
	signal_context->msg = strdup("Service-adaptor DBus interface was vanished");
	signal_context->task = task;

	GError *worker_error = NULL;
	g_thread_try_new("service-adaptor-client-signal-emitter", (GThreadFunc) _service_signal_emitter,
			(void *) signal_context, &worker_error);

	g_clear_error(&worker_error);
	worker_error = NULL;

	FUNC_END();
	return NULL;
}
