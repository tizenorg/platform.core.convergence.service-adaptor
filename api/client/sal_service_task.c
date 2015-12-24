/*
 * Service Task
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
#include <gio/gio.h>

#include <app.h>

#include "service_adaptor_errors.h"
#include "service_adaptor_internal.h"
#include "sal_service_adaptor.h"
#include "sal_service_adaptor_internal.h"
#include "sal_service_task.h"
#include "sal_service_task_internal.h"
#include "sal_service_auth_internal.h"
#include "sal_service_storage_internal.h"

/******************************************************************************
 * Global variables and defines
 ******************************************************************************/

GList *service_tasks = NULL;
GThreadPool *thread_pool = NULL;

/******************************************************************************
 * Private interface
 ******************************************************************************/

/******************************************************************************
 * Private interface definition
 ******************************************************************************/

static void _service_task_async_func(gpointer data, gpointer user_data)
{
	service_task_h task = (service_task_h) data;

	if (NULL != task->oauth1) {
		service_auth_oauth1_start(task->oauth1);
	} else if (NULL != task->cloud_file) {
		service_storage_cloud_start(task->cloud_file);
	} else {
		return;
	}

	/* TODO: in IPC */
	if (NULL != task->storage_progress_callback) {
		task->storage_progress_callback(100, 100, task->user_data);
	}
	if (NULL != task->storage_state_callback) {
		task->storage_state_callback(SERVICE_STORAGE_TASK_COMPLETED, task->user_data);
	}
	if (NULL != task->storage_result_callback) {
		task->storage_result_callback(SERVICE_ADAPTOR_ERROR_NONE, task->user_data);
	}
	if (NULL != task->storage_file_list_callback) {
		service_storage_cloud_file_h file_list = (service_storage_cloud_file_h) g_malloc0(sizeof(service_storage_cloud_file_s));
		file_list->is_dir = false;
		file_list->cloud_path = "/sample.txt";
		file_list->size = 1;
		task->storage_file_list_callback(SERVICE_ADAPTOR_ERROR_NONE, (service_storage_cloud_file_h) file_list, task->user_data);
	}
}

/******************************************************************************
 * Public interface definition
 ******************************************************************************/

API int service_task_connect()
{
	SAL_FN_CALL;

	thread_pool = g_thread_pool_new(_service_task_async_func, NULL, -1, FALSE, NULL);
	RETVM_IF(NULL == thread_pool, SERVICE_ADAPTOR_ERROR_SYSTEM, "g_thread_pool_new() Failed");

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_task_disconnect()
{
	SAL_FN_CALL;

	/* TODO: stop current stated task */

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_task_start(service_task_h task)
{
	SAL_FN_CALL;

	RETV_IF(NULL == task, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	service_tasks = g_list_append(service_tasks, task);

	g_thread_pool_push(thread_pool, (gpointer) task, NULL);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_task_stop(service_task_h task)
{
	SAL_FN_CALL;

	RETV_IF(NULL == task, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	if (NULL != task->cloud_file) {
		service_storage_cloud_stop(task->cloud_file);
	} else {
		return SERVICE_ADAPTOR_ERROR_NO_DATA;
	}

	service_tasks = g_list_remove(service_tasks, task);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_task_set_uri(service_task_h task, const char *uri)
{
	SAL_FN_CALL;

	RETV_IF(NULL == task, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == uri, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	task->uri = strdup(uri);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_task_get_uri(service_task_h task, char **uri)
{
	SAL_FN_CALL;

	RETV_IF(NULL == task, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == uri, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	*uri = g_strdup(task->uri);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_task_set_progress_callback(service_task_h task, service_task_progress_cb callback, void *user_data)
{
	SAL_FN_CALL;

	RETV_IF(NULL == task, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == callback, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	task->progress_callback = callback;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_task_unset_progress_callback(service_task_h task)
{
	SAL_FN_CALL;

	RETV_IF(NULL == task, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	task->progress_callback = NULL;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_task_set_state_changed_callback(service_task_h task, service_task_state_changed_cb callback, void *user_data)
{
	SAL_FN_CALL;

	RETV_IF(NULL == task, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == callback, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	task->state_callback = callback;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int  service_task_unset_state_changed_callback(service_task_h task)
{
	SAL_FN_CALL;

	RETV_IF(NULL == task, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	task->state_callback = NULL;

	return SERVICE_ADAPTOR_ERROR_NONE;
}
