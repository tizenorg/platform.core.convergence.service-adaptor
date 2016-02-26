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
 * File: shop-adaptor.h
 * Desc: Shop Adaptor APIs
 * Created on: Oct, 2014
 * Auth: Jaehoon Lee <j_hoon85.lee@samsung.com>
 *
 *****************************************************************************/
/**
 *      @file           shop-adaptor.h
 *      @brief         Defines interface of Shop Adaptor
 *      @version    0.1
 */

#ifndef __SHOP_ADAPTOR_H__
#define __SHOP_ADAPTOR_H__

#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif

#include <glib.h>
#include <stdint.h>
/**
 * @ brief Shop adaptor error code
 */
typedef enum shop_error_code_e
{
	SHOP_ADAPTOR_ERROR_NONE                     =  0,
	SHOP_ADAPTOR_ERROR_LAUNCH                    = 1,    /**< 1 ~ 99: internal error*/
	SHOP_ADAPTOR_ERROR_INIT                      = 2,
	SHOP_ADAPTOR_ERROR_DEINIT                    = 3,
	SHOP_ADAPTOR_ERROR_CREATE                    = 4,
	SHOP_ADAPTOR_ERROR_DESTROY                   = 5,
	SHOP_ADAPTOR_ERROR_START                     = 6,
	SHOP_ADAPTOR_ERROR_STOP                      = 7,
	SHOP_ADAPTOR_ERROR_CONNECT                   = 8,
	SHOP_ADAPTOR_ERROR_DISCONNECT                = 9,
	SHOP_ADAPTOR_ERROR_NOT_FOUND                 = 10,
	SHOP_ADAPTOR_ERROR_CORRUPTED                 = 11,
	SHOP_ADAPTOR_ERROR_UNSUPPORTED               = 12,
	SHOP_ADAPTOR_ERROR_INVALID_HANDLE            = 13,
	SHOP_ADAPTOR_ERROR_INVALID_ARGUMENT          = 14,
	SHOP_ADAPTOR_ERROR_INVALID_ARGUMENT_TYPE     = 15,
	SHOP_ADAPTOR_ERROR_NOT_AUTHORIZED            = 16,
	SHOP_ADAPTOR_ERROR_ADAPTOR_INTERNAL          = 17,
	SHOP_ADAPTOR_ERROR_PLUGIN_INTERNAL           = 18,
	SHOP_ADAPTOR_ERROR_SERVER_INTERNAL           = 19,
	SHOP_ADAPTOR_ERROR_DBUS                      = 20,
	SHOP_ADAPTOR_ERROR_CALLBACK_TIME_OUT         = 21,
	SHOP_ADAPTOR_ERROR_MAX
} shop_error_code_t;


/**
 *  @ brief Shop adaptor plugin handle
 */
typedef struct shop_adaptor_plugin_s *shop_adaptor_plugin_h;

/**
 *  @ brief Shop adaptor
 */
typedef struct shop_adaptor_s *shop_adaptor_h;

/**
* @brief Describes infromation about shop
*/
typedef struct _service_adaptor_shop_info_s
{
	int category_id;		/**< specifies category id of item*/
	long item_id;			/**< specifies item id*/
	long sticker_id;		/**< specifies sticker id*/
	char *lang_cd;			/**< specifies display language type*/
	char *cntry_cd;			/**< specifies country code*/
	int rwidth;			/**< specifies device resolution width*/
	int rheight;			/**< specifies device resolution height*/
	int start_idx;			/**< default value : 0*/
	int count;			/**< default value : 5*/
} shop_adaptor_shop_info_s;

/**
* @ brief Describes infromation about item
*/
typedef struct _service_adaptor_shop_item_s
{
	long item_id;			/**< specifies item id*/
	int category_id;		/**< specifies category id of item*/
	long *sticker_ids;		/**< specifies sticker id*/
	unsigned int sticker_ids_len; /**< specifies sticker id list length*/
	char *title;			/**< specifies title*/
	char *character;		/**< specifies character*/
	int version;			/**< specifies version*/
	char *download_url;	/**< specifies download_url*/
	char *panel_url;		/**< specifies panel_url*/
	char *sticker_url;		/**< specifies sticker_url*/
	long file_size;			/**< specifies file_size*/
	int count;			/**< specifies count*/
	char *character_code;		/**< specifies character_code*/
	long long int startdate;			/**< specifies startdate*/
	long long int enddate;			/**< specifies enddate*/
	long long int expired_date;		/**< specifies expired_date*/
	long long int valid_period;		/**< specifies valid_period*/
} shop_adaptor_shop_item_s;


