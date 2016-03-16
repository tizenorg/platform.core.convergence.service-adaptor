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
 * File: dbus-client-storage.c
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
#include "dbus_client_storage.h"
#include "service_adaptor_client_type.h"
#include "service_adaptor_client_log.h"
#include "service_adaptor_client_storage.h"
#include "service_adaptor_client_storage_internal.h"

#include "private/service-adaptor-client.h"
#include "private/service-adaptor-client-storage.h"

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

void __get_file_progress_type(GVariant *parameters,
						long long int *_file_uid,
						unsigned long long *_progress_size,
						unsigned long long *_total_size)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *res_struct[service_adaptor_file_progress_s_type_length];

	for (size_t j = 0; j < service_adaptor_file_progress_s_type_length; j++) {
		res_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	do {
		GVariant *fd_res_struct[service_adaptor_file_descriptor_s_type_length];
		for (size_t k = 0; k < service_adaptor_file_descriptor_s_type_length; k++) {
			fd_res_struct[k] = g_variant_get_child_value(res_struct[idx], k);
		}
		int fd_idx = 0;
		*_file_uid = (long long int) g_variant_get_int64(fd_res_struct[fd_idx++]);

		for (size_t k = 0; k < service_adaptor_file_descriptor_s_type_length; k++) {
			g_variant_unref(fd_res_struct[k]);
		}
		idx++;
	} while (0);

	*_progress_size = (unsigned long long) g_variant_get_uint64(res_struct[idx++]);
	*_total_size = (unsigned long long) g_variant_get_uint64(res_struct[idx++]);

	for (size_t j = 0; j < service_adaptor_file_progress_s_type_length; j++) {
		g_variant_unref(res_struct[j]);
	}
}

void __get_file_transfer_state_changed_type(GVariant *parameters,
						long long int *_file_uid,
						int *_state)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *res_struct[service_adaptor_file_transfer_state_changed_s_type_length];

	for (size_t j = 0; j < service_adaptor_file_transfer_state_changed_s_type_length; j++) {
		res_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	do {
		GVariant *fd_res_struct[service_adaptor_file_descriptor_s_type_length];
		for (size_t k = 0; k < service_adaptor_file_descriptor_s_type_length; k++) {
			fd_res_struct[k] = g_variant_get_child_value(res_struct[idx], k);
		}
		int fd_idx = 0;
		*_file_uid = (long long int) g_variant_get_int64(fd_res_struct[fd_idx++]);

		for (size_t k = 0; k < service_adaptor_file_descriptor_s_type_length; k++) {
			g_variant_unref(fd_res_struct[k]);
		}
		idx++;
	} while (0);

	*_state = (int) g_variant_get_int32(res_struct[idx++]);

	for (size_t j = 0; j < service_adaptor_file_transfer_state_changed_s_type_length; j++) {
		g_variant_unref(res_struct[j]);
	}
}

