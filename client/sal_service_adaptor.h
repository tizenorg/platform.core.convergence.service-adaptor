/*
 * Service Adaptor
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

#ifndef __SERVICE_ADAPTOR_H__
#define __SERVICE_ADAPTOR_H__

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#include <glib.h>

/**
 * @addtogroup  SERVICE_ADAPTOR_MODULE
 * @{
 */

/**
 * @brief Type of service in plugin
 */
typedef enum _service_plugin_type_e
{
	SERVICE_PLUGIN_AUTH     = (0x01 << 0),          /**< Auth service type flag */
	SERVICE_PLUGIN_STORAGE  = (0x01 << 1),          /**< Storage service type flag */
} service_plugin_type_e;

/** 2.4
 * @brief Type of service in plugin
 */
typedef enum _service_plugin_service_type_e
{
        SERVICE_PLUGIN_SERVICE_AUTH     = (0x01 << 0),          /**< Auth service type flag */
        SERVICE_PLUGIN_SERVICE_STORAGE  = (0x01 << 1),          /**< Storage service type flag */
} service_plugin_service_type_e;

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

/** 2.4
* @brief The handle for connection and managing plugin handle of Service Adaptor
* @details The handle can be created by service_adaptor_create()<br>
*  When a handle is no longer needed, use service_adaptor_destroy()
* @see #service_adaptor_create()
* @see #service_adaptor_destroy()
*/
typedef struct _service_adaptor_s *service_adaptor_h;

/**
 * @brief The handle for connection and managing handle of Service Plugin
 * @details The handle can be created by service_plugin_create()<br>
 *  When a handle is no longer needed, use service_plugin_destroy()
 * @see #service_plugin_create()
 * @see #service_plugin_destroy()
 */
typedef struct _service_plugin_s *service_plugin_h;

/**
 * @brief Callback for service_adaptor_foreach_plugin API
 *
 * @param[in]    uri      The service plugin's unique uri, this value be set by plugin
 * @param[in]    service_mask    Masked value for <b>installed</b> service plugins, this value can be masked multiple enum (#service_plugin_service_type_e)
 * @param[in]    user_data       Passed data from #service_adaptor_foreach_plugin()
 * @remarks      @a service_mask check using 'bit and' operation with #service_plugin_service_type_e
 * @remarks      - for example,
 * @remarks      &nbsp;&nbsp;&nbsp;&nbsp;        if(@a service_mask & SERVICE_PLUGIN_SERVICE_STORAGE)
 * @remarks      &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;        { @a USING_STORAGE_CODE }
 * @remarks Do not release memory of @a plugin_uri
 * @see          service_plugin_service_type_e
 * @return @c true to continue with the next iteration of the loop,
 *         otherwise @c false to break out of the loop
 * @pre  service_adaptor_foreach_plugin() will invoke this callback.
 */
typedef bool (*service_adaptor_plugin_cb)(const char *uri,
		int service_mask,
		void *user_data);

/**
 * @brief Callback for service_plugin_login API
 *
 * @param[in]    result
 * @param[in]    user_data       Passed data from #service_plugin_login()
 * @remarks
 * @return void
 * @pre  service_plugin_login() will invoke this callback.
 */
typedef void (*service_plugin_login_cb)(int result,
		void *user_data);

/*==================================================================================================
                                         FUNCTION PROTOTYPES
==================================================================================================*/

/** 2.4
* @brief Create Service Adaptor
* @since_tizen 2.4
*
* @param[out]   service_adaptor The Service Adaptor handle
* @remarks      @a service_adaptor must be released memory using service_adaptor_destroy(), when a program no longer needs any function of Service Adaptor
* @see          service_adaptor_destroy()
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
* @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
*/
int service_adaptor_create(service_adaptor_h *service_adaptor);

/** 2.4
* @brief        Destroy Service Adaptor
* @details      It must called after a program no longer needs any function of Service Adaptor
* @since_tizen 2.4
*
* @param[in]    service_adaptor The handle of Service Adaptor
* @see          service_adaptor_create()
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
* @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
* @pre  @a service_adaptor must be issued by service_adaptor_create()
*/
int service_adaptor_destroy(service_adaptor_h service_adaptor);

