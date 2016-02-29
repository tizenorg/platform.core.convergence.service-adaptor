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
 * File: dbus-client-auth.c
 * Desc:
 * Created on: Oct, 2014
 * Auth: Yongjin Kim <youth.kim@samsung.com>
 *
 *****************************************************************************/

#include <gio/gio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <glib-object.h>

#include <dbus-server.h>

#include "dbus_client.h"
#include "dbus_client_auth.h"
#include "service_adaptor_client_type.h"
#include "service_adaptor_client_log.h"
#include "private/service-adaptor-client-auth.h"

#include "util/service_adaptor_client_util.h"

/******************************************************************************
 * Global variables and defines
 ******************************************************************************/

/******************************************************************************
 * Private interface
 ******************************************************************************/

/******************************************************************************
 * Private interface definition
 ******************************************************************************/

/******************************************************************************
 * Public interface definition
 ******************************************************************************/

/**	@brief
 *	@return	service_adaptor_error_s
 *	@remarks :
 */
int _dbus_get_auth_plugin_list(GList **plugin_list,
						const char *imsi,
						service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();

	ipc_check_proxy(sac_interface_proxy);

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			PRIVATE_DBUS_GET_AUTH_PLUGIN_LIST_METHOD,
			g_variant_new("(s)",
					imsi),
			G_DBUS_CALL_FLAGS_NONE,
			-1,
			NULL,
			&g_error);

	if (NULL == call_result) {
		error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;

		if (NULL != g_error) {
			error->msg = __SAFE_STRDUP(g_error->message);
			g_error_free(g_error);
		}
	} else {
		if (FALSE == g_variant_is_of_type(call_result, G_VARIANT_TYPE(MAKE_RETURN_TYPE(private_plugin_list_type)))) {
			error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->msg = strdup("D-Bus return type error");
			ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		} else {
			GVariant *call_result_struct[3];
			call_result_struct[0] = g_variant_get_child_value(call_result, 0);
			call_result_struct[1] = g_variant_get_child_value(call_result, 1);
			call_result_struct[2] = g_variant_get_child_value(call_result, 2);

			uint64_t remote_call_result = g_variant_get_uint64(call_result_struct[1]);

			if (SERVICE_ADAPTOR_ERROR_NONE != remote_call_result) {
				error->code = remote_call_result;
				error->msg = ipc_g_variant_dup_string(call_result_struct[2]);
				ret = SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED;
			} else {
				gsize list_count = g_variant_n_children(call_result_struct[0]);

				for (gsize i = 0; i < list_count; i++) {
					GVariant *plugin_info_struct[private_service_adaptor_plugin_s_type_length];
					GVariant *plugin_info_entry_v = g_variant_get_child_value(call_result_struct[0], i);
					service_adaptor_plugin_s *info = (service_adaptor_plugin_s *) g_malloc0(sizeof(service_adaptor_plugin_s));

					for (size_t j = 0; j < private_service_adaptor_plugin_s_type_length; j++) {
						plugin_info_struct[j] = g_variant_get_child_value(plugin_info_entry_v, j);
					}

					int idx = 0;
					info->name = ipc_g_variant_dup_string(plugin_info_struct[idx++]);
					info->login = g_variant_get_boolean(plugin_info_struct[idx++]);

					for (size_t j = 0; j < private_service_adaptor_plugin_s_type_length; j++) {
						g_variant_unref(plugin_info_struct[j]);
					}

					*plugin_list = g_list_append(*plugin_list, info);
				}
			}

			g_variant_unref(call_result_struct[0]);
			g_variant_unref(call_result_struct[1]);
			g_variant_unref(call_result_struct[2]);
		}

		g_variant_unref(call_result);
	}

	return ret;
}

/**	@brief
 *	@return	service_adaptor_error_s
 *	@remarks :
 */
int _dbus_set_auth(const char *service_name,
						const char *imsi,
						const char *plugin_name,
						const char *app_id,
						const char *app_secret,
						unsigned int service_id,
						void *user_data,
						service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();

	ipc_check_proxy(sac_interface_proxy);

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			PRIVATE_DBUS_SET_AUTH_METHOD,
			g_variant_new("(" private_service_adaptor_set_auth_s_type ")",
					service_name, imsi, plugin_name, app_id, app_secret, "", "", service_id),
			G_DBUS_CALL_FLAGS_NONE,
			-1,
			NULL,
			&g_error);

	if (NULL == call_result) {
		error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;

		if (NULL != g_error) {
			sac_error("G_IO_ERROR DEBUG (%d)", (int)(g_error->code));
			if (g_error->code == G_IO_ERROR_TIMED_OUT) {
				error->code = SERVICE_ADAPTOR_ERROR_NETWORK;
				ret = SERVICE_ADAPTOR_ERROR_NETWORK;
			}
			error->msg = __SAFE_STRDUP(g_error->message);
			g_error_free(g_error);
		}
	} else {
		if (FALSE == g_variant_is_of_type(call_result, G_VARIANT_TYPE("(ts)"))) {
			error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->msg = strdup("D-Bus return type error");
			ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		} else {
			GVariant *call_result_struct[2];
			call_result_struct[0] = g_variant_get_child_value(call_result, 0);
			call_result_struct[1] = g_variant_get_child_value(call_result, 1);

			uint64_t remote_call_result = g_variant_get_uint64(call_result_struct[0]);

			if (SERVICE_ADAPTOR_ERROR_NONE != remote_call_result) {
				error->code = remote_call_result;
				error->msg = ipc_g_variant_dup_string(call_result_struct[1]);
				ret = _get_result_code(remote_call_result);
			}

			g_variant_unref(call_result_struct[0]);
			g_variant_unref(call_result_struct[1]);
		}

		g_variant_unref(call_result);
	}

	return ret;
}

