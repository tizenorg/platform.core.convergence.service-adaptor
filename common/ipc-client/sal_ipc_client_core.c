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

#include "service_adaptor_errors.h"
#include "service_adaptor_internal.h"
#include "sal_ipc_client.h"
#include "sal_ipc_client_core.h"

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

API int ipc_service_adaptor_connect(const char *uri, GList **plugins)
{
	SAL_FN_CALL;

	RETV_IF(NULL == uri, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	char *request_method = DBUS_SERVICE_ADAPTOR_CONNECT_METHOD;
	GVariant *request_data = g_variant_new("(" service_adaptor_connect_req_s_type ")", uri);

	char *reply_type = service_adaptor_connect_res_s_type;
	int reply_size = RETURN_LENGTH + 1;
	GVariant *reply = NULL;

	ret = sal_ipc_client_call_request(request_method, request_data, reply_type, &reply);
	RETVM_IF(SERVICE_ADAPTOR_ERROR_NONE != ret, ret, "ipc_client_call_request() Failed(%d)", ret);

	GVariant *reply_info[reply_size];
	ipc_create_variant_info(reply, reply_size, (GVariant ***) &reply_info);

	int idx = 0;
	int info_size = service_adaptor_connect_res_s_type_length;
	GVariant *info[info_size];
	ipc_create_variant_info(reply_info[idx++], info_size, (GVariant ***) &info);

	int idx2 = 0;
	GList *plugin_list = NULL;
	gsize uri_info_size = g_variant_n_children(info[idx2]);

	for (gsize i = 0; i < uri_info_size; i++)
	{
		GVariant *uri_info_struct;
		GVariant *uri_info_entry_v = g_variant_get_child_value(info[idx2], i);
		uri_info_struct = g_variant_get_child_value(uri_info_entry_v, 0);

		plugin_list = g_list_append(plugin_list, ipc_insure_g_variant_dup_string(uri_info_struct));

		g_variant_unref(uri_info_struct);
	}

	*plugins = plugin_list;

	int ipc_ret = g_variant_get_int32(reply_info[idx++]);
	char *ipc_msg = ipc_insure_g_variant_dup_string(reply_info[idx++]);

	ipc_destroy_variant_info(reply_info, reply_size);

	g_variant_unref(reply);

	RETVM_IF(SERVICE_ADAPTOR_ERROR_NONE != ipc_ret, SERVICE_ADAPTOR_ERROR_INTERNAL, "IPC Result Failed(%d): %s", ipc_ret, ipc_msg);

	SAL_FREE(ipc_msg);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int ipc_service_adaptor_disconnect(const char *uri)
{
	SAL_FN_CALL;

	RETV_IF(NULL == uri, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	char *request_method = DBUS_SERVICE_ADAPTOR_DISCONNECT_METHOD;
	GVariant *request_data = g_variant_new("(" service_adaptor_disconnect_s_type ")", uri);

	char *reply_type = NULL;
	int reply_size = RETURN_LENGTH;
	GVariant *reply = NULL;

	ret = sal_ipc_client_call_request(request_method, request_data, reply_type, &reply);
	RETVM_IF(SERVICE_ADAPTOR_ERROR_NONE != ret, ret, "ipc_client_call_request() Failed(%d)", ret);

	GVariant *reply_info[reply_size];
	ipc_create_variant_info(reply, reply_size, (GVariant ***) &reply_info);

	int idx = 0;
	int ipc_ret = g_variant_get_int32(reply_info[idx++]);
	char *ipc_msg = ipc_insure_g_variant_dup_string(reply_info[idx++]);

	ipc_destroy_variant_info(reply_info, reply_size);

	g_variant_unref(reply);

	RETVM_IF(SERVICE_ADAPTOR_ERROR_NONE != ipc_ret, SERVICE_ADAPTOR_ERROR_INTERNAL, "IPC Result Failed(%d): %s", ipc_ret, ipc_msg);

	SAL_FREE(ipc_msg);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int ipc_service_plugin_create(const char *uri)
{
	SAL_FN_CALL;

	RETV_IF(NULL == uri, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	char *request_method = DBUS_SERVICE_PLUGIN_CREATE_METHOD;
	GVariant *request_data = g_variant_new("(" service_plugin_create_s_type ")", uri);

	char *reply_type = NULL;
	int reply_size = RETURN_LENGTH;
	GVariant *reply = NULL;

	ret = sal_ipc_client_call_request(request_method, request_data, reply_type, &reply);
	RETVM_IF(SERVICE_ADAPTOR_ERROR_NONE != ret, ret, "ipc_client_call_request() Failed(%d)", ret);

	GVariant *reply_info[reply_size];
	ipc_create_variant_info(reply, reply_size, (GVariant ***) &reply_info);

	int idx = 0;
	int ipc_ret = g_variant_get_int32(reply_info[idx++]);
	char *ipc_msg = ipc_insure_g_variant_dup_string(reply_info[idx++]);

	ipc_destroy_variant_info(reply_info, reply_size);

	g_variant_unref(reply);

	RETVM_IF(SERVICE_ADAPTOR_ERROR_NONE != ipc_ret, SERVICE_ADAPTOR_ERROR_INTERNAL, "IPC Result Failed(%d): %s", ipc_ret, ipc_msg);

	SAL_FREE(ipc_msg);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int ipc_service_plugin_destroy(const char *uri)
{
	SAL_FN_CALL;

	RETV_IF(NULL == uri, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	char *request_method = DBUS_SERVICE_PLUGIN_DESTROY_METHOD;
	GVariant *request_data = g_variant_new("(" service_plugin_destroy_s_type ")", uri);

	char *reply_type = NULL;
	int reply_size = RETURN_LENGTH;
	GVariant *reply = NULL;

	ret = sal_ipc_client_call_request(request_method, request_data, reply_type, &reply);
	RETVM_IF(SERVICE_ADAPTOR_ERROR_NONE != ret, ret, "ipc_client_call_request() Failed(%d)", ret);

	GVariant *reply_info[reply_size];
	ipc_create_variant_info(reply, reply_size, (GVariant ***) &reply_info);

	int idx = 0;
	int ipc_ret = g_variant_get_int32(reply_info[idx++]);
	char *ipc_msg = ipc_insure_g_variant_dup_string(reply_info[idx++]);

	ipc_destroy_variant_info(reply_info, reply_size);

	g_variant_unref(reply);

	RETVM_IF(SERVICE_ADAPTOR_ERROR_NONE != ipc_ret, SERVICE_ADAPTOR_ERROR_INTERNAL, "IPC Result Failed(%d): %s", ipc_ret, ipc_msg);

	SAL_FREE(ipc_msg);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

