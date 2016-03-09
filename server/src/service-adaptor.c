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
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <glib-object.h>
#include <glib-unix.h>

#include "service-adaptor.h"
#include "service-adaptor-auth.h"
#include "service-adaptor-contact.h"
#include "service-adaptor-message.h"
#include "service-adaptor-shop.h"
#include "service-adaptor-storage.h"
#include "service-adaptor-push.h"
#include "service-adaptor-plugin.h"
#include "service-adaptor-type.h"
#include "service-adaptor-log.h"
#include "dbus-ipc.h"
#include "dbus-server.h"
#include "dbus-service-adaptor.h"
#include "util/client_checker.h"
#include "util/ping_manager.h"

#include <bundle.h>

service_adaptor_h g_service_adaptor = (void *) NULL;

static char *safe_strdup(const char *str)
{
	if (NULL == str) {
		return NULL;
	} else {
		return strdup(str);
	}
}

static void safe_free(void **ptrptr)
{
	if (ptrptr != NULL && *ptrptr != NULL) {
		free(*ptrptr);
		*ptrptr = NULL;
	}
}

/******************************************************************************
 * Global variables and defines
 ******************************************************************************/

static GMainLoop *g_default_loop = NULL;

/******************************************************************************
 * Private interface
 ******************************************************************************/

#define __init_context_info_s(x)        do { \
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


/******************************************************************************
 * Private interface definition
 ******************************************************************************/

/******************************************************************************
 * Public interface definition
 ******************************************************************************/

service_adaptor_h service_adaptor_get_handle()
{
	service_adaptor_debug("Get adaptor handle");

	return g_service_adaptor;
}

void debug_service_context(GList *service_list)
{
#ifdef SERVICE_ADAPTOR_DEBUG_CONTEXT
	if (NULL == service_list) {
		return;
	}

	int service_count = g_list_length(service_list);

	for (int i = 0; i < service_count; i++) {
		service_adaptor_service_context_h service = g_list_nth_data(service_list, i);

		service_adaptor_debug_func("[CONTEXT_DEBUG]  ============ index %d =============", i);
		if (service) {
			service_adaptor_debug_func("[CONTEXT_DEBUG] service_name(%s)", service->service_name);
			service_adaptor_debug_func("[CONTEXT_DEBUG] plugin_uri (%s)", service->plugin_uri);
			service_adaptor_debug_func("[CONTEXT_DEBUG] ctx [auth(%p) contact(%p) storage(%p) message(%p) push(%p) shop(%p)]",
					service->auth_context, service->contact_context, service->storage_context,
					service->message_context, service->push_context, service->shop_context);
			service_adaptor_debug_func("[CONTEXT_DEBUG] server_info (%p)", service->server_info);
			service_adaptor_debug_func("[CONTEXT_DEBUG] connected (%d)", (int)service->connected);
			service_adaptor_debug_func("[CONTEXT_DEBUG]==========================================");
		}
	}
#endif
}

static void __glog_handler_cb(const gchar *log_domain,
				GLogLevelFlags log_level,
				const gchar *message,
				gpointer user_data)
{
	service_adaptor_error("============================================================");
	service_adaptor_error("============================================================");
	service_adaptor_error("================== Critical GLib Error =====================");
	service_adaptor_error("============================================================");
	service_adaptor_error("============================================================");
	service_adaptor_error("=== Log Domain : %s", log_domain);
	service_adaptor_error("=== Level : %d", (int)log_level);
	service_adaptor_error("=== Message : %s", message);
	service_adaptor_error("============================================================");
	service_adaptor_error("============================================================");
}

static void glog_handler_init()
{
	service_adaptor_info("glib log handler init : %d",
			(int)g_log_set_handler("GLib", G_LOG_LEVEL_CRITICAL, __glog_handler_cb, NULL));
}

service_adaptor_service_context_h service_adaptor_get_service_context(service_adaptor_h service_adaptor,
						const char *service_name)
{
FUNC_START();
	service_adaptor_debug("Get service context");

	if ((NULL == service_adaptor) || (NULL == service_name)) {
		service_adaptor_error("Invalid argument");
		return NULL;
	}

	int service_count = g_list_length(service_adaptor->service_list);
	/* debug_service_context(service_adaptor->service_list); */

	for (int i = 0; i < service_count; i++) {
		service_adaptor_service_context_h service = g_list_nth_data(service_adaptor->service_list, i);

		if ((NULL != service) && (0 == strncmp(service->service_name, service_name, strlen(service_name)))) {
			return service;
		}
	}

	service_adaptor_warning("First time connected : service_name(%s)", service_name);

FUNC_END();
	return NULL;
}

GList *service_adaptor_get_services_by_plugin_uri(service_adaptor_h service_adaptor,
						const char *plugin_uri)
{
FUNC_START();
	service_adaptor_debug("Get service context");

	if (NULL == service_adaptor) {
		service_adaptor_error("Invalid argument");
		return NULL;
	}

	GList *service_list = NULL;
	int service_count = g_list_length(service_adaptor->service_list);
	service_adaptor_debug("service count : %d", service_count);

	for (int i = 0; i < service_count; i++) {
		service_adaptor_service_context_h service = g_list_nth_data(service_adaptor->service_list, i);
		if (NULL != service) {
			service_adaptor_debug("service name : %s", service->service_name);
		}

		if ((NULL != service) && (0 == strncmp(service->plugin_uri, plugin_uri, strlen(plugin_uri)))) {
			service_list = g_list_append(service_list, service);
		}
	}

	if (NULL == service_list) {
		service_adaptor_info("Could not get service context with plugin_uri(%s)", plugin_uri);
	}

FUNC_END();
	return service_list;
}

service_adaptor_internal_error_code_e service_adaptor_bind_storage_context(service_adaptor_h service_adaptor,
						service_adaptor_service_context_h service_src,
						service_adaptor_service_context_h service_dst)
{
FUNC_START();
	service_adaptor_debug("START");

	if ((NULL == service_adaptor) || (NULL == service_src) || (NULL == service_dst)) {
		service_adaptor_error("Invalid parameter");
		return SERVICE_ADAPTOR_INTERNAL_ERROR_INVALID_ARGUMENT;
	}

	storage_adaptor_plugin_h plugin = storage_adaptor_get_plugin_by_name(service_adaptor->storage_handle, service_dst->plugin_uri);
	if (NULL != service_dst->storage_context) {
		storage_adaptor_destroy_plugin_context(plugin, service_dst->storage_context);
	}

	service_dst->storage_context = service_src->storage_context;

	service_adaptor_debug("END");

FUNC_END();
	return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
}

