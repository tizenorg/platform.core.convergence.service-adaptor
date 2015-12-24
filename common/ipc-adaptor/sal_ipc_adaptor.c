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
#include <app_control.h>

#include "sal_types.h"
#include "sal_log.h"
#include "sal_ipc.h"

/******************************************************************************
 * Global variables and defines
******************************************************************************/

/*
#define ERROR_MESSAGE_MAX_LENGH		2048

static __thread int last_error_code = 0;

static __thread char last_error_message[ERROR_MESSAGE_MAX_LENGH] = {0, };
*/

/******************************************************************************
 * Private interface
 ******************************************************************************/

static int __launch_app(const char *app_id,
		const char *operation);

static void __proxy_created_cb(GObject *source_object,
		GAsyncResult *res,
		gpointer user_data);

static void __on_name_appeared(GDBusConnection *connection,
		const gchar *name,
		const gchar *name_owner,
		gpointer user_data);

static void __on_name_vanished(GDBusConnection *connection,
		const gchar *name,
		gpointer user_data);

static void __on_signal(GDBusProxy *proxy,
		gchar *sender_name,
		gchar *signal_name,
		GVariant *parameters,
		gpointer user_data);


/******************************************************************************
 * Private interface definition
 ******************************************************************************/

static int __launch_app(const char *app_id, const char *operation)
{
	SAL_FN_CALL;
	int ret = SAL_ERROR_NONE;

	app_control_h app_control = NULL;
	ret = app_control_create(&app_control);
	TRYM_IF(ret, "app_control create failed : %d", ret);

	ret = app_control_set_app_id(app_control, app_id);
	TRYM_IF(ret, "app_control set app_id failed : %d", ret);

	ret = app_control_set_operation(app_control, operation);
	TRYM_IF(ret, "app_control set operation failed : %d", ret);

	ret = app_control_send_launch_request(app_control, NULL, NULL);
	TRYM_IF(ret, "app_control send launch request failed : %d", ret);

	app_control_destroy(app_control);
	app_control = NULL;

	SAL_FN_END;
	return ret;

catch:
	if (app_control)
		app_control_destroy(app_control);

	return SAL_ERROR_INTERNAL;
}

/* 0: adaptor handle, 1: callback, 2: user_data */
USER_DATA_TYPEDEF(ipc_adaptor_data_t, 3);

static void __proxy_created_cb(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
	SAL_FN_CALL;
	GError *error = NULL;
	GDBusProxy *_proxy = g_dbus_proxy_new_for_bus_finish(res, &error);
	if (error) {
		SAL_ERROR("Getting proxy error : %d, %s", error->code, error->message);
		g_error_free(error);
		error = NULL;
	} else if (_proxy) {
		SAL_INFO("Getting proxy success");
		USER_DATA_DEFINE(ipc_adaptor_data_t, _callback_data) = (USER_DATA_TYPE(ipc_adaptor_data_t) *)user_data;
		sal_ipc_adaptor_h _adaptor = (sal_ipc_adaptor_h)USER_DATA_ELEMENT(_callback_data, 0);
		_adaptor->proxy = _proxy;

		g_signal_connect(_proxy,
			"g-signal",
			G_CALLBACK(__on_signal),
			user_data);
	}
	SAL_FN_END;
}

static void __on_name_appeared(GDBusConnection *connection,
		const gchar *name,
		const gchar *name_owner,
		gpointer user_data)
{
	SAL_FN_CALL;

	SAL_INFO("Provider Appeared : name<%s>, name_owner<%s>", name, name_owner);
	USER_DATA_DEFINE(ipc_adaptor_data_t, _callback_data) = (USER_DATA_TYPE(ipc_adaptor_data_t) *)user_data;
	sal_ipc_adaptor_h adaptor = (sal_ipc_adaptor_h)USER_DATA_ELEMENT(_callback_data, 0);
	sal_ipc_adaptor_launch_cb callback = (sal_ipc_adaptor_launch_cb)USER_DATA_ELEMENT(_callback_data, 1);
	void *user_data = (void *)USER_DATA_ELEMENT(_callback_data, 2);

	adaptor->connected = true;
	callback(adaptor, true, user_data);

	SAL_FN_END;
}

static void __on_name_vanished(GDBusConnection *connection,
		const gchar *name,
		gpointer user_data)
{
	SAL_FN_CALL;

	SAL_INFO("Provider Vanished : name<%s>", name);
	USER_DATA_DEFINE(ipc_adaptor_data_t, _callback_data) = (USER_DATA_TYPE(ipc_adaptor_data_t) *)user_data;
	sal_ipc_adaptor_h adaptor = (sal_ipc_adaptor_h)USER_DATA_ELEMENT(_callback_data, 0);
	sal_ipc_adaptor_launch_cb callback = (sal_ipc_adaptor_launch_cb)USER_DATA_ELEMENT(_callback_data, 1);
	void *user_data = (void *)USER_DATA_ELEMENT(_callback_data, 2);

	adaptor->connected = false;
	callback(adaptor, false, user_data);

	SAL_FN_END;
}