/**
 * @brief Create Service Adaptor
 * @since_tizen	2.4
 *
 * @param[out]	service_adaptor The Service Adaptor handle
 * @remarks	@a service_adaptor must be released memory using service_adaptor_destroy(), when a program no longer needs any function of Service Adaptor
 * @see		service_adaptor_destroy()
 * @return 0 on success, otherwise a negative error value
 * @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
 * @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
 */
int service_adaptor_connect();

/**
 * @brief	Destroy Service Adaptor
 * @details	It must called after a program no longer needs any function of Service Adaptor
 * @since_tizen	2.4
 *
 * @param[in]	service_adaptor The handle of Service Adaptor
 * @see		service_adaptor_create()
 * @return 0 on success, otherwise a negative error value
 * @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
 * @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
 * @pre  @a service_adaptor must be issued by service_adaptor_create()
 */
int service_adaptor_disconnect();

/** 2.4
* @brief Foreach the list of plugin
* @details Iterate to all installed plugin
* @since_tizen 2.4
*
* @param[in]    service_adaptor The handle of Service Adaptor
* @param[in]    callback        The callback for foreach plugin
* @param[in]    user_data       Passed data to callback
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_NO_DATA There is no available plugins
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
* @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
* @pre  @a service_adaptor must be issued by service_adaptor_create()
*/
int service_adaptor_foreach_plugin(service_adaptor_h service_adaptor,
                                                service_adaptor_plugin_cb callback,
                                                void *user_data);

/**
 * @brief Foreach the list of plugin
 * @details Iterate to all installed plugin
 * @since_tizen 2.4
 *
 * @param[in]    callback        The callback for foreach plugin
 * @param[in]    user_data       Passed data to callback
 * @return 0 on success, otherwise a negative error value
 * @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
 * @retval #SERVICE_ADAPTOR_ERROR_NO_DATA There is no available plugins
 * @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
 * @pre  @a service_adaptor must be issued by service_adaptor_create()
 */
int service_adaptor_foreach_plugin2(service_adaptor_plugin_cb callback,
		void *user_data);

/**
 * @brief Gets service specfic last result
 * @details This function retrieves the last error code that be issued from plugin.<br>
 *  When if API function returns #SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED, gets using this function.
 * @since_tizen 2.4
 *
 * @param[out]   err     The error number that is defined service plugin SPEC
 * @remarks      Thread safe functions
 * @remarks      The result number's detail specification is defined service plugin or provider.
 * @remarks      The detail error message can be got using service_adaptor_get_last_error_message()
 * @see          service_adaptor_get_last_error_message()
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
 * @param[out]   message The error message that is defined service plugin SPEC
 * @remarks      @a message must be released using free()
 * @remarks      Thread safe functions
 * @remarks      The result string's detail specification is defined service plugin or provider.
 * @see          service_adaptor_get_last_result()
 * @return 0 on success, otherwise a negative error value
 * @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
 * @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SERVICE_ADAPTOR_ERROR_NO_DATA There is no error message
 * @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
 */
int service_adaptor_get_last_error_message(char **message);

/*
int service_adaptor_add_plugin(service_adaptor_h adaptor_client, const char *plugin_uri);
int service_adaptor_remove_plugin(service_adaptor_h adaptor_client, const char *plugin_uri);
int service_adaptor_get_auth(service_adaptor_h adaptor_client, const char *uri, service_auth_h *auth);
*/

/** 2.4
* @brief Create service plugin handle
* @details Create plugin handle using @a plugin_uri
* @since_tizen 2.4
*
* @param[in]    service_adaptor The handle of Service Adaptor
* @param[in]    plugin_uri      The specfic string for use plugin, this values are set by plugin
* @param[out]   plugin          The handle for use Plugin APIs
* @remarks      @a plugin must be released memory using service_plugin_destroy() when you no longer needs plugin's API
* @see          service_plugin_destroy()
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
* @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
* @pre  @a service_adaptor must be issued by service_adaptor_create()
*/
int service_adaptor_create_plugin(service_adaptor_h service_adaptor,
                                                const char *plugin_uri,
                                                service_plugin_h *plugin);

