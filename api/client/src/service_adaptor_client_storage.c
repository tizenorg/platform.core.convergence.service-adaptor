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
		free(_file_info);
		free(_media_meta);
		free(_cloud_meta);

		FUNC_STOP();
		return NULL;
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
		FUNC_STOP();
		return 1;
	}

	if (NULL == *file_info) {
		FUNC_STOP();
		return 0;
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


/**	@brief	Uploads a server file and writes it to local file
 *	@return	int
 *	@remarks :
 */
int service_storage_upload_file(service_plugin_h handle,
						const char *upload_path,
						const char *server_path,
						service_storage_file_h *file_info)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;

	if ((NULL == handle) || (NULL == upload_path) || (NULL == server_path)) {
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	if (NULL == handle->service_handle_name) {
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_INVALID_STATE;
	}

	ret = _dbus_upload_file(handle->service_handle_name, upload_path, server_path, file_info, NULL, &error);

	if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
		service_adaptor_set_last_result(error.code, error.msg);
		free(error.msg);
	}

	sac_api_end(ret);
	return ret;
}

/**	@brief	Requests
 *	@return	int
 *	@remarks :
 */
int service_storage_get_root_directory(service_plugin_h handle,
						char **root_folder_path)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;

	if (NULL == handle) {
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	if (NULL == handle->service_handle_name) {
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_INVALID_STATE;
	}

	ret = _dbus_get_root_folder_path(handle->service_handle_name, root_folder_path, NULL, &error);

	if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
		service_adaptor_set_last_result(error.code, error.msg);
		free(error.msg);
	}

	sac_api_end(ret);
	return ret;
}

/**	@brief	Requests
 *	@return	int
 *	@remarks :
 */
int service_storage_make_directory(service_plugin_h handle,
						const char *folder_path,
						service_storage_file_h *file_info)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;

	if (NULL == handle) {
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	if (NULL == handle->service_handle_name) {
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_INVALID_STATE;
	}

	ret = _dbus_make_directory(handle->service_handle_name, folder_path, file_info, NULL, &error);

	if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
		service_adaptor_set_last_result(error.code, error.msg);
		free(error.msg);
	}

	sac_api_end(ret);
	return ret;
}


/**	@brief	Requests
 *	@return	int
 *	@remarks :
 */
int service_storage_remove_file(service_plugin_h handle,
						const char *file_path)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;

	if ((NULL == handle) || (NULL == file_path)) {
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	if (NULL == handle->service_handle_name) {
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_INVALID_STATE;
	}

	ret = _dbus_remove_file(handle->service_handle_name, file_path, NULL, &error);

	if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
		service_adaptor_set_last_result(error.code, error.msg);
		free(error.msg);
	}

	sac_api_end(ret);
	return ret;
}


/**	@brief	Requests
 *	@return	int
 *	@remarks :
 */
int service_storage_remove_directory(service_plugin_h handle,
						const char *folder_path)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;

	if ((NULL == handle) || (NULL == folder_path)) {
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	if (NULL == handle->service_handle_name) {
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_INVALID_STATE;
	}

	ret = _dbus_remove_directory(handle->service_handle_name, folder_path, NULL, &error);

	if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
		service_adaptor_set_last_result(error.code, error.msg);
		free(error.msg);
	}

	sac_api_end(ret);
	return ret;
}


/**	@brief	Requests
 *	@return	int
 *	@remarks :
 */
int service_storage_move_file(service_plugin_h handle,
						const char *src_file_path,
						const char *dst_file_path,
						service_storage_file_h *file_info)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;

	if ((NULL == handle) || (NULL == src_file_path) || (NULL == dst_file_path)) {
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	if (NULL == handle->service_handle_name) {
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_INVALID_STATE;
	}

	ret = _dbus_move_file(handle->service_handle_name, src_file_path, dst_file_path, file_info, NULL, &error);

	if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
		service_adaptor_set_last_result(error.code, error.msg);
		free(error.msg);
	}

	sac_api_end(ret);
	return ret;
}


/**	@brief	Requests
 *	@return	int
 *	@remarks :
 */
