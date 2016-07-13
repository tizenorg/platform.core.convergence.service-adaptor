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
 * File: service-adaptor-client-storage.c
 * Desc:
 * Created on: Oct, 2014
 * Auth: Yongjin Kim <youth.kim@samsung.com>
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "service_adaptor_client_private.h"
#include "service_adaptor_client_type.h"
#include "service_adaptor_client_log.h"
#include "service_adaptor_client_storage.h"
#include "service_adaptor_client_storage_internal.h"
#include "dbus_client.h"
#include "dbus_client_storage.h"

#include "util/service_adaptor_client_util.h"

/****************** private feature */
#include "private/service-adaptor-client-storage.h"
/****************** private feature */

#define SAFE_STRDUP(src) (src) ? strdup(src) : NULL

#define ASYNC_OPERATION_UPLOAD_FILE 1
#define ASYNC_OPERATION_DOWNLOAD_FILE 2
#define ASYNC_OPERATION_DOWNLOAD_THUMBNAIL 3

#define TIZEN_PRIVILEGE_NAME_INTERNET	"http://tizen.org/privilege/internet"

struct _service_storage_file_list_s {
	service_storage_file_h *list;
	int length;
};

/**
* @brief Release memory for service_storage_file_h
*
* @param[in]    void
* @return service_storage_file_h
* @retval Allocated and filled default value file_info's pointer
*/
service_storage_file_h service_storage_create_file_info(void)
{
	FUNC_START();
	service_storage_file_h _file_info = NULL;
	_file_info = (service_storage_file_h) calloc(1, sizeof(struct _service_storage_file_s));

	service_storage_media_meta_s *_media_meta = NULL;
	_media_meta = (service_storage_media_meta_s *) calloc(1, sizeof(service_storage_media_meta_s));

	service_storage_cloud_meta_s *_cloud_meta = NULL;
	_cloud_meta = (service_storage_cloud_meta_s *) calloc(1, sizeof(service_storage_cloud_meta_s));

	if ((NULL == _file_info) || (NULL == _media_meta) || (NULL == _cloud_meta)) {
		free(_file_info); /* LCOV_EXCL_LINE */
		free(_media_meta); /* LCOV_EXCL_LINE */
		free(_cloud_meta); /* LCOV_EXCL_LINE */

		FUNC_STOP(); /* LCOV_EXCL_LINE */
		return NULL; /* LCOV_EXCL_LINE */
	}

	_media_meta->mime_type		= NULL;
	_media_meta->title		= NULL;
	_media_meta->album		= NULL;
	_media_meta->artist		= NULL;
	_media_meta->genere		= NULL;
	_media_meta->recorded_date	= NULL;
	_media_meta->width		= -1;
	_media_meta->height		= -1;
	_media_meta->duration		= -1;
	_media_meta->copyright		= NULL;
	_media_meta->track_num		= NULL;
	_media_meta->description	= NULL;
	_media_meta->composer		= NULL;
	_media_meta->year		= NULL;
	_media_meta->bitrate		= -1;
	_media_meta->samplerate		= -1;
	_media_meta->channel		= -1;
	_media_meta->extra_media_meta	= NULL;

	_cloud_meta->service_name	= NULL;
	_cloud_meta->usage_byte		= 0ULL;
	_cloud_meta->quota_byte		= 0ULL;
	_cloud_meta->extra_cloud_meta	= NULL;

	_file_info->plugin_name         = NULL;
	_file_info->object_id           = NULL;
	_file_info->storage_path        = NULL;
	_file_info->file_size           = 0ULL;
	_file_info->created_time        = 0ULL;
	_file_info->modified_time       = 0ULL;
	_file_info->file_info_index     = -1;
	_file_info->content_type        = SERVICE_ADAPTOR_STORAGE_CONTENT_TYPE_DEFAULT;
	_file_info->media_meta          = _media_meta;
	_file_info->cloud_meta          = _cloud_meta;
	_file_info->extra_file_info	= NULL;

	FUNC_END();
	return _file_info;
}

