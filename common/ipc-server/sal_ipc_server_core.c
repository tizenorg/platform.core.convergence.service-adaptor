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

#include <stdint.h>
#include <glib.h>
#include <gio/gio.h>

#include "sal_types.h"
#include "sal_log.h"
#include "sal.h"
#include "sal_ipc_server_core.h"


/******************************************************************************
 * Private interface
 ******************************************************************************/

/* request callback internal */
static void __connect_cb(ipc_server_session_h session);

static void __disconnect_cb(ipc_server_session_h session);

static void __plugin_start_cb(ipc_server_session_h session);

static void __plugin_stop_cb(ipc_server_session_h session);


/* response function internal */
static void __response_connect(ipc_server_session_h session, GList *plugin_uris);

static void __response_disconnect(ipc_server_session_h session);

static void __response_plugin_start(ipc_server_session_h session, const char *plugin_handle);

static void __response_plugin_stop(ipc_server_session_h session);

static void __response_fail(ipc_server_session_h session, int result, int error_code, const char *message);


/* response fail function internal */
static void __simple_fail_cb(ipc_server_session_h session, int ret, int err, const char *message);

static void __connect_fail_cb(ipc_server_session_h session, int ret, int err, const char *message);

static void __plugin_start_fail_cb(ipc_server_session_h session, int ret, int err, const char *message);


/******************************************************************************
 * Global variables and defines
 ******************************************************************************/

struct _dbus_interface_map
{
	char *method_name;
	void (*func)(void *data);
};

static struct _dbus_interface_map __interface_map[] = {
		{DBUS_SERVICE_ADAPTOR_CONNECT_METHOD,		__connect_cb},
		{DBUS_SERVICE_ADAPTOR_DISCONNECT_METHOD,	__disconnect_cb},
		{DBUS_SERVICE_PLUGIN_START_METHOD,			__plugin_start_cb},
		{DBUS_SERVICE_PLUGIN_STOP_METHOD,			__plugin_stop_cb},
		NULL,
	};

struct _dbus_fail_response_map
{
	char *method_name;
	void (*func)(ipc_server_session_h session, int ret, int err, const char *message);
};

static struct _dbus_fail_response_map __fail_response_map[] = {
		{DBUS_SERVICE_ADAPTOR_CONNECT_METHOD,		__connect_fail_cb},
		{DBUS_SERVICE_ADAPTOR_DISCONNECT_METHOD,	__simple_fail_cb},
		{DBUS_SERVICE_PLUGIN_START_METHOD,			__plugin_start_fail_cb},
		{DBUS_SERVICE_PLUGIN_STOP_METHOD,			__simple_fail_cb},
		NULL,
	};


static ipc_server_base_req_s req_callbacks = {0, };

static ipc_server_base_res_s response_methods = {
		__response_connect,
		__response_disconnect,
		__response_plugin_start,
		__response_plugin_stop,
		__response_fail,
	};

/******************************************************************************
 * Private interface definition
 ******************************************************************************/

/* request callbacks  */
static void __connect_cb(ipc_server_session_h session)
{
	SAL_FN_CALL;

	sal_debug("gets parameters from gvriant");
	int _client_pid = 0;
	char *_client_uri = NULL;
	g_variant_get_child(session->parameters, 0, service_adaptor_connect_req_s_type,
			&_client_pid, &_client_uri);

	sal_debug("invokes callback");
	req_callbacks.connect_cb(session, _client_pid, _client_uri);

	SAL_FREE(_client_uri);

	SAL_FN_END;
}

static void __disconnect_cb(ipc_server_session_h session)
{
	SAL_FN_CALL;

	sal_debug("gets parameters from gvriant");
	int _client_pid = 0;
	char *_client_uri = NULL;
	g_variant_get_child(session->parameters, 0, service_adaptor_disconnect_req_s_type,
			&_client_pid, &_client_uri);

	sal_debug("invokes callback");
	req_callbacks.connect_cb(session, _client_pid, _client_uri);

	SAL_FREE(_client_uri);
	SAL_FN_END;
}

static void __plugin_start_cb(ipc_server_session_h session)
{
	SAL_FN_CALL;

	sal_debug("gets parameters from gvriant");
	int _client_pid = 0;
	char *_client_uri = NULL;
	char *_plugin_uri = NULL;
	int _service_mask = 7;
	g_variant_get_child(session->parameters, 0, service_adaptor_connect_req_s_type,
			&_client_pid, &_client_uri, &_plugin_uri);

	sal_debug("invokes callback");
	req_callbacks.plugin_start_cb(session, _client_pid, _client_uri,
			_plugin_uri, _service_mask);
	/* TODO support service_mask*/

	SAL_FREE(_client_uri);
	SAL_FREE(_plugin_uri);

	SAL_FN_END;
}

