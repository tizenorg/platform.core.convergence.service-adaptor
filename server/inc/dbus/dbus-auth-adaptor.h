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

#ifndef __DBUS_AUTH_ADAPTOR_H__
#define __DBUS_AUTH_ADAPTOR_H__

#include <glib.h>
#include <gio/gio.h>

void auth_adaptor_method_call(GDBusConnection *connection,
						const gchar *sender,
						const gchar *object_path,
						const gchar *interface_name,
						const gchar *method_name,
						GVariant *parameters,
						GDBusMethodInvocation *invocation,
						gpointer user_data);

void auth_external_method_call(const char *service_name,
						const char *api_uri,
						const unsigned char *req_data,
						int req_len,
						unsigned char **res_data,
						int *res_len,
						int *ret_code,
						char *ret_msg);

#endif /* __DBUS_AUTH_ADAPTOR_H__ */