static void __on_signal(GDBusProxy *proxy,
		gchar *sender_name,
		gchar *signal_name,
		GVariant *parameters,
		gpointer user_data)
{
	SAL_FN_CALL;

	SAL_FN_END;
}


/******************************************************************************
 * Public interface definition
 ******************************************************************************/

API int sal_ipc_adaptor_create(const char *plugin_uri, GMainContext *context, sal_ipc_adaptor_h *adaptor)
{
	SAL_FN_CALL;

#if !GLIB_CHECK_VERSION(2, 35, 0)
	g_type_init();
#endif

	int ret = SAL_ERROR_NONE;
	RETV_IF(NULL == plugin_uri, SAL_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == adaptor, SAL_ERROR_INVALID_PARAMETER);

	sal_ipc_adaptor_h _adaptor = NULL;
	char *_uri = NULL;
	GMainContext *_ctx = NULL;

	_adaptor = g_malloc0(sizeof(sal_ipc_adaptor_s));
	TRYVM_IF(NULL == _adaptor, ret = SAL_ERROR_OUT_OF_MEMORY, "Out of Memory");

	_uri = strdup(plugin_uri);
	TRYVM_IF(NULL == _uri, ret = SAL_ERROR_OUT_OF_MEMORY, "Out of Memory");

	if (context)
		_ctx = context;
	else if (NULL == (_ctx = g_main_context_get_thread_default()))
		_ctx = g_main_context_default();

	_adaptor->plugin_uri = _uri;
	_adaptor->ipc_context = _ctx;
	_adaptor->watcher_id = 0U;
	_adaptor->connection = NULL;
	_adaptor->proxy = NULL;
	_adaptor->connected = false;

	*adaptor = _adaptor;

	SAL_FN_END;
	return ret;

catch:
	free(_uri);
	free(_adaptor);

	return ret;
}

API int sal_ipc_adaptor_destroy(sal_ipc_adaptor_h adaptor)
{
	SAL_FN_CALL;
	int ret = SAL_ERROR_NONE;

	RETV_IF(NULL == adaptor, SAL_ERROR_INVALID_PARAMETER);

	free(adaptor->plugin_uri);
	if (adaptor->connection)
		g_object_unref(adaptor->connection);
	if (adaptor->proxy)
		g_object_unref(adaptor->proxy);
	if (adaptor->watcher_id)
		g_bus_unwatch_name(adaptor->watcher_id);

	adaptor->plugin_uri = NULL;
	adaptor->ipc_context = NULL;
	adaptor->watcher_id = 0U;
	adaptor->connection = NULL;
	adaptor->proxy = NULL;
	adaptor->connected = false;
	g_free(adaptor);

	SAL_FN_END;
	return ret;
}

API int sal_ipc_adaptor_channel_open_with_provider(sal_ipc_adaptor_h adaptor, sal_ipc_adaptor_launch_cb callback, void *user_data)
{
	SAL_FN_CALL;
	int ret = SAL_ERROR_NONE;

	RETV_IF(NULL == adaptor, SAL_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == callback, SAL_ERROR_INVALID_PARAMETER);

	USER_DATA_DEFINE(ipc_adaptor_data_t, _callback_data) = NULL;
	USER_DATA_VAL(_callback_data) = USER_DATA_CREATE(ipc_adaptor_data_t);
	RETV_IF(NULL == USER_DATA_VAL(_callback_data), SAL_ERROR_OUT_OF_MEMORY);

	USER_DATA_ELEMENT(_callback_data, 0) = (user_data_t)adaptor;
	USER_DATA_ELEMENT(_callback_data, 1) = (user_data_t)callback;
	USER_DATA_ELEMENT(_callback_data, 2) = (user_data_t)user_data;

	const char *_operation = "http://tizen.org/appcontrol/operation/service-provider/channel"
	ret = __launch_app(plugin_uri, _operation);
	RETV_IF(ret, SAL_ERROR_INTERNAL);

	char *bus_name = NULL;
	char *obj_parh = NULL;

	bus_name = g_strdup_printf("%s.%s", SERVICE_PROVIDER_BUS_NAME_PREFIX, adaptor->plugin_uri);
	TRYM_IF(NULL == bus_name, "Out of memory");

	obj_path = g_strdup(bus_name);
	TRYM_IF(NULL == obj_path, "Out of memory");
	SAL_STR_REPLACE(obj_path, '.', '/');

	GError *error = NULL;
	g_dbus_proxy_new_for_bus(G_BUS_TYPE_SESSION,
			G_DBUS_PROXY_FLAGS_NONE,
			(GDBusInterfaceInfo *)NULL,
			bus_name,
			obj_path,
			SERVICE_PROVIDER_BUS_INTERFACE,
			(GCancellable *)NULL,
			(GAsyncReadyCallback)__proxy_created_cb,
			(gpointer)USER_DATA_VAL(_callback_data));

	guint watcher_id = g_bus_watch_name(G_BUS_TYPE_SESSION,
			bus_name,
			G_BUS_NAME_WATCHER_FLAGS_NONE,
			__on_name_appeared,
			__on_name_vanished,
			(gpointer)USER_DATA_VAL(_callback_data),
			(GDestroyNotify)USER_DATA_DESTROY_FUNC);

	adaptor->watcher_id = watcher_id;

	SAL_FN_END;
	return ret;

catch:
	USER_DATA_DESTROY(_callback_data);
	g_free(bus_name);
	free(obj_path);

	SAL_FN_END;
	return ret;
}

