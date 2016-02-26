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

#ifndef __CONTACT_ADAPTOR_H__
#define __CONTACT_ADAPTOR_H__

#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif

#include <glib.h>
#include <stdint.h>
#include <inttypes.h>

/**
* @brief Contact Adaptor error code
*/
typedef enum contact_error_code_e
{
	CONTACT_ADAPTOR_ERROR_NONE                     =  0,
	CONTACT_ADAPTOR_ERROR_LAUNCH                    = 1,    /**< 1 ~ 99: internal error*/
        CONTACT_ADAPTOR_ERROR_INIT                      = 2,
        CONTACT_ADAPTOR_ERROR_DEINIT                    = 3,
        CONTACT_ADAPTOR_ERROR_CREATE                    = 4,
        CONTACT_ADAPTOR_ERROR_DESTROY                   = 5,
        CONTACT_ADAPTOR_ERROR_START                     = 6,
        CONTACT_ADAPTOR_ERROR_STOP                      = 7,
        CONTACT_ADAPTOR_ERROR_CONNECT                   = 8,
        CONTACT_ADAPTOR_ERROR_DISCONNECT                = 9,
        CONTACT_ADAPTOR_ERROR_NOT_FOUND                 = 10,
        CONTACT_ADAPTOR_ERROR_CORRUPTED                 = 11,
        CONTACT_ADAPTOR_ERROR_UNSUPPORTED               = 12,
        CONTACT_ADAPTOR_ERROR_INVALID_HANDLE            = 13,
        CONTACT_ADAPTOR_ERROR_INVALID_ARGUMENT          = 14,
        CONTACT_ADAPTOR_ERROR_INVALID_ARGUMENT_TYPE     = 15,
        CONTACT_ADAPTOR_ERROR_NOT_AUTHORIZED            = 16,
        CONTACT_ADAPTOR_ERROR_ADAPTOR_INTERNAL          = 17,
        CONTACT_ADAPTOR_ERROR_PLUGIN_INTERNAL           = 18,
        CONTACT_ADAPTOR_ERROR_SERVER_INTERNAL           = 19,
        CONTACT_ADAPTOR_ERROR_DBUS                      = 20,
        CONTACT_ADAPTOR_ERROR_CALLBACK_TIME_OUT         = 21,
	CONTACT_ADAPTOR_ERROR_MAX
} contact_error_code_t;


typedef enum _contact_plugin_internal_error_code_e
{
	CONTACT_PLUGIN_ERROR_HTTP_BAD_REQUEST		= 400,
	CONTACT_PLUGIN_ERROR_HTTP_UNAUTHORIZED		= 401,
	CONTACT_PLUGIN_ERROR_HTTP_FORBIDDEN		= 403,
	CONTACT_PLUGIN_ERROR_HTTP_NOT_FOUND		= 404,
	CONTACT_PLUGIN_ERROR_HTTP_METHOD_NOT_ALLOWED	= 405,

	CONTACT_PLUGIN_ERROR_CURL_COULDNT_CONNECT	= 601,
	CONTACT_PLUGIN_ERROR_CURL_TIME_OUT		= 602,
	CONTACT_PLUGIN_ERROR_CURL_ETC			= 698,
	CONTACT_PLUGIN_ERROR_CURL_UNKNOWN		= 699,

	CONTACT_PLUGIN_ERROR_URL			= 997,
	CONTACT_PLUGIN_ERROR_ETC			= 998,
	CONTACT_PLUGIN_ERROR_UNKNOWN			= 999,
} contact_plugin_internal_error_code_e;




/**
* @brief Contact Adaptor plugin handle
*/
typedef struct contact_adaptor_plugin_s *contact_adaptor_plugin_h;

/**
* @brief Contact Adaptor handle
*/
typedef struct contact_adaptor_s *contact_adaptor_h;

/**
* @brief Contact Adaptor plugin context structure
*/
typedef struct contact_adaptor_plugin_context_s
{
	char	*duid;		/**< header: access control >*/
	char	*access_token;	/**< header: access control >*/
	void	*plugin_info;	/**< plugin's own context >*/

	// Adaptor define
	char	*plugin_uri;
	char	*service_name;

} contact_adaptor_plugin_context_t;
typedef struct contact_adaptor_plugin_context_s *contact_adaptor_plugin_context_h;

/**
* @brief Contact Adaptor error code
*/
typedef struct contact_adaptor_error_code_s
{
	int64_t	code;
	char	*msg;

} contact_adaptor_error_code_t;
typedef struct contact_adaptor_error_code_s *contact_adaptor_error_code_h;