void __get_file_info_s_type(GVariant *call_result_struct,
						service_storage_file_h *file_info)
{
	service_storage_file_h _file_info = NULL;
	GVariant *res_info_struct[service_adaptor_file_info_s_type_length];

	for (size_t j = 0; j < service_adaptor_file_info_s_type_length; j++) {
		res_info_struct[j] = g_variant_get_child_value(call_result_struct, j);
	}

	int idx1 = 0, idx2 = 0, idx3 = 0;
	_file_info = service_storage_create_file_info();
	if (NULL == _file_info) {
		*file_info = NULL;

		for (size_t j = 0; j < service_adaptor_file_info_s_type_length; j++) {
			g_variant_unref(res_info_struct[j]);
		}
		return;
	}

	_file_info->plugin_name			= ipc_g_variant_dup_string(res_info_struct[idx1++]);
	_file_info->object_id			= ipc_g_variant_dup_string(res_info_struct[idx1++]);
	_file_info->storage_path		= ipc_g_variant_dup_string(res_info_struct[idx1++]);
	_file_info->file_size			= g_variant_get_uint64(res_info_struct[idx1++]);
	_file_info->created_time		= g_variant_get_uint64(res_info_struct[idx1++]);
	_file_info->modified_time		= g_variant_get_uint64(res_info_struct[idx1++]);
	_file_info->file_info_index		= g_variant_get_int32(res_info_struct[idx1++]);
	_file_info->content_type		= g_variant_get_int32(res_info_struct[idx1++]);

	GVariant *media_meta_info[service_adaptor_content_meta_s_type_length];
	do {	/* get media_meta */
		for (size_t k = 0; k < service_adaptor_content_meta_s_type_length; k++) {
			media_meta_info[k] = g_variant_get_child_value(res_info_struct[idx1], k);
		}

		_file_info->media_meta->mime_type		= ipc_g_variant_dup_string(media_meta_info[idx2++]);
		_file_info->media_meta->title			= ipc_g_variant_dup_string(media_meta_info[idx2++]);
		_file_info->media_meta->album			= ipc_g_variant_dup_string(media_meta_info[idx2++]);
		_file_info->media_meta->artist			= ipc_g_variant_dup_string(media_meta_info[idx2++]);
		_file_info->media_meta->genere			= ipc_g_variant_dup_string(media_meta_info[idx2++]);
		_file_info->media_meta->recorded_date		= ipc_g_variant_dup_string(media_meta_info[idx2++]);
		_file_info->media_meta->width			= g_variant_get_int32(media_meta_info[idx2++]);
		_file_info->media_meta->height			= g_variant_get_int32(media_meta_info[idx2++]);
		_file_info->media_meta->duration		= g_variant_get_int32(media_meta_info[idx2++]);
		_file_info->media_meta->copyright		= ipc_g_variant_dup_string(media_meta_info[idx2++]);
		_file_info->media_meta->track_num		= ipc_g_variant_dup_string(media_meta_info[idx2++]);
		_file_info->media_meta->description		= ipc_g_variant_dup_string(media_meta_info[idx2++]);
		_file_info->media_meta->composer		= ipc_g_variant_dup_string(media_meta_info[idx2++]);
		_file_info->media_meta->year			= ipc_g_variant_dup_string(media_meta_info[idx2++]);
		_file_info->media_meta->bitrate			= g_variant_get_int32(media_meta_info[idx2++]);
		_file_info->media_meta->samplerate		= g_variant_get_int32(media_meta_info[idx2++]);
		_file_info->media_meta->channel			= g_variant_get_int32(media_meta_info[idx2++]);
		_file_info->media_meta->extra_media_meta	= ipc_g_variant_dup_string(media_meta_info[idx2++]);

		for (size_t k = 0; k < service_adaptor_content_meta_s_type_length; k++) {
			g_variant_unref(media_meta_info[k]);
		}
	} while (0);
	idx1++;

	GVariant *cloud_meta_info[service_adaptor_cloud_meta_s_type_length];
	do { /* get cloud_meta */
		for (size_t l = 0; l < service_adaptor_cloud_meta_s_type_length; l++) {
			cloud_meta_info[l] = g_variant_get_child_value(res_info_struct[idx1], l);
		}

		_file_info->cloud_meta->service_name		= ipc_g_variant_dup_string(cloud_meta_info[idx3++]);
		_file_info->cloud_meta->usage_byte		= g_variant_get_uint64(cloud_meta_info[idx3++]);
		_file_info->cloud_meta->quota_byte		= g_variant_get_uint64(cloud_meta_info[idx3++]);
		_file_info->cloud_meta->extra_cloud_meta	= ipc_g_variant_dup_string(cloud_meta_info[idx3++]);

		for (size_t l = 0; l < service_adaptor_cloud_meta_s_type_length; l++) {
			g_variant_unref(cloud_meta_info[l]);
		}
	} while (0);
	idx1++;


	_file_info->extra_file_info	= ipc_g_variant_dup_string(res_info_struct[idx1++]);

	for (size_t j = 0; j < service_adaptor_file_info_s_type_length; j++) {
		g_variant_unref(res_info_struct[j]);
	}

	*file_info = _file_info;
}

