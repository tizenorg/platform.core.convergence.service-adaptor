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
#include <unistd.h>

#include "service-adaptor.h"
#include "service-adaptor-auth.h"
#include "service-adaptor-push.h"
#include "service-adaptor-storage.h"
#include "service-adaptor-message.h"
#include "service-adaptor-type.h"
#include "service-adaptor-log.h"
#include "dbus-auth-adaptor.h"
#include "dbus-server.h"
#include "dbus-util.h"
#include "util/client_checker.h"

#include <bundle.h>

#define AUTH_FLAG	(0x01 << 0)
#define STORAGE_FLAG	(0x01 << 1)
#define RET_MSG_LEN	2048
#define __SAFE_STRDUP(x)	(x) ? strdup(x) : strdup("")
#define __init_context_info_s(x)	do { \
						(x).user_id = NULL; \
						(x).app_id = NULL; \
						(x).service_id = 0U; \
						(x).imsi = NULL; \
						(x).duid = NULL; \
						(x).msisdn = NULL; \
						(x).access_token = NULL; \
						(x).refresh_token = NULL; \
						(x).property = NULL; \
					} while (0)

#define _PLUGIN_PROPERTY_KEY_BASE	"http://tizen.org/service-adaptor/plugin/property/"

void auth_adaptor_method_call(GDBusConnection *connection,
						const gchar *sender,
						const gchar *object_path,
						const gchar *interface_name,
						const gchar *method_name,
						GVariant *parameters,
						GDBusMethodInvocation *invocation,
						gpointer user_data)
{
FUNC_START();
	service_adaptor_internal_error_code_e ret_code = SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
	char ret_msg[RET_MSG_LEN] = {0, };

/************************************************************************
 *
 *                        private feature
 */

	if (0 == g_strcmp0(method_name, PRIVATE_DBUS_GET_AUTH_PLUGIN_LIST_METHOD)) {
		service_adaptor_debug_func("Auth API : get_auth_plugin_list Called");
		GList *plugin_list = NULL;
		GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
		char *imsi = ipc_g_variant_dup_string(in_parameters);

		GVariantBuilder *builder = g_variant_builder_new(G_VARIANT_TYPE(private_plugin_list_type));

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		auth_adaptor_h adaptor = service_adaptor_get_auth_adaptor(service_adaptor);

FUNC_STEP();
		if (NULL == adaptor) {
			ret_code = SERVICE_ADAPTOR_INTERNAL_ERROR_INIT;
			snprintf(ret_msg, RET_MSG_LEN, "Not Initialized");

			GVariant *response = g_variant_new(MAKE_RETURN_TYPE(private_plugin_list_type), builder, (uint64_t) ret_code, ret_msg);
			g_dbus_method_invocation_return_value(invocation, response);

			g_variant_builder_unref(builder);
			free(imsi);
			return;
		}

		service_adaptor_debug_func("get auth plugins");
		plugin_list = auth_adaptor_get_plugins(adaptor);

FUNC_STEP();
		for (GList *list = g_list_first(plugin_list); list != NULL; list = g_list_next(list)) {
			auth_adaptor_plugin_h auth_plugin = (auth_adaptor_plugin_h) list->data;

			bool is_login = false;

			auth_adaptor_plugin_context_h auth_context =
					auth_adaptor_create_plugin_context(auth_plugin, "", "", "", "", imsi, "is_auth_checker");

FUNC_STEP();
			if (NULL == auth_context) {
				service_adaptor_error("Could not create auth plugin context");
				ret_code = SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_FOUND;
				snprintf(ret_msg, RET_MSG_LEN, "Could not create auth plugin context");
				continue;
			}

			int is_auth = -1;

			auth_adaptor_error_code_h error_code = NULL;
			service_adaptor_debug_func("call is_auth");
			ret_code = auth_adaptor_is_auth(auth_plugin, auth_context, NULL, &is_auth, &error_code, NULL);
			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NONE != ret_code) {
				ret_code = SERVICE_ADAPTOR_INTERNAL_ERROR_CREATE;
				service_adaptor_error("Is_auth Function Internal error : %d", ret_code);

				if ((NULL != error_code) && (NULL != error_code->msg)) {
					service_adaptor_error("[%lld] %s", error_code->code, error_code->msg);
				}
			}
			auth_adaptor_destroy_error_code(&error_code);
			auth_adaptor_destroy_plugin_context(auth_plugin, auth_context);
			auth_context = NULL;

			service_adaptor_debug_func("auth_plugins : is_auth(%d)", is_auth);
			if (1 == is_auth) {
				is_login = true;
			}

/*			int plugin_count = 1; */
/*			char *plugin_uri = NULL; */

/*			auth_adaptor_get_plugin_uri(auth_plugin, &plugin_uri); */

			service_adaptor_debug_func("wrapping auth_plugins");
			/*For product */

			g_variant_builder_open(builder, G_VARIANT_TYPE(private_service_adaptor_plugin_s_type));
			safe_g_variant_builder_add_string(builder, "CONTACT");
			g_variant_builder_add(builder, "b", is_login);
			g_variant_builder_close(builder);

			g_variant_builder_open(builder, G_VARIANT_TYPE(private_service_adaptor_plugin_s_type));
			safe_g_variant_builder_add_string(builder, "MESSAGE");
			g_variant_builder_add(builder, "b", is_login);
			g_variant_builder_close(builder);

			g_variant_builder_open(builder, G_VARIANT_TYPE(private_service_adaptor_plugin_s_type));
			safe_g_variant_builder_add_string(builder, "SHOP");
			g_variant_builder_add(builder, "b", is_login);
			g_variant_builder_close(builder);

			g_variant_builder_open(builder, G_VARIANT_TYPE(private_service_adaptor_plugin_s_type));
			safe_g_variant_builder_add_string(builder, "SPP");
			g_variant_builder_add(builder, "b", is_login);
			g_variant_builder_close(builder);

			g_variant_builder_open(builder, G_VARIANT_TYPE(private_service_adaptor_plugin_s_type));
			safe_g_variant_builder_add_string(builder, "ORS");
			g_variant_builder_add(builder, "b", is_login);
			g_variant_builder_close(builder);


			/*
			for (int i = 0; i < plugin_count; i++) {
				plugin->name = plugin_uri;

				g_variant_builder_open(builder, G_VARIANT_TYPE(private_service_adaptor_plugin_s_type));

				safe_g_variant_builder_add_string(builder, plugin->name);
				g_variant_builder_add(builder, "b", plugin->login);

				g_variant_builder_close(builder);
			}
			*/

			ret_code = SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
		}

		if (NULL == plugin_list) {
			FUNC_STEP();
			ret_code = SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_FOUND;
			snprintf(ret_msg, RET_MSG_LEN, "No Plugin");
		}

		GVariant *response = g_variant_new(MAKE_RETURN_TYPE(private_plugin_list_type), builder, (uint64_t) ret_code, ret_msg);
		g_dbus_method_invocation_return_value(invocation, response);

		g_variant_builder_unref(builder);
		free(imsi);
		g_list_free(plugin_list);
	} else if (0 == g_strcmp0(method_name, PRIVATE_DBUS_SET_AUTH_METHOD)) {
		SERVICE_ADAPTOR_API_TIME_CHECK_START();
		service_adaptor_debug_func("Auth API : set_auth Called");

		GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
		GVariant *req_struct[private_service_adaptor_set_auth_s_type_length];

		for (size_t j = 0; j < private_service_adaptor_set_auth_s_type_length; j++) {
			req_struct[j] = g_variant_get_child_value(in_parameters, j);
		}

		int idx = 0;
		char *service_name = ipc_g_variant_dup_string(req_struct[idx++]);
		char *imsi = ipc_g_variant_dup_string(req_struct[idx++]);
		char *cluster_name = ipc_g_variant_dup_string(req_struct[idx++]);
		char *app_id = g_variant_dup_string(req_struct[idx++], NULL);
		char *app_secret = g_variant_dup_string(req_struct[idx++], NULL);
		char *user_id = g_variant_dup_string(req_struct[idx++], NULL);
		char *user_password = g_variant_dup_string(req_struct[idx++], NULL);
		unsigned int service_id = g_variant_get_uint32(req_struct[idx++]);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();

		service_adaptor_debug("Disconnects service [%s] if same context is already binded", service_name);
		service_adaptor_disconnect(service_adaptor, service_name);

		/* Private (Coreapps) specfic */
		bool auth = true, storage = false, contact = false, message = false, push = false, shop = false;
		if (NULL != cluster_name) {
			if (0 == strncmp(cluster_name, "CONTACT", strlen("CONTACT"))) {
				auth = true;
				contact = true;
				storage = true;
				push = true;
			}
			if (0 == strncmp(cluster_name, "MESSAGE", strlen("MESSAGE"))) {
				auth = true;
				message = true;
				storage = true;
				push = true;
			}
			if (0 == strncmp(cluster_name, "ORS", strlen("ORS"))) {
				auth = true;
				push = true;
			}
			if (0 == strncmp(cluster_name, "SHOP", strlen("SHOP"))) {
				auth = true;
				shop = true;
			}
		}

		service_adaptor_debug_func("[For CoreApps service] create SPP context");
		char push_service_name[1024] = {0, };
		snprintf(push_service_name, 1024, "preloaded_service/plugin='%s'&app_id='%s'",
				"com.samsung.coreapps", "82e51a05286942d1");

		service_adaptor_service_context_h service_spp = NULL;
		if (push) {
			service_spp = service_adaptor_get_service_context(service_adaptor, push_service_name);

			if (NULL == service_spp) {
				service_adaptor_debug("[For CoreApps service] Connect push service");
				/* default service (push: spp) */
				service_adaptor_context_info_s spp_context_info;
				__init_context_info_s(spp_context_info);

				spp_context_info.user_id = "";
				spp_context_info.app_id = "82e51a05286942d1";
				spp_context_info.imsi = imsi ? imsi : "";
				spp_context_info.service_id = service_id;

				ret_code = service_adaptor_connect(service_adaptor, &spp_context_info, push_service_name,
						"com.samsung.coreapps", "", "",
						false, false, false, false, true, false,
						ret_msg);

				if (SERVICE_ADAPTOR_INTERNAL_ERROR_NONE != ret_code) {
					FUNC_STEP();
					service_adaptor_warning("Cannot Connect %s Plugin", cluster_name);
				}
			} else {
				service_adaptor_debug_func("[For CoreApps service] get Lagecy SPP context (try reconnect)");
				ret_code = service_adaptor_reconnect_push_plugin(service_adaptor, service_spp);
				service_adaptor_debug_func("re-connect ret (%d)", ret_code);
			}
		}

		/* default service (core apps) */
		service_adaptor_debug_func("[For CoreApps service] create CoreApps context");
		service_adaptor_context_info_s orca_context_info;
		__init_context_info_s(orca_context_info);

		orca_context_info.app_id = app_id;
		orca_context_info.user_id = user_id;
		orca_context_info.imsi = imsi ? imsi : "";
		orca_context_info.service_id = service_id;

		ret_code = service_adaptor_connect(service_adaptor, &orca_context_info, service_name,
				"com.samsung.coreapps", user_password, app_secret,
				auth, storage, contact, message, false, shop,
				ret_msg);


		if (SERVICE_ADAPTOR_INTERNAL_ERROR_NONE != ret_code) {
			if (0 >= strlen(ret_msg)) {
				snprintf(ret_msg, RET_MSG_LEN, "Cannot Connect Plugin <Cluster : %s> (Please see plugin's Dlog)", cluster_name);
			}
			service_adaptor_error("set_auth failed : %d <service_name : %s>", (int)ret_code, service_name);
		} else {
			service_adaptor_info("set_auth success <service_name : %s>", service_name);
		}

		service_spp = service_adaptor_get_service_context(service_adaptor, push_service_name);

		service_adaptor_debug_func("[For CoreApps service] bind push context");
		service_adaptor_service_context_h service_orca =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if ((NULL != service_spp) && (NULL != service_orca)) {
			FUNC_STEP();
			service_adaptor_bind_push_context(service_adaptor, service_spp, service_orca);
		}

		for (size_t j = 0; j < private_service_adaptor_set_auth_s_type_length; j++) {
			g_variant_unref(req_struct[j]);
		}

		g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)", (uint64_t) ret_code, ret_msg));
		SERVICE_ADAPTOR_API_TIME_CHECK_PAUSE();
		SERVICE_ADAPTOR_API_TIME_CHECK_TOTAL_REPORT(service_name);

		free(service_name);
		free(imsi);
		free(cluster_name);
		free(app_id);
		free(app_secret);
		free(user_id);
		free(user_password);
	} else if (0 == g_strcmp0(method_name, DBUS_GET_AUTH_PLUGIN_LIST_METHOD)) {
/*
 *                       private feature
 *
 ***********************************************************************/

/************************************************************************
 *
 *                        public feature
 */
		service_adaptor_debug("[START] Get Auth Plugin List");

		GList *auth_plugin_list = NULL;
		GList *storage_plugin_list = NULL;
		GHashTable *plugin_list = g_hash_table_new_full(g_str_hash, g_str_equal, free, NULL);
		service_adaptor_debug_func("hash table init (for sorting plugin list)");

		GVariantBuilder *builder = g_variant_builder_new(G_VARIANT_TYPE(plugin_list_type));

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		auth_adaptor_h auth_adaptor = service_adaptor_get_auth_adaptor(service_adaptor);
		storage_adaptor_h storage_adaptor = service_adaptor_get_storage_adaptor(service_adaptor);

		service_adaptor_debug_func("adaptors get. service_adaptor(%p) auth_adaptor(%p) storage_adaptor(%p)",
				service_adaptor, auth_adaptor, storage_adaptor);

		if ((NULL == auth_adaptor) && (NULL == storage_adaptor)) {
			ret_code = SERVICE_ADAPTOR_INTERNAL_ERROR_INIT;
			snprintf(ret_msg, RET_MSG_LEN, "Not Initialized");
			service_adaptor_warning("Could not get adaptors");

			GVariant *response = g_variant_new(MAKE_RETURN_TYPE(plugin_list_type), builder, (uint64_t) ret_code, ret_msg);
			g_dbus_method_invocation_return_value(invocation, response);

			g_variant_builder_unref(builder);
			return;
		}

		int installed_flag = 0;

		auth_plugin_list = auth_adaptor_get_plugins(auth_adaptor);
		for (GList *list = g_list_first(auth_plugin_list); list != NULL; list = g_list_next(list)) {
			auth_adaptor_plugin_h auth_plugin = (auth_adaptor_plugin_h) list->data;
			char *uri = NULL;
			auth_adaptor_get_plugin_uri(auth_plugin, &uri);
			service_adaptor_debug_func("auth plugin uri(%s)", uri);
			if (uri) {
				installed_flag = (int)g_hash_table_lookup(plugin_list, (gconstpointer)(uri));
				g_hash_table_insert(plugin_list, (void *)(uri), (void *)(installed_flag | AUTH_FLAG));
			} else {
				service_adaptor_error("Auth plugin URI is NULL");
			}
		}

		storage_plugin_list = storage_adaptor_get_plugins(storage_adaptor);
		for (GList *list = g_list_first(storage_plugin_list); list != NULL; list = g_list_next(list)) {
			storage_adaptor_plugin_h storage_plugin = (storage_adaptor_plugin_h) list->data;
			char *uri = NULL;
			storage_adaptor_get_plugin_uri(storage_plugin, &uri);
			service_adaptor_debug_func("storage plugin uri(%s)", uri);
			if (uri) {
				installed_flag = (int)g_hash_table_lookup(plugin_list, (gconstpointer)(uri));
				g_hash_table_insert(plugin_list, (gpointer)(uri), (gpointer)(installed_flag | STORAGE_FLAG));
			} else {
				service_adaptor_error("Storage plugin URI is NULL");
			}
		}

		g_list_free(auth_plugin_list);
		auth_plugin_list = NULL;
		g_list_free(storage_plugin_list);
		storage_plugin_list = NULL;

		int plugin_uri_count = 0;
		GHashTableIter iter;
		gpointer key;
		gpointer value;
		g_hash_table_iter_init(&iter, plugin_list);
		while (g_hash_table_iter_next(&iter, &key, &value)) {
			service_adaptor_debug_func("total plugin uri(%s) mask(%d)", (char *)key, (int)value);
			g_variant_builder_open(builder, G_VARIANT_TYPE(service_adaptor_plugin_s_type));

			safe_g_variant_builder_add_string(builder, (char *)key);
			g_variant_builder_add(builder, "i", (int)value);

			g_variant_builder_close(builder);
			plugin_uri_count++;
		}

		if (0 == plugin_uri_count) {
			service_adaptor_debug_func("total plugin count (0)");
/*			ret_code = SERVICE_ADAPTOR_INTERNAL_ERROR_NO_DATA; */
/*			strncpy(ret_msg, "No Plugin"); */
		}

		GVariant *response = g_variant_new(MAKE_RETURN_TYPE(plugin_list_type), builder, (uint64_t) ret_code, ret_msg);
		g_dbus_method_invocation_return_value(invocation, response);

		g_variant_builder_unref(builder);
		service_adaptor_debug("[End] Get Auth Plugin List");
	} else if (0 == g_strcmp0(method_name, DBUS_SET_AUTH_METHOD)) {
		service_adaptor_debug("[START] Start Plugin");
		GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
		GVariant *req_struct[service_adaptor_set_auth_s_type_length];

		for (size_t j = 0; j < service_adaptor_set_auth_s_type_length; j++) {
			req_struct[j] = g_variant_get_child_value(in_parameters, j);
		}

		int idx = 0;

		char security_cookie[21] = {0, };
		gsize cookie_cnt = g_variant_n_children(req_struct[idx]);
		service_adaptor_info("cookie count : %d", cookie_cnt);
		/*service_adaptor_info("cookie : ");*/
		for (int i = 0; i < cookie_cnt; i++) {
			g_variant_get_child(req_struct[idx], i, "(y)", &(security_cookie[i]));
			/*service_adaptor_debug_func("%c", security_cookie[i]);*/
		}

		idx++;

		unsigned char *raw_data = NULL;
		int raw_data_len = (int) g_variant_n_children(req_struct[idx]);
		service_adaptor_info("raw data len : %d", raw_data_len);
		raw_data = (unsigned char *) calloc(raw_data_len, sizeof(unsigned char));
		bundle *plugin_property = NULL;
		if (NULL != raw_data) {
			for (int i = 0; i < raw_data_len; i++) {
				g_variant_get_child(req_struct[idx], i, "(y)", &(raw_data[i]));
			}
			plugin_property = bundle_decode(raw_data, raw_data_len);
		}
		idx++;

		char *service_name = ipc_g_variant_dup_string(req_struct[idx++]);
		char *plugin_uri = ipc_g_variant_dup_string(req_struct[idx++]);
		char *app_id = ipc_g_variant_dup_string(req_struct[idx++]);
		char *app_secret = ipc_g_variant_dup_string(req_struct[idx++]);
		char *user_id = ipc_g_variant_dup_string(req_struct[idx++]);
		char *user_password = ipc_g_variant_dup_string(req_struct[idx++]);
		int enable_mask = g_variant_get_int32(req_struct[idx++]);

		service_adaptor_debug("security cookie : %s", security_cookie);
		service_adaptor_debug("service name : %s", service_name);
		service_adaptor_debug("plugin_uri : %s", plugin_uri);
		service_adaptor_debug_func("app_id : %s", app_id);
		service_adaptor_debug_func("app_secret : %s", app_secret);
		service_adaptor_debug_func("user_id : %s", user_id);
		service_adaptor_debug_func("user_password : %s", user_password);
		service_adaptor_debug("enable mask : %d", enable_mask);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();

		service_adaptor_service_context_h service =
			service_adaptor_get_service_context(service_adaptor, service_name);

		if ((NULL != service) && (NULL != service->service_name) &&
				(0 == strncmp(service->service_name, service_name ? service_name : "", strlen(service->service_name)))) {
			service_adaptor_debug("Disconnect context for same service_name (%s)", service_name);
			service_adaptor_disconnect(service_adaptor, service_name);
			service = NULL;
		}

		service_adaptor_context_info_s new_context_info;
		__init_context_info_s(new_context_info);

		new_context_info.app_id = app_id;
		new_context_info.user_id = user_id;
		new_context_info.property = plugin_property;

		bool auth_enable = (enable_mask & AUTH_FLAG) ? true : false;
		bool storage_enable = (enable_mask & STORAGE_FLAG) ? true : false;

		ret_code = service_adaptor_connect(service_adaptor, &new_context_info, service_name,
				plugin_uri, user_password, app_secret, auth_enable, storage_enable, false, false, false, false, ret_msg);
		service_adaptor_debug("Called service_adaptor_connect (ret : %d)", ret_code);

		if (SERVICE_ADAPTOR_INTERNAL_ERROR_NONE != ret_code) {
			snprintf(ret_msg, RET_MSG_LEN,  "Cannot Connect %s Plugin", plugin_uri);
			ret_code = SERVICE_ADAPTOR_INTERNAL_ERROR_CONNECT;
		} else {
			service_adaptor_debug("Adds security cookie to client_checker (%d)",
					client_checker_add_client(service_name, security_cookie));
		}

		g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)", (uint64_t) ret_code, ret_msg));
		service_adaptor_debug("[End] Start Plugin");

		free(service_name);
		free(plugin_uri);
		free(app_id);
		free(app_secret);
		free(user_id);
		free(user_password);
		free(raw_data);

		free(new_context_info.imsi);
		free(new_context_info.duid);
		free(new_context_info.msisdn);
		free(new_context_info.access_token);
		free(new_context_info.refresh_token);
		if (new_context_info.property) {
			bundle_free((bundle *)new_context_info.property);
			new_context_info.property = NULL;
		}
	} else if ((0 == g_strcmp0(method_name, DBUS_IS_AUTH_METHOD)) || (0 == g_strcmp0(method_name, DBUS_JOIN_METHOD))) {
		service_adaptor_debug("[START] Is auth / join");
		GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
		GVariant *req_struct[service_adaptor_is_auth_req_s_type_length];

		for (size_t j = 0; j < service_adaptor_is_auth_req_s_type_length; j++) {
			req_struct[j] = g_variant_get_child_value(in_parameters, j);
		}

		int idx = 0;
		char *plugin_uri = ipc_g_variant_dup_string(req_struct[idx++]);
		char *user_id = NULL;
		char *user_pw = NULL;
		char *app_key = NULL;
		char *app_secret = NULL;
		char *imsi = NULL;
		char *device_id = NULL;
		char *service_name = "login_checker";

		unsigned char *raw_data = NULL;
		int raw_data_len = (int) g_variant_n_children(req_struct[idx]);
		service_adaptor_info("raw data len : %d", raw_data_len);
		raw_data = (unsigned char *) calloc(raw_data_len, sizeof(unsigned char));
		if (NULL != raw_data) {
			for (int i = 0; i < raw_data_len; i++) {
				g_variant_get_child(req_struct[idx], i, "(y)", &(raw_data[i]));
			}
			bundle *bd = bundle_decode(raw_data, raw_data_len);
			if (bd) {
				char *str = NULL;
				if (!bundle_get_str(bd, _PLUGIN_PROPERTY_KEY_BASE"user_id", &str)) {
					if (str) {
						user_id = strdup(str);
					}
				}
				str = NULL;
				if (!bundle_get_str(bd, _PLUGIN_PROPERTY_KEY_BASE"user_pw", &str)) {
					if (str) {
						user_pw = strdup(str);
					}
				}
				str = NULL;
				if (!bundle_get_str(bd, _PLUGIN_PROPERTY_KEY_BASE"app_key", &str)) {
					if (str) {
						app_key = strdup(str);
					}
				}
				str = NULL;
				if (!bundle_get_str(bd, _PLUGIN_PROPERTY_KEY_BASE"app_secret", &str)) {
					if (str) {
						app_secret = strdup(str);
					}
				}
				str = NULL;
				if (!bundle_get_str(bd, _PLUGIN_PROPERTY_KEY_BASE"imsi", &str)) {
					if (str) {
						imsi = strdup(str);
					}
				}
				str = NULL;
				if (!bundle_get_str(bd, _PLUGIN_PROPERTY_KEY_BASE"device_id", &str)) {
					if (str) {
						device_id = strdup(str);
					}
				}
				str = NULL;
				bundle_free(bd);
				bd = NULL;
			}
			free(raw_data);
		}

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		auth_adaptor_h auth_adaptor = service_adaptor_get_auth_adaptor(service_adaptor);
		auth_adaptor_plugin_h auth_plugin = auth_adaptor_get_plugin_by_name(auth_adaptor, plugin_uri);
		auth_adaptor_plugin_context_h auth_context =
				auth_adaptor_create_plugin_context(auth_plugin, user_id, user_pw, app_key, app_secret, imsi, service_name);

		auth_adaptor_error_code_h error = NULL;
		if (0 == g_strcmp0(method_name, DBUS_IS_AUTH_METHOD)) {
			int _is_auth = 0;
			service_adaptor_debug("try auth_adaptor_is_auth");
			ret_code = auth_adaptor_is_auth(auth_plugin, auth_context, NULL, &_is_auth, &error, NULL);
			if (ret_code) {
				if (error) {
					ret_code = error->code;
					snprintf(ret_msg, RET_MSG_LEN,  "%s", error->msg);
				} else {
					snprintf(ret_msg, RET_MSG_LEN,  "No error detailed");
				}
			}
			GVariant *response = g_variant_new(MAKE_RETURN_TYPE("(b)"), (_is_auth ? true : false), (uint64_t)ret_code, ret_msg);
			g_dbus_method_invocation_return_value(invocation, response);
		} else {
			service_adaptor_debug("try auth_adaptor_join");
			ret_code = auth_adaptor_join(auth_plugin, auth_context, device_id, NULL, &error, NULL);
			if (ret_code) {
				if (error) {
					ret_code = error->code;
					snprintf(ret_msg, RET_MSG_LEN,  "%s", error->msg);
				} else {
					snprintf(ret_msg, RET_MSG_LEN,  "No error detailed");
				}
			}
			g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)", (uint64_t)ret_code, ret_msg));
		}
		auth_adaptor_destroy_error_code(&error);
		auth_adaptor_destroy_plugin_context(auth_plugin, auth_context);
		auth_context = NULL;

		free(plugin_uri);
		free(user_id);
		free(user_pw);
		free(app_key);
		free(app_secret);
		free(imsi);
		free(device_id);
		service_adaptor_debug("[END] Is auth / join");
	}
