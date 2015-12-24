/*
 * Service Adaptor IPC
 *
 * Copyright (c) 2014 - 2015 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Yongjin Kim <youth.kim@samsung.com>
 *          Jinhyeong Ahn <jinh.ahn@samsung.com>
 *          Jiwon Kim <jiwon177.kim@samsung.com>
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

#ifndef __TIZEN_CONVERGENCE_SAL_IPC_H__
#define __TIZEN_CONVERGENCE_SAL_IPC_H__

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#include <glib.h>
#include <gio/gio.h>

/**
 * ##################################
 * # { client - server IPC protocol
 */

/**
 * Service Adaptor D-Bus server bus name.
 */
#define SERVICE_ADAPTOR_BUS_NAME "org.tizen.serviceadaptor.client"

/**
 * Service Adaptor D-Bus server object path.
 */
#define SERVICE_ADAPTOR_OBJECT_PATH "/org/tizen/serviceadaptor/client"

/**
 * Service Adaptor D-Bus interface.
 */
#define SERVICE_ADAPTOR_INTERFACE "org.tizen.serviceadaptor.client.interface"

/**
 * struct for dbus.
 */
#define service_adaptor_connect_req_s_type_length 2
#define service_adaptor_connect_req_s_type \
	"(" \
	"i" /* int pid */ \
	"s" /* char *uri */ \
	")"

#define service_adaptor_connect_res_s_type_length 1
#define service_adaptor_connect_res_s_type \
	"(" \
	"a(s)" /* char **uri */ \
	")"

#define service_adaptor_disconnect_s_type_length 2
#define service_adaptor_disconnect_s_type \
	"(" \
	"i" /* int pid */ \
	"s" /* char *uri */ \
	")"

#define service_plugin_start_req_s_type_length 3
#define service_plugin_start_req_s_type \
	"(" \
	"i" /* int pid */ \
	"s" /* char *uri */ \
	"s" /* char *plugin_uri */ \
	")"

#define service_plugin_start_res_s_type_length 1
#define service_plugin_start_res_s_type \
	"(" \
	"s" /* char *plugin_handle */ \
	")"

#define service_plugin_stop_s_type_length 1
#define service_plugin_stop_s_type \
	"(" \
	"s" /* char *plugin_handle */ \
	")"

#define service_auth_oauth1_s_type_length 2
#define service_auth_oauth1_s_type \
	"(" \
	"s" /* char *access_token */ \
	"s" /* char *operation */ \
	")"

#define service_auth_oauth1_req_s_type_length 2
#define service_auth_oauth1_req_s_type \
	"(" \
	"s" /* char *uri */ \
	service_auth_oauth1_s_type /* service_auth_oauth1_s oauth1 */ \
	")"

#define service_auth_oauth1_res_s_type_length 1
#define service_auth_oauth1_res_s_type \
	"(" \
	service_auth_oauth1_s_type /* service_auth_oauth1_s oauth1 */ \
	")"

#define service_storage_cloud_file_s_type_length 6
#define service_storage_cloud_file_s_type \
	"(" \
	"b" /* bool is_dir */ \
	"s" /* char *dir_path */ \
	"s" /* char *local_path */ \
	"s" /* char *cloud_path */ \
	"t" /* uint64 size */ \
	"s" /* char *operation */ \
	")"

#define service_storage_cloud_file_req_s_type_length 2
#define service_storage_cloud_file_req_s_type \
	"(" \
	"s" /* char *uri */ \
	service_storage_cloud_file_s_type /* service_storage_cloud_file_s file */ \
	")"

#define service_storage_cloud_file_res_s_type_length 2
#define service_storage_cloud_file_res_s_type \
	"(" \
	service_storage_cloud_file_s_type /* service_storage_cloud_file_s file */ \
	"a" service_storage_cloud_file_s_type /* GList *files */  \
	")"

/**
 * array of structures
 */
#define plugin_list_type				"a(s)"
#define file_list_type					"a" service_storage_cloud_file_s_type

/**
 * append error code to the type
 */
#define RETURN_LENGTH					3

/**
 * DBus APIs
 */
#define DBUS_SERVICE_ADAPTOR			"dbus_00"
#define DBUS_SERVICE_AUTH				"dbus_02"
#define DBUS_SERVICE_STORAGE			"dbus_03"
#define DBUS_NAME_LENGTH				7

#define DBUS_SERVICE_ADAPTOR_CONNECT_METHOD		DBUS_SERVICE_ADAPTOR "_connect"
#define DBUS_SERVICE_ADAPTOR_DISCONNECT_METHOD	DBUS_SERVICE_ADAPTOR "_disconnect"

#define DBUS_SERVICE_PLUGIN_START_METHOD		DBUS_SERVICE_ADAPTOR "_plugin_start"
#define DBUS_SERVICE_PLUGIN_STOP_METHOD			DBUS_SERVICE_ADAPTOR "_plugin_stop"

#define DBUS_SERVICE_AUTH_OAUTH1_METHOD			DBUS_SERVICE_AUTH "_service_auth_oauth1"