/**
* @brief Contact Adaptor contact API information request format
*/
typedef struct contact_adaptor_contact_info_req_s
{
	char	*tp;	/**< mandatory >*/
	char	*id;	/**< mandatory >*/
	char	*pn;	/**< mandatory >*/
	char	*nm;	/**< mandatory >*/

	char	*cc;	/**< optional >*/

} contact_adaptor_contact_info_req_t;
typedef struct contact_adaptor_contact_info_req_s *contact_adaptor_contact_info_req_h;

/**
* @brief Contact Adaptor contact API request format
*/
typedef struct contact_adaptor_contact_req_s
{
	long long int				tt;
	contact_adaptor_contact_info_req_h	*cts;
	unsigned int				cts_len;

} contact_adaptor_contact_req_t;
typedef struct contact_adaptor_contact_req_s *contact_adaptor_contact_req_h;

typedef enum
{
	CONTACT_ADAPTOR_REQUEST_SET	= 1,
	CONTACT_ADAPTOR_REQUEST_DEL	= 2,
} contact_adaptor_request_type_e;

typedef struct contact_adaptor_contact_image_s
{
	int	req_type;
	int 	no;	/**< mandatory >*/
	char	*img;	/**< mandatory >*/
} contact_adaptor_contact_image_t;
typedef struct contact_adaptor_contact_image_s *contact_adaptor_contact_image_h;

/**
* @brief Contact Adaptor contact API information response format
*/
typedef struct contact_adaptor_contact_info_res_s
{
	char		*duid;		/**< mandatory >*/
	char		*id;		/**< mandatory >*/
	char		*msisdn;	/**< mandatory >*/

	char		*ty;		/**< optional >*/
	char		*cc;		/**< optional >*/
	char		*pn;		/**< optional >*/
	char		*nm;		/**< optional >*/
	char		**evnt;		/**< optional >*/
	unsigned int	evnt_len;	/**< optional >*/
	contact_adaptor_contact_image_h	*imgs;	/**< optional >*/
	unsigned int	imgs_len;	/**< optional >*/
	char		**adrs;		/**< optional >*/
	unsigned int	adrs_len;	/**< optional >*/
	char		**mail;		/**< optional >*/
	unsigned int	mail_len;	/**< optional >*/
	char		*org;		/**< optional >*/
	char		*prsc;		/**< optional >*/
	char		*status;	/**< optional >*/
	unsigned int	sids;		/**< optional >*/
	int 		type;		/**< optional >*/
	char 		*url;		/**< optional >*/
} contact_adaptor_contact_info_res_t;
typedef struct contact_adaptor_contact_info_res_s *contact_adaptor_contact_info_res_h;

/**
* @brief Contact Adaptor contact API response format
*/
typedef struct contact_adaptor_contact_res_s
{
        long long int				tt;
        contact_adaptor_contact_info_res_h	*cts;
        unsigned int				cts_len;

} contact_adaptor_contact_res_t;
typedef struct contact_adaptor_contact_res_s *contact_adaptor_contact_res_h;

/**
* @brief Contact Adaptor profile API request format
*/
typedef struct contact_adaptor_profile_req_s
{
	char*	cc;
	char*	pn;
	char*	nm;
	char**	evnt;
	unsigned int	evnt_len;
	char*	img;
	char**	adrs;
	unsigned int	adrs_len;
	char**	mail;
	unsigned int	mail_len;
	char*	org;
	char*	prsc;
	char* 	status;

} contact_adaptor_profile_req_t;
typedef struct contact_adaptor_profile_req_s *contact_adaptor_profile_req_h;

/**
* @brief Contact Adaptor profile API response format
*/
typedef struct contact_adaptor_profile_res_s
{
	char*	nm;
	char*	img;
	char*	prsc;
	char* 	status;

} contact_adaptor_profile_res_t;
typedef struct contact_adaptor_profile_res_s *contact_adaptor_profile_res_h;

/**
* @brief Contact Adaptor profile API image file path format
*/
typedef struct contact_adaptor_file_path_s
{
	char**	file_paths;
	unsigned int file_paths_len;

} contact_adaptor_file_path_t;
typedef struct contact_adaptor_file_path_s *contact_adaptor_file_path_h;

/**
* @brief Contact Adaptor privacy API information request format
*/
typedef struct contact_adaptor_privacy_info_req_s
{
	char*	cc;
	char*	pn;

} contact_adaptor_privacy_info_req_t;
typedef struct contact_adaptor_privacy_info_req_s *contact_adaptor_privacy_info_req_h;

