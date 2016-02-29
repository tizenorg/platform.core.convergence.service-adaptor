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

#include "service-adaptor.h"
#include "service-adaptor-auth.h"
#include "service-adaptor-type.h"
#include "service-adaptor-log.h"
#include "dbus-ipc.h"
#include "dbus-server.h"
#include "dbus-service-adaptor.h"
#include "dbus-auth-adaptor.h"
#include "auth-adaptor.h"

/*#define AUTH_PLUGIN_PATH	"/usr/lib/auth-adaptor/plugins"*/
#define MAX_RETRY		2

auth_adaptor_h service_adaptor_get_auth_adaptor(service_adaptor_h service_adaptor)
{
	service_adaptor_debug("Get auth adaptor");

	if ((void *) NULL == service_adaptor) {
		service_adaptor_error("Invalid argument");
		return NULL;
	}

	return service_adaptor->auth_handle;
}

auth_adaptor_plugin_context_h service_adaptor_get_auth_context(service_adaptor_h service_adaptor,
						const char* imsi,
						const char *app_id)
{
	service_adaptor_debug("Get auth context");

	service_adaptor_warning("Could not get auth context with IMSI(%s) and AppID(%s)", imsi, app_id);

	return NULL;
}

service_adaptor_internal_error_code_e service_adaptor_connect_auth_plugin(service_adaptor_h service_adaptor,
						service_adaptor_context_info_s *context_info,
						const char *service_name,
						const char *plugin_uri,
						const char *user_password,
						const char *app_secret,
						service_adaptor_service_context_h *service,
						char *ret_msg)
{
	service_adaptor_info("Connect to auth plugin (%s)", service_name);

	int ret = SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;

	if ((NULL == service_adaptor) || (NULL == context_info) || (NULL == plugin_uri) || (NULL == service)) {
		service_adaptor_error("Invalid parameter");
		snprintf(ret_msg, 2048, "auth plugin connect failed [%d]", (int)__LINE__);
		return SERVICE_ADAPTOR_INTERNAL_ERROR_INVALID_ARGUMENT;
	}

	auth_adaptor_h adaptor = service_adaptor_get_auth_adaptor(service_adaptor);

	if (NULL == adaptor) {
		service_adaptor_error("Could not get auth adaptor");
		snprintf(ret_msg, 2048, "auth plugin connect failed [%d]", (int)__LINE__);
		return SERVICE_ADAPTOR_INTERNAL_ERROR_INVALID_ARGUMENT;
	}

	auth_adaptor_plugin_h plugin = auth_adaptor_get_plugin_by_name(adaptor, plugin_uri);

	if (NULL == plugin) {
		service_adaptor_error("Could not get auth plugin handle by plugin name (%s)", plugin_uri);
		snprintf(ret_msg, 2048, "auth plugin connect failed [%d]", (int)__LINE__);
		return SERVICE_ADAPTOR_INTERNAL_ERROR_INVALID_ARGUMENT;
	}

	auth_adaptor_plugin_context_h auth_context = auth_adaptor_create_plugin_context(plugin,
			context_info->user_id, user_password, context_info->app_id, app_secret,
			context_info->imsi, service_name);

	if (NULL == auth_context) {
		service_adaptor_error("Could not create auth plugin context (service = %s)(server = %s)", service_name, plugin_uri);
		snprintf(ret_msg, 2048, "auth plugin connect failed [%d]", (int)__LINE__);
		return SERVICE_ADAPTOR_INTERNAL_ERROR_CREATE;
	}

	int is_auth = 1;
	/*int retry = 0;*/
	auth_adaptor_error_code_h error_code = NULL;

	/* 1) Login to auth plugin */
	service_adaptor_debug("Try to login to auth plugin (%s)", context_info->app_id);

	SERVICE_ADAPTOR_API_TIME_CHECK_PAUSE();
	SERVICE_ADAPTOR_PLUGIN_API_TIME_CHECK_START(SA_TIME_CHECK_FLAG_AUTH);
	ret = auth_adaptor_login(plugin, auth_context, is_auth, (void *)context_info->property, &error_code, NULL);
	SERVICE_ADAPTOR_PLUGIN_API_TIME_CHECK_PAUSE(SA_TIME_CHECK_FLAG_AUTH);
	SERVICE_ADAPTOR_API_TIME_CHECK_START();

	service_adaptor_info("LOGIN: %d", ret);
	if (AUTH_ADAPTOR_ERROR_NONE == ret) {
		service_adaptor_debug_func("============= Login Successed ===============");
		service_adaptor_debug_func("============= Auth context : %p ==========", auth_context);
		if (NULL != auth_context) {
			service_adaptor_debug_secure("============= Access token : %s ==========", auth_context->access_token);
			service_adaptor_debug_secure("============= Unique id : %s ==========", auth_context->uid);
		}
	} else {
		service_adaptor_debug_func("============= Login Failed ===============");
		auth_adaptor_destroy_plugin_context(plugin, auth_context);
		auth_context = NULL;
		if (NULL != error_code) {
			ret = (int)error_code->code;
			if (NULL != error_code->msg) {
				service_adaptor_error("[%lld] %s", error_code->code, error_code->msg);
				snprintf(ret_msg, 2048, "auth plugin login failed [%lld][%s]", (long long int)error_code->code, error_code->msg);
			} else {
				service_adaptor_error("Unexpected error occurred (error code is empty)");
				snprintf(ret_msg, 2048, "auth plugin connect failed [%d]", (int)__LINE__);
			}
			auth_adaptor_destroy_error_code(&error_code);
		} else {
			ret = SERVICE_ADAPTOR_PROTOCOL_RETURN_CODE_AUTH_NOT_AUTHORIZED;
		}
		return ret;
	}
	auth_adaptor_destroy_error_code(&error_code);

	/* Get server info */
	GHashTable *server_info = NULL;
	ret = auth_adaptor_get_server_info(plugin, auth_context, NULL, &server_info, &error_code, NULL);
	if (ret) {
		service_adaptor_warning("Could not get server info from auth plugin: %d", ret);
		if (NULL != error_code) {
			service_adaptor_warning("[%lld] %s", error_code->code, error_code->msg);
		}
		auth_adaptor_destroy_error_code(&error_code);
	}

	(*service)->context_info->duid = auth_adaptor_get_uid_dup(auth_context);
	(*service)->context_info->msisdn = auth_adaptor_get_msisdn_dup(auth_context);
	(*service)->context_info->access_token = auth_adaptor_get_access_token_dup(auth_context);

	(*service)->auth_context = auth_context;
	(*service)->connected |= 0x0000001;
	(*service)->server_info = server_info;

	service_adaptor_debug("Connected to auth plugin");

	return ret;
}

