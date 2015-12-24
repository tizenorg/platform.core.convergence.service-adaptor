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

#ifndef __TIZEN_CONVERGENCE_SAL_IPC_PROVIDER_BASE_H__
#define __TIZEN_CONVERGENCE_SAL_IPC_PROVIDER_BASE_H__

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#include <glib.h>
#include <gio/gio.h>

#include "sal_ipc_provider_types.h"

/*
 *	request handle type
 */
typedef struct _ipc_provider_base_req_s
{
	void (*session_start_cb)(ipc_provider_session_h session, const char *client_uri);

	void (*session_stop_cb)(ipc_provider_session_h session, const char *session_uri);

} ipc_provider_base_req_s;

/*
 *	response API handle
 */
typedef struct _ipc_provider_base_res_s
{
	void (*session_start)(ipc_provider_session_h session, const char *session_uri);

	void (*session_stop)(ipc_provider_session_h session);

	void (*fail)(ipc_provider_session_h session, int result, int error_code, const char *message);

} ipc_provider_base_res_s;

/*
 *	response API handle
 */
typedef struct _ipc_provider_base_signal_s
{
	void (*channel_opened_signal)(void);

	void (*channel_closed_signal)(void);

} ipc_provider_base_signal_s;

API int ipc_provider_base_init(ipc_provider_base_req_s *base_req);

API ipc_provider_base_res_s *ipc_provider_get_base_res_handle(void);

API ipc_provider_base_signal_s *ipc_provider_get_base_signal_handle(void);

static ipc_provider_method_call_s ipc_provider_base_method_call;

static ipc_provider_method_call_s ipc_provider_base_method_call;

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_CONVERGENCE_SAL_IPC_PROVIDER_BASE_H__ */

