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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <gio/gio.h>
#include <libgen.h>

#include "service-adaptor.h"
#include "service-adaptor-storage.h"
#include "service-adaptor-type.h"
#include "service-adaptor-log.h"
#include "dbus-storage-adaptor.h"
#include "dbus-server.h"
#include "dbus-server-type.h"
#include "dbus-util.h"
#include "util/client_checker.h"

#define MEDIA_META_CLEAR(x) do {\
		(x)->mime_type          = NULL;\
		(x)->title              = NULL;\
		(x)->album              = NULL;\
		(x)->artist             = NULL;\
		(x)->genere             = NULL;\
		(x)->recorded_date      = NULL;\
		(x)->width              = -1;\
		(x)->height             = -1;\
		(x)->duration           = -1;\
		(x)->copyright          = NULL;\
		(x)->track_num          = NULL;\
		(x)->description        = NULL;\
		(x)->composer           = NULL;\
		(x)->year               = NULL;\
		(x)->bitrate            = -1;\
		(x)->samplerate         = -1;\
		(x)->channel            = -1;\
		(x)->extra_media_meta   = NULL; } while (0)

#define CLOUD_META_CLEAR(x) do {\
		(x)->service_name       = NULL;\
		(x)->usage_byte         = 0ULL;\
		(x)->quota_byte         = 0ULL;\
		(x)->extra_cloud_meta   = NULL; } while (0)

#define FILE_INFO_CLEAR(x) do {\
                (x)->plugin_uri          = NULL;\
                (x)->object_id           = NULL;\
                (x)->storage_path        = NULL;\
                (x)->file_size           = 0ULL;\
\
		(x)->revision            = -1;\
		(x)->timestamp           = 0ULL;\
		(x)->type                = NULL;\
		(x)->deleted             = -1;\
		(x)->expired_time        = 0ULL;\
		(x)->download_count      = -0U;\
		(x)->max_download_count  = -0U;\
		(x)->file_info_index     = -1;\
		(x)->tag                 = NULL;\
		(x)->file_share_token    = NULL;\
\
		(x)->created_time        = 0ULL;\
		(x)->modified_time       = 0ULL;\
		(x)->file_info_index     = -1;\
		(x)->content_type        = STORAGE_ADAPTOR_CONTENT_TYPE_DEFAULT;\
		(x)->extra_file_info     = NULL;\
\
		(x)->media_meta          = NULL;\
		(x)->cloud_meta          = NULL; } while (0)


#define __check_error_code()	do { \
		if (NULL == error_code) { \
			error_code = &_error; \
			if (ret_code) { \
				error_code->code = ret_code; \
				error_code->msg = strdup("[PLUGIN_ERROR] Unknown (Couldn't get detail message from plugin)"); \
			} else { \
				error_code->code = STORAGE_ADAPTOR_ERROR_NONE; \
				error_code->msg = strdup(""); \
			} \
		} \
	} while (0)

void __get_file_transfer_req_type(GVariant *parameters,
						char **service_name,
						char **server_path,
						char **download_path)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *req_struct[service_adaptor_file_transfer_req_s_type_length];

	for (size_t j = 0; j < service_adaptor_file_transfer_req_s_type_length; j++) {
		req_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*service_name = ipc_g_variant_dup_string(req_struct[idx++]);
	*server_path = ipc_g_variant_dup_string(req_struct[idx++]);
	*download_path = ipc_g_variant_dup_string(req_struct[idx++]);

	for (size_t j = 0; j < service_adaptor_file_transfer_req_s_type_length; j++) {
		g_variant_unref(req_struct[j]);
	}
}

void __get_download_thumbnail_req_type(GVariant *parameters,
						char **service_name,
						char **server_path,
						char **download_path,
						int *thumbnail_size)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *req_struct[service_adaptor_download_thumbnail_req_s_type_length];

	for (size_t j = 0; j < service_adaptor_download_thumbnail_req_s_type_length; j++) {
		req_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*service_name = ipc_g_variant_dup_string(req_struct[idx++]);
	*server_path = ipc_g_variant_dup_string(req_struct[idx++]);
	*download_path = ipc_g_variant_dup_string(req_struct[idx++]);
	*thumbnail_size = (int) g_variant_get_int32(req_struct[idx++]);

	for (size_t j = 0; j < service_adaptor_download_thumbnail_req_s_type_length; j++) {
		g_variant_unref(req_struct[j]);
	}
}

/* private feature */
GVariant *__private_create_file_transfer_res_type(int fd,
						storage_adaptor_error_code_h error_code)
{
	storage_adaptor_error_code_t _error;
	_error.msg = NULL;

	if (NULL == error_code) {
		error_code = &_error;
		if (0 == fd) {
			error_code->code = STORAGE_PLUGIN_ERROR_FILE_OPEN_FAILED;
			error_code->msg = strdup("Open file failed");
		} else {
			error_code->code = STORAGE_ADAPTOR_ERROR_NONE;
			error_code->msg = strdup("");
		}
	}

	GVariant *response = g_variant_new(MAKE_RETURN_TYPE(private_service_adaptor_file_s_type), fd,
			(uint64_t) error_code->code, __safe_add_string(error_code->msg));

	free(_error.msg);

	return response;
}

GVariant *__private_create_file_publish_transfer_res_type(storage_adaptor_file_info_h file_info,
						storage_adaptor_error_code_h error_code)
{
	char publish_url[1024] = {0, };
	storage_adaptor_error_code_t _error;
	_error.msg = NULL;

	if (NULL == error_code) {
		error_code = &_error;
		error_code->code = STORAGE_ADAPTOR_ERROR_NONE;
		error_code->msg = strdup("");
	}

	if ((NULL == file_info) || (NULL == file_info->file_share_token)) {
		snprintf(publish_url, 1024, "%s?auth_code=%s", "", "");
		error_code->code = STORAGE_ADAPTOR_ERROR_SERVER_INTERNAL;
		free(error_code->msg);
		error_code->msg = strdup("Invalid Publish Token");
	} else {
		snprintf(publish_url, 1024, "%s?auth_code=%s", file_info->file_share_token->public_token,
				file_info->file_share_token->auth_code);
	}

	GVariant *response = g_variant_new(MAKE_RETURN_TYPE(private_service_adaptor_file_publish_s_type), publish_url,
			(uint64_t) error_code->code, __safe_add_string(error_code->msg));

	free(_error.msg);

	return response;
}

void __private_get_file_status_req_type(GVariant *parameters,
						char **service_name,
						int *fd)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *req_struct[private_service_adaptor_file_status_req_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_file_status_req_s_type_length; j++) {
		req_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*service_name = ipc_g_variant_dup_string(req_struct[idx++]);
	*fd = g_variant_get_int32(req_struct[idx++]);

	for (size_t j = 0; j < private_service_adaptor_file_status_req_s_type_length; j++) {
		g_variant_unref(req_struct[j]);
	}
}

GVariant *__private_create_file_status_res_type(int64_t progress_size,
						int64_t total_size,
						storage_adaptor_transfer_state_e state,
						storage_adaptor_error_code_h error_code)
{
	storage_adaptor_error_code_t _error;
	_error.msg = NULL;

	if (NULL == error_code) {
		error_code = &_error;
		error_code->code = STORAGE_ADAPTOR_ERROR_NONE;
		error_code->msg = strdup("");
	}

	GVariant *response = g_variant_new(MAKE_RETURN_TYPE(private_service_adaptor_file_status_res_s_type),
			total_size, progress_size, (uint64_t) state,
			(uint64_t) error_code->code, __safe_add_string(error_code->msg));

	free(_error.msg);

	return response;
}

/* public feature */
void __get_get_root_folder_path_req_type(GVariant *parameters,
						char **service_name)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *req_struct[service_adaptor_get_root_folder_path_req_s_type_length];

	for (size_t j = 0; j < service_adaptor_get_root_folder_path_req_s_type_length; j++) {
		req_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*service_name = ipc_g_variant_dup_string(req_struct[idx++]);

	for (size_t j = 0; j < service_adaptor_get_root_folder_path_req_s_type_length; j++) {
		g_variant_unref(req_struct[j]);
	}
}

void __get_make_directory_req_type(GVariant *parameters,
						char **service_name,
						char **folder_path)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *req_struct[service_adaptor_make_directory_req_s_type_length];

	for (size_t j = 0; j < service_adaptor_make_directory_req_s_type_length; j++) {
		req_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*service_name	= ipc_g_variant_dup_string(req_struct[idx++]);
	*folder_path	= ipc_g_variant_dup_string(req_struct[idx++]);

	for (size_t j = 0; j < service_adaptor_make_directory_req_s_type_length; j++) {
		g_variant_unref(req_struct[j]);
	}
}

void __get_remove_file_req_type(GVariant *parameters,
						char **service_name,
						char **file_path)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *req_struct[service_adaptor_remove_file_req_s_type_length];

	for (size_t j = 0; j < service_adaptor_remove_file_req_s_type_length; j++) {
		req_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*service_name = ipc_g_variant_dup_string(req_struct[idx++]);
	*file_path = ipc_g_variant_dup_string(req_struct[idx++]);

	for (size_t j = 0; j < service_adaptor_remove_file_req_s_type_length; j++) {
		g_variant_unref(req_struct[j]);
	}
}

void __get_remove_directory_req_type(GVariant *parameters,
						char **service_name,
						char **folder_path)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *req_struct[service_adaptor_remove_directory_req_s_type_length];

	for (size_t j = 0; j < service_adaptor_remove_directory_req_s_type_length; j++) {
		req_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*service_name = ipc_g_variant_dup_string(req_struct[idx++]);
	*folder_path = ipc_g_variant_dup_string(req_struct[idx++]);

	for (size_t j = 0; j < service_adaptor_remove_directory_req_s_type_length; j++) {
		g_variant_unref(req_struct[j]);
	}
}

void __get_move_file_req_type(GVariant *parameters,
						char **service_name,
						char **src_file_path,
						char **des_file_path)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *req_struct[service_adaptor_move_file_req_s_type_length];

	for (size_t j = 0; j < service_adaptor_move_file_req_s_type_length; j++) {
		req_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*service_name = ipc_g_variant_dup_string(req_struct[idx++]);
	*src_file_path = ipc_g_variant_dup_string(req_struct[idx++]);
	*des_file_path = ipc_g_variant_dup_string(req_struct[idx++]);

	for (size_t j = 0; j < service_adaptor_move_file_req_s_type_length; j++) {
		g_variant_unref(req_struct[j]);
	}
}

void __get_move_directory_req_type(GVariant *parameters,
						char **service_name,
						char **src_folder_path,
						char **des_folder_path)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *req_struct[service_adaptor_move_directory_req_s_type_length];

	for (size_t j = 0; j < service_adaptor_move_directory_req_s_type_length; j++) {
		req_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*service_name = ipc_g_variant_dup_string(req_struct[idx++]);
	*src_folder_path = ipc_g_variant_dup_string(req_struct[idx++]);
	*des_folder_path = ipc_g_variant_dup_string(req_struct[idx++]);

	for (size_t j = 0; j < service_adaptor_move_directory_req_s_type_length; j++) {
		g_variant_unref(req_struct[j]);
	}
}

void __get_get_file_list_req_type(GVariant *parameters,
						char **service_name,
						char **parent_path)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *req_struct[service_adaptor_get_file_list_req_s_type_length];

	for (size_t j = 0; j < service_adaptor_get_file_list_req_s_type_length; j++) {
		req_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*service_name	= ipc_g_variant_dup_string(req_struct[idx++]);
	*parent_path	= ipc_g_variant_dup_string(req_struct[idx++]);

	for (size_t j = 0; j < service_adaptor_get_file_list_req_s_type_length; j++) {
		g_variant_unref(req_struct[j]);
	}
}

void __get_open_upload_file_req_type(GVariant *parameters,
						char **service_name,
						char **local_path,
						char **upload_path)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *req_struct[service_adaptor_open_upload_file_req_s_type_length];

	for (size_t j = 0; j < service_adaptor_open_upload_file_req_s_type_length; j++) {
		req_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*service_name	= ipc_g_variant_dup_string(req_struct[idx++]);
	*local_path	= ipc_g_variant_dup_string(req_struct[idx++]);
	*upload_path	= ipc_g_variant_dup_string(req_struct[idx++]);

	for (size_t j = 0; j < service_adaptor_open_upload_file_req_s_type_length; j++) {
		g_variant_unref(req_struct[j]);
	}
}

