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
 * File: service-adaptor-client.h
 * Desc: Service Adaptor APIs
 * Created on: Oct, 2014
 * Auth: Yongjin Kim <youth.kim@samsung.com>
 *
 *****************************************************************************/
/**
 *	@file		service-adaptor-client.h
 *	@brief		Defines interface of Service Adaptor
 *	@version	0.1
 */

#ifndef __PRIVATE_SERVICE_ADAPTOR_CLIENT_H__
#define __PRIVATE_SERVICE_ADAPTOR_CLIENT_H__

#include <stdbool.h>
#include <service_adaptor_client.h>
#include "service_adaptor_client_type.h"

/**
 * @brief Enumerations of result code for Service Adaptor
 */
typedef enum _service_adaptor_result_e {
	SERVICE_ADAPTOR_RESULT_SUCCEEDED		= SERVICE_ADAPTOR_ERROR_NONE,	/**< specifies status as none*/
	SERVICE_ADAPTOR_RESULT_FAILED			= -1,	/**< specifies status as none*/
	SERVICE_ADAPTOR_RESULT_CANCELED			= -2	/**< specifies status as none*/
} __service_adaptor_result_e;	/*TODO deplicate*/

typedef int service_adaptor_result_e; /* For version compatibility */

/**
 * @brief Enumerations of error code for Service Adaptor
 */
typedef enum _private_service_adaptor_error_e {
	SERVICE_ADAPTOR_ERROR_LAUNCH			= 1,	/**< 1 ~ 99: internal error*/
	SERVICE_ADAPTOR_ERROR_INIT			= 2,
	SERVICE_ADAPTOR_ERROR_DEINIT			= 3,
	SERVICE_ADAPTOR_ERROR_CREATE			= 4,
	SERVICE_ADAPTOR_ERROR_DESTROY			= 5,
	SERVICE_ADAPTOR_ERROR_START			= 6,
	SERVICE_ADAPTOR_ERROR_STOP			= 7,
	SERVICE_ADAPTOR_ERROR_CONNECT			= 8,
	SERVICE_ADAPTOR_ERROR_DISCONNECT		= 9,
	SERVICE_ADAPTOR_ERROR_NOT_FOUND			= 10,
	SERVICE_ADAPTOR_ERROR_CORRUPTED			= 11,
	SERVICE_ADAPTOR_ERROR_UNSUPPORTED		= 12,
	SERVICE_ADAPTOR_ERROR_INVALID_HANDLE		= 13,
	SERVICE_ADAPTOR_ERROR_INVALID_ARGUMENT		= 14,
	SERVICE_ADAPTOR_ERROR_INVALID_ARGUMENT_TYPE	= 15,
	SERVICE_ADAPTOR_ERROR_ADAPTOR_INTERNAL		= 17,
	SERVICE_ADAPTOR_ERROR_PLUGIN_INTERNAL		= 18,
	SERVICE_ADAPTOR_ERROR_SERVER_INTERNAL		= 19,
	SERVICE_ADAPTOR_ERROR_DBUS			= 20,
	SERVICE_ADAPTOR_ERROR_CALLBACK_TIME_OUT		= 21,
	SERVICE_ADAPTOR_ERROR_INTERNAL_MAX		= 99,

	SERVICE_ADAPTOR_ERROR_NETWORK			= TIZEN_ERROR_SERVICE_ADAPTOR | 0xf1,
} private_service_adaptor_error_e;

/**
 * @brief Type of service in plugin (ext internal)
 */
typedef enum {
	SERVICE_PLUGIN_SERVICE_CONTACT	= (0x01 << 2),		/**< Contact service type flag */
	SERVICE_PLUGIN_SERVICE_MESSAGE	= (0x01 << 3),		/**< Message service type flag */
	SERVICE_PLUGIN_SERVICE_SHOP	= (0x01 << 4),		/**< Shop service type flag */
	SERVICE_PLUGIN_SERVICE_PUSH	= (0x01 << 5),		/**< Push service type flag */
} internal_service_plugin_service_type_e;

/**
* @brief Describes infromation about plugin
*/
typedef struct _service_adaptor_plugin_s {
	char *name;				/**< specifies status as none*/
	bool login;				/**< specifies status as none*/
} service_adaptor_plugin_s;

/**
* @brief The handle for Service Adaptor error
*/
typedef service_adaptor_plugin_s *service_adaptor_plugin_h;

/**
* @brief Callback for service_adaptor_connect API
*
* @param[in]	signal		specifies signal code
* @param[in]	msg		specifies signal message
* @return       void.
* @pre	service_adaptor_connect() will invoke this callback.
* @see
*/
typedef void(*service_adaptor_signal_cb)(service_adaptor_h handle,
						service_adaptor_signal_code_e signal,
						char *msg);


/*==================================================================================================
					FUNCTION PROTOTYPES
==================================================================================================*/


#endif /* __PRIVATE_SERVICE_ADAPTOR_CLIENT_H__ */
