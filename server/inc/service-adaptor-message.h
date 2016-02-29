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

#ifndef __SERVICE_ADAPTOR_MESSAGE_H__
#define __SERVICE_ADAPTOR_MESSAGE_H__

#include "service-adaptor-type.h"
#include "message-adaptor.h"

message_adaptor_h service_adaptor_get_message_adaptor(service_adaptor_h service_adaptor);

service_adaptor_internal_error_code_e service_adaptor_connect_message_plugin(service_adaptor_h service_adaptor,
						service_adaptor_service_context_h service,
						char *ret_msg);

service_adaptor_internal_error_code_e service_adaptor_disconnect_message_plugin(service_adaptor_h service_adaptor,
						service_adaptor_service_context_h service);

service_adaptor_internal_error_code_e service_adaptor_message_connection_create(message_adaptor_plugin_h plugin,
						message_adaptor_plugin_context_h context,
						message_adaptor_error_code_h *error);

service_adaptor_internal_error_code_e service_adaptor_message_set_connection(message_adaptor_plugin_h plugin,
						message_adaptor_plugin_context_h message_context,
						message_connection_policy_e policy,
						message_adaptor_error_code_t **error_code);

message_adaptor_h service_adaptor_create_message();

message_adaptor_listener_h service_adaptor_register_message_listener(message_adaptor_h message_adaptor);

#endif /* __SERVICE_ADAPTOR_MESSAGE_ADAPTOR_H__ */
