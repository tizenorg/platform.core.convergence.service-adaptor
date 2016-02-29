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

#ifndef __DBUS_SERVICE_ADAPTOR_H__
#define __DBUS_SERVICE_ADAPTOR_H__

#include <glib.h>
#include <gio/gio.h>

void service_adaptor_method_call(GDBusConnection *connection,
						const gchar *sender,
						const gchar *object_path,
						const gchar *interface_name,
						const gchar *method_name,
						GVariant *parameters,
						GDBusMethodInvocation *invocation,
						gpointer user_data);

service_adaptor_internal_error_code_e dbus_push_data_callback(uint32_t service_id,
						push_adaptor_notification_data_h app_data,
						service_adaptor_internal_error_h error_code,
						void *server_data);

service_adaptor_internal_error_code_e dbus_service_adaptor_signal_callback(service_adaptor_internal_signal_code_e signal_code,
						const char *signal_msg);

#endif /* __DBUS_SERVICE_ADAPTOR_H__ */
