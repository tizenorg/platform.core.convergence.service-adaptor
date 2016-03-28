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

#ifndef __AUTH_ADAPTOR_H__
#define __AUTH_ADAPTOR_H__

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif

#include <glib.h>
#include <stdint.h>

/**
 * @brief Enumerations of error code for Auth Adaptor
 */
typedef enum auth_error_code_e {
	AUTH_ADAPTOR_ERROR_NONE                      = 0,
	AUTH_ADAPTOR_ERROR_LAUNCH                    = 1,    /**< 1 ~ 99: internal error*/
	AUTH_ADAPTOR_ERROR_INIT                      = 2,
	AUTH_ADAPTOR_ERROR_DEINIT                    = 3,
	AUTH_ADAPTOR_ERROR_CREATE                    = 4,
	AUTH_ADAPTOR_ERROR_DESTROY                   = 5,
	AUTH_ADAPTOR_ERROR_START                     = 6,
	AUTH_ADAPTOR_ERROR_STOP                      = 7,
	AUTH_ADAPTOR_ERROR_CONNECT                   = 8,
	AUTH_ADAPTOR_ERROR_DISCONNECT                = 9,
	AUTH_ADAPTOR_ERROR_NOT_FOUND                 = 10,
	AUTH_ADAPTOR_ERROR_CORRUPTED                 = 11,
	AUTH_ADAPTOR_ERROR_UNSUPPORTED               = 12,
	AUTH_ADAPTOR_ERROR_INVALID_HANDLE            = 13,
	AUTH_ADAPTOR_ERROR_INVALID_ARGUMENT          = 14,
	AUTH_ADAPTOR_ERROR_INVALID_ARGUMENT_TYPE     = 15,
	AUTH_ADAPTOR_ERROR_NOT_AUTHORIZED            = 16,
	AUTH_ADAPTOR_ERROR_ADAPTOR_INTERNAL          = 17,
	AUTH_ADAPTOR_ERROR_PLUGIN_INTERNAL           = 18,
	AUTH_ADAPTOR_ERROR_SERVER_INTERNAL           = 19,
	AUTH_ADAPTOR_ERROR_DBUS                      = 20,
	AUTH_ADAPTOR_ERROR_CALLBACK_TIME_OUT         = 21,
	AUTH_ADAPTOR_ERROR_MAX
} auth_error_code_t;

/**
 * @brief Enumerations of error code for Auth Plugin Internal Error
 */
typedef enum _auth_plugin_internal_error_code_e {
	AUTH_PLUGIN_ERROR_HTTP_BAD_REQUEST           = 400,
	AUTH_PLUGIN_ERROR_HTTP_UNAUTHORIZED          = 401,
	AUTH_PLUGIN_ERROR_HTTP_FORBIDDEN             = 403,
	AUTH_PLUGIN_ERROR_HTTP_NOT_FOUND             = 404,
	AUTH_PLUGIN_ERROR_HTTP_METHOD_NOT_ALLOWED    = 405,
	AUTH_PLUGIN_ERROR_HTTP_BAD_GATEWAY           = 502,
	AUTH_PLUGIN_ERROR_HTTP_SERVICE_UNAVAILBLE    = 503,
	AUTH_PLUGIN_ERROR_HTTP_INSUFFICIENT_AUTH     = 507,
	AUTH_PLUGIN_ERROR_HTTP_ETC                   = 598,
	AUTH_PLUGIN_ERROR_HTTP_UNKNOWN               = 599,

	AUTH_PLUGIN_ERROR_CURL_COULDNT_CONNECT       = 601,
	AUTH_PLUGIN_ERROR_CURL_TIMEOUT               = 602,
	AUTH_PLUGIN_ERROR_CURL_ETC                   = 698,
	AUTH_PLUGIN_ERROR_CURL_UNKNOWN               = 699,

	AUTH_PLUGIN_ERROR_FILE_OPEN_FAILED           = 701,
	AUTH_PLUGIN_ERROR_FILE_NOT_EXIST             = 702,
	AUTH_PLUGIN_ERROR_FILE_ETC                   = 798,
	AUTH_PLUGIN_ERROR_FILE_UNKNOWN               = 799,

	AUTH_PLUGIN_ERROR_MEMORY_ALLOCATION_FAILED   = 801,
	AUTH_PLUGIN_ERROR_MEMORY_ETC                 = 898,
	AUTH_PLUGIN_ERROR_MEMORY_UNKNOWN             = 899,

	AUTH_PLUGIN_ERROR_THREAD_CREATE_FAILED       = 901,
	AUTH_PLUGIN_ERROR_THREAD_STOPPED             = 902,
	AUTH_PLUGIN_ERROR_THREAD_ETC                 = 908,
	AUTH_PLUGIN_ERROR_THREAD_UNNOWN              = 909,

	AUTH_PLUGIN_ERROR_CALLBACK_TIME_OUT	     = 997,
	AUTH_PLUGIN_ERROR_ETC                        = 998,
	AUTH_PLUGIN_ERROR_UNKNOWN                    = 999,
} auth_plugin_internal_error_code_e;