int service_storage_move_directory(service_plugin_h handle,
						const char *src_folder_path,
						const char *dst_folder_path,
						service_storage_file_h *file_info)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;

	if ((NULL == handle) || (NULL == src_folder_path) || (NULL == dst_folder_path) || (NULL == file_info)) {
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	if (NULL == handle->service_handle_name) {
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_INVALID_STATE;
	}

	ret = _dbus_move_directory(handle->service_handle_name, src_folder_path, dst_folder_path, file_info, NULL, &error);

	if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
		service_adaptor_set_last_result(error.code, error.msg);
		free(error.msg);
	}

	sac_api_end(ret);
	return ret;
}

/**	@brief	Requests
 *	@return	int
 *	@remarks :
 */
int service_storage_get_directory_entries(service_plugin_h handle,
						const char *parent_path,
						service_storage_file_h **file_info_list,
						unsigned int *file_info_list_len)

{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;

	if ((NULL == handle) || (NULL == parent_path) || (NULL == file_info_list) || (NULL == file_info_list_len)) {
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	if (NULL == handle->service_handle_name) {
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_INVALID_STATE;
	}

	ret = _dbus_get_file_list(handle->service_handle_name, parent_path, file_info_list, file_info_list_len, NULL, &error);

	if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
		service_adaptor_set_last_result(error.code, error.msg);
		free(error.msg);
	}

	sac_api_end(ret);
	return ret;
}

int service_storage_download_file(service_plugin_h handle,
						const char *cloud_path,
						const char *download_path)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;

	if ((NULL == handle) || (NULL == cloud_path) || (NULL == download_path)) {
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	if (NULL == handle->service_handle_name) {
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_INVALID_STATE;
	}

	ret = _dbus_download_file(handle->service_handle_name, cloud_path, download_path, NULL, &error);

	if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
		service_adaptor_set_last_result(error.code, error.msg);
		free(error.msg);
	}

	sac_api_end(ret);
	return ret;
}

