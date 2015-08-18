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
#include "sal_ipc_server_storage.h"
#include "storage_adaptor.h"
#include "sal_service_storage.h"
#include "sal_service_storage_internal.h"

//******************************************************************************
//* Global variables and defines
//******************************************************************************

//******************************************************************************
//* Private interface
//******************************************************************************

//******************************************************************************
//* Private interface definition
//******************************************************************************

void _cloud_remove_file_cb(int result, cloud_file_h file, void *user_data)
{
	SAL_FN_CALL;

	ipc_reply_data_h reply = (ipc_reply_data_h) user_data;

	int ipc_ret = SERVICE_ADAPTOR_ERROR_NONE;
	char *ipc_msg = NULL;
	GVariant *ipc_data = NULL;

	GVariantBuilder *builder = g_variant_builder_new(G_VARIANT_TYPE(file_list_type));

	ipc_create_error_msg(ipc_ret, &ipc_msg);
	ipc_data = g_variant_new(ipc_make_return_type(reply->type), file->is_dir, SAL_IPC_STR(file->dir_path), SAL_IPC_STR(file->local_path), SAL_IPC_STR(file->cloud_path), file->size, SAL_IPC_STR(file->operation), builder, ipc_ret, SAL_IPC_STR(ipc_msg));
	g_dbus_method_invocation_return_value(reply->invocation, ipc_data);

	SAL_FREE(ipc_msg);
	ipc_free_reply_data(reply);
	g_variant_builder_unref(builder);

	SAL_FN_END;
}

int _get_cloud_file(GVariant *reply_info, service_storage_cloud_file_h *file)
{
	SAL_FN_CALL;

	service_storage_cloud_file_h cloud_file = (service_storage_cloud_file_h) g_malloc0(sizeof(service_storage_cloud_file_s));

	int info_size = service_storage_cloud_file_s_type_length;
	GVariant *info[info_size];
	ipc_create_variant_info(reply_info, info_size, (GVariant ***) &info);

	int idx = 0;
	cloud_file->is_dir = g_variant_get_boolean(info[idx++]);
	cloud_file->dir_path = ipc_insure_g_variant_dup_string(info[idx++]);
	cloud_file->local_path = ipc_insure_g_variant_dup_string(info[idx++]);
	cloud_file->cloud_path = ipc_insure_g_variant_dup_string(info[idx++]);
	cloud_file->size = g_variant_get_uint64(info[idx++]);
	cloud_file->operation = ipc_insure_g_variant_dup_string(info[idx++]);

	ipc_destroy_variant_info(info, info_size);

	*file = cloud_file;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

int _cloud_execute_operation(storage_plugin_h plugin, service_storage_cloud_file_h file, ipc_reply_data_h reply)
{
	SAL_FN_CALL;

	RETV_IF(NULL == plugin, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == plugin->cloud, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == file, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	if (0 == strcmp(file->operation, SERVICE_STORAGE_CLOUD_REMOVE_FILE_URI))
	{
		ret = plugin->cloud->cloud_remove_file(plugin, file->cloud_path, _cloud_remove_file_cb, reply);

		return ret;
	}
	else if (0 == strcmp(file->operation, SERVICE_STORAGE_CLOUD_DOWNLOAD_FILE_URI))
	{
		ret = plugin->cloud->cloud_remove_file(plugin, file->cloud_path, _cloud_remove_file_cb, reply);

		return ret;
	}
	else if (0 == strcmp(file->operation, SERVICE_STORAGE_CLOUD_UPLOAD_FILE_URI))
	{
		ret = plugin->cloud->cloud_remove_file(plugin, file->cloud_path, _cloud_remove_file_cb, reply);

		return ret;
	}
	else if (0 == strcmp(file->operation, SERVICE_STORAGE_CLOUD_DOWNLOAD_FILE_THUMBNAIL_URI))
	{
		ret = plugin->cloud->cloud_remove_file(plugin, file->cloud_path, _cloud_remove_file_cb, reply);

		return ret;
	}
	else if (0 == strcmp(file->operation, SERVICE_STORAGE_CLOUD_GET_FILE_LIST_URI))
	{
		ret = plugin->cloud->cloud_remove_file(plugin, file->cloud_path, _cloud_remove_file_cb, reply);

		return ret;
	}

	return SERVICE_ADAPTOR_ERROR_INTERNAL;
}

//******************************************************************************
//* Public interface definition
//******************************************************************************

API void service_storage_method_call(GDBusConnection *connection,
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

	if (0 == g_strcmp0(method_name, DBUS_SERVICE_STORAGE_CLOUD_FILE_METHOD))
	{
		int idx = 0;
		int size = service_storage_cloud_file_req_s_type_length;
		GVariant *req_info[size];

		ipc_create_variant_info(in_parameters, size, (GVariant ***) &req_info);

		char *uri = ipc_insure_g_variant_dup_string(req_info[idx++]);
		service_storage_cloud_file_h file = NULL;
		_get_cloud_file(req_info[idx++], &file);

		SAL_INFO("uri: %s", uri);

		ipc_ret = SERVICE_ADAPTOR_ERROR_INTERNAL;
		ipc_type = strdup(service_storage_cloud_file_res_s_type);

		sal_h sal = sal_get_handle();
		TRYVM_IF(NULL == sal, ipc_ret = SERVICE_ADAPTOR_ERROR_INTERNAL, "sal_get_handle() Failed");

		storage_plugin_h plugin = storage_adaptor_get_plugin(sal->storage, uri);

		ipc_reply_data_h reply = (ipc_reply_data_h) g_malloc0(sizeof(ipc_reply_data_s));
		reply->invocation = invocation;
		reply->type = strdup(ipc_type);

		ipc_ret = _cloud_execute_operation(plugin, file, reply);
		TRY_IF(SERVICE_ADAPTOR_ERROR_NONE == ipc_ret, "cloud_execute_operation() Request Successed");

		GVariantBuilder *builder = g_variant_builder_new(G_VARIANT_TYPE(file_list_type));
		ipc_create_error_msg(ipc_ret, &ipc_msg);
		ipc_data = g_variant_new(ipc_make_return_type(ipc_type), false, "", "", 0, 0, builder, ipc_ret, SAL_IPC_STR(ipc_msg));

		g_variant_builder_unref(builder);
		ipc_destroy_variant_info(req_info, size);
	}

	g_dbus_method_invocation_return_value(invocation, ipc_data);

catch:
	SAL_FREE(uri);
	SAL_FREE(ipc_msg);
	SAL_FREE(ipc_type);

	SAL_FN_END;
}
