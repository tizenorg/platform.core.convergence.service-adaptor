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

#include "service_adaptor_errors.h"
#include "service_adaptor_internal.h"
#include "sal.h"
#include "sal_ipc_server.h"
#include "sal_ipc_server_core.h"

//******************************************************************************
//* Global variables and defines
//******************************************************************************

//******************************************************************************
//* Private interface
//******************************************************************************

//******************************************************************************
//* Private interface definition
//******************************************************************************

//******************************************************************************
//* Public interface definition
//******************************************************************************

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

	int ipc_ret = SERVICE_ADAPTOR_ERROR_NONE;
	char *ipc_msg = NULL;
	char *ipc_type = NULL;
	GVariant *ipc_data = NULL;

	char *uri = NULL;

	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);

	if (0 == g_strcmp0(method_name, DBUS_SERVICE_ADAPTOR_CONNECT_METHOD))
	{
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

		for (gsize i = 0; i < plugins_size; i++)
		{
			ipc_insure_g_variant_builder_add_array_string(builder, plugins[i]);
		}

		ipc_create_error_msg(ipc_ret, &ipc_msg);
		ipc_data = g_variant_new(ipc_make_return_type(ipc_type), builder, ipc_ret, SAL_IPC_STR(ipc_msg));

		g_variant_builder_unref(builder);
		ipc_destroy_variant_info(req_info, size);
	}
	else if (0 == g_strcmp0(method_name, DBUS_SERVICE_ADAPTOR_DISCONNECT_METHOD))
	{
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

	int ipc_ret = SERVICE_ADAPTOR_ERROR_NONE;
	char *ipc_msg = NULL;
	char *ipc_type = NULL;
	GVariant *ipc_data = NULL;

	char *uri = NULL;

	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);

	if (0 == g_strcmp0(method_name, DBUS_SERVICE_PLUGIN_CREATE_METHOD))
	{
		int idx = 0;
		int size = service_plugin_create_s_type_length;
		GVariant *req_info[size];

		ipc_create_variant_info(in_parameters, size, (GVariant ***) &req_info);

		char *uri = ipc_insure_g_variant_dup_string(req_info[idx++]);

		SAL_INFO("uri: %s", uri);

		ipc_ret = SERVICE_ADAPTOR_ERROR_INTERNAL;

		sal_h sal = sal_get_handle();

		if (NULL != sal)
		{
			ipc_ret = auth_adaptor_ref_plugin(sal->auth, uri);
		}

		ipc_create_error_msg(ipc_ret, &ipc_msg);
		ipc_data = g_variant_new(ipc_make_return_type(ipc_type), ipc_ret, SAL_IPC_STR(ipc_msg));

		ipc_destroy_variant_info(req_info, size);
	}
	else if (0 == g_strcmp0(method_name, DBUS_SERVICE_PLUGIN_DESTROY_METHOD))
	{
		int idx = 0;
		int size = service_plugin_destroy_s_type_length;
		GVariant *req_info[size];

		ipc_create_variant_info(in_parameters, size, (GVariant ***) &req_info);

		char *uri = ipc_insure_g_variant_dup_string(req_info[idx++]);

		SAL_INFO("uri: %s", uri);

		ipc_ret = SERVICE_ADAPTOR_ERROR_INTERNAL;

		sal_h sal = sal_get_handle();

		if (NULL != sal)
		{
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
/*
service_adaptor_internal_error_code_e dbus_service_adaptor_signal_callback(service_adaptor_internal_signal_code_e signal_code,
						const char *signal_msg)
{
	SAL_FN_CALL;

	GError* error = NULL;
	GDBusConnection *dbus_connection = dbus_get_connection();

	if (NULL != dbus_connection)
	{
		GVariant *response = g_variant_new("(ts)", (uint64_t) signal_code, signal_msg);

		g_dbus_connection_emit_signal(dbus_connection,
				NULL,
				SERVICE_ADAPTOR_OBJECT_PATH,
				SERVICE_ADAPTOR_INTERFACE,
				DBUS_SERVICE_ADAPTOR_SIGNAL,
				response,
				&error );

		if (NULL != error)
		{
			service_adaptor_debug("Unable to send msg: %s", error->message);
			return SERVICE_ADAPTOR_INTERNAL_ERROR_DBUS;
		}
	}

	return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
}
*/
