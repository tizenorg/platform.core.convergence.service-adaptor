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

#ifndef __PUSH_ADAPTOR_H__
#define __PUSH_ADAPTOR_H__

#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif

#include <glib.h>
#include <stdint.h>
#include <inttypes.h>

/**
* @brief Push Adaptor error code
*/
typedef enum push_error_code_e {
	PUSH_ADAPTOR_ERROR_NONE                     =  0,
	PUSH_ADAPTOR_ERROR_LAUNCH                    = 1,    /**< 1 ~ 99: internal error*/
	PUSH_ADAPTOR_ERROR_INIT                      = 2,
	PUSH_ADAPTOR_ERROR_DEINIT                    = 3,
	PUSH_ADAPTOR_ERROR_CREATE                    = 4,
	PUSH_ADAPTOR_ERROR_DESTROY                   = 5,
	PUSH_ADAPTOR_ERROR_START                     = 6,
	PUSH_ADAPTOR_ERROR_STOP                      = 7,
	PUSH_ADAPTOR_ERROR_CONNECT                   = 8,
	PUSH_ADAPTOR_ERROR_DISCONNECT                = 9,
	PUSH_ADAPTOR_ERROR_NOT_FOUND                 = 10,
	PUSH_ADAPTOR_ERROR_CORRUPTED                 = 11,
	PUSH_ADAPTOR_ERROR_UNSUPPORTED               = 12,
	PUSH_ADAPTOR_ERROR_INVALID_HANDLE            = 13,
	PUSH_ADAPTOR_ERROR_INVALID_ARGUMENT          = 14,
	PUSH_ADAPTOR_ERROR_INVALID_ARGUMENT_TYPE     = 15,
	PUSH_ADAPTOR_ERROR_NOT_AUTHORIZED            = 16,
	PUSH_ADAPTOR_ERROR_ADAPTOR_INTERNAL          = 17,
	PUSH_ADAPTOR_ERROR_PLUGIN_INTERNAL           = 18,
	PUSH_ADAPTOR_ERROR_SERVER_INTERNAL           = 19,
	PUSH_ADAPTOR_ERROR_DBUS                      = 20,
	PUSH_ADAPTOR_ERROR_CALLBACK_TIME_OUT         = 21,
	PUSH_ADAPTOR_ERROR_MAX
} push_error_code_t;

/**
* @brief Push Adaptor plugin handle
*/
typedef struct push_adaptor_plugin_s *push_adaptor_plugin_h;

/**
* @brief Push Adaptor handle
*/
typedef struct push_adaptor_s *push_adaptor_h;

/**
* @brief Push Adaptor plugin context structure
*/
typedef struct push_adaptor_plugin_context_s {
	char	*push_app_id;	/**< unique key >*/
	char	*plugin_uri;
	void	*plugin_info;

	int	state;		/* Adaptor defined and share with plugin (getter: adaptor, setter: plugin) */
} push_adaptor_plugin_context_t;
typedef struct push_adaptor_plugin_context_s *push_adaptor_plugin_context_h;

typedef enum {
	PUSH_ADAPTOR_STATE_DISCONNECTED	= 0,
	PUSH_ADAPTOR_STATE_CONNECTED	= 1,
} push_adaptor_connection_state_e;

/**
* @brief Push Adaptor error code
*/
typedef struct push_adaptor_error_code_s {
	int64_t	code;
	char	*msg;

} push_adaptor_error_code_t;
typedef struct push_adaptor_error_code_s *push_adaptor_error_code_h;

/**
* @brief Push Adaptor notification data
*/
typedef struct push_adaptor_notification_data_s {
	char		*data;
	char		*msg;
	long long int	time_stamp;
	char		*sender;
	char		*app_id;
	char		*session_info;
	char		*request_id;
	int		type;
} push_adaptor_notification_data_t;
typedef struct push_adaptor_notification_data_s *push_adaptor_notification_data_h;