static void __plugin_stop_cb(ipc_server_session_h session)
{
	SAL_FN_CALL;

	sal_debug("gets parameters from gvriant");
	char *_plugin_handle = NULL;
	g_variant_get_child(session->parameters, 0, service_adaptor_connect_req_s_type,
			&_plugin_handle);

	sal_debug("invokes callback");
	req_callbacks.plugin_stop_cb(session, _plugin_handle);

	SAL_FREE(_plugin_handle);

	SAL_FN_END;
}


/* response functions  */
static void __response_connect(ipc_server_session_h session, GList *plugin_uris)
{
	SAL_FN_CALL;

	GVariantBuilder *plugins = g_variant_builder_new("a(s)");

	SAL_FOREACH_GLIST(iterator, plugin_uris) {
		sal_debug("plugin_uri : %s", (const char *)iterator->data);
		ipc_insure_g_variant_builder_add_array_string(plugins, (const char *)iterator->data);
	}

	sal_debug("creates response to gvaiant");
	GVariant *response = g_variant_new(SAL_IPC_RETURN_TYPE(service_adaptor_connect_res_s_type),
			builder, SAL_IPC_PAYLOAD_SKIP);

	sal_debug("invoke gdbus response");
	g_dbus_method_invocation_return_value(session->invocation, response);

	g_variant_builder_unref(plugins);
	g_variant_unref(response);
	g_free(session);

	SAL_FN_END;
}

static void __response_disconnect(ipc_server_session_h session)
{
	SAL_FN_CALL;

	sal_debug("creates response to gvaiant");
	GVariant *response = g_variant_new(SAL_IPC_SIMPLE_TYPE, SAL_IPC_PAYLOAD_SKIP);

	sal_debug("invoke gdbus response");
	g_dbus_method_invocation_return_value(session->invocation, response);

	g_variant_unref(response);
	g_free(session);

	SAL_FN_END;
}

static void __response_plugin_start(ipc_server_session_h session, const char *plugin_handle)
{
	SAL_FN_CALL;

	sal_debug("creates response to gvaiant");
	GVariant *response = g_variant_new(SAL_IPC_RETURN_TYPE(service_plugin_start_res_s_type),
			plugin_handle, SAL_IPC_PAYLOAD_SKIP);

	sal_debug("invoke gdbus response");
	g_dbus_method_invocation_return_value(session->invocation, response);

	g_variant_unref(response);
	g_free(session);

	SAL_FN_END;
}

static void __response_plugin_stop(ipc_server_session_h session)
{
	SAL_FN_CALL;

	sal_debug("creates response to gvaiant");
	GVariant *response = g_variant_new(SAL_IPC_SIMPLE_TYPE, SAL_IPC_PAYLOAD_SKIP);

	sal_debug("invoke gdbus response");
	g_dbus_method_invocation_return_value(session->invocation, response);

	g_variant_unref(response);
	g_free(session);

	SAL_FN_END;
}

static void __response_fail(ipc_server_session_h session, int result, int error_code, const char *message)
{
	SAL_FN_CALL;

	for (int i = 0; __fail_response_map[i]; i++) {
		if (!strncmp(session->method_name, __fail_response_map[i].method_name,
				strlen(__fail_response_map[i].method_name))) {
			sal_debug("<%s> method return fail", session->method_name);
			__fail_response_map[i].func(session, result, error_code, message);
		}
	}

	/* TODO unref session->invocation or return error */
	g_free(session);

	SAL_FN_END;
}

static void __simple_fail_cb(ipc_server_session_h session, int ret, int err, const char *message)
{
	SAL_FN_CALL;

	sal_debug("creates response to gvaiant");
	GVariant *response = g_variant_new(SAL_IPC_SIMPLE_TYPE,
			ret, err, SAL_IPC_SAFE_STR(message));

	sal_debug("invoke gdbus response");
	g_dbus_method_invocation_return_value(session->invocation, response);

	g_variant_unref(response);
	g_free(session);

	SAL_FN_END;
}

static void __connect_fail_cb(ipc_server_session_h session, int ret, int err, const char *message)
{
	SAL_FN_CALL;

	sal_debug("creates response to gvaiant");
	GVariantBuilder *plugins = g_variant_builder_new("a(s)");
	GVariant *response = g_variant_new(SAL_IPC_RETURN_TYPE(service_adaptor_connect_res_s_type),
			builder, ret, err, SAL_IPC_SAFE_STR(message));

	sal_debug("invoke gdbus response");
	g_dbus_method_invocation_return_value(session->invocation, response);

	g_variant_builder_unref(plugins);
	g_variant_unref(response);
	g_free(session);

	SAL_FN_END;
}