service_adaptor_internal_error_code_e service_adaptor_bind_push_context(service_adaptor_h service_adaptor,
						service_adaptor_service_context_h service_src,
						service_adaptor_service_context_h service_dst)
{
FUNC_START();
	service_adaptor_debug("START");

	if ((NULL == service_adaptor) || (NULL == service_src) || (NULL == service_dst)) {
		service_adaptor_error("Invalid parameter");
		return SERVICE_ADAPTOR_INTERNAL_ERROR_INVALID_ARGUMENT;
	}

	/*push_adaptor_plugin_h plugin = push_adaptor_get_plugin_by_name(service_adaptor->push_handle, service_dst->plugin_uri);*/
	if (NULL != service_dst->push_context) {
		service_dst->push_context = NULL;
	}

	service_dst->push_context = service_src->push_context;

	service_adaptor_debug("END");

FUNC_END();
	return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
}

service_adaptor_internal_error_code_e service_adaptor_bind_context(service_adaptor_h service_adaptor,
						service_adaptor_service_context_h service_src,
						service_adaptor_service_context_h service_dst)
{
FUNC_START();
	service_adaptor_debug("START");

	if ((NULL == service_adaptor) || (NULL == service_src) || (NULL == service_dst)) {
		service_adaptor_error("Invalid parameter");
		return SERVICE_ADAPTOR_INTERNAL_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == service_dst->auth_context) {
		service_dst->auth_context = service_src->auth_context;
	}

	if (NULL == service_dst->contact_context) {
		service_dst->contact_context = service_src->contact_context;
	}

	if (NULL == service_dst->message_context) {
		service_dst->message_context = service_src->message_context;
	}

	if (NULL == service_dst->shop_context) {
		service_dst->shop_context = service_src->shop_context;
	}

	if (NULL == service_dst->storage_context) {
		service_dst->storage_context = service_src->storage_context;
	}

	if (NULL == service_dst->push_context) {
		service_dst->push_context = service_src->push_context;
	}

	service_adaptor_debug("END");

FUNC_END();
	return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
}

/**
 * @brief Check service_context is binded
 * @param[in] service_adaptor	specifies Service-adaptor handle
 * @param[in] app_id		specifies app_id
 * @param[in] service_id	specifies service_id
 * @return count of context, otherwise a negative error value
 **/
int service_adaptor_is_service_binded(service_adaptor_h service_adaptor,
						const char *service_package_id)
{
FUNC_START();
	if ((NULL == service_adaptor) || (NULL == service_package_id)) {
		return 0;
	}

	if ((NULL == service_adaptor->service_list) || (0 >= g_list_length(service_adaptor->service_list))) {
		return 0;
	}

	GList *list = NULL;
	char *temp_service_name;
	for (list = g_list_first(service_adaptor->service_list); NULL != list; list = g_list_next(list)) {
		if (NULL != list->data) {
			temp_service_name = ((service_adaptor_service_context_h) list->data)->service_name;
			if ((NULL != temp_service_name)) {
				if (0 == strcmp(temp_service_name, service_package_id)) {
					return 1;
				}
			}
		}
	}
FUNC_END();
	return 0;
}

