/*
 * Service Adaptor IPC Server
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

static GMainContext *g_working_context = NULL;

/**
 * D-Bus server thread
 */
static GThread *dbus_server_thread = NULL;

/**
 * D-Bus server thread main loop context
 */
static GMainContext *dbus_server_context = NULL;

/**
 * D-Bus server thread main loop
 */
static GMainLoop *dbus_server_loop = NULL;

/**
 * D-Bus server thread pool
 */
static GThreadPool *thread_pool = NULL;

/**
 * Compiled introspection data describing D-Bus interface
 */
static GDBusNodeInfo *introspection_data = NULL;

/**
 * D-Bus server owner id
 */
static guint owner_id = 0;

/**
 * D-Bus connection used by server
 */
static GDBusConnection *dbus_connection = NULL;

/**
 * Introspection data describing D-Bus interface
 */
static const gchar introspection_xml[] =
"<node>"
"  <interface name='" SERVICE_PROVIDER_BASE_INTERFACE "'>"
"    <method name='"  SERVICE_PROVIDER_BASE_SESSION_START "'>"
"      <arg type='"   SERVICE_PROVIDER_BASE_SESSION_START_REQ "' name='req' direction='in'/>"
"      <arg type='"   SERVICE_PROVIDER_BASE_SESSION_START_RES "' name='res' direction='out'/>"
"    </method>"
"    <method name='"  SERVICE_PROVIDER_BASE_SESSION_STOP "'>"
"      <arg type='"   SERVICE_PROVIDER_BASE_SESSION_STOP_REQ "' name='req' direction='in'/>"
"      <arg type='"   SERVICE_PROVIDER_BASE_SESSION_STOP_RES "' name='res' direction='out'/>"
"    </method>"
"    <method name='"  SERVICE_PROVIDER_STORAGE_OPEN "'>"
"      <arg type='"   SERVICE_PROVIDER_STORAGE_OPEN_REQ "' name='req' direction='in'/>"
"      <arg type='"   SERVICE_PROVIDER_STORAGE_OPEN_RES "' name='res' direction='out'/>"
"    </method>"
"    <method name='"  SERVICE_PROVIDER_STORAGE_CLOSE "'>"
"      <arg type='"   SERVICE_PROVIDER_STORAGE_CLOSE_REQ "' name='req' direction='in'/>"
"      <arg type='"   SERVICE_PROVIDER_STORAGE_CLOSE_RES "' name='res' direction='out'/>"
"    </method>"
"  </interface>"
"</node>";

/******************************************************************************
 * Private interface
 ******************************************************************************/

/******************************************************************************
 * Private interface definition
 ******************************************************************************/

static void _method_call_async_func(gpointer data, gpointer user_data)
{
	SAL_FN_CALL;

	ipc_server_session_h handle = data;

	RET_IF(NULL == handle);

	SAL_INFO("Call %s", handle->method_name);

	if (0 == strncmp(handle->method_name, DBUS_SERVICE_ADAPTOR, DBUS_NAME_LENGTH)) {
		g_main_context_invoke(g_working_context, sal_server_base_method_call, (void *)handle);

	} else if (0 == strncmp(handle->method_name, DBUS_SERVICE_AUTH, DBUS_NAME_LENGTH)) {
		g_main_context_invoke(g_working_context, sal_server_auth_method_call, (void *)handle);

	} else if (0 == strncmp(handle->method_name, DBUS_SERVICE_STORAGE, DBUS_NAME_LENGTH)) {
		g_main_context_invoke(g_working_context, sal_server_storage_method_call, (void *)handle);
	}
}

/**
 * @brief Service Adaptor D-Bus interface method call handler.
 *
 * Service Adaptor D-Bus interface method call handler. Called when client calls some function defined in
 * Service Adaptor D-Bus interface.
 * @param connection A GDBusConnection.
 * @param sender The unique bus name of the remote caller.
 * @param object_path The object path that the method was invoked on.
 * @param interface_name The D-Bus interface name the method was invoked on.
 * @param method_name The name of the method that was invoked.
 * @param parameters A GVariant tuple with parameters.
 * @param invocation A GDBusMethodInvocation object that can be used to return a value or error.
 * @param user_data The user_data gpointer passed to g_dbus_connection_register_object().
 */
static void _handle_method_call(GDBusConnection *connection,
		const gchar *sender,
		const gchar *object_path,
		const gchar *interface_name,
		const gchar *method_name,
		GVariant *parameters,
		GDBusMethodInvocation *invocation,
		gpointer user_data)
{
	SAL_FN_CALL;

	ipc_server_session_h handle = (ipc_server_session_h) g_malloc0(sizeof(ipc_server_session_s));

	handle->connection = connection;
	handle->sender = (gchar *) sender;
	handle->object_path = (gchar *) object_path;
	handle->interface_name = (gchar *) interface_name;
	handle->method_name = (gchar *) method_name;
	handle->parameters = parameters;
	handle->invocation = invocation;
	handle->user_data = user_data;

	g_thread_pool_push(thread_pool, (gpointer) handle, NULL);
}

