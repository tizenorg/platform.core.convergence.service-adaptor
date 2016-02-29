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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <gio/gio.h>

#include "service-adaptor.h"
#include "service-adaptor-shop.h"
#include "service-adaptor-type.h"
#include "service-adaptor-log.h"
#include "dbus-shop-adaptor.h"
#include "dbus-server.h"
#include "dbus-util.h"

void __destroy_shop_info_s(shop_adaptor_shop_info_s *info)
{
	if (NULL == info) {
		return;
	}

	free(info->lang_cd);
	free(info->cntry_cd);
}

void __destroy_shop_item_s(shop_adaptor_shop_item_s *item)
{
	if (NULL == item) {
		return;
	}

	free(item->sticker_ids);
	free(item->title);
	free(item->character);
	free(item->download_url);
	free(item->panel_url);
	free(item->sticker_url);
	free(item->character_code);
}


void __get_shop_req_type(GVariant *parameters,
						char **service_name,
						shop_adaptor_shop_info_s *info)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *req_struct[private_service_adaptor_shop_req_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_shop_req_s_type_length; j++) {
		req_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*service_name = ipc_g_variant_dup_string(req_struct[idx++]);

	GVariant *req_info_struct[private_service_adaptor_shop_info_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_shop_info_s_type_length; j++) {
		req_info_struct[j] = g_variant_get_child_value(req_struct[idx], j);
	}

	int idx2 = 0;
	info->category_id = g_variant_get_int32(req_info_struct[idx2++]);
	info->item_id = g_variant_get_int32(req_info_struct[idx2++]);
	info->sticker_id = g_variant_get_int32(req_info_struct[idx2++]);
	info->lang_cd = ipc_g_variant_dup_string(req_info_struct[idx2++]);
	info->cntry_cd = ipc_g_variant_dup_string(req_info_struct[idx2++]);
	info->rwidth = g_variant_get_int32(req_info_struct[idx2++]);
	info->rheight = g_variant_get_int32(req_info_struct[idx2++]);
	info->start_idx = g_variant_get_int32(req_info_struct[idx2++]);
	info->count = g_variant_get_int32(req_info_struct[idx2++]);

	for (size_t j = 0; j < private_service_adaptor_shop_info_s_type_length; j++) {
		g_variant_unref(req_info_struct[j]);
	}

	for (size_t j = 0; j < private_service_adaptor_shop_req_s_type_length; j++) {
		g_variant_unref(req_struct[j]);
	}
}

GVariant *__create_shop_item_res_type(shop_adaptor_shop_item_s *item,
						shop_adaptor_error_code_t *error_code)
{
	shop_adaptor_shop_item_s _item;
	_item.sticker_ids_len = 0;
	_item.item_id = 0;
	_item.category_id = 0;
	_item.title = "";
	_item.character = "";
	_item.version = 0;
	_item.download_url = "";
	_item.panel_url = "";
	_item.sticker_url = "";
	_item.file_size = 0;
	_item.count = 0;
	_item.character_code = "";
	_item.startdate = 0;
	_item.enddate = 0;
	_item.expired_date = 0;
	_item.valid_period = 0;

	if (NULL == item) {
		item = &_item;
	}

	GVariantBuilder *builder = g_variant_builder_new(G_VARIANT_TYPE("a(i)"));

	for (gsize j = 0; j < item->sticker_ids_len; j++) {
		g_variant_builder_add(builder, "(i)", item->sticker_ids[j]);
	}

	GVariant *response = g_variant_new(MAKE_RETURN_TYPE(private_service_adaptor_shop_item_s_type),
			item->item_id, item->category_id, builder, item->sticker_ids_len,
			__safe_add_string(item->title), __safe_add_string(item->character),
			item->version, __safe_add_string(item->download_url), __safe_add_string(item->panel_url),
			__safe_add_string(item->sticker_url), item->file_size, item->count,
			__safe_add_string(item->character_code), item->startdate, item->enddate,
			item->expired_date, item->valid_period,
			(uint64_t) error_code->code, __safe_add_string(error_code->msg));

	g_variant_builder_unref(builder);

	return response;
}

