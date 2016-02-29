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
 * File: dbus-client-shop.c
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
#include "dbus_client_shop.h"
#include "service_adaptor_client_type.h"
#include "private/service-adaptor-client-shop.h"

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

void __get_item_list_res_type(GVariant *call_result_struct,
						service_adaptor_shop_item_s ***items,
						unsigned int *items_len,
						void **server_data)
{
	GVariant *res_info_struct[private_service_adaptor_shop_res_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_shop_res_s_type_length; j++) {
		res_info_struct[j] = g_variant_get_child_value(call_result_struct, j);
	}

	int idx = 0;
	gsize item_count = g_variant_n_children(res_info_struct[idx]);

	*items = (service_adaptor_shop_item_s **) g_malloc0(sizeof(service_adaptor_shop_item_s *) * item_count);

	for (gsize i = 0; i < item_count; i++) {
		GVariant *item_info_struct[private_service_adaptor_shop_item_s_type_length];
		GVariant *item_info_entry_v = g_variant_get_child_value(res_info_struct[idx], i);
		(*items)[i] = (service_adaptor_shop_item_s *) g_malloc0(sizeof(service_adaptor_shop_item_s));

		for (size_t j = 0; j < private_service_adaptor_shop_item_s_type_length; j++) {
			item_info_struct[j] = g_variant_get_child_value(item_info_entry_v, j);
		}

		int idx2 = 0;
		(*items)[i]->item_id = g_variant_get_int32(item_info_struct[idx2++]);
		(*items)[i]->category_id = g_variant_get_int32(item_info_struct[idx2++]);

		gsize ids_count = g_variant_n_children(item_info_struct[idx2]);
		(*items)[i]->sticker_ids = (long *) g_malloc0(sizeof(long) * ids_count);

		for (gsize k = 0; k < ids_count; k++) {
			GVariant *ids_info_entry_v = g_variant_get_child_value(item_info_struct[idx2], k);
			GVariant *ids_info_struct = g_variant_get_child_value(ids_info_entry_v, 0);;

			(*items)[i]->sticker_ids[k] = g_variant_get_int32(ids_info_struct);

			g_variant_unref(ids_info_struct);
		}
		idx2++;

		(*items)[i]->sticker_ids_len	= g_variant_get_uint32(item_info_struct[idx2++]);
		(*items)[i]->title		= ipc_g_variant_dup_string(item_info_struct[idx2++]);
		(*items)[i]->character		= ipc_g_variant_dup_string(item_info_struct[idx2++]);
		(*items)[i]->version		= g_variant_get_int32(item_info_struct[idx2++]);
		(*items)[i]->download_url	= ipc_g_variant_dup_string(item_info_struct[idx2++]);
		(*items)[i]->panel_url		= ipc_g_variant_dup_string(item_info_struct[idx2++]);
		(*items)[i]->sticker_url	= ipc_g_variant_dup_string(item_info_struct[idx2++]);
		(*items)[i]->file_size		= g_variant_get_int32(item_info_struct[idx2++]);
		(*items)[i]->count		= g_variant_get_int32(item_info_struct[idx2++]);
		(*items)[i]->character_code	= ipc_g_variant_dup_string(item_info_struct[idx2++]);
		(*items)[i]->startdate		= (long long int) g_variant_get_int64(item_info_struct[idx2++]);
		(*items)[i]->enddate		= (long long int) g_variant_get_int64(item_info_struct[idx2++]);
		(*items)[i]->expired_date	= (long long int) g_variant_get_int64(item_info_struct[idx2++]);
		(*items)[i]->valid_period	= (long long int) g_variant_get_int64(item_info_struct[idx2++]);

		for (size_t j = 0; j < private_service_adaptor_shop_item_s_type_length; j++) {
			g_variant_unref(item_info_struct[j]);
		}
	}
	idx++;

	*items_len = g_variant_get_uint32(res_info_struct[idx++]);

	for (size_t j = 0; j < private_service_adaptor_shop_res_s_type_length; j++) {
		g_variant_unref(res_info_struct[j]);
	}
}

GVariant *__create_shop_req_type(const char *service_name,
						service_adaptor_shop_info_s *info,
						void *user_data)
{
	GVariant *request = g_variant_new("(" private_service_adaptor_shop_req_s_type ")", __safe_add_string(service_name), info->category_id, info->item_id, info->sticker_id, __safe_add_string(info->lang_cd), __safe_add_string(info->cntry_cd), info->rwidth, info->rheight, info->start_idx, info->count);

	return request;
}