/**
 * @brief Service Adaptor D-Bus interface get property call handler.
 *
 * Service Adaptor D-Bus interface get property call handler.
 * @param connection A GDBusConnection.
 * @param sender The unique bus name of the remote caller.
 * @param object_path The object path that the method was invoked on.
 * @param interface_name The D-Bus interface name for the property.
 * @param property_name The name of the property to get the value of.
 * @param error Return location for error.
 * @param user_data The user_data gpointer passed to g_dbus_connection_register_object().
 * @return A GVariant with the value for property_name or NULL if error is set. If the returned GVariant
 *         is floating, it is consumed - otherwise its reference count is decreased by one.
 */
static GVariant *_handle_get_property(GDBusConnection *connection,
		const gchar *sender,
		const gchar *object_path,
		const gchar *interface_name,
		const gchar *property_name,
		GError **error,
		gpointer user_data)
{
	SAL_FN_CALL;

	GVariant *ret = NULL;

	return ret;
}

/**
 * @brief Service Adaptor D-Bus interface set property call handler.
 *
 * Service Adaptor D-Bus interface set property call handler.
 * @param connection A GDBusConnection.
 * @param sender The unique bus name of the remote caller.
 * @param object_path The object path that the method was invoked on.
 * @param interface_name The D-Bus interface name for the property.
 * @param property_name The name of the property to get the value of.
 * @param value The value to set the property to.
 * @param error Return location for error.
 * @param user_data     The user_data gpointer passed to g_dbus_connection_register_object().
 * @return TRUE if the property was set to value, FALSE if error is set.
 */
static gboolean _handle_set_property(GDBusConnection *connection,
		const gchar *sender,
		const gchar *object_path,
		const gchar *interface_name,
		const gchar *property_name,
		GVariant *value,
		GError **error,
		gpointer user_data)
{
	SAL_FN_CALL;

	gboolean ret = false;

	if (NULL == *error) {
		ret = true;
	}

	return ret;
}

/**
 * D-Bus handlers vtable.
 */
static const GDBusInterfaceVTable interface_vtable = {
	_handle_method_call,
	_handle_get_property,
	_handle_set_property
};

/**
 * @brief Callback function called when D-Bus bus name for Service Adaptor D-Bus server is acquired.
 *
 * Callback function called when D-Bus bus name for Service Adaptor D-Bus server is acquired.
 * @param connection The GDBusConnection to a message bus.
 * @param name The name that is requested to be owned.
 * @param user_data User data passed to g_bus_own_name().
 */
static void _on_bus_acquired(GDBusConnection *connection,
		const gchar *name,
		gpointer user_data)
{
	SAL_FN_CALL;

	guint registration_id;

	registration_id = g_dbus_connection_register_object(connection,
			SERVICE_ADAPTOR_OBJECT_PATH,
			introspection_data->interfaces[0],
			&interface_vtable,
			NULL, /* user_data */
			NULL, /* user_data_free_func */
			NULL); /* GError** */

	g_assert(registration_id > 0);
}

/**
 * @brief Callback function called when D-Bus name for Service Adaptor D-Bus server is acquired.
 *
 * Callback function called when D-Bus name for Service Adaptor D-Bus server is acquired.
 * @param connection The GDBusConnection on which to acquired the name.
 * @param name The name being owned.
 * @param user_data User data passed to g_bus_own_name() or g_bus_own_name_on_connection().
 */
static void _on_name_acquired(GDBusConnection *connection,
		const gchar *name,
		gpointer user_data)
{
	SAL_FN_CALL;

	dbus_connection = connection;
	g_object_ref(dbus_connection);
}

/**
 * @brief Callback function called when the Service Adaptor D-Bus name is lost or connection has been closed.
 *
 * Callback function called when the Service Adaptor D-Bus name is lost or connection has been closed.
 * @param connection The GDBusConnection on which to acquire the name or NULL if the connection was disconnected.
 * @param name The name being owned.
 * @param user_data User data passed to g_bus_own_name() or g_bus_own_name_on_connection().
 */
static void _on_name_lost(GDBusConnection *connection,
		const gchar *name,
		gpointer user_data)
{
	SAL_FN_CALL;

	if (NULL != dbus_connection) {
		g_object_unref(dbus_connection);
		dbus_connection = NULL;
	}

	SAL_INFO("Unexpected D-bus bus name lost");

	/* Send SIGINT to main thread to stop File Manager process and cleanly close Service Adaptor */
	kill(getpid(), SIGINT);
}