/**
 * @ brief Shop adaptor plugin context structure
 */
typedef struct shop_adaptor_plugin_context_s
{
	char *duid;
	char *access_token;
	char *appid;
	char *apptype;
	char *plugin_uri;

} shop_adaptor_plugin_context_t;

typedef struct shop_adaptor_plugin_context_s *shop_adaptor_plugin_context_h;


typedef enum shop_plugin_result_code_e
{
	SHOP_PLUGIN_RESULT_SUCCEDED = 0,
	SHOP_PLUGIN_RESULT_FAILED = -1,
	SHOP_PLUGIN_RESULT_CANCELED = -2
} shop_plugin_result_code_t;

/**
 * @brief Enumerations of signal code for Service Adaptor
 */
typedef enum _shop_adaptor_error_e
{
	SERVICE_ADAPTOR_ERROR_NONE = 0,
	SERVICE_ADAPTOR_ERROR_INTERNAL = -1,
	SERVICE_ADAPTOR_ERROR_DBUS = -2,
	SERVICE_ADAPTOR_ERROR_MAX
} shop_adaptor_error_e;

/**
* @brief Describes infromation about Service Adaptor's error
*/
typedef struct shop_adaptor_error_code_s
{
	int64_t code;
        char *msg;
} shop_adaptor_error_code_t;
typedef struct shop_adaptor_error_code_s *shop_adaptor_error_code_h;

typedef struct shop_plugin_error_s
{
        char *code;
        char *msg;
} shop_plugin_error_t;

typedef struct shop_adaptor_listener_s
{
} shop_adaptor_listener_t;
typedef struct shop_adaptor_listener_s *shop_adaptor_listener_h;


/**
 * Shop adaptor listener for plugins (example)
 * Listener is used by plugins
 */
typedef struct shop_adaptor_plugin_listener_s
{
} shop_adaptor_plugin_listener_t;
typedef struct shop_adaptor_plugin_listener_s *shop_adaptor_plugin_listener_h;

/**
 * @ brief Shop adaptor plugin handle
 */
typedef struct shop_adaptor_plugin_handle_s
{
	// Mandatory functions to handle plugin in adaptor
	//struct shop_adaptor_plugin_handle_s * (*create_plugin_handle)(void);

	shop_error_code_t (*create_context)(shop_adaptor_plugin_context_h *context,
						char *duid,
						char *access_token,
						char *app_id,
						char *apptype);

	shop_error_code_t (*destroy_context)(shop_adaptor_plugin_context_h context);

	shop_error_code_t (*destroy_handle)(struct shop_adaptor_plugin_handle_s *handle);
	shop_error_code_t (*set_listener)(shop_adaptor_plugin_listener_h listener);
	shop_error_code_t (*unset_listener)(void);
	// Mandatory end
	// Optional
	shop_error_code_t (*get_item_list_v1) (shop_adaptor_plugin_context_h handle,
						shop_adaptor_shop_info_s *info,
						void *user_data,
						shop_adaptor_shop_item_s ***items,
						unsigned int *items_len,
						shop_adaptor_error_code_h *error_code,
						void **server_data);

	shop_error_code_t (*download_item_package_v1)(shop_adaptor_plugin_context_h handle,
						shop_adaptor_shop_info_s *info,
						void *user_data,
						shop_adaptor_shop_item_s **item,
						shop_adaptor_error_code_h *error_code,
						void **server_data);

	shop_error_code_t (*download_sticker_v1)(shop_adaptor_plugin_context_h handle,
						shop_adaptor_shop_info_s *info,
						void *user_data,
						shop_adaptor_shop_item_s **item,
						shop_adaptor_error_code_h *error_code,
						void **server_data);

	shop_error_code_t (*get_panel_url_v1)(shop_adaptor_plugin_context_h handle,
						shop_adaptor_shop_info_s *info,
						void *user_data,
						shop_adaptor_shop_item_s **item,
						shop_adaptor_error_code_h *error_code,
						void **server_data);

	shop_error_code_t (*set_server_info)(shop_adaptor_plugin_context_h handle,
						GHashTable *server_info,
						shop_adaptor_error_code_h *error_code);
	// Optional end

	// Mandatory
	char *plugin_uri;		// get from config file
	// Mandatory end

} shop_adaptor_plugin_handle_t;
typedef struct shop_adaptor_plugin_handle_s *shop_adaptor_plugin_handle_h;