/**
* @brief The handle for Auth Plugin
*/
typedef struct auth_adaptor_plugin_s *auth_adaptor_plugin_h;

/**
* @brief The handle for Auth Adaptor
*/
typedef struct auth_adaptor_s *auth_adaptor_h;

/**
* @brief Describes infromation about Plugin Context
*/
typedef struct auth_adaptor_plugin_context_s {
	/* Context variables */
	int context_id;
	auth_adaptor_plugin_h plugin_handle;

	/* User define (input by service-adaptor) */
	char    *user_id;
	char    *user_password;
	char    *app_id;
	char    *app_secret;
	char	*service_name;

	/* Plugin define (input by plugin) */
	char    *plugin_uri;	/* mandatory (package id) */
	void	*plugin_data;	/* optional */

	/* Plugin define (input by server response after autholization) */
	char    *access_token;
	char	*refresh_token;
	char	*uid;

	/* For product feature */
	char	*msisdn;
	char	*imsi;
	char    *plugin_name;
	/* char    *cluster_name; */
} auth_adaptor_plugin_context_t;
typedef struct auth_adaptor_plugin_context_s *auth_adaptor_plugin_context_h;

/**
* @brief Describes infromation about error code from plugin internal
*/
typedef struct auth_adaptor_error_code_s {
	int64_t code;
	char    *msg;
} auth_adaptor_error_code_t;
/**
* @brief The handle for error code
*/
typedef struct auth_adaptor_error_code_s *auth_adaptor_error_code_h;

/**
* @brief The handle for Auth Plugin Listener
*/
typedef struct auth_adaptor_plugin_listener_s *auth_adaptor_plugin_listener_h;