/**
* @brief Release memory for service_storage_file_h
*
* @param[in]    file_info       specifies Service Adaptor file information handle
* @return service_storage_file_h
* @retval released file_info's pointer
*/
int service_storage_unref_file_info(service_storage_file_h *file_info)
{
	FUNC_START();
	if (NULL == file_info) {
		FUNC_STOP(); /* LCOV_EXCL_LINE */
		return 1; /* LCOV_EXCL_LINE */
	}

	if (NULL == *file_info) {
		FUNC_STOP(); /* LCOV_EXCL_LINE */
		return 0; /* LCOV_EXCL_LINE */
	}
	service_storage_file_h _file_info = *file_info;

	free(_file_info->plugin_name);
	free(_file_info->object_id);
	free(_file_info->storage_path);
	free(_file_info->extra_file_info);

	service_storage_media_meta_s *_media_meta = _file_info->media_meta;

	if (NULL != _media_meta) {
		free(_media_meta->mime_type);
		free(_media_meta->title);
		free(_media_meta->album);
		free(_media_meta->artist);
		free(_media_meta->genere);
		free(_media_meta->recorded_date);
		free(_media_meta->copyright);
		free(_media_meta->track_num);
		free(_media_meta->description);
		free(_media_meta->composer);
		free(_media_meta->year);
		free(_media_meta->extra_media_meta);
	}

	service_storage_cloud_meta_s *_cloud_meta = _file_info->cloud_meta;

	if (NULL != _cloud_meta) {
		free(_cloud_meta->service_name);
		free(_cloud_meta->extra_cloud_meta);
	}

	free((*file_info)->media_meta);
	free((*file_info)->cloud_meta);
	free(*file_info);
	*file_info = NULL;

	FUNC_END();
	return 0;
}

/******* Async implementation */

int service_storage_create_upload_task(service_plugin_h plugin,
						const char *file_path,
						const char *upload_path,
						service_storage_task_h *task)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;

	if ((NULL == plugin) || (NULL == file_path) || (NULL == upload_path) || (NULL == task)) {
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	if (NULL == plugin->service_handle_name) {
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_INVALID_STATE;
	}

	if (CLIENT_APP_TYPE_APPLICATION == plugin->app_type) {
		int privilege_ret = 0;
		privilege_ret = _dbus_get_privilege_check_result(plugin->service_handle_name, TIZEN_PRIVILEGE_NAME_INTERNET, NULL, &error);
		if (SERVICE_ADAPTOR_ERROR_NONE != privilege_ret) {
			sac_error("Privilege check error (ret : %d)", privilege_ret); /* LCOV_EXCL_LINE */
			return SERVICE_ADAPTOR_ERROR_PERMISSION_DENIED; /* LCOV_EXCL_LINE */
		}
	}

	service_storage_task_h _task = (service_storage_task_h) calloc(1, sizeof(service_storage_task_t));
	if (NULL == _task) {
		ret = SERVICE_ADAPTOR_ERROR_UNKNOWN; /* LCOV_EXCL_LINE */
		service_adaptor_set_last_result(ret, "Memory allocation failed"); /* LCOV_EXCL_LINE */
		return ret; /* LCOV_EXCL_LINE */
	}

	long long int task_id = 0;
	ret = _dbus_open_upload_file(plugin->service_handle_name, file_path, upload_path, &task_id, &error);

	if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
		service_adaptor_set_last_result(error.code, error.msg); /* LCOV_EXCL_LINE */
		free(error.msg); /* LCOV_EXCL_LINE */
		free(_task); /* LCOV_EXCL_LINE */
	} else {
		_task->service_handle_name = strdup(plugin->service_handle_name);
		_task->task_id = task_id;
		_task->operation = ASYNC_OPERATION_UPLOAD_FILE;
		_task->state_callback = NULL;
		_task->state_user_data = NULL;
		_task->progress_callback = NULL;
		_task->progress_user_data = NULL;

		_task->param1 = (void *)strdup(file_path);
		_task->param2 = (void *)strdup(upload_path);
		_task->param3 = NULL;

		*task = _task;

		service_adaptor_task_h callback_task = NULL;
		while ((callback_task = _queue_get_task((int64_t)_task->task_id))) {
			_queue_del_task(callback_task);
			callback_task = NULL;
		}
		_queue_add_task((int64_t)_task->task_id,
				(uint32_t) NULL, (void *) _task, NULL);
	}

	sac_api_end(ret);
	return ret;
}


