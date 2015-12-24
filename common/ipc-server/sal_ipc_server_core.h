/*
 * Service Adaptor Server Core IPC
 *
 * Copyright (c) 2014 - 2015 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Yongjin Kim <youth.kim@samsung.com>
 *          Jinhyeong Ahn <jinh.ahn@samsung.com>
 *          Jiwon Kim <jiwon177.kim@samsung.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef __TIZEN_CONVERGENCE_SAL_IPC_SERVER_CORE_H__
#define __TIZEN_CONVERGENCE_SAL_IPC_SERVER_CORE_H__

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#include <glib.h>
#include <gio/gio.h>

typedef struct _base_method_req_s
{
	void (*connect_cb)(method_call_h method, int pid, const char *client_uri);
	void (*disconnect_cb)(int pid, const char *client_uri);
} base_method_req_s;

typedef struct _base_method_res_s
{
	void (*connect_success)(method_call_h method, GList *plugin_uris);
	void (*disconnect_success)(method_call_h method);

	void (*api_fail)(method_call_h method);
} base_method_req_s;

typedef struct _plugin_method_handle_s
{
	void (*start_cb)(const char *client_uri, const char *plugin_uri, int service_mask); /* TODO: add property param */
	void (*stop_cb)(const char *client_uri, const char *plugin_uri);
} plugin_method_handle_s;

/*
void service_adaptor_method_call(GDBusConnection *connection,
		const gchar *sender,
		const gchar *object_path,
		const gchar *interface_name,
		const gchar *method_name,
		GVariant *parameters,
		GDBusMethodInvocation *invocation,
		gpointer user_data);

void service_plugin_method_call(GDBusConnection *connection,
		const gchar *sender,
		const gchar *object_path,
		const gchar *interface_name,
		const gchar *method_name,
		GVariant *parameters,
		GDBusMethodInvocation *invocation,
		gpointer user_data);
*/
#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_CONVERGENCE_SAL_IPC_SERVER_CORE_H__ */

