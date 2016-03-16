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
 * File: dbus-client-storage.h
 * Desc: D-Bbus IPC client APIs for storage
 * Created on: Oct, 2014
 * Auth: Yongjin Kim <youth.kim@samsung.com>
 *
 *****************************************************************************/
/**
 *	@file		dbus-client-storage.h
 *	@brief		Defines interface of D-Bus IPC
 *	@version	0.1
 */

#ifndef __TIZEN_SOCIAL_SERVICE_ADAPTOR_DBUS_CLIENT_STORAGE_H__
#define __TIZEN_SOCIAL_SERVICE_ADAPTOR_DBUS_CLIENT_STORAGE_H__

#include <glib.h>
#include <gio/gio.h>
#include "service_adaptor_client_type.h"
#include "service_adaptor_client_storage.h"
#include "service_adaptor_client_storage_internal.h"

//////////////////////// private feature
#include "private/service-adaptor-client-storage.h"
//////////////////////// private feature

void on_storage_signal(GDBusProxy *proxy,
						gchar *sender_name,
						gchar *signal_name,
						GVariant *parameters,
						gpointer user_data);

int _dbus_download_file_async(const char *service_name,
						const char *server_path,
						const char *download_path,
						service_storage_file_h *file_handle,
						void *user_data,
						service_adaptor_error_s *error);

int _dbus_upload_file_async(const char *service_name,
						const char *upload_path,
						const char *server_path,
						service_storage_file_h *file_handle,
						void *user_data,
						service_adaptor_error_s *error);

int _dbus_remove_file(const char *service_name,
						const char *file_path,
						void *user_data,
						service_adaptor_error_s *error);

int _dbus_get_file_list(const char *service_name,
						const char *parent_path,
						service_storage_file_h **file_info_list,
						unsigned int *file_info_list_len,
						void *user_data,
						service_adaptor_error_s *error);

int _dbus_open_upload_file(const char *_service_name,
						const char *_file_path,
						const char *_upload_path,
						long long int *_task_id,
						service_adaptor_error_s *error);

int _dbus_open_download_file(const char *_service_name,
						const char *_storage_path,
						const char *_download_path,
						long long int *_task_id,
						service_adaptor_error_s *error);

int _dbus_open_download_thumbnail(const char *_service_name,
						const char *_storage_path,
						const char *_download_path,
						int _thumbnail_size,
						long long int *_task_id,
						service_adaptor_error_s *error);

int _dbus_close_file_task(const char *_service_name,
						long long int _task_id,
						service_adaptor_error_s *error);

int _dbus_start_upload_file(const char *_service_name,
						long long int _task_id,
						const char *_storage_path,
						bool _need_progress,
						bool _need_state,
						service_adaptor_error_s *error);

int _dbus_start_download_file(const char *_service_name,
						long long int _task_id,
						const char *_storage_path,
						bool _need_progress,
						bool _need_state,
						service_adaptor_error_s *error);

int _dbus_start_download_thumbnail(const char *_service_name,
						long long int _task_id,
						const char *_storage_path,
						int thumbnail_size,
						bool _need_progress,
						bool _need_state,
						service_adaptor_error_s *error);

int _dbus_cancel_upload_file(const char *_service_name,
						long long int _task_id,
						service_adaptor_error_s *error);

int _dbus_cancel_download_file(const char *_service_name,
						long long int _task_id,
						service_adaptor_error_s *error);

int _dbus_cancel_download_thumbnail(const char *_service_name,
						long long int _task_id,
						service_adaptor_error_s *error);

int _dbus_get_privilege_check_result(const char *service_name,
						const char *privilege_name,
						void **server_data,
						service_adaptor_error_s *error);

///////////////////// private feature

#endif /* __TIZEN_SOCIAL_SERVICE_ADAPTOR_DBUS_CLIENT_STORAGE_H__ */