/**
* @brief Contact Adaptor privacy API request format
*/
typedef struct contact_adaptor_privacy_req_s
{
	unsigned int	lvl;
	contact_adaptor_privacy_info_req_h*	cts;
	unsigned int	cts_len;

} contact_adaptor_privacy_req_t;
typedef struct contact_adaptor_privacy_req_s *contact_adaptor_privacy_req_h;

/**
* @brief Contact Adaptor privacy API response format
*/
typedef struct contact_adaptor_privacy_res_s
{
	unsigned int	lvl;
	unsigned int	prscon;

} contact_adaptor_privacy_res_t;
typedef struct contact_adaptor_privacy_res_s *contact_adaptor_privacy_res_h;

/**
* @brief Contact Adaptor presence API information format
*/
typedef struct contact_adaptor_presence_info_s
{
	char		*prsc;
	unsigned int	prscon;
	char		*status;

} contact_adaptor_presence_info_t;
typedef struct contact_adaptor_presence_info_s *contact_adaptor_presence_info_h;

/**
* @brief Callback on pushed message received for Service Adaptor (not in use)
*
* @param[in]	user_data	specifies user_data passed to message received API
* @return	void
*/
typedef void(*contact_adaptor_service_on_message_received_cb)(void *user_data);

/**
* @brief Callback on pushed message received for contact plugin (not in use)
*
* @param[in]	user_data       specifies user_data passed to message received API
* @return	void
*/
typedef void(*contact_adaptor_plugin_on_message_received_cb)(void *user_data);

/**
* @brief Contact Adaptor listener for Service Adaptor
*/
typedef struct contact_adaptor_listener_s
{
        contact_adaptor_service_on_message_received_cb _on_message_received;

} contact_adaptor_listener_t;
typedef struct contact_adaptor_listener_s *contact_adaptor_listener_h;

/**
* @brief Contact Adaptor listener for plugins
*/
typedef struct contact_adaptor_plugin_listener_s
{
        contact_adaptor_plugin_on_message_received_cb _on_message_received;

} contact_adaptor_plugin_listener_t;
typedef struct contact_adaptor_plugin_listener_s *contact_adaptor_plugin_listener_h;