void __get_open_download_file_req_type(GVariant *parameters,
						char **service_name,
						char **storage_path,
						char **local_path)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *req_struct[service_adaptor_open_download_file_req_s_type_length];

	for (size_t j = 0; j < service_adaptor_open_download_file_req_s_type_length; j++) {
		req_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*service_name	= ipc_g_variant_dup_string(req_struct[idx++]);
	*storage_path	= ipc_g_variant_dup_string(req_struct[idx++]);
	*local_path	= ipc_g_variant_dup_string(req_struct[idx++]);

	for (size_t j = 0; j < service_adaptor_open_download_file_req_s_type_length; j++) {
		g_variant_unref(req_struct[j]);
	}
}

void __get_open_download_thumbnail_req_type(GVariant *parameters,
						char **service_name,
						char **storage_path,
						char **local_path,
						int *thumbnail_size)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *req_struct[service_adaptor_open_download_thumbnail_req_s_type_length];

	for (size_t j = 0; j < service_adaptor_open_download_thumbnail_req_s_type_length; j++) {
		req_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*service_name	= ipc_g_variant_dup_string(req_struct[idx++]);
	*storage_path	= ipc_g_variant_dup_string(req_struct[idx++]);
	*local_path	= ipc_g_variant_dup_string(req_struct[idx++]);
	*thumbnail_size = g_variant_get_int32(req_struct[idx++]);

	for (size_t j = 0; j < service_adaptor_open_download_thumbnail_req_s_type_length; j++) {
		g_variant_unref(req_struct[j]);
	}
}

void __get_close_file_req_type(GVariant *parameters,
						char **service_name,
						long long int *file_uid)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *req_struct[service_adaptor_close_file_req_s_type_length];

	for (size_t j = 0; j < service_adaptor_close_file_req_s_type_length; j++) {
		req_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*service_name = ipc_g_variant_dup_string(req_struct[idx++]);

	do {
		GVariant *fd_req_struct[service_adaptor_file_descriptor_s_type_length];
		for (size_t k = 0; k < service_adaptor_file_descriptor_s_type_length; k++) {
			fd_req_struct[k] = g_variant_get_child_value(req_struct[idx], k);
		}
		int fd_idx = 0;
		*file_uid = (long long int) g_variant_get_int64(fd_req_struct[fd_idx++]);

		for (size_t k = 0; k < service_adaptor_file_descriptor_s_type_length; k++) {
			g_variant_unref(fd_req_struct[k]);
		}
		idx++;
	} while (0);

	for (size_t j = 0; j < service_adaptor_close_file_req_s_type_length; j++) {
		g_variant_unref(req_struct[j]);
	}
}

void __get_start_upload_file_req_type(GVariant *parameters,
						char **service_name,
						long long int *file_uid,
						char **storage_path,
						bool *need_progress,
						bool *need_state)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *req_struct[service_adaptor_start_upload_file_req_s_type_length];

	for (size_t j = 0; j < service_adaptor_start_upload_file_req_s_type_length; j++) {
		req_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*service_name = ipc_g_variant_dup_string(req_struct[idx++]);

	do {
		GVariant *fd_req_struct[service_adaptor_file_descriptor_s_type_length];
		for (size_t k = 0; k < service_adaptor_file_descriptor_s_type_length; k++) {
			fd_req_struct[k] = g_variant_get_child_value(req_struct[idx], k);
		}
		int fd_idx = 0;
		*file_uid = (long long int) g_variant_get_int64(fd_req_struct[fd_idx++]);

		for (size_t k = 0; k < service_adaptor_file_descriptor_s_type_length; k++) {
			g_variant_unref(fd_req_struct[k]);
		}
		idx++;
	} while (0);
	*storage_path = ipc_g_variant_dup_string(req_struct[idx++]);
	*need_progress = g_variant_get_boolean(req_struct[idx++]);
	*need_state = g_variant_get_boolean(req_struct[idx++]);

	for (size_t j = 0; j < service_adaptor_start_upload_file_req_s_type_length; j++) {
		g_variant_unref(req_struct[j]);
	}
}


void __get_start_download_file_req_type(GVariant *parameters,
						char **service_name,
						long long int *file_uid,
						char **storage_path,
						bool *need_progress,
						bool *need_state)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *req_struct[service_adaptor_start_download_file_req_s_type_length];

	for (size_t j = 0; j < service_adaptor_start_download_file_req_s_type_length; j++) {
		req_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*service_name = ipc_g_variant_dup_string(req_struct[idx++]);

	do {
		GVariant *fd_req_struct[service_adaptor_file_descriptor_s_type_length];
		for (size_t k = 0; k < service_adaptor_file_descriptor_s_type_length; k++) {
			fd_req_struct[k] = g_variant_get_child_value(req_struct[idx], k);
		}
		int fd_idx = 0;
		*file_uid = (long long int) g_variant_get_int64(fd_req_struct[fd_idx++]);

		for (size_t k = 0; k < service_adaptor_file_descriptor_s_type_length; k++) {
			g_variant_unref(fd_req_struct[k]);
		}
		idx++;
	} while (0);
	*storage_path = ipc_g_variant_dup_string(req_struct[idx++]);
	*need_progress = g_variant_get_boolean(req_struct[idx++]);
	*need_state = g_variant_get_boolean(req_struct[idx++]);

	for (size_t j = 0; j < service_adaptor_start_download_file_req_s_type_length; j++) {
		g_variant_unref(req_struct[j]);
	}
}


void __get_start_download_thumbnail_req_type(GVariant *parameters,
						char **service_name,
						long long int *file_uid,
						char **storage_path,
						int *thumbnail_size,
						bool *need_progress,
						bool *need_state)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *req_struct[service_adaptor_start_download_thumbnail_req_s_type_length];

	for (size_t j = 0; j < service_adaptor_start_download_thumbnail_req_s_type_length; j++) {
		req_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*service_name = ipc_g_variant_dup_string(req_struct[idx++]);

	do {
		GVariant *fd_req_struct[service_adaptor_file_descriptor_s_type_length];
		for (size_t k = 0; k < service_adaptor_file_descriptor_s_type_length; k++) {
			fd_req_struct[k] = g_variant_get_child_value(req_struct[idx], k);
		}
		int fd_idx = 0;
		*file_uid = (long long int) g_variant_get_int64(fd_req_struct[fd_idx++]);

		for (size_t k = 0; k < service_adaptor_file_descriptor_s_type_length; k++) {
			g_variant_unref(fd_req_struct[k]);
		}
		idx++;
	} while (0);
	*storage_path = ipc_g_variant_dup_string(req_struct[idx++]);
	*thumbnail_size = (int) g_variant_get_int32(req_struct[idx++]);
	*need_progress = g_variant_get_boolean(req_struct[idx++]);
	*need_state = g_variant_get_boolean(req_struct[idx++]);

	for (size_t j = 0; j < service_adaptor_start_download_thumbnail_req_s_type_length; j++) {
		g_variant_unref(req_struct[j]);
	}
}

void __get_cancel_file_task_req_type(GVariant *parameters,
						char **service_name,
						long long int *file_uid)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *req_struct[service_adaptor_cancel_file_task_req_s_type_length];

	for (size_t j = 0; j < service_adaptor_cancel_file_task_req_s_type_length; j++) {
		req_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*service_name = ipc_g_variant_dup_string(req_struct[idx++]);

	do {
		GVariant *fd_req_struct[service_adaptor_file_descriptor_s_type_length];
		for (size_t k = 0; k < service_adaptor_file_descriptor_s_type_length; k++) {
			fd_req_struct[k] = g_variant_get_child_value(req_struct[idx], k);
		}
		int fd_idx = 0;
		*file_uid = (long long int) g_variant_get_int64(fd_req_struct[fd_idx++]);

		for (size_t k = 0; k < service_adaptor_file_descriptor_s_type_length; k++) {
			g_variant_unref(fd_req_struct[k]);
		}
		idx++;
	} while (0);

	for (size_t j = 0; j < service_adaptor_cancel_file_task_req_s_type_length; j++) {
		g_variant_unref(req_struct[j]);
	}
}

GVariant *__create_file_descriptor_res_type(long long int file_uid,
						storage_adaptor_error_code_h error_code)
{
	storage_adaptor_error_code_t _error;
	_error.msg = NULL;

	if (NULL == error_code) {
		error_code = &_error;
		error_code->code = STORAGE_ADAPTOR_ERROR_NONE;
		error_code->msg = strdup("");
	}

	GVariant *response = g_variant_new(MAKE_RETURN_TYPE(service_adaptor_file_descriptor_s_type),
			(int64_t) file_uid,
			(uint64_t) error_code->code, __safe_add_string(error_code->msg));

	free(_error.msg);
	return response;
}



GVariant *__create_get_root_folder_path_res_type(const char *root_folder_path,
						storage_adaptor_error_code_h error_code)
{
	storage_adaptor_error_code_t _error;
	_error.msg = NULL;

	if (NULL == error_code) {
		error_code = &_error;
		error_code->code = STORAGE_ADAPTOR_ERROR_NONE;
		error_code->msg = strdup("");
	}

	GVariant *response = g_variant_new(MAKE_RETURN_TYPE(service_adaptor_get_root_folder_path_res_s_type),
			__safe_add_string(root_folder_path),
			(uint64_t) error_code->code, __safe_add_string(error_code->msg));

	free(_error.msg);
	return response;
}

GVariant *__create_file_info_res_type(storage_adaptor_file_info_h file_info,
						storage_adaptor_error_code_h error_code)
{
	storage_adaptor_error_code_h _error_code = error_code;
	storage_adaptor_error_code_t dummy_error_code;
	dummy_error_code.code = 0LL;
	dummy_error_code.msg = "";

	if (NULL == _error_code) {
		_error_code = &dummy_error_code;
	}

	storage_adaptor_file_info_h _file_info = file_info;
	storage_adaptor_file_info_t dummy_file_info;
	storage_adaptor_media_meta_s _media_meta;
	storage_adaptor_cloud_meta_s _cloud_meta;

	FILE_INFO_CLEAR(&dummy_file_info);
	MEDIA_META_CLEAR(&_media_meta);
	CLOUD_META_CLEAR(&_cloud_meta);

	dummy_file_info.media_meta          = &_media_meta;
	dummy_file_info.cloud_meta          = &_cloud_meta;

	if (NULL == _file_info) {
		_file_info = &dummy_file_info;
	}

	if (NULL == _file_info->media_meta) {
		_file_info->media_meta = dummy_file_info.media_meta;
	}

	if (NULL == _file_info->cloud_meta) {
		_file_info->cloud_meta = dummy_file_info.cloud_meta;
	}

	GVariant *return_val = NULL;
	return_val = g_variant_new(MAKE_RETURN_TYPE(service_adaptor_file_info_s_type),
			/* file_info default */
			__safe_add_string(_file_info->plugin_uri),
			__safe_add_string(_file_info->object_id),
			__safe_add_string(_file_info->storage_path),
			(uint64_t) _file_info->file_size,
			(uint64_t) _file_info->created_time,
			(uint64_t) _file_info->modified_time,
			(int32_t) _file_info->file_info_index,
			(int32_t) _file_info->content_type,

			/* media_meta */
			__safe_add_string(_file_info->media_meta->mime_type),
			__safe_add_string(_file_info->media_meta->title),
			__safe_add_string(_file_info->media_meta->album),
			__safe_add_string(_file_info->media_meta->artist),
			__safe_add_string(_file_info->media_meta->genere),
			__safe_add_string(_file_info->media_meta->recorded_date),
			(int32_t) _file_info->media_meta->width,
			(int32_t) _file_info->media_meta->height,
			(int32_t) _file_info->media_meta->duration,
			__safe_add_string(_file_info->media_meta->copyright),
			__safe_add_string(_file_info->media_meta->track_num),
			__safe_add_string(_file_info->media_meta->description),
			__safe_add_string(_file_info->media_meta->composer),
			__safe_add_string(_file_info->media_meta->year),
			(int32_t) _file_info->media_meta->bitrate,
			(int32_t) _file_info->media_meta->samplerate,
			(int32_t) _file_info->media_meta->channel,
			__safe_add_string(_file_info->media_meta->extra_media_meta),

			/* cloud_meta */
			__safe_add_string(_file_info->cloud_meta->service_name),
			(uint64_t) _file_info->cloud_meta->usage_byte,
			(uint64_t) _file_info->cloud_meta->quota_byte,
			__safe_add_string(_file_info->cloud_meta->extra_cloud_meta),

			__safe_add_string(_file_info->extra_file_info),

			(uint64_t) _error_code->code, __safe_add_string(_error_code->msg));

	return return_val;
}