service_adaptor_internal_error_code_e service_adaptor_connect(service_adaptor_h service_adaptor,
						service_adaptor_context_info_s *context_info,
						const char *service_name,
						const char *plugin_uri,
						const char *user_password,
						const char *app_secret,
						bool auth_enable,
						bool storage_enable,
						bool contact_enable,
						bool message_enable,
						bool push_enable,
						bool shop_enable,
						char *ret_msg)
{
FUNC_START();
	service_adaptor_debug("Connect to adaptors: %s", plugin_uri);

	if ((NULL == service_adaptor) || (NULL == context_info) || (NULL == service_name)
			|| (NULL == plugin_uri)) {
		service_adaptor_error("Invalid parameter");
		return SERVICE_ADAPTOR_INTERNAL_ERROR_INVALID_ARGUMENT;
	}

	service_adaptor_service_context_h service =
			service_adaptor_get_service_context(service_adaptor, service_name);

	if (NULL != service) {
		service_adaptor_info("Already connected to adaptors: %s", service_name);
		return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
	} else {
		service_adaptor_debug("Create service_context_h");
		service = (service_adaptor_service_context_h) calloc(1, sizeof(service_adaptor_service_context_s));
		service_adaptor_context_info_s *_context_info =
			(service_adaptor_context_info_s *) calloc(1, sizeof(service_adaptor_context_info_s));

		if ((NULL == service) || (NULL == _context_info)) {
			service_adaptor_error("Critical : Memory allocation failed");
			free(service);
			free(_context_info);
			snprintf(ret_msg, 2048, "Critical : There is no memory remained");
			return SERVICE_ADAPTOR_INTERNAL_ERROR_ADAPTOR_INTERNAL;
		}

		g_mutex_init(&service->service_context_mutex);
		g_cond_init(&service->service_context_cond);

		service->auth_context = NULL;
		service->contact_context = NULL;
		service->message_context = NULL;
		service->shop_context = NULL;
		service->push_context = NULL;
		service->storage_context = NULL;

		service->service_name = safe_strdup(service_name);
		service->plugin_uri = safe_strdup(plugin_uri);

		service->context_info = _context_info;

		service->context_info->user_id = safe_strdup(context_info->user_id);
		service->context_info->app_id = safe_strdup(context_info->app_id);
		service->context_info->imsi = safe_strdup(context_info->imsi);
		service->context_info->service_id = context_info->service_id;
		service->context_info->property = context_info->property; /* Not dup */
		context_info->property = NULL;

		service->connected = 0x0000000;
	}

FUNC_STEP();
	/* 1) Connect to AUTH PLUGIN */
	int ret = SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
	int last_res = SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;

	if (auth_enable) {
		FUNC_STEP();
		ret += (last_res = service_adaptor_connect_auth_plugin(service_adaptor, service->context_info, service_name,
				plugin_uri, user_password, app_secret, &service, ret_msg));

		if ((SERVICE_ADAPTOR_INTERNAL_ERROR_NONE != ret) || (NULL == service)) {
			service_adaptor_error("Could not connect to auth plugin: %d", ret);
			goto CONNECT_API_CHECK_RESULT;
		}
	}

	/* 2) Connect to CONTACT PLUGIN */
	if (contact_enable) {
		FUNC_STEP();
		ret += (last_res = service_adaptor_connect_contact_plugin(service_adaptor, service, ret_msg));

		if (SERVICE_ADAPTOR_INTERNAL_ERROR_NONE != ret) {
			service_adaptor_warning("Could not connect to contact plugin: %d", ret);
			goto CONNECT_API_CHECK_RESULT;
		}
	}

	/* 3) Connect to MESSAGE PLUGIN */
	if (message_enable) {
		FUNC_STEP();
		ret += (last_res = service_adaptor_connect_message_plugin(service_adaptor, service, ret_msg));

		if (SERVICE_ADAPTOR_INTERNAL_ERROR_NONE != ret) {
			service_adaptor_warning("Could not connect to message plugin: %d", ret);
			goto CONNECT_API_CHECK_RESULT;
		}
	}

	/* 4) Connect to SHOP PLUGIN */
	if (shop_enable) {
		FUNC_STEP();
		ret += (last_res = service_adaptor_connect_shop_plugin(service_adaptor, service, ret_msg));

		if (SERVICE_ADAPTOR_INTERNAL_ERROR_NONE != ret) {
			service_adaptor_warning("Could not connect to shop plugin: %d", ret);
			goto CONNECT_API_CHECK_RESULT;
		}
	}

	/* 5) Connect to STORAGE PLUGIN */
	if (storage_enable) {
		FUNC_STEP();
		ret += (last_res = service_adaptor_connect_storage_plugin(service_adaptor, service, app_secret, ret_msg));

		if (SERVICE_ADAPTOR_INTERNAL_ERROR_NONE != ret) {
			service_adaptor_warning("Could not connect to storage plugin: %d", ret);
			goto CONNECT_API_CHECK_RESULT;
		}
	}

	/* 6) Connect to PUSH PLUGIN */
	if (push_enable) {
		FUNC_STEP();
		ret += (last_res = service_adaptor_connect_push_plugin(service_adaptor, service, ret_msg));

		if (SERVICE_ADAPTOR_INTERNAL_ERROR_NONE != ret) {
			service_adaptor_warning("Could not connect to push plugin: %d", ret);
			goto CONNECT_API_CHECK_RESULT;
		}
	}

CONNECT_API_CHECK_RESULT:
	if (ret || (0 == service->connected)) {
		service_adaptor_warning("Plugin Connect failed : ret(%d) connected(%d)", ret, service->connected);
		service_adaptor_warning("<service_name : %s>", service_name);

		g_mutex_clear(&service->service_context_mutex);
		g_cond_clear(&service->service_context_cond);

		free(service->context_info->user_id);
		free(service->context_info->app_id);
		free(service->context_info->imsi);

		free(service->context_info);

		free(service->service_name);
		free(service->plugin_uri);

		free(service);

		return last_res;
	}


	service_adaptor->service_list = g_list_append(service_adaptor->service_list, service);

	service_adaptor_debug_func("Connect success <service_name : %s>", service_name);
	debug_service_context(service_adaptor->service_list);
FUNC_END();
	return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
}

service_adaptor_internal_error_code_e service_adaptor_disconnect(service_adaptor_h service_adaptor,
						const char *service_name)
{
FUNC_START();
	service_adaptor_debug("Disconnects adaptor contexts");

	service_adaptor_service_context_h service = NULL;
	while (NULL != (service = service_adaptor_get_service_context(service_adaptor, service_name))) {

		if (NULL == service) {
			service_adaptor_debug("service context already released");
			return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
		}

		service_adaptor_internal_error_code_e ret = SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
		ret = service_adaptor_disconnect_message_plugin(service_adaptor, service);
		service_adaptor_debug("Disconnected message (%d)", ret);

		ret = service_adaptor_disconnect_contact_plugin(service_adaptor, service);
		service_adaptor_debug("Disconnected contact (%d)", ret);

		ret = service_adaptor_disconnect_shop_plugin(service_adaptor, service);
		service_adaptor_debug("Disconnected shop (%d)", ret);

		ret = service_adaptor_disconnect_storage_plugin(service_adaptor, service);
		service_adaptor_debug("Disconnected storage (%d)", ret);

		ret = service_adaptor_disconnect_push_plugin(service_adaptor, service);
		service_adaptor_debug("Disconnected push (%d)", ret);

		ret = service_adaptor_disconnect_auth_plugin(service_adaptor, service);
		service_adaptor_debug("Disconnected auth (%d)", ret);

		if ((NULL != service) && (NULL != service->server_info)) {
			g_hash_table_destroy(service->server_info);
		}

		service_adaptor_debug_func("Clears mutex & cond");
		g_mutex_clear(&service->service_context_mutex);
		g_cond_clear(&service->service_context_cond);

		free(service->service_name);
		free(service->plugin_uri);

		service_adaptor_debug_func("Clears context info");
		if (NULL != (service->context_info)) {
			free(service->context_info->user_id);
			free(service->context_info->app_id);
			free(service->context_info->imsi);
			free(service->context_info->duid);
			free(service->context_info->msisdn);
			free(service->context_info->access_token);
			free(service->context_info->refresh_token);
			if (service->context_info->property) {
				bundle_free((bundle *)(service->context_info->property));
			}
		}
		free(service->context_info);

		service_adaptor_debug_func("Removes from service list");
		service_adaptor->service_list = g_list_remove(service_adaptor->service_list, service);
		free(service);
		service = NULL;
	}
	service_adaptor_debug("Disconnected from adaptors");

FUNC_END();
	return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
}