/*
 *                       public feature
 *
 ***********************************************************************/

FUNC_END();
}

void auth_external_method_call(const char *service_name,
						const char *api_uri,
						const unsigned char *req_data,
						int req_len,
						unsigned char **res_data,
						int *res_len,
						int *ret_code,
						char *ret_msg)
{
	service_adaptor_debug("<Start> Auth External request");
	service_adaptor_h service_adaptor = service_adaptor_get_handle();
	service_adaptor_service_context_h service =
		service_adaptor_get_service_context(service_adaptor, service_name);

	if (NULL == service) {
		service_adaptor_info("Try API without context(%s)", service_name);
		/*
		*ret_code = AUTH_ADAPTOR_ERROR_NOT_FOUND;
		snprintf(ret_msg, 2047, "Can not get service context");

		return;
		*/
	}

	auth_adaptor_h adaptor = service_adaptor_get_auth_adaptor(service_adaptor);
	auth_adaptor_plugin_h plugin = NULL;
	auth_adaptor_plugin_context_h auth_context = NULL;

	if (NULL != service) {
		auth_context = service->auth_context;
		if (NULL != auth_context) {
			plugin = auth_adaptor_get_plugin_by_name(adaptor, auth_context->plugin_uri);
		} else {
			service_adaptor_error("Couldn't find context");
		}
	} else {
		/* It will be removed on public API */
		plugin = auth_adaptor_get_plugin_by_name(adaptor, "com.samsung.coreapps");
	}

	auth_adaptor_error_code_h error = NULL;
	if (NULL != plugin) {
		auth_error_code_t ret = AUTH_ADAPTOR_ERROR_NONE;
		service_adaptor_debug_func("Call auth_adaptor_external_request");
		ret = auth_adaptor_external_request(plugin, auth_context, api_uri,
				req_data, req_len, res_data, res_len, &error);
		service_adaptor_debug_func("API ret (%d)", ret);
	}

	if (NULL != error) {
		*ret_code = (int) error->code;
		service_adaptor_debug_func("error_code (%lld)(%s)", error->code, error->msg);
		if (NULL != error->msg) {
			snprintf(ret_msg, 2047, "%s", error->msg);
		}
		free(error->msg);
		free(error);
	} else {
		*ret_code = 0;
	}
	service_adaptor_debug("<End> Auth External request");
}