/**
* @brief Callback for service_adaptor_connect_push_service API for Service Adaptor
*
* @param[in]    notification	specifies pushed notification message with data, session info, and etc.
* @param[in]    user_data       specifies user data passed when plugin context is created.
* @return       void
* @pre		push_adaptor_connect will invoke this callback.
* @see
*/
typedef void(*push_adaptor_service_on_notification_received_cb)
			(push_adaptor_notification_data_h notification, void *user_data);

/**
* @brief Callback for service_adaptor_connect_push_service API for push plugin
*
* @param[in]    notification    specifies pushed notification message with data, session info, and etc.
* @param[in]    user_data       specifies user_data passed when plugin context is created.
* @return       void
* @pre		push_adaptor_connect will invoke this callback.
* @see
*/
typedef void(*push_adaptor_plugin_on_notification_received_cb)
			(push_adaptor_notification_data_h notification, void *user_data);

/**
* @brief Push Adaptor listener for Service Adaptor
*/
typedef struct push_adaptor_listener_s {
	push_adaptor_service_on_notification_received_cb        _on_notification_received;

} push_adaptor_listener_t;
typedef struct push_adaptor_listener_s *push_adaptor_listener_h;

/**
* @brief Push Adaptor listener for plugins
*/
typedef struct push_adaptor_plugin_listener_s {
	push_adaptor_plugin_on_notification_received_cb         _on_notification_received;

} push_adaptor_plugin_listener_t;
typedef struct push_adaptor_plugin_listener_s *push_adaptor_plugin_listener_h;

/**
 * @brief Push Adaptor plugin handle
 */
typedef struct push_adaptor_plugin_handle_s {
	/**< mandatory >*/
	push_error_code_t (*create_context)(push_adaptor_plugin_context_h *context,
							const char *push_app_id);

	/**< mandatory >*/
	push_error_code_t (*destroy_context)(push_adaptor_plugin_context_h context);

	/**< mandatory >*/
	push_error_code_t (*destroy_handle)(struct push_adaptor_plugin_handle_s *handle);

	/**< mandatory >*/
	push_error_code_t (*set_listener)(push_adaptor_plugin_listener_h listener);

	/**< mandatory >*/
	push_error_code_t (*unset_listener)(void);

	/**< optional >*/
	push_error_code_t (*set_server_info)(push_adaptor_plugin_context_h context,
							GHashTable *server_info);

	/**< optional >*/
	push_error_code_t (*connect)(push_adaptor_plugin_context_h context);

	/**< optional >*/
	push_error_code_t (*disconnect)(push_adaptor_plugin_context_h context);

	/**< optional >*/
	push_error_code_t (*request_unread_notification)(push_adaptor_plugin_context_h context);

	/**< mandatory >*/
	char *plugin_uri;

} push_adaptor_plugin_handle_t;
typedef struct push_adaptor_plugin_handle_s *push_adaptor_plugin_handle_h;

/**
* @brief Creates Push Adaptor
*
* @param[in]    plugin_dir      specifies directory path where plugins are stored
* @return	push_adaptor_h on success, otherwise NULL value
*/
EXPORT_API
push_adaptor_h push_adaptor_create(const char *plugins_dir);

/**
* @brief Destroys push adaptor. If push adaptor was started it is stopped first.
*
* @param[in]    adaptor         specifies push adaptor handle to be destroyed
* @return       void
*/
EXPORT_API
void push_adaptor_destroy(push_adaptor_h adaptor);

/**
* @brief Starts push adaptor and loads plugins that were found in plugins search dir
* specified in push_adaptor_create
*
* @param[in]    adaptor		specifies push adaptor handle
* @return	0 on success, otherwise a positive error value
* @retval	error code defined in push_error_code_e - PUSH_ADAPTOR_ERROR_NONE if successful
*/
EXPORT_API
int push_adaptor_start(push_adaptor_h adaptor);

/**
* @brief Stops push adaptor
*
* @param[in]    adaptor         specifies push adaptor handle
* @return	0 on success, otherwise a positive error value
* @retval	error code defined in push_error_code_e - PUSH_ADAPTOR_ERROR_NONE if successful
*/
EXPORT_API
int push_adaptor_stop(push_adaptor_h adaptor);

