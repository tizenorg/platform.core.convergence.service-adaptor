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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <gio/gio.h>

#include "service-adaptor.h"
#include "service-adaptor-type.h"
#include "service-adaptor-log.h"
#include "dbus-service-adaptor.h"
#include "dbus-server.h"
#include "dbus-server-type.h"
#include "dbus-util.h"

#include "dbus-auth-adaptor.h"
#include "dbus-storage-adaptor.h"
#include "dbus-contact-adaptor.h"
#include "dbus-message-adaptor.h"
#include "dbus-shop-adaptor.h"

#include "util/ping_manager.h"

#define RET_MSG_LEN	2048

void service_adaptor_method_call(GDBusConnection *connection,
						const gchar *sender,
						const gchar *object_path,
						const gchar *interface_name,
						const gchar *method_name,
						GVariant *parameters,
						GDBusMethodInvocation *invocation,
						gpointer user_data)
{
FUNC_START();
	service_adaptor_internal_error_code_e ret_code = SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
	char ret_msg[RET_MSG_LEN] = {0,};

	if ((0 == g_strcmp0(method_name, PRIVATE_DBUS_CONNECT_SERVICE_ADAPTOR_METHOD)) ||
		(0 == g_strcmp0(method_name, DBUS_CONNECT_SERVICE_ADAPTOR_METHOD))) {
#if !GLIB_CHECK_VERSION(2, 32, 0)
		g_thread_init(NULL);
#endif
#if !GLIB_CHECK_VERSION(2, 35, 0)
		g_type_init();
#endif
		int waiting_cnt = 10;
		service_adaptor_h service_adaptor = service_adaptor_get_handle();

FUNC_STEP();
		while ((NULL == service_adaptor) && (0 < waiting_cnt)) {
			service_adaptor = service_adaptor_get_handle();
			waiting_cnt--;
			service_adaptor_debug("Retry service_adaptor_get_handle()");
			sleep(1);
		}

FUNC_STEP();
		if (NULL == service_adaptor) {
			service_adaptor_error("Cannot get handle of service adaptor");
			ret_code = SERVICE_ADAPTOR_INTERNAL_ERROR_INIT;
			strncpy(ret_msg, "Cannot get handle of service adaptor", (RET_MSG_LEN - 1));

			g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)", (uint64_t) ret_code, ret_msg));
			return;
		}

FUNC_STEP();
		gint64 timeout = g_get_monotonic_time() + 10 * G_TIME_SPAN_SECOND;
		g_mutex_lock(&service_adaptor->service_adaptor_mutex);

FUNC_STEP();
		while (!service_adaptor->started) {
			if (!g_cond_wait_until(&service_adaptor->service_adaptor_cond, &service_adaptor->service_adaptor_mutex, timeout)) {
				ret_code = SERVICE_ADAPTOR_INTERNAL_ERROR_INIT;
				strncpy(ret_msg, "Timeout", (RET_MSG_LEN - 1));
				service_adaptor_warning("Service adaptor Initalize timed out");

				g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)", (uint64_t) ret_code, ret_msg));
				g_mutex_unlock(&service_adaptor->service_adaptor_mutex);

				return;
			}
		}

		GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
		char *client_profile = NULL;
		g_variant_get(in_parameters, private_service_adaptor_essential_s_type, &client_profile);
		service_adaptor_info("[DM] client profile <%s>", client_profile);
		char client_name[1024] = {0, };
		int client_pid = 0;
		int sr = sscanf(client_profile, "%s%d", client_name, &client_pid);
		if (sr == 2) {
				service_adaptor_info("[DM] client<%s> pid<%d>", client_name, client_pid);
				#ifndef DETAILED_PEER
				ping_manager_peer_connected(client_pid);
				#else
				peer_info_s info;
				info.pid = client_pid;
				info.name = strdup(client_name);
				ping_manager_peer_connected(&info);
				free(info.name);
				info.name = NULL;
				#endif
		}

		g_mutex_unlock(&service_adaptor->service_adaptor_mutex);

		service_adaptor_info("Client Connected Successful");
		g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)", (uint64_t) ret_code, ret_msg));
