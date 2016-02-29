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
#include "dbus-contact-adaptor.h"
#include "contact-adaptor.h"

/*#define CONTACT_PLUGIN_PATH	"/usr/lib/contact-adaptor/plugins"*/

contact_adaptor_h service_adaptor_get_contact_adaptor(service_adaptor_h service_adaptor)
{
	service_adaptor_debug("Get contact adaptor");

	if ((void *) NULL == service_adaptor) {
		service_adaptor_error("Invalid argument");
		return NULL;
	}

	return service_adaptor->contact_handle;
}

service_adaptor_internal_error_code_e service_adaptor_connect_contact_plugin(service_adaptor_h service_adaptor,
						service_adaptor_service_context_h service,
						char *ret_msg)
{
	service_adaptor_debug("Connect to contact plugin");

	if ((NULL == service_adaptor) || (NULL == service)) {
		service_adaptor_error("Invalid parameter");
		snprintf(ret_msg, 2048, "contact plugin connect failed [%d]", (int)__LINE__);
		return SERVICE_ADAPTOR_INTERNAL_ERROR_INVALID_ARGUMENT;
	}

	contact_adaptor_h adaptor = service_adaptor_get_contact_adaptor(service_adaptor);
	contact_adaptor_plugin_h plugin = contact_adaptor_get_plugin_by_name(adaptor, service->plugin_uri);

	if (NULL == service->context_info) {
		service_adaptor_error("Invalid service->context_info");
		snprintf(ret_msg, 2048, "contact plugin connect failed [%d]", (int)__LINE__);
		return SERVICE_ADAPTOR_INTERNAL_ERROR_INVALID_ARGUMENT;
	} else if ((NULL == service->context_info->duid)
			|| (NULL == service->context_info->access_token)) {
		service_adaptor_error("Invalid duid or access_token");
		service_adaptor_debug_secure("Invalid duid or access_token: %s, %s",
				service->context_info->duid, service->context_info->access_token);
		snprintf(ret_msg, 2048, "contact plugin connect failed [%d]", (int)__LINE__);
		return SERVICE_ADAPTOR_INTERNAL_ERROR_INVALID_ARGUMENT;
	}

	contact_adaptor_plugin_context_h contact_context = contact_adaptor_create_plugin_context(plugin,
			service->context_info->duid, service->context_info->access_token, service->service_name);

	if (NULL == contact_context) {
		service_adaptor_debug_func("Could not get contact plugin context: %s, %s",
				service->context_info->duid, service->context_info->access_token);
		snprintf(ret_msg, 2048, "contact plugin connect failed [%d]", (int)__LINE__);
		return SERVICE_ADAPTOR_INTERNAL_ERROR_CORRUPTED;
	}

	/* Set server info */
	int ret = 0;
	contact_adaptor_error_code_h error = NULL;
	SERVICE_ADAPTOR_API_TIME_CHECK_PAUSE();
	SERVICE_ADAPTOR_PLUGIN_API_TIME_CHECK_START(SA_TIME_CHECK_FLAG_CONTACT);
	ret = contact_adaptor_set_server_info(plugin, contact_context, service->server_info, NULL, &error, NULL);
	SERVICE_ADAPTOR_PLUGIN_API_TIME_CHECK_PAUSE(SA_TIME_CHECK_FLAG_CONTACT);
	SERVICE_ADAPTOR_API_TIME_CHECK_START();
	if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
		service_adaptor_warning("Could not set contact plugin server information: %d", ret);
		if (NULL != error) {
			service_adaptor_warning("[%lld] %s", error->code, error->msg);
		}
		contact_adaptor_destroy_error_code(&error);
	}

	service->contact_context = contact_context;
	service->connected |= 0x0000010;

	service_adaptor_debug("Connected to contact plugin");

	return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
}

service_adaptor_internal_error_code_e service_adaptor_disconnect_contact_plugin(service_adaptor_h service_adaptor,
						service_adaptor_service_context_h service)
{
	service_adaptor_debug("Disconnect from contact plugin");

	service_adaptor_debug("get contact adaptor");
	contact_adaptor_h contact_adaptor = service_adaptor_get_contact_adaptor(service_adaptor);
	if ((NULL != service->contact_context) && (NULL != contact_adaptor)) {
		service_adaptor_debug("disconnects contact");
		FUNC_STEP();
		contact_adaptor_plugin_h contact_plugin = contact_adaptor_get_plugin_by_name(contact_adaptor, service->contact_context->plugin_uri);

		if (NULL == contact_plugin) {
			service_adaptor_error("Cannot find plugin");
		} else {
			service_adaptor_debug("destroys contact context");
			contact_adaptor_destroy_plugin_context(contact_plugin, service->contact_context);
			service->contact_context = NULL;
		}
	}

	service_adaptor_debug("Disconnected from contact plugin");

	return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
}

contact_adaptor_h service_adaptor_create_contact()
{
	contact_adaptor_h contact_adaptor = contact_adaptor_create(CONTACT_PLUGIN_PATH);

	if (NULL == contact_adaptor) {
		service_adaptor_error("Could not create contact adaptor");
		return NULL;
	}

	service_adaptor_debug("Contact adaptor created");

	return contact_adaptor;
}

contact_adaptor_listener_h service_adaptor_register_contact_listener(contact_adaptor_h contact_adaptor)
{
	if ((void *) NULL == contact_adaptor) {
		service_adaptor_error("Could not create contact adaptor");
		return NULL;
	}

	contact_adaptor_listener_h contact_listener =
			(contact_adaptor_listener_h) malloc(sizeof(contact_adaptor_listener_t));

	return contact_listener;
}
