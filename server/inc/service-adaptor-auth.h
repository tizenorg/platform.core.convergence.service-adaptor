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

#ifndef __SERVICE_ADAPTOR_AUTH_H__
#define __SERVICE_ADAPTOR_AUTH_H__

#include "service-adaptor-type.h"
#include "auth-adaptor.h"

auth_adaptor_h service_adaptor_get_auth_adaptor(service_adaptor_h service_adaptor);

auth_adaptor_plugin_context_h service_adaptor_get_auth_context(service_adaptor_h service_adaptor,
						const char* imsi,
						const char *app_id);

service_adaptor_internal_error_code_e service_adaptor_connect_auth_plugin(service_adaptor_h service_adaptor,
						service_adaptor_context_info_s *context_info,
						const char *service_name,
						const char *plugin_uri,
						const char *user_password,
						const char *app_secret,
						service_adaptor_service_context_h *service,
						char *ret_msg);

service_adaptor_internal_error_code_e service_adaptor_disconnect_auth_plugin(service_adaptor_h service_adaptor,
						service_adaptor_service_context_h service);

auth_adaptor_h service_adaptor_create_auth();

auth_adaptor_listener_h service_adaptor_register_auth_listener(auth_adaptor_h auth_adaptor);

#endif /* __SERVICE_ADAPTOR_AUTH_H__ */
