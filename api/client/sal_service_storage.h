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

#ifndef __SERVICE_STORAGE_H__
#define __SERVICE_STORAGE_H__

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C"
{
#endif
/*
#include "sal_service_adaptor.h"
#include "sal_service_task.h"

#define SERVICE_STORAGE_CLOUD_REMOVE_FILE_URI   "http://tizen.org/service-adaptor/storage/cloud/remove_file"
#define SERVICE_STORAGE_CLOUD_DOWNLOAD_FILE_URI "http://tizen.org/service-adaptor/storage/cloud/download_file"
#define SERVICE_STORAGE_CLOUD_UPLOAD_FILE_URI   "http://tizen.org/service-adaptor/storage/cloud/upload_file"
#define SERVICE_STORAGE_CLOUD_DOWNLOAD_FILE_THUMBNAIL_URI       "http://tizen.org/service-adaptor/storage/cloud/download_file_thumbnail"
#define SERVICE_STORAGE_CLOUD_GET_FILE_LIST_URI "http://tizen.org/service-adaptor/storage/cloud/get_file_list"

typedef struct _service_storage_cloud_file_s *service_storage_cloud_file_h;

typedef bool (*service_storage_cloud_file_cb)(int result, service_storage_cloud_file_h file, void *user_data);
*/
/*==================================================================================================
                                         FUNCTION PROTOTYPES
==================================================================================================*/
/*
int service_storage_cloud_file_create(service_plugin_h plugin, service_storage_cloud_file_h *file);
int service_storage_cloud_file_clone(service_storage_cloud_file_h src_file, service_storage_cloud_file_h *dst_file);

int service_storage_cloud_file_destroy(service_storage_cloud_file_h file);
int service_storage_cloud_file_set_callback(service_storage_cloud_file_h file, service_storage_cloud_file_cb callback, void *user_data);
int service_storage_cloud_file_unset_callback(service_storage_cloud_file_h file);
int service_storage_cloud_file_set_cloud_path(service_storage_cloud_file_h file, const char *cloud_path);
int service_storage_cloud_file_get_cloud_path(service_storage_cloud_file_h file, char **cloud_path);
int service_storage_cloud_file_set_local_path(service_storage_cloud_file_h file, const char *local_path);
int service_storage_cloud_file_get_local_path(service_storage_cloud_file_h file, char **local_path);
int service_storage_cloud_file_set_size(service_storage_cloud_file_h file, unsigned long long size);
int service_storage_cloud_file_get_size(service_storage_cloud_file_h file, unsigned long long *size);
int service_storage_cloud_file_set_operation(service_storage_cloud_file_h file, const char *operation);
int service_storage_cloud_file_get_operation(service_storage_cloud_file_h file, char **operation);
int service_storage_cloud_file_is_directory(service_storage_cloud_file_h file, bool *is_dir);
int service_storage_cloud_file_foreach_file(service_storage_cloud_file_h file, service_storage_cloud_file_cb callback, void *user_data);
int service_storage_cloud_file_create_task(service_storage_cloud_file_h file, service_task_h *task);
int service_storage_cloud_file_destroy_task(service_task_h task);

// 2.4
*/

#ifdef __cplusplus
}
#endif

#endif /* __SERVICE_STORAGE_H__ */
