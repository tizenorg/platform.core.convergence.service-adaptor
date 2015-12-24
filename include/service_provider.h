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

#ifndef __TIZEN_SOCIAL_SERVICE_PROVIDER_H__
#define __TIZEN_SOCIAL_SERVICE_PROVIDER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <service_provider_type.h>

/**
 * @addtogroup	SERVICE_PROVIDER_MODULE

 * @{
 */

/**
 * @brief	Sets storage provider
 * @details	It means the Service Provider provides storage feature.\n
 * The #storage_provider_s 's elements (callbacks) are called by Service Adaptor.
 * @since_tizen 3.0
 *
 * @param[in]	storage_provider	storage_provider spec
 * @param[in]	user_data		Passed data to callback
 *
 * @remarks	It should be called in app_created_cb() callback.
 * @remarks	For working successfully, #service_provider_open_channel() must be called in on_app_control().
 * @remarks	All of the structure elements(callbacks) must assigned to provider's functions.
 * @remarks	If some element(s) is(are) NULL, you can not provide storage features.
 * @remarks	Be careful to assign dangling pointer, during initialization of structure.
 * @remarks	The provider callbacks will be invoked on <b>additional thread</b>(Not main thread).
 *
 * @return 0 on success, otherwise a negative error value
 * @retval	#SERVICE_ADAPTOR_ERROR_NONE					Successful
 * @retval	#SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval	#SERVICE_ADAPTOR_ERROR_OUT_OF_MEMORY		Out of memory
 * @retval	#SERVICE_ADAPTOR_ERROR_UNKNOWN				Unknown error
 *
 * @code
 * bool service_app_create(void *data)
 * {
 *     //...
 *
 *     struct storage_provider_s _storage_cb = {NULL, }; // Assigning "{NULL, }" is important
 *     _storage_cb.open = __internal_open_cb;
 *     // Todo: Set all callbacks
 *
 *     int ret = service_provider_set_storage_provider(&_storage_cb, NULL);
 *
 *     // Todo: add your code here.
 *     return true;
 * }
 * @endcode
 */
int service_provider_set_storage_provider(storage_provider_s *storage_provider, void *user_data);

/**
 * @brief	Open communication channel and provides control right to Service Adaptor
 * @details	It is important API for Service Provider.\n
 * All of the communication with Service Adaptor can be started from this API.
 * If Service Adaptor needs to make communication channel with Service Provider, it can send app-control with APP_CONTROL_OPERATION_SERVICE_PROVIDER_CHANNEL operation.\n
 * At that time, Service Provider calls this API with app_control_h, the service channel is opened in this API.\n
 * \n
 * From this API, the @a channel will be opened between Service Adaptor and Service Provider.\n
 * \n
 * Refer to @ref SERVICE_PROVIDER_MODULE_CHANNEL_DESCRIPTION
 *
 * @since_tizen 3.0
 *
 * @param[in]	app_control	Handle of app-control
 * @param[in]	callback	The callback will be invoked after channel opened/closed
 * @param[in]	user_data	Passed data to callback
 *
 * @remarks	It must be called in app_control_cb()
 * @remarks	If return value is #SERVICE_ADAPTOR_ERROR_NO_DATA, there is no matched App Control operation ID
 * @remarks	After this API called successfully, the application can be terminated by Service Adaptor when a channel is closed.\n
 * When the Service Provider receives to request of closing channel, service_app_exit() can be called according return value from channel event callback.
 *
 * @return 0 on success, otherwise a negative error value
 * @retval	#SERVICE_ADAPTOR_ERROR_NONE					Successful
 * @retval	#SERVICE_ADAPTOR_ERROR_NO_DATA				There is no data available
 * @retval	#SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval	#SERVICE_ADAPTOR_ERROR_OUT_OF_MEMORY		Out of memory
 * @retval	#SERVICE_ADAPTOR_ERROR_UNKNOWN				Unknown error
 *
 * @see #APP_CONTROL_OPERATION_SERVICE_PROVIDER_CHANNEL
 * @pre	For provide Storage feature, service_provider_set_storage_provider() must be called before this.
 * @post #service_provider_channel_cb() will be invoked
 *
 * @code
 * void service_app_control(app_control_h app_control, void *data)
 * {
 *     char *_operation = NULL;
 *     int ret = app_control_get_operation(app_control, &_operation);
 *
 *     if (!ret && !strncmp(APP_CONTROL_OPERATION_SERVICE_PROVIDER_CHANNEL, _operation, strlen(APP_CONTROL_OPERATION_SERVICE_PROVIDER_CHANNEL)))
 *         ret = service_provider_open_channel(app_control, __channel_state_cb, NULL);
 *
 *     // Todo: add your code here.
 *     return;
 * }
 * @endcode
 */
int service_provider_open_channel(app_control_h app_control, service_provider_channel_cb callback, void *user_data);

/**
 * @brief Provides detailed error message to API user
 * @details If some provider callback will returns #SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED, it should be called before return.\n
 * The client can obtain detailed message using #service_adaptor_get_last_result() and  #service_adaptor_get_last_error_message().
 * @since_tizen 3.0
 *
 * @param[in]	code	Error code
 * @param[in]	message	Error message
 *
 * @remarks	It must be called in provider callback.
 * @remarks	It is worked thread safely.
 *
 * @return 0 on success, otherwise a negative error value
 * @retval	#SERVICE_ADAPTOR_ERROR_NONE					Successful
 * @retval	#SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval	#SERVICE_ADAPTOR_ERROR_OUT_OF_MEMORY		Out of memory
 * @retval	#SERVICE_ADAPTOR_ERROR_UNKNOWN				Unknown error
 *
 * @see	storage_provider_s
 */