/**
 * @brief Create service plugin handle
 * @details Create plugin handle using @a plugin_uri
 * @since_tizen 2.4
 *
 * @param[in]    service_adaptor The handle of Service Adaptor
 * @param[in]    plugin_uri      The specfic string for use plugin, this values are set by plugin
 * @param[out]   plugin          The handle for use Plugin APIs
 * @remarks      @a plugin must be released memory using service_plugin_destroy() when you no longer needs plugin's API
 * @see          service_plugin_destroy()
 * @return 0 on success, otherwise a negative error value
 * @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
 * @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
 * @pre  @a service_adaptor must be issued by service_adaptor_create()
 */
int service_plugin_create(const char *uri, service_plugin_h *plugin);

/**
 * @brief        Destroy service plugin handle
 * @details      It must called after a program no longer needs APIs of specfic plugin
 * @since_tizen 2.4
 *
 * @param[in]    plugin  The handle for use Plugin APIs
 * @see  service_adaptor_create_plugin()
 * @return 0 on success, otherwise a negative error value
 * @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
 * @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
 * @pre  @a plugin must be issued by service_adaptor_create_plugin()
 */
int service_plugin_destroy(service_plugin_h plugin);

/**
 * @brief Add Plugin Property
 * @details The plguin property is used for plugin's basic or optional requirement.<br>
 *  This value is not used in Adaptor layer, but it can be uesd to important Key for plugin with service provider.<br>
 * @since_tizen 2.4
 *
 * @param[in]    plugin  The handle for use Plugin APIs
 * @param[in]    key     The key of plugin property
 * @param[in]    value   The value of plugin property that matched @a key
 * @remarks      Some kind of property key(Not mandatory) is defined in this API (That is named to SERVICE_PLUGIN_PROPERTY_XXX)
 * @remarks      If the @a key already exists in the property its current value is replaced with the new @a value.
 * @remarks      @a plugin must be released memory using #service_plugin_destroy() when you no longer needs plugin's API
 * @return 0 on success, otherwise a negative error value
 * @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
 * @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
 * @see  SERVICE_PLUGIN_PROPERTY_APP_KEY
 * @see  SERVICE_PLUGIN_PROPERTY_APP_SECRET
 * @see  SERVICE_PLUGIN_PROPERTY_USER_ID
 * @pre  @a plugin must be issued by service_adaptor_create_plugin()
 */
int service_plugin_add_property(service_plugin_h plugin,
		const char *key,
		const char *value);

/**
 * @brief Remove Plugin Property
 * @since_tizen 2.4
 *
 * @param[in]    plugin  The handle for use Plugin APIs
 * @param[in]    key     The key of plugin property
 * @remarks      Some kind of property key(Not mandatory) is defined in this API (That is named to SERVICE_PLUGIN_PROPERTY_XXX)
 * @return 0 on success, otherwise a negative error value
 * @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
 * @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
 * @see  SERVICE_PLUGIN_PROPERTY_APP_KEY
 * @see  SERVICE_PLUGIN_PROPERTY_APP_SECRET
 * @see  SERVICE_PLUGIN_PROPERTY_USER_ID
 */
int service_plugin_remove_property(service_plugin_h plugin,
		const char *key);

/**
 * @brief Gets Plugin Property
 * @since_tizen 2.4
 *
 * @param[in]    plugin  The handle for use Plugin APIs
 * @param[in]    key     The key of plugin property
 * @param[out]   value   The value of plugin property that matched @a key
 * @remarks      Some kind of property key(Not mandatory) is defined in this API (That is named to SERVICE_PLUGIN_PROPERTY_XXX)
 * @remarks      @a value must be released using free()
 * @return 0 on success, otherwise a negative error value
 * @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
 * @retval #SERVICE_ADAPTOR_ERROR_NO_DATA There is no property
 * @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
 * @see  SERVICE_PLUGIN_PROPERTY_APP_KEY
 * @see  SERVICE_PLUGIN_PROPERTY_APP_SECRET
 * @see  SERVICE_PLUGIN_PROPERTY_USER_ID
 * @pre  The function get property already set by service_adaptor_set_plugin_property()
 */
int service_plugin_get_property(service_plugin_h plugin,
		const char *key,
		char **value);

