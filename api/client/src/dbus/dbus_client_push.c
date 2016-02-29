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
 * File: dbus-client-push.c
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

#include <dbus-server.h>

#include "dbus_client.h"
#include "dbus_client_push.h"
#include "service_adaptor_client_type.h"
#include "private/service-adaptor-client-push.h"
#include "service_adaptor_client_log.h"

#include "util/service_adaptor_client_util.h"
/******************************************************************************
 * Global variables and defines
 ******************************************************************************/

/******************************************************************************
 * Private interface
 ******************************************************************************/

/******************************************************************************
 * Private interface definition
 ******************************************************************************/

#define __SAFE_FREE(x)          do { free(x); (x) = NULL; } while (0)

void __get_push_data_type(GVariant *parameters,
						uint32_t *service_id,
						service_adaptor_push_notification_s *noti_info)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *res_struct[private_service_adaptor_push_data_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_push_data_s_type_length; j++) {
		res_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*service_id = g_variant_get_uint32(res_struct[idx++]);
	noti_info->data = ipc_g_variant_dup_string(res_struct[idx++]);
	noti_info->message = ipc_g_variant_dup_string(res_struct[idx++]);
	noti_info->time = g_variant_get_int64(res_struct[idx++]);

	for (size_t j = 0; j < private_service_adaptor_push_data_s_type_length; j++) {
		g_variant_unref(res_struct[j]);
	}
}

/******************************************************************************
 * Public interface definition
 ******************************************************************************/

void on_push_signal(GDBusProxy *proxy,
						gchar *sender_name,
						gchar *signal_name,
						GVariant *parameters,
						gpointer user_data)
{
	if (0 == g_strcmp0(signal_name, PRIVATE_DBUS_PUSH_DATA_SIGNAL)) {
		uint32_t service_id = 0;
		service_adaptor_push_notification_s noti_info;
		service_adaptor_error_s *error_code = NULL;
		service_adaptor_error_s error;
		error.msg = NULL;
		__get_push_data_type(parameters, &service_id, &noti_info);

		service_adaptor_task_h task = _signal_queue_get_task(service_id);

		sac_debug("==on_push_signal== task(%p) service_id(%u)", task, service_id);

		if (NULL == task) {
			__SAFE_FREE(noti_info.data);
			__SAFE_FREE(noti_info.message);
			return;
		}

		GVariant *call_result[2];
		call_result[0] = g_variant_get_child_value(parameters, 1);
		call_result[1] = g_variant_get_child_value(parameters, 2);

		uint64_t remote_call_result = g_variant_get_uint64(call_result[0]);

		if (SERVICE_ADAPTOR_ERROR_NONE != remote_call_result) {
			error_code = &error;

			if (NULL != error_code) {
				error_code->code = remote_call_result;
				error_code->msg = ipc_g_variant_dup_string(call_result[1]);
			}
		}

		sac_debug("push data : %s", noti_info.data);
		sac_debug("push message : %s", noti_info.message);
		sac_debug("push time : %lld", noti_info.time);

		service_adaptor_push_notification_cb callback = (service_adaptor_push_notification_cb) task->callback;

		if (NULL != callback) {
			callback(task->handle, &noti_info, error_code, task->user_data);
		}
		__SAFE_FREE(noti_info.data);
		__SAFE_FREE(noti_info.message);
		__SAFE_FREE(error.msg);
	}
}

static int __dbus_push_simple_operation(const char *file_name,
						service_adaptor_error_s *error,
						const char *method_name)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();

	ipc_check_proxy(sac_interface_proxy);

	GVariant *request = g_variant_new("(" private_service_adaptor_push_register_req_s_type ")", file_name);

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			method_name,
			request,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
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

	return ret;
}

int _dbus_push_register(const char *file_name,
						service_adaptor_error_s *error)
{
	return __dbus_push_simple_operation(file_name, error, PRIVATE_DBUS_PUSH_REGISTER_METHOD);
}

int _dbus_push_deregister(const char *file_name,
						service_adaptor_error_s *error)
{
	return __dbus_push_simple_operation(file_name, error, PRIVATE_DBUS_PUSH_DEREGISTER_METHOD);
}
