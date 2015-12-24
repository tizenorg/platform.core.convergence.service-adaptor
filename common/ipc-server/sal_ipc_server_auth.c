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
#include "sal_ipc_server.h"
#include "sal_ipc_server_auth.h"
#include "auth_adaptor.h"
#include "sal_service_auth.h"
#include "sal_service_auth_internal.h"

/******************************************************************************
 * Global variables and defines
 ******************************************************************************/

/******************************************************************************
 * Private interface
 ******************************************************************************/

/******************************************************************************
 * Private interface definition
 ******************************************************************************/

void _oauth1_get_access_token_cb(int result, oauth1_h oauth1, void *user_data)
{
	SAL_FN_CALL;

	ipc_reply_data_h reply = (ipc_reply_data_h) user_data;

	int ipc_ret = SAL_ERROR_NONE;
	char *ipc_msg = NULL;
	GVariant *ipc_data = NULL;

	ipc_create_error_msg(ipc_ret, &ipc_msg);
	ipc_data = g_variant_new(ipc_make_return_type(reply->type), SAL_IPC_STR(oauth1->access_token), SAL_IPC_STR(oauth1->operation), ipc_ret, SAL_IPC_STR(ipc_msg));
	g_dbus_method_invocation_return_value(reply->invocation, ipc_data);

	SAL_FREE(ipc_msg);
	ipc_free_reply_data(reply);
}

int _get_oauth1(GVariant *reply_info, service_auth_oauth1_h *oauth1)
{
	SAL_FN_CALL;

	service_auth_oauth1_h auth_oauth1 = (service_auth_oauth1_h) g_malloc0(sizeof(service_auth_oauth1_s));

	int info_size = service_auth_oauth1_s_type_length;
	GVariant *info[info_size];
	ipc_create_variant_info(reply_info, info_size, (GVariant ***) &info);

	int idx = 0;
	auth_oauth1->access_token = ipc_insure_g_variant_dup_string(info[idx++]);
	auth_oauth1->operation = ipc_insure_g_variant_dup_string(info[idx++]);

	ipc_destroy_variant_info(info, info_size);

	*oauth1 = auth_oauth1;

	return SAL_ERROR_NONE;
}

int _oauth1_execute_operation(auth_plugin_h plugin, service_auth_oauth1_h oauth1, ipc_reply_data_h reply)
{
	SAL_FN_CALL;

	RETV_IF(NULL == plugin, SAL_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == plugin->oauth1, SAL_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == oauth1, SAL_ERROR_INVALID_PARAMETER);

	int ret = SAL_ERROR_NONE;

	if (0 == strcmp(oauth1->operation, SERVICE_AUTH_OAUTH1_0_GET_ACCESS_TOKEN_URI)) {
		ret = plugin->oauth1->oauth1_get_access_token(plugin, _oauth1_get_access_token_cb, reply);

		return ret;
	} else if (0 == strcmp(oauth1->operation, SERVICE_AUTH_OAUTH1_0_GET_EXTRA_DATA_URI)) {
		return ret;
	}

	return SAL_ERROR_INTERNAL;
}

/******************************************************************************
 * Public interface definition
 ******************************************************************************/

API void service_auth_method_call(GDBusConnection *connection,
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

	if (0 == g_strcmp0(method_name, DBUS_SERVICE_AUTH_OAUTH1_METHOD)) {
		int idx = 0;
		int size = service_auth_oauth1_req_s_type_length;
		GVariant *req_info[size];

		ipc_create_variant_info(in_parameters, size, (GVariant ***) &req_info);

		char *uri = ipc_insure_g_variant_dup_string(req_info[idx++]);
		service_auth_oauth1_h oauth1 = NULL;
		_get_oauth1(req_info[idx++], &oauth1);

		SAL_INFO("uri: %s", uri);

		ipc_ret = SAL_ERROR_INTERNAL;
		ipc_type = strdup(service_auth_oauth1_res_s_type);

		sal_h sal = sal_get_handle();
		TRYVM_IF(NULL == sal, ipc_ret = SAL_ERROR_INTERNAL, "sal_get_handle() Failed");

		auth_plugin_h plugin = auth_adaptor_get_plugin(sal->auth, uri);

		ipc_reply_data_h reply = (ipc_reply_data_h) g_malloc0(sizeof(ipc_reply_data_s));
		reply->invocation = invocation;
		reply->type = strdup(ipc_type);

		ipc_ret = _oauth1_execute_operation(plugin, oauth1, reply);
		TRY_IF(SAL_ERROR_NONE == ipc_ret, "oauth1_execute_operation() Request Successed");

		ipc_create_error_msg(ipc_ret, &ipc_msg);
		ipc_data = g_variant_new(ipc_make_return_type(ipc_type), SAL_IPC_STR(NULL), SAL_IPC_STR(NULL), ipc_ret, SAL_IPC_STR(ipc_msg));

		ipc_destroy_variant_info(req_info, size);
	}

	g_dbus_method_invocation_return_value(invocation, ipc_data);

catch:
	SAL_FREE(uri);
	SAL_FREE(ipc_msg);
	SAL_FREE(ipc_type);

	SAL_FN_END;
}
