/*
 * Service Storage
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

#include <stdio.h>
#include <string.h>
#include <glib.h>

#include <app.h>

#include "service_adaptor_errors.h"
#include "service_adaptor_internal.h"
#include "sal_service_adaptor.h"
#include "sal_service_adaptor_internal.h"
#include "sal_service_task.h"
#include "sal_service_task_internal.h"
#include "sal_service_storage.h"
#include "sal_service_storage_internal.h"
#include "sal_ipc_client_storage.h"

//******************************************************************************
//* Global variables and defines
//******************************************************************************

//******************************************************************************
//* Private interface
//******************************************************************************

//******************************************************************************
//* Private interface definition
//******************************************************************************

int service_storage_cloud_start(service_storage_cloud_file_h file)
{
	SAL_FN_CALL;

	RETV_IF(NULL == file, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == file->plugin, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	char *uri = NULL;

	ret = service_plugin_get_uri(file->plugin, &uri);
	RETVM_IF(SERVICE_ADAPTOR_ERROR_NONE != ret, ret, "service_plugin_get_uri() Failed(%d)", ret);

	service_storage_cloud_file_h cloud_file = NULL;
	ret = ipc_service_storage_cloud_file(uri, file, &cloud_file);
	RETVM_IF(SERVICE_ADAPTOR_ERROR_NONE != ret, ret, "ipc_service_storage_cloud_file() Failed(%d)", ret);

	RETV_IF(NULL == file->callback, SERVICE_ADAPTOR_ERROR_NONE);

	file->callback(SERVICE_ADAPTOR_ERROR_NONE, cloud_file,  file->user_data);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

int service_storage_cloud_stop(service_storage_cloud_file_h file)
{
	SAL_FN_CALL;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

//******************************************************************************
//* Public interface definition
//******************************************************************************

API int service_storage_cloud_file_create(service_plugin_h plugin, service_storage_cloud_file_h *file)
{
	SAL_FN_CALL;

	RETV_IF(NULL == plugin, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == file, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	service_storage_cloud_file_h cloud_file = (service_storage_cloud_file_h) g_malloc0(sizeof(service_storage_cloud_file_s));
	cloud_file->plugin = plugin;
	cloud_file->callback = NULL;
	cloud_file->is_dir = false;
	cloud_file->size = 0;
	cloud_file->dir_path = NULL;
	cloud_file->local_path = NULL;
	cloud_file->cloud_path = NULL;
	cloud_file->operation = NULL;
	cloud_file->files = NULL;

	*file = cloud_file;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_storage_cloud_file_clone(service_storage_cloud_file_h src_file, service_storage_cloud_file_h *dst_file)
{
	SAL_FN_CALL;

	RETV_IF(NULL == src_file, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == dst_file, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	service_storage_cloud_file_h cloud_file = (service_storage_cloud_file_h) g_malloc0(sizeof(service_storage_cloud_file_s));
	cloud_file->plugin = src_file->plugin;
	cloud_file->callback = src_file->callback;
	cloud_file->is_dir = src_file->is_dir;
	cloud_file->size = src_file->size;
	cloud_file->dir_path = g_strdup(src_file->dir_path);
	cloud_file->local_path = g_strdup(src_file->local_path);
	cloud_file->cloud_path = g_strdup(src_file->cloud_path);
	cloud_file->operation = g_strdup(src_file->operation);
	// TODO: g_list_copy_deep()
	if (NULL != src_file->files)
	{
		cloud_file->files = g_list_copy(src_file->files);
	}
	else
	{
		cloud_file->files = NULL;
	}

	*dst_file = cloud_file;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_storage_cloud_file_destroy(service_storage_cloud_file_h file)
{
	SAL_FN_CALL;

	RETV_IF(NULL == file, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	SAL_FREE(file->dir_path);
	SAL_FREE(file->local_path);
	SAL_FREE(file->cloud_path);
	SAL_FREE(file->operation);
	SAL_FREE(file);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_storage_cloud_file_set_callback(service_storage_cloud_file_h file, service_storage_cloud_file_cb callback, void *user_data)
{
	SAL_FN_CALL;

	RETV_IF(NULL == file, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == callback, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	file->callback = callback;
	file->user_data = user_data;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_storage_cloud_file_unset_callback(service_storage_cloud_file_h file)
{
	SAL_FN_CALL;

	RETV_IF(NULL == file, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	file->callback = NULL;
	file->user_data = NULL;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_storage_cloud_file_set_cloud_path(service_storage_cloud_file_h file, const char *cloud_path)
{
	SAL_FN_CALL;

	RETV_IF(NULL == file, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cloud_path, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	file->cloud_path = strdup(cloud_path);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_storage_cloud_file_get_cloud_path(service_storage_cloud_file_h file, char **cloud_path)
{
	SAL_FN_CALL;

	RETV_IF(NULL == file, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cloud_path, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	*cloud_path = g_strdup(file->cloud_path);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_storage_cloud_file_set_local_path(service_storage_cloud_file_h file, const char *local_path)
{
	SAL_FN_CALL;

	RETV_IF(NULL == file, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == local_path, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	file->local_path = strdup(local_path);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_storage_cloud_file_get_local_path(service_storage_cloud_file_h file, char **local_path)
{
	SAL_FN_CALL;

	RETV_IF(NULL == file, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == local_path, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	*local_path = g_strdup(file->local_path);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_storage_cloud_file_set_size(service_storage_cloud_file_h file, unsigned long long size)
{
	SAL_FN_CALL;

	RETV_IF(NULL == file, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	file->size = size;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_storage_cloud_file_get_size(service_storage_cloud_file_h file, unsigned long long *size)
{
	SAL_FN_CALL;

	RETV_IF(NULL == file, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == size, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	*size = file->size;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_storage_cloud_file_set_operation(service_storage_cloud_file_h file, const char *operation)
{
	SAL_FN_CALL;

	RETV_IF(NULL == file, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == operation, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	file->operation = strdup(operation);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_storage_cloud_file_get_operation(service_storage_cloud_file_h file, char **operation)
{
	SAL_FN_CALL;

	RETV_IF(NULL == file, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == operation, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	*operation = g_strdup(file->operation);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_storage_cloud_file_is_directory(service_storage_cloud_file_h file, bool *is_dir)
{
	SAL_FN_CALL;

	RETV_IF(NULL == file, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == is_dir, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	*is_dir = file->is_dir;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_storage_cloud_file_foreach_file(service_storage_cloud_file_h file, service_storage_cloud_file_cb callback, void *user_data)
{
	SAL_FN_CALL;

	RETV_IF(NULL == file, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == callback, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	RETV_IF(false == file->is_dir, SERVICE_ADAPTOR_ERROR_NO_DATA);

	RETV_IF(0 == g_list_length(file->files), SERVICE_ADAPTOR_ERROR_NO_DATA);

	for (GList *list = g_list_first(file->files); list != NULL; list = list->next)
	{
		service_storage_cloud_file_h file_data = (service_storage_cloud_file_h) list->data;

		bool ret = callback(SERVICE_ADAPTOR_ERROR_NONE, file_data, user_data);
		RETV_IF(false == ret, SERVICE_ADAPTOR_ERROR_NONE);
	}

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_storage_cloud_file_create_task(service_storage_cloud_file_h file, service_task_h *task)
{
	SAL_FN_CALL;

	RETV_IF(NULL == file, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == task, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	service_task_h service_task = (service_task_h) g_malloc0(sizeof(service_task_s));
	service_task->cloud_file = file;

	*task = service_task;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_storage_cloud_file_destroy_task(service_task_h task)
{
	SAL_FN_CALL;

	RETV_IF(NULL == task, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	SAL_FREE(task);

	return SERVICE_ADAPTOR_ERROR_NONE;
}
