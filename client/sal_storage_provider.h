/*
 * Storage Plugin Client
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

#ifndef __STORAGE_PLUGIN_CLIENT_H__
#define __STORAGE_PLUGIN_CLIENT_H__

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#include <app.h>

#include "service_adaptor_errors.h"

#define CLOUD_LOCAL_PATH_KEY	"local_path"
#define CLOUD_CLOUD_PATH_KEY	"cloud_path"

/**
 * @brief Describes infromation about Cloud Service
 * @key name, ...
 */
#define CLOUD_REMOVE_FILE_URI   "http://tizen.org/service-adaptor/storage/cloud/remove_file"
#define CLOUD_DOWNLOAD_FILE_URI "http://tizen.org/service-adaptor/storage/cloud/download_file"
#define CLOUD_UPLOAD_FILE_URI   "http://tizen.org/service-adaptor/storage/cloud/upload_file"
#define CLOUD_DOWNLOAD_FILE_THUMBNAIL_URI       "http://tizen.org/service-adaptor/storage/cloud/download_file_thumbnail"
#define CLOUD_GET_FILE_LIST_URI "http://tizen.org/service-adaptor/storage/cloud/get_file_list"

/**
 * @brief Describes infromation about Storage Plugin Handle
 */
typedef struct _storage_provider_s
{
	// Cloud
	service_adaptor_error_e (*cloud_remove_file)(const char *cloud_path);

	// Posix
} storage_provider_s;
typedef struct _storage_provider_s *storage_provider_h;

int storage_provider_create(storage_provider_h *provider);
app_control_h storage_provider_message(storage_provider_h provider, const char *operation, void *user_data);
int storage_provider_add_extra_data(storage_provider_h provider, app_control_h reply);

#ifdef __cplusplus
}
#endif

#endif /* __STORAGE_PLUGIN_CLIENT_H__ */
