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
#include <unistd.h>

#include "service-adaptor.h"
#include "service-adaptor-push.h"
#include "service-adaptor-type.h"
#include "service-adaptor-log.h"
#include "dbus-push-adaptor.h"
#include "dbus-server.h"
#include "dbus-util.h"
#include "util/client_checker.h"

#define AUTH_FLAG	(0x01 << 0)
#define STORAGE_FLAG	(0x01 << 1)
#define RET_MSG_LEN	2048
#define __SAFE_STRDUP(x)	(x) ? strdup(x) : strdup("")

void push_adaptor_method_call(GDBusConnection *connection,
						const gchar *sender,
						const gchar *object_path,
						const gchar *interface_name,
						const gchar *method_name,
						GVariant *parameters,
						GDBusMethodInvocation *invocation,
						gpointer user_data)
{
	service_adaptor_internal_error_code_e ret_code = SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
	/* char ret_msg[RET_MSG_LEN] = "";
	memset(ret_msg, 0, RET_MSG_LEN); */

/************************************************************************
 *
 *                        private feature
 */
	if (0 == g_strcmp0(method_name, PRIVATE_DBUS_PUSH_REGISTER_METHOD)) {
		service_adaptor_debug("[START] Push register");
		GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
		GVariant *req_struct[private_service_adaptor_push_register_req_s_type_length];

		for (size_t j = 0; j < private_service_adaptor_push_register_req_s_type_length; j++) {
			req_struct[j] = g_variant_get_child_value(in_parameters, j);
		}

		int idx = 0;

		char *service_file_name	= ipc_g_variant_dup_string(req_struct[idx++]);

		service_adaptor_debug("service_file : %s", service_file_name);

		char *err = NULL;
		ret_code = service_adaptor_push_register(service_file_name, &err);

		g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)", (uint64_t) ret_code, __safe_add_string(err)));

		free(err);
		free(service_file_name);
		service_adaptor_debug("[End] Push register");
	} else if (0 == g_strcmp0(method_name, PRIVATE_DBUS_PUSH_DEREGISTER_METHOD)) {
		service_adaptor_debug("[START] Push deregister");
		GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
		GVariant *req_struct[private_service_adaptor_push_register_req_s_type_length];

		for (size_t j = 0; j < private_service_adaptor_push_register_req_s_type_length; j++) {
			req_struct[j] = g_variant_get_child_value(in_parameters, j);
		}

		int idx = 0;

		char *service_file_name	= ipc_g_variant_dup_string(req_struct[idx++]);

		service_adaptor_debug("service_file : %s", service_file_name);

		char *err = NULL;
		ret_code = service_adaptor_push_deregister(service_file_name, &err);

		g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)", (uint64_t) ret_code, __safe_add_string(err)));

		free(err);
		free(service_file_name);
		service_adaptor_debug("[End] Push deregister");
	}
/*
 *                       private feature
 *
 ***********************************************************************/
}

typedef struct __push_data_s {
	long long int timestamp;
	char *method;
	char *data;
	char *message;
} _push_data_s;

static void g_bus_async_ready_callback(GObject *source_object,
			GAsyncResult *res,
			gpointer user_data)
{
	service_adaptor_debug("[START] Push dbus proxy creation callback");
	GDBusProxy *_proxy = (GDBusProxy *) source_object;
	_push_data_s *push_data = (_push_data_s *) user_data;

	GVariant *req = g_variant_new("(xss)", push_data->timestamp, push_data->data, push_data->message);

	service_adaptor_info("Push dbus message send");
	g_dbus_proxy_call(_proxy,
			push_data->method,
			req,
			(G_DBUS_CALL_FLAGS_NONE | G_DBUS_MESSAGE_FLAGS_NO_REPLY_EXPECTED),
			-1,
			NULL,	/* GCancellable *cancellable */
			NULL,	/* GAsyncReadyCallback callback */
			NULL);	/* gpointer user_data */

	free(push_data->data);
	push_data->data = NULL;
	free(push_data->message);
	push_data->message = NULL;
	free(push_data->method);
	push_data->method = NULL;
	free(push_data);
	service_adaptor_debug("[End] Push dbus proxy creation callback");
}

void dbus_send_to_push_with_activation(int bus_type,
						const char *bus_name,
						const char *object_path,
						const char *interface,
						const char *method,
						void **proxy,
						long long int timestamp,
						const char *data,
						const char *message)
{
	service_adaptor_debug("[START] Push dbus activation message send");

	_push_data_s *push_data = (_push_data_s *) calloc(1, sizeof(_push_data_s));
	char *_data = strdup(data ? data : "");
	char *_message = strdup(message ? message : "");
	char *_method = strdup(method ? method : "");
	if ((NULL == push_data) || (NULL == _data) || (NULL == _message)) {
		free(push_data);
		free(_data);
		free(_message);
		free(_method);
		service_adaptor_error("Memory allocation failed");
		return;
	}
	push_data->timestamp	= timestamp;
	push_data->data		= _data;
	push_data->message	= _message;
	push_data->method	= _method;

	service_adaptor_debug_func("[PARAM_DBG] bus_type  : %d", bus_type);
	service_adaptor_debug_func("[PARAM_DBG] bus_name  : %s", bus_name);
	service_adaptor_debug_func("[PARAM_DBG] obj_path  : %s", object_path);
	service_adaptor_debug_func("[PARAM_DBG] interface : %s", interface);
	service_adaptor_debug_func("[PARAM_DBG] method    : %s", method);
	service_adaptor_debug_func("[PARAM_DBG] proxy     : %p", *proxy);
	service_adaptor_debug_func("");
	service_adaptor_debug_func("[PARAM_DBG] timestamp : %lld", timestamp);
	service_adaptor_debug_func("[PARAM_DBG] data      : %s", data);
	service_adaptor_debug_func("[PARAM_DBG] message   : %s", message);

	service_adaptor_info("Try dbus proxy creation (Async)");
	g_dbus_proxy_new_for_bus((GBusType) bus_type,
			G_DBUS_PROXY_FLAGS_NONE,
			NULL,
			bus_name,
			object_path,
			interface,
			NULL,
			g_bus_async_ready_callback,
			(void *) push_data);

	service_adaptor_debug("[End] Push dbus activation message send");
}