/**
* @brief Contact Adaptor plugin handle
*/
typedef struct contact_adaptor_plugin_handle_s
{
	/**< mandatory >*/
	contact_error_code_t (*create_context)(contact_adaptor_plugin_context_h *context,
						const char *duid,
						const char *access_token);

	/**< mandatory >*/
	contact_error_code_t (*destroy_context)(contact_adaptor_plugin_context_h context);

	/**< mandatory >*/
	contact_error_code_t (*destroy_handle)(struct contact_adaptor_plugin_handle_s *handle);

	/**< mandatory >*/
	contact_error_code_t (*set_listener)(contact_adaptor_plugin_listener_h listener);

	/**< mandatory >*/
	contact_error_code_t (*unset_listener)(void);

	/**< optional >*/
	contact_error_code_t (*set_server_info)(contact_adaptor_plugin_context_h context,
						GHashTable *server_info,
						void *user_data,
						contact_adaptor_error_code_h *error,
						void **server_data);

	/**< optional >*/
	contact_error_code_t (*new_contact_list)(contact_adaptor_plugin_context_h context,
                                                contact_adaptor_contact_req_h request,
						void *user_data,
                                                contact_adaptor_contact_res_h *response,
						contact_adaptor_error_code_h *error,
						void **server_data);

	/**< optional >*/
	contact_error_code_t (*set_contact_list)(contact_adaptor_plugin_context_h context,
                                                contact_adaptor_contact_req_h request,
						void *user_data,
                                                contact_adaptor_contact_res_h *response,
						contact_adaptor_error_code_h *error,
						void **server_data);

	/**< optional >*/
	contact_error_code_t (*get_contact_infos_latest)(contact_adaptor_plugin_context_h context,
                                                contact_adaptor_contact_req_h request,
						void *user_data,
                                                contact_adaptor_contact_res_h *response,
						contact_adaptor_error_code_h *error,
						void **server_data);

	/**< optional >*/
        contact_error_code_t (*get_contact_infos_polling)(contact_adaptor_plugin_context_h context,
                                                contact_adaptor_contact_req_h request,
						void *user_data,
                                                contact_adaptor_contact_res_h *response,
						contact_adaptor_error_code_h *error,
						void **server_data);

	/**< optional >*/
        contact_error_code_t (*set_me_profile_with_push)(contact_adaptor_plugin_context_h context,
                                                contact_adaptor_profile_req_h request,
						void *user_data,
                                                contact_adaptor_profile_res_h *response,
						contact_adaptor_error_code_h *error,
						void **server_data);

	/**< optional >*/
        contact_error_code_t (*get_profile)(contact_adaptor_plugin_context_h context,
                                                contact_adaptor_profile_req_h request,
						void *user_data,
                                                contact_adaptor_profile_res_h *response,
						contact_adaptor_error_code_h *error,
						void **server_data);

	/**< optional >*/
        contact_error_code_t (*set_me_profile_image_meta_with_push)
						(contact_adaptor_plugin_context_h context,
						contact_adaptor_contact_image_h	*imgs,
						unsigned int imgs_len,
						void *user_data,
                                                contact_adaptor_error_code_h *error,
						void **server_data);

	/**< optional >*/
        contact_error_code_t (*delete_me_profile_image_meta_with_push)
						(contact_adaptor_plugin_context_h context,
						void *user_data,
                                                contact_adaptor_error_code_h *error,
						void **server_data);

	/**< optional >*/
        contact_error_code_t (*set_me_profile_privacy)(contact_adaptor_plugin_context_h context,
                                                contact_adaptor_privacy_req_h request,
						void *user_data,
                                                contact_adaptor_privacy_res_h *response,
                                                contact_adaptor_error_code_h *error,
						void **server_data);

	/**< optional >*/
        contact_error_code_t (*get_me_profile_privacy)(contact_adaptor_plugin_context_h context,
                                                contact_adaptor_privacy_req_h request,
						void *user_data,
                                                contact_adaptor_privacy_res_h *response,
                                                contact_adaptor_error_code_h *error,
						void **server_data);

	/**< optional >*/
        contact_error_code_t (*set_me_presence_with_push)(contact_adaptor_plugin_context_h context,
                                                contact_adaptor_presence_info_h request,
						void *user_data,
                                                contact_adaptor_presence_info_h *response,
						contact_adaptor_error_code_h *error,
						void **server_data);

	/**< optional >*/
        contact_error_code_t (*set_me_presence_on_off_with_push)(contact_adaptor_plugin_context_h context,
                                                contact_adaptor_presence_info_h request,
						void *user_data,
                                                contact_adaptor_presence_info_h *response,
						contact_adaptor_error_code_h *error,
						void **server_data);

	contact_error_code_t (*set_me_profile_type)(contact_adaptor_plugin_context_h context,
						int req_type,
						void *user_data,
						char **url,
						contact_adaptor_error_code_h *error,
						void **server_data);

	/**< mandatory >*/
	char *plugin_uri;	// get from config file

} contact_adaptor_plugin_handle_t;
typedef struct contact_adaptor_plugin_handle_s *contact_adaptor_plugin_handle_h;

/**
* @brief Creates Contact Adaptor.
*
* @param[in]    plugin_dir      specifies directory path where plugins are stored
* @return       contact_adaptor_h on success, otherwise NULL value
*/
EXPORT_API
contact_adaptor_h contact_adaptor_create(const char *plugins_dir);

/**
* @brief Destroys contact adaptor. If contact adaptor was started it is stopped first.
*
* @param[in]	adaptor		specifies contact adaptor handle to be destroyed
* @return	void
*/
EXPORT_API
void contact_adaptor_destroy(contact_adaptor_h adaptor);

/**
* @brief Starts contact adaptor and loads plugins that are found in contact_adaptor_create().
*
* @param[in]	adaptor		specifies contact adaptor handle
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in contact_error_code_e - CONTACT_ADAPTOR_ERROR_NONE if successful
*/
EXPORT_API
int contact_adaptor_start(contact_adaptor_h adaptor);

/**
* @brief Stops contact adaptor.
*
* @param[in]    adaptor		specifies contact adaptor handle
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in contact_error_code_e - CONTACT_ADAPTOR_ERROR_NONE if successful
*/
EXPORT_API
int contact_adaptor_stop(contact_adaptor_h adaptor);

/**
* @brief Registers plugin state listener
*
* @param[in]    adaptor		specifies contact adaptor handle
* @param[in]	listener	specifies contact adaptor listener handle
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in contact_error_code_e - CONTACT_ADAPTOR_ERROR_NONE if successful
*/
EXPORT_API
int contact_adaptor_register_listener(contact_adaptor_h adaptor,
						contact_adaptor_listener_h listener);

