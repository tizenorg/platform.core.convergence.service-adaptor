/*
 * oAuth 1.0 Service
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

#ifndef __OAUTH1_SERVICE_H__
#define __OAUTH1_SERVICE_H__

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <glib.h>

/**
 * @brief Describes infromation about oAuth1 Service
 * @key name, ...
 */
typedef struct _oauth1_s
{
	char *access_token;
	char *operation;
} oauth1_s;
typedef struct _oauth1_s *oauth1_h;

/**
* @brief Callback for oauth1 API
*
* @param[in]    access_token
* @param[in]    user_data       Passed data from #oauth1_get_access_token()
* @remarks
* @pre  oauth1() will invoke this callback.
*/
typedef void (*oauth1_cb)(int result, oauth1_h oauth1, void *user_data);

/**
 * @brief Describes infromation about oAuth 1.0 Service
 * @key access token, ...
 */
typedef struct _oauth1_service_s
{
	int (*oauth1_get_access_token)(void *plugin, oauth1_cb callback, void *user_data);
	int (*oauth1_get_extra_data)(void *plugin, const char *key, oauth1_cb callback, void *user_data);
} oauth1_service_s;
typedef struct _oauth1_service_s *oauth1_service_h;

/*==================================================================================================
                                         FUNCTION PROTOTYPES
==================================================================================================*/

/**
* @brief Register oAuth 1.0 Service
* @since_tizen 3.0
*
* @param[in]	oauth1	The oAuth 1.0 service handle
* @param[in]	service
* @remarks
* @see		oauth1_unregister_service()
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
* @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
*/
int oauth1_register_service(oauth1_service_h oauth1, GHashTable *service);

/**
* @brief Unregister oAuth 1.0 Service
* @since_tizen 3.0
*
* @param[in]	oauth1	The oAuth 1.0 service handle
* @remarks
* @see		oauth1_register_service()
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
* @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
*/
int oauth1_unregister_service(oauth1_service_h oauth1);

#ifdef __cplusplus
}
#endif

#endif /* __OAUTH1_SERVICE_H__ */
