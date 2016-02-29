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
#include "service-adaptor-shop.h"
#include "service-adaptor-type.h"
#include "service-adaptor-log.h"
#include "dbus-ipc.h"
#include "dbus-server.h"
#include "dbus-service-adaptor.h"
#include "dbus-shop-adaptor.h"
#include "shop-adaptor.h"

/*#define SHOP_PLUGIN_PATH	"/usr/lib/shop-adaptor/plugins"*/
#define APP_TYPE		"FM"

shop_adaptor_h service_adaptor_get_shop_adaptor(service_adaptor_h service_adaptor)
{
	service_adaptor_debug("Get shop adaptor");

	if ((void *) NULL == service_adaptor) {
		service_adaptor_error("Invalid argument");
		return NULL;
	}

	return service_adaptor->shop_handle;
}

service_adaptor_internal_error_code_e service_adaptor_connect_shop_plugin(service_adaptor_h service_adaptor,
						service_adaptor_service_context_h service,
						char *ret_msg)
{
	service_adaptor_debug("Connect to shop plugin");

	if ((NULL == service_adaptor) || (NULL == service)) {
		service_adaptor_error("Invalid parameter");
		snprintf(ret_msg, 2048, "shop plugin connect failed [%d]", (int)__LINE__);
		return SERVICE_ADAPTOR_INTERNAL_ERROR_INVALID_ARGUMENT;
	}

	shop_adaptor_h adaptor = service_adaptor_get_shop_adaptor(service_adaptor);
	shop_adaptor_plugin_h plugin = shop_adaptor_get_plugin_by_name(adaptor, service->plugin_uri);

	if (NULL == service->context_info) {
		service_adaptor_error("Invalid service->context_info");
		snprintf(ret_msg, 2048, "shop plugin connect failed [%d]", (int)__LINE__);
		return SERVICE_ADAPTOR_INTERNAL_ERROR_INVALID_ARGUMENT;
	} else if ((NULL == service->context_info->duid)
			|| (NULL == service->context_info->access_token)) {
		service_adaptor_error("Invalid duid or access_token");
		service_adaptor_debug_secure("Invalid duid or access_token: %s, %s",
				service->context_info->duid, service->context_info->access_token);
		snprintf(ret_msg, 2048, "shop plugin connect failed [%d]", (int)__LINE__);
		return SERVICE_ADAPTOR_INTERNAL_ERROR_INVALID_ARGUMENT;
	}

	shop_adaptor_plugin_context_h shop_context = shop_adaptor_create_plugin_context(
			plugin, service->plugin_uri, service->context_info->duid,
			service->context_info->access_token, service->context_info->app_id, APP_TYPE);

	if (NULL == shop_context) {
		service_adaptor_debug_func("Could not get shop plugin context");
		service_adaptor_debug_secure("Could not get shop plugin context: %s, %s",
				service->context_info->duid, service->context_info->access_token);
		snprintf(ret_msg, 2048, "shop plugin connect failed [%d]", (int)__LINE__);
		return SERVICE_ADAPTOR_INTERNAL_ERROR_CORRUPTED;
	}

	/* Set server info */
	int ret = 0;
	shop_adaptor_error_code_h error = NULL;
	SERVICE_ADAPTOR_API_TIME_CHECK_PAUSE();
	SERVICE_ADAPTOR_PLUGIN_API_TIME_CHECK_START(SA_TIME_CHECK_FLAG_SHOP);
	ret = shop_adaptor_set_server_info(plugin, shop_context, service->server_info, &error);
	SERVICE_ADAPTOR_PLUGIN_API_TIME_CHECK_PAUSE(SA_TIME_CHECK_FLAG_SHOP);
	SERVICE_ADAPTOR_API_TIME_CHECK_START();
	if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
		service_adaptor_warning("Could not set shop plugin server information: %d", ret);
		if (NULL != error) {
			service_adaptor_warning("[%lld] %s", error->code, error->msg);
		}
		shop_adaptor_destroy_error_code(&error);
	}

	service->shop_context = shop_context;
	service->connected |= 0x0001000;

	service_adaptor_debug("Connected to shop plugin");

	return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
}

service_adaptor_internal_error_code_e service_adaptor_disconnect_shop_plugin(service_adaptor_h service_adaptor,
						service_adaptor_service_context_h service)
{
	service_adaptor_debug("Disconnect from shop plugin");

	service_adaptor_debug("get shop adaptor");
	shop_adaptor_h shop_adaptor = service_adaptor_get_shop_adaptor(service_adaptor);
	if ((NULL != service->shop_context) && (NULL != shop_adaptor)) {
		service_adaptor_debug("disconnects shop");
		FUNC_STEP();
		shop_adaptor_plugin_h shop_plugin = shop_adaptor_get_plugin_by_name(shop_adaptor, service->shop_context->plugin_uri);

		if (NULL == shop_plugin) {
			service_adaptor_error("Cannot find plugin");
		} else {
			service_adaptor_debug("dsetroys shop context");
			shop_adaptor_destroy_plugin_context(shop_plugin, service->shop_context);
			service->shop_context = NULL;
		}
	}

	service_adaptor_debug("Disconnected from shop plugin");

	return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
}

shop_adaptor_h service_adaptor_create_shop()
{
	shop_adaptor_h shop_adaptor = shop_adaptor_create(SHOP_PLUGIN_PATH);

	if (NULL == shop_adaptor) {
		service_adaptor_error("Could not create shop adaptor");
		return NULL;
	}

	service_adaptor_debug("Shop adaptor created");

	return shop_adaptor;
}

shop_adaptor_listener_h service_adaptor_register_shop_listener(shop_adaptor_h shop_adaptor)
{
	if (NULL == shop_adaptor) {
		service_adaptor_error("Could not create shop adaptor");
		return NULL;
	}

	shop_adaptor_listener_h shop_listener =
			(shop_adaptor_listener_h) malloc(sizeof(shop_adaptor_listener_t));

	return shop_listener;
}