/**
* @brief Describes infromation about Plugin Handle
*/
typedef struct auth_adaptor_plugin_handle_s {
	/* Mandatory functions to handle plugin in adaptor */
	auth_error_code_t (*create_context)(auth_adaptor_plugin_context_h *context,
							const char *user_id,
							const char *user_password,
							const char *app_id,
							const char *app_secret,
							const char *imsi);

	auth_error_code_t (*destroy_context)(auth_adaptor_plugin_context_h context);
	auth_error_code_t (*destroy_handle)(struct auth_adaptor_plugin_handle_s *handle);
	auth_error_code_t (*set_listener)(auth_adaptor_plugin_listener_h listener);
	auth_error_code_t (*unset_listener)(void);
	/* Mandatory end */

	/* Optional */
	auth_error_code_t (*is_auth)(auth_adaptor_plugin_context_h context,
							void *request,
							int *is_auth,
							auth_adaptor_error_code_h *error,
							void *response);

	auth_error_code_t (*join)(auth_adaptor_plugin_context_h context,
							const char *device_id,
							void *request,
							auth_adaptor_error_code_h *error,
							void *response);

	auth_error_code_t (*login)(auth_adaptor_plugin_context_h context,
							void *request,
							auth_adaptor_error_code_h *error,
							void *response);

	auth_error_code_t (*refresh_access_token)(auth_adaptor_plugin_context_h context,
							void *request,
							auth_adaptor_error_code_h *error,
							void *response);

	auth_error_code_t (*set_service_status)(auth_adaptor_plugin_context_h context,
							const int service_id,
							const int status,
							void *request,
							auth_adaptor_error_code_h *error,
							void *response);

	auth_error_code_t (*get_msisdn)(auth_adaptor_plugin_context_h context,
							void *request,
							char **msisdn,
							auth_adaptor_error_code_h *error,
							void *response);

	auth_error_code_t (*get_service_status)(auth_adaptor_plugin_context_h context,
							const int service_id,
							void *request,
							int *status,
							auth_adaptor_error_code_h *error,
							void *response);

	auth_error_code_t (*get_service_policy)(auth_adaptor_plugin_context_h context,
							const int service_id,
							void *request,
							char **default_status,
							char **policy_feature,
							char **policy_version,
							char **policy_doc_url,
							auth_adaptor_error_code_h *error,
							void *response);

	auth_error_code_t (*get_server_info)(auth_adaptor_plugin_context_h context,
							void *request,
							GHashTable **server_info,
							auth_adaptor_error_code_h *error,
							void *response);

	auth_error_code_t (*external_request)(auth_adaptor_plugin_context_h context,
							const char *api_uri,
							const unsigned char *req_bundle_raw,
							int req_len,
							unsigned char **res_bundle_raw,
							int *res_len,
							auth_adaptor_error_code_h *error);
	/* Optional end */

	/* Mandatory */
	char *plugin_uri;	/* package id */
	/* Mandatory end */

} auth_adaptor_plugin_handle_t;
typedef struct auth_adaptor_plugin_handle_s *auth_adaptor_plugin_handle_h;

/**
 * Callback function variable for service adaptor
 */

/*
typedef void (*auth_adaptor_service_login_reply_cb)(char *imsi,
						char *plugin_uri,
						char *app_id,
						char *msisdn,
						void *response);
*/

/**
* @brief Describes infromation about Callback Listener (referenced by Service Adaptor)
*/
typedef struct auth_adaptor_listener_s {
/*
	void (*login_reply)(char *imsi,
							char *plugin_uri,
							char *app_id,
							char *msisdn,
							void *response);
*/
} auth_adaptor_listener_t;
/**
* @brief The handle for Auth Adaptor Listener
*/
typedef struct auth_adaptor_listener_s *auth_adaptor_listener_h;


/**
 * Callback function variables for plugins
 * These callbacks are expected to be support by plugins
 */

/**
* @brief Callback for auth_adaptor_plugin_handle->login API (referenced by Auth Plugin)
*
* @param[in]	context		specifies Auth-adaptor Plugin context
* @param[in]	error_code	specifies Error code
* @param[in]	response	specifies Optional value from specific Plugin (Recommend Json Object format)
* @return	void.
* @pre  auth_adaptor_plugin_handle->login() will invoke this callback.
* @see
*/
/*
typedef void (*auth_adaptor_plugin_login_reply_cb)(auth_adaptor_plugin_context_h context,
						auth_adaptor_error_code_h error_code,
						void *response);
*/
/**
* @brief Describes infromation about Callback Listener (referenced by Auth Plugin)
*/
typedef struct auth_adaptor_plugin_listener_s {
	/*
	auth_adaptor_plugin_login_reply_cb		auth_adaptor_login_reply;
	*/
} auth_adaptor_plugin_listener_t;

/**
 * @brief Loads plugin from selected path
 */
EXPORT_API
int auth_adaptor_load_plugin(auth_adaptor_h adaptor,
						const char *plugin_path);


/* For 3rd party plugin packages */
EXPORT_API
int auth_adaptor_load_plugin_from_package(auth_adaptor_h adaptor,
						const char *package_id,
						const char *plugin_path);

/**
 * @brief Unloads selected plugin
 */
EXPORT_API
int auth_adaptor_unload_plugin(auth_adaptor_h adaptor,
						auth_adaptor_plugin_h plugin);

