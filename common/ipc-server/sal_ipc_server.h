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

#ifndef __TIZEN_CONVERGENCE_SAL_IPC_SERVER_H__
#define __TIZEN_CONVERGENCE_SAL_IPC_SERVER_H__

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#include <glib.h>

#include "sal_ipc_server_types.h"
#include "sal_ipc_server_core.h"
#include "sal_ipc_server_auth.h"
#include "sal_ipc_server_storage.h"

int sal_ipc_server_init(GMainContext *working_context,
		ipc_server_base_req_s *base_method,
		ipc_server_auth_req_s *auth_method,
		ipc_server_storage_req_s *storage_method);

int sal_ipc_server_deinit(void);

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_CONVERGENCE_SAL_IPC_SERVER_H__ */