int service_storage_download_thumbnail(service_plugin_h handle,
						const char *storage_path,
						const char *local_path,
						int thumbnail_size)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;

	if ((NULL == handle) || (NULL == storage_path) || (NULL == local_path)) {
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	if (NULL == handle->service_handle_name) {
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_INVALID_STATE;
	}

	ret = _dbus_download_thumbnail(handle->service_handle_name, storage_path, local_path, thumbnail_size, &error);

	if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
		service_adaptor_set_last_result(error.code, error.msg);
		free(error.msg);
	}

	sac_api_end(ret);
	return ret;
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
/*
	if (CLIENT_APP_TYPE_APPLICATION == plugin->app_type) {
		int privilege_ret = 0;
		privilege_ret = privilege_checker_check_privilege(TIZEN_PRIVILEGE_NAME_INTERNET);
		if (PRIVILEGE_CHECKER_ERR_NONE != privilege_ret) {
			sac_error("Privilege check error (ret : %d)", privilege_ret);
			return SERVICE_ADAPTOR_ERROR_PERMISSION_DENIED;
		}
	}
*/

	service_storage_task_h _task = (service_storage_task_h) calloc(1, sizeof(service_storage_task_t));
	if (NULL == _task) {
		ret = SERVICE_ADAPTOR_ERROR_UNKNOWN;
		service_adaptor_set_last_result(ret, "Memory allocation failed");
		return ret;
	}

	long long int task_id = 0;
	ret = _dbus_open_upload_file(plugin->service_handle_name, file_path, upload_path, &task_id, &error);

	if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
		service_adaptor_set_last_result(error.code, error.msg);
		free(error.msg);
		free(_task);
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
/*
	if (CLIENT_APP_TYPE_APPLICATION == plugin->app_type) {
		int privilege_ret = 0;
		privilege_ret = privilege_checker_check_privilege(TIZEN_PRIVILEGE_NAME_INTERNET);
		if (PRIVILEGE_CHECKER_ERR_NONE != privilege_ret) {
			sac_error("Privilege check error (ret : %d)", privilege_ret);
			return SERVICE_ADAPTOR_ERROR_PERMISSION_DENIED;
		}
	}
*/
	service_storage_task_h _task = (service_storage_task_h) calloc(1, sizeof(service_storage_task_t));
	if (NULL == _task) {
		ret = SERVICE_ADAPTOR_ERROR_UNKNOWN;
		service_adaptor_set_last_result(ret, "Memory allocation failed");
		return ret;
	}

	long long int task_id = 0;
	ret = _dbus_open_download_file(plugin->service_handle_name, storage_path, download_path, &task_id, &error);

	if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
		service_adaptor_set_last_result(error.code, error.msg);
		free(error.msg);
		free(_task);
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
/*
	if (CLIENT_APP_TYPE_APPLICATION == plugin->app_type) {
		int privilege_ret = 0;
		privilege_ret = privilege_checker_check_privilege(TIZEN_PRIVILEGE_NAME_INTERNET);
		if (PRIVILEGE_CHECKER_ERR_NONE != privilege_ret) {
			sac_error("Privilege check error (ret : %d)", privilege_ret);
			return SERVICE_ADAPTOR_ERROR_PERMISSION_DENIED;
		}
	}
*/
	service_storage_task_h _task = (service_storage_task_h) calloc(1, sizeof(service_storage_task_t));
	int *t_size = (int *)calloc(1, sizeof(int));
	if ((NULL == _task) || (NULL == t_size)) {
		ret = SERVICE_ADAPTOR_ERROR_UNKNOWN;
		service_adaptor_set_last_result(ret, "Memory allocation failed");
		free(_task);
		free(t_size);
		return ret;
	}

	long long int task_id = 0;
	ret = _dbus_open_download_thumbnail(plugin->service_handle_name, storage_path, download_path, thumbnail_size, &task_id, &error);

	if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
		service_adaptor_set_last_result(error.code, error.msg);
		free(error.msg);
		free(_task);
		free(t_size);
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
		service_adaptor_set_last_result(error.code, error.msg);
		free(error.msg);
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
		sac_info("Invalid async task");
		ret = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
		error.code = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
		error.msg = strdup("Invalid async task operation");
	break;
	}

	if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
		service_adaptor_set_last_result(error.code, error.msg);
		free(error.msg);
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
		sac_info("Invalid async task");
		ret = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
		error.code = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
		error.msg = strdup("Invalid async task operation");
	break;
	}

	if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
		service_adaptor_set_last_result(error.code, error.msg);
		free(error.msg);
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
		service_adaptor_set_last_result(error.code, error.msg);
		free(error.msg);
		if ((NULL != files) && (0 < files_len)) {
			int i;
			for (i = 0; i < files_len; i++) {
				service_storage_unref_file_info(&files[i]);
			}
			free(files);
			files = NULL;
		}
	} else {
		if (0 < files_len) {
			file_list = (service_storage_file_list_h) calloc(1, sizeof(struct _service_storage_file_list_s));
			if (NULL == file_list) {
				sac_error("Critical : Memory allocation failed");
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
/*
	if (CLIENT_APP_TYPE_APPLICATION == plugin->app_type) {
		int privilege_ret = 0;
		privilege_ret = privilege_checker_check_privilege(TIZEN_PRIVILEGE_NAME_INTERNET);
		if (PRIVILEGE_CHECKER_ERR_NONE != privilege_ret) {
			sac_error("Privilege check error (ret : %d)", privilege_ret);
			return SERVICE_ADAPTOR_ERROR_PERMISSION_DENIED;
		}
	}
*/
	struct __async_wrapper_context *params = NULL;
	params = (struct __async_wrapper_context *) calloc(1, sizeof(struct __async_wrapper_context));

	if (NULL == params) {
		sac_warning("Memory allocation failed");
		ret = SERVICE_ADAPTOR_ERROR_UNKNOWN;
	} else {
		params->plugin = plugin;
		params->path = strdup(dir_path);
		params->callback = (void *)callback;
		params->user_data = user_data;

		int thread_ret = 0;
		pthread_t get_list_thread;
		thread_ret = pthread_create(&get_list_thread, NULL, _get_file_list_runnable, (void *)params);

		if (thread_ret) {
			sac_warning("Thread create failed");
			ret = SERVICE_ADAPTOR_ERROR_UNKNOWN;
			free(params->path);
			free(params);
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
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_UNKNOWN;
	}
	_list->length = src_list->length;
	_list->list = (service_storage_file_h *) calloc(_list->length, sizeof(struct _service_storage_file_s *));
	if (NULL == _list->list) {
		free(_list);
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_UNKNOWN;
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
		FUNC_STOP();
		return SERVICE_ADAPTOR_ERROR_UNKNOWN;
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
		service_adaptor_set_last_result(error.code, error.msg);
		free(error.msg);
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
/*
	if (CLIENT_APP_TYPE_APPLICATION == plugin->app_type) {
		int privilege_ret = 0;
		privilege_ret = privilege_checker_check_privilege(TIZEN_PRIVILEGE_NAME_INTERNET);
		if (PRIVILEGE_CHECKER_ERR_NONE != privilege_ret) {
			sac_error("Privilege check error (ret : %d)", privilege_ret);
			return SERVICE_ADAPTOR_ERROR_PERMISSION_DENIED;
		}
	}
*/
	struct __async_wrapper_context *params = NULL;
	params = (struct __async_wrapper_context *) calloc(1, sizeof(struct __async_wrapper_context));

	if (NULL == params) {
		sac_warning("Memory allocation failed");
		ret = SERVICE_ADAPTOR_ERROR_UNKNOWN;
	} else {
		params->plugin = plugin;
		params->path = strdup(remove_path);
		params->callback = (void *)callback;
		params->user_data = user_data;

		int thread_ret = 0;
		pthread_t remove_thread;
		thread_ret = pthread_create(&remove_thread, NULL, _remove_runnable, (void *)params);

		if (thread_ret) {
			sac_warning("Thread create failed");
			ret = SERVICE_ADAPTOR_ERROR_UNKNOWN;
			free(params->path);
			free(params);
		}
	}

	return ret;

}




/******************************** private feature */


/**	@brief	Registers File Progress Listener
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_register_file_progress_listener(service_adaptor_h handle,
						service_adaptor_file_progress_cb callback,
						void *user_data)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	_signal_queue_add_task(PRIVATE_SIGNAL_FILE_PROGRESS_ID, (uint32_t) callback, handle, user_data);

	return ret;
}

/**	@brief	Unregisters File Progress Listener
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_unregister_file_progress_listener(service_adaptor_h handle,
						void *user_data)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	service_adaptor_task_h task = _signal_queue_get_task(PRIVATE_SIGNAL_FILE_PROGRESS_ID);

	if (NULL == task) {
		return SERVICE_ADAPTOR_ERROR_UNKNOWN;
	}

	_signal_queue_del_task(task);

	return ret;
}

/**	@brief	Registers File Transfer Completion Listener
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_register_file_transfer_completion_listener(service_adaptor_h handle,
						service_adaptor_file_transfer_completion_cb callback,
						void *user_data)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	_signal_queue_add_task(PRIVATE_SIGNAL_FILE_TRANSFER_COMPLETION_ID, (uint32_t) callback, handle, user_data);

	return ret;
}

/**	@brief	Unregisters Forward Online Message Listener
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_unregister_file_transfer_completion_listener(service_adaptor_h handle,
						void *user_data)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	service_adaptor_task_h task = _signal_queue_get_task(PRIVATE_SIGNAL_FILE_TRANSFER_COMPLETION_ID);

	if (NULL == task) {
		return SERVICE_ADAPTOR_ERROR_UNKNOWN;
	}

	_signal_queue_del_task(task);

	return ret;
}

/**	@brief	Downloads a server file and writes it to local file
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_download_file(service_adaptor_h handle,
						const char *server_path,
						const char *download_path,
						service_adaptor_error_s **error_code,
						void *user_data)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;

	if ((NULL == handle) || (NULL == handle->plugin) || (NULL == server_path) || (NULL == download_path)) {
		sac_error("Invalid Parameter");
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER, "Invalid Argument");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	ret = _dbus_download_file(handle->service_name, server_path, download_path, user_data, &error);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		_assign_error_code(&error, error_code);
	}

	return ret;
}

/**	@brief	Downloads a server file and writes it to local file (Async)
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_download_file_async(service_adaptor_h handle,
						const char *server_path,
						const char *download_path,
						service_adaptor_file_h *file_handle,
						service_adaptor_error_s **error_code,
						void *user_data)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;

	if ((NULL == handle) || (NULL == handle->plugin) || (NULL == server_path) || (NULL == download_path)) {
		sac_error("Invalid Parameter");
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER, "Invalid Argument");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	ret = _private_dbus_download_file_async(handle->service_name, server_path, download_path, file_handle, user_data, &error);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		_assign_error_code(&error, error_code);
	}

	return ret;
}

/**	@brief	Uploads a server file and writes it to local file
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_upload_file(service_adaptor_h handle,
						const char *upload_path,
						const char *server_path,
						service_adaptor_error_s **error_code,
						void *user_data)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;

	if ((NULL == handle) || (NULL == handle->plugin) || (NULL == upload_path) || (NULL == server_path)) {
		sac_error("Invalid Parameter");
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER, "Invalid Argument");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	service_storage_file_h file = NULL;
	ret = _dbus_upload_file(handle->service_name, upload_path, server_path, &file, user_data, &error);
	service_storage_unref_file_info(&file);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		_assign_error_code(&error, error_code);
	}

	return ret;
}

/**	@brief	Uploads a server file and writes it to local file (Async)
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_upload_file_async(service_adaptor_h handle,
						const char *upload_path,
						const char *server_path,
						service_adaptor_file_h *file_handle,
						service_adaptor_error_s **error_code,
						void *user_data)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;

	if ((NULL == handle) || (NULL == handle->plugin) || (NULL == upload_path) || (NULL == server_path)) {
		sac_error("Invalid Parameter");
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER, "Invalid Argument");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	ret = _private_dbus_upload_file_async(handle->service_name, upload_path, server_path, file_handle, user_data, &error);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		_assign_error_code(&error, error_code);
	}

	return ret;
}

/**	@brief	Downloads a server file and writes it to local file
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_download_file_publish(service_adaptor_h handle,
						const char *publish_url,
						const char *download_path,
						service_adaptor_error_s **error_code,
						void *user_data)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;

	if ((NULL == handle) || (NULL == handle->plugin) || (NULL == publish_url) || (NULL == download_path)) {
		sac_error("Invalid Parameter");
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER, "Invalid Argument");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	ret = _dbus_download_file_publish(handle->service_name, publish_url, download_path, user_data, &error);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		_assign_error_code(&error, error_code);
	}

	return ret;
}

/**	@brief	Downloads a server file and writes it to local file (Async)
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_download_file_publish_async(service_adaptor_h handle,
						const char *publish_url,
						const char *download_path,
						service_adaptor_file_h *file_handle,
						service_adaptor_error_s **error_code,
						void *user_data)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;

	if ((NULL == handle) || (NULL == handle->plugin) || (NULL == publish_url) || (NULL == download_path)) {
		sac_error("Invalid Parameter");
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER, "Invalid Argument");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	ret = _dbus_download_file_publish_async(handle->service_name, publish_url, download_path, file_handle, user_data, &error);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		_assign_error_code(&error, error_code);
	}

	return ret;
}

/**	@brief	Uploads a server file and writes it to local file
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_upload_file_publish(service_adaptor_h handle,
						const char *upload_path,
						const char *server_path,
						char **publish_url,
						service_adaptor_error_s **error_code,
						void *user_data)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;

	if ((NULL == handle) || (NULL == handle->plugin) || (NULL == upload_path) || (NULL == server_path) || (NULL == publish_url)) {
		sac_error("Invalid Parameter");
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER, "Invalid Argument");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	ret = _dbus_upload_file_publish(handle->service_name, upload_path, server_path, publish_url, user_data, &error);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		_assign_error_code(&error, error_code);
	}

	return ret;
}

/**	@brief	Uploads a server file and writes it to local file (Async)
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_upload_file_publish_async(service_adaptor_h handle,
						const char *upload_path,
						const char *server_path,
						service_adaptor_file_h *file_handle,
						service_adaptor_error_s **error_code,
						void *user_data)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;

	if ((NULL == handle) || (NULL == handle->plugin) || (NULL == upload_path) || (NULL == server_path)) {
		sac_error("Invalid Parameter");
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER, "Invalid Argument");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	ret = _dbus_upload_file_publish_async(handle->service_name, upload_path, server_path, file_handle, user_data, &error);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		_assign_error_code(&error, error_code);
	}

	return ret;
}

/**	@brief	Downloads a thumbnail file and writes it to local file
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_download_thumbnail_publish(service_adaptor_h handle,
						const char *publish_url,
						const char *download_path,
						service_adaptor_thumbnail_size_e size,
						service_adaptor_error_s **error_code,
						void *user_data)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;
	char thumbnail_url[2048] = {0,};

	if ((NULL == handle) || (NULL == handle->plugin) || (NULL == publish_url) || (NULL == download_path)) {
		sac_error("Invalid Parameter");
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER, "Invalid Argument");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	char public_token[1024], auth_code[1024];
	sscanf(publish_url, "%[^'?']?auth_code=%s", public_token, auth_code);
	snprintf(thumbnail_url, 2048, "%s_%d?auth_code=%s", public_token, size, auth_code);

	ret = _dbus_download_file_publish(handle->service_name, thumbnail_url, download_path, user_data, &error);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		_assign_error_code(&error, error_code);
	}

	return ret;
}

/**	@brief	Requests File Status
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_get_file_status(service_adaptor_h handle,
						service_adaptor_file_h file_handle,
						service_adaptor_file_status_s **status,
						service_adaptor_error_s **error_code,
						void **server_data)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;

	if ((NULL == handle) || (NULL == handle->plugin) || (NULL == file_handle) || (NULL == status)) {
		sac_error("Invalid Parameter");
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER, "Invalid Argument");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	ret = _dbus_get_file_status(handle->service_name, file_handle, status, server_data, &error);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		_assign_error_code(&error, error_code);
	}

	return ret;
}

/**	@brief	Cancels File Transfer
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_cancel_file_transfer(service_adaptor_h handle,
						service_adaptor_file_h file_handle,
						service_adaptor_error_s **error_code,
						void **server_data)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;

	if ((NULL == handle) || (NULL == handle->plugin) || (NULL == file_handle)) {
		sac_error("Invalid Parameter");
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER, "Invalid Argument");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	ret = _dbus_cancel_file_transfer(handle->service_name, file_handle, server_data, &error);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		_assign_error_code(&error, error_code);
	}

	return ret;
}

/**	@brief	Pause File Transfer
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_pause_file_transfer(service_adaptor_h handle,
						service_adaptor_file_h file_handle,
						service_adaptor_error_s **error_code,
						void **server_data)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;

	if ((NULL == handle) || (NULL == handle->plugin) || (NULL == file_handle)) {
		sac_error("Invalid Parameter");
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER, "Invalid Argument");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	ret = _dbus_pause_file_transfer(handle->service_name, file_handle, server_data, &error);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		_assign_error_code(&error, error_code);
	}

	return ret;
}

/**	@brief	Resume File Transfer
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_resume_file_transfer(service_adaptor_h handle,
						service_adaptor_file_h file_handle,
						service_adaptor_error_s **error_code,
						void **server_data)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;

	if ((NULL == handle) || (NULL == handle->plugin) || (NULL == file_handle)) {
		sac_error("Invalid Parameter");
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER, "Invalid Argument");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	ret = _dbus_resume_file_transfer(handle->service_name, file_handle, server_data, &error);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		_assign_error_code(&error, error_code);
	}

	return ret;
}

/******************************** private feature */