GVariant *__create_shop_item_list_res_type(shop_adaptor_shop_item_s **items,
						unsigned int items_len,
						shop_adaptor_error_code_t *error_code)
{
	if (NULL == items) {
		items_len = 0;
	}

	GVariantBuilder *builder = g_variant_builder_new(G_VARIANT_TYPE(private_shop_item_res_list_type));
	GVariantBuilder *builder_in;

	for (gsize i = 0; i < items_len; i++) {
		g_variant_builder_open(builder, G_VARIANT_TYPE(private_service_adaptor_shop_item_s_type));
		g_variant_builder_add(builder, "i", (int) items[i]->item_id);
		g_variant_builder_add(builder, "i", items[i]->category_id);

		/* sticker_ids */
		builder_in = g_variant_builder_new(G_VARIANT_TYPE("a(i)"));

		for (gsize j = 0; j < items[i]->sticker_ids_len; j++) {
			g_variant_builder_add(builder_in, "(i)", (int) items[i]->sticker_ids[j]);
		}

		g_variant_builder_add(builder, "a(i)", builder_in);
		g_variant_builder_unref(builder_in);

		g_variant_builder_add(builder, "u", items[i]->sticker_ids_len);
		safe_g_variant_builder_add_string(builder, items[i]->title);
		safe_g_variant_builder_add_string(builder, items[i]->character);
		g_variant_builder_add(builder, "i", items[i]->version);
		safe_g_variant_builder_add_string(builder, items[i]->download_url);
		safe_g_variant_builder_add_string(builder, items[i]->panel_url);
		safe_g_variant_builder_add_string(builder, items[i]->sticker_url);
		g_variant_builder_add(builder, "i", (int) items[i]->file_size);
		g_variant_builder_add(builder, "i", items[i]->count);
		safe_g_variant_builder_add_string(builder, items[i]->character_code);
		g_variant_builder_add(builder, "x", items[i]->startdate);
		g_variant_builder_add(builder, "x", items[i]->enddate);
		g_variant_builder_add(builder, "x", items[i]->expired_date);
		g_variant_builder_add(builder, "x", items[i]->valid_period);
		g_variant_builder_close(builder);
	}

	GVariant *response = g_variant_new(MAKE_RETURN_TYPE(private_service_adaptor_shop_res_s_type),
			builder, items_len, (uint64_t) error_code->code, __safe_add_string(error_code->msg));

	g_variant_builder_unref(builder);

	return response;
}