GVariant *__create_get_file_list_res_type(storage_adaptor_file_info_h *file_info_list,
						unsigned int file_info_len,
						storage_adaptor_error_code_h error_code)
{
	storage_adaptor_error_code_t _error;
	_error.msg = NULL;

	if (NULL == error_code) {
		error_code = &_error;
		error_code->code = STORAGE_ADAPTOR_ERROR_NONE;
		error_code->msg = strdup("");
	}

	if (NULL == file_info_list) {
		file_info_len = 0;
	}

	GVariantBuilder *builder_file_info = g_variant_builder_new(G_VARIANT_TYPE(storage_file_info_list_type));

	storage_adaptor_file_info_h _file_info = NULL;
	storage_adaptor_file_info_h dummy_file_info = storage_adaptor_create_file_info();

	for (gsize i = 0; i < file_info_len; i++) {
		_file_info = file_info_list[i];
		if (NULL == _file_info) {
			_file_info = dummy_file_info;
		}

		if (NULL == _file_info->media_meta) {
			_file_info->media_meta = dummy_file_info->media_meta;
		}

		if (NULL == _file_info->cloud_meta) {
			_file_info->cloud_meta = dummy_file_info->cloud_meta;
		}

		g_variant_builder_open(builder_file_info, G_VARIANT_TYPE(service_adaptor_file_info_s_type));

		/* file_info default */
		g_variant_builder_add(builder_file_info, "s", __safe_add_string(_file_info->plugin_uri));
		g_variant_builder_add(builder_file_info, "s", __safe_add_string(_file_info->object_id));
		g_variant_builder_add(builder_file_info, "s", __safe_add_string(_file_info->storage_path));
		g_variant_builder_add(builder_file_info, "t", (uint64_t) _file_info->file_size);
		g_variant_builder_add(builder_file_info, "t", (uint64_t) _file_info->created_time);
		g_variant_builder_add(builder_file_info, "t", (uint64_t) _file_info->modified_time);
		g_variant_builder_add(builder_file_info, "i", (int32_t) _file_info->file_info_index);
		g_variant_builder_add(builder_file_info, "i", (int32_t) _file_info->content_type);

		/* media_meta */

		g_variant_builder_add_value(builder_file_info, g_variant_new(service_adaptor_content_meta_s_type,
				__safe_add_string(_file_info->media_meta->mime_type),
				__safe_add_string(_file_info->media_meta->title),
				__safe_add_string(_file_info->media_meta->album),
				__safe_add_string(_file_info->media_meta->artist),
				__safe_add_string(_file_info->media_meta->genere),
				__safe_add_string(_file_info->media_meta->recorded_date),
				(int32_t) _file_info->media_meta->width,
				(int32_t) _file_info->media_meta->height,
				(int32_t) _file_info->media_meta->duration,
				__safe_add_string(_file_info->media_meta->copyright),
				__safe_add_string(_file_info->media_meta->track_num),
				__safe_add_string(_file_info->media_meta->description),
				__safe_add_string(_file_info->media_meta->composer),
				__safe_add_string(_file_info->media_meta->year),
				(int32_t) _file_info->media_meta->bitrate,
				(int32_t) _file_info->media_meta->samplerate,
				(int32_t) _file_info->media_meta->channel,
				__safe_add_string(_file_info->media_meta->extra_media_meta)));

		/* cloud_meta */
		g_variant_builder_add_value(builder_file_info, g_variant_new(service_adaptor_cloud_meta_s_type,
				__safe_add_string(_file_info->cloud_meta->service_name),
				(uint64_t) _file_info->cloud_meta->usage_byte,
				(uint64_t) _file_info->cloud_meta->quota_byte));

		g_variant_builder_add(builder_file_info, "s", __safe_add_string(_file_info->extra_file_info));

		g_variant_builder_close(builder_file_info);
	}
	storage_adaptor_destroy_file_info(&dummy_file_info);

	GVariant *response = g_variant_new(MAKE_RETURN_TYPE(service_adaptor_get_file_list_res_s_type),
			builder_file_info, (uint32_t)file_info_len,
			(uint64_t) error_code->code, __safe_add_string(error_code->msg));

	free(_error.msg);
	return response;
}

void __separate_path_to_dir_base(char *full_path,
						char **dir_path,
						char **base_path)
{
	if ((NULL == full_path) || (0 >= strlen(full_path))) {
		*base_path = strdup("");
		*dir_path = strdup("");
		return;
	}

	char *base = strrchr(full_path, '/');
	if (NULL == base) {
		*base_path = strdup(full_path);
		*dir_path = strdup("");
	} else if (0 == strcmp(full_path, base)) {
		*base_path = strdup(full_path);
		*dir_path = strdup("");
	} else {
		*base_path = strdup(base + 1);
		if ((base - full_path) > 1) {
			*dir_path = strndup(full_path, (base - full_path));
		} else {
			*dir_path = strdup("");
		}
	}
}