service_adaptor_internal_error_code_e service_adaptor_start(service_adaptor_h service_adaptor)
{
FUNC_START();
	service_adaptor_debug("Service Adaptor: Start");

	if (NULL == service_adaptor) {
		service_adaptor_error("Invalid argument");
		return SERVICE_ADAPTOR_INTERNAL_ERROR_INVALID_ARGUMENT;
	}

	int ret = auth_adaptor_start(service_adaptor->auth_handle);

	if (AUTH_ADAPTOR_ERROR_NONE != ret) {
		service_adaptor_error("Could not start auth adaptor");
		service_adaptor_info("Auth-adaptor is mandatory");
/*		return SERVICE_ADAPTOR_INTERNAL_ERROR_LAUNCH; */
	}

	service_adaptor_debug("Auth Adaptor: Started");

	ret = contact_adaptor_start(service_adaptor->contact_handle);

	if (CONTACT_ADAPTOR_ERROR_NONE != ret) {
		service_adaptor_warning("Could not start contact adaptor");
/*		return SERVICE_ADAPTOR_INTERNAL_ERROR_LAUNCH; */
	}

	service_adaptor_debug("Contact Adaptor: Started");

	ret = message_adaptor_start(service_adaptor->message_handle);

	if (MESSAGE_ADAPTOR_ERROR_NONE != ret) {
		service_adaptor_warning("Could not start message adaptor");
/*		return SERVICE_ADAPTOR_INTERNAL_ERROR_LAUNCH; */
	}

	service_adaptor_debug("Message Adaptor: Started");

	ret = shop_adaptor_start(service_adaptor->shop_handle);

	if (SHOP_ADAPTOR_ERROR_NONE != ret) {
		service_adaptor_warning("Could not start shop adaptor");
/*		return SERVICE_ADAPTOR_INTERNAL_ERROR_LAUNCH; */
	}

	service_adaptor_debug("Shop Adaptor: Started");

	ret = storage_adaptor_start(service_adaptor->storage_handle);

	if (STORAGE_ADAPTOR_ERROR_NONE != ret) {
		service_adaptor_warning("Could not start storage adaptor");
/*		return SERVICE_ADAPTOR_INTERNAL_ERROR_LAUNCH; */
	}

	service_adaptor_debug("Storage Adaptor: Started");

	ret = push_adaptor_start(service_adaptor->push_handle);

	if (PUSH_ADAPTOR_ERROR_NONE != ret) {
		service_adaptor_warning("Could not start push adaptor");
/*		return SERVICE_ADAPTOR_INTERNAL_ERROR_LAUNCH; */
	}

	service_adaptor_debug("Push Adaptor: Started");
/*
	ret = service_adaptor_scan_all_packages_async(service_adaptor);
	service_adaptor_debug("Scan all packages ret(%d)", ret);

	ret = service_adaptor_set_package_installed_callback(service_adaptor);
	service_adaptor_debug("Sets package installed callback ret(%d)", ret);
*/
	g_mutex_lock(&service_adaptor->service_adaptor_mutex);
	service_adaptor->started = 1;
	g_cond_signal(&service_adaptor->service_adaptor_cond);
	g_mutex_unlock(&service_adaptor->service_adaptor_mutex);

FUNC_END();
	return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
}

service_adaptor_internal_error_code_e service_adaptor_stop(service_adaptor_h service_adaptor)
{
FUNC_START();
	service_adaptor_debug("Service Adaptor: Stop");

	if (NULL == service_adaptor) {
		service_adaptor_error("Invalid argument");
		return SERVICE_ADAPTOR_INTERNAL_ERROR_INVALID_ARGUMENT;
	}

	if (0 <= service_adaptor->started) {
		service_adaptor_error("Service Adaptor is not running");
		return SERVICE_ADAPTOR_INTERNAL_ERROR_START;
	}

FUNC_STEP();
	int ret = auth_adaptor_stop(service_adaptor->auth_handle);
	ret += contact_adaptor_stop(service_adaptor->contact_handle);
	ret += message_adaptor_stop(service_adaptor->message_handle);
	ret += shop_adaptor_stop(service_adaptor->shop_handle);
	ret += storage_adaptor_stop(service_adaptor->storage_handle);
	ret += push_adaptor_stop(service_adaptor->push_handle);

	service_adaptor->started = 0;

	if (0 != ret) {
		service_adaptor_error("Error while stopping adaptors");
		return SERVICE_ADAPTOR_INTERNAL_ERROR_CORRUPTED;
	}

	service_adaptor_debug("Service Adaptor: Stopped");

FUNC_END();
	return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
}

/**************************************************************************
 * Create / Destroy Adaptors
 **************************************************************************/