void shop_adaptor_method_call(GDBusConnection *connection,
						const gchar *sender,
						const gchar *object_path,
						const gchar *interface_name,
						const gchar *method_name,
						GVariant *parameters,
						GDBusMethodInvocation *invocation,
						gpointer user_data)
{
	service_adaptor_internal_error_code_e ret_code = SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;

	if (0 == g_strcmp0(method_name, PRIVATE_DBUS_GET_ITEM_LIST_METHOD)) {
		char *service_name = NULL;
		shop_adaptor_shop_info_s info;
		shop_adaptor_shop_item_s **items = NULL;
		unsigned int items_len = 0;
		void *user_data = NULL;
		void *server_data = NULL;
		shop_adaptor_error_code_t *error_code = NULL;
		shop_adaptor_error_code_t _error;
		_error.code = SHOP_ADAPTOR_ERROR_NONE;
		_error.msg = NULL;

		__get_shop_req_type(parameters, &service_name, &info);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = SHOP_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = "Can not get service context";

			GVariant *response = __create_shop_item_list_res_type(items, items_len, error_code);
			g_dbus_method_invocation_return_value(invocation, response);

			free(service_name);
			__destroy_shop_info_s(&info);
			return;
		}

		shop_adaptor_h adaptor = service_adaptor_get_shop_adaptor(service_adaptor);
		shop_adaptor_plugin_h plugin = NULL;

		if (NULL != service->shop_context) {
			plugin = shop_adaptor_get_plugin_by_name(adaptor, service->shop_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			ret_code = shop_adaptor_get_item_list_v1(plugin, service->shop_context,
					&info, user_data, &items, &items_len, &error_code, &server_data);

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->shop_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%lld: %s)", error_code ? error_code->code : 0ULL, error_code ? error_code->msg : NULL);
				shop_adaptor_destroy_error_code(&error_code);

				service_adaptor_debug("Re-try API");
				ret_code = shop_adaptor_get_item_list_v1(plugin, service->shop_context,
						&info, user_data, &items, &items_len, &error_code, &server_data);
			}

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NONE != ret_code) {
				service_adaptor_error("Can not run shop_adaptor_get_item_list_v1()");
			}
		}

		if (NULL == error_code) {
			error_code = &_error;
			error_code->code = CONTACT_ADAPTOR_ERROR_NONE;
			error_code->msg = strdup("");
		}

		GVariant *response = __create_shop_item_list_res_type(items, items_len, error_code);
		g_dbus_method_invocation_return_value(invocation, response);

		if (error_code != &_error) {
			free(error_code->msg);
			free(error_code);
			error_code = NULL;
		} else {
			free(_error.msg);
		}
		free(service_name);
		__destroy_shop_info_s(&info);
		if ((NULL != items) && (0U < items_len)) {
			for (int i = 0; i < items_len; i++) {
				__destroy_shop_item_s(items[i]);
				free(items[i]);
			}
			free(items);
		}
	} else if (0 == g_strcmp0(method_name, PRIVATE_DBUS_DOWNLOAD_ITEM_PACKAGE_METHOD)) {
		char *service_name = NULL;
		shop_adaptor_shop_info_s info;
		shop_adaptor_shop_item_s *item = NULL;
		shop_adaptor_error_code_t *error_code = NULL;
		shop_adaptor_error_code_t _error;
		_error.code = SHOP_ADAPTOR_ERROR_NONE;
		_error.msg = NULL;

		__get_shop_req_type(parameters, &service_name, &info);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = SHOP_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = "Can not get service context";

			GVariant *response = __create_shop_item_res_type(item, error_code);
			g_dbus_method_invocation_return_value(invocation, response);

			free(service_name);
			__destroy_shop_info_s(&info);
			return;
		}

		shop_adaptor_h adaptor = service_adaptor_get_shop_adaptor(service_adaptor);
		shop_adaptor_plugin_h plugin = NULL;

		if (NULL != service->shop_context) {
			plugin = shop_adaptor_get_plugin_by_name(adaptor, service->shop_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			ret_code = shop_adaptor_download_item_package_v1(plugin, service->shop_context,
					&info, NULL, &item, &error_code, NULL);

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->shop_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%lld: %s)", error_code ? error_code->code : 0ULL, error_code ? error_code->msg : NULL);
				shop_adaptor_destroy_error_code(&error_code);

				service_adaptor_debug("Re-try API");
				ret_code = shop_adaptor_download_item_package_v1(plugin, service->shop_context,
						&info, NULL, &item, &error_code, NULL);
			}
		}

		if (NULL == error_code) {
			error_code = &_error;
			error_code->code = CONTACT_ADAPTOR_ERROR_NONE;
			error_code->msg = strdup("");
		}

		GVariant *response = __create_shop_item_res_type(item, error_code);
		g_dbus_method_invocation_return_value(invocation, response);

		if (error_code != &_error) {
			free(error_code->msg);
			free(error_code);
			error_code = NULL;
		} else {
			free(_error.msg);
		}
		free(service_name);
		__destroy_shop_info_s(&info);
		__destroy_shop_item_s(item);
		free(item);
	} else if (0 == g_strcmp0(method_name, PRIVATE_DBUS_DOWNLOAD_STICKER_METHOD)) {
		char *service_name = NULL;
		shop_adaptor_shop_info_s info;
		shop_adaptor_shop_item_s *item = NULL;
		shop_adaptor_error_code_t *error_code = NULL;
		shop_adaptor_error_code_t _error;
		_error.code = SHOP_ADAPTOR_ERROR_NONE;
		_error.msg = NULL;

		__get_shop_req_type(parameters, &service_name, &info);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = SHOP_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = "Can not get service context";

			GVariant *response = __create_shop_item_res_type(item, error_code);
			g_dbus_method_invocation_return_value(invocation, response);

			free(service_name);
			__destroy_shop_info_s(&info);
			return;
		}

		shop_adaptor_h adaptor = service_adaptor_get_shop_adaptor(service_adaptor);
		shop_adaptor_plugin_h plugin = NULL;

		if (NULL != service->shop_context) {
			plugin = shop_adaptor_get_plugin_by_name(adaptor, service->shop_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			ret_code = shop_adaptor_download_sticker_v1(plugin, service->shop_context,
					&info, NULL, &item, &error_code, NULL);

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->shop_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%lld: %s)", error_code ? error_code->code : 0ULL, error_code ? error_code->msg : NULL);
				shop_adaptor_destroy_error_code(&error_code);

				service_adaptor_debug("Re-try API");
				ret_code = shop_adaptor_download_sticker_v1(plugin, service->shop_context,
						&info, NULL, &item, &error_code, NULL);
			}
		}

		if (NULL == error_code) {
			error_code = &_error;
			error_code->code = CONTACT_ADAPTOR_ERROR_NONE;
			error_code->msg = strdup("");
		}

		GVariant *response = __create_shop_item_res_type(item, error_code);
		g_dbus_method_invocation_return_value(invocation, response);

		if (error_code != &_error) {
			free(error_code->msg);
			free(error_code);
			error_code = NULL;
		} else {
			free(_error.msg);
		}
		free(service_name);
		__destroy_shop_info_s(&info);
		__destroy_shop_item_s(item);
		free(item);
	} else if (0 == g_strcmp0(method_name, PRIVATE_DBUS_GET_PANEL_URL_METHOD)) {
		char *service_name = NULL;
		shop_adaptor_shop_info_s info;
		shop_adaptor_shop_item_s *item = NULL;
		shop_adaptor_error_code_t *error_code = NULL;
		shop_adaptor_error_code_t _error;
		_error.code = SHOP_ADAPTOR_ERROR_NONE;
		_error.msg = NULL;

		__get_shop_req_type(parameters, &service_name, &info);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = SHOP_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = "Can not get service context";

			GVariant *response = __create_shop_item_res_type(item, error_code);
			g_dbus_method_invocation_return_value(invocation, response);

			free(service_name);
			__destroy_shop_info_s(&info);
			return;
		}

		shop_adaptor_h adaptor = service_adaptor_get_shop_adaptor(service_adaptor);
		shop_adaptor_plugin_h plugin = NULL;

		if (NULL != service->shop_context) {
			plugin = shop_adaptor_get_plugin_by_name(adaptor, service->shop_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			ret_code = shop_adaptor_get_panel_url_v1(plugin, service->shop_context,
					&info, NULL, &item, &error_code, NULL);

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->shop_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%lld: %s)", error_code ? error_code->code : 0ULL, error_code ? error_code->msg : NULL);
				shop_adaptor_destroy_error_code(&error_code);

				service_adaptor_debug("Re-try API");
				ret_code = shop_adaptor_get_panel_url_v1(plugin, service->shop_context,
						&info, NULL, &item, &error_code, NULL);
			}
		}

		if (NULL == error_code) {
			error_code = &_error;
			error_code->code = CONTACT_ADAPTOR_ERROR_NONE;
			error_code->msg = strdup("");
		}

		GVariant *response = __create_shop_item_res_type(item, error_code);
		g_dbus_method_invocation_return_value(invocation, response);

		if (error_code != &_error) {
			free(error_code->msg);
			free(error_code);
			error_code = NULL;
		} else {
			free(_error.msg);
		}
		free(service_name);
		__destroy_shop_info_s(&info);
		__destroy_shop_item_s(item);
		free(item);
	}
}