int service_storage_create_download_task(service_plugin_h plugin,
						const char *storage_path,
						const char *download_path,
						service_storage_task_h *task)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;

	if ((NULL == plugin) || (NULL == storage_path) || (NULL == download_path) || (NULL == task)) {
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	if (NULL == plugin->service_handle_name) {
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_INVALID_STATE;
	}

	if (CLIENT_APP_TYPE_APPLICATION == plugin->app_type) {
		int privilege_ret = 0;
		privilege_ret = _dbus_get_privilege_check_result(plugin->service_handle_name, TIZEN_PRIVILEGE_NAME_INTERNET, NULL, &error);
		if (SERVICE_ADAPTOR_ERROR_NONE != privilege_ret) {
			sac_error("Privilege check error (ret : %d)", privilege_ret); /* LCOV_EXCL_LINE */
			return SERVICE_ADAPTOR_ERROR_PERMISSION_DENIED; /* LCOV_EXCL_LINE */
		}
	}

	service_storage_task_h _task = (service_storage_task_h) calloc(1, sizeof(service_storage_task_t));
	if (NULL == _task) {
		ret = SERVICE_ADAPTOR_ERROR_UNKNOWN; /* LCOV_EXCL_LINE */
		service_adaptor_set_last_result(ret, "Memory allocation failed"); /* LCOV_EXCL_LINE */
		return ret; /* LCOV_EXCL_LINE */
	}

	long long int task_id = 0;
	ret = _dbus_open_download_file(plugin->service_handle_name, storage_path, download_path, &task_id, &error);

	if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
		service_adaptor_set_last_result(error.code, error.msg); /* LCOV_EXCL_LINE */
		free(error.msg); /* LCOV_EXCL_LINE */
		free(_task); /* LCOV_EXCL_LINE */
	} else {
		_task->service_handle_name = strdup(plugin->service_handle_name);
		_task->task_id = task_id;
		_task->operation = ASYNC_OPERATION_DOWNLOAD_FILE;
		_task->state_callback = NULL;
		_task->state_user_data = NULL;
		_task->progress_callback = NULL;
		_task->progress_user_data = NULL;

		_task->param1 = (void *)strdup(storage_path);
		_task->param2 = (void *)strdup(download_path);
		_task->param3 = NULL;

		*task = _task;

		service_adaptor_task_h callback_task = NULL;
		while ((callback_task = _queue_get_task((int64_t)_task->task_id))) {
			_queue_del_task(callback_task);
			callback_task = NULL;
		}
		_queue_add_task((int64_t)_task->task_id,
				(uint32_t) NULL, (void *) _task, NULL);
	}
	sac_api_end(ret);
	return ret;
}