/**
 * @brief Gets plugin name
 */
EXPORT_API
void auth_adaptor_get_plugin_uri(auth_adaptor_plugin_h plugin, char **plugin_uri);

/**
 * @brief Destroy error code
 */
EXPORT_API
void auth_adaptor_destroy_error_code(auth_adaptor_error_code_h *error_code);

/**
 * @brief Creates auth adaptor
 */
EXPORT_API
auth_adaptor_h auth_adaptor_create(const char *plugins_dir);

/**
 * @brief Destroys auth adaptor. If auth adaptor was started it is stopped first.
 */
EXPORT_API
void auth_adaptor_destroy(auth_adaptor_h adaptor);

/**
 * @brief Starts auth adaptor and loads plugins that were found in plugins search dir
 * @brief specified in auth_adaptor_create
 */
EXPORT_API
int auth_adaptor_start(auth_adaptor_h adaptor);

/**
 * @brief Stops auth adaptor.
 */
EXPORT_API
int auth_adaptor_stop(auth_adaptor_h adaptor);

/**
 * @brief Registers plugin state listener
 */
EXPORT_API
int auth_adaptor_register_listener(auth_adaptor_h adaptor,
						auth_adaptor_listener_h listener);

/**
 * @brief Unregisters plugin state listener
 */
EXPORT_API
int auth_adaptor_unregister_listener(auth_adaptor_h adaptor,
						auth_adaptor_listener_h listener);

/**
 * @brief Creates plugin context.
 */
EXPORT_API
auth_adaptor_plugin_context_h auth_adaptor_create_plugin_context(auth_adaptor_plugin_h plugin,
						const char *user_id,
						const char *user_password,
						const char *app_id,
						const char *app_secret,
						const char *imsi,
						const char *service_name);

/**
 * @brief Destroys plugin context.
 */
EXPORT_API
void auth_adaptor_destroy_plugin_context(auth_adaptor_plugin_h plugin,
						auth_adaptor_plugin_context_h context);

/**
 * @brief Gets plugin with specified unique name
 */
EXPORT_API
auth_adaptor_plugin_h auth_adaptor_get_plugin_by_name(auth_adaptor_h adaptor,
						const char *plugin_uri);

/**
 * @brief Gets plugins
 */
EXPORT_API
GList *auth_adaptor_get_plugins(auth_adaptor_h adaptor);

/**********************************************************/
/* Adaptor get Element Functions                          */
/**********************************************************/

/*
 * @brief	Get Access token allocated by 'strdup'(Use after 'auth_adaptor_login' function)
 * @return : NULL or string(need free)
 */
EXPORT_API
char *auth_adaptor_get_access_token_dup(auth_adaptor_plugin_context_h context);

/*
 * @brief	Get duid allocated by 'strdup' (Use after 'auth_adaptor_login' function)
 * @return	NULL or string(need free)
 */
EXPORT_API
char *auth_adaptor_get_uid_dup(auth_adaptor_plugin_context_h context);

/*
 * @brief	Get MSISDN allocated by 'strdup' (Use after 'auth_adaptor_login' function)
 * @return	NULL or string(need free)
 */
EXPORT_API
char *auth_adaptor_get_msisdn_dup(auth_adaptor_plugin_context_h context);

/**********************************************************/
/* Adaptor Plugin call Functions                          */
/**********************************************************/