/**
* @brief Unregisters plugin state listener
*
* @param[in]    adaptor		specifies contact adaptor handle
* @param[in]    listener	specifies contact adaptor listener handle
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in contact_error_code_e - CONTACT_ADAPTOR_ERROR_NONE if successful
*/
EXPORT_API
int contact_adaptor_unregister_listener(contact_adaptor_h adaptor,
						contact_adaptor_listener_h listener);

/**
* @brief Creates plugin context
*
* @param[in]	plugin		specifies contact adaptor plugin handle
* @param[in]	duid		specifies device unique ID
* @param[in]	access_token	specifies access token issued by Auth Adaptor
* @return	contact_adaptor_plugin_context_h on success, otherwise NULL value
*/
EXPORT_API
contact_adaptor_plugin_context_h contact_adaptor_create_plugin_context(contact_adaptor_plugin_h plugin,
						const char *duid,
						const char *access_token,
						const char *service_name);

/**
* @brief Destroys plugin context.
*
* @param[in]	plugin		specifies contact adaptor plugin handle
* @param[in]	context		specifies contact adaptor plugin context handle
* @return	void
*/
EXPORT_API
void contact_adaptor_destroy_plugin_context(contact_adaptor_plugin_h plugin,
						contact_adaptor_plugin_context_h context);

/**
* @brief Gets plugin with specified unique name
*
* @param[in]    adaptor         specifies contact adaptor handle
* @param[in]	plugin_name	specifies plugin name to be searched for
* @return	contact_adaptor_plugin_h on success, otherwise NULL value
*/
EXPORT_API
contact_adaptor_plugin_h contact_adaptor_get_plugin_by_name(contact_adaptor_h adaptor,
						const char *plugin_uri);


/**
* @brief Loads plugin from selected path
*
* @param[in]	adaptor         specifies contact adaptor handle
* @param[in]	plugin_path	specifies plugin's saved path
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in contact_error_code_e - CONTACT_ADAPTOR_ERROR_NONE if successful
*/
EXPORT_API
int contact_adaptor_load_plugin(contact_adaptor_h adaptor,
						const char *plugin_path);

/**
* @brief Unloads selected plugin
*
* @param[in]	adaptor		specifies contact adaptor handle
* @param[in]	plugin		specifies contact adaptor plugin handle
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in contact_error_code_e - CONTACT_ADAPTOR_ERROR_NONE if successful
*/
EXPORT_API
int contact_adaptor_unload_plugin(contact_adaptor_h adaptor,
						contact_adaptor_plugin_h plugin);

/**
* @brief Get plugin list of contact adaptor handle has
*
* @param[in]	adaptor		specifies contact adaptor handle
* @return	GList pointer on success, otherwise NULL value
*/
EXPORT_API
GList *contact_adaptor_get_plugins(contact_adaptor_h adaptor);

/**
* @brief Gets plugin name
*
* @param[in]	plugin		specifies contact adaptor plugin handle
* @return	const char pointer on success, otherwise NULL value
*/
EXPORT_API
const char* contact_adaptor_get_plugin_name(contact_adaptor_plugin_h plugin);

/**
 * @brief Refresh access token was issued from auth-adaptor
 *
 * @param[in]	context			specifies Contact Adaptor Plugin Context handle
 * @param[in]	new_access_token	specifies New access token
 * @return 	contact_adaptor_error_code_h on success, otherwise NULL value
*/
EXPORT_API
contact_error_code_t contact_adaptor_refresh_access_token(contact_adaptor_plugin_context_h context,
						const char *new_access_token);

/**
 * @brief Refresh unique id was issued from auth-adaptor
 *
 * @param[in]	context			specifies Contact Adaptor Plugin Context handle
 * @param[in]	new_access_token	specifies New access token
 * @return 	contact_adaptor_error_code_h on success, otherwise NULL value
*/
EXPORT_API
contact_error_code_t contact_adaptor_refresh_uid(contact_adaptor_plugin_context_h context,
						const char *new_uid);

/**
* @brief Create error code
*
* @param[in]	code		specifies error code number
* @param[in]	msg             specifies error message
* @return 	contact_adaptor_error_code_h on success, otherwise NULL value
*/
EXPORT_API
contact_adaptor_error_code_h contact_adaptor_create_error_code(const int64_t code,
						const char *msg);

/**
* @brief Destroy error code
*
* @param[in]	error_code	specifies error code handle
* @return	void
*/
EXPORT_API
void contact_adaptor_destroy_error_code(contact_adaptor_error_code_h *error_code);