int service_storage_create_download_thumbnail_task(service_plugin_h plugin,
						const char *storage_path,
						const char *download_path,
						int thumbnail_size,
						service_storage_task_h *task)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;

	if ((NULL == plugin) || (NULL == storage_path) || (NULL == download_path) || (NULL == task)) {
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	if (NULL == plugin->service_handle_name) {
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_INVALID_STATE;
	}

	if (CLIENT_APP_TYPE_APPLICATION == plugin->app_type) {
		int privilege_ret = 0;
		privilege_ret = _dbus_get_privilege_check_result(plugin->service_handle_name, TIZEN_PRIVILEGE_NAME_INTERNET, NULL, &error);
		if (SERVICE_ADAPTOR_ERROR_NONE != privilege_ret) {
			sac_error("Privilege check error (ret : %d)", privilege_ret); /* LCOV_EXCL_LINE */
			return SERVICE_ADAPTOR_ERROR_PERMISSION_DENIED; /* LCOV_EXCL_LINE */
		}
	}

	service_storage_task_h _task = (service_storage_task_h) calloc(1, sizeof(service_storage_task_t));
	int *t_size = (int *)calloc(1, sizeof(int));
	if ((NULL == _task) || (NULL == t_size)) {
		ret = SERVICE_ADAPTOR_ERROR_UNKNOWN; /* LCOV_EXCL_LINE */
		service_adaptor_set_last_result(ret, "Memory allocation failed"); /* LCOV_EXCL_LINE */
		free(_task); /* LCOV_EXCL_LINE */
		free(t_size); /* LCOV_EXCL_LINE */
		return ret; /* LCOV_EXCL_LINE */
	}

	long long int task_id = 0;
	ret = _dbus_open_download_thumbnail(plugin->service_handle_name, storage_path, download_path, thumbnail_size, &task_id, &error);

	if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
		service_adaptor_set_last_result(error.code, error.msg); /* LCOV_EXCL_LINE */
		free(error.msg); /* LCOV_EXCL_LINE */
		free(_task); /* LCOV_EXCL_LINE */
		free(t_size); /* LCOV_EXCL_LINE */
	} else {
		_task->service_handle_name = strdup(plugin->service_handle_name);
		_task->task_id = task_id;
		_task->operation = ASYNC_OPERATION_DOWNLOAD_THUMBNAIL;
		_task->state_callback = NULL;
		_task->state_user_data = NULL;
		_task->progress_callback = NULL;
		_task->progress_user_data = NULL;

		_task->param1 = (void *)strdup(storage_path);
		_task->param2 = (void *)strdup(download_path);
		*t_size = thumbnail_size;
		_task->param3 = (void *)t_size;

		*task = _task;

		service_adaptor_task_h callback_task = NULL;
		while ((callback_task = _queue_get_task((int64_t)_task->task_id))) {
			_queue_del_task(callback_task);
			callback_task = NULL;
		}
		_queue_add_task((int64_t)_task->task_id,
				(uint32_t) NULL, (void *) _task, NULL);

	}
	sac_api_end(ret);
	return ret;
}

int service_storage_destroy_task(service_storage_task_h task)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;

	if (NULL == task) {
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	ret = _dbus_close_file_task(task->service_handle_name, task->task_id, &error);

	if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
		service_adaptor_set_last_result(error.code, error.msg); /* LCOV_EXCL_LINE */
		free(error.msg); /* LCOV_EXCL_LINE */
	}

	service_adaptor_task_h callback_task = NULL;
	while ((callback_task = _queue_get_task((int64_t)task->task_id))) {
		_queue_del_task(callback_task);
		callback_task = NULL;
	}

	free(task->service_handle_name);
	free(task->param1);
	free(task->param2);
	free(task->param3);
	free(task);

	sac_api_end(ret);
	return ret;
}

int service_storage_start_task(service_storage_task_h task)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;

	if (NULL == task) {
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	switch (task->operation) {
	case ASYNC_OPERATION_UPLOAD_FILE:
		sac_info("Start upload async task");
		ret = _dbus_start_upload_file(task->service_handle_name,
				task->task_id, (char *)task->param2,
				task->progress_callback ? true : false, task->state_callback ? true : false, &error);
	break;
	case ASYNC_OPERATION_DOWNLOAD_FILE:
		sac_info("Start download async task");
		ret = _dbus_start_download_file(task->service_handle_name,
				task->task_id, (char *)task->param1,
				task->progress_callback ? true : false, task->state_callback ? true : false, &error);
	break;

	case ASYNC_OPERATION_DOWNLOAD_THUMBNAIL:
		sac_info("Start download thumbnail async task");
		ret = _dbus_start_download_thumbnail(task->service_handle_name,
				task->task_id, (char *)task->param1, *((int *)task->param3),
				task->progress_callback ? true : false, task->state_callback ? true : false, &error);
	break;

	default:
		sac_info("Invalid async task"); /* LCOV_EXCL_LINE */
		ret = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER; /* LCOV_EXCL_LINE */
		error.code = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER; /* LCOV_EXCL_LINE */
		error.msg = strdup("Invalid async task operation"); /* LCOV_EXCL_LINE */
		break; /* LCOV_EXCL_LINE */
	}

	if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
		service_adaptor_set_last_result(error.code, error.msg); /* LCOV_EXCL_LINE */
		free(error.msg); /* LCOV_EXCL_LINE */
	}

	sac_api_end(ret);
	return ret;
}