/**
* @brief Registers plugin state listener
*
* @param[in]    adaptor         specifies push adaptor handle
* @param[in]	listener	specifies push adaptor listener handle
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in push_error_code_e - PUSH_ADAPTOR_ERROR_NONE if successful
*/
EXPORT_API
int push_adaptor_register_listener(push_adaptor_h adaptor,
						push_adaptor_listener_h listener);

/**
* @brief Unregisters plugin state listener
*
* @param[in]    adaptor         specifies push adaptor handle
* @param[in]    listener	specifies push adaptor listener handle
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in push_error_code_e - PUSH_ADAPTOR_ERROR_NONE if successful
*/
EXPORT_API
int push_adaptor_unregister_listener(push_adaptor_h adaptor,
						push_adaptor_listener_h listener);

/**
* @brief Creates plugin context
*
* @param[in]    plugin		specifies push adaptor plugin handle
* @param[in]    push_app_id	specifies push service application ID
* @return       push_adaptor_plugin_context_h on success, otherwise NULL value
*/
EXPORT_API
push_adaptor_plugin_context_h push_adaptor_create_plugin_context(push_adaptor_plugin_h plugin,
						const char *plugin_uri,
						const char *push_app_id);

/**
* @brief Destroys plugin context
*
* @param[in]    plugin          specifies push adaptor plugin handle
* @param[in]    context		specifies push adaptor plugin context handle
* @return       void
*/
EXPORT_API
void push_adaptor_destroy_plugin_context(push_adaptor_plugin_h plugin,
						push_adaptor_plugin_context_h context);

/**
* @brief Gets plugin with specified unique name
*
* @param[in]    adaptor         specifies push adaptor handle
* @param[in]    plugin_name     specifies plugin name to be searched for
* @return       push_adaptor_plugin_h on success, otherwise NULL value
*/
EXPORT_API
push_adaptor_plugin_h push_adaptor_get_plugin_by_name(push_adaptor_h adaptor,
						const char *plugin_uri);

/**
* @brief Loads plugin from selected path
*
* @param[in]    adaptor         specifies push adaptor handle
* @param[in]    plugin_path     specifies plugin's saved path
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in push_error_code_e - PUSH_ADAPTOR_ERROR_NONE if successful
*/
EXPORT_API
int push_adaptor_load_plugin(push_adaptor_h adaptor,
						const char *plugin_path);

/**
* @brief Unloads selected plugin
*
* @param[in]    adaptor         specifies push adaptor handle
* @param[in]    plugin          specifies push adaptor plugin handle
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in push_error_code_e - PUSH_ADAPTOR_ERROR_NONE if successful
*/
EXPORT_API
int push_adaptor_unload_plugin(push_adaptor_h adaptor,
						push_adaptor_plugin_h plugin);

/**
* @brief Create error code
*
* @param[in]	code		specifies error code number
* @param[in]	msg		specifies error message
* @return	push_adaptor_error_code_h on success, otherwise NULL value
*/
EXPORT_API
push_adaptor_error_code_h push_adaptor_create_error_code(const int64_t code, const char *msg);

/**
* @brief Destroy error code
*
* @param[in]	error_code	specifies error code handle
* @return	void
*/
EXPORT_API
void push_adaptor_destroy_error_code(push_adaptor_error_code_h *error_code);

/**
* @brief Get plugin list
*
* @param[in]	adaptor		specifies push adaptor handle
* @return	GList pointer on success, otherwise NULL value
*/
EXPORT_API
GList *push_adaptor_get_plugins(push_adaptor_h adaptor);

/**
 * @brief Set server information for Push Plugin
 *
 * @param[in]    plugin		specifies Push Adaptor Plugin handle
 * @param[in]    context	specifies Push Adaptor Plugin Context handle
 * @param[in]    server_info	specifies server information for Push Plugin
 * @param[out]   error		specifies error code
* @return 0 on success, otherwise a positive error value
 * @retval error code defined in push_error_code_t - PUSH_ADAPTOR_ERROR_NONE if Successful
 */
