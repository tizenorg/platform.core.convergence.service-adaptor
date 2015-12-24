/*
 * Service Adaptor IPC Client
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

#include <glib.h>
#include <gio/gio.h>

#include "sal_types.h"
#include "sal_log.h"
#include "sal_ipc.h"

/******************************************************************************
 * Global variables and defines
 ******************************************************************************/
#define ERROR_MESSAGE_MAX_LENGH		2048

static __thread int last_error_code = 0;

static __thread char last_error_message[ERROR_MESSAGE_MAX_LENGH] = {0, };

/**
 * Service Adaptor D-Bus client thread data
 */
typedef struct _dbus_client_thread_data_s {
	GMutex connection_mutex;	/* Mutex used to protect connection status data */
	GCond connection_cond;		/* Conditional variable used to signal that connection was established */
	int connection_cond_signaled;	/* Additional variable used to signal that connection was established */
} dbus_client_thread_data_s;
typedef struct _dbus_client_thread_data_s *dbus_client_thread_data_h;

/**
 * D-Bus client thread.
 */
static GThread *dbus_client_thread = NULL;

/**
 * D-Bus client thread main loop context.
 */
static GMainContext *dbus_client_context = NULL;

/**
 * D-Bus client thread main loop.
 */
static GMainLoop *dbus_client_loop = NULL;

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
static GDBusProxy *interface_proxy = NULL;

/******************************************************************************
 * Private interface
 ******************************************************************************/

static gpointer _dbus_client_thread_func(gpointer data);

static int _dbus_client_start(dbus_client_thread_data_h thread_data);

static int _dbus_client_stop();

static void _on_name_appeared(GDBusConnection *connection,
		const gchar *name,
		const gchar *name_owner,
		gpointer user_data);

static void _on_name_vanished(GDBusConnection *connection,
		const gchar *name,
		gpointer user_data);

static void _on_signal(GDBusProxy *proxy,
		gchar *sender_name,
		gchar *signal_name,
		GVariant *parameters,
		gpointer user_data);

/******************************************************************************
 * Private interface definition
 ******************************************************************************/

/**
 * @brief D-Bus client thread function.
 *
 * D-Bus client thread function. It initialises all client D-Bus interfaces.
 * @param data Data passed to thread.
 * @return Result data from thread (always NULL in this implementation).
 */
static gpointer _dbus_client_thread_func(gpointer data)
{
	SAL_FN_CALL;

	int ret = 0;

	dbus_client_context = g_main_context_new();
	dbus_client_loop = g_main_loop_new(dbus_client_context, FALSE);
	g_main_context_push_thread_default(dbus_client_context);

	ret = _dbus_client_start(data);

	if (SAL_ERROR_NONE == ret) {
		g_main_loop_run(dbus_client_loop);
	}

	_dbus_client_stop();

	g_main_context_pop_thread_default(dbus_client_context);
	g_main_loop_unref(dbus_client_loop);
	dbus_client_loop = NULL;
	g_main_context_unref(dbus_client_context);
	dbus_client_context = NULL;

	free(data);

	return NULL;
}

/**
 * @brief Initialises Service Adaptor D-Bus client internal structures.
 *
 * Initialises Service Adaptor D-Bus client internal structures.
 * @param thread_data Pointer to thread data used to signal that connection was successfully established.
 * @return 0 on success, error code when some structures could not be initialised.
 */