void __get_get_file_list_res_type(GVariant *in_parameters,
						service_storage_file_h **file_info_list,
						unsigned int *file_info_list_len)
{
	GVariant *res_info_struct[service_adaptor_get_file_list_res_s_type_length];

	for (size_t j = 0; j < service_adaptor_get_file_list_res_s_type_length; j++) {
		res_info_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	gsize list_count = g_variant_n_children(res_info_struct[idx]);
	*file_info_list = (service_storage_file_s **) calloc(list_count, sizeof(service_storage_file_s *));

	if (NULL != *file_info_list) {
		for (gsize i = 0; i < list_count; i++) {
			GVariant *info_entry_v = g_variant_get_child_value(res_info_struct[idx], i);

			__get_file_info_s_type(info_entry_v, &((*file_info_list)[i]));

			g_variant_unref(info_entry_v);
		}
		idx++;
		*file_info_list_len = g_variant_get_uint32(res_info_struct[idx++]);
	} else {
		*file_info_list_len = 0U;
	}

	for (size_t j = 0; j < service_adaptor_get_file_list_res_s_type_length; j++) {
		g_variant_unref(res_info_struct[j]);
	}
}

void __get_file_descriptor_s_type(GVariant *call_result_struct,
						long long int *_file_uid)
{
	GVariant *res_info_struct[service_adaptor_file_descriptor_s_type_length];

	for (size_t j = 0; j < service_adaptor_file_descriptor_s_type_length; j++) {
		res_info_struct[j] = g_variant_get_child_value(call_result_struct, j);
	}

	int idx = 0;
	*_file_uid = (long long int) g_variant_get_int64(res_info_struct[idx++]);

	for (size_t j = 0; j < service_adaptor_file_descriptor_s_type_length; j++) {
		g_variant_unref(res_info_struct[j]);
	}
}


/******************************************************************************
 * Public interface definition
 ******************************************************************************/

void on_storage_signal(GDBusProxy *proxy,
						gchar *sender_name,
						gchar *signal_name,
						GVariant *parameters,
						gpointer user_data)
{
	if (0 == g_strcmp0(signal_name, DBUS_STORAGE_FILE_PROGRESS_SIGNAL)) {
		long long int file_uid = 0;
		unsigned long long progress_size = 0;
		unsigned long long total_size = 0;

		__get_file_progress_type(parameters, &file_uid, &progress_size, &total_size);

		service_adaptor_task_h task = _queue_get_task(file_uid);

		if (NULL == task) {
			sac_warning("Callback task get failed");
			return;
		} else if (NULL == task->handle) {
			sac_warning("Callback task->handle get failed");
			return;
		} else {
			service_storage_task_h storage_task = (service_storage_task_h) task->handle;

			if (NULL != storage_task->progress_callback) {
				sac_debug("Call progress callback[%lld] (%llu/%llu byte)", file_uid, progress_size, total_size);
				storage_task->progress_callback(progress_size, total_size, storage_task->progress_user_data);
			}
		}
	} else if (0 == g_strcmp0(signal_name, DBUS_STORAGE_FILE_TRANSFER_STATE_CHANGED_SIGNAL)) {
		long long int file_uid = 0;
		int state = 0;

		__get_file_transfer_state_changed_type(parameters, &file_uid, &state);

		/* Convert to sac enum from sa enum */
		service_storage_task_state_e _state;
		switch (state) {
		case SERVICE_ADAPTOR_FILE_TRANSFER_STATE_IN_PROGRESS:
			_state = SERVICE_STORAGE_TASK_IN_PROGRESS;
			break;
		case SERVICE_ADAPTOR_FILE_TRANSFER_STATE_COMPLETED:
			_state = SERVICE_STORAGE_TASK_COMPLETED;
			break;
		case SERVICE_ADAPTOR_FILE_TRANSFER_STATE_CANCELED:
			_state = SERVICE_STORAGE_TASK_CANCELED;
			break;
		case SERVICE_ADAPTOR_FILE_TRANSFER_STATE_FAILED:
			_state = SERVICE_STORAGE_TASK_FAILED;
			break;
		default:
			return;
		}

		GVariant *call_result[2];
		call_result[0] = g_variant_get_child_value(parameters, 1);
		call_result[1] = g_variant_get_child_value(parameters, 2);
		uint64_t remote_call_result = g_variant_get_uint64(call_result[0]);
		char *remote_call_message = ipc_g_variant_dup_string(call_result[1]);

		service_adaptor_set_last_result((int) remote_call_result, remote_call_message);
		free(remote_call_message);

		service_adaptor_task_h task = _queue_get_task(file_uid);

		if (NULL == task) {
			sac_warning("Callback task get failed");
			return;
		} else if (NULL == task->handle) {
			sac_warning("Callback task->handle get failed");
			return;
		} else {
			service_storage_task_h storage_task = (service_storage_task_h) task->handle;

			if (NULL != storage_task->state_callback) {
				sac_debug("Call state callback[%lld] (%d state)", file_uid, _state);
				storage_task->state_callback(_state, storage_task->state_user_data);
			}
		}
	}
}

int _dbus_remove_file(const char *service_name,
						const char *file_path,
						void *user_data,
						service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();

	ipc_check_proxy(sac_interface_proxy);

	GVariant *request = g_variant_new("(" service_adaptor_remove_file_req_s_type ")", service_name, file_path);

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			DBUS_REMOVE_FILE_METHOD,
			request,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
			NULL,
			&g_error);

	ret = _ipc_get_simple_result(call_result, g_error, error);

	return ret;


}

