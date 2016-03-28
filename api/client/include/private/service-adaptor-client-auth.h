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
 * File: service-adaptor-client-auth.h
 * Desc: Service Adaptor APIs
 * Created on: Oct, 2014
 * Auth: Yongjin Kim <youth.kim@samsung.com>
 *
 *****************************************************************************/
/**
 *	@file		service-adaptor-client-auth.h
 *	@brief		Defines interface of Service Adaptor's Auth
 *	@version	0.1
 */

#ifndef __PRIVATE_SERVICE_ADAPTOR_CLIENT_AUTH_H__
#define __PRIVATE_SERVICE_ADAPTOR_CLIENT_AUTH_H__

#include "service-adaptor-client.h"
#include "service_adaptor_client_type.h"

/*==================================================================================================
					FUNCTION PROTOTYPES
==================================================================================================*/

/**
* @brief Gets Auth Plugin List
*
* @param[in]	handle		the handle which is returned in service_adaptor_connect
* @param[out]	plugins		list of auth plugins including plugin name and available
* @param[out]	plugins_len	length of plugins
* @param[out]	error_code	specifies error code
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_RESULT_SUCCEEDED if Successful
*/
int service_adaptor_get_auth_plugin(service_adaptor_h handle,
						service_adaptor_plugin_h **plugins,
						unsigned int *plugins_len,
						service_adaptor_error_s **error_code);

/**
* @brief Sets Plugin
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	plugin_handle	The handle which is returned in service_adaptor_get_auth_plugin
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_RESULT_SUCCEEDED if Successful
*/
int service_adaptor_set_auth_plugin(service_adaptor_h handle,
						service_adaptor_plugin_h plugin_handle);

/**
* @brief Requests Channel Auth
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	service_name	specifies service name like "com.serviceadaptor.message"
* @param[in]	app_id		specifies app id
* @param[in]	app_secret	specifies app secret
* @param[in]    service_id      specifies service id (0: contact, 1: free message)
* @param[in]	imsi		specifies imsi
* @param[out]	error_code	specifies error code
* @param[in]	user_data	specifies user_data (json) passed in API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_RESULT_SUCCEEDED if Successful
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
* @retval #SERVICE_ADAPTOR_ERROR_NOT_AUTHORIZED Need authorization
* @retval #SERVICE_ADAPTOR_ERROR_NETWORK Failed by network
* @retval #SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE IPC failed with Service Adaptor Daemon
* @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
*/
int service_adaptor_set_auth(service_adaptor_h handle,
						const char *service_name,
						const char *app_id,
						const char *app_secret,
						unsigned int service_id,
						service_adaptor_error_s **error_code,
						void *user_data);

#endif /* __PRIVATE_SERVICE_ADAPTOR_CLIENT_AUTH_H__ */