EXPORT_API
void contact_adaptor_destroy_contact_req_s(contact_adaptor_contact_req_h req);

EXPORT_API
void contact_adaptor_destroy_contact_res_s(contact_adaptor_contact_res_h res);

EXPORT_API
void contact_adaptor_destroy_profile_req_s(contact_adaptor_profile_req_h req);

EXPORT_API
void contact_adaptor_destroy_profile_res_s(contact_adaptor_profile_res_h res);

EXPORT_API
void contact_adaptor_destroy_file_path_s(contact_adaptor_file_path_h path);

EXPORT_API
void contact_adaptor_destroy_privacy_req_s(contact_adaptor_privacy_req_h req);

EXPORT_API
void contact_adaptor_destroy_privacy_res_s(contact_adaptor_privacy_res_h res);

EXPORT_API
void contact_adaptor_destroy_presence_info_s(contact_adaptor_presence_info_h info);

/**
 * @brief Set server information for Contact Plugin
 *
 * @param[in]    plugin		specifies Contact Adaptor Plugin handle
 * @param[in]    context	specifies Contact Adaptor Plugin Context handle
 * @param[in]    server_info	specifies server information for Contact Plugin
 * @param[in]    user_data	specifies optional parameter
 * @param[out]   error		specifies error code
 * @param[out]   server_data	specifies optional parameter
 * @return 0 on success, otherwise a positive error value
 * @retval error code defined in contact_error_code_t - CONTACT_ADAPTOR_ERROR_NONE if Successful
 */
EXPORT_API
contact_error_code_t contact_adaptor_set_server_info(contact_adaptor_plugin_h plugin,
					contact_adaptor_plugin_context_h context,
					GHashTable *server_info,
					void *user_data,
					contact_adaptor_error_code_h *error,
					void **server_data);

/**
* @brief Resets contact information in Contact server and upload native contact information of device to
* the server
*
* @param[in]	plugin		specifies contact adaptor plugin handle
* @param[in]	context		specifies contact adaptor plugin context handle
* @param[in]	request		specifies contact adaptor contact API request handle
* @param[in]	user_data	specifies user side arbitrary data
* @param[out]	response	specifies contact adaptor contact API response handle
* @param[out]	error           specifies returned error code handle
* @param[out]	server_data	specifies server side arbitrary data
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in contact_error_code_e - CONTACT_ADAPTOR_ERROR_NONE if successful
*/
EXPORT_API
contact_error_code_t contact_adaptor_new_contact_list(contact_adaptor_plugin_h plugin,
						contact_adaptor_plugin_context_h context,
                                                contact_adaptor_contact_req_h request,
						void *user_data,
                                                contact_adaptor_contact_res_h *response,
						contact_adaptor_error_code_h *error,
						void **server_data);

/**
* @brief Synchronized native contact information of device with contact server according to type
* "type" field of each contact
*
* @param[in]    plugin          specifies contact adaptor plugin handle
* @param[in]    context         specifies contact adaptor plugin context handle
* @param[in]    request         specifies contact adaptor contact API request handle
* @param[in]    user_data       specifies user side arbitrary data
* @param[out]   response        specifies contact adaptor contact API response handle
* @param[out]   error           specifies returned error code handle
* @param[out]   server_data     specifies server side arbitrary data
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in contact_error_code_e - CONTACT_ADAPTOR_ERROR_NONE if successful
*/
EXPORT_API
contact_error_code_t contact_adaptor_set_contact_list(contact_adaptor_plugin_h plugin,
						contact_adaptor_plugin_context_h context,
                                                contact_adaptor_contact_req_h request,
						void *user_data,
                                                contact_adaptor_contact_res_h *response,
						contact_adaptor_error_code_h *error,
						void **server_data);

/**
* @brief Gets profile and service registration information of each contact
*
* @param[in]    plugin          specifies contact adaptor plugin handle
* @param[in]    context         specifies contact adaptor plugin context handle
* @param[in]    request         specifies contact adaptor contact API request handle
* @param[in]    user_data       specifies user side arbitrary data
* @param[out]   response        specifies contact adaptor contact API response handle
* @param[out]   error           specifies returned error code handle
* @param[out]   server_data     specifies server side arbitrary data
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in contact_error_code_e - CONTACT_ADAPTOR_ERROR_NONE if successful
*/
EXPORT_API
contact_error_code_t contact_adaptor_get_contact_infos_latest(contact_adaptor_plugin_h plugin,
						contact_adaptor_plugin_context_h context,
                                                contact_adaptor_contact_req_h request,
						void *user_data,
                                                contact_adaptor_contact_res_h *response,
						contact_adaptor_error_code_h *error,
						void **server_data);

