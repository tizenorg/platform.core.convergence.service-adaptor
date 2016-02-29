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

#ifndef __DBUS_PUSH_ADAPTOR_H__
#define __DBUS_PUSH_ADAPTOR_H__

#include <glib.h>
#include <gio/gio.h>

void push_adaptor_method_call(GDBusConnection *connection,
						const gchar *sender,
						const gchar *object_path,
						const gchar *interface_name,
						const gchar *method_name,
						GVariant *parameters,
						GDBusMethodInvocation *invocation,
						gpointer user_data);

void dbus_send_to_push_with_activation(int bus_type,
                                                const char *bus_name,
                                                const char *object_path,
                                                const char *interface,
                                                const char *method,
                                                void **proxy,
                                                long long int timestamp,
                                                const char *data,
                                                const char *message);

#endif /* __DBUS_PUSH_ADAPTOR_H__ */