void storage_adaptor_method_call(GDBusConnection *connection,
						const gchar *sender,
						const gchar *object_path,
						const gchar *interface_name,
						const gchar *method_name,
						GVariant *parameters,
						GDBusMethodInvocation *invocation,
						gpointer user_data)
{
	service_adaptor_internal_error_code_e ret_code = SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;

	service_adaptor_debug("Received %s() call", method_name);

	/* private feature */
	if (0 == g_strcmp0(method_name, PRIVATE_DBUS_DOWNLOAD_FILE_METHOD)) {
		char *service_name = NULL;
		char *server_path = NULL;
		char *download_path = NULL;
		storage_adaptor_error_code_h error_code = NULL;
		storage_adaptor_error_code_t _error;
		_error.msg = NULL;

		__get_file_transfer_req_type(parameters, &service_name, &server_path, &download_path);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			error_code = &_error;

			service_adaptor_error("Can not get service context: %s", service_name);
			error_code->code = STORAGE_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = strdup("Can not get service context");

			g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)",
					(uint64_t) error_code->code, __safe_add_string(error_code->msg)));

			free(_error.msg);

			free(service_name);
			free(server_path);
			free(download_path);
			return;
		}

		storage_adaptor_h adaptor = service_adaptor_get_storage_adaptor(service_adaptor);
		storage_adaptor_plugin_h plugin = NULL;

		if (NULL != service->storage_context) {
			plugin = storage_adaptor_get_plugin_by_name(adaptor, service->storage_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			char *parent_folder = NULL;
			char *file_name = NULL;
			__separate_path_to_dir_base(server_path, &parent_folder, &file_name);

			ret_code = storage_adaptor_download_file_sync(plugin, service->storage_context,
					parent_folder, file_name, download_path, NULL, &error_code, NULL);

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->storage_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%lld: %s)", error_code ? error_code->code : 0ULL, error_code ? error_code->msg : NULL);
				storage_adaptor_destroy_error_code(&error_code);

				service_adaptor_debug("Re-try API");
				ret_code = storage_adaptor_download_file_sync(plugin, service->storage_context,
						parent_folder, file_name, download_path, NULL, &error_code, NULL);
			}

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NONE != ret_code) {
				service_adaptor_error("Can not run storage_adaptor_download_file_sync()");
			}
			free(parent_folder);
			free(file_name);
		}

		__check_error_code();

		g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)",
				(uint64_t) error_code->code, __safe_add_string(error_code->msg)));

		if (error_code != &_error) {
			free(error_code->msg);
			free(error_code);
			error_code = NULL;
		} else {
			free(_error.msg);
		}

		free(service_name);
		free(server_path);
		free(download_path);
	} else if (0 == g_strcmp0(method_name, PRIVATE_DBUS_DOWNLOAD_FILE_ASYNC_METHOD)) {
		char *service_name = NULL;
		char *server_path = NULL;
		char *download_path = NULL;
		int fd = 0;
		storage_adaptor_error_code_h error_code = NULL;
		storage_adaptor_error_code_t _error;
		_error.msg = NULL;

		__get_file_transfer_req_type(parameters, &service_name, &server_path, &download_path);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = STORAGE_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = strdup("Can not get service context");

			GVariant *response = __private_create_file_transfer_res_type(fd, error_code);
			g_dbus_method_invocation_return_value(invocation, response);

			free(_error.msg);
			free(service_name);
			free(server_path);
			free(download_path);
			return;
		}

		storage_adaptor_h adaptor = service_adaptor_get_storage_adaptor(service_adaptor);
		storage_adaptor_plugin_h plugin = NULL;

		if (NULL != service->storage_context) {
			plugin = storage_adaptor_get_plugin_by_name(adaptor, service->storage_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			char *parent_folder = NULL;
			char *file_name = NULL;
			__separate_path_to_dir_base(server_path, &parent_folder, &file_name);

			ret_code = storage_adaptor_download_file_async(plugin, service->storage_context,
					parent_folder, file_name, download_path, NULL, (void *) &fd, &error_code);

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->storage_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%lld: %s)", error_code ? error_code->code : 0ULL, error_code ? error_code->msg : NULL);
				storage_adaptor_destroy_error_code(&error_code);

				service_adaptor_debug("Re-try API");
				ret_code = storage_adaptor_download_file_async(plugin, service->storage_context,
						parent_folder, file_name, download_path, NULL, (void *) &fd, &error_code);
			}

			free(parent_folder);
			free(file_name);
		}

		__check_error_code();

		GVariant *response = __private_create_file_transfer_res_type(fd, error_code);
		g_dbus_method_invocation_return_value(invocation, response);

		if (error_code != &_error) {
			free(error_code->msg);
			free(error_code);
			error_code = NULL;
		} else {
			free(_error.msg);
		}
		free(service_name);
		free(server_path);
		free(download_path);
	} else if (0 == g_strcmp0(method_name, PRIVATE_DBUS_UPLOAD_FILE_METHOD)) {
		char *service_name = NULL;
		char *upload_path = NULL;
		char *server_path = NULL;
		storage_adaptor_error_code_h error_code = NULL;
		storage_adaptor_error_code_t _error;
		_error.msg = NULL;

		__get_file_transfer_req_type(parameters, &service_name, &upload_path, &server_path);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = STORAGE_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = strdup("Can not get service context");

			g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)",
					(uint64_t) error_code->code, __safe_add_string(error_code->msg)));

			free(_error.msg);
			free(service_name);
			free(server_path);
			free(upload_path);
			return;
		}

		storage_adaptor_h adaptor = service_adaptor_get_storage_adaptor(service_adaptor);
		storage_adaptor_plugin_h plugin = NULL;

		if (NULL != service->storage_context) {
			plugin = storage_adaptor_get_plugin_by_name(adaptor, service->storage_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			storage_adaptor_file_info_h file_info = NULL;
			char *parent_folder = NULL;
			char *file_name = NULL;
			__separate_path_to_dir_base(server_path, &parent_folder, &file_name);

			ret_code = storage_adaptor_upload_file_sync(plugin, service->storage_context,
					parent_folder, file_name, upload_path, false, NULL, &file_info, &error_code, NULL);

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->storage_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%lld: %s)", error_code ? error_code->code : 0ULL, error_code ? error_code->msg : NULL);
				storage_adaptor_destroy_error_code(&error_code);

				service_adaptor_debug("Re-try API");
				ret_code = storage_adaptor_upload_file_sync(plugin, service->storage_context,
						parent_folder, file_name, upload_path, false, NULL, &file_info, &error_code, NULL);
			}
			free(parent_folder);
			free(file_name);
		}

		__check_error_code();

		g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)",
				(uint64_t) error_code->code, __safe_add_string(error_code->msg)));

		if (error_code != &_error) {
			free(error_code->msg);
			free(error_code);
			error_code = NULL;
		} else {
			free(_error.msg);
		}
		free(service_name);
		free(server_path);
		free(upload_path);
	} else if (0 == g_strcmp0(method_name, PRIVATE_DBUS_UPLOAD_FILE_ASYNC_METHOD)) {
		char *service_name = NULL;
		char *server_path = NULL;
		char *upload_path = NULL;
		int fd = 0;
		storage_adaptor_error_code_h error_code = NULL;
		storage_adaptor_error_code_t _error;
		_error.msg = NULL;

		__get_file_transfer_req_type(parameters, &service_name, &upload_path, &server_path);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = STORAGE_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = strdup("Can not get service context");

			GVariant *response = __private_create_file_transfer_res_type(fd, error_code);
			g_dbus_method_invocation_return_value(invocation, response);

			free(_error.msg);
			free(service_name);
			free(server_path);
			free(upload_path);
			return;
		}

		storage_adaptor_h adaptor = service_adaptor_get_storage_adaptor(service_adaptor);
		storage_adaptor_plugin_h plugin = NULL;

		if (NULL != service->storage_context) {
			plugin = storage_adaptor_get_plugin_by_name(adaptor, service->storage_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			char *parent_folder = NULL;
			char *file_name = NULL;
			__separate_path_to_dir_base(server_path, &parent_folder, &file_name);

			ret_code = storage_adaptor_upload_file_async(plugin, service->storage_context,
					parent_folder, file_name, upload_path, false, NULL, (void *) &fd, &error_code);

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->storage_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%lld: %s)", error_code ? error_code->code : 0ULL, error_code ? error_code->msg : NULL);
				storage_adaptor_destroy_error_code(&error_code);

				service_adaptor_debug("Re-try API");
				ret_code = storage_adaptor_upload_file_async(plugin, service->storage_context,
						parent_folder, file_name, upload_path, false, NULL, (void *) &fd, &error_code);
			}
			free(parent_folder);
			free(file_name);
		}

		__check_error_code();

		GVariant *response = __private_create_file_transfer_res_type(fd, error_code);
		g_dbus_method_invocation_return_value(invocation, response);

		if (error_code != &_error) {
			free(error_code->msg);
			free(error_code);
			error_code = NULL;
		} else {
			free(_error.msg);
		}
		free(service_name);
		free(server_path);
		free(upload_path);
	} else if (0 == g_strcmp0(method_name, PRIVATE_DBUS_DOWNLOAD_FILE_PUBLISH_METHOD)) {
		char *service_name = NULL;
		char *server_path = NULL;
		char *download_path = NULL;
		storage_adaptor_error_code_h error_code = NULL;
		storage_adaptor_error_code_t _error;
		_error.msg = NULL;

		__get_file_transfer_req_type(parameters, &service_name, &server_path, &download_path);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = STORAGE_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = strdup("Can not get service context");

			g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)",
					(uint64_t) error_code->code, __safe_add_string(error_code->msg)));

			free(_error.msg);
			free(service_name);
			free(server_path);
			free(download_path);
			return;
		}

		storage_adaptor_h adaptor = service_adaptor_get_storage_adaptor(service_adaptor);
		storage_adaptor_plugin_h plugin = NULL;

		if (NULL != service->storage_context) {
			plugin = storage_adaptor_get_plugin_by_name(adaptor, service->storage_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			char public_token[1024] = {0,};
			char auth_code[1024] = {0,};

			sscanf(server_path, "%[^'?']?auth_code=%s", public_token, auth_code);
			ret_code = storage_adaptor_download_file_sync_by_public_token(plugin, service->storage_context,
					public_token, auth_code, download_path, NULL, &error_code, NULL);

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->storage_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%lld: %s)", error_code ? error_code->code : 0ULL, error_code ? error_code->msg : NULL);
				storage_adaptor_destroy_error_code(&error_code);

				service_adaptor_debug("Re-try API");
				ret_code = storage_adaptor_download_file_sync_by_public_token(plugin, service->storage_context,
						public_token, auth_code, download_path, NULL, &error_code, NULL);
			}
		}

		__check_error_code();

		g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)",
				(uint64_t) error_code->code, __safe_add_string(error_code->msg)));

		if (error_code != &_error) {
			free(error_code->msg);
			free(error_code);
			error_code = NULL;
		} else {
			free(_error.msg);
		}
		free(service_name);
		free(server_path);
		free(download_path);
	} else if (0 == g_strcmp0(method_name, PRIVATE_DBUS_DOWNLOAD_FILE_PUBLISH_ASYNC_METHOD)) {
		char *service_name = NULL;
		char *server_path = NULL;
		char *download_path = NULL;
		int fd = 0;
		storage_adaptor_error_code_h error_code = NULL;
		storage_adaptor_error_code_t _error;
		_error.msg = NULL;

		__get_file_transfer_req_type(parameters, &service_name, &server_path, &download_path);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = STORAGE_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = strdup("Can not get service context");

			GVariant *response = __private_create_file_transfer_res_type(fd, error_code);
			g_dbus_method_invocation_return_value(invocation, response);

			free(_error.msg);
			free(service_name);
			free(server_path);
			free(download_path);
			return;
		}

		storage_adaptor_h adaptor = service_adaptor_get_storage_adaptor(service_adaptor);
		storage_adaptor_plugin_h plugin = NULL;

		if (NULL != service->storage_context) {
			plugin = storage_adaptor_get_plugin_by_name(adaptor, service->storage_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			char public_token[1024] = {0,};
			char auth_code[1024] = {0,};

			sscanf(server_path, "%[^'?']?auth_code=%s", public_token, auth_code);

			ret_code = storage_adaptor_download_file_async_by_public_token(plugin, service->storage_context,
					public_token, auth_code, download_path, NULL, (void *) &fd, &error_code);

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->storage_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%lld: %s)", error_code ? error_code->code : 0ULL, error_code ? error_code->msg : NULL);
				storage_adaptor_destroy_error_code(&error_code);

				service_adaptor_debug("Re-try API");
				ret_code = storage_adaptor_download_file_async_by_public_token(plugin, service->storage_context,
						public_token, auth_code, download_path, NULL, (void *) &fd, &error_code);
			}
		}

		__check_error_code();

		GVariant *response = __private_create_file_transfer_res_type(fd, error_code);
		g_dbus_method_invocation_return_value(invocation, response);

		if (error_code != &_error) {
			free(error_code->msg);
			free(error_code);
			error_code = NULL;
		} else {
			free(_error.msg);
		}
		free(service_name);
		free(server_path);
		free(download_path);
	} else if (0 == g_strcmp0(method_name, PRIVATE_DBUS_UPLOAD_FILE_PUBLISH_METHOD)) {
		char *service_name = NULL;
		char *upload_path = NULL;
		char *server_path = NULL;
		storage_adaptor_file_info_h file_info = NULL;
		storage_adaptor_error_code_h error_code = NULL;
		storage_adaptor_error_code_t _error;
		_error.msg = NULL;

		__get_file_transfer_req_type(parameters, &service_name, &upload_path, &server_path);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = STORAGE_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = strdup("Can not get service context");

			GVariant *response = __private_create_file_publish_transfer_res_type(file_info, error_code);
			g_dbus_method_invocation_return_value(invocation, response);

			free(_error.msg);
			free(service_name);
			free(server_path);
			free(upload_path);
			return;
		}

		storage_adaptor_h adaptor = service_adaptor_get_storage_adaptor(service_adaptor);
		storage_adaptor_plugin_h plugin = NULL;

		if (NULL != service->storage_context) {
			plugin = storage_adaptor_get_plugin_by_name(adaptor, service->storage_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			char *parent_folder = NULL;
			char *file_name = NULL;
			__separate_path_to_dir_base(server_path, &parent_folder, &file_name);

			ret_code = storage_adaptor_upload_file_sync(plugin, service->storage_context,
					parent_folder, file_name, upload_path, true, NULL, &file_info, &error_code, NULL);

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->storage_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%lld: %s)", error_code ? error_code->code : 0ULL, error_code ? error_code->msg : NULL);
				storage_adaptor_destroy_error_code(&error_code);

				service_adaptor_debug("Re-try API");
				storage_adaptor_destroy_file_info(&file_info);
				ret_code = storage_adaptor_upload_file_sync(plugin, service->storage_context,
						parent_folder, file_name, upload_path, true, NULL, &file_info, &error_code, NULL);
			}
			free(parent_folder);
			free(file_name);
		}

		__check_error_code();

		GVariant *response = __private_create_file_publish_transfer_res_type(file_info, error_code);
		g_dbus_method_invocation_return_value(invocation, response);
		storage_adaptor_destroy_file_info(&file_info);

		if (error_code != &_error) {
			free(error_code->msg);
			free(error_code);
			error_code = NULL;
		} else {
			free(_error.msg);
		}
		free(service_name);
		free(server_path);
		free(upload_path);
	} else if (0 == g_strcmp0(method_name, PRIVATE_DBUS_UPLOAD_FILE_PUBLISH_ASYNC_METHOD)) {
		char *service_name = NULL;
		char *server_path = NULL;
		char *upload_path = NULL;
		int fd = 0;
		storage_adaptor_error_code_h error_code = NULL;
		storage_adaptor_error_code_t _error;
		_error.msg = NULL;

		__get_file_transfer_req_type(parameters, &service_name, &upload_path, &server_path);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = STORAGE_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = strdup("Can not get service context");

			GVariant *response = __private_create_file_transfer_res_type(fd, error_code);
			g_dbus_method_invocation_return_value(invocation, response);

			free(_error.msg);
			free(service_name);
			free(server_path);
			free(upload_path);
			return;
		}

		storage_adaptor_h adaptor = service_adaptor_get_storage_adaptor(service_adaptor);
		storage_adaptor_plugin_h plugin = NULL;

		if (NULL != service->storage_context) {
			plugin = storage_adaptor_get_plugin_by_name(adaptor, service->storage_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			char *parent_folder = NULL;
			char *file_name = NULL;
			__separate_path_to_dir_base(server_path, &parent_folder, &file_name);

			ret_code = storage_adaptor_upload_file_async(plugin, service->storage_context,
					parent_folder, file_name, upload_path, true, NULL, (void *) &fd, &error_code);

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->storage_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%lld: %s)", error_code ? error_code->code : 0ULL, error_code ? error_code->msg : NULL);
				storage_adaptor_destroy_error_code(&error_code);

				service_adaptor_debug("Re-try API");
				ret_code = storage_adaptor_upload_file_async(plugin, service->storage_context,
						parent_folder, file_name, upload_path, true, NULL, (void *) &fd, &error_code);
			}
			free(parent_folder);
			free(file_name);
		}

		__check_error_code();

		GVariant *response = __private_create_file_transfer_res_type(fd, error_code);
		g_dbus_method_invocation_return_value(invocation, response);

		if (error_code != &_error) {
			free(error_code->msg);
			free(error_code);
			error_code = NULL;
		} else {
			free(_error.msg);
		}
		free(service_name);
		free(server_path);
		free(upload_path);
	} else if (0 == g_strcmp0(method_name, PRIVATE_DBUS_GET_FILE_STATUS_METHOD)) {
		char *service_name = NULL;
		int fd = 0;
		uint64_t progress_size = 0;
		uint64_t total_size = 0;
		storage_adaptor_transfer_state_e state = STORAGE_ADAPTOR_TRANSFER_STATUS_FINISHED;
		storage_adaptor_error_code_h error_code = NULL;
		storage_adaptor_error_code_t _error;
		_error.msg = NULL;

		__private_get_file_status_req_type(parameters, &service_name, &fd);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = STORAGE_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = strdup("Can not get service context");

			GVariant *response = __private_create_file_status_res_type((int64_t) progress_size, (int64_t) total_size, state, error_code);
			g_dbus_method_invocation_return_value(invocation, response);

			free(_error.msg);
			free(service_name);
			return;
		}

		storage_adaptor_h adaptor = service_adaptor_get_storage_adaptor(service_adaptor);
		storage_adaptor_plugin_h plugin = NULL;

		if (NULL != service->storage_context) {
			plugin = storage_adaptor_get_plugin_by_name(adaptor, service->storage_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			ret_code = storage_adaptor_get_transfer_state(plugin, service->storage_context,
					(void *) fd, NULL, &state, &error_code, NULL);

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->storage_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%lld: %s)", error_code ? error_code->code : 0ULL, error_code ? error_code->msg : NULL);
				storage_adaptor_destroy_error_code(&error_code);

				service_adaptor_debug("Re-try API");
				ret_code = storage_adaptor_get_transfer_state(plugin, service->storage_context,
						(void *) fd, NULL, &state, &error_code, NULL);
			}
			ret_code = storage_adaptor_get_transfer_progress(plugin, service->storage_context,
					(void *) fd, NULL, &progress_size, &total_size, &error_code, NULL);
		}

		__check_error_code();

		GVariant *response = __private_create_file_status_res_type((int64_t) progress_size, (int64_t) total_size, state, error_code);
		g_dbus_method_invocation_return_value(invocation, response);

		if (error_code != &_error) {
			free(error_code->msg);
			free(error_code);
			error_code = NULL;
		} else {
			free(_error.msg);
		}
		free(service_name);
	} else if (0 == g_strcmp0(method_name, PRIVATE_DBUS_CANCEL_FILE_TRANSFER_METHOD)) {
		char *service_name = NULL;
		int fd = 0;
		storage_adaptor_error_code_h error_code = NULL;
		storage_adaptor_error_code_t _error;
		_error.msg = NULL;

		__private_get_file_status_req_type(parameters, &service_name, &fd);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = STORAGE_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = strdup("Can not get service context");

			g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)",
					(uint64_t) error_code->code, __safe_add_string(error_code->msg)));

			free(_error.msg);
			free(service_name);
			return;
		}

		storage_adaptor_h adaptor = service_adaptor_get_storage_adaptor(service_adaptor);
		storage_adaptor_plugin_h plugin = NULL;

		if (NULL != service->storage_context) {
			plugin = storage_adaptor_get_plugin_by_name(adaptor, service->storage_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			storage_adaptor_transfer_state_e state = STORAGE_ADAPTOR_TRANSFER_STATUS_CANCEL;

			ret_code = storage_adaptor_set_transfer_state(plugin, service->storage_context,
					(void *) fd, state, NULL, &error_code, NULL);

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->storage_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%lld: %s)", error_code ? error_code->code : 0ULL, error_code ? error_code->msg : NULL);
				storage_adaptor_destroy_error_code(&error_code);

				service_adaptor_debug("Re-try API");
				ret_code = storage_adaptor_set_transfer_state(plugin, service->storage_context,
						(void *) fd, state, NULL, &error_code, NULL);
			}
		}

		__check_error_code();

		g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)",
				(uint64_t) error_code->code, __safe_add_string(error_code->msg)));

		if (error_code != &_error) {
			free(error_code->msg);
			free(error_code);
			error_code = NULL;
		} else {
			free(_error.msg);
		}
		free(service_name);
	} else if (0 == g_strcmp0(method_name, PRIVATE_DBUS_PAUSE_FILE_TRANSFER_METHOD)) {
		char *service_name = NULL;
		int fd = 0;
		storage_adaptor_error_code_h error_code = NULL;
		storage_adaptor_error_code_t _error;
		_error.msg = NULL;

		__private_get_file_status_req_type(parameters, &service_name, &fd);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = STORAGE_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = strdup("Can not get service context");

			g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)",
					(uint64_t) error_code->code, __safe_add_string(error_code->msg)));

			free(_error.msg);
			free(service_name);
			return;
		}

		storage_adaptor_h adaptor = service_adaptor_get_storage_adaptor(service_adaptor);
		storage_adaptor_plugin_h plugin = NULL;

		if (NULL != service->storage_context) {
			plugin = storage_adaptor_get_plugin_by_name(adaptor, service->storage_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			storage_adaptor_transfer_state_e state = STORAGE_ADAPTOR_TRANSFER_STATUS_PAUSE;
			ret_code = storage_adaptor_set_transfer_state(plugin, service->storage_context,
					(void *) fd, state, NULL, &error_code, NULL);

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->storage_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%lld: %s)", error_code ? error_code->code : 0ULL, error_code ? error_code->msg : NULL);
				storage_adaptor_destroy_error_code(&error_code);

				service_adaptor_debug("Re-try API");
				ret_code = storage_adaptor_set_transfer_state(plugin, service->storage_context,
						(void *) fd, state, NULL, &error_code, NULL);
			}
		}

		__check_error_code();

		g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)",
				(uint64_t) error_code->code, __safe_add_string(error_code->msg)));

		if (error_code != &_error) {
			free(error_code->msg);
			free(error_code);
			error_code = NULL;
		} else {
			free(_error.msg);
		}
		free(service_name);
	} else if (0 == g_strcmp0(method_name, PRIVATE_DBUS_RESUME_FILE_TRANSFER_METHOD)) {
		char *service_name = NULL;
		int fd = 0;
		storage_adaptor_error_code_h error_code = NULL;
		storage_adaptor_error_code_t _error;
		_error.msg = NULL;

		__private_get_file_status_req_type(parameters, &service_name, &fd);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = STORAGE_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = strdup("Can not get service context");

			g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)",
					(uint64_t) error_code->code, __safe_add_string(error_code->msg)));

			free(_error.msg);
			free(service_name);
			return;
		}

		storage_adaptor_h adaptor = service_adaptor_get_storage_adaptor(service_adaptor);
		storage_adaptor_plugin_h plugin = NULL;

		if (NULL != service->storage_context) {
			plugin = storage_adaptor_get_plugin_by_name(adaptor, service->storage_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			storage_adaptor_transfer_state_e state = STORAGE_ADAPTOR_TRANSFER_STATUS_RESUME;
			ret_code = storage_adaptor_set_transfer_state(plugin, service->storage_context,
					(void *) fd, state, NULL, &error_code, NULL);

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->storage_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%lld: %s)", error_code ? error_code->code : 0ULL, error_code ? error_code->msg : NULL);
				storage_adaptor_destroy_error_code(&error_code);

				service_adaptor_debug("Re-try API");
				ret_code = storage_adaptor_set_transfer_state(plugin, service->storage_context,
						(void *) fd, state, NULL, &error_code, NULL);
			}
		}

		__check_error_code();

		g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)",
				(uint64_t) error_code->code, __safe_add_string(error_code->msg)));

		if (error_code != &_error) {
			free(error_code->msg);
			free(error_code);
			error_code = NULL;
		} else {
			free(_error.msg);
		}
		free(service_name);
	} else if (0 == g_strcmp0(method_name, DBUS_DOWNLOAD_FILE_METHOD)) { /* public feature */
		char *service_name = NULL;
		char *server_path = NULL;
		char *download_path = NULL;
		storage_adaptor_error_code_h error_code = NULL;
		storage_adaptor_error_code_t _error;
		_error.msg = NULL;

		__get_file_transfer_req_type(parameters, &service_name, &server_path, &download_path);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = STORAGE_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = strdup("Can not get service context");

			g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)",
					(uint64_t) error_code->code, __safe_add_string(error_code->msg)));

			free(_error.msg);
			free(service_name);
			free(server_path);
			free(download_path);
			return;
		}

		storage_adaptor_h adaptor = service_adaptor_get_storage_adaptor(service_adaptor);
		storage_adaptor_plugin_h plugin = NULL;

		if (NULL != service->storage_context) {
			plugin = storage_adaptor_get_plugin_by_name(adaptor, service->storage_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			char *parent_folder = NULL;
			char *file_name = NULL;
			__separate_path_to_dir_base(server_path, &parent_folder, &file_name);

			ret_code = storage_adaptor_download_file_sync(plugin, service->storage_context,
					parent_folder, file_name, download_path, NULL, &error_code, NULL);

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->storage_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%lld: %s)", error_code ? error_code->code : 0ULL, error_code ? error_code->msg : NULL);
				storage_adaptor_destroy_error_code(&error_code);

				service_adaptor_debug("Re-try API");
				ret_code = storage_adaptor_download_file_sync(plugin, service->storage_context,
						parent_folder, file_name, download_path, NULL, &error_code, NULL);
			}

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NONE != ret_code) {
				service_adaptor_error("Can not run storage_adaptor_download_file_sync()");
			}
			free(parent_folder);
			free(file_name);
		}

		__check_error_code();

		g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)",
				(uint64_t) error_code->code, __safe_add_string(error_code->msg)));

		if (error_code != &_error) {
			free(error_code->msg);
			free(error_code);
			error_code = NULL;
		} else {
			free(_error.msg);
		}
		free(service_name);
		free(server_path);
		free(download_path);
	} else if (0 == g_strcmp0(method_name, DBUS_DOWNLOAD_THUMBNAIL_METHOD)) {
		char *service_name = NULL;
		char *server_path = NULL;
		char *download_path = NULL;
		int thumbnail_size;
		storage_adaptor_error_code_h error_code = NULL;
		storage_adaptor_error_code_t _error;
		_error.msg = NULL;

		__get_download_thumbnail_req_type(parameters, &service_name, &server_path, &download_path, &thumbnail_size);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = STORAGE_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = strdup("Can not get service context");

			g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)",
					(uint64_t) error_code->code, __safe_add_string(error_code->msg)));

			free(_error.msg);
			free(service_name);
			free(server_path);
			free(download_path);
			return;
		}

		storage_adaptor_h adaptor = service_adaptor_get_storage_adaptor(service_adaptor);
		storage_adaptor_plugin_h plugin = NULL;

		if (NULL != service->storage_context) {
			plugin = storage_adaptor_get_plugin_by_name(adaptor, service->storage_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			char *parent_folder = NULL;
			char *file_name = NULL;
			__separate_path_to_dir_base(server_path, &parent_folder, &file_name);

			ret_code = storage_adaptor_download_thumbnail(plugin, service->storage_context,
					parent_folder, file_name, download_path, thumbnail_size, NULL, &error_code, NULL);

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->storage_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%lld: %s)", error_code ? error_code->code : 0ULL, error_code ? error_code->msg : NULL);
				storage_adaptor_destroy_error_code(&error_code);

				service_adaptor_debug("Re-try API");
				ret_code = storage_adaptor_download_thumbnail(plugin, service->storage_context,
						parent_folder, file_name, download_path, thumbnail_size, NULL, &error_code, NULL);
			}

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NONE != ret_code) {
				service_adaptor_error("Can not run storage_adaptor_download_thumbnail()");
			}
			free(parent_folder);
			free(file_name);
		}

		__check_error_code();

		g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)",
				(uint64_t) error_code->code, __safe_add_string(error_code->msg)));

		if (error_code != &_error) {
			free(error_code->msg);
			free(error_code);
			error_code = NULL;
		} else {
			free(_error.msg);
		}
		free(service_name);
		free(server_path);
		free(download_path);
	} else if (0 == g_strcmp0(method_name, DBUS_UPLOAD_FILE_METHOD)) {
		char *service_name = NULL;
		char *upload_path = NULL;
		char *server_path = NULL;
		storage_adaptor_error_code_h error_code = NULL;
		storage_adaptor_error_code_t _error;
		_error.msg = NULL;
		storage_adaptor_file_info_h file_info = NULL;

		__get_file_transfer_req_type(parameters, &service_name, &upload_path, &server_path);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = STORAGE_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = strdup("Can not get service context");

			GVariant *res_v = __create_file_info_res_type(file_info, error_code);
			g_dbus_method_invocation_return_value(invocation, res_v);

			free(_error.msg);
			free(service_name);
			free(server_path);
			free(upload_path);
			return;
		}

		storage_adaptor_h adaptor = service_adaptor_get_storage_adaptor(service_adaptor);
		storage_adaptor_plugin_h plugin = NULL;

		if (NULL != service->storage_context) {
			plugin = storage_adaptor_get_plugin_by_name(adaptor, service->storage_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			char *parent_folder = NULL;
			char *file_name = NULL;
			__separate_path_to_dir_base(server_path, &parent_folder, &file_name);

			ret_code = storage_adaptor_upload_file_sync(plugin, service->storage_context,
					parent_folder, file_name, upload_path, false, NULL, &file_info, &error_code, NULL);

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->storage_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%lld: %s)", error_code ? error_code->code : 0ULL, error_code ? error_code->msg : NULL);
				storage_adaptor_destroy_error_code(&error_code);
				storage_adaptor_destroy_file_info(&file_info);

				service_adaptor_debug("Re-try API");
				ret_code = storage_adaptor_upload_file_sync(plugin, service->storage_context,
						parent_folder, file_name, upload_path, false, NULL, &file_info, &error_code, NULL);
			}
			free(parent_folder);
			free(file_name);
		}

		__check_error_code();

		GVariant *res_v = __create_file_info_res_type(file_info, error_code);
		g_dbus_method_invocation_return_value(invocation, res_v);

		storage_adaptor_destroy_file_info(&file_info);

		if (error_code != &_error) {
			free(error_code->msg);
			free(error_code);
			error_code = NULL;
		} else {
			free(_error.msg);
		}
		free(service_name);
		free(server_path);
		free(upload_path);
	} else if (0 == g_strcmp0(method_name, DBUS_GET_ROOT_FOLDER_PATH_METHOD)) {
		char *service_name = NULL;
		char *root_folder_path = NULL;
		storage_adaptor_error_code_h error_code = NULL;
		storage_adaptor_error_code_t _error;
		_error.msg = NULL;

		__get_get_root_folder_path_req_type(parameters, &service_name);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = STORAGE_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = strdup("Can not get service context");

			GVariant *response = __create_get_root_folder_path_res_type(root_folder_path, error_code);
			g_dbus_method_invocation_return_value(invocation, response);

			free(_error.msg);
			free(service_name);
			return;
		}

		storage_adaptor_h adaptor = service_adaptor_get_storage_adaptor(service_adaptor);
		storage_adaptor_plugin_h plugin = NULL;

		if (NULL != service->storage_context) {
			plugin = storage_adaptor_get_plugin_by_name(adaptor, service->storage_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			ret_code = storage_adaptor_get_root_folder_path(plugin, service->storage_context,
					NULL, &root_folder_path, &error_code, NULL);

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->storage_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%lld: %s)", error_code ? error_code->code : 0ULL, error_code ? error_code->msg : NULL);
				storage_adaptor_destroy_error_code(&error_code);

				service_adaptor_debug("Re-try API");
				ret_code = storage_adaptor_get_root_folder_path(plugin, service->storage_context,
						NULL, &root_folder_path, &error_code, NULL);
			}
		}

		__check_error_code();

		GVariant *response = __create_get_root_folder_path_res_type(root_folder_path, error_code);
		g_dbus_method_invocation_return_value(invocation, response);

		if (error_code != &_error) {
			free(error_code->msg);
			free(error_code);
			error_code = NULL;
		} else {
			free(_error.msg);
		}
		free(service_name);
		free(root_folder_path);
	} else if (0 == g_strcmp0(method_name, DBUS_MAKE_DIRECTORY_METHOD)) {
		char *service_name = NULL;
		char *folder_path = NULL;
		storage_adaptor_error_code_h error_code = NULL;
		storage_adaptor_error_code_t _error;
		_error.msg = NULL;
		storage_adaptor_file_info_h file_info = NULL;

		__get_make_directory_req_type(parameters, &service_name, &folder_path);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = STORAGE_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = strdup("Can not get service context");

			GVariant *res_v = __create_file_info_res_type(file_info, error_code);
			g_dbus_method_invocation_return_value(invocation, res_v);

			free(_error.msg);
			free(service_name);
			free(folder_path);
			return;
		}

		storage_adaptor_h adaptor = service_adaptor_get_storage_adaptor(service_adaptor);
		storage_adaptor_plugin_h plugin = NULL;

		if (NULL != service->storage_context) {
			plugin = storage_adaptor_get_plugin_by_name(adaptor, service->storage_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			char *folder_path_dir = NULL;
			char *folder_path_base = NULL;

			__separate_path_to_dir_base(folder_path, &folder_path_dir, &folder_path_base);

			ret_code = storage_adaptor_make_directory(plugin, service->storage_context,
					folder_path_dir, folder_path_base, NULL, &file_info, &error_code, NULL);

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->storage_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%lld: %s)", error_code ? error_code->code : 0ULL, error_code ? error_code->msg : NULL);
				storage_adaptor_destroy_error_code(&error_code);
				storage_adaptor_destroy_file_info(&file_info);

				service_adaptor_debug("Re-try API");
				ret_code = storage_adaptor_make_directory(plugin, service->storage_context,
						folder_path_dir, folder_path_base, NULL, &file_info, &error_code, NULL);
			}
			free(folder_path_dir);
			free(folder_path_base);
		}

		__check_error_code();

		GVariant *res_v = __create_file_info_res_type(file_info, error_code);
		g_dbus_method_invocation_return_value(invocation, res_v);

		storage_adaptor_destroy_file_info(&file_info);


		if (error_code != &_error) {
			free(error_code->msg);
			free(error_code);
			error_code = NULL;
		} else {
			free(_error.msg);
		}
		free(service_name);
		free(folder_path);
	} else if (0 == g_strcmp0(method_name, DBUS_REMOVE_FILE_METHOD)) {
		char *service_name = NULL;
		char *file_path = NULL;
		storage_adaptor_error_code_h error_code = NULL;
		storage_adaptor_error_code_t _error;
		_error.msg = NULL;
		storage_adaptor_file_info_h file_info = NULL;

		__get_remove_file_req_type(parameters, &service_name, &file_path);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = STORAGE_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = strdup("Can not get service context");

			g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)",
					(uint64_t) error_code->code, __safe_add_string(error_code->msg)));

			free(_error.msg);
			free(service_name);
			free(file_path);
			return;
		}

		storage_adaptor_h adaptor = service_adaptor_get_storage_adaptor(service_adaptor);
		storage_adaptor_plugin_h plugin = NULL;

		if (NULL != service->storage_context) {
			plugin = storage_adaptor_get_plugin_by_name(adaptor, service->storage_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			char *file_path_dir = NULL;
			char *file_path_base = NULL;

			__separate_path_to_dir_base(file_path, &file_path_dir, &file_path_base);
			ret_code = storage_adaptor_delete_file(plugin, service->storage_context,
					file_path_dir, file_path_base, NULL,
					&file_info, &error_code, NULL);

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->storage_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%lld: %s)", error_code ? error_code->code : 0ULL, error_code ? error_code->msg : NULL);
				storage_adaptor_destroy_error_code(&error_code);
				storage_adaptor_destroy_file_info(&file_info);

				service_adaptor_debug("Re-try API");
				ret_code = storage_adaptor_delete_file(plugin, service->storage_context,
						file_path_dir, file_path_base, NULL,
						&file_info, &error_code, NULL);
			}
			free(file_path_dir);
			free(file_path_base);
		}

		__check_error_code();

		g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)",
				(uint64_t) error_code->code, __safe_add_string(error_code->msg)));

		if (error_code != &_error) {
			free(error_code->msg);
			free(error_code);
			error_code = NULL;
		} else {
			free(_error.msg);
		}
		free(service_name);
		free(file_path);
		storage_adaptor_destroy_file_info(&file_info);
	} else if (0 == g_strcmp0(method_name, DBUS_REMOVE_DIRECTORY_METHOD)) {
		char *service_name = NULL;
		char *folder_path = NULL;
		storage_adaptor_error_code_h error_code = NULL;
		storage_adaptor_error_code_t _error;
		_error.msg = NULL;
		storage_adaptor_file_info_h file_info = NULL;

		__get_remove_directory_req_type(parameters, &service_name, &folder_path);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = STORAGE_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = strdup("Can not get service context");

			g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)",
					(uint64_t) error_code->code, __safe_add_string(error_code->msg)));

			free(_error.msg);
			free(service_name);
			free(folder_path);
			return;
		}

		storage_adaptor_h adaptor = service_adaptor_get_storage_adaptor(service_adaptor);
		storage_adaptor_plugin_h plugin = NULL;

		if (NULL != service->storage_context) {
			plugin = storage_adaptor_get_plugin_by_name(adaptor, service->storage_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			char *folder_path_dir = NULL;
			char *folder_path_base = NULL;

			__separate_path_to_dir_base(folder_path, &folder_path_dir, &folder_path_base);
			ret_code = storage_adaptor_remove_directory(plugin, service->storage_context,
					folder_path_dir, folder_path_base, NULL,
					&file_info, &error_code, NULL);

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->storage_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%lld: %s)", error_code ? error_code->code : 0ULL, error_code ? error_code->msg : NULL);
				storage_adaptor_destroy_error_code(&error_code);
				storage_adaptor_destroy_file_info(&file_info);

				service_adaptor_debug("Re-try API");
				ret_code = storage_adaptor_remove_directory(plugin, service->storage_context,
						folder_path_dir, folder_path_base, NULL,
						&file_info, &error_code, NULL);
			}
			free(folder_path_dir);
			free(folder_path_base);
		}

		__check_error_code();

		g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)",
				(uint64_t) error_code->code, __safe_add_string(error_code->msg)));

		if (error_code != &_error) {
			free(error_code->msg);
			free(error_code);
			error_code = NULL;
		} else {
			free(_error.msg);
		}
		free(service_name);
		free(folder_path);

		storage_adaptor_destroy_file_info(&file_info);
	} else if (0 == g_strcmp0(method_name, DBUS_MOVE_FILE_METHOD)) {
		char *service_name = NULL;
		char *src_file_path = NULL;
		char *dst_file_path = NULL;
		storage_adaptor_error_code_h error_code = NULL;
		storage_adaptor_error_code_t _error;
		_error.msg = NULL;
		storage_adaptor_file_info_h file_info = NULL;

		__get_move_file_req_type(parameters, &service_name, &src_file_path, &dst_file_path);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = STORAGE_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = strdup("Can not get service context");

			GVariant *res_v = __create_file_info_res_type(file_info, error_code);
			g_dbus_method_invocation_return_value(invocation, res_v);

			free(_error.msg);
			free(service_name);
			free(src_file_path);
			free(dst_file_path);
			return;
		}

		storage_adaptor_h adaptor = service_adaptor_get_storage_adaptor(service_adaptor);
		storage_adaptor_plugin_h plugin = NULL;

		if (NULL != service->storage_context) {
			plugin = storage_adaptor_get_plugin_by_name(adaptor, service->storage_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			char *src_file_path_dir = NULL;
			char *src_file_path_base = NULL;
			char *dst_file_path_dir = NULL;
			char *dst_file_path_base = NULL;

			__separate_path_to_dir_base(src_file_path, &src_file_path_dir, &src_file_path_base);
			__separate_path_to_dir_base(dst_file_path, &dst_file_path_dir, &dst_file_path_base);
			ret_code = storage_adaptor_move_file(plugin, service->storage_context,
					src_file_path_dir, src_file_path_base,
					dst_file_path_dir, dst_file_path_base, NULL,
					&file_info, &error_code, NULL);

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->storage_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%lld: %s)", error_code ? error_code->code : 0ULL, error_code ? error_code->msg : NULL);
				storage_adaptor_destroy_error_code(&error_code);
				storage_adaptor_destroy_file_info(&file_info);

				service_adaptor_debug("Re-try API");
				ret_code = storage_adaptor_move_file(plugin, service->storage_context,
						src_file_path_dir, src_file_path_base,
						dst_file_path_dir, dst_file_path_base, NULL,
						&file_info, &error_code, NULL);
			}
			free(src_file_path_dir);
			free(src_file_path_base);
			free(dst_file_path_dir);
			free(dst_file_path_base);
		}

		__check_error_code();

		GVariant *res_v = __create_file_info_res_type(file_info, error_code);
		g_dbus_method_invocation_return_value(invocation, res_v);

		storage_adaptor_destroy_file_info(&file_info);

		if (error_code != &_error) {
			free(error_code->msg);
			free(error_code);
			error_code = NULL;
		} else {
			free(_error.msg);
		}
		free(service_name);
		free(src_file_path);
		free(dst_file_path);
	} else if (0 == g_strcmp0(method_name, DBUS_MOVE_DIRECTORY_METHOD)) {
		char *service_name = NULL;
		char *src_folder_path = NULL;
		char *dst_folder_path = NULL;
		storage_adaptor_error_code_h error_code = NULL;
		storage_adaptor_error_code_t _error;
		_error.msg = NULL;
		storage_adaptor_file_info_h file_info = NULL;

		__get_move_directory_req_type(parameters, &service_name, &src_folder_path, &dst_folder_path);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = STORAGE_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = strdup("Can not get service context");

			GVariant *res_v = __create_file_info_res_type(file_info, error_code);
			g_dbus_method_invocation_return_value(invocation, res_v);

			free(_error.msg);
			free(service_name);
			free(src_folder_path);
			free(dst_folder_path);
			return;
		}

		storage_adaptor_h adaptor = service_adaptor_get_storage_adaptor(service_adaptor);
		storage_adaptor_plugin_h plugin = NULL;

		if (NULL != service->storage_context) {
			plugin = storage_adaptor_get_plugin_by_name(adaptor, service->storage_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			char *src_folder_path_dir = NULL;
			char *src_folder_path_base = NULL;
			char *dst_folder_path_dir = NULL;
			char *dst_folder_path_base = NULL;

			__separate_path_to_dir_base(src_folder_path, &src_folder_path_dir, &src_folder_path_base);
			__separate_path_to_dir_base(dst_folder_path, &dst_folder_path_dir, &dst_folder_path_base);
			ret_code = storage_adaptor_move_directory(plugin, service->storage_context,
					src_folder_path_dir, src_folder_path_base,
					dst_folder_path_dir, dst_folder_path_base, NULL,
					&file_info, &error_code, NULL);

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->storage_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%lld: %s)", error_code ? error_code->code : 0ULL, error_code ? error_code->msg : NULL);
				storage_adaptor_destroy_error_code(&error_code);
				storage_adaptor_destroy_file_info(&file_info);

				service_adaptor_debug("Re-try API");
				ret_code = storage_adaptor_move_directory(plugin, service->storage_context,
						src_folder_path_dir, src_folder_path_base,
						dst_folder_path_dir, dst_folder_path_base, NULL,
						&file_info, &error_code, NULL);
			}
			free(src_folder_path_dir);
			free(src_folder_path_base);
			free(dst_folder_path_dir);
			free(dst_folder_path_base);
		}

		__check_error_code();

		GVariant *res_v = __create_file_info_res_type(file_info, error_code);
		g_dbus_method_invocation_return_value(invocation, res_v);

		storage_adaptor_destroy_file_info(&file_info);

		if (error_code != &_error) {
			free(error_code->msg);
			free(error_code);
			error_code = NULL;
		} else {
			free(_error.msg);
		}
		free(service_name);
		free(src_folder_path);
		free(dst_folder_path);
	} else if (0 == g_strcmp0(method_name, DBUS_GET_FILE_LIST_METHOD)) {
		char *service_name = NULL;
		char *parent_path = NULL;
		storage_adaptor_error_code_h error_code = NULL;
		storage_adaptor_error_code_t _error;
		_error.msg = NULL;
		storage_adaptor_file_info_h *file_info_list = NULL;
		int file_info_list_len = 0;

		__get_get_file_list_req_type(parameters, &service_name, &parent_path);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = STORAGE_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = strdup("Can not get service context");

			GVariant *response = __create_get_file_list_res_type(file_info_list, (unsigned int)file_info_list_len, error_code);
			g_dbus_method_invocation_return_value(invocation, response);

			free(_error.msg);
			free(service_name);
			free(parent_path);
			return;
		}

		storage_adaptor_h adaptor = service_adaptor_get_storage_adaptor(service_adaptor);
		storage_adaptor_plugin_h plugin = NULL;

		if (NULL != service->storage_context) {
			plugin = storage_adaptor_get_plugin_by_name(adaptor, service->storage_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			char *parent_path_dir = NULL;
			char *parent_path_base = NULL;

			__separate_path_to_dir_base(parent_path, &parent_path_dir, &parent_path_base);
			ret_code = storage_adaptor_list(plugin, service->storage_context,
					parent_path_dir, parent_path_base, NULL,
					&file_info_list, &file_info_list_len, &error_code, NULL);

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->storage_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%lld: %s)", error_code ? error_code->code : 0ULL, error_code ? error_code->msg : NULL);
				storage_adaptor_destroy_error_code(&error_code);
				if ((0 < file_info_list_len) && (NULL != file_info_list)) {
					int i;
					for (i = 0; i < file_info_list_len; i++) {
						storage_adaptor_destroy_file_info(&file_info_list[i]);
					}
					free(file_info_list);
					file_info_list = NULL;
					file_info_list_len = 0;
				}

				service_adaptor_debug("Re-try API");
				ret_code = storage_adaptor_list(plugin, service->storage_context,
						parent_path_dir, parent_path_base, NULL,
						&file_info_list, &file_info_list_len, &error_code, NULL);
			}
			free(parent_path_dir);
			free(parent_path_base);
		}

		__check_error_code();

		GVariant *response = __create_get_file_list_res_type(file_info_list, (unsigned int)file_info_list_len, error_code);
		g_dbus_method_invocation_return_value(invocation, response);

		if (error_code != &_error) {
			free(error_code->msg);
			free(error_code);
			error_code = NULL;
		} else {
			free(_error.msg);
		}
		free(service_name);
		free(parent_path);
		if ((0 < file_info_list_len) && (NULL != file_info_list)) {
			int i;
			for (i = 0; i < file_info_list_len; i++) {
				storage_adaptor_destroy_file_info(&file_info_list[i]);
			}
			free(file_info_list);
			file_info_list = NULL;
			file_info_list_len = 0;
		}
	} else if (0 == g_strcmp0(method_name, DBUS_OPEN_UPLOAD_FILE_METHOD)) {
		char *service_name = NULL;
		char *local_path = NULL;
		char *upload_path = NULL;
		long long int file_uid = 0;
		storage_adaptor_error_code_h error_code = NULL;
		storage_adaptor_error_code_t _error;
		_error.msg = NULL;

		__get_open_upload_file_req_type(parameters, &service_name, &local_path, &upload_path);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = STORAGE_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = strdup("Can not get service context");

			GVariant *res_v = __create_file_descriptor_res_type(file_uid, error_code);
			g_dbus_method_invocation_return_value(invocation, res_v);

			free(service_name);
			free(local_path);
			free(upload_path);

			free(_error.msg);
			return;
		}

		storage_adaptor_h adaptor = service_adaptor_get_storage_adaptor(service_adaptor);
		storage_adaptor_plugin_h plugin = NULL;

		if (NULL != service->storage_context) {
			plugin = storage_adaptor_get_plugin_by_name(adaptor, service->storage_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			/*ret_code = client_checker_check_access_right_read(service_name, local_path);
			service_adaptor_debug("Permission check : %d", ret_code);

			if (!ret_code) {*/
			ret_code = storage_adaptor_open_file(plugin, service->storage_context,
					local_path, STORAGE_ADAPTOR_FILE_ACCESS_READ,
					&file_uid, &error_code);

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->storage_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%lld: %s)", error_code ? error_code->code : 0ULL, error_code ? error_code->msg : NULL);
				storage_adaptor_destroy_error_code(&error_code);

				service_adaptor_debug("Re-try API");
				ret_code = storage_adaptor_open_file(plugin, service->storage_context,
						local_path, STORAGE_ADAPTOR_FILE_ACCESS_READ,
						&file_uid, &error_code);
			}
