/*
 * Service Adaptor Client IPC
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

#ifndef __TIZEN_CONVERGENCE_SAL_IPC_CLIENT_H__
#define __TIZEN_CONVERGENCE_SAL_IPC_CLIENT_H__

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#include <glib.h>
#include <gio/gio.h>

#include "sal_ipc_client_core.h"
#include "sal_ipc_client_auth.h"
#include "sal_ipc_client_storage.h"

int sal_ipc_client_init(void);

int sal_ipc_client_deinit(void);

int sal_ipc_client_call_request(const char *request_method, GVariant *request_data, const char *reply_type, GVariant **reply_info);

int sal_ipc_client_get_simple_response(GVariant *response);

int sal_ipc_client_get_data_response(GVariant *response, GVariant **data);


API void sal_ipc_client_init_last_error(void);

API void sal_ipc_client_set_last_error(int error, const char *message);

API int sal_ipc_client_get_last_error(void);

API char *sal_ipc_client_get_last_message(void);

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_CONVERGENCE_SAL_IPC_CLIENT_H__ */