service_adaptor_h service_adaptor_create()
{
FUNC_START();
	service_adaptor_debug("Service Adaptor: Create");

	/* create handle of adaptor */
	auth_adaptor_h auth_handle = service_adaptor_create_auth();

	if (NULL == auth_handle) {
		service_adaptor_error("Could not create auth adaptor");
		return NULL;
	}

	contact_adaptor_h contact_handle = service_adaptor_create_contact();

	if (NULL == contact_handle) {
		service_adaptor_error("Could not create contact adaptor");
		auth_adaptor_destroy(auth_handle);
		return NULL;
	}

	message_adaptor_h message_handle = service_adaptor_create_message();

	if (NULL == message_handle) {
		service_adaptor_error("Could not create message adaptor");
		auth_adaptor_destroy(auth_handle);
		contact_adaptor_destroy(contact_handle);
		return NULL;
	}

	shop_adaptor_h shop_handle = service_adaptor_create_shop();

	if (NULL == shop_handle) {
		service_adaptor_error("Could not create shop adaptor");
		auth_adaptor_destroy(auth_handle);
		contact_adaptor_destroy(contact_handle);
		message_adaptor_destroy(message_handle);
		return NULL;
	}

	storage_adaptor_h storage_handle = service_adaptor_create_storage();

	if (NULL == storage_handle) {
		service_adaptor_error("Could not create storage adaptor");
		auth_adaptor_destroy(auth_handle);
		contact_adaptor_destroy(contact_handle);
		message_adaptor_destroy(message_handle);
		shop_adaptor_destroy(shop_handle);
		return NULL;
	}

	push_adaptor_h push_handle = service_adaptor_create_push();

	if (NULL == push_handle) {
		service_adaptor_error("Could not create push adaptor");
		auth_adaptor_destroy(auth_handle);
		contact_adaptor_destroy(contact_handle);
		message_adaptor_destroy(message_handle);
		shop_adaptor_destroy(shop_handle);
		storage_adaptor_destroy(storage_handle);
		return NULL;
	}

	/* register listener of adaptor */
	auth_adaptor_listener_h auth_listener = service_adaptor_register_auth_listener(auth_handle);

	if (NULL == auth_listener) {
		service_adaptor_error("Could not create auth listener");
		auth_adaptor_destroy(auth_handle);
		contact_adaptor_destroy(contact_handle);
		message_adaptor_destroy(message_handle);
		shop_adaptor_destroy(shop_handle);
		storage_adaptor_destroy(storage_handle);
		push_adaptor_destroy(push_handle);
		return NULL;
	}

	contact_adaptor_listener_h contact_listener = service_adaptor_register_contact_listener(contact_handle);

	if (NULL == contact_listener) {
		service_adaptor_error("Could not create contact listener");
		auth_adaptor_unregister_listener(auth_handle, auth_listener);

		auth_adaptor_destroy(auth_handle);
		contact_adaptor_destroy(contact_handle);
		message_adaptor_destroy(message_handle);
		shop_adaptor_destroy(shop_handle);
		storage_adaptor_destroy(storage_handle);
		push_adaptor_destroy(push_handle);
		return NULL;
	}

	message_adaptor_listener_h message_listener = service_adaptor_register_message_listener(message_handle);

	if (NULL == message_listener) {
		service_adaptor_error("Could not create message listener");
		auth_adaptor_unregister_listener(auth_handle, auth_listener);
		contact_adaptor_unregister_listener(contact_handle, contact_listener);

		auth_adaptor_destroy(auth_handle);
		contact_adaptor_destroy(contact_handle);
		message_adaptor_destroy(message_handle);
		shop_adaptor_destroy(shop_handle);
		storage_adaptor_destroy(storage_handle);
		push_adaptor_destroy(push_handle);
		return NULL;
	}

	shop_adaptor_listener_h shop_listener = service_adaptor_register_shop_listener(shop_handle);

	if (NULL == shop_listener) {
		service_adaptor_error("Could not create shop listener");
		auth_adaptor_unregister_listener(auth_handle, auth_listener);
		contact_adaptor_unregister_listener(contact_handle, contact_listener);
		message_adaptor_unregister_listener(message_handle, message_listener);

		auth_adaptor_destroy(auth_handle);
		contact_adaptor_destroy(contact_handle);
		message_adaptor_destroy(message_handle);
		shop_adaptor_destroy(shop_handle);
		storage_adaptor_destroy(storage_handle);
		push_adaptor_destroy(push_handle);
		return NULL;
	}

	storage_adaptor_listener_h storage_listener = service_adaptor_register_storage_listener(storage_handle);

	if (NULL == storage_listener) {
		service_adaptor_error("Could not create storage listener");
		auth_adaptor_unregister_listener(auth_handle, auth_listener);
		contact_adaptor_unregister_listener(contact_handle, contact_listener);
		message_adaptor_unregister_listener(message_handle, message_listener);
		shop_adaptor_unregister_listener(shop_handle, shop_listener);

		auth_adaptor_destroy(auth_handle);
		contact_adaptor_destroy(contact_handle);
		message_adaptor_destroy(message_handle);
		shop_adaptor_destroy(shop_handle);
		storage_adaptor_destroy(storage_handle);
		push_adaptor_destroy(push_handle);
		return NULL;
	}

	push_adaptor_listener_h push_listener = service_adaptor_register_push_listener(push_handle);

	if (NULL == push_listener) {
		service_adaptor_error("Could not create push listener");
		auth_adaptor_unregister_listener(auth_handle, auth_listener);
		contact_adaptor_unregister_listener(contact_handle, contact_listener);
		message_adaptor_unregister_listener(message_handle, message_listener);
		shop_adaptor_unregister_listener(shop_handle, shop_listener);
		storage_adaptor_unregister_listener(storage_handle, storage_listener);

		auth_adaptor_destroy(auth_handle);
		contact_adaptor_destroy(contact_handle);
		message_adaptor_destroy(message_handle);
		shop_adaptor_destroy(shop_handle);
		storage_adaptor_destroy(storage_handle);
		push_adaptor_destroy(push_handle);
		return NULL;
	}

	/* create Service Adaptor */
	service_adaptor_h service_adaptor = (service_adaptor_h) g_malloc0(sizeof(service_adaptor_s));

	if (NULL == service_adaptor) {
		service_adaptor_error("Could not create service adaptor");
		auth_adaptor_unregister_listener(auth_handle, auth_listener);
		contact_adaptor_unregister_listener(contact_handle, contact_listener);
		message_adaptor_unregister_listener(message_handle, message_listener);
		shop_adaptor_unregister_listener(shop_handle, shop_listener);
		storage_adaptor_unregister_listener(storage_handle, storage_listener);
		push_adaptor_unregister_listener(push_handle, push_listener);

		auth_adaptor_destroy(auth_handle);
		contact_adaptor_destroy(contact_handle);
		message_adaptor_destroy(message_handle);
		shop_adaptor_destroy(shop_handle);
		storage_adaptor_destroy(storage_handle);
		push_adaptor_destroy(push_handle);
		return NULL;
	}

FUNC_STEP();
	service_adaptor->auth_handle		= auth_handle;
	service_adaptor->contact_handle		= contact_handle;
	service_adaptor->message_handle		= message_handle;
	service_adaptor->shop_handle		= shop_handle;
	service_adaptor->storage_handle		= storage_handle;
	service_adaptor->push_handle		= push_handle;

	service_adaptor->auth_listener		= auth_listener;
	service_adaptor->push_listener		= push_listener;
	service_adaptor->shop_listener		= shop_listener;
	service_adaptor->contact_listener	= contact_listener;
	service_adaptor->storage_listener	= storage_listener;
	service_adaptor->message_listener	= message_listener;

	service_adaptor->service_list = NULL;
	service_adaptor->started = 0;

	g_mutex_init(&service_adaptor->service_adaptor_mutex);
	g_cond_init(&service_adaptor->service_adaptor_cond);

	service_adaptor_debug("Service Adaptor: Created");

	service_adaptor_info("Init client checker (%d)", client_checker_init());

FUNC_END();
	return service_adaptor;
}

