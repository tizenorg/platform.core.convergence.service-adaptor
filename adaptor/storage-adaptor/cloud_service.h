/*
 * Cloud Service
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

#ifndef __CLOUD_SERVICE_H__
#define __CLOUD_SERVICE_H__

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#include <glib.h>

/**
 * @brief Describes infromation about Cloud Service
 * @key name, ...
 */
typedef struct _cloud_file_s
{
	bool is_dir;
	char *dir_path;
	char *local_path;
	char *cloud_path;
	unsigned long long size;
	char *operation;

	GList *files;
} cloud_file_s;
typedef struct _cloud_file_s *cloud_file_h;

/**
* @brief Callback for Cloud File Operation API
*
* @param[in]    plugin
* @param[in]    path
* @param[in]    callback
* @param[in]    user_data       Passed data from #oauth1_get_access_token()
* @remarks
* @pre  cloud_remove_file() will invoke this callback.
*/
typedef void (*cloud_file_cb)(int result, cloud_file_h cloud_file, void *user_data);

/**
 * @brief Describes infromation about Cloud Service
 * @key name, ...
 */
typedef struct _cloud_service_s
{
	int (*cloud_remove_file)(void *plugin, const char *cloud_path, cloud_file_cb callback, void *user_data);
	int (*cloud_download_file)(void *plugin, const char *cloud_path, const char *local_path, cloud_file_cb callback, void *user_data);
	int (*cloud_upload_file)(void *plugin, const char *local_path, const char *cloud_path, cloud_file_cb callback, void *user_data);
	int (*cloud_download_file_thumbnail)(void *plugin, const char *cloud_path, const char *local_path, cloud_file_cb callback, void *user_data);
	int (*cloud_get_file_list)(void *plugin, const char *cloud_path, const char *local_path, cloud_file_cb callback, void *user_data);
} cloud_service_s;
typedef struct _cloud_service_s *cloud_service_h;

/*==================================================================================================
                                         FUNCTION PROTOTYPES
==================================================================================================*/

/**
* @brief Register Cloud Service
* @since_tizen 3.0
*
* @param[in]    cloud  The Cloud service handle
* @param[in]    service
* @remarks
* @see          cloud_unregister_service()
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
* @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
*/
int cloud_register_service(cloud_service_h cloud, GHashTable *service);

/**
* @brief Unregister Cloud Service
* @since_tizen 3.0
*
* @param[in]    cloud  The Cloud service handle
* @remarks
* @see          cloud_register_service()
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
* @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
*/
int cloud_unregister_service(cloud_service_h cloud);

#ifdef __cplusplus
}
#endif

#endif /* __CLOUD_SERVICE_H__ */
