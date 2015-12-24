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

#ifndef __TIZEN_SOCIAL_SERVICE_ADAPTOR_CLIENT_H__
#define __TIZEN_SOCIAL_SERVICE_ADAPTOR_CLIENT_H__

#include <bundle.h>
#include <service_adaptor_type.h>
#include <service_adaptor_client_plugin.h>
#include <service_adaptor_client_storage.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file service_adaptor_client.h
 */

/**
 * @addtogroup	SERVICE_PLUGIN_MODULE
 * @{
 */

/**
* @brief The handle for connection and managing plugin handle of Service Adaptor
* @details The handle can be created by service_adaptor_create()<br>
*  When a handle is no longer needed, use service_adaptor_destroy()
* @since_tizen 2.4
*
* @see #service_adaptor_create()
* @see #service_adaptor_destroy()
*/
typedef struct _service_adaptor_s *service_adaptor_h;

/**
* @since_tizen 2.4
* @brief Callback for service_adaptor_foreach_plugin API
*
* @param[in]	plugin_uri	The service plugin's unique uri, this value be set by plugin
* @param[in]	service_mask	Masked value for <b>installed</b> service plugins, this value can be masked multiple enum (#service_plugin_service_type_e)
* @param[in]	user_data	Passed data from #service_adaptor_foreach_plugin()
* @remarks	@a service_mask check using 'bit and' operation with #service_plugin_service_type_e
* @remarks	- for example,
* @remarks	&nbsp;&nbsp;&nbsp;&nbsp;	if(@a service_mask & SERVICE_PLUGIN_SERVICE_STORAGE)
* @remarks	&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;	{ @a USING_STORAGE_CODE }
* @remarks Do not release memory of @a plugin_uri
* @see		service_plugin_service_type_e
* @return @c true to continue with the next iteration of the loop,
*         otherwise @c false to break out of the loop
* @pre	service_adaptor_foreach_plugin() will invoke this callback.
*/
typedef bool (*service_adaptor_plugin_cb)(char *plugin_uri,
						int service_mask,
						void *user_data);
/*==================================================================================================
                                         FUNCTION PROTOTYPES
==================================================================================================*/

/**
* @brief Create Service Adaptor
* @since_tizen 2.4
*
* @param[out]	service_adaptor	The Service Adaptor handle
* @remarks	@a service_adaptor must be released memory using service_adaptor_destroy(), when a program no longer needs any function of Service Adaptor
* @see		service_adaptor_destroy()
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
* @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
*/
int service_adaptor_create(service_adaptor_h *service_adaptor);

/**
* @brief	Destroy Service Adaptor
* @details	It must called after a program no longer needs any function of Service Adaptor
* @since_tizen 2.4
*
* @param[in]	service_adaptor	The handle of Service Adaptor
* @see		service_adaptor_create()
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
* @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
* @pre	@a service_adaptor must be issued by service_adaptor_create()
*/
int service_adaptor_destroy(service_adaptor_h service_adaptor);

/**
* @brief Foreach the list of plugin
* @details Iterate to all installed plugin
* @since_tizen 2.4
*
* @param[in]	service_adaptor	The handle of Service Adaptor
* @param[in]	callback	The callback for foreach plugin
* @param[in]	user_data	Passed data to callback
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_NO_DATA There is no available plugins
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
* @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
* @pre	@a service_adaptor must be issued by service_adaptor_create()
*/
int service_adaptor_foreach_plugin(service_adaptor_h service_adaptor,
						service_adaptor_plugin_cb callback,
						void *user_data);

/**
* @brief Create service plugin handle
* @details Create plugin handle using @a plugin_uri
* @since_tizen 2.4
*
* @param[in]	service_adaptor	The handle of Service Adaptor
* @param[in]	plugin_uri	The specfic string for use plugin, this values are set by plugin
* @param[out]	plugin		The handle for use Plugin APIs
* @remarks	@a plugin must be released memory using service_plugin_destroy() when you no longer needs plugin's API
* @see		service_plugin_destroy()
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
* @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
* @pre	@a service_adaptor must be issued by service_adaptor_create()
*/
int service_adaptor_create_plugin(service_adaptor_h service_adaptor,
						const char *plugin_uri,
						service_plugin_h *plugin);

/**
* @brief Gets service specfic last result
* @details This function retrieves the last error code that be issued from plugin.<br>
*  When if API function returns #SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED, gets using this function.
* @since_tizen 2.4
*
* @param[out]	err	The error number that is defined service plugin SPEC
* @remarks	Thread safe functions
* @remarks	The result number's detail specification is defined service plugin or provider.
* @remarks	The detail error message can be got using service_adaptor_get_last_error_message()
* @see		service_adaptor_get_last_error_message()
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
* @retval #SERVICE_ADAPTOR_ERROR_NO_DATA There is no result
* @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
*/
int service_adaptor_get_last_result(int *err);

/**
* @brief Gets service specfic last result error message
* @details This function retrieves the last error code that be issued from plugin.<br>
*  When if API function returns #SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED, gets using this function.
* @since_tizen 2.4
*
* @param[out]	message	The error message that is defined service plugin SPEC
* @remarks	@a message must be released using free()
* @remarks	Thread safe functions
* @remarks	The result string's detail specification is defined service plugin or provider.
* @see		service_adaptor_get_last_result()
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
* @retval #SERVICE_ADAPTOR_ERROR_NO_DATA There is no error message
* @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
*/
int service_adaptor_get_last_error_message(char **message);

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
*/
int service_plugin_get_property(service_plugin_h plugin,
						const char *key,
						char **value);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif /* __TIZEN_SOCIAL_SERVICE_ADAPTOR_CLIENT_H__ */
