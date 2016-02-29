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
 * File: service_adaptor_client_storage_private.h
 * Desc: Service Adaptor APIs
 * Created on: Feb, 2015
 * Auth: Jiwon Kim <jiwon177.kim@samsung.com>
 *
 *****************************************************************************/

#ifndef __TIZEN_SOCIAL_SERVICE_ADAPTOR_CLIENT_PRIVATE_H__
#define __TIZEN_SOCIAL_SERVICE_ADAPTOR_CLIENT_PRIVATE_H__

#if defined _WIN32 || defined __CYGWIN__
	#define DLL_IMPORT __declspec(dllimport)
	#define DLL_EXPORT __declspec(dllexport)
	#define DLL_LOCAL
#else
	#if __GNUC__ >= 4
		#define DLL_IMPORT __attribute__ ((visibility ("default")))
		#define DLL_EXPORT __attribute__ ((visibility ("default")))
		#define DLL_LOCAL  __attribute__ ((visibility ("hidden")))
	#else
		#define DLL_IMPORT
		#define DLL_EXPORT
		#define DLL_LOCAL
	#endif
#endif

#ifdef SERVICE_ADAPTOR_CLIENT_DLL
	#ifdef SERVICE_ADAPTOR_CLIENT_DLL_EXPORTS
		#define SERVICE_ADAPTOR_CLIENT_PUBLIC_API DLL_EXPORT
	#else
		#define SERVICE_ADAPTOR_CLIENT_PUBLIC_API DLL_IMPORT
	#endif
	#define SERVICE_ADAPTOR_CLIENT_LOCAL_API DLL_LOCAL
#else
	#define SERVICE_ADAPTOR_CLIENT_PUBLIC_API
	#define SERVICE_ADAPTOR_CLIENT_LOCAL_API
#endif

#include "service_adaptor_client.h"
#include "service_adaptor_client_type.h"
#include "private/service-adaptor-client.h"
#include <glib.h>

/**
* @brief Describes infromation about Service Adaptor
*/
struct _service_adaptor_s
{
	void *on_signal;

//	struct _service_plugin_s **plugins; //TODO
//	int plugin_count;	// TODO

/////////////////////// private feature
	char *service_name;			/**< specifies status as none*/
	char *user_id;				/**< specifies status as none*/
	char *app_id;				/**< specifies status as none*/
	unsigned int service_id;
	char *imsi;				/**< specifies status as none*/
	GMutex set_auth_mutex;

	service_adaptor_plugin_s *plugin;	/**< specifies status as none*/
/////////////////////// private feature
};

typedef struct _plugin_entry_s
{
	char *plugin_uri;
	int installed_mask;
}plugin_entry_t;

typedef enum _client_app_type_e
{
	CLIENT_APP_TYPE_APPLICATION = 1,
	CLIENT_APP_TYPE_ETC = 0,
}client_app_type_e;

struct _service_plugin_s
{
	struct _service_adaptor_s *adaptor;

	char *service_handle_name;
	void *optional_property;
	char *plugin_uri;
	int enabled_mask;
	client_app_type_e app_type;
};





/**
* @brief Callback for getting login result
* @since_tizen 2.4
*
* @param[in]    result          Result code for login request (see #service_adaptor_error_e)
* @param[in]    user_data       Passed data from request function
* @remarks      The @a result values #SERVICE_ADAPTOR_ERROR_NONE Successful
* @remarks      The @a result values #SERVICE_ADAPTOR_ERROR_TIMED_OUT Timed out
* @remarks      The @a result values #SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED Failed in Plugin internal
* @remarks	The @a result values #SERVICE_ADAPTOR_ERROR_NETWORK_UNREACHABLE Failed by network issue
* @remarks      The @a result values #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
* @see #service_adaptor_error_e
* @see service_plugin_request_login()
* @pre  #service_plugin_request_login() will invoke this callback.
*/
typedef void (*service_plugin_login_cb)(int result,
						void *user_data);



/**
* @brief Set callback for changing state of Service Adaptor
* @since_tizen 2.4
*
* @param[in]	service_adaptor	The handle of Service Adaptor
* @param[in]	callback	The callback function to receive message from service adaptor daemon
* @param[in]	user_data	Passed data to callback
* @remarks	@a service_adaptor must be released memory using service_adaptor_destroy() when you finish using service adaptor
* @see		service_adaptor_destroy()
* @see		service_adaptor_signal_cb()
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_HANDLE Invalid handle
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
* @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
* @pre	@a service_adaptor must be issued by service_adaptor_create()
*/
int service_adaptor_set_state_changed_cb(service_adaptor_h service_adaptor,
						service_adaptor_signal_cb callback,
						void *user_data);

/**
* @brief Set callback for changing state of Service Adaptor
* @since_tizen 2.4
*
* @param[in]	service_adaptor	The handle of Service Adaptor
* @remarks	@a service_adaptor must be released memory using service_adaptor_destroy() when you finish using service adaptor
* @see		service_adaptor_destroy()
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_HANDLE Invalid handle
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
* @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
* @pre	@a service_adaptor must be issued by service_adaptor_create()
*/
int service_adaptor_unset_state_changed_cb(service_adaptor_h service_adaptor);

/**
* @brief Gets whether a plugin is logined or not logined
* @details 'Login' includes both authentication(sign-in by user) and authorization (allowed by user) <br>
* If plugin does not need authentication or authorization, service_plugin_is_login_required() API's result is true on anytime.
* @since_tizen 2.4
*
* @param[in]	plugin		The handle for use Plugin APIs
* @param[out]	required	true on login required, otherwise false
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
* @retval #SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE IPC failed with Service Adaptor Daemon
*/
int service_plugin_is_login_required(service_plugin_h plugin,
						bool *required);

/**
* @brief Requests login to plugin
* @details 'Login' includes both authentication(sign-in by user) and authorization (allowed by user) <br>
* If the plugin does not logined by user, you must request login using this API.<br>
* You can check whether logined or not using service_plugin_is_login_required().<br>
* If not logined, you will be failed #service_plugin_start().<br>
* Of caurse, if plugin does not need authentication or authorization, service_plugin_is_login_required() API's result is true on anytime.
* @since_tizen 2.4
*
* @param[in]    plugin          The handle for use Plugin APIs
* @param[in]    callback        The callback for getting result this operation
* @param[in]    user_data       The user data to be passed to the callback function
* @see          service_plugin_start()
* @see          service_plugin_login_cb()
* @see          service_adaptor_get_last_result()
* @see          service_adaptor_get_last_error_message()
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_STATE The handle's state is invalid
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
* @retval #SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE IPC failed with Service Adaptor Daemon
* @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
* @post #service_plugin_login_cb() will be invoked
*/
int service_plugin_request_login(service_plugin_h plugin,
						service_plugin_login_cb callback,
						void *user_data);

/**
* @brief The type for Service Adaptor
*/
typedef struct _service_adaptor_s service_adaptor_s;

typedef struct _service_plugin_s service_plugin_s;

void _service_adaptor_set_last_result (int code, const char *message);

#define service_adaptor_set_last_result(code, msg)	do { \
								sac_error("Error occured (%d)(%s)", (int)(code), (msg)); \
								_service_adaptor_set_last_result((code), (msg)); \
							} while (0)

#endif /*__TIZEN_SOCIAL_SERVICE_ADAPTOR_CLIENT_PRIVATE_H__*/
