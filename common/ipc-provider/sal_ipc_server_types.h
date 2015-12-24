/*
 * Service Adaptor Server IPC
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

#ifndef __TIZEN_CONVERGENCE_SAL_IPC_SERVER_TYPES_H__
#define __TIZEN_CONVERGENCE_SAL_IPC_SERVER_TYPES_H__

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#include <glib.h>

/**
 * information for method call
 */
typedef struct _ipc_server_session_s {
	GDBusConnection *connection;
	gchar *sender;
	gchar *object_path;
	gchar *interface_name;
	gchar *method_name;
	GVariant *parameters;
	GDBusMethodInvocation *invocation;
	gpointer user_data;
} ipc_server_session_s;

typedef struct _ipc_server_session_s *ipc_server_session_h;

#define SAL_IPC_PAYLOAD_SKIP		0,0,""

#define SAL_SIZE_OF_TAB(arr)	(int)(sizeof(arr) / sizeof(arr[0]))

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_CONVERGENCE_SAL_IPC_SERVER_TYPES_H__ */