void service_adaptor_destroy(service_adaptor_h service_adaptor)
{
FUNC_START();
	service_adaptor_debug("Service Adaptor: Destroy");
	client_checker_deinit();
	service_adaptor_info("Deinit client checker");

	if ((void *) NULL == service_adaptor) {
		service_adaptor_debug("Invalid argument");
		return;
	}

	/* 1) deinit D-Bus */
	dbus_ipc_server_layer_deinit();

	service_adaptor_debug("Service adaptor destroyed: D-Bus");

	/* 2) destroy service list */
	if (NULL != service_adaptor->service_list) {
		g_list_free(service_adaptor->service_list);
		service_adaptor->service_list = NULL;
	}

	service_adaptor_debug("Service adaptor destroyed: service list");

	/* 3) stop service adaptor */
	if (0 < service_adaptor->started) {
		service_adaptor_error("Service Adaptor is running. Force stopping before destroy");
		service_adaptor_stop(service_adaptor);
	}

	service_adaptor_safe_free(service_adaptor);

	service_adaptor_debug("Service Adaptor: Destroyed");
FUNC_END();
}

void *servive_adaptor_preload_service_files(void *data)
{
FUNC_START();
	service_adaptor_debug("5 sec sleep for load service files (TODO change)");
	sleep(5);
	service_adaptor_internal_error_code_e ret;
	push_activate_h *services = NULL;
	int svc_len = 0;
	ret = service_adaptor_ref_enabled_push_services(&services, &svc_len);

	service_adaptor_debug_func("### Preload service len : %d", svc_len);
	if (SERVICE_ADAPTOR_INTERNAL_ERROR_NONE == ret) {
		for (int i = 0; i < svc_len; i++) {
			service_adaptor_context_info_s new_context_info;
			__init_context_info_s(new_context_info);
			new_context_info.app_id = services[i]->app_id;
			char service_name[1024] = {0, };
			snprintf(service_name, 1024, "preloaded_service/plugin='%s'&app_id='%s'",
					services[i]->plugin_uri, services[i]->app_id);
			char ret_msg[2048] = {0, };
			ret = service_adaptor_connect(g_service_adaptor, &new_context_info, service_name,
					services[i]->plugin_uri, "", "",
					false, false, false, false, true, false, ret_msg);
			service_adaptor_debug_func("### Preload service : ret(%d) service_name(%s)", ret, service_name);
		}
		free(services);
	}

FUNC_END();
	return NULL;
}

/**
 * @brief init service adaptor
 *
 * @return	void.
 */
service_adaptor_internal_error_code_e service_adaptor_init()
{
FUNC_START();
	service_adaptor_debug("Service Adaptor: Initialize");

	int ret = SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;

	glog_handler_init();

	/* 1) create adaptor (memory allocation) */
	service_adaptor_h service_adaptor = service_adaptor_create();

	if (NULL == service_adaptor) {
		service_adaptor_error("Could not create Service Adaptor");
		return SERVICE_ADAPTOR_INTERNAL_ERROR_CREATE;
	}

	service_adaptor_debug("Service Adaptor: created");

	/* 2) start adaptor (plugin load, get IMIS list) */
	ret = service_adaptor_start(service_adaptor);

	if (SERVICE_ADAPTOR_INTERNAL_ERROR_NONE != ret) {
		service_adaptor_error("Could not load Adaptors: %d", ret);
		return SERVICE_ADAPTOR_INTERNAL_ERROR_START;
	}

	service_adaptor_debug("Service Adaptor: started");
/*	TODO it will be activated for on-demand */
	ping_manager_init(5, g_default_loop);

	/* 3) assign to global service adaptor handle */
	g_service_adaptor = service_adaptor;

	/* 4) init dbus */
	ret = dbus_ipc_server_layer_init();

	if (SERVICE_ADAPTOR_INTERNAL_ERROR_NONE != ret) {
		service_adaptor_error("Could not init D-Bus IPC server: %d", ret);
		return SERVICE_ADAPTOR_INTERNAL_ERROR_DBUS;
	}

	service_adaptor_debug("Service Adaptor: init D-Bus");
	service_adaptor_debug("Service Adaptor: Initialized (%d)", service_adaptor->started);

	service_adaptor_debug("Service Adaptor: init preload service");
	pthread_t job;
	pthread_create(&job, NULL, servive_adaptor_preload_service_files, NULL);
FUNC_END();
	return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
}

/**
 * @brief deinit service adaptor
 *
 * @param[in]	service_adaptor		specifies handle of service adaptor
 * @return	void.
 */
void service_adaptor_deinit()
{
	service_adaptor_debug("Service Adaptor: Deinitialize");

	if (NULL == g_service_adaptor) {
		service_adaptor_error("Invalid argument");
		return;
	}

	service_adaptor_h service_adaptor = g_service_adaptor;
	g_service_adaptor = NULL;
	service_adaptor_destroy(service_adaptor);
	service_adaptor_debug("Service Adaptor: Deinitialized");
}

/**
 * @brief main signal function
 *
 * @param[in]	data		specifies user data passed by main function
 * @return	void.
 */
static gint sigterm_callback(void *data)
{
	service_adaptor_info("Service Adaptor Shutdown");

	g_main_loop_quit((GMainLoop *)data);

	return FALSE;
}

/******************************************************************************
  Public interface definition
 *****************************************************************************/

service_adaptor_internal_error_code_e service_adaptor_auth_refresh(service_adaptor_h service_adaptor,
						const char *service_name,
						const char *plugin_uri)
{
FUNC_START();
	service_adaptor_debug("Auth refresh Start");

	service_adaptor_service_context_h service_context =
			service_adaptor_get_service_context(service_adaptor, service_name);

FUNC_END();
	return service_adaptor_auth_refresh_with_service_context(service_adaptor, service_context, plugin_uri);
}