/**
* @brief Gets profile and service registration information of contact that have been updated since
* last update
*
* @param[in]    plugin          specifies contact adaptor plugin handle
* @param[in]    context         specifies contact adaptor plugin context handle
* @param[in]    request         specifies contact adaptor contact API request handle
* @param[in]    user_data       specifies user side arbitrary data
* @param[out]   response        specifies contact adaptor contact API response handle
* @param[out]   error           specifies returned error code handle
* @param[out]   server_data     specifies server side arbitrary data
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in contact_error_code_e - CONTACT_ADAPTOR_ERROR_NONE if successful
*/
EXPORT_API
contact_error_code_t contact_adaptor_get_contact_infos_polling(contact_adaptor_plugin_h plugin,
						contact_adaptor_plugin_context_h context,
                                                contact_adaptor_contact_req_h request,
						void *user_data,
                                                contact_adaptor_contact_res_h *response,
						contact_adaptor_error_code_h *error,
						void **server_data);

/**
* @brief Sets or updates device's profile to server
*
* @param[in]    plugin          specifies contact adaptor plugin handle
* @param[in]    context         specifies contact adaptor plugin context handle
* @param[in]    request         specifies contact adaptor profile API request handle
* @param[in]    user_data       specifies user side arbitrary data
* @param[out]   response        specifies contact adaptor profile API response handle
* @param[out]   error           specifies returned error code handle
* @param[out]   server_data     specifies server side arbitrary data
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in contact_error_code_e - CONTACT_ADAPTOR_ERROR_NONE if successful
*/
EXPORT_API
contact_error_code_t contact_adaptor_set_me_profile_with_push(contact_adaptor_plugin_h plugin,
                                                contact_adaptor_plugin_context_h context,
                                                contact_adaptor_profile_req_h request,
						void *user_data,
                                                contact_adaptor_profile_res_h *response,
						contact_adaptor_error_code_h *error,
						void **server_data);

/**
* @brief Gets the profile information of a contact which is correspondent with country code and phone number
*
* @param[in]    plugin          specifies contact adaptor plugin handle
* @param[in]    context         specifies contact adaptor plugin context handle
* @param[in]    request         specifies contact adaptor profile API request handle
* @param[in]    user_data       specifies user side arbitrary data
* @param[out]   response        specifies contact adaptor profile API response handle
* @param[out]   error           specifies returned error code handle
* @param[out]   server_data     specifies server side arbitrary data
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in contact_error_code_e - CONTACT_ADAPTOR_ERROR_NONE if successful
*/
EXPORT_API
contact_error_code_t contact_adaptor_get_profile(contact_adaptor_plugin_h plugin,
                                                contact_adaptor_plugin_context_h context,
                                                contact_adaptor_profile_req_h request,
						void *user_data,
                                                contact_adaptor_profile_res_h *response,
						contact_adaptor_error_code_h *error,
						void **server_data);

/**
* @brief Uploads profile image meta to file server
*
* @param[in]    plugin          specifies contact adaptor plugin handle
* @param[in]    context         specifies contact adaptor plugin context handle
* @param[in]    request         specifies contact adaptor profile API image file request handle
* @param[in]    user_data       specifies user side arbitrary data
* @param[out]   response        specifies contact adaptor profile API image file response handle
* @param[out]   error           specifies returned error code handle
* @param[out]   server_data     specifies server side arbitrary data
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in contact_error_code_e - CONTACT_ADAPTOR_ERROR_NONE if successful
*/
EXPORT_API
contact_error_code_t contact_adaptor_set_me_profile_image_meta_with_push(contact_adaptor_plugin_h plugin,
                                                contact_adaptor_plugin_context_h context,
						contact_adaptor_contact_image_h	*imgs,
						unsigned int imgs_len,
						void *user_data,
						contact_adaptor_error_code_h *error,
						void **server_data);

/**
* @brief Deletes profile image meta from profile server
*
* @param[in]    plugin          specifies contact adaptor plugin handle
* @param[in]    context         specifies contact adaptor plugin context handle
* @param[in]    request         specifies contact adaptor profile API image file request handle
* @param[in]    user_data       specifies user side arbitrary data
* @param[out]   response        specifies contact adaptor profile API image file response handle
* @param[out]   error           specifies returned error code handle
* @param[out]   server_data     specifies server side arbitrary data
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in contact_error_code_e - CONTACT_ADAPTOR_ERROR_NONE if successful
*/
EXPORT_API
contact_error_code_t contact_adaptor_delete_me_profile_image_meta_with_push(
						contact_adaptor_plugin_h plugin,
                                                contact_adaptor_plugin_context_h context,
						void *user_data,
						contact_adaptor_error_code_h *error,
						void **server_data);