FUNC_END();
	} else if (0 == g_strcmp0(method_name, DBUS_DISCONNECT_SERVICE_ADAPTOR_METHOD)) {
		GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
		char *client_profile = NULL;
		g_variant_get(in_parameters, private_service_adaptor_essential_s_type, &client_profile);
		service_adaptor_info("[DM] client profile <%s>", client_profile);
		char client_name[1024] = {0, };
		int client_pid = 0;
		int sr = sscanf(client_profile, "%s%d", client_name, &client_pid);
		if (sr == 2) {
				service_adaptor_info("[DM][dis] client<%s> pid<%d>", client_name, client_pid);
				#ifndef DETAILED_PEER
				ping_manager_peer_disconnected(client_pid);
				#else
				peer_info_s info;
				info.pid = client_pid;
				info.name = strdup(client_name);
				ping_manager_peer_disconnected(&info);
				free(info.name);
				info.name = NULL;
				#endif
		}

		service_adaptor_info("Client Disconnected Successful");
		g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)", (uint64_t) ret_code, ret_msg));
	} else if (0 == g_strcmp0(method_name, PRIVATE_DBUS_EXTERNAL_REQ_METHOD)) {
		service_adaptor_debug("[START] External request method");
		GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
		GVariant *req_struct[private_service_adaptor_external_req_s_type_length];

		for (size_t j = 0; j < private_service_adaptor_external_req_s_type_length; j++) {
			req_struct[j] = g_variant_get_child_value(in_parameters, j);
		}
		int idx = 0;
		char *service_name	= ipc_g_variant_dup_string(req_struct[idx++]);
		int service_flag	= g_variant_get_int32(req_struct[idx++]);
		char *api_uri		= ipc_g_variant_dup_string(req_struct[idx++]);

		service_adaptor_debug("service_name : %s / %d", service_name, service_flag);
		service_adaptor_debug("api_uri : %s", api_uri);
		int raw_data_len = g_variant_n_children(req_struct[idx]);
		unsigned char *raw_data = (unsigned char *) calloc((raw_data_len + 1), sizeof(unsigned char));

		if (NULL != raw_data) {
			for (int k = 0; k < raw_data_len; k++) {
				g_variant_get_child(req_struct[idx], k, "(y)", &(raw_data[k]));
			}
		}

		for (size_t j = 0; j < private_service_adaptor_external_req_s_type_length; j++) {
			g_variant_unref(req_struct[j]);
		}
		service_adaptor_debug_func("req_data_len (%d)", raw_data_len);
		service_adaptor_debug_func("req_data (%s)", raw_data);

		int ret = 0;
		unsigned char *res_data = NULL;
		int res_data_len = 0;

		/* Call API */
		switch (service_flag) {
		case SERVICE_ADAPTOR_PROTOCOL_SERVICE_TYPE_AUTH:
			service_adaptor_debug("Call auth_external_method_call");
			auth_external_method_call(service_name, api_uri, raw_data, raw_data_len,
					&res_data, &res_data_len, &ret, ret_msg);
			break;
		case SERVICE_ADAPTOR_PROTOCOL_SERVICE_TYPE_STORAGE:
			break;
		case SERVICE_ADAPTOR_PROTOCOL_SERVICE_TYPE_CONTACT:
			break;
		case SERVICE_ADAPTOR_PROTOCOL_SERVICE_TYPE_MESSAGE:
			break;
		case SERVICE_ADAPTOR_PROTOCOL_SERVICE_TYPE_SHOP:
			break;
		case SERVICE_ADAPTOR_PROTOCOL_SERVICE_TYPE_PUSH:
			break;
		default:
			break;
		}

		service_adaptor_debug_func("res_data_len (%d)", res_data_len);
		service_adaptor_debug_func("res_data (%s)", res_data);
		GVariantBuilder *builder_raw = g_variant_builder_new(G_VARIANT_TYPE(service_adaptor_raw_data_s_type));
		if (NULL != res_data) {
			for (int r = 0; r < res_data_len; r++) {
				g_variant_builder_add(builder_raw, "(y)", (guchar)res_data[r]);
			}
		}

		g_dbus_method_invocation_return_value(invocation,
				g_variant_new(MAKE_RETURN_TYPE(service_adaptor_raw_data_s_type), builder_raw, (uint64_t) ret, ret_msg));
		g_variant_builder_unref(builder_raw);
		service_adaptor_debug("[End] External request method");

		free(service_name);
		free(api_uri);
		free(raw_data);
		free(res_data);
	}
}

service_adaptor_internal_error_code_e dbus_push_data_callback(uint32_t service_id,
						push_adaptor_notification_data_h app_data,
						service_adaptor_internal_error_h error_code,
						void *server_data)
{
FUNC_START();
	GError* error = NULL;
	GDBusConnection *dbus_connection = dbus_get_connection();

	if (NULL != dbus_connection) {

		GVariant *response = g_variant_new(MAKE_RETURN_TYPE(private_service_adaptor_push_data_s_type), service_id, __safe_add_string(app_data->data), __safe_add_string(app_data->msg), app_data->time_stamp, (uint64_t) error_code->code, __safe_add_string(error_code->msg));

		g_dbus_connection_emit_signal(dbus_connection,
				NULL,
				SERVICE_ADAPTOR_OBJECT_PATH,
				SERVICE_ADAPTOR_INTERFACE,
				PRIVATE_DBUS_PUSH_DATA_SIGNAL,
				response,
				&error);

		if (NULL != error) {
			service_adaptor_debug("Unable to send msg: %s", error->message);
			return SERVICE_ADAPTOR_INTERNAL_ERROR_DBUS;
		}
	}

FUNC_END();
	return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
}

service_adaptor_internal_error_code_e dbus_service_adaptor_signal_callback(service_adaptor_internal_signal_code_e signal_code,
						const char *signal_msg)
{
FUNC_START();
	GError* error = NULL;
	GDBusConnection *dbus_connection = dbus_get_connection();

	if (NULL != dbus_connection) {
		GVariant *response = g_variant_new("(ts)", (uint64_t) signal_code, (signal_msg ? signal_msg : ""));

		g_dbus_connection_emit_signal(dbus_connection,
				NULL,
				SERVICE_ADAPTOR_OBJECT_PATH,
				SERVICE_ADAPTOR_INTERFACE,
				PRIVATE_DBUS_SERVICE_ADAPTOR_SIGNAL,
				response,
				&error);

		if (NULL != error) {
			service_adaptor_debug("Unable to send msg: %s", error->message);
			return SERVICE_ADAPTOR_INTERNAL_ERROR_DBUS;
		}
	}

FUNC_END();
	return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
}

