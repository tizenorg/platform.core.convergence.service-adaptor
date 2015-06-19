/*
 * POSIX Service
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

#ifndef __POSIX_SERVICE_H__
#define __POSIX_SERVICE_H__

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
typedef struct _posix_service_s
{
	int (*posix_open_service)(void *plugin,
			const char *path,
			int flags,
			void *user_data);

	int (*posix_read_service)(void *plugin,
			const char *path,
			char **buf,
			size_t size,
			off_t offset,
			void *user_data);

	int (*posix_write_service)(void *plugin,
			const char *path,
			const char *buf,
			size_t size,
			off_t offset,
			void *user_data);

	int (*posix_close_service)(void *plugin,
			const char *path,
			int flags,
			void *user_data);
} posix_service_s;
typedef struct _posix_service_s *posix_service_h;

/*==================================================================================================
                                         FUNCTION PROTOTYPES
==================================================================================================*/

/**
* @brief Register Posix Service
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
int posix_register_service(posix_service_h posix, GHashTable *service);

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
int posix_unregister_service(posix_service_h posix);

#ifdef __cplusplus
}
#endif

#endif /* __POSIX_SERVICE_H__ */