static int _dbus_client_start(dbus_client_thread_data_h thread_data)
{
	SAL_FN_CALL;

	GError *error = NULL;

	RETV_IF(NULL != connection, SAL_ERROR_INTERNAL);
	RETV_IF(NULL != interface_proxy, SAL_ERROR_INTERNAL);

	connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);

	if (NULL == connection) {
		g_error_free(error);

		return SAL_ERROR_INTERNAL;
	}

	interface_proxy = g_dbus_proxy_new_sync(connection,
			G_DBUS_PROXY_FLAGS_NONE,
			NULL,
			SERVICE_ADAPTOR_BUS_NAME,
			SERVICE_ADAPTOR_OBJECT_PATH,
			SERVICE_ADAPTOR_INTERFACE,
			NULL,
			&error);

	if (NULL == interface_proxy) {
		g_error_free(error);

		g_object_unref(connection);
		connection = NULL;

		return SAL_ERROR_INTERNAL;
	}

	watcher_id = g_bus_watch_name(G_BUS_TYPE_SYSTEM,
			SERVICE_ADAPTOR_BUS_NAME,
			G_BUS_NAME_WATCHER_FLAGS_NONE,
			_on_name_appeared,
			_on_name_vanished,
			thread_data,
			NULL);

	if (0 == watcher_id) {
		g_object_unref(interface_proxy);
		interface_proxy = NULL;

		g_object_unref(connection);
		connection = NULL;

		return SAL_ERROR_INTERNAL;
	}

	int res = g_signal_connect(interface_proxy,
			"g-signal",
			G_CALLBACK(_on_signal),
			NULL);

	if (0 == res) {
		g_object_unref(interface_proxy);
		interface_proxy = NULL;

		g_object_unref(connection);
		connection = NULL;

		g_bus_unwatch_name(watcher_id);
		watcher_id = 0;

		return SAL_ERROR_INTERNAL;
	}

	return SAL_ERROR_NONE;
}

/**
 * @brief Deinitialises Service Adaptor D-Bus client internal structures.
 *
 * Deinitialises Service Adaptor D-Bus client internal structures.
 */
static int _dbus_client_stop()
{
	SAL_FN_CALL;

	if (NULL != interface_proxy) {
		g_object_unref(interface_proxy);
		interface_proxy = NULL;
	}

	if (NULL != connection) {
		g_object_unref(connection);
		connection = NULL;
	}

	if (0 != watcher_id) {
		g_bus_unwatch_name(watcher_id);
		watcher_id = 0;
	}

	return SAL_ERROR_NONE;
}

/**
 * @brief Service Adaptor availability callback function.
 *
 * Service Adaptor availability callback function. Called when Service Adaptor appears at D-Bus bus.
 * @param connection D-Bus connection.
 * @param name The name being watched.
 * @param name_owner Unique name of the owner of the name being watched.
 * @param user_data User data passed to g_bus_watch_name().
 */
static void _on_name_appeared(GDBusConnection *connection,
		const gchar *name,
		const gchar *name_owner,
		gpointer user_data)
{
	SAL_FN_CALL;

	dbus_client_thread_data_h thread_data = (dbus_client_thread_data_h) user_data;

	if (NULL != thread_data) {
		g_mutex_lock(&thread_data->connection_mutex);
		thread_data->connection_cond_signaled = 1;
		g_cond_signal(&thread_data->connection_cond);
		g_mutex_unlock(&thread_data->connection_mutex);
	}
}

/**
 * @brief Service Adaptor availability callback function.
 *
 * Service Adaptor availability callback function. Called when Service Adaptor disappears from D-Bus bus.
 * @param connection D-Bus connection.
 * @param name he name being watched.
 * @param user_data User data passed to g_bus_watch_name().
 */