service_adaptor_internal_error_code_e service_adaptor_auth_refresh_with_service_context(service_adaptor_h service_adaptor,
						service_adaptor_service_context_h service_context,
						const char *plugin_uri)
{
FUNC_START();
	service_adaptor_debug("Auth refresh with service context Start");

	if (NULL == service_context) {
		service_adaptor_warning("Parameter is NULL (service_adaptor_h service_adaptor)");
		return SERVICE_ADAPTOR_INTERNAL_ERROR_INVALID_HANDLE;
	}

	if (NULL == service_context->auth_context) {
		service_adaptor_warning("Parameter is NULL (service_context->auth_context)");
		return SERVICE_ADAPTOR_INTERNAL_ERROR_INVALID_HANDLE;
	}

	int ret = SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
	char *old_access_token = NULL;
	char *new_access_token = NULL;
	char *new_uid = NULL;

	service_adaptor_debug("Get contexts and plugins from service_adaptor_h");
	if (NULL != service_context->context_info) {
		old_access_token = service_context->context_info->access_token;
	} else {
		service_adaptor_warning("Element is NULL (service_context->context->info)");
		return SERVICE_ADAPTOR_INTERNAL_ERROR_INVALID_HANDLE;
	}
	auth_adaptor_plugin_h auth_plugin =
			auth_adaptor_get_plugin_by_name(service_adaptor->auth_handle, plugin_uri);
	auth_adaptor_error_code_h	auth_error	= NULL;

	int is_auth = -1;
	service_adaptor_debug("Call is_auth");
	ret = auth_adaptor_is_auth(auth_plugin, service_context->auth_context, NULL, &is_auth, &auth_error, NULL);

	if (0 == is_auth) {
		service_adaptor_error("Auth was withdrew (Can not use all service)");
		/*TODO change context_info's auth flag */
		return SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED;
	}
	auth_adaptor_destroy_error_code(&auth_error);

	service_adaptor_debug("Call auth_adaptor_login");
	ret = auth_adaptor_login(auth_plugin, service_context->auth_context, is_auth, NULL, &auth_error, NULL);

	if ((NULL != old_access_token) && (AUTH_ADAPTOR_ERROR_NONE == ret)) {
		if (NULL == service_context->auth_context->access_token) {
			service_adaptor_error("auth_plugin returns login success but auth_access token is empty");
			service_context->auth_context->access_token = strdup(old_access_token);
			ret = SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED;
		} else {
			if (0 == strcmp(old_access_token, service_context->auth_context->access_token)) {
				service_adaptor_debug("Call auth_adaptor_login_refresh");
				ret = auth_adaptor_login_refresh(auth_plugin, service_context->auth_context, NULL, &auth_error, NULL);
			}

			if ((AUTH_ADAPTOR_ERROR_NONE == ret) &&
					(NULL != service_context->auth_context->access_token) &&
					(0 == strcmp(old_access_token, service_context->auth_context->access_token))) {
				service_adaptor_debug("Access token was not changed");
			} else if (AUTH_ADAPTOR_ERROR_NONE == ret) {
				service_adaptor_debug("Changing access token start");

				new_access_token = auth_adaptor_get_access_token_dup(service_context->auth_context);
				if (NULL != new_access_token) {
					service_adaptor_debug_secure("New access token : %s", new_access_token);
					free(service_context->context_info->access_token);
					service_context->context_info->access_token = new_access_token;
					service_adaptor_debug("service_context->context_info->access_token was changed");

					ret = contact_adaptor_refresh_access_token(service_context->contact_context, new_access_token);
					if (!ret) {
						service_adaptor_debug("service_context->contact_context was changed");
					}

					ret = storage_adaptor_refresh_access_token(service_context->storage_context, new_access_token);
					if (!ret) {
						service_adaptor_debug("service_context->storage_context was changed");
					}

					ret = message_adaptor_refresh_access_token(service_context->message_context, new_access_token);
					if (!ret) {
						service_adaptor_debug("service_context->message_context was changed");
					}

					ret = shop_adaptor_refresh_access_token(service_context->shop_context, new_access_token);
					if (!ret) {
						service_adaptor_debug("service_context->shop_context was changed");
					}
				}

				new_uid = auth_adaptor_get_uid_dup(service_context->auth_context);
				if (NULL != new_uid) {
					service_adaptor_debug_secure("New unique id : %s", new_uid);
					free(service_context->context_info->duid);
					service_context->context_info->duid = new_uid;
					service_adaptor_debug("service_context->context_info->uid was changed");

					ret = contact_adaptor_refresh_uid(service_context->contact_context, new_uid);
					if (!ret) {
						service_adaptor_debug("service_context->contact_context was changed");
					}

					ret = storage_adaptor_refresh_uid(service_context->storage_context, new_uid);
					if (!ret) {
						service_adaptor_debug("service_context->storage_context was changed");
					}

					ret = message_adaptor_refresh_uid(service_context->message_context, new_uid);
					if (!ret) {
						service_adaptor_debug("service_context->message_context was changed");
					}

					ret = shop_adaptor_refresh_uid(service_context->shop_context, new_uid);
					if (!ret) {
						service_adaptor_debug("service_context->shop_context was changed");
					}
				}
			} else {
				service_adaptor_error("Login refresh failed");
				if (NULL != auth_error) {
					service_adaptor_error("Auth error code(%lld) message(%s)", auth_error->code, auth_error->msg);
				}
			}
		}
	} else {
		service_adaptor_error("Login refresh failed");
		if (NULL != auth_error) {
			service_adaptor_error("Auth error code(%lld) message(%s)", auth_error->code, auth_error->msg);
		}
	}

	service_adaptor_debug("Auth refresh End");
FUNC_END();
	return ret;
}


/**
 * @brief main function
 *
 * @param[in]	argc		specifies count of arguments
 * @param[in]	argv		specifies value list of arguments
 * @return	void.
 */
int main(int argc, char *argv[])
{
	int ret = SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
	GMainLoop *loop;

#if !GLIB_CHECK_VERSION(2, 32, 0)
	g_thread_init(NULL);
#endif
#if !GLIB_CHECK_VERSION(2, 35, 0)
	g_type_init();
#endif

	/* mainloop of main thread */
	loop = g_main_loop_new(NULL, FALSE);
	g_default_loop = loop;

	ret = service_adaptor_init();

	if (SERVICE_ADAPTOR_INTERNAL_ERROR_NONE != ret) {
		service_adaptor_error("Service Adaptor initialize error: %d\n", ret);
		return -1;
	}

	/* installing signal handlers */
	g_unix_signal_add_full(G_PRIORITY_HIGH, SIGINT,
			sigterm_callback, loop, NULL);
	g_unix_signal_add_full(G_PRIORITY_HIGH, SIGTERM,
			sigterm_callback, loop, NULL);

	/* start application's main loop */
	g_main_loop_run(loop);

	/* cleanup after mainloop */
	g_main_loop_unref(loop);

	service_adaptor_h service_adaptor = service_adaptor_get_handle();
	service_adaptor_deinit(service_adaptor);

	return ret;
}

#ifdef SERVICE_ADAPTOR_DEBUG_TIME_CHECK

#include <sys/time.h>
/******************* for debug func *************/

/* sa time */
static __thread long start_time = 0;
static __thread long now_time = 0;
static __thread long delayed_time = 0;