/**
 * @ brief Loads plugin from selected path
 */
int shop_adaptor_load_plugin(shop_adaptor_h adaptor, const char *plugin_path);

/**
 * @ brief Unloads selected plugin
 */
int shop_adaptor_unload_plugin(shop_adaptor_h adaptor, shop_adaptor_plugin_h plugin);

/**
 * @ brief Refresh access token
 */
EXPORT_API
shop_error_code_t shop_adaptor_refresh_access_token(shop_adaptor_plugin_context_h context,
						const char *new_access_token);

/**
 * @ brief Refresh uid
 */
EXPORT_API
shop_error_code_t shop_adaptor_refresh_uid(shop_adaptor_plugin_context_h context,
						const char *new_uid);

/**
 * @ brief Create error code
 */
EXPORT_API
shop_adaptor_error_code_h shop_adaptor_create_error_code(const int64_t code, const char *msg);

/**
 * @ brief Destroy error code
 */
EXPORT_API
void shop_adaptor_destroy_error_code(shop_adaptor_error_code_h *error_code);

/**
 * @ brief Creates shop adaptor
 */
EXPORT_API
shop_adaptor_h shop_adaptor_create(const char *plugins_dir);

/**
 * @ brief Destroys shop adaptor
 * @ brief Destroys shop adaptor. If shop adaptor was started it is stopped first.
 */
EXPORT_API
void shop_adaptor_destroy(shop_adaptor_h adaptor);

/**
 * @ brief Starts shop adaptor
 * @ brief Starts shop adaptor and loads plugins that were found in plugins search dir
 * @ brief specified in shop_adaptor_create
 */
EXPORT_API
int shop_adaptor_start(shop_adaptor_h adaptor);

/**
 * @ brief Stops shop adaptor.
 */
EXPORT_API
int shop_adaptor_stop(shop_adaptor_h adaptor);

/**
 * @ brief Registers plugin state listener
 */
EXPORT_API
int shop_adaptor_register_listener(shop_adaptor_h adaptor, shop_adaptor_listener_h listener);

/**
 * @ brief Unregisters plugin state listener
 */
EXPORT_API
int shop_adaptor_unregister_listener(shop_adaptor_h adaptor, shop_adaptor_listener_h listener);

/**
 * @ brief Creates plugin context.
 */
EXPORT_API
shop_adaptor_plugin_context_h shop_adaptor_create_plugin_context(shop_adaptor_plugin_h plugin,
						char *plugin_uri,
						char *duid,
						char *access_token,
						char *app_id,
						char *apptype);

/**
 * @ brief Destroys plugin context.
 */
EXPORT_API
void shop_adaptor_destroy_plugin_context(shop_adaptor_plugin_h plugin, shop_adaptor_plugin_context_h plugin_context);

/**
 * @ brief Gets plugin with specified unique name
 */
EXPORT_API
shop_adaptor_plugin_h shop_adaptor_get_plugin_by_name(shop_adaptor_h adaptor, const char *plugin_name);

/**
 * @brief Set server information for Shop Plugin
 *
 * @param[in]    plugin		specifies Shop Adaptor Plugin handle
 * @param[in]    context	specifies Shop Adaptor Plugin Context handle
 * @param[in]    server_info	specifies server information for Shop Plugin
 * @param[out]   error		specifies error code
* @return 0 on success, otherwise a positive error value
 * @retval error code defined in shop_error_code_t - SHOP_ADAPTOR_ERROR_NONE if Successful
 */
EXPORT_API
shop_error_code_t shop_adaptor_set_server_info(shop_adaptor_plugin_h plugin,
						shop_adaptor_plugin_context_h context,
						GHashTable *server_info,
						shop_adaptor_error_code_h *error_code);