static void __plugin_start_fail_cb(ipc_server_session_h session, int ret, int err, const char *message)
{
	SAL_FN_CALL;

	sal_debug("creates response to gvaiant");
	GVariant *response = g_variant_new(SAL_IPC_RETURN_TYPE(service_plugin_start_res_s_type),
			"", ret, err, SAL_IPC_SAFE_STR(message));

	sal_debug("invoke gdbus response");
	g_dbus_method_invocation_return_value(session->invocation, response);

	g_variant_unref(response);
	g_free(session);

	SAL_FN_END;
}


/******************************************************************************
 * Public interface definition
 ******************************************************************************/

API int ipc_server_base_init(ipc_server_base_req_s *base_req)
{
	SAL_FN_CALL;

	RETV_IF(NULL == base_req, SAL_ERROR_INTERNAL);
	RETV_IF(NULL == base_req->connect_cb, SAL_ERROR_INTERNAL);
	RETV_IF(NULL == base_req->disconnect_cb, SAL_ERROR_INTERNAL);
	RETV_IF(NULL == base_req->plugin_start_cb, SAL_ERROR_INTERNAL);
	RETV_IF(NULL == base_req->plugin_stop_cb, SAL_ERROR_INTERNAL);

	req_callbacks.connect_cb		= base_req->connect_cb;
	req_callbacks.disconnect_cb		= base_req->disconnect_cb;
	req_callbacks.plugin_start_cb	= base_req->plugin_start_cb;
	req_callbacks.plugin_stop_cb	= base_req->plugin_stop_cb;

	SAL_FN_END;
	return SAL_ERROR_NONE;
}

/* It will be invoked on working thread */
API gboolean sal_server_base_method_call(void *data)
{
	SAL_FN_CALL;

	ipc_server_session_h session = (ipc_server_session_h) data;
	sal_info("===== method called : %s =====", session->method_name);

	bool catched = false;
	for (int i = 0; __interface_map[i]; i++) {
		if (!strncmp(session->method_name, __interface_map[i].method_name,
				strlen(__interface_map[i].method_name))) {
			catched = true;
			__interface_map[i].func(session);
		}
	}

	if (false == catched) {
		/* TODO add error handling */
		sal_error("function does not matched (%s)", ipc_data->method_name);
	}

	SAL_FN_END;
	return FALSE;
}

API ipc_server_base_res_s *ipc_server_get_base_res_handle(void)
{
	return &response_methods;
}