int service_storage_cancel_task(service_storage_task_h task)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;

	if (NULL == task) {
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	switch (task->operation) {
	case ASYNC_OPERATION_UPLOAD_FILE:
		sac_info("Start upload async task");
		ret = _dbus_cancel_upload_file(task->service_handle_name,
				task->task_id, &error);
	break;
	case ASYNC_OPERATION_DOWNLOAD_FILE:
		sac_info("Start download async task");
		ret = _dbus_cancel_download_file(task->service_handle_name,
				task->task_id, &error);
	break;

	case ASYNC_OPERATION_DOWNLOAD_THUMBNAIL:
		sac_info("Start download thumbnail async task");
		ret = _dbus_cancel_download_thumbnail(task->service_handle_name,
				task->task_id, &error);
	break;

	default:
		sac_info("Invalid async task"); /* LCOV_EXCL_LINE */
		ret = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER; /* LCOV_EXCL_LINE */
		error.code = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER; /* LCOV_EXCL_LINE */
		error.msg = strdup("Invalid async task operation"); /* LCOV_EXCL_LINE */
		break; /* LCOV_EXCL_LINE */
	}

	if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
		service_adaptor_set_last_result(error.code, error.msg); /* LCOV_EXCL_LINE */
		free(error.msg); /* LCOV_EXCL_LINE */
	}

	sac_api_end(ret);
	return ret;

}

int service_storage_set_task_progress_cb(service_storage_task_h task,
						service_storage_task_progress_cb callback,
						void *user_data)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	if ((NULL == task) || (NULL == callback)) {
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	task->progress_callback = callback;
	task->progress_user_data = user_data;

	sac_api_end(ret);
	return ret;
}

int service_storage_unset_task_progress_cb(service_storage_task_h task)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	if (NULL == task) {
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	task->progress_callback = NULL;
	task->progress_user_data = NULL;

	sac_api_end(ret);
	return ret;
}

int service_storage_set_task_state_changed_cb(service_storage_task_h task,
						service_storage_task_state_cb callback,
						void *user_data)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	if ((NULL == task) || (NULL == callback)) {
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	task->state_callback = callback;
	task->state_user_data = user_data;

	sac_api_end(ret);
	return ret;
}

int service_storage_unset_task_state_changed_cb(service_storage_task_h task)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	if (NULL == task) {
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	task->state_callback = NULL;
	task->state_user_data = NULL;

	sac_api_end(ret);
	return ret;
}

struct __async_wrapper_context {
	service_plugin_h plugin;
	char *path;
	void *callback;
	void *user_data;
};

void *_get_file_list_runnable(void *_data)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	struct __async_wrapper_context *_params = (struct __async_wrapper_context *)_data;

	service_storage_file_list_h file_list = NULL;

	service_adaptor_error_s error;
	error.msg = NULL;
	service_storage_file_h *files = NULL;
	unsigned int files_len = 0;

	ret = _dbus_get_file_list(_params->plugin->service_handle_name, _params->path, &files, &files_len, NULL, &error);

	if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
		service_adaptor_set_last_result(error.code, error.msg); /* LCOV_EXCL_LINE */
		free(error.msg); /* LCOV_EXCL_LINE */
		if ((NULL != files) && (0 < files_len)) { /* LCOV_EXCL_LINE */
			int i; /* LCOV_EXCL_LINE */
			for (i = 0; i < files_len; i++) { /* LCOV_EXCL_LINE */
				service_storage_unref_file_info(&files[i]); /* LCOV_EXCL_LINE */
			} /* LCOV_EXCL_LINE */
			free(files); /* LCOV_EXCL_LINE */
			files = NULL; /* LCOV_EXCL_LINE */
		}
	} else {
		if (0 < files_len) {
			file_list = (service_storage_file_list_h) calloc(1, sizeof(struct _service_storage_file_list_s));
			if (NULL == file_list) {
				sac_error("Critical : Memory allocation failed"); /* LCOV_EXCL_LINE */
				ret = SERVICE_ADAPTOR_ERROR_UNKNOWN;
			} else {
				file_list->list = files;
				file_list->length = (int) files_len;
			}
		} else {
			ret = SERVICE_ADAPTOR_ERROR_NO_DATA;
		}
	}

	((service_storage_file_list_cb)_params->callback)(ret, file_list, _params->user_data);

	int i;
	for (i = 0; i < files_len; i++) {
		service_storage_unref_file_info(&files[i]);
	}
	free(files);
	free(file_list);

	free(_params->path);
	free(_params);

	sac_api_end(ret);
	return NULL;
}