int service_provider_set_last_error(int code, const char *message);

/**
 * @brief Sets session event callback
 * @details The @a session will be started/stopped by Service Adaptor.\n
 * Originally, it is requested from client.\n
 * \n
 * Refer to @ref SERVICE_PROVIDER_MODULE_SESSION_DESCRIPTION
 * @since_tizen 3.0
 *
 * @param[in]	callback	The callback will be invoked when a session event occurred
 * @param[in]	user_data	Passed data to callback
 *
 * @remarks	For getting callback successfully, you must call #service_provider_open_channel() in app_control_cb()
 * @remarks	This API should be called in app_create_cb()
 *
 * @return 0 on success, otherwise a negative error value
 * @retval	#SERVICE_ADAPTOR_ERROR_NONE					Successful
 * @retval	#SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval	#SERVICE_ADAPTOR_ERROR_OUT_OF_MEMORY		Out of memory
 * @retval	#SERVICE_ADAPTOR_ERROR_UNKNOWN				Unknown error
 *
 * @see	#service_provider_open_channel()
 * @see #service_provider_session_event_cb()
 * @see	#service_plugin_start()
 * @see	#service_plugin_stop()
 */
int service_provider_set_session_event_cb(service_provider_session_event_cb callback, void *user_data);

/**
 * @brief Unsets session event callback
 * @since_tizen 3.0
 *
 * @return 0 on success, otherwise a negative error value
 * @retval	#SERVICE_ADAPTOR_ERROR_NONE					Successful
 * @retval	#SERVICE_ADAPTOR_ERROR_UNKNOWN				Unknown error
 */
int service_provider_unset_session_event_cb(void);

/**
 * @brief Gets property of session information
 * @details	The 'property' is kept until to stop session.\n
 * The property was added by client.\n
 * Service Provider can check that client has a right for using self.
 * @since_tizen 3.0
 *
 * @param[in]	session		The handle of session information
 * @param[in]	key			The key of property
 * @param[out]	value		The value of property
 *
 * @remarks	Do not release of memory for a @a value
 *
 * @return 0 on success, otherwise a negative error value
 * @retval	#SERVICE_ADAPTOR_ERROR_NONE					Successful
 * @retval	#SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval	#SERVICE_ADAPTOR_ERROR_OUT_OF_MEMORY		Out of memory
 * @retval	#SERVICE_ADAPTOR_ERROR_UNKNOWN				Unknown error
 *
 * @see	#service_plugin_add_property()
 */
int service_provider_get_session_property(service_provider_session_h session, const char *key, char **value);

/**
 * @brief Foreach the properties of session
 * @details	The 'property' was added from client.\n
 * @since_tizen 3.0
 *
 * @param[in]	session		The handle of session information
 * @param[in]	callback	The callback for foreach property
 * @param[in]	user_data	Passed data to callback
 *
 * @remarks	The foreach callback will be invoked from same thread.
 *
 * @return 0 on success, otherwise a negative error value
 * @retval	#SERVICE_ADAPTOR_ERROR_NONE					Successful
 * @retval	#SERVICE_ADAPTOR_ERROR_NO_DATA				There is no data available
 * @retval	#SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval	#SERVICE_ADAPTOR_ERROR_OUT_OF_MEMORY		Out of memory
 * @retval	#SERVICE_ADAPTOR_ERROR_UNKNOWN				Unknown error
 *
 * @see	#service_plugin_add_property()
 * @see	service_provider_session_property_cb()
 */
int service_provider_foreach_session_property(service_provider_session_h session, service_provider_session_property_cb callback, void *user_data);

/**
 * @brief Gets service mask from session.
 * @details	The 'service_mask' was set by client.\n
 * Service Provider can check which service will be needed, and start initialize.
 * @since_tizen 3.0
 *
 * @param[in]	session			The handle of session information
 * @param[out]	service_mask	The flag for use service plugins, this flag can be masked multiple enum (#service_plugin_service_type_e)
 * @remarks	@a service_mask should be checked using 'bit and' operation with #service_plugin_service_type_e
 *
 * @return 0 on success, otherwise a negative error value
 * @retval	#SERVICE_ADAPTOR_ERROR_NONE					Successful
 * @retval	#SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval	#SERVICE_ADAPTOR_ERROR_OUT_OF_MEMORY		Out of memory
 * @retval	#SERVICE_ADAPTOR_ERROR_UNKNOWN				Unknown error
 *
 * @code
 * {
 *     // ...
 *     int m_mask = 0;
 *     int ret = service_provider_get_session_service_mask(m_session, &m_mask);
 *     if (ret == SERVICE_ADAPTOR_ERROR_NONE) {
 *         if (m_mask & SERVICE_PLUGIN_SERVICE_STORAGE) {
 *             // Todo: Initialize storage service
 *         }
 *         // ...
 *     }
 *     // ...
 *     return ret;
 * }
 * @endcode
 */
int service_provider_get_session_service_mask(service_provider_session_h session, int *service_mask);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif /* __TIZEN_SOCIAL_SERVICE_ADAPTOR_PROVIDER_H__ */