/**
* @brief Sets the level of privacy
*
* @param[in]    plugin          specifies contact adaptor plugin handle
* @param[in]    context         specifies contact adaptor plugin context handle
* @param[in]    request         specifies contact adaptor privacy API request handle
* @param[in]    user_data       specifies user side arbitrary data
* @param[out]   response        specifies contact adaptor privacy API response handle
* @param[out]   error           specifies returned error code handle
* @param[out]   server_data     specifies server side arbitrary data
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in contact_error_code_e - CONTACT_ADAPTOR_ERROR_NONE if successful
*/
EXPORT_API
contact_error_code_t contact_adaptor_set_me_profile_privacy(contact_adaptor_plugin_h plugin,
                                                contact_adaptor_plugin_context_h context,
                                                contact_adaptor_privacy_req_h request,
						void *user_data,
                                                contact_adaptor_privacy_res_h *response,
						contact_adaptor_error_code_h *error,
						void **server_data);

/**
* @brief Gets my profile's privacy level
*
* @param[in]    plugin          specifies contact adaptor plugin handle
* @param[in]    context         specifies contact adaptor plugin context handle
* @param[in]    request         specifies contact adaptor privacy API request handle
* @param[in]    user_data       specifies user side arbitrary data
* @param[out]   response        specifies contact adaptor privacy API response handle
* @param[out]   error           specifies returned error code handle
* @param[out]   server_data     specifies server side arbitrary data
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in contact_error_code_e - CONTACT_ADAPTOR_ERROR_NONE if successful
*/
EXPORT_API
contact_error_code_t contact_adaptor_get_me_profile_privacy(contact_adaptor_plugin_h plugin,
                                                contact_adaptor_plugin_context_h context,
                                                contact_adaptor_privacy_req_h request,
						void *user_data,
                                                contact_adaptor_privacy_res_h *response,
						contact_adaptor_error_code_h *error,
						void **server_data);

/**
* @brief Sets my presence information
*
* @param[in]    plugin          specifies contact adaptor plugin handle
* @param[in]    context         specifies contact adaptor plugin context handle
* @param[in]    request         specifies contact adaptor presence API request handle
* @param[in]    user_data       specifies user side arbitrary data
* @param[out]   response        specifies contact adaptor presence API response handle
* @param[out]   error           specifies returned error code handle
* @param[out]   server_data     specifies server side arbitrary data
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in contact_error_code_e - CONTACT_ADAPTOR_ERROR_NONE if successful
*/
EXPORT_API
contact_error_code_t contact_adaptor_set_me_presence_with_push(contact_adaptor_plugin_h plugin,
                                                contact_adaptor_plugin_context_h context,
						contact_adaptor_presence_info_h request,
						void *user_data,
						contact_adaptor_presence_info_h *response,
						contact_adaptor_error_code_h *error,
						void **server_data);

/**
* @brief Sets my presence on/off information
*
* @param[in]    plugin          specifies contact adaptor plugin handle
* @param[in]    context         specifies contact adaptor plugin context handle
* @param[in]    request         specifies contact adaptor presence API request handle
* @param[in]    user_data       specifies user side arbitrary data
* @param[out]   response        specifies contact adaptor presence API response handle
* @param[out]   error           specifies returned error code handle
* @param[out]   server_data     specifies server side arbitrary data
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in contact_error_code_e - CONTACT_ADAPTOR_ERROR_NONE if successful
*/
EXPORT_API
contact_error_code_t contact_adaptor_set_me_presence_on_off_with_push(contact_adaptor_plugin_h plugin,
                                                contact_adaptor_plugin_context_h context,
                                                contact_adaptor_presence_info_h request,
						void *user_data,
                                                contact_adaptor_presence_info_h *response,
						contact_adaptor_error_code_h *error,
						void **server_data);

EXPORT_API
contact_error_code_t contact_adaptor_set_me_profile_type(contact_adaptor_plugin_h plugin,
                                                contact_adaptor_plugin_context_h context,
						int req_type,
						void *user_data,
						char **url,
						contact_adaptor_error_code_h *error,
						void **server_data);

#endif /* __CONTACT_ADAPTOR_H__ */
