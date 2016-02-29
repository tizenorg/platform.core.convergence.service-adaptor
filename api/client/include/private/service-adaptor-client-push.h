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
 * File: service-adaptor-client-push.h
 * Desc: Service Adaptor APIs
 * Created on: Oct, 2014
 * Auth: Yongjin Kim <youth.kim@samsung.com>
 *
 *****************************************************************************/
/**
 *	@file		service-adaptor-client-push.h
 *	@brief		Defines interface of Service Adaptor's Push
 *	@version	0.1
 */

#ifndef __SERVICE_ADAPTOR_CLIENT_PUSH_H__
#define __SERVICE_ADAPTOR_CLIENT_PUSH_H__

#include "service-adaptor-client.h"
#include "service_adaptor_client_type.h"

/**
* @brief Describes push information
*/
typedef struct _service_adaptor_push_notification_s
{
	char *data;			/**< specifies status as none*/
	char *message;			/**< specifies status as none*/
	long long int time;		/**< specifies status as none*/
} service_adaptor_push_notification_s;

/**
* @brief Callback for service_adaptor_connect_push_service API
*
* @param[in]	handle		specifies Service Adaptor handle. use this handle to get internal data
* @param[in]	noti_info	specifies push_notification information
* @param[in]	error_code	specifies error code
* @param[in]	user_data	specifies user_data passed in service_adaptor_connect_push_service API
* @return	void
* @pre	service_adaptor_connect_push_service will invoke this callback.
* @see
*/
typedef void(* service_adaptor_push_notification_cb)(service_adaptor_h handle,
						service_adaptor_push_notification_s *noti_info,
						service_adaptor_error_s *error_code,
						void *user_data);

/*==================================================================================================
                                         FUNCTION PROTOTYPES
==================================================================================================*/

/**
* @brief Registers a callback function to receive push notification from push service
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	callback	the callback function to invoke
* @param[out]	error_code	specifies error code
* @param[in]	user_data	specifies user_data (json) passed in API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_RESULT_SUCCEEDED if Successful
*/
int service_adaptor_connect_push_service(service_adaptor_h handle,
						service_adaptor_push_notification_cb callback,
						service_adaptor_error_s **error_code,
						void *user_data);

/**
* @brief Deregisters the callback function that had been registered to push service
*
* @param[in]	handle		specifies Service Adaptor handle
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_RESULT_SUCCEEDED if Successful
*/
int service_adaptor_disconnect_push_service(service_adaptor_h handle);


int service_adaptor_register_push_service(service_adaptor_h handle,
						const char *service_file_name);

int service_adaptor_deregister_push_service(service_adaptor_h handle,
						const char *service_file_name);

#endif /* __SERVICE_ADAPTOR_CLIENT_PUSH_H__ */
