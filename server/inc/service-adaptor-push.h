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

#ifndef __SERVICE_ADAPTOR_PUSH_H__
#define __SERVICE_ADAPTOR_PUSH_H__

#include "service-adaptor-type.h"
#include "push-adaptor.h"


typedef struct _push_activate_s
{
	/* service_file_name (unique) */
	char *file_name;

	/* push info */
	char *plugin_uri;
	char *app_id;
	char *session_info;

	/* bus info */
	int bus_type;
	char *bus_name;
	char *object_path;
	char *interface;
	char *method;
	void *proxy;

	/* general */
	char *exec;
} push_activate_t;
typedef push_activate_t *push_activate_h;


push_adaptor_h service_adaptor_get_push_adaptor(service_adaptor_h service_adaptor);

push_adaptor_plugin_context_h service_adaptor_get_push_context(service_adaptor_h service_adaptor,
						const char *imsi,
						const char *app_id);

service_adaptor_internal_error_code_e service_adaptor_connect_push_plugin(service_adaptor_h service_adaptor,
						service_adaptor_service_context_h service,
						char *ret_msg);

service_adaptor_internal_error_code_e service_adaptor_disconnect_push_plugin(service_adaptor_h service_adaptor,
						service_adaptor_service_context_h service);

service_adaptor_internal_error_code_e service_adaptor_reconnect_push_plugin(service_adaptor_h service_adaptor,
						service_adaptor_service_context_h service);

push_adaptor_h service_adaptor_create_push();

push_adaptor_listener_h service_adaptor_register_push_listener(push_adaptor_h push_adaptor);

service_adaptor_internal_error_code_e service_adaptor_push_register(const char *service_file, char **error_msg);

service_adaptor_internal_error_code_e service_adaptor_push_deregister(const char *service_file, char **error_msg);

service_adaptor_internal_error_code_e service_adaptor_ref_enabled_push_services(push_activate_h **services, int *services_len);

#endif /* __SERVICE_ADAPTOR_PUSH_H__ */
