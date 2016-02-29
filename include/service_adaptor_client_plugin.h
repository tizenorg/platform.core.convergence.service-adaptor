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

#ifndef __TIZEN_SOCIAL_SERVICE_ADAPTOR_CLIENT_PLUGIN_H__
#define __TIZEN_SOCIAL_SERVICE_ADAPTOR_CLIENT_PLUGIN_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup	SERVICE_PLUGIN_MODULE

 * @{
 */

/**
 * @brief Type of service in plugin
 */
typedef enum
{
	SERVICE_PLUGIN_SERVICE_AUTH	= (0x01 << 0),		/**< Auth service type flag */
	SERVICE_PLUGIN_SERVICE_STORAGE	= (0x01 << 1),		/**< Storage service type flag */
}service_plugin_service_type_e;

/**
* @brief Definition for the service_plugin property: The application id be issued from service provider for 3rd party developer.
* @since_tizen 2.4
* @see service_plugin_add_property()
* @see service_plugin_remove_property()
* @see service_plugin_get_property()
 */
#define SERVICE_PLUGIN_PROPERTY_APP_KEY		"http://tizen.org/service-adaptor/plugin/property/app_key"

/**
* @brief Definition for the service_plugin property: The application password be issued from service provider for 3rd party developer.
* @since_tizen 2.4
* @see service_plugin_add_property()
* @see service_plugin_remove_property()
* @see service_plugin_get_property()
*/
#define SERVICE_PLUGIN_PROPERTY_APP_SECRET	"http://tizen.org/service-adaptor/plugin/property/app_secret"

/**
* @brief Definition for the service_plugin property: The user id for using specific service.
* @since_tizen 2.4
* @see service_plugin_add_property()
* @see service_plugin_remove_property()
* @see service_plugin_get_property()
*/
#define SERVICE_PLUGIN_PROPERTY_USER_ID		"http://tizen.org/service-adaptor/plugin/property/user_id"


/**
* @brief The Plugn handle for Service Adaptor
* @details The handle can be created by service_adaptor_create_plugin()<br>
*  When a handle is no longer needed, use service_plugin_destroy()
* @see service_adaptor_create_plugin()
* @see service_plugin_destroy()
*/
typedef struct _service_plugin_s *service_plugin_h;

/**
* @brief	Destroy Service Adaptor
* @details	It must called after a program no longer needs APIs of specfic plugin
* @since_tizen 2.4
*
* @param[in]	plugin	The handle for use Plugin APIs
* @see	service_adaptor_create_plugin()
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
* @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
* @pre	@a plugin must be issued by service_adaptor_create_plugin()
*/
int service_plugin_destroy(service_plugin_h plugin);

/**
* @brief Add Plugin Property
* @details The plguin property is used for plugin's basic or optional requirement.<br>
*  This value is not used in Adaptor layer, but it can be uesd to important Key for plugin with service provider.<br>
* @since_tizen 2.4
*
* @param[in]	plugin	The handle for use Plugin APIs
* @param[in]	key	The key of plugin property
* @param[in]	value	The value of plugin property that matched @a key
* @remarks	Some kind of property key(Not mandatory) is defined in this API (That is named to SERVICE_PLUGIN_PROPERTY_XXX)
* @remarks	If the @a key already exists in the property its current value is replaced with the new @a value.
* @remarks	@a plugin must be released memory using #service_plugin_destroy() when you no longer needs plugin's API
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
* @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
* @see	SERVICE_PLUGIN_PROPERTY_APP_KEY
* @see	SERVICE_PLUGIN_PROPERTY_APP_SECRET
* @see	SERVICE_PLUGIN_PROPERTY_USER_ID
* @pre	@a plugin must be issued by service_adaptor_create_plugin()
*/
int service_plugin_add_property(service_plugin_h plugin,
						const char *key,
						const char *value);

/**
* @brief Remove Plugin Property
* @since_tizen 2.4
*
* @param[in]	plugin	The handle for use Plugin APIs
* @param[in]	key	The key of plugin property
* @remarks	Some kind of property key(Not mandatory) is defined in this API (That is named to SERVICE_PLUGIN_PROPERTY_XXX)
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
* @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
* @see	SERVICE_PLUGIN_PROPERTY_APP_KEY
* @see	SERVICE_PLUGIN_PROPERTY_APP_SECRET
* @see	SERVICE_PLUGIN_PROPERTY_USER_ID
*/
int service_plugin_remove_property(service_plugin_h plugin,
						const char *key);

/**
* @brief Gets Plugin Property
* @since_tizen 2.4
*
* @param[in]	plugin	The handle for use Plugin APIs
* @param[in]	key	The key of plugin property
* @param[out]	value	The value of plugin property that matched @a key
* @remarks	Some kind of property key(Not mandatory) is defined in this API (That is named to SERVICE_PLUGIN_PROPERTY_XXX)
* @remarks	@a value must be released using free()
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_NO_DATA There is no property
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
* @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
* @see	SERVICE_PLUGIN_PROPERTY_APP_KEY
* @see	SERVICE_PLUGIN_PROPERTY_APP_SECRET
* @see	SERVICE_PLUGIN_PROPERTY_USER_ID
* @pre	The function get property already set by service_adaptor_set_plugin_property()
*/
int service_plugin_get_property(service_plugin_h plugin,
						const char *key,
						char **value);

/**
* @brief Requests start initalization for service plugin
* @since_tizen 2.4
*
* @param[in]	plugin		The handle for use Plugin APIs
* @param[in]	service_mask	The flag for use service plugins, this flag can be masked multiple enum (#service_plugin_service_type_e)
* @remarks	@a service_mask must be input using 'bit or' operation with #service_plugin_service_type_e
* @remarks	- for example,
* @remarks	&nbsp;&nbsp;&nbsp;&nbsp;	<b>int</b> @a service_mask |= SERVIE_PLUGIN_SERVICE_AUTH;
* @remarks	&nbsp;&nbsp;&nbsp;&nbsp;	@a service_mask |= SERVICE_PLUGIN_SERVICE_STORAGE;
* @remarks	&nbsp;&nbsp;&nbsp;&nbsp;	<b>int</b> ret = service_plugin_start(@a m_plugin, @a service_mask);
* @remarks	If a program needs to stop plugin manually, use #service_plugin_stop(). <br>But in #service_plugin_destroy(), automatically stop service plugin
* @see		service_plugin_service_type_e
* @see		service_plugin_stop()
* @return 0 on success, otherwise a negative error value
* @return If return value is #SERVICE_ADAPTOR_ERROR_NOT_AUTHORIZED, request authorization to signup application
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
* @retval #SERVICE_ADAPTOR_ERROR_NOT_AUTHORIZED Need authorization
* @retval #SERVICE_ADAPTOR_ERROR_TIMED_OUT Timed out
* @retval #SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE IPC failed with Service Adaptor Daemon
* @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
*/
int service_plugin_start(service_plugin_h plugin,
						int service_mask);

/**
* @brief Requests stop manually for service plugin
* @since_tizen 2.4
*
* @param[in]	plugin		The handle for use Plugin APIs
* @remarks	If a program needs to stop plugin manually, use this function. <br>But in #service_plugin_destroy(), automatically stop service plugin
* @remarks	@a plugin must be released memory using #service_plugin_destroy() when you no longer needs plugin's API
* @see		service_plugin_start()
* @see		service_plugin_destroy()
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_TIMED_OUT Timed out
* @retval #SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE IPC failed with Service Adaptor Daemon
* @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
* @pre	service_plugin_start()
*/
int service_plugin_stop(service_plugin_h plugin);

/**
 * @}
 */




#ifdef __cplusplus
}
#endif
#endif /* __TIZEN_SOCIAL_SERVICE_ADAPTOR_CLIENT_PLUGIN_H__ */