void __get_shop_res_type(GVariant *call_result_struct,
						service_adaptor_shop_item_s **item,
						void **server_data)
{
	GVariant *res_info_struct[private_service_adaptor_shop_item_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_shop_item_s_type_length; j++) {
		res_info_struct[j] = g_variant_get_child_value(call_result_struct, j);
	}

	int idx = 0;
	*item = (service_adaptor_shop_item_s *) calloc(1, sizeof(service_adaptor_shop_item_s));
	if (NULL != *item) {
		(*item)->item_id = g_variant_get_int32(res_info_struct[idx++]);
		(*item)->category_id = g_variant_get_int32(res_info_struct[idx++]);

		gsize ids_count = g_variant_n_children(res_info_struct[idx]);
		(*item)->sticker_ids = (long *) calloc(ids_count, sizeof(long));

		if (NULL != (*item)->sticker_ids) {
			for (gsize i = 0; i < ids_count; i++) {
				GVariant *ids_info_entry_v = g_variant_get_child_value(res_info_struct[idx], i);
				GVariant *ids_info_struct = g_variant_get_child_value(ids_info_entry_v, 0);;

				(*item)->sticker_ids[i] = g_variant_get_int32(ids_info_struct);

				g_variant_unref(ids_info_struct);
			}
			idx++;
			(*item)->sticker_ids_len = g_variant_get_uint32(res_info_struct[idx++]);
		} else {
			(*item)->sticker_ids_len = 0U;
			idx++;
			idx++;
		}
		(*item)->title = ipc_g_variant_dup_string(res_info_struct[idx++]);
		(*item)->character = ipc_g_variant_dup_string(res_info_struct[idx++]);
		(*item)->version = g_variant_get_int32(res_info_struct[idx++]);
		(*item)->download_url = ipc_g_variant_dup_string(res_info_struct[idx++]);
		(*item)->panel_url = ipc_g_variant_dup_string(res_info_struct[idx++]);
		(*item)->sticker_url = ipc_g_variant_dup_string(res_info_struct[idx++]);
		(*item)->file_size = g_variant_get_int32(res_info_struct[idx++]);
		(*item)->count = g_variant_get_int32(res_info_struct[idx++]);
		(*item)->character_code = ipc_g_variant_dup_string(res_info_struct[idx++]);
		(*item)->startdate = g_variant_get_int64(res_info_struct[idx++]);
		(*item)->enddate = g_variant_get_int64(res_info_struct[idx++]);
		(*item)->expired_date = g_variant_get_int64(res_info_struct[idx++]);
		(*item)->valid_period = g_variant_get_int64(res_info_struct[idx++]);
	}

	for (size_t j = 0; j < private_service_adaptor_shop_item_s_type_length; j++) {
		g_variant_unref(res_info_struct[j]);
	}
}

/******************************************************************************
 * Public interface definition
 ******************************************************************************/

/**	@brief
 *	@return	service_adaptor_error_s
 *	@remarks :
 */
int _dbus_get_item_list(const char *service_name,
						service_adaptor_shop_info_s *info,
						void *user_data,
						service_adaptor_shop_item_s ***items,
						unsigned int *items_len,
						void **server_data,
						service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();

	ipc_check_proxy(sac_interface_proxy);

	GVariant *request = __create_shop_req_type(service_name, info, user_data);

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			PRIVATE_DBUS_GET_ITEM_LIST_METHOD,
			request,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
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
		if (FALSE == g_variant_is_of_type(call_result, G_VARIANT_TYPE(MAKE_RETURN_TYPE(private_service_adaptor_shop_res_s_type)))) {
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
				ret = _get_result_code(remote_call_result);
			} else {
				if ((NULL != items) && (NULL != items_len)) {
					__get_item_list_res_type(call_result_struct[0], items, items_len, server_data);
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

int _dbus_download_item_package(const char *service_name,
						service_adaptor_shop_info_s *info,
						void *user_data,
						service_adaptor_shop_item_s **item,
						void **server_data,
						service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();

	ipc_check_proxy(sac_interface_proxy);

	GVariant *request = __create_shop_req_type(service_name, info, user_data);

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			PRIVATE_DBUS_DOWNLOAD_ITEM_PACKAGE_METHOD,
			request,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
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
		if (FALSE == g_variant_is_of_type(call_result, G_VARIANT_TYPE(MAKE_RETURN_TYPE(private_service_adaptor_shop_item_s_type)))) {
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
				ret = _get_result_code(remote_call_result);
			} else {
				if (NULL != item) {
					__get_shop_res_type(call_result_struct[0], item, server_data);
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

int _dbus_download_sticker(const char *service_name,
						service_adaptor_shop_info_s *info,
						void *user_data,
						service_adaptor_shop_item_s **item,
						void **server_data,
						service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();

	ipc_check_proxy(sac_interface_proxy);

	GVariant *request = __create_shop_req_type(service_name, info, user_data);

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			PRIVATE_DBUS_DOWNLOAD_STICKER_METHOD,
			request,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
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
		if (FALSE == g_variant_is_of_type(call_result, G_VARIANT_TYPE(MAKE_RETURN_TYPE(private_service_adaptor_shop_item_s_type)))) {
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
				ret = _get_result_code(remote_call_result);
			} else {
				if (NULL != item) {
					__get_shop_res_type(call_result_struct[0], item, server_data);
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

int _dbus_get_panel_url(const char *service_name,
						service_adaptor_shop_info_s *info,
						void *user_data,
						service_adaptor_shop_item_s **item,
						void **server_data,
						service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();

	ipc_check_proxy(sac_interface_proxy);

	GVariant *request = __create_shop_req_type(service_name, info, user_data);

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			PRIVATE_DBUS_GET_PANEL_URL_METHOD,
			request,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
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
		if (FALSE == g_variant_is_of_type(call_result, G_VARIANT_TYPE(MAKE_RETURN_TYPE(private_service_adaptor_shop_item_s_type)))) {
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
				ret = _get_result_code(remote_call_result);
			} else {
				if (NULL != item) {
					__get_shop_res_type(call_result_struct[0], item, server_data);
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