int service_storage_get_file_list(service_plugin_h plugin,
						const char *dir_path,
						service_storage_file_list_cb callback,
						void *user_data)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	if ((NULL == plugin) || (NULL == callback) || (NULL == dir_path)) {
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	if (NULL == plugin->service_handle_name) {
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_INVALID_STATE;
	}

	if (CLIENT_APP_TYPE_APPLICATION == plugin->app_type) {
		int privilege_ret = 0;
		service_adaptor_error_s error;
		error.msg = NULL;
		privilege_ret = _dbus_get_privilege_check_result(plugin->service_handle_name, TIZEN_PRIVILEGE_NAME_INTERNET, NULL, &error);
		if (SERVICE_ADAPTOR_ERROR_NONE != privilege_ret) {
			sac_error("Privilege check error (ret : %d)", privilege_ret); /* LCOV_EXCL_LINE */
			return SERVICE_ADAPTOR_ERROR_PERMISSION_DENIED; /* LCOV_EXCL_LINE */
		}
	}

	struct __async_wrapper_context *params = NULL;
	params = (struct __async_wrapper_context *) calloc(1, sizeof(struct __async_wrapper_context));

	if (NULL == params) {
		sac_warning("Memory allocation failed"); /* LCOV_EXCL_LINE */
		ret = SERVICE_ADAPTOR_ERROR_UNKNOWN; /* LCOV_EXCL_LINE */
	} else {
		params->plugin = plugin;
		params->path = strdup(dir_path);
		params->callback = (void *)callback;
		params->user_data = user_data;

		int thread_ret = 0;
		pthread_t get_list_thread;
		thread_ret = pthread_create(&get_list_thread, NULL, _get_file_list_runnable, (void *)params);

		if (thread_ret) {
			sac_warning("Thread create failed"); /* LCOV_EXCL_LINE */
			ret = SERVICE_ADAPTOR_ERROR_UNKNOWN; /* LCOV_EXCL_LINE */
			free(params->path); /* LCOV_EXCL_LINE */
			free(params); /* LCOV_EXCL_LINE */
		}
	}

	return ret;
}

int service_storage_file_list_clone(service_storage_file_list_h src_list,
						service_storage_file_list_h *dst_list)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	if ((NULL == src_list) || (NULL == dst_list)) {
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	service_storage_file_list_h _list = NULL;
	_list = (service_storage_file_list_h) calloc(1, sizeof(struct _service_storage_file_list_s));
	if (NULL == _list) {
		FUNC_STOP(); /* LCOV_EXCL_LINE */
		return SERVICE_ADAPTOR_ERROR_UNKNOWN; /* LCOV_EXCL_LINE */
	}
	_list->length = src_list->length;
	_list->list = (service_storage_file_h *) calloc(_list->length, sizeof(struct _service_storage_file_s *));
	if (NULL == _list->list) {
		free(_list); /* LCOV_EXCL_LINE */
		FUNC_STOP(); /* LCOV_EXCL_LINE */
		return SERVICE_ADAPTOR_ERROR_UNKNOWN; /* LCOV_EXCL_LINE */
	}

	int i = 0;
	for (i = 0; i < _list->length; i++) {
		service_storage_file_h file = NULL;
		service_storage_file_clone(src_list->list[i], &file);
		_list->list[i] = file;
	}

	*dst_list = _list;

	sac_api_end(ret);
	return ret;
}