int _dbus_get_file_list(const char *service_name,
						const char *parent_path,
						service_storage_file_h **file_info_list,
						unsigned int *file_info_list_len,
						void *user_data,
						service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();

	ipc_check_proxy(sac_interface_proxy);

	GVariant *request = g_variant_new("(" service_adaptor_get_file_list_req_s_type ")", service_name, parent_path);

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			DBUS_GET_FILE_LIST_METHOD,
			request,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
			NULL,
			&g_error);

	_ipc_get_complex_result(MAKE_RETURN_TYPE(service_adaptor_get_file_list_res_s_type),
		__get_get_file_list_res_type(call_result_struct[0], file_info_list, file_info_list_len);
	);

	return ret;
}

int _dbus_open_upload_file(const char *_service_name,
						const char *_file_path,
						const char *_upload_path,
						long long int *_task_id,
						service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();

	ipc_check_proxy(sac_interface_proxy);

	GVariant *request = g_variant_new("(" service_adaptor_open_upload_file_req_s_type ")", _service_name, _file_path, _upload_path);

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			DBUS_OPEN_UPLOAD_FILE_METHOD,
			request,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
			NULL,
			&g_error);

	_ipc_get_complex_result(MAKE_RETURN_TYPE(service_adaptor_file_descriptor_s_type),
		long long int file_uid = 0;
		__get_file_descriptor_s_type(call_result_struct[0], &file_uid);
		*_task_id = file_uid;
	);

	return ret;
}

int _dbus_open_download_file(const char *_service_name,
						const char *_storage_path,
						const char *_download_path,
						long long int *_task_id,
						service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();

	ipc_check_proxy(sac_interface_proxy);

	GVariant *request = g_variant_new("(" service_adaptor_open_download_file_req_s_type ")", _service_name, _storage_path, _download_path);

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			DBUS_OPEN_DOWNLOAD_FILE_METHOD,
			request,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
			NULL,
			&g_error);

	_ipc_get_complex_result(MAKE_RETURN_TYPE(service_adaptor_file_descriptor_s_type),
		long long int file_uid = 0;
		__get_file_descriptor_s_type(call_result_struct[0], &file_uid);
		*_task_id = file_uid;
	);

	return ret;
}

int _dbus_open_download_thumbnail(const char *_service_name,
						const char *_storage_path,
						const char *_download_path,
						int _thumbnail_size,
						long long int *_task_id,
						service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();

	ipc_check_proxy(sac_interface_proxy);

	GVariant *request = g_variant_new("(" service_adaptor_open_download_thumbnail_req_s_type ")", _service_name, _storage_path, _download_path, _thumbnail_size);

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			DBUS_OPEN_DOWNLOAD_THUMBNAIL_METHOD,
			request,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
			NULL,
			&g_error);

	_ipc_get_complex_result(MAKE_RETURN_TYPE(service_adaptor_file_descriptor_s_type),
		long long int file_uid = 0;
		__get_file_descriptor_s_type(call_result_struct[0], &file_uid);
		*_task_id = file_uid;
	);

	return ret;
}

int __dbus_send_simple_fd_for_task(GVariant *request,
						const char *_method_name,
						service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();

	ipc_check_proxy(sac_interface_proxy);

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			_method_name,
			request,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
			NULL,
			&g_error);

	ret = _ipc_get_simple_result(call_result, g_error, error);

	return ret;
}