/*			} else {
				error_code = storage_adaptor_create_error_code(SERVICE_ADAPTOR_PROTOCOL_RETURN_CODE_COMMON_PERMISSION_DENIED,
						clieht_checker_get_last_error());
			}*/
		}

		__check_error_code();

		GVariant *res_v = __create_file_descriptor_res_type(file_uid, error_code);
		g_dbus_method_invocation_return_value(invocation, res_v);

		if (error_code != &_error) {
			free(error_code->msg);
			free(error_code);
			error_code = NULL;
		} else {
			free(_error.msg);
		}
		free(service_name);
		free(local_path);
		free(upload_path);
	} else if (0 == g_strcmp0(method_name, DBUS_OPEN_DOWNLOAD_FILE_METHOD)) {
		char *service_name = NULL;
		char *storage_path = NULL;
		char *local_path = NULL;
		long long int file_uid = 0;
		storage_adaptor_error_code_h error_code = NULL;
		storage_adaptor_error_code_t _error;
		_error.msg = NULL;

		__get_open_download_file_req_type(parameters, &service_name, &storage_path, &local_path);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
			service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = STORAGE_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = strdup("Can not get service context");

			GVariant *res_v = __create_file_descriptor_res_type(file_uid, error_code);
			g_dbus_method_invocation_return_value(invocation, res_v);

			free(service_name);
			free(storage_path);
			free(local_path);

			free(_error.msg);
			return;
		}

		storage_adaptor_h adaptor = service_adaptor_get_storage_adaptor(service_adaptor);
		storage_adaptor_plugin_h plugin = NULL;

		if (NULL != service->storage_context) {
			plugin = storage_adaptor_get_plugin_by_name(adaptor, service->storage_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			/*ret_code = client_checker_check_access_right_create(service_name, local_path);
			service_adaptor_debug("Permission check : %d", ret_code);

			if (!ret_code) {*/
			ret_code = storage_adaptor_open_file(plugin, service->storage_context,
					local_path, STORAGE_ADAPTOR_FILE_ACCESS_WRITE,
					&file_uid, &error_code);

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->storage_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%lld: %s)", error_code ? error_code->code : 0ULL, error_code ? error_code->msg : NULL);
				storage_adaptor_destroy_error_code(&error_code);

				service_adaptor_debug("Re-try API");
				ret_code = storage_adaptor_open_file(plugin, service->storage_context,
						local_path, STORAGE_ADAPTOR_FILE_ACCESS_WRITE,
						&file_uid, &error_code);
			}
