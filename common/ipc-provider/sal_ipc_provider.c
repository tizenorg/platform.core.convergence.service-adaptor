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
#include <app.h>

#include "sal_types.h"
#include "sal_log.h"
#include "sal_ipc.h"

#include "sal_ipc_provider.h"

/******************************************************************************
 * Global variables and defines
 ******************************************************************************/

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

/*
 * Do not free
 */
static char *g_dbus_busname = NULL;

/*
 * Do not free
 */
static char *g_dbus_objectpath = NULL;

/**
 * Introspection data describing D-Bus interface
 */
static const gchar introspection_xml[] =
"<node>"
"  <interface name='" SERVICE_PROVIDER_BUS_INTERFACE "'>"
"    <method name='" SERVICE_PROVIDER_BASE_SESSION_START "'>"
"      <arg type='" SERVICE_PROVIDER_BASE_SESSION_START_REQ "' name='req' direction='in'/>"
"      <arg type='" SERVICE_PROVIDER_BASE_SESSION_START_RES "' name='res' direction='out'/>"
"    </method>"
"    <method name='" SERVICE_PROVIDER_BASE_SESSION_STOP "'>"
"      <arg type='" SERVICE_PROVIDER_BASE_SESSION_STOP_REQ "' name='req' direction='in'/>"
"      <arg type='" SERVICE_PROVIDER_BASE_SESSION_STOP_RES "' name='res' direction='out'/>"
"    </method>"
"    <method name='" SERVICE_PROVIDER_STORAGE_DOWNLOAD "'>"
"      <arg type='" SERVICE_PROVIDER_STORAGE_DOWNLOAD_REQ "' name='req' direction='in'/>"
"      <arg type='" SERVICE_PROVIDER_STORAGE_DOWNLOAD_RES "' name='res' direction='out'/>"
"    </method>"
"  </interface>"
"</node>";

/******************************************************************************
 * Private interface
 ******************************************************************************/

USER_DATA_TYPEDEF(ipc_state_data_t, 2);

/******************************************************************************
 * Private interface definition
 ******************************************************************************/