int _sal_ipc_server_start()
{
	SAL_FN_CALL;

	RETV_IF(NULL != introspection_data, SAL_ERROR_INTERNAL);
	RETV_IF(0 != owner_id, SAL_ERROR_INTERNAL);

	introspection_data = g_dbus_node_info_new_for_xml(introspection_xml, NULL);
	RETVM_IF(NULL == introspection_data, SAL_ERROR_INTERNAL, "g_dbus_node_info_new_for_xml() Failed");

	thread_pool = g_thread_pool_new(_method_call_async_func, NULL, -1, FALSE, NULL);
	RETVM_IF(NULL == thread_pool, SAL_ERROR_SYSTEM, "g_thread_pool_new() Failed");

	owner_id = g_bus_own_name(G_BUS_TYPE_SYSTEM,
			SERVICE_ADAPTOR_BUS_NAME,
			G_BUS_NAME_OWNER_FLAGS_NONE,
			_on_bus_acquired,
			_on_name_acquired,
			_on_name_lost,
			NULL,
			NULL);

	if (0 == owner_id) {
		g_dbus_node_info_unref(introspection_data);
		introspection_data = NULL;

		return SAL_ERROR_SYSTEM;
	}

	return SAL_ERROR_NONE;
}

int _sal_ipc_server_stop()
{
	SAL_FN_CALL;

	if (NULL != thread_pool) {
		g_thread_pool_free(thread_pool, TRUE, TRUE);
	}

	if (0 != owner_id) {
		g_bus_unown_name(owner_id);
		owner_id = 0;
	}

	if (NULL != introspection_data) {
		g_dbus_node_info_unref(introspection_data);
		introspection_data = NULL;
	}

	return SAL_ERROR_NONE;
}
/**
 * @brief D-Bus server thread function.
 *
 * D-Bus server thread function. It initialises structures and callbacks needed to export D-Bus interfaces.
 * @param data Data passed to thread.
 * @return Result data from thread (always NULL in this implementation).
 */
static gpointer _dbus_server_thread_func(gpointer data)
{
	SAL_FN_CALL;

	int ret = 0;

	dbus_server_context = g_main_context_new();
	dbus_server_loop = g_main_loop_new(dbus_server_context, FALSE);
	g_main_context_push_thread_default(dbus_server_context);

	ret = _sal_ipc_server_start();

	if (SAL_ERROR_NONE == ret) {
		g_main_loop_run(dbus_server_loop);
	}

	_sal_ipc_server_stop();

	g_main_context_pop_thread_default(dbus_server_context);
	g_main_loop_unref(dbus_server_loop);
	dbus_server_loop = NULL;
	g_main_context_unref(dbus_server_context);
	dbus_server_context = NULL;

	return NULL;
}

/******************************************************************************
 * Public interface definition
 ******************************************************************************/

API int sal_ipc_server_init(GMainContext *working_context,
		ipc_server_base_req_s *base_method,
		ipc_server_auth_req_s *auth_method,
		ipc_server_storage_req_s *storage_method)
{
	SAL_FN_CALL;

	RETVM_IF(NULL == base_method, SAL_ERROR_INTERNAL, "Please check param");
	RETVM_IF(NULL == auth_method, SAL_ERROR_INTERNAL, "Please check param");
	RETVM_IF(NULL == storage_method, SAL_ERROR_INTERNAL, "Please check param");

	RETVM_IF(NULL != dbus_server_thread, SAL_ERROR_INTERNAL, "IPC server thread is already running");

	RETV_IF(ipc_server_base_init(base_method), SAL_ERROR_INTERNAL);
	RETV_IF(ipc_server_auth_init(auth_method), SAL_ERROR_INTERNAL);
	RETV_IF(ipc_server_storage_init(storage_method), SAL_ERROR_INTERNAL);

	g_working_context = working_context;

	dbus_server_thread = g_thread_new("IPC Server", _dbus_server_thread_func, NULL);

	return SAL_ERROR_NONE;
}

API int sal_ipc_server_deinit()
{
	SAL_FN_CALL;

	if (NULL != dbus_server_loop) {
		if (g_main_loop_is_running(dbus_server_loop)) {
			g_main_loop_quit(dbus_server_loop);
		}
	}

	if (NULL != dbus_server_thread) {
		g_thread_join(dbus_server_thread);
		dbus_server_thread = NULL;
	}

	if (NULL != dbus_server_loop) {
		g_main_loop_unref(dbus_server_loop);
		dbus_server_loop = NULL;
	}

	if (NULL != dbus_server_context) {
		g_main_context_pop_thread_default(dbus_server_context);
		g_main_context_unref(dbus_server_context);
		dbus_server_context = NULL;
	}

	return SAL_ERROR_NONE;
}