/*
API void service_adaptor_method_call(GDBusConnection *connection,
		const gchar *sender,
		const gchar *object_path,
		const gchar *interface_name,
		const gchar *method_name,
		GVariant *parameters,
		GDBusMethodInvocation *invocation,
		gpointer user_data)
{
	SAL_FN_CALL;

	int ipc_ret = SAL_ERROR_NONE;
	char *ipc_msg = NULL;
	char *ipc_type = NULL;
	GVariant *ipc_data = NULL;

	char *uri = NULL;

	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);

	if (0 == g_strcmp0(method_name, DBUS_SERVICE_ADAPTOR_CONNECT_METHOD)) {
		int idx = 0;
		int size = service_adaptor_connect_req_s_type_length;
		GVariant *req_info[size];

		ipc_create_variant_info(in_parameters, size, (GVariant ***) &req_info);

		char *uri = ipc_insure_g_variant_dup_string(req_info[idx++]);

		SAL_INFO("uri: %s", uri);

		ipc_type = strdup(service_adaptor_connect_res_s_type);
		ipc_ret = sal_adaptor_connect(uri);

		int plugins_size = 0;
		char **plugins = NULL;
		GVariantBuilder *builder = g_variant_builder_new(G_VARIANT_TYPE(plugin_list_type));

		ipc_ret = sal_adaptor_get_plugins(&plugins, &plugins_size);

		for (gsize i = 0; i < plugins_size; i++) {
			ipc_insure_g_variant_builder_add_array_string(builder, plugins[i]);
		}

		ipc_create_error_msg(ipc_ret, &ipc_msg);
		ipc_data = g_variant_new(ipc_make_return_type(ipc_type), builder, ipc_ret, SAL_IPC_STR(ipc_msg));

		g_variant_builder_unref(builder);
		ipc_destroy_variant_info(req_info, size);
	} else if (0 == g_strcmp0(method_name, DBUS_SERVICE_ADAPTOR_DISCONNECT_METHOD)) {
		int idx = 0;
		int size = service_adaptor_disconnect_s_type_length;
		GVariant *req_info[size];

		ipc_create_variant_info(in_parameters, size, (GVariant ***) &req_info);

		char *uri = ipc_insure_g_variant_dup_string(req_info[idx++]);

		SAL_INFO("uri: %s", uri);

		ipc_ret = sal_adaptor_disconnect(uri);

		ipc_create_error_msg(ipc_ret, &ipc_msg);
		ipc_data = g_variant_new(ipc_make_return_type(ipc_type), ipc_ret, SAL_IPC_STR(ipc_msg));

		ipc_destroy_variant_info(req_info, size);
	}

	g_dbus_method_invocation_return_value(invocation, ipc_data);

	SAL_FREE(uri);
	SAL_FREE(ipc_msg);
	SAL_FREE(ipc_type);

	SAL_FN_END;
}

API void service_plugin_method_call(GDBusConnection *connection,
		const gchar *sender,
		const gchar *object_path,
		const gchar *interface_name,
		const gchar *method_name,
		GVariant *parameters,
		GDBusMethodInvocation *invocation,
		gpointer user_data)
{
	SAL_FN_CALL;

	int ipc_ret = SAL_ERROR_NONE;
	char *ipc_msg = NULL;
	char *ipc_type = NULL;
	GVariant *ipc_data = NULL;

	char *uri = NULL;

	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);

	if (0 == g_strcmp0(method_name, DBUS_SERVICE_PLUGIN_CREATE_METHOD)) {
		int idx = 0;
		int size = service_plugin_create_s_type_length;
		GVariant *req_info[size];

		ipc_create_variant_info(in_parameters, size, (GVariant ***) &req_info);

		char *uri = ipc_insure_g_variant_dup_string(req_info[idx++]);

		SAL_INFO("uri: %s", uri);

		ipc_ret = SAL_ERROR_INTERNAL;

		sal_h sal = sal_get_handle();

		if (NULL != sal) {
			ipc_ret = auth_adaptor_ref_plugin(sal->auth, uri);
		}

		ipc_create_error_msg(ipc_ret, &ipc_msg);
		ipc_data = g_variant_new(ipc_make_return_type(ipc_type), ipc_ret, SAL_IPC_STR(ipc_msg));

		ipc_destroy_variant_info(req_info, size);
	} else if (0 == g_strcmp0(method_name, DBUS_SERVICE_PLUGIN_DESTROY_METHOD)) {
		int idx = 0;
		int size = service_plugin_destroy_s_type_length;
		GVariant *req_info[size];

		ipc_create_variant_info(in_parameters, size, (GVariant ***) &req_info);

		char *uri = ipc_insure_g_variant_dup_string(req_info[idx++]);

		SAL_INFO("uri: %s", uri);

		ipc_ret = SAL_ERROR_INTERNAL;

		sal_h sal = sal_get_handle();

		if (NULL != sal) {
			ipc_ret = auth_adaptor_unref_plugin(sal->auth, uri);
		}

		ipc_create_error_msg(ipc_ret, &ipc_msg);
		ipc_data = g_variant_new(ipc_make_return_type(ipc_type), ipc_ret, SAL_IPC_STR(ipc_msg));

		ipc_destroy_variant_info(req_info, size);
	}

	g_dbus_method_invocation_return_value(invocation, ipc_data);

	SAL_FREE(uri);
	SAL_FREE(ipc_msg);
	SAL_FREE(ipc_type);

	SAL_FN_END;
}
*/
/*
service_adaptor_internal_error_code_e dbus_service_adaptor_signal_callback(service_adaptor_internal_signal_code_e signal_code,
						const char *signal_msg)
{
	SAL_FN_CALL;

	GError* error = NULL;
	GDBusConnection *dbus_connection = dbus_get_connection();

	if (NULL != dbus_connection) {
		GVariant *response = g_variant_new("(ts)", (uint64_t) signal_code, signal_msg);

		g_dbus_connection_emit_signal(dbus_connection,
				NULL,
				SERVICE_ADAPTOR_OBJECT_PATH,
				SERVICE_ADAPTOR_INTERFACE,
				DBUS_SERVICE_ADAPTOR_SIGNAL,
				response,
				&error );

		if (NULL != error) {
			service_adaptor_debug("Unable to send msg: %s", error->message);
			return SERVICE_ADAPTOR_INTERNAL_ERROR_DBUS;
		}
	}

	return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
}
*/
