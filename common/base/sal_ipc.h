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

#define SAL_IPC_STR(x)	(x==NULL)?"":x

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
#define service_adaptor_connect_req_s_type_length 1
#define service_adaptor_connect_req_s_type \
	"(" \
	"s" /* char *uri */ \
	")"

#define service_adaptor_connect_res_s_type_length 1
#define service_adaptor_connect_res_s_type \
	"(" \
	"a(s)" /* char **uri */ \
	")"

#define service_adaptor_disconnect_s_type_length 1
#define service_adaptor_disconnect_s_type \
	"(" \
	"s" /* char *uri */ \
	")"

#define service_plugin_create_s_type_length 1
#define service_plugin_create_s_type \
	"(" \
	"s" /* char *uri */ \
	")"

#define service_plugin_destroy_s_type_length 1
#define service_plugin_destroy_s_type \
	"(" \
	"s" /* char *uri */ \
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
#define RETURN_LENGTH					2

/**
 * DBus APIs
 */
#define DBUS_SERVICE_ADAPTOR				"dbus_00"
#define DBUS_SERVICE_PLUGIN				"dbus_01"
#define DBUS_SERVICE_AUTH				"dbus_02"
#define DBUS_SERVICE_STORAGE				"dbus_03"
#define DBUS_NAME_LENGTH				7

#define DBUS_SERVICE_ADAPTOR_CONNECT_METHOD		DBUS_SERVICE_ADAPTOR "_service_adaptor_connect"
#define DBUS_SERVICE_ADAPTOR_DISCONNECT_METHOD		DBUS_SERVICE_ADAPTOR "_service_adaptor_disconnect"

#define DBUS_SERVICE_PLUGIN_CREATE_METHOD		DBUS_SERVICE_PLUGIN "_service_plugin_create"
#define DBUS_SERVICE_PLUGIN_DESTROY_METHOD		DBUS_SERVICE_PLUGIN "_service_plugin_destroy"

#define DBUS_SERVICE_AUTH_OAUTH1_METHOD			DBUS_SERVICE_AUTH "_service_auth_oauth1"

#define DBUS_SERVICE_STORAGE_CLOUD_FILE_METHOD		DBUS_SERVICE_STORAGE "_service_storage_cloud_file"

#define DBUS_SERVICE_ADAPTOR_NOTIFY_SIGNAL		DBUS_SERVICE_ADAPTOR "_service_adaptor_signal_notify"

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

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_CONVERGENCE_SAL_IPC_H__ */
