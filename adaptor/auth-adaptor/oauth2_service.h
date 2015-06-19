/*
 * oAuth 2.0 Service
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

#ifndef __OAUTH2_SERVICE_H__
#define __OAUTH2_SERVICE_H__

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
* @brief Callback for oauth2_get_access_token API
*
* @param[in]    access_token
* @param[in]    user_data       Passed data from #oauth2_get_access_token()
* @remarks
* @pre  oauth2_get_access_token() will invoke this callback.
*/
typedef void (*oauth2_get_access_token_cb)(const char *access_token, void *user_data);

/**
* @brief Callback for oauth2_get_extra_data API
*
* @param[in]    value
* @param[in]    user_data       Passed data from #oauth2_get_access_token()
* @remarks
* @pre  oauth2_get_extra_data() will invoke this callback.
*/
typedef void (*oauth2_get_extra_data_cb)(const char *value, void *user_data);

/**
 * @brief Describes infromation about oAuth 2.0 Service
 * @key access token, ...
 */
typedef struct _oauth2_service_s
{
	int (*oauth2_get_access_token)(void *plugin, char **access_token, void *user_data);
	int (*oauth2_get_extra_data)(void *plugin, const char *key, char **value, void *user_data);
} oauth2_service_s;
typedef struct _oauth2_service_s *oauth2_service_h;

/*==================================================================================================
                                         FUNCTION PROTOTYPES
==================================================================================================*/

/**
* @brief Register oAuth 2.0 Service
* @since_tizen 3.0
*
* @param[in]	oauth2	The oAuth 2.0 service handle
* @param[in]	service
* @remarks
* @see		oauth2_unregister_service()
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
* @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
*/
int oauth2_register_service(oauth2_service_h oauth2, GHashTable *service);

/**
* @brief Unregister oAuth 2.0 Service
* @since_tizen 3.0
*
* @param[in]	oauth2	The oAuth 2.0 service handle
* @remarks
* @see		oauth2_register_service()
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
* @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
*/
int oauth2_unregister_service(oauth2_service_h oauth2);

#ifdef __cplusplus
}
#endif

#endif /* __OAUTH2_SERVICE_H__ */