int _dbus_close_file_task(const char *_service_name,
						long long int _task_id,
						service_adaptor_error_s *error)
{
	GVariant *request = g_variant_new("("service_adaptor_close_file_req_s_type")", _service_name, (int64_t)_task_id);

	return __dbus_send_simple_fd_for_task(request, DBUS_CLOSE_FILE_METHOD, error);
}

int _dbus_cancel_upload_file(const char *_service_name,
						long long int _task_id,
						service_adaptor_error_s *error)
{
	GVariant *request = g_variant_new("("service_adaptor_cancel_file_task_req_s_type")", _service_name, (int64_t)_task_id);

	return __dbus_send_simple_fd_for_task(request, DBUS_CANCEL_UPLOAD_FILE_METHOD, error);
}

int _dbus_cancel_download_file(const char *_service_name,
						long long int _task_id,
						service_adaptor_error_s *error)
{
	GVariant *request = g_variant_new("("service_adaptor_cancel_file_task_req_s_type")", _service_name, (int64_t)_task_id);

	return __dbus_send_simple_fd_for_task(request, DBUS_CANCEL_DOWNLOAD_FILE_METHOD, error);
}

int _dbus_cancel_download_thumbnail(const char *_service_name,
						long long int _task_id,
						service_adaptor_error_s *error)
{
	GVariant *request = g_variant_new("("service_adaptor_cancel_file_task_req_s_type")", _service_name, (int64_t)_task_id);

	return __dbus_send_simple_fd_for_task(request, DBUS_CANCEL_DOWNLOAD_THUMBNAIL_METHOD, error);
}





int _dbus_start_upload_file(const char *_service_name,
						long long int _task_id,
						const char *_storage_path,
						bool _need_progress,
						bool _need_state,
						service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();

	ipc_check_proxy(sac_interface_proxy);

	GVariant *request = g_variant_new("("service_adaptor_start_upload_file_req_s_type")", _service_name, (int64_t)_task_id, _storage_path, _need_progress, _need_state);

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			DBUS_START_UPLOAD_FILE_METHOD,
			request,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
			NULL,
			&g_error);

	ret = _ipc_get_simple_result(call_result, g_error, error);

	return ret;
}


int _dbus_start_download_file(const char *_service_name,
						long long int _task_id,
						const char *_storage_path,
						bool _need_progress,
						bool _need_state,
						service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();

	ipc_check_proxy(sac_interface_proxy);

	GVariant *request = g_variant_new("("service_adaptor_start_download_file_req_s_type")", _service_name, (int64_t)_task_id, _storage_path, _need_progress, _need_state);

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			DBUS_START_DOWNLOAD_FILE_METHOD,
			request,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
			NULL,
			&g_error);

	ret = _ipc_get_simple_result(call_result, g_error, error);

	return ret;
}


int _dbus_start_download_thumbnail(const char *_service_name,
						long long int _task_id,
						const char *_storage_path,
						int _thumbnail_size,
						bool _need_progress,
						bool _need_state,
						service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();

	ipc_check_proxy(sac_interface_proxy);

	GVariant *request = g_variant_new("("service_adaptor_start_download_thumbnail_req_s_type")", _service_name, (int64_t)_task_id, _storage_path, (int32_t)_thumbnail_size, _need_progress, _need_state);

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			DBUS_START_DOWNLOAD_THUMBNAIL_METHOD,
			request,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
			NULL,
			&g_error);

	ret = _ipc_get_simple_result(call_result, g_error, error);

	return ret;
}

int _dbus_get_privilege_check_result(const char *service_name,
						const char *privilege_name,
						void **server_data,
						service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();

	ipc_check_proxy(sac_interface_proxy);

	if ((NULL == service_name) || (NULL == privilege_name)) {
		error->code = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
		error->msg = strdup("Invalid Param");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	GVariant *request = g_variant_new("(" private_service_adaptor_privilege_check_req_s_type ")", __safe_add_string(service_name), __safe_add_string(privilege_name));

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			PRIVATE_DBUS_GET_PRIVILEGE_CHECK_RESULT_METHOD,
			request,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
			NULL,
			&g_error);

	ret = _ipc_get_simple_result(call_result, g_error, error);

	return ret;
}