/*			} else {
				error_code = storage_adaptor_create_error_code(SERVICE_ADAPTOR_PROTOCOL_RETURN_CODE_COMMON_PERMISSION_DENIED,
						clieht_checker_get_last_error());
			}*/
		}

		__check_error_code();

		GVariant *res_v = __create_file_descriptor_res_type(file_uid, error_code);
		g_dbus_method_invocation_return_value(invocation, res_v);

		if (error_code != &_error) {
			free(error_code->msg);
			free(error_code);
			error_code = NULL;
		} else {
			free(_error.msg);
		}
		free(service_name);
		free(storage_path);
		free(local_path);
	} else if (0 == g_strcmp0(method_name, DBUS_OPEN_DOWNLOAD_THUMBNAIL_METHOD)) {
		char *service_name = NULL;
		char *storage_path = NULL;
		char *local_path = NULL;
		int thumbnail_size = 0;
		long long int file_uid = 0;
		storage_adaptor_error_code_h error_code = NULL;
		storage_adaptor_error_code_t _error;
		_error.msg = NULL;

		__get_open_download_thumbnail_req_type(parameters, &service_name, &storage_path, &local_path, &thumbnail_size);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = STORAGE_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = strdup("Can not get service context");

			GVariant *res_v = __create_file_descriptor_res_type(file_uid, error_code);
			g_dbus_method_invocation_return_value(invocation, res_v);

			free(service_name);
			free(storage_path);
			free(local_path);

			free(_error.msg);
			return;
		}

		storage_adaptor_h adaptor = service_adaptor_get_storage_adaptor(service_adaptor);
		storage_adaptor_plugin_h plugin = NULL;

		if (NULL != service->storage_context) {
			plugin = storage_adaptor_get_plugin_by_name(adaptor, service->storage_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			/*ret_code = client_checker_check_access_right_create(service_name, local_path);
			service_adaptor_debug("Permission check : %d", ret_code);

			if (!ret_code) {*/
			ret_code = storage_adaptor_open_file(plugin, service->storage_context,
					local_path, STORAGE_ADAPTOR_FILE_ACCESS_WRITE,
					&file_uid, &error_code);

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->storage_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%lld: %s)", error_code ? error_code->code : 0ULL, error_code ? error_code->msg : NULL);
				storage_adaptor_destroy_error_code(&error_code);

				service_adaptor_debug("Re-try API");
				ret_code = storage_adaptor_open_file(plugin, service->storage_context,
						local_path, STORAGE_ADAPTOR_FILE_ACCESS_WRITE,
						&file_uid, &error_code);
			}
