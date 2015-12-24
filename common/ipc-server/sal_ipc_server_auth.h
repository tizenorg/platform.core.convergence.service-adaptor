/*
 * Service Adaptor Server Auth IPC
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

#ifndef __TIZEN_CONVERGENCE_SAL_IPC_SERVER_AUTH_H__
#define __TIZEN_CONVERGENCE_SAL_IPC_SERVER_AUTH_H__

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#include <glib.h>

#include "sal_ipc_server_types.h"

typedef struct _ipc_server_auth_req_s
{
	void (*chunk_cb)(ipc_server_session_h session);
} ipc_server_auth_req_s;

typedef struct _ipc_server_auth_res_s
{
	void (*chunk)(ipc_server_session_h session);

	void (*fail)(ipc_server_session_h session, int result, int error_code, const char *message);
} ipc_server_auth_res_s;

API int ipc_server_auth_init(ipc_server_auth_req_s *auth_req);

API gboolean sal_server_auth_method_call(void *data);

API ipc_server_auth_res_s *ipc_server_get_auth_res_handle(void);

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_CONVERGENCE_SAL_IPC_SERVER_AUTH_H__ */