/**
 * @brief Logins Plugin using Property
 * @since_tizen 3.0
 *
 * @param[in]    plugin  The handle for use Plugin APIs
 * @param[in]    callback        The callback for login
 * @param[in]    user_data       Passed data to callback
 * @remarks
 * @remarks      @a value must be released using free()
 * @return 0 on success, otherwise a negative error value
 * @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
 * @retval #SERVICE_ADAPTOR_ERROR_NO_DATA There is no property
 * @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
 * @pre  The function get property already set by service_adaptor_set_plugin_property()
 */
int service_plugin_login(service_plugin_h plugin, service_plugin_login_cb callback, void *user_data);

/** 2.4
* @brief Requests start initalization for service plugin
* @since_tizen 2.4
*
* @param[in]    plugin          The handle for use Plugin APIs
* @param[in]    service_mask    The flag for use service plugins, this flag can be masked multiple enum (#service_plugin_service_type_e)
* @remarks      @a service_mask must be input using 'bit or' operation with #service_plugin_service_type_e
* @remarks      - for example,
* @remarks      &nbsp;&nbsp;&nbsp;&nbsp;        <b>int</b> @a service_mask |= SERVIE_PLUGIN_SERVICE_AUTH;
* @remarks      &nbsp;&nbsp;&nbsp;&nbsp;        @a service_mask |= SERVICE_PLUGIN_SERVICE_STORAGE;
* @remarks      &nbsp;&nbsp;&nbsp;&nbsp;        <b>int</b> ret = service_plugin_start(@a m_plugin, @a service_mask);
* @remarks      If a program needs to stop plugin manually, use #service_plugin_stop(). <br>But in #service_plugin_destroy(), automatically stop service plugin
* @see          service_plugin_service_type_e
* @see          service_plugin_stop()
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
 * @brief Requests start initalization for service plugin
 * @since_tizen 2.4
 *
 * @param[in]    plugin          The handle for use Plugin APIs
 * @remarks      @a service_mask must be input using 'bit or' operation with #service_plugin_service_type_e
 * @remarks      - for example,
 * @remarks      &nbsp;&nbsp;&nbsp;&nbsp;        <b>int</b> @a service_mask |= SERVIE_PLUGIN_SERVICE_AUTH;
 * @remarks      &nbsp;&nbsp;&nbsp;&nbsp;        @a service_mask |= SERVICE_PLUGIN_SERVICE_STORAGE;
 * @remarks      &nbsp;&nbsp;&nbsp;&nbsp;        <b>int</b> ret = service_plugin_start(@a m_plugin, @a service_mask);
 * @remarks      If a program needs to stop plugin manually, use #service_plugin_stop(). <br>But in #service_plugin_destroy(), automatically stop service plugin
 * @see          service_plugin_service_type_e
 * @see          service_plugin_stop()
 * @return 0 on success, otherwise a negative error value
 * @return If return value is #SERVICE_ADAPTOR_ERROR_NOT_AUTHOLIZED, request signup to autholization application
 * @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
 * @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SERVICE_ADAPTOR_ERROR_NOT_AUTHOLIZED Need atholization
 * @retval #SERVICE_ADAPTOR_ERROR_TIMED_OUT Timed out
 * @retval #SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE IPC failed with Service Adaptor Daemon
 * @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
 */
int service_plugin_start2(service_plugin_h plugin);

/**
 * @brief Requests stop manually for service plugin
 * @since_tizen 2.4
 *
 * @param[in]    plugin          The handle for use Plugin APIs
 * @remarks      If a program needs to stop plugin manually, use this function. <br>But in #service_plugin_destroy(), automatically stop service plugin
 * @remarks      @a plugin must be released memory using #service_plugin_destroy() when you no longer needs plugin's API
 * @see          service_plugin_start()
 * @see          service_plugin_destroy()
 * @return 0 on success, otherwise a negative error value
 * @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
 * @retval #SERVICE_ADAPTOR_ERROR_TIMED_OUT Timed out
 * @retval #SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE IPC failed with Service Adaptor Daemon
 * @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
 * @pre  service_plugin_start()
 */
int service_plugin_stop(service_plugin_h plugin);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __SERVICE_ADAPTOR_H__ */