/*			} else {
				error_code = storage_adaptor_create_error_code(SERVICE_ADAPTOR_PROTOCOL_RETURN_CODE_COMMON_PERMISSION_DENIED,
						clieht_checker_get_last_error());
			}*/
		}

		__check_error_code();

		GVariant *res_v = __create_file_descriptor_res_type(file_uid, error_code);
		g_dbus_method_invocation_return_value(invocation, res_v);

		if (error_code != &_error) {
			free(error_code->msg);
			free(error_code);
			error_code = NULL;
		} else {
			free(_error.msg);
		}
		free(service_name);
		free(storage_path);
		free(local_path);
	} else if (0 == g_strcmp0(method_name, DBUS_CLOSE_FILE_METHOD)) {
		char *service_name = NULL;
		long long int file_uid = 0;
		storage_adaptor_error_code_h error_code = NULL;
		storage_adaptor_error_code_t _error;
		_error.msg = NULL;

		__get_close_file_req_type(parameters, &service_name, &file_uid);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = STORAGE_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = strdup("Can not get service context");

			g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)", (uint64_t) error_code->code, __safe_add_string(error_code->msg)));

			free(service_name);

			free(_error.msg);
			return;
		}

		storage_adaptor_h adaptor = service_adaptor_get_storage_adaptor(service_adaptor);
		storage_adaptor_plugin_h plugin = NULL;

		if (NULL != service->storage_context) {
			plugin = storage_adaptor_get_plugin_by_name(adaptor, service->storage_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			ret_code = storage_adaptor_close_file(plugin, service->storage_context,
					file_uid, &error_code);

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->storage_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%lld: %s)", error_code ? error_code->code : 0ULL, error_code ? error_code->msg : NULL);
				storage_adaptor_destroy_error_code(&error_code);

				service_adaptor_debug("Re-try API");
				ret_code = storage_adaptor_close_file(plugin, service->storage_context,
						file_uid, &error_code);
			}
		}

		__check_error_code();

		g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)", (uint64_t) error_code->code, __safe_add_string(error_code->msg)));

		if (error_code != &_error) {
			free(error_code->msg);
			free(error_code);
			error_code = NULL;
		} else {
			free(_error.msg);
		}
		free(service_name);
	} else if (0 == g_strcmp0(method_name, DBUS_START_UPLOAD_FILE_METHOD)) {
		char *service_name = NULL;
		long long int file_uid = 0;
		char *storage_path = NULL;
		bool need_progress = false;
		bool need_state = false;
		storage_adaptor_error_code_h error_code = NULL;
		storage_adaptor_error_code_t _error;
		_error.msg = NULL;

		__get_start_upload_file_req_type(parameters, &service_name, &file_uid, &storage_path, &need_progress, &need_state);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = STORAGE_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = strdup("Can not get service context");

			g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)", (uint64_t) error_code->code, __safe_add_string(error_code->msg)));

			free(service_name);
			free(storage_path);

			free(_error.msg);
			return;
		}

		storage_adaptor_h adaptor = service_adaptor_get_storage_adaptor(service_adaptor);
		storage_adaptor_plugin_h plugin = NULL;

		char *storage_path_dir = NULL;
		char *storage_path_base = NULL;
		__separate_path_to_dir_base(storage_path, &storage_path_dir, &storage_path_base);

		if (NULL != service->storage_context) {
			plugin = storage_adaptor_get_plugin_by_name(adaptor, service->storage_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			ret_code = storage_adaptor_start_upload_task(plugin, service->storage_context,
					file_uid, storage_path_dir, storage_path_base, need_progress, &error_code, NULL);

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->storage_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%lld: %s)", error_code ? error_code->code : 0ULL, error_code ? error_code->msg : NULL);
				storage_adaptor_destroy_error_code(&error_code);

				service_adaptor_debug("Re-try API");
				ret_code = storage_adaptor_start_upload_task(plugin, service->storage_context,
						file_uid, storage_path_dir, storage_path_base, need_progress, &error_code, NULL);
			}
		}
		free(storage_path_dir);
		free(storage_path_base);

		__check_error_code();

		g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)", (uint64_t) error_code->code, __safe_add_string(error_code->msg)));

		if (error_code != &_error) {
			free(error_code->msg);
			free(error_code);
			error_code = NULL;
		} else {
			free(_error.msg);
		}
		free(service_name);
		free(storage_path);
	} else if (0 == g_strcmp0(method_name, DBUS_START_DOWNLOAD_FILE_METHOD)) {
		char *service_name = NULL;
		long long int file_uid = 0;
		char *storage_path = NULL;
		bool need_progress = false;
		bool need_state = false;
		storage_adaptor_error_code_h error_code = NULL;
		storage_adaptor_error_code_t _error;
		_error.msg = NULL;

		__get_start_download_file_req_type(parameters, &service_name, &file_uid, &storage_path, &need_progress, &need_state);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = STORAGE_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = strdup("Can not get service context");

			g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)", (uint64_t) error_code->code, __safe_add_string(error_code->msg)));

			free(service_name);
			free(storage_path);

			free(_error.msg);
			return;
		}

		storage_adaptor_h adaptor = service_adaptor_get_storage_adaptor(service_adaptor);
		storage_adaptor_plugin_h plugin = NULL;

		char *storage_path_dir = NULL;
		char *storage_path_base = NULL;
		__separate_path_to_dir_base(storage_path, &storage_path_dir, &storage_path_base);

		if (NULL != service->storage_context) {
			plugin = storage_adaptor_get_plugin_by_name(adaptor, service->storage_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			ret_code = storage_adaptor_start_download_task(plugin, service->storage_context,
					storage_path_dir, storage_path_base, file_uid, need_progress, &error_code, NULL);

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->storage_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%lld: %s)", error_code ? error_code->code : 0ULL, error_code ? error_code->msg : NULL);
				storage_adaptor_destroy_error_code(&error_code);

				service_adaptor_debug("Re-try API");
				ret_code = storage_adaptor_start_download_task(plugin, service->storage_context,
						storage_path_dir, storage_path_base, file_uid, need_progress, &error_code, NULL);
			}
		}
		free(storage_path_dir);
		free(storage_path_base);

		__check_error_code();

		g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)", (uint64_t) error_code->code, __safe_add_string(error_code->msg)));

		if (error_code != &_error) {
			free(error_code->msg);
			free(error_code);
			error_code = NULL;
		} else {
			free(_error.msg);
		}
		free(service_name);
		free(storage_path);
	} else if (0 == g_strcmp0(method_name, DBUS_START_DOWNLOAD_THUMBNAIL_METHOD)) {
		char *service_name = NULL;
		long long int file_uid = 0;
		char *storage_path = NULL;
		int thumbnail_size = 0;
		bool need_progress = false;
		bool need_state = false;
		storage_adaptor_error_code_h error_code = NULL;
		storage_adaptor_error_code_t _error;
		_error.msg = NULL;

		__get_start_download_thumbnail_req_type(parameters, &service_name, &file_uid,
				&storage_path, &thumbnail_size, &need_progress, &need_state);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = STORAGE_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = strdup("Can not get service context");

			g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)", (uint64_t) error_code->code, __safe_add_string(error_code->msg)));

			free(service_name);
			free(storage_path);

			free(_error.msg);
			return;
		}

		storage_adaptor_h adaptor = service_adaptor_get_storage_adaptor(service_adaptor);
		storage_adaptor_plugin_h plugin = NULL;

		char *storage_path_dir = NULL;
		char *storage_path_base = NULL;
		__separate_path_to_dir_base(storage_path, &storage_path_dir, &storage_path_base);

		if (NULL != service->storage_context) {
			plugin = storage_adaptor_get_plugin_by_name(adaptor, service->storage_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			ret_code = storage_adaptor_start_download_thumb_task(plugin, service->storage_context,
					storage_path_dir, storage_path_base, file_uid, thumbnail_size, need_progress, &error_code, NULL);

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->storage_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%lld: %s)", error_code ? error_code->code : 0ULL, error_code ? error_code->msg : NULL);
				storage_adaptor_destroy_error_code(&error_code);

				service_adaptor_debug("Re-try API");
				ret_code = storage_adaptor_start_download_thumb_task(plugin, service->storage_context,
						storage_path_dir, storage_path_base, file_uid, thumbnail_size, need_progress, &error_code, NULL);
			}
		}
		free(storage_path_dir);
		free(storage_path_base);

		__check_error_code();

		g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)", (uint64_t) error_code->code, __safe_add_string(error_code->msg)));

		if (error_code != &_error) {
			free(error_code->msg);
			free(error_code);
			error_code = NULL;
		} else {
			free(_error.msg);
		}
		free(service_name);
		free(storage_path);
	} else if ((0 == g_strcmp0(method_name, DBUS_CANCEL_UPLOAD_FILE_METHOD))
			|| (0 == g_strcmp0(method_name, DBUS_CANCEL_DOWNLOAD_FILE_METHOD))
			|| (0 == g_strcmp0(method_name, DBUS_CANCEL_DOWNLOAD_THUMBNAIL_METHOD))) {
		char *service_name = NULL;
		long long int file_uid = 0;
		storage_adaptor_error_code_h error_code = NULL;
		storage_adaptor_error_code_t _error;
		_error.msg = NULL;

		__get_cancel_file_task_req_type(parameters, &service_name, &file_uid);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = STORAGE_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = strdup("Can not get service context");

			g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)", (uint64_t) error_code->code, __safe_add_string(error_code->msg)));

			free(service_name);

			free(_error.msg);
			return;
		}

		storage_adaptor_h adaptor = service_adaptor_get_storage_adaptor(service_adaptor);
		storage_adaptor_plugin_h plugin = NULL;

		if (NULL != service->storage_context) {
			plugin = storage_adaptor_get_plugin_by_name(adaptor, service->storage_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			if (0 == g_strcmp0(method_name, DBUS_CANCEL_UPLOAD_FILE_METHOD)) {
				ret_code = storage_adaptor_cancel_upload_task(plugin, service->storage_context,
						file_uid, &error_code);
			} else if (0 == g_strcmp0(method_name, DBUS_CANCEL_DOWNLOAD_FILE_METHOD)) {
				ret_code = storage_adaptor_cancel_download_task(plugin, service->storage_context,
						file_uid, &error_code);
			} else {
				ret_code = storage_adaptor_cancel_download_thumb_task(plugin, service->storage_context,
						file_uid, &error_code);
			}

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->storage_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%lld: %s)", error_code ? error_code->code : 0ULL, error_code ? error_code->msg : NULL);
				storage_adaptor_destroy_error_code(&error_code);

				service_adaptor_debug("Re-try API");
				if (0 == g_strcmp0(method_name, DBUS_CANCEL_UPLOAD_FILE_METHOD)) {
					ret_code = storage_adaptor_cancel_upload_task(plugin, service->storage_context,
							file_uid, &error_code);
				} else if (0 == g_strcmp0(method_name, DBUS_CANCEL_DOWNLOAD_FILE_METHOD)) {
					ret_code = storage_adaptor_cancel_download_task(plugin, service->storage_context,
							file_uid, &error_code);
				} else {
					ret_code = storage_adaptor_cancel_download_thumb_task(plugin, service->storage_context,
							file_uid, &error_code);
				}
			}
		}

		__check_error_code();

		g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)", (uint64_t) error_code->code, __safe_add_string(error_code->msg)));

		if (error_code != &_error) {
			free(error_code->msg);
			free(error_code);
			error_code = NULL;
		} else {
			free(_error.msg);
		}

		free(service_name);
	}

}