EXPORT_API
push_error_code_t push_adaptor_set_server_info(push_adaptor_plugin_h plugin,
						push_adaptor_plugin_context_h context,
						GHashTable *server_info,
						push_adaptor_error_code_h *error);

/**
* @brief Connects to push service with Push App ID handed over when creates plugin context
*
* @param[in]    plugin          specifies push adaptor plugin handle
* @param[in]    context         specifies push adaptor plugin context handle
* @param[out]   error           specifies error code
* @return	0 on success, otherwise a positive error value
* @retval	error code defined in push_error_code_e - PUSH_ADAPTOR_ERROR_NONE if successful
*/
EXPORT_API
push_error_code_t push_adaptor_connect(push_adaptor_plugin_h plugin,
						push_adaptor_plugin_context_h context,
						push_adaptor_error_code_h *error);

/**
* @brief Disconnects from push service with Push App ID handed over when creates plugin context
*
* @param[in]    plugin          specifies push adaptor plugin handle
* @param[in]    context         specifies push adaptor plugin context handle
* @param[out]   error           specifies error code
* @return	0 on success, otherwise a positive error value
* @retval	error code defined in push_error_code_e - PUSH_ADAPTOR_ERROR_NONE if successful
*/
EXPORT_API
push_error_code_t push_adaptor_disconnect(push_adaptor_plugin_h plugin,
						push_adaptor_plugin_context_h context,
						push_adaptor_error_code_h *error);

EXPORT_API
push_error_code_t push_adaptor_is_connected(push_adaptor_plugin_h plugin,
						push_adaptor_plugin_context_h context,
						int *is_connected);

/**
* @brief Asynchronous request to get unread notifications
*
* @param[in]	plugin		specifies push adaptor plugin handle
* @param[in]    context		specifies push adaptor plugin context handle
* @param[out]	error		specifies error code
* @return	0 on success, otherwise a positive error value
* @retval	error code defined in push_error_code_e - PUSH_ADAPTOR_ERROR_NONE if successful
*/
EXPORT_API
push_error_code_t push_adaptor_request_unread_notification(push_adaptor_plugin_h plugin,
						push_adaptor_plugin_context_h context,
						push_adaptor_error_code_h *error);


/*
EXPORT_API
push_error_code_t push_adaptor_register(push_adaptor_plugin_h, push_adaptor_plugin_context_h,
					void *svc_in, void *opt_in, void **svc_out, void **opt_out);

EXPORT_API
push_error_code_t push_adaptor_deregister(push_adaptor_plugin_h, push_adaptor_plugin_context_h,
					void *svc_in, void *opt_in, void **svc_out, void **opt_out);

EXPORT_API
push_error_code_t push_adaptor_get_notification_data(push_adaptor_plugin_h, push_adaptor_plugin_context_h,
					void *svc_in, void *opt_in, void **svc_out, void **opt_out);

EXPORT_API
push_error_code_t push_adaptor_get_notification_message(push_adaptor_plugin_h, push_adaptor_plugin_context_h,
					void *svc_in, void *opt_in, void **svc_out, void **opt_out);

EXPORT_API
push_error_code_t push_adaptor_get_notification_time(push_adaptor_plugin_h, push_adaptor_plugin_context_h,
					void *svc_in, void *opt_in, void **svc_out, void **opt_out);

EXPORT_API
push_error_code_t push_adaptor_get_unread_notification(push_adaptor_plugin_h, push_adaptor_plugin_context_h,
					void *svc_in, void *opt_in, void **svc_out, void **opt_out);

EXPORT_API
push_error_code_t push_adaptor_get_registration_id(push_adaptor_plugin_h, push_adaptor_plugin_context_h,
					void *svc_in, void *opt_in, void **svc_out, void **opt_out);

EXPORT_API
push_error_code_t push_adaptor_free_notification(push_adaptor_plugin_h, push_adaptor_plugin_context_h,
					void *svc_in, void *opt_in, void **svc_out, void **opt_out);
*/

#endif /* __PUSH_ADAPTOR_H__ */
