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
#include "sal_service_storage.h"
/*
#include "service_adaptor_errors.h"
#include "service_adaptor_internal.h"
#include "sal_service_adaptor.h"
#include "sal_service_adaptor_internal.h"
#include "sal_service_task.h"
#include "sal_service_task_internal.h"
#include "sal_service_storage_internal.h"
#include "sal_ipc_client_storage.h"
*/
/******************************************************************************
 * Global variables and defines
 ******************************************************************************/

/******************************************************************************
 * Private interface
 ******************************************************************************/

/******************************************************************************
 * Private interface definition
 ******************************************************************************/
/*
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
*/
/******************************************************************************
 * Public interface definition
 ******************************************************************************/
/*
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
	if (NULL != src_file->files) {
		cloud_file->files = g_list_copy(src_file->files);
	} else {
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

	for (GList *list = g_list_first(file->files); list != NULL; list = list->next) {
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
*/
/******************************************************************************
 * 2.4 Public interface definition
 ******************************************************************************/
/*
API int service_storage_get_file_list(service_plugin_h plugin,
						const char *dir_path,
						service_storage_file_list_cb callback,
						void *user_data)
{
	RETV_IF(NULL == plugin, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == dir_path, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == callback, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	service_storage_cloud_file_h file = NULL;
	service_storage_cloud_file_create(plugin, &file);
	service_storage_cloud_file_set_operation(file, SERVICE_STORAGE_CLOUD_GET_FILE_LIST_URI);
	service_storage_cloud_file_set_cloud_path(file, dir_path);

	service_task_h service_task = (service_task_h) g_malloc0(sizeof(service_task_s));
	service_task->cloud_file = file;
	service_task->storage_file_list_callback = callback;
	service_task->user_data = user_data;

	return service_task_start(service_task);
}

API int service_storage_remove(service_plugin_h plugin,
						const char *remove_path,
						service_storage_result_cb callback,
						void *user_data)
{
	RETV_IF(NULL == plugin, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == remove_path, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == callback, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	service_storage_cloud_file_h file = NULL;
	service_storage_cloud_file_create(plugin, &file);
	service_storage_cloud_file_set_operation(file, SERVICE_STORAGE_CLOUD_REMOVE_FILE_URI);
	service_storage_cloud_file_set_cloud_path(file, remove_path);

	service_task_h service_task = (service_task_h) g_malloc0(sizeof(service_task_s));
	service_task->cloud_file = file;
	service_task->storage_result_callback = callback;
	service_task->user_data = user_data;

	return service_task_start(service_task);
}

API int service_storage_create_upload_task(service_plugin_h plugin,
						const char *file_path,
						const char *upload_path,
						service_storage_task_h *task)
{
	RETV_IF(NULL == plugin, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == file_path, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == upload_path, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == task, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	service_storage_cloud_file_h file = NULL;
	service_storage_cloud_file_create(plugin, &file);
	service_storage_cloud_file_set_operation(file, SERVICE_STORAGE_CLOUD_UPLOAD_FILE_URI);
	service_storage_cloud_file_set_cloud_path(file, upload_path);
	service_storage_cloud_file_set_local_path(file, file_path);

	service_task_h service_task = (service_task_h) g_malloc0(sizeof(service_task_s));
	service_task->cloud_file = file;

	*task = (service_storage_task_h) service_task;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_storage_create_download_task(service_plugin_h plugin,
						const char *storage_path,
						const char *download_path,
						service_storage_task_h *task)
{
	RETV_IF(NULL == plugin, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == storage_path, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == download_path, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == task, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	service_storage_cloud_file_h file = NULL;
	service_storage_cloud_file_create(plugin, &file);
	service_storage_cloud_file_set_operation(file, SERVICE_STORAGE_CLOUD_DOWNLOAD_FILE_URI);
	service_storage_cloud_file_set_cloud_path(file, storage_path);
	service_storage_cloud_file_set_local_path(file, download_path);

	service_task_h service_task = (service_task_h) g_malloc0(sizeof(service_task_s));
	service_task->cloud_file = file;

	*task = (service_storage_task_h) service_task;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_storage_create_download_thumbnail_task(service_plugin_h plugin,
						const char *storage_path,
						const char *download_path,
						int thumbnail_size,
						service_storage_task_h *task)
{
	RETV_IF(NULL == plugin, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == storage_path, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == download_path, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(0 > thumbnail_size, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == task, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	service_storage_cloud_file_h file = NULL;
	service_storage_cloud_file_create(plugin, &file);
	service_storage_cloud_file_set_operation(file, SERVICE_STORAGE_CLOUD_DOWNLOAD_FILE_THUMBNAIL_URI);
	service_storage_cloud_file_set_cloud_path(file, storage_path);
	service_storage_cloud_file_set_local_path(file, download_path);

	service_task_h service_task = (service_task_h) g_malloc0(sizeof(service_task_s));
	service_task->cloud_file = file;

	*task = (service_storage_task_h) service_task;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_storage_destroy_task(service_storage_task_h task)
{
	RETV_IF(NULL == task, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	return service_storage_cloud_file_destroy_task((service_task_h) task);
}

API int service_storage_start_task(service_storage_task_h task)
{
	RETV_IF(NULL == task, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	return service_task_start((service_task_h) task);
}

API int service_storage_cancel_task(service_storage_task_h task)
{
	RETV_IF(NULL == task, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

//	return service_task_stop((service_task_h) task);
	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_storage_set_task_progress_cb(service_storage_task_h task,
						service_storage_task_progress_cb callback,
						void *user_data)
{
	RETV_IF(NULL == task, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == callback, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	service_task_h service_task = (service_task_h) task;
	service_task->storage_progress_callback = callback;
	service_task->user_data = user_data;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_storage_unset_task_progress_cb(service_storage_task_h task)
{
	RETV_IF(NULL == task, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	service_task_h service_task = (service_task_h) task;
	service_task->storage_progress_callback = NULL;
	service_task->user_data = NULL;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_storage_set_task_state_changed_cb(service_storage_task_h task,
						service_storage_task_state_cb callback,
						void *user_data)
{
	RETV_IF(NULL == task, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == callback, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	service_task_h service_task = (service_task_h) task;
	service_task->storage_state_callback = callback;
	service_task->user_data = user_data;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_storage_unset_task_state_changed_cb(service_storage_task_h task)
{
	RETV_IF(NULL == task, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	service_task_h service_task = (service_task_h) task;
	service_task->storage_state_callback = NULL;
	service_task->user_data = NULL;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_storage_file_list_clone(service_storage_file_list_h src_list,
						service_storage_file_list_h *dst_list)
{
	RETV_IF(NULL == src_list, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == dst_list, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	*dst_list = src_list;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_storage_file_list_destroy(service_storage_file_list_h list)
{
	RETV_IF(NULL == list, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_storage_file_list_get_length(service_storage_file_list_h list,
						int *length)
{
	RETV_IF(NULL == list, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == length, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	service_storage_cloud_file_h file = (service_storage_cloud_file_h) list;
	*length = file->size;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_storage_file_list_foreach_file(service_storage_file_list_h list,
						service_storage_file_cb callback,
						void *user_data)
{
	RETV_IF(NULL == list, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == callback, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	service_storage_cloud_file_h file = (service_storage_cloud_file_h) list;
	callback((service_storage_file_h) file, user_data);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_storage_file_clone(service_storage_file_h src_file,
						service_storage_file_h *dst_file)
{
	RETV_IF(NULL == src_file, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == dst_file, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	*dst_file = src_file;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_storage_file_destroy(service_storage_file_h file)
{
	RETV_IF(NULL == file, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_storage_file_is_dir(service_storage_file_h file,
						bool *is_dir)
{
	RETV_IF(NULL == file, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == is_dir, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	service_storage_cloud_file_h cloud_file = (service_storage_cloud_file_h) file;
	*is_dir = cloud_file->is_dir;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_storage_file_get_size(service_storage_file_h file,
						unsigned long long *size)
{
	RETV_IF(NULL == file, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == size, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	service_storage_cloud_file_h cloud_file = (service_storage_cloud_file_h) file;
	*size = cloud_file->size;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_storage_file_get_logical_path(service_storage_file_h file,
						char **path)
{
	RETV_IF(NULL == file, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == path, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	service_storage_cloud_file_h cloud_file = (service_storage_cloud_file_h) file;
	*path = strdup(cloud_file->cloud_path);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_storage_file_get_physical_path(service_storage_file_h file,
						char **path)
{
	RETV_IF(NULL == file, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == path, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	service_storage_cloud_file_h cloud_file = (service_storage_cloud_file_h) file;
	*path = strdup(cloud_file->cloud_path);

	return SERVICE_ADAPTOR_ERROR_NONE;
}
*/