int service_storage_file_list_destroy(service_storage_file_list_h list)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	if (NULL == list) {
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	int i;
	for (i = 0; i < list->length; i++) {
		service_storage_file_destroy(list->list[i]);
	}
	free(list);

	sac_api_end(ret);
	return ret;
}

int service_storage_file_clone(service_storage_file_h src_file,
						service_storage_file_h *dst_file)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	if ((NULL == src_file) || (NULL == dst_file)) {
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	service_storage_file_h src = src_file;
	service_storage_file_h new_file = service_storage_create_file_info();

	if (NULL == new_file) {
		FUNC_STOP(); /* LCOV_EXCL_LINE */
		return SERVICE_ADAPTOR_ERROR_UNKNOWN; /* LCOV_EXCL_LINE */
	}

	if (NULL != src->media_meta) {
		new_file->media_meta->mime_type	=
			SAFE_STRDUP(src->media_meta->mime_type);
		new_file->media_meta->title =
			SAFE_STRDUP(src->media_meta->title);
		new_file->media_meta->album =
			SAFE_STRDUP(src->media_meta->album);
		new_file->media_meta->artist =
			SAFE_STRDUP(src->media_meta->artist);
		new_file->media_meta->genere =
			SAFE_STRDUP(src->media_meta->genere);
		new_file->media_meta->recorded_date =
			SAFE_STRDUP(src->media_meta->recorded_date);
		new_file->media_meta->width =
			src->media_meta->width;
		new_file->media_meta->height =
			src->media_meta->height;
		new_file->media_meta->duration =
			src->media_meta->duration;
		new_file->media_meta->copyright =
			SAFE_STRDUP(src->media_meta->copyright);
		new_file->media_meta->track_num =
			SAFE_STRDUP(src->media_meta->track_num);
		new_file->media_meta->description =
			SAFE_STRDUP(src->media_meta->description);
		new_file->media_meta->composer =
			SAFE_STRDUP(src->media_meta->composer);
		new_file->media_meta->year =
			SAFE_STRDUP(src->media_meta->year);
		new_file->media_meta->bitrate =
			src->media_meta->bitrate;
		new_file->media_meta->samplerate =
			src->media_meta->samplerate;
		new_file->media_meta->channel =
			src->media_meta->channel;
		new_file->media_meta->extra_media_meta =
			SAFE_STRDUP(src->media_meta->extra_media_meta);
	}

	if (NULL != src->cloud_meta) {
		new_file->cloud_meta->service_name =
			SAFE_STRDUP(src->cloud_meta->service_name);
		new_file->cloud_meta->usage_byte =
			src->cloud_meta->usage_byte;
		new_file->cloud_meta->quota_byte =
			src->cloud_meta->quota_byte;
		new_file->cloud_meta->extra_cloud_meta =
			SAFE_STRDUP(src->cloud_meta->extra_cloud_meta);
	}

	new_file->plugin_name =
		SAFE_STRDUP(src->plugin_name);
	new_file->object_id =
		SAFE_STRDUP(src->object_id);
	new_file->storage_path =
		SAFE_STRDUP(src->storage_path);
	new_file->file_size =
		src->file_size;
	new_file->created_time =
		src->created_time;
	new_file->modified_time =
		src->modified_time;
	new_file->file_info_index =
		src->file_info_index;
	new_file->content_type =
		src->content_type;
	new_file->extra_file_info =
		SAFE_STRDUP(src->extra_file_info);

	*dst_file = new_file;

	sac_api_end(ret);
	return ret;
}

int service_storage_file_destroy(service_storage_file_h file)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	if (NULL == file) {
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}
	service_storage_file_h file_info = file;
	service_storage_unref_file_info(&file_info);

	return ret;
}

int service_storage_file_list_get_length(service_storage_file_list_h list,
						int *length)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	if ((NULL == list) || (NULL == length)) {
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	*length = list->length;

	sac_api_end(ret);
	return ret;
}

int service_storage_file_list_foreach_file(service_storage_file_list_h list,
						service_storage_file_cb callback,
						void *user_data)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	if ((NULL == list) || (NULL == callback)) {
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	if (0 > list->length) {
		ret = SERVICE_ADAPTOR_ERROR_NO_DATA;
	} else {
		int i;
		bool is_continue = true;
		for (i = 0; i < list->length; i++) {
			if (is_continue) {
				is_continue = callback(list->list[i], user_data);
			}
		}
	}

	sac_api_end(ret);
	return ret;
}