/**
* @brief Check Account Registration [Sync API]
*
* @param[in]	plugin		specifies Auth-adaptor Plugin handle
* @param[in]	context		specifies Auth-adaptor Plugin context
* @param[in]	request		specifies Optional value for specific Plugin (Recommend Json Object format)
* @param[out]	is_auth		specifies Registered Flag (Registered : 1, Not registered : 0)
* @param[out]	error_code	specifies Error code
* @param[out]	response	specifies Optional value from specific Plugin (Recommend Json Object format)
* @return 0 on success, otherwise a positive error value
* @retval error code defined in auth_error_code_e - AUTH_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
auth_error_code_t auth_adaptor_is_auth(auth_adaptor_plugin_h plugin,
						auth_adaptor_plugin_context_h context,
						void *request,
						int *is_auth,
						auth_adaptor_error_code_h *error_code,
						void *response);
/**
* @brief Request Access Token (included auth_adaptor_context) [Sync API]
*
* @param[in]	plugin		specifies Auth-adaptor Plugin handle
* @param[in]	context		specifies Auth-adaptor Plugin context
* @param[in]	is_auth		specifies Registration Flag (Must be issued from "auth_adaptor_is_auth" Function)
* @param[in]	request		specifies Optional value for specific Plugin (Recommend Json Object format)
* @param[out]	error_code	specifies Error code
* @param[out]	response	specifies Optional value from specific Plugin (Recommend Json Object format)
* @return 0 on success, otherwise a positive error value
* @retval error code defined in auth_error_code_e - AUTH_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
auth_error_code_t auth_adaptor_login(auth_adaptor_plugin_h plugin,
						auth_adaptor_plugin_context_h context,
						int is_auth,
						void *request,
						auth_adaptor_error_code_h *error_code,
						void *response);

/**
* @brief Request for Refreshing Access Token (included auth_adaptor_context) [Sync API]
*
* @param[in]	plugin		specifies Auth-adaptor Plugin handle
* @param[in]	context		specifies Auth-adaptor Plugin context
* @param[in]	request		specifies Optional value for specific Plugin (Recommend Json Object format)
* @param[out]	error_code	specifies Error code
* @param[out]	response	specifies Optional value from specific Plugin (Recommend Json Object format)
* @return 0 on success, otherwise a positive error value
* @retval error code defined in auth_error_code_e - AUTH_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
auth_error_code_t auth_adaptor_login_refresh(auth_adaptor_plugin_h plugin,
						auth_adaptor_plugin_context_h context,
						void *request,
						auth_adaptor_error_code_h *error_code,
						void *response);

/**
* @brief Request Account Registration [Sync API]
*
* @param[in]	plugin		specifies Auth-adaptor Plugin handle
* @param[in]	context		specifies Auth-adaptor Plugin context
* @param[in]	device_id	specifies Device Unique ID
* @param[in]	request		specifies Optional value for specific Plugin (Recommend Json Object format)
* @param[out]	error_code	specifies Error code
* @param[out]	response	specifies Optional value from specific Plugin (Recommend Json Object format)
* @return 0 on success, otherwise a positive error value
* @retval error code defined in auth_error_code_e - AUTH_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
auth_error_code_t auth_adaptor_join(auth_adaptor_plugin_h plugin,
						auth_adaptor_plugin_context_h context,
						const char *device_id,
						void *request,
						auth_adaptor_error_code_h *error_code,
						void *response);

/**
* @brief Request Account Information (MSISDN) [Sync API]
*
* @param[in]	plugin		specifies Auth-adaptor Plugin handle
* @param[in]	context		specifies Auth-adaptor Plugin context
* @param[in]	request		specifies Optional value for specific Plugin (Recommend Json Object format)
* @param[out]	msisdn		specifies MSISDN
* @param[out]	error_code	specifies Error code
* @param[out]	response	specifies Optional value from specific Plugin (Recommend Json Object format)
* @return 0 on success, otherwise a positive error value
* @retval error code defined in auth_error_code_e - AUTH_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
auth_error_code_t auth_adaptor_get_msisdn(auth_adaptor_plugin_h plugin,
						auth_adaptor_plugin_context_h context,
						void *request,
						char **msisdn,
						auth_adaptor_error_code_h *error_code,
						void *response);

/**
* @brief Request Changing Service Status (Enable, Disable or etc...) [Sync API]
*
* @param[in]	plugin		specifies Auth-adaptor Plugin handle
* @param[in]	context		specifies Auth-adaptor Plugin context
* @param[in]	service_id	specifies Service ID (The value is depended by Server)
* @param[in]	status		specifies Service Status (The value is depended by Server)
* @param[in]	request		specifies Optional value for specific Plugin (Recommend Json Object format)
* @param[out]	error_code	specifies Error code
* @param[out]	response	specifies Optional value from specific Plugin (Recommend Json Object format)
* @return 0 on success, otherwise a positive error value
* @retval error code defined in auth_error_code_e - AUTH_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
auth_error_code_t auth_adaptor_set_service_status(auth_adaptor_plugin_h plugin,
						auth_adaptor_plugin_context_h context,
						const int service_id,
						const int status,
						void *request,
						auth_adaptor_error_code_h *error_code,
						void *response);

/**
* @brief Request Service Status [Sync API]
*
* @param[in]	plugin		specifies Auth-adaptor Plugin handle
* @param[in]	context		specifies Auth-adaptor Plugin context
* @param[in]	service_id	specifies Service ID (The value is depended by Server)
* @param[in]	request		specifies Optional value for specific Plugin (Recommend Json Object format)
* @param[out]	status		specifies Service Status (The value is depended by Server)
* @param[out]	error_code	specifies Error code
* @param[out]	response	specifies Optional value from specific Plugin (Recommend Json Object format)
* @return 0 on success, otherwise a positive error value
* @retval error code defined in auth_error_code_e - AUTH_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
auth_error_code_t auth_adaptor_get_service_status(auth_adaptor_plugin_h plugin,
						auth_adaptor_plugin_context_h context,
						const int service_id,
						void *request,
						int *status,
						auth_adaptor_error_code_h *error_code,
						void *response);

/**
* @brief Request Service Policy [Sync API]
*
* @param[in]	plugin		specifies Auth-adaptor Plugin handle
* @param[in]	context		specifies Auth-adaptor Plugin context
* @param[in]	service_id	specifies Service ID (The value is depended by Server)
* @param[in]	request		specifies Optional value for specific Plugin (Recommend Json Object format)
* @param[in]	default_status	specifies Service default Status Policy
* @param[in]	policy_feature	specifies Service Policy Feature
* @param[in]	policy_version	specifies Service Policy Version
* @param[in]	policy_doc_url	specifies Service Policy Document URL
* @param[out]	error_code	specifies Error code
* @param[out]	response	specifies Optional value from specific Plugin (Recommend Json Object format)
* @return 0 on success, otherwise a positive error value
* @retval error code defined in auth_error_code_e - AUTH_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
auth_error_code_t auth_adaptor_get_service_policy(auth_adaptor_plugin_h plugin,
						auth_adaptor_plugin_context_h context,
						const int service_id,
						void *request,
						char **default_status,
						char **policy_feature,
						char **policy_version,
						char **policy_doc_url,
						auth_adaptor_error_code_h *error_code,
						void *response);

/**
* @brief Request Permitted Server information for Account [Sync API]
*
* @param[in]	plugin		specifies Auth-adaptor Plugin handle
* @param[in]	context		specifies Auth-adaptor Plugin context
* @param[in]	request		specifies Optional value for specific Plugin (Recommend Json Object format)
* @param[out]	server_info	specifies server information (URL, Scheme, Port)
* @param[out]	error_code	specifies Error code
* @param[out]	response	specifies Optional value from specific Plugin (Recommend Json Object format)
* @return 0 on success, otherwise a positive error value
* @retval error code defined in auth_error_code_e - AUTH_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
auth_error_code_t auth_adaptor_get_server_info(auth_adaptor_plugin_h plugin,
						auth_adaptor_plugin_context_h context,
						void *request,
						GHashTable **server_info,
						auth_adaptor_error_code_h *error_code,
						void *response);

EXPORT_API
auth_error_code_t auth_adaptor_external_request(auth_adaptor_plugin_h plugin,
						auth_adaptor_plugin_context_h context,
						const char *api_uri,
						const unsigned char *req_bundle_raw,
						int req_len,
						unsigned char **res_bundle_raw,
						int *res_len,
						auth_adaptor_error_code_h *error_code);

#ifdef __cplusplus
}
#endif

#endif /* __AUTH_ADAPTOR_H__ */