/* auth */
static __thread long a_start_time = 0;
static __thread long a_now_time = 0;
static __thread long a_delayed_time = 0;

/* storage */
static __thread long s_start_time = 0;
static __thread long s_now_time = 0;
static __thread long s_delayed_time = 0;

/* contact */
static __thread long c_start_time = 0;
static __thread long c_now_time = 0;
static __thread long c_delayed_time = 0;

/* message */
static __thread long m_start_time = 0;
static __thread long m_now_time = 0;
static __thread long m_delayed_time = 0;

/* push */
static __thread long p_start_time = 0;
static __thread long p_now_time = 0;
static __thread long p_delayed_time = 0;

/* shop */
static __thread long h_start_time = 0;
static __thread long h_now_time = 0;
static __thread long h_delayed_time = 0;

static __thread int a_count = 0;
static __thread int s_count = 0;
static __thread int c_count = 0;
static __thread int m_count = 0;
static __thread int p_count = 0;
static __thread int h_count = 0;

static __thread struct timeval tv;

#endif
void SERVICE_ADAPTOR_API_TIME_CHECK_START()
{
#ifdef SERVICE_ADAPTOR_DEBUG_TIME_CHECK
	gettimeofday(&tv, NULL);
	start_time = tv.tv_sec;
#endif
}
void SERVICE_ADAPTOR_API_TIME_CHECK_PAUSE()
{
#ifdef SERVICE_ADAPTOR_DEBUG_TIME_CHECK
	gettimeofday(&tv, NULL);
	now_time = tv.tv_sec;

	delayed_time += (now_time - start_time);
#endif
}

void SERVICE_ADAPTOR_PLUGIN_API_TIME_CHECK_START(sa_time_check_flag_e flag)
{
#ifdef SERVICE_ADAPTOR_DEBUG_TIME_CHECK
	if (SA_TIME_CHECK_FLAG_AUTH == flag) {
		gettimeofday(&tv, NULL);
		a_start_time = tv.tv_sec;
	}
	if (SA_TIME_CHECK_FLAG_STORAGE == flag) {
		gettimeofday(&tv, NULL);
		s_start_time = tv.tv_sec;
	}
	if (SA_TIME_CHECK_FLAG_CONTACT == flag) {
		gettimeofday(&tv, NULL);
		c_start_time = tv.tv_sec;
	}
	if (SA_TIME_CHECK_FLAG_MESSAGE == flag) {
		gettimeofday(&tv, NULL);
		m_start_time = tv.tv_sec;
	}
	if (SA_TIME_CHECK_FLAG_PUSH == flag) {
		gettimeofday(&tv, NULL);
		p_start_time = tv.tv_sec;
	}
	if (SA_TIME_CHECK_FLAG_SHOP == flag) {
		gettimeofday(&tv, NULL);
		h_start_time = tv.tv_sec;
	}
#endif
}
void SERVICE_ADAPTOR_PLUGIN_API_TIME_CHECK_PAUSE(sa_time_check_flag_e flag)
{
#ifdef SERVICE_ADAPTOR_DEBUG_TIME_CHECK
	if (SA_TIME_CHECK_FLAG_AUTH == flag) {
		gettimeofday(&tv, NULL);
		a_now_time = tv.tv_sec;

		a_delayed_time += (a_now_time - a_start_time);
		a_count++;
	}
	if (SA_TIME_CHECK_FLAG_STORAGE == flag) {
		gettimeofday(&tv, NULL);
		s_now_time = tv.tv_sec;

		s_delayed_time += (s_now_time - s_start_time);
		s_count++;
	}
	if (SA_TIME_CHECK_FLAG_CONTACT == flag) {
		gettimeofday(&tv, NULL);
		c_now_time = tv.tv_sec;

		c_delayed_time += (c_now_time - c_start_time);
		c_count++;
	}
	if (SA_TIME_CHECK_FLAG_MESSAGE == flag) {
		gettimeofday(&tv, NULL);
		m_now_time = tv.tv_sec;

		m_delayed_time += (m_now_time - m_start_time);
		m_count++;
	}
	if (SA_TIME_CHECK_FLAG_PUSH == flag) {
		gettimeofday(&tv, NULL);
		p_now_time = tv.tv_sec;

		p_delayed_time += (p_now_time - p_start_time);
		p_count++;
	}
	if (SA_TIME_CHECK_FLAG_SHOP == flag) {
		gettimeofday(&tv, NULL);
		h_now_time = tv.tv_sec;

		h_delayed_time += (h_now_time - h_start_time);
		h_count++;
	}
#endif
}
void SERVICE_ADAPTOR_API_TIME_CHECK_TOTAL_REPORT(const char *service_name)
{
#ifdef SERVICE_ADAPTOR_DEBUG_TIME_CHECK
	service_adaptor_debug_func("[TIMECHECK]================================================");
	service_adaptor_debug_func("[TIMECHECK]================================================");
	service_adaptor_debug_func("[TIMECHECK]     Total set_auth time report (TID : %lld)", (long long int)syscall(__NR_gettid));
	service_adaptor_debug_func("[TIMECHECK]     Service name : %s", service_name);
	service_adaptor_debug_func("[TIMECHECK] Total delay time : %ld sec",
			(delayed_time + a_delayed_time + s_delayed_time + c_delayed_time + m_delayed_time + p_delayed_time + h_delayed_time));
	service_adaptor_debug_func("[TIMECHECK] Adaptor : %ld sec", delayed_time);
	service_adaptor_debug_func("[TIMECHECK] Auth plugin : %ld sec, called : %d", a_delayed_time, a_count);
	service_adaptor_debug_func("[TIMECHECK] Storage plugin : %ld sec, called : %d", s_delayed_time, s_count);
	service_adaptor_debug_func("[TIMECHECK] Contact plugin : %ld sec, called : %d", c_delayed_time, c_count);
	service_adaptor_debug_func("[TIMECHECK] Message plugin : %ld sec, called : %d", m_delayed_time, m_count);
	service_adaptor_debug_func("[TIMECHECK] Push plugin : %ld sec, called : %d", p_delayed_time, p_count);
	service_adaptor_debug_func("[TIMECHECK] Shop plugin : %ld sec, called : %d", h_delayed_time, h_count);
	service_adaptor_debug_func("[TIMECHECK]================================================");
	service_adaptor_debug_func("[TIMECHECK]================================================");
#endif
}