int service_storage_file_get_logical_path(service_storage_file_h file,
						char **path)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	if ((NULL == file) || (NULL == path)) {
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	*path = SAFE_STRDUP(file->storage_path);

	sac_api_end(ret);
	return ret;
}

int service_storage_file_get_physical_path(service_storage_file_h file,
						char **path)
{

	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	if ((NULL == file) || (NULL == path)) {
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	*path = SAFE_STRDUP(file->object_id);

	sac_api_end(ret);
	return ret;
}

int service_storage_file_is_dir(service_storage_file_h file,
						bool *is_dir)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	if ((NULL == file) || (NULL == is_dir)) {
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	if (SERVICE_ADAPTOR_STORAGE_CONTENT_TYPE_FOLDER == file->content_type) {
		*is_dir = true;
	} else {
		*is_dir = false;
	}

	sac_api_end(ret);
	return ret;
}

int service_storage_file_get_size(service_storage_file_h file,
						unsigned long long *size)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	if ((NULL == file) || (NULL == size)) {
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	*size = file->file_size;

	sac_api_end(ret);
	return ret;
}

void *_remove_runnable(void *_data)
{
	FUNC_START();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	struct __async_wrapper_context *_params = (struct __async_wrapper_context *)_data;

	service_adaptor_error_s error;
	error.msg = NULL;

	ret = _dbus_remove_file(_params->plugin->service_handle_name, _params->path, NULL, &error);

	if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
		service_adaptor_set_last_result(error.code, error.msg); /* LCOV_EXCL_LINE */
		free(error.msg); /* LCOV_EXCL_LINE */
	}

	((service_storage_result_cb)_params->callback)(ret, _params->user_data);

	free(_params->path);
	free(_params);

	FUNC_END();
	return NULL;
}



int service_storage_remove(service_plugin_h plugin,
						const char *remove_path,
						service_storage_result_cb callback,
						void *user_data)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	if ((NULL == plugin) || (NULL == callback) || (NULL == remove_path)) {
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	if (NULL == plugin->service_handle_name) {
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_INVALID_STATE;
	}

	if (CLIENT_APP_TYPE_APPLICATION == plugin->app_type) {
		int privilege_ret = 0;
		service_adaptor_error_s error;
		error.msg = NULL;
		privilege_ret = _dbus_get_privilege_check_result(plugin->service_handle_name, TIZEN_PRIVILEGE_NAME_INTERNET, NULL, &error);
		if (SERVICE_ADAPTOR_ERROR_NONE != privilege_ret) {
			sac_error("Privilege check error (ret : %d)", privilege_ret); /* LCOV_EXCL_LINE */
			return SERVICE_ADAPTOR_ERROR_PERMISSION_DENIED; /* LCOV_EXCL_LINE */
		}
	}

	struct __async_wrapper_context *params = NULL;
	params = (struct __async_wrapper_context *) calloc(1, sizeof(struct __async_wrapper_context));

	if (NULL == params) {
		sac_warning("Memory allocation failed"); /* LCOV_EXCL_LINE */
		ret = SERVICE_ADAPTOR_ERROR_UNKNOWN; /* LCOV_EXCL_LINE */
	} else {
		params->plugin = plugin;
		params->path = strdup(remove_path);
		params->callback = (void *)callback;
		params->user_data = user_data;

		int thread_ret = 0;
		pthread_t remove_thread;
		thread_ret = pthread_create(&remove_thread, NULL, _remove_runnable, (void *)params);

		if (thread_ret) {
			sac_warning("Thread create failed"); /* LCOV_EXCL_LINE */
			ret = SERVICE_ADAPTOR_ERROR_UNKNOWN; /* LCOV_EXCL_LINE */
			free(params->path); /* LCOV_EXCL_LINE */
			free(params); /* LCOV_EXCL_LINE */
		}
	}

	return ret;

}