#define DBUS_SERVICE_STORAGE_CLOUD_FILE_METHOD	DBUS_SERVICE_STORAGE "_service_storage_cloud_file"

#define DBUS_SERVICE_ADAPTOR_NOTIFY_SIGNAL		DBUS_SERVICE_ADAPTOR "_service_adaptor_signal_notify"

/**
 * # client - server IPC protocol End  }
 * ##################################
 */


/**
 * ##################################
 * # { adaptor - provider IPC protocol
 */

#define SERVICE_PROVIDER_BUS_NAME_PREFIX		"serviceprovider"

#define SERVICE_PROVIDER_BASE_INTERFACE		"base"
#define SERVICE_PROVIDER_AUTH_INTERFACE		"auth"
#define SERVICE_PROVIDER_STORAGE_INTERFACE	"storage"

/*
 * DBus Methods
 */

/*
 * Provider:base - Session start callback
**/
#define SERVICE_PROVIDER_BASE_SESSION_START		"session_start"
/* req param len */
#define SERVICE_PROVIDER_BASE_SESSION_START_REQ_LEN 1
/* req params */
#define SERVICE_PROVIDER_BASE_SESSION_START_REQ	\
	"(" \
	"s" /* client info */\
	")"
/* res param len */
#define SERVICE_PROVIDER_BASE_SESSION_START_RES_LEN 4
/* res params */
#define SERVICE_PROVIDER_BASE_SESSION_START_RES	\
	"(" \
	"s" /* session info */\
	"i" /* result */\
	"i" /* error code */\
	"s" /* error message */\
	")"

/*
 * Provider:base - Session stop callback
**/
#define SERVICE_PROVIDER_BASE_SESSION_STOP		"session_stop"
/* req param len */
#define SERVICE_PROVIDER_BASE_SESSION_STOP_REQ_LEN 1
/* req params */
#define SERVICE_PROVIDER_BASE_SESSION_STOP_REQ	\
	"(" \
	"s" /* session info */\
	")"
/* res param len */
#define SERVICE_PROVIDER_BASE_SESSION_STOP_RES_LEN 3
/* res params */
#define SERVICE_PROVIDER_BASE_SESSION_STOP_RES	\
	"(" \
	"i" /* result */\
	"i" /* error code */\
	"s" /* error message */\
	")"

/*
 * Provider:storage - Open callback
**/
#define SERVICE_PROVIDER_STORAGE_DOWNLOAD			"download"
/* req param len */
#define SERVICE_PROVIDER_STORAGE_DOWNLOAD_REQ_LEN	3
/* req params */
#define SERVICE_PROVIDER_STORAGE_DOWNLOAD_REQ \
	"("\
	"s" /* session uri */\
	"s" /* local path */\
	"s" /* cloud path */\
	")"

/* res param len */
#define SERVICE_PROVIDER_STORAGE_DOWNLOAD_RES_LEN 4
/* res params */
#define SERVICE_PROVIDER_STORAGE_DOWNLOAD_RES \
	"(" \
	"i" /* fd */\
	"i" /* result */\
	"i" /* error code */\
	"s" /* error message */\
	")"

/*
 * Provider:storage - Close callback
**/
#define SERVICE_PROVIDER_STORAGE_CLOSE			"close"
/* req param len */
#define SERVICE_PROVIDER_STORAGE_CLOSE_REQ_LEN	2
/* req params */
#define SERVICE_PROVIDER_STPRAGE_CLOSE_REQ \
	"("\
	"s" /* session info */\
	"i" /* fd */\
	")"
/* res param len */
#define SERVICE_PROVIDER_STORAGE_CLOSE_RES_LEN 3
/* res params */
#define SERVICE_PROVIDER_STORAGE_CLOSE_RES \
	"(" \
	"i" /* result */\
	"i" /* error code */\
	"s" /* error message */\
	")"

/**
 * # adaptor - provider IPC protocol End  }
 * ##################################
 */


typedef struct _ipc_reply_data_s
{
	GDBusMethodInvocation *invocation;
	char *type;
} ipc_reply_data_s;
typedef struct _ipc_reply_data_s *ipc_reply_data_h;

char *ipc_insure_g_variant_dup_string(GVariant *string);
void ipc_insure_g_variant_builder_add_array_string(GVariantBuilder *builder, const char *str);
char *ipc_make_return_type(const char *type);
void ipc_create_error_msg(int code, char **ipc_msg);
void ipc_create_variant_info(GVariant *parameters, int size, GVariant ***var_info);
void ipc_destroy_variant_info(GVariant **var_info, int size);
void ipc_free_reply_data(ipc_reply_data_h reply);

#define SAL_IPC_RETURN_TYPE(type)   "(" type "iis)"
#define SAL_IPC_SIMPLE_TYPE         "(iis)"
#define SAL_IPC_SAFE_STR(str)		((str) ? (str) : "")
#define SAL_IPC_STR(x)				(x==NULL)?"":x

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_CONVERGENCE_SAL_IPC_H__ */
