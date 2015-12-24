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

#ifndef __TIZEN_SOCIAL_SERVICE_ADAPTOR_TYPE_H__
#define __TIZEN_SOCIAL_SERVICE_ADAPTOR_TYPE_H__

#include <tizen.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file service_adaptor_type.h
 */

/**
 * @addtogroup	SERVICE_ADAPTOR_MODULE
 * @{
 */

/**
 * @brief Enumerations of result code for Service Adaptor
 * @since_tizen 2.4
 */
typedef enum
{
	SERVICE_ADAPTOR_ERROR_NONE					= TIZEN_ERROR_NONE,						/**< Success */
	SERVICE_ADAPTOR_ERROR_NOT_SUPPORTED			= TIZEN_ERROR_NOT_SUPPORTED,			/**< Service plugin does not support API */
	SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER		= TIZEN_ERROR_INVALID_PARAMETER,		/**< The parameter is invalid */
	SERVICE_ADAPTOR_ERROR_TIMED_OUT				= TIZEN_ERROR_TIMED_OUT,				/**< API time out */
	SERVICE_ADAPTOR_ERROR_NO_DATA				= TIZEN_ERROR_NO_DATA,					/**< There is no data available */
	SERVICE_ADAPTOR_ERROR_PERMISSION_DENIED		= TIZEN_ERROR_PERMISSION_DENIED,		/**< Permission denied */
	SERVICE_ADAPTOR_ERROR_UNKNOWN				= TIZEN_ERROR_UNKNOWN,					/**< Unknown error */
	SERVICE_ADAPTOR_ERROR_OUT_OF_MEMORY			= TIZEN_ERROR_OUT_OF_MEMORY,			/**< Out of memory */
	SERVICE_ADAPTOR_ERROR_NETWORK_UNREACHABLE	= TIZEN_ERROR_NETWORK_UNREACHABLE,		/**< Network is unreachable */
	SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE			= TIZEN_ERROR_SERVICE_ADAPTOR | 0x01,	/**< IPC Connection unstabled */
	SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED			= TIZEN_ERROR_SERVICE_ADAPTOR | 0x02,	/**< The error occured from Plugin, See detail from service_adaptor_get_last_result() and Plugin SPEC */
	SERVICE_ADAPTOR_ERROR_NOT_AUTHORIZED		= TIZEN_ERROR_SERVICE_ADAPTOR | 0x03,	/**< Need Authorization */
	SERVICE_ADAPTOR_ERROR_INVALID_STATE			= TIZEN_ERROR_SERVICE_ADAPTOR | 0x04,	/**< The handle state is invalid for processing API */
}service_adaptor_error_e;

/**
 * @brief Type of service in plugin
 * @since_tizen 2.4
 */
typedef enum
{
	SERVICE_PLUGIN_SERVICE_AUTH	= (0x01 << 0),		/**< Auth service type flag */
	SERVICE_PLUGIN_SERVICE_STORAGE	= (0x01 << 1),		/**< Storage service type flag */
}service_plugin_service_type_e;

/**
* @brief Definition for the service_plugin property: The application id be issued from service provider for 3rd party developer.
* @since_tizen 2.4
*
* @see service_plugin_add_property()
* @see service_plugin_remove_property()
* @see service_plugin_get_property()
 */
#define SERVICE_PLUGIN_PROPERTY_APP_KEY		"http://tizen.org/service-adaptor/plugin/property/app_key"

/**
* @brief Definition for the service_plugin property: The application password be issued from service provider for 3rd party developer.
* @since_tizen 2.4
*
* @see service_plugin_add_property()
* @see service_plugin_remove_property()
* @see service_plugin_get_property()
*/
#define SERVICE_PLUGIN_PROPERTY_APP_SECRET	"http://tizen.org/service-adaptor/plugin/property/app_secret"

/**
* @brief Definition for the service_plugin property: The user id for using specific service.
* @since_tizen 2.4
*
* @see service_plugin_add_property()
* @see service_plugin_remove_property()
* @see service_plugin_get_property()
*/
#define SERVICE_PLUGIN_PROPERTY_USER_ID		"http://tizen.org/service-adaptor/plugin/property/user_id"

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif /* __TIZEN_SOCIAL_SERVICE_ADAPTOR_TYPE_H__ */