////////////////////////////////////////////////////////////
// Adaptor Plugin call Functions
////////////////////////////////////////////////////////////

/**
* @brief Get List of Item
*
* @param[in]	plugin		specifies Shop-adaptor Plugin handle
* @param[in]	context		specifies Shop-adaptor Plugin context
* @param[in]	info			specifies Properties of Items to be requested
* @param[in]	user_data	specifies Additional Input(Recommend Json Object format, Optional value)
* @param[out]items		specifies Item List(array)
* @param[out]items_len		specifies The number of Item List
* @param[out]	error_code	specifies Error code
* @param[out]server_data	specifies Reply from Server(JSON format, Optional value)
* @return 0 on success, otherwise a negative error value
* @retval error code defined in shop_error_code_e - SHOP_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
shop_error_code_t shop_adaptor_get_item_list_v1 (shop_adaptor_plugin_h plugin,
						shop_adaptor_plugin_context_h context,
						shop_adaptor_shop_info_s *info,
						void *user_data,
						shop_adaptor_shop_item_s ***items,
						unsigned int *items_len,
						shop_adaptor_error_code_h *error_code,
						void **server_data);

/**
* @brief Get Item information for Download
*
* @param[in]	plugin		specifies Shop-adaptor Plugin handle
* @param[in]	context		specifies Shop-adaptor Plugin context
* @param[in]	info			specifies Properties of Items to be requested
* @param[in]	user_data	specifies Additional Input(Recommend Json Object format, Optional value)
* @param[out]item			specifies Item List
* @param[out]	error_code	specifies Error code
* @param[out]server_data	specifies Reply from Server(JSON format, Optional value)
* @return 0 on success, otherwise a negative error value
* @retval error code defined in shop_error_code_e - SHOP_ADAPTOR_ERROR_NONE if Successful
*/

EXPORT_API
shop_error_code_t shop_adaptor_download_item_package_v1(shop_adaptor_plugin_h plugin,
						shop_adaptor_plugin_context_h context,
						shop_adaptor_shop_info_s *info,
						void *user_data,
						shop_adaptor_shop_item_s **item,
						shop_adaptor_error_code_h *error_code,
						void **server_data);

/**
* @brief Get Sticker information for Download
*
* @param[in]	plugin		specifies Shop-adaptor Plugin handle
* @param[in]	context		specifies Shop-adaptor Plugin context
* @param[in]	info			specifies Properties of Items to be requested
* @param[in]	user_data	specifies Additional Input(Recommend Json Object format, Optional value)
* @param[out]item			specifies Item List
* @param[out]	error_code	specifies Error code
* @param[out]server_data	specifies Reply from Server(JSON format, Optional value)
* @return 0 on success, otherwise a negative error value
* @retval error code defined in shop_error_code_e - SHOP_ADAPTOR_ERROR_NONE if Successful
*/

EXPORT_API
shop_error_code_t shop_adaptor_download_sticker_v1(shop_adaptor_plugin_h plugin,
						shop_adaptor_plugin_context_h context,
						shop_adaptor_shop_info_s *info,
						void *user_data,
						shop_adaptor_shop_item_s **item,
						shop_adaptor_error_code_h *error_code,
						void **server_data);

/**
* @brief Get Item Panel URL
*
* @param[in]	plugin		specifies Shop-adaptor Plugin handle
* @param[in]	context		specifies Shop-adaptor Plugin context
* @param[in]	info			specifies Properties of Items to be requested
* @param[in]	user_data	specifies Additional Input(Recommend Json Object format, Optional value)
* @param[out]item			specifies Item List
* @param[out]	error_code	specifies Error code
* @param[out]server_data	specifies Reply from Server(JSON format, Optional value)
* @return 0 on success, otherwise a negative error value
* @retval error code defined in shop_error_code_e - SHOP_ADAPTOR_ERROR_NONE if Successful
*/

EXPORT_API
shop_error_code_t shop_adaptor_get_panel_url_v1(shop_adaptor_plugin_h plugin,
						shop_adaptor_plugin_context_h context,
						shop_adaptor_shop_info_s *info,
						void *user_data,
						shop_adaptor_shop_item_s **item,
						shop_adaptor_error_code_h *error_code,
						void **server_data);

#endif /* __SHOP_ADAPTOR_H__ */