service_adaptor_internal_error_code_e service_adaptor_disconnect_auth_plugin(service_adaptor_h service_adaptor,
						service_adaptor_service_context_h service)
{
	service_adaptor_debug("Disconnect from auth plugin");

	service_adaptor_debug("get auth adaptor");
	auth_adaptor_h auth_adaptor = service_adaptor_get_auth_adaptor(service_adaptor);
	if ((NULL != service->auth_context) && (NULL != auth_adaptor)) {
		service_adaptor_debug("disconnects auth");
		FUNC_STEP();
		auth_adaptor_plugin_h auth_plugin = auth_adaptor_get_plugin_by_name(auth_adaptor, service->auth_context->plugin_uri);

		if (NULL == auth_plugin) {
			service_adaptor_error("Cannot find plugin");
		} else {
			service_adaptor_debug("dsetroys auth context");
			auth_adaptor_destroy_plugin_context(auth_plugin, service->auth_context);
			service->auth_context = NULL;
		}
	}

	service_adaptor_debug("Disconnected from auth plugin");

	return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
}

auth_adaptor_h service_adaptor_create_auth()
{
	auth_adaptor_h auth_adaptor = auth_adaptor_create(AUTH_PLUGIN_PATH);

	if ((void *) NULL == auth_adaptor) {
		service_adaptor_error("Could not create auth adaptor");
		return NULL;
	}

	service_adaptor_debug("Auth adaptor created");

	return auth_adaptor;
}

auth_adaptor_listener_h service_adaptor_register_auth_listener(auth_adaptor_h auth_adaptor)
{
	if ((void *) NULL == auth_adaptor) {
		service_adaptor_error("Could not create auth adaptor");
		return NULL;
	}

	auth_adaptor_listener_h auth_listener =
			(auth_adaptor_listener_h) malloc(sizeof(auth_adaptor_listener_t));

	if ((void *) NULL == auth_listener) {
		service_adaptor_error("Could not create auth listener");
		return NULL;
	}

	service_adaptor_debug("Auth adaptor listener created");

	return auth_listener;
}