/* private feature */
service_adaptor_internal_error_code_e private_dbus_storage_file_progress_callback(int32_t fd,
						uint64_t progress_size,
						uint64_t total_size,
						storage_adaptor_error_code_h error_code,
						void *server_data)
{
	GError* error = NULL;
	GDBusConnection *dbus_connection = dbus_get_connection();

	if (NULL != dbus_connection) {
		GVariant *response = g_variant_new(MAKE_RETURN_TYPE(private_service_adaptor_file_progress_s_type),
				fd, progress_size, total_size, (uint64_t) error_code->code, __safe_add_string(error_code->msg));

		g_dbus_connection_emit_signal(dbus_connection,
				NULL,
				SERVICE_ADAPTOR_OBJECT_PATH,
				SERVICE_ADAPTOR_INTERFACE,
				PRIVATE_DBUS_STORAGE_FILE_PROGRESS_SIGNAL,
				response,
				&error);

		if (NULL != error) {
			service_adaptor_debug("Unable to send msg: %s", error->message);
			return SERVICE_ADAPTOR_INTERNAL_ERROR_DBUS;
		}
	}

	return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
}

service_adaptor_internal_error_code_e private_dbus_storage_file_transfer_completion_callback(int32_t fd,
						char *publish_url,
						storage_adaptor_error_code_h error_code,
						void *server_data)
{
	GError* error = NULL;
	GDBusConnection *dbus_connection = dbus_get_connection();

	if (NULL != dbus_connection) {
		GVariant *response = g_variant_new(MAKE_RETURN_TYPE(private_service_adaptor_file_transfer_completion_s_type),
				fd, __safe_add_string(publish_url), (uint64_t) error_code->code, __safe_add_string(error_code->msg));

		g_dbus_connection_emit_signal(dbus_connection,
				NULL,
				SERVICE_ADAPTOR_OBJECT_PATH,
				SERVICE_ADAPTOR_INTERFACE,
				PRIVATE_DBUS_STORAGE_FILE_TRANSFER_COMPLETION_SIGNAL,
				response,
				&error);

		if (NULL != error) {
			service_adaptor_debug("Unable to send msg: %s", error->message);
			return SERVICE_ADAPTOR_INTERNAL_ERROR_DBUS;
		}
	}

	return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
}


/* public feature */
service_adaptor_internal_error_code_e dbus_storage_file_progress_callback(long long int file_uid,
						unsigned long long progress_size,
						unsigned long long total_size)
{
	GError* error = NULL;
	GDBusConnection *dbus_connection = dbus_get_connection();

	if (NULL != dbus_connection) {
		GVariant *response = g_variant_new("("service_adaptor_file_progress_s_type")",
				(int64_t) file_uid, (uint64_t) progress_size, (uint64_t) total_size);

		g_dbus_connection_emit_signal(dbus_connection,
				NULL,
				SERVICE_ADAPTOR_OBJECT_PATH,
				SERVICE_ADAPTOR_INTERFACE,
				DBUS_STORAGE_FILE_PROGRESS_SIGNAL,
				response,
				&error);

		if (NULL != error) {
			service_adaptor_debug("Unable to send msg: %s", error->message);
			return SERVICE_ADAPTOR_INTERNAL_ERROR_DBUS;
		}
	}

	return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
}

service_adaptor_internal_error_code_e dbus_storage_file_transfer_state_changed_callback(long long int file_uid,
						storage_adaptor_transfer_state_e state,
						storage_adaptor_error_code_h _error_code)
{
	GError* error = NULL;
	GDBusConnection *dbus_connection = dbus_get_connection();

	int _state = 0;

	switch (state) {
	case STORAGE_ADAPTOR_TRANSFER_STATE_IN_PROGRESS:
		_state = SERVICE_ADAPTOR_FILE_TRANSFER_STATE_IN_PROGRESS;
		break;
	case STORAGE_ADAPTOR_TRANSFER_STATE_FINISHED:
		_state = SERVICE_ADAPTOR_FILE_TRANSFER_STATE_COMPLETED;
		break;
	case STORAGE_ADAPTOR_TRANSFER_STATE_CANCELED:
		_state = SERVICE_ADAPTOR_FILE_TRANSFER_STATE_CANCELED;
		break;
	case STORAGE_ADAPTOR_TRANSFER_STATE_FAILED:
		_state = SERVICE_ADAPTOR_FILE_TRANSFER_STATE_FAILED;
		break;
	default:
		return SERVICE_ADAPTOR_INTERNAL_ERROR_UNSUPPORTED;
	}

	if (NULL != dbus_connection) {
		GVariant *response = g_variant_new(MAKE_RETURN_TYPE(service_adaptor_file_transfer_state_changed_s_type),
				(int64_t) file_uid, (int32_t) _state, (uint64_t) _error_code->code, __safe_add_string(_error_code->msg));

		g_dbus_connection_emit_signal(dbus_connection,
				NULL,
				SERVICE_ADAPTOR_OBJECT_PATH,
				SERVICE_ADAPTOR_INTERFACE,
				DBUS_STORAGE_FILE_TRANSFER_STATE_CHANGED_SIGNAL,
				response,
				&error);

		if (NULL != error) {
			service_adaptor_debug("Unable to send msg: %s", error->message);
			return SERVICE_ADAPTOR_INTERNAL_ERROR_DBUS;
		}
	}

	return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
}
