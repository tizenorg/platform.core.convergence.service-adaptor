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
#include "sal_ipc_client_storage.h"
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

int _get_cloud_file(GVariant *reply_info, service_storage_cloud_file_h *file)
{
	service_storage_cloud_file_h cloud_file = (service_storage_cloud_file_h) g_malloc0(sizeof(service_storage_cloud_file_s));

	int info_size = service_storage_cloud_file_res_s_type_length;
	GVariant *info[info_size];
	ipc_create_variant_info(reply_info, info_size, (GVariant ***) &info);

	int idx = 0;
	int file_info_size = service_storage_cloud_file_s_type_length;
	GVariant *file_info[file_info_size];
	ipc_create_variant_info(info[idx++], file_info_size, (GVariant ***) &file_info);

	int idx2 = 0;
	cloud_file->is_dir = g_variant_get_boolean(file_info[idx2++]);
	cloud_file->dir_path = ipc_insure_g_variant_dup_string(file_info[idx2++]);
	cloud_file->local_path = ipc_insure_g_variant_dup_string(file_info[idx2++]);
	cloud_file->cloud_path = ipc_insure_g_variant_dup_string(file_info[idx2++]);
	cloud_file->size = g_variant_get_uint64(file_info[idx2++]);
	cloud_file->operation = ipc_insure_g_variant_dup_string(file_info[idx2++]);

	ipc_destroy_variant_info(file_info, file_info_size);

	gsize files_size = g_variant_n_children(info[idx]);

	for (gsize i = 0; i < files_size; i++)
	{
		GVariant *files_struct;
		GVariant *files_entry_v = g_variant_get_child_value(info[idx], i);
		files_struct = g_variant_get_child_value(files_entry_v, 0);

		cloud_file->files = g_list_append(cloud_file->files, ipc_insure_g_variant_dup_string(files_struct));

		g_variant_unref(files_struct);
	}

	ipc_destroy_variant_info(info, info_size);

	// TODO: reorder files because it is just serialized. it makes tree structure.

	*file = cloud_file;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

//******************************************************************************
//* Public interface definition
//******************************************************************************

API int ipc_service_storage_cloud_file(const char *uri, service_storage_cloud_file_h req_file, service_storage_cloud_file_h *res_file)
{
	SAL_FN_CALL;

	RETV_IF(NULL == uri, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == req_file, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	char *request_method = DBUS_SERVICE_STORAGE_CLOUD_FILE_METHOD;
	GVariant *request_data = g_variant_new("(" service_storage_cloud_file_req_s_type ")", uri, req_file->is_dir, SAL_IPC_STR(req_file->dir_path), SAL_IPC_STR(req_file->local_path), SAL_IPC_STR(req_file->cloud_path), req_file->size, SAL_IPC_STR(req_file->operation));

	char *reply_type = service_storage_cloud_file_res_s_type;
	int reply_size = RETURN_LENGTH + 1;
	GVariant *reply = NULL;

	ret = sal_ipc_client_call_request(request_method, request_data, reply_type, &reply);
	RETVM_IF(SERVICE_ADAPTOR_ERROR_NONE != ret, ret, "ipc_client_call_request() Failed(%d)", ret);

	GVariant *reply_info[reply_size];
	ipc_create_variant_info(reply, reply_size, (GVariant ***) &reply_info);

	int idx = 0;
	service_storage_cloud_file_h cloud_file = NULL;
	_get_cloud_file(reply_info[idx++], &cloud_file);

	int ipc_ret = g_variant_get_int32(reply_info[idx++]);
	char *ipc_msg = ipc_insure_g_variant_dup_string(reply_info[idx++]);

	ipc_destroy_variant_info(reply_info, reply_size);

	g_variant_unref(reply);

	RETVM_IF(SERVICE_ADAPTOR_ERROR_NONE != ipc_ret, SERVICE_ADAPTOR_ERROR_INTERNAL, "IPC Result Failed(%d): %s", ipc_ret, ipc_msg);

	*res_file = cloud_file;

	SAL_FREE(ipc_msg);

	return SERVICE_ADAPTOR_ERROR_NONE;
}