static void _on_name_vanished(GDBusConnection *connection,
		const gchar *name,
		gpointer user_data)
{
	SAL_FN_CALL;

	if (NULL != interface_proxy) {
		g_object_unref(interface_proxy);
		interface_proxy = NULL;
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
static void _on_signal(GDBusProxy *proxy,
		gchar *sender_name,
		gchar *signal_name,
		GVariant *parameters,
		gpointer user_data)
{
	SAL_FN_CALL;

	if (0 == strncmp(signal_name, DBUS_SERVICE_ADAPTOR, DBUS_NAME_LENGTH)) {
		/* TODO: */
/*
		on_service_signal(proxy,
				sender_name,
				signal_name,
				parameters,
				user_data);
*/
	}
}

/******************************************************************************
 * Public interface definition
 ******************************************************************************/

API void sal_ipc_client_set_last_error(int error, const char *message)
{
	RET_IF(!error);
	RET_IF(!message);

	last_error_code = error;
	snprintf(last_error_message, ERROR_MESSAGE_MAX_LENGH, message);
}

API int sal_ipc_client_get_last_error(void)
{
	return last_error_code;
}

API char *sal_ipc_client_get_last_message(void)
{
	return last_error_message;
}

API int sal_ipc_client_get_interface(GDBusProxy **interface)
{
	SAL_FN_CALL;

	*interface = interface_proxy;

	return SAL_ERROR_NONE;
}

API int sal_ipc_client_init()
{
	SAL_FN_CALL;

#if !GLIB_CHECK_VERSION(2, 32, 0)
	g_thread_init(NULL);
#endif
#if !GLIB_CHECK_VERSION(2, 35, 0)
	g_type_init();
#endif

	RETVM_IF(NULL != dbus_client_thread, SAL_ERROR_INTERNAL, "D-Bus client thread is already running");

	dbus_client_thread_data_h thread_data =
			(dbus_client_thread_data_h) malloc(sizeof(dbus_client_thread_data_s));

	g_mutex_init(&thread_data->connection_mutex);
	g_cond_init(&thread_data->connection_cond);
	thread_data->connection_cond_signaled = 0;

	dbus_client_thread = g_thread_new("Service Adaptor D-Bus Client", _dbus_client_thread_func, thread_data);

	gint64 timeout = g_get_monotonic_time() + 5 * G_TIME_SPAN_SECOND;
	g_mutex_lock(&thread_data->connection_mutex);
	while (!thread_data->connection_cond_signaled) {
		if (!g_cond_wait_until(&thread_data->connection_cond, &thread_data->connection_mutex, timeout)) {
			g_mutex_unlock(&thread_data->connection_mutex);
			return SAL_ERROR_INTERNAL;
		}
	}

	g_mutex_unlock(&thread_data->connection_mutex);

	return SAL_ERROR_NONE;
}

API int sal_ipc_client_deinit()
{
	SAL_FN_CALL;

	if (NULL != dbus_client_loop) {
		if (g_main_loop_is_running(dbus_client_loop)) {
			g_main_loop_quit(dbus_client_loop);
		}
	}

	if (NULL != dbus_client_thread) {
		g_thread_join(dbus_client_thread);
		dbus_client_thread = NULL;
	}

	if (NULL != dbus_client_loop) {
		g_main_loop_unref(dbus_client_loop);
		dbus_client_loop = NULL;
	}

	if (NULL != dbus_client_context) {
		g_main_context_pop_thread_default(dbus_client_context);
		g_main_context_unref(dbus_client_context);
		dbus_client_context = NULL;
	}
	return SAL_ERROR_NONE;
}

API int sal_ipc_client_call_request(const char *request_method, GVariant *request_data, const char *reply_type, GVariant **reply_info)
{
	GError *error = NULL;
	GVariant *reply = NULL;
	int ret = SAL_ERROR_NONE;

	GDBusProxy *interface_proxy = NULL;
	ret = sal_ipc_client_get_interface(&interface_proxy);
	RETV_IF(SAL_ERROR_NONE != ret, SAL_ERROR_INTERNAL);

	reply = g_dbus_proxy_call_sync(interface_proxy,
			request_method,
			request_data,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
			NULL,
			&error);

	RETVM_IF(NULL == reply, SAL_ERROR_INTERNAL, "IPC Request Failed: %s", error->message);
	RETV_IF(false == g_variant_is_of_type(reply, (GVariantType *) SAL_IPC_RETURN_TYPE(reply_type), SAL_ERROR_INTERNAL);

	*reply_info = reply;

	return SAL_ERROR_NONE;
}