API int sal_ipc_adaptor_channel_close_with_provider(sal_ipc_adaptor_h adaptor)
{
	SAL_FN_CALL;
	int ret = SAL_ERROR_NONE;

	RETV_IF(NULL == adaptor, SAL_ERROR_INVALID_PARAMETER);

	if (adaptor->watcher_id)
		g_bus_unwatch_name(adaptor->watcher_id);
	adaptor->watch_id = 0U;

	if (adaptor->proxy)
		g_object_unref(adaptor->proxy);
	adaptor->proxy = NULL;

	adaptor->connected = false;

	SAL_FN_END;
	return ret;
}

/*

API void sal_ipc_client_init_last_error(void)
{
	last_error_code = 0;
	last_error_message[0] = '\0';
}

API void sal_ipc_client_set_last_error(int error, const char *message)
{
	SAL_ERR("<thread-safe> set last error : [%d][%s]", error, message);

	last_error_code = error;
	if (!message)
		last_error_message[0] = '\0';
	else
		snprintf(last_error_message, ERROR_MESSAGE_MAX_LENGH, "%s", message);
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
	RETV_IF(false == g_variant_is_of_type(reply, (GVariantType *) reply_type), SAL_ERROR_INTERNAL);

	*reply_info = reply;

	return SAL_ERROR_NONE;
}

API int sal_ipc_client_get_simple_response(GVariant *response)
{
    int reply_size = RETURN_LENGTH;

    GVariant *reply_info[reply_size];
    ipc_create_variant_info(response, reply_size, (GVariant ***) &reply_info);

    int idx = 0;
    int ret = g_variant_get_int32(reply_info[idx++]);

	SAL_INFO("<<- ipc remote ret : %d ->>", ret);
	switch (ret) {
	case SAL_ERROR_NONE:
		SAL_DBG("case : SAL_ERROR_NONE");
		break;
	case SAL_ERROR_PLUGIN_FAILED:
		SAL_DBG("case : SAL_ERROR_PLUGIN_FAILED");
    	int ipc_ecode = g_variant_get_int32(reply_info[idx++]);
		char *ipc_msg = ipc_insure_g_variant_dup_string(reply_info[idx++]);

		sal_ipc_client_set_last_error(ipc_ecode, ipc_msg);
		SAL_FREE(ipc_msg);
		break;
	default:
		SAL_DBG("case : default");
		break;
	}

	ipc_destroy_variant_info(reply_info, reply_size);

	return ret;
}

API int sal_ipc_client_get_data_response(GVariant *response, GVariant **data)
{
    int reply_size = RETURN_LENGTH + 1;

    GVariant *reply_info[reply_size];
    ipc_create_variant_info(response, reply_size, (GVariant ***) &reply_info);

    int idx = 1;
    int ret = g_variant_get_int32(reply_info[idx++]);

	SAL_INFO("<<- ipc remote ret : %d ->>", ret);
	switch (ret) {
	case SAL_ERROR_NONE:
		SAL_DBG("case : SAL_ERROR_NONE");
		*data = g_variant_ref(reply_info[0]);
		break;
	case SAL_ERROR_PLUGIN_FAILED:
		SAL_DBG("case : SAL_ERROR_PLUGIN_FAILED");
    	int ipc_ecode = g_variant_get_int32(reply_info[idx++]);
		char *ipc_msg = ipc_insure_g_variant_dup_string(reply_info[idx++]);

		sal_ipc_client_set_last_error(ipc_ecode, ipc_msg);
		SAL_FREE(ipc_msg);
		break;
	default:
		SAL_DBG("case : default");
		break;
	}

	ipc_destroy_variant_info(reply_info, reply_size);

	return ret;
}
*/
