/*
 * Service Adaptor Server Storage IPC
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

#ifndef __TIZEN_CONVERGENCE_SAL_IPC_PROVIDER_STORAGE_H__
#define __TIZEN_CONVERGENCE_SAL_IPC_PROVIDER_STORAGE_H__

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

typedef struct _ipc_provider_storage_req_s
{
	void (*download_cb)(ipc_provider_session_h session, const char *session_uri,
			const char *local_path, const char *cloud_path);
} ipc_provider_storage_req_s;

typedef struct _ipc_provider_storage_res_s
{
	void (*download)(ipc_provider_session_h session, int fd);

	void (*fail)(ipc_provider_session_h session, int result, int error_code, const char *message);
} ipc_provider_storage_res_s;

API int ipc_provider_storage_init(ipc_provider_storage_req_s *storage_req);

API ipc_provider_storage_res_s *ipc_provider_get_storage_res_handle(void);

ipc_provider_method_call_s ipc_provider_storage_method_call;

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_CONVERGENCE_SAL_IPC_PROVIDER_STORAGE_H__ */

