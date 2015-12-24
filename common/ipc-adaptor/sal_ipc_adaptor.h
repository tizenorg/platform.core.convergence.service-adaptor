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

#ifndef __TIZEN_CONVERGENCE_SAL_IPC_ADAPTOR_H__
#define __TIZEN_CONVERGENCE_SAL_IPC_ADAPTOR_H__

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#include <glib.h>
#include <gio/gio.h>

#include "sal_ipc_adaptor_types.h"

typedef void (*sal_ipc_adaptor_launch_cb)(sal_ipc_adaptor_h adaptor, bool is_alived, void *user_data);

API int sal_ipc_adaptor_create(const char *plugin_uri, GMainContext *context, sal_ipc_adaptor_h *adaptor);

API int sal_ipc_adaptor_destroy(sal_ipc_adaptor_h adaptor);

API int sal_ipc_adaptor_channel_open_with_provider(sal_ipc_adaptor_h adaptor, sal_ipc_adaptor_launch_cb callback, void *user_data);

API int sal_ipc_adaptor_channel_close_with_provider(sal_ipc_adaptor_h adaptor);


#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_CONVERGENCE_SAL_IPC_ADAPTOR_H__ */