static void _method_call_async_func(gpointer data, gpointer user_data)
{
	SAL_FN_CALL;

	RET_IF(NULL == data);

	ipc_provider_session_h handle = (ipc_provider_session_h)data;

	SAL_INFO("Call %s", handle->method_name);

	if (0 == strncmp(SERVICE_PROVIDER_STORAGE_PREFIX, handle->method_name, SERVICE_PROVIDER_PREFIX_LEN)) {
		ipc_provider_storage_method_call(data);
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

	ipc_provider_session_h handle = (ipc_provider_session_h) g_malloc0(sizeof(ipc_provider_session_s));

	handle->connection = connection;
	handle->sender = (gchar *) sender;
	handle->object_path = (gchar *) object_path;
	handle->interface_name = (gchar *) interface_name;
	handle->method_name = (gchar *) method_name;
	handle->parameters = parameters;
	handle->invocation = invocation;
	handle->user_data = user_data;

	if (0 == strncmp(SERVICE_PROVIDER_BASE_PREFIX, method_name, SERVICE_PROVIDER_PREFIX_LEN)) {
		GMainContext *global_main_context = g_main_context_default();
		g_main_context_invoke(global_main_context, ipc_provider_base_method_call, (user_data_t)handle);
	} else { /* == SAL_SERVICE_STORAGE */
		g_thread_pool_push(thread_pool, (user_data_t)handle, NULL);
	}
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
	char *_object_path = g_dbus_objectpath;

	registration_id = g_dbus_connection_register_object(connection,
			_object_path,
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


	USER_DATA_DEFINE(ipc_state_data_t, _callback_data) = (USER_DATA_TYPE(ipc_state_data_t) *)user_data;
	ipc_provider_connection_cb callback = (ipc_provider_connection_cb)USER_DATA_ELEMENT(_callback_data, 0);
	void *_user_data = (void *)USER_DATA_ELEMENT(_callback_data, 1);

	if (callback)
		callback(IPC_PROVIDER_CONNECTION_OPENED, _user_data);
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

	USER_DATA_DEFINE(ipc_state_data_t, _callback_data) = (USER_DATA_TYPE(ipc_state_data_t) *)user_data;
	ipc_provider_connection_cb callback = (ipc_provider_connection_cb)USER_DATA_ELEMENT(_callback_data, 0);
	void *_user_data = (void *)USER_DATA_ELEMENT(_callback_data, 1);

	if (callback)
		callback(IPC_PROVIDER_CONNECTION_CLOSED, _user_data);

	if (NULL != dbus_connection) {
		g_object_unref(dbus_connection);
		dbus_connection = NULL;
	}

	SAL_INFO("Unexpected D-bus bus name lost");

	/* Send SIGINT to main thread to stop File Manager process and cleanly close Service Adaptor */
	/* kill(getpid(), SIGINT); */
}

int _sal_ipc_provider_start()
{
	SAL_FN_CALL;

	RETV_IF(NULL != introspection_data, SAL_ERROR_INTERNAL);
	RETV_IF(0 != owner_id, SAL_ERROR_INTERNAL);

	return SAL_ERROR_NONE;
}


/******************************************************************************
 * Public interface definition
 ******************************************************************************/

API int sal_ipc_provider_init(ipc_provider_base_req_s *base_method,
		ipc_provider_storage_req_s *storage_method,
		ipc_provider_connection_cb callback,
		void *user_data)
{
	SAL_FN_CALL;

	RETVM_IF(NULL == base_method, SAL_ERROR_INVALID_PARAMETER, "Please check param");
	RETVM_IF(NULL == storage_method, SAL_ERROR_INVALID_PARAMETER, "Please check param");
	RETVM_IF(NULL == callback, SAL_ERROR_INVALID_PARAMETER, "Please check param");

	introspection_data = g_dbus_node_info_new_for_xml(introspection_xml, NULL);
	RETVM_IF(NULL == introspection_data, SAL_ERROR_INTERNAL, "g_dbus_node_info_new_for_xml() Failed");

	char *app_id = NULL;
	char *bus_name = NULL;
	char *obj_path = NULL;

	USER_DATA_DEFINE(ipc_state_data_t, _callback_data) = NULL;

	USER_DATA_VAL(_callback_data) = USER_DATA_CREATE(ipc_state_data_t);
	TRYM_IF(NULL == USER_DATA_VAL(_callback_data), "Out of memory");
	USER_DATA_ELEMENT(_callback_data, 0) = (user_data_t)callback;
	USER_DATA_ELEMENT(_callback_data, 1) = (user_data_t)user_data;

	int ret = app_get_id(&app_id);
	TRYM_IF(APP_ERROR_NONE != ret, "app_id get failed");
	TRYM_IF(NULL == app_id, "Out of memory");

	bus_name = g_strdup_printf("%s.%s", SERVICE_PROVIDER_BUS_NAME_PREFIX, app_id);
	TRYM_IF(NULL == bus_name, "Out of memory");

	obj_path = g_strdup(bus_name);
	TRYM_IF(NULL == obj_path, "Out of memory");

	SAL_STR_REPLACE(obj_path, '.', '/');

	thread_pool = g_thread_pool_new(_method_call_async_func, NULL, -1, FALSE, NULL);
	TRYM_IF(NULL == thread_pool, "g_thread_pool_new() Failed");

	owner_id = g_bus_own_name(G_BUS_TYPE_SESSION,
			bus_name,
			G_BUS_NAME_OWNER_FLAGS_NONE,
			_on_bus_acquired,
			_on_name_acquired,
			_on_name_lost,
			USER_DATA_VAL(_callback_data),
			(GDestroyNotify)USER_DATA_DESTROY_FUNC);

	TRYM_IF(0 == owner_id, "Dbus own name failed");

	g_dbus_busname = bus_name;
	g_dbus_objectpath = obj_path;

	return SAL_ERROR_NONE;

catch:
	USER_DATA_DESTROY(_callback_data);
	SAL_FREE(obj_path);
	SAL_FREE(bus_name);
	SAL_FREE(app_id);
 	if (introspection_data) {
		g_dbus_node_info_unref(introspection_data);
		introspection_data = NULL;
	}

	if (NULL != thread_pool) {
		g_thread_pool_free(thread_pool, TRUE, TRUE);
		thread_pool = NULL;
	}

	return SAL_ERROR_SYSTEM;
}

API int sal_ipc_provider_deinit()
{
	SAL_FN_CALL;

	SAL_FREE(g_dbus_busname);
	SAL_FREE(g_dbus_objectpath);

	if (NULL != thread_pool) {
		g_thread_pool_free(thread_pool, TRUE, TRUE);
		thread_pool = NULL;
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

API int sal_ipc_provider_emit_signal(const char *signal_name, GVariant *parameters)
{
	RETVM_IF(0 >= owner_id, SAL_ERROR_INTERNAL, "Bus not ready");
	RETVM_IF(NULL == dbus_connection, SAL_ERROR_INTERNAL, "Bus not ready");
	RETVM_IF(NULL == g_dbus_busname, SAL_ERROR_INTERNAL, "Bus not ready");
	RETVM_IF(NULL == g_dbus_objectpath, SAL_ERROR_INTERNAL, "Bus not ready");

	RETVM_IF(NULL == signal_name, SAL_ERROR_INVALID_PARAMETER, "Invalid parameter");
	RETVM_IF(NULL == parameters, SAL_ERROR_INVALID_PARAMETER, "Invalid parameter");

	GError *err = NULL;
	g_dbus_connection_emit_signal(dbus_connection, g_dbus_busname, g_dbus_objectpath,
			SERVICE_PROVIDER_BUS_INTERFACE, signal_name, parameters, &err);
	if (err) {
		SAL_ERROR("Signal emit error : %d, %s", err->code, err->message);
		g_error_free(err);
		err = NULL;
		return SAL_ERROR_IPC_UNSTABLE;
	}
	return SAL_ERROR_NONE;
}
