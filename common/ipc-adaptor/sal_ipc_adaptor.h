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

#include "sal_ipc_adaptor_base.h"
#include "sal_ipc_adaptor_storage.h"

typedef void (*sal_ipc_adaptor_launch_cb)(bool is_alived, void *user_data);

API int sal_ipc_adaptor_get_list(GList **plugin_uris);

API int sal_ipc_adaptor_channel_open_with_provider(const char *plugin_uri, sal_ipc_adaptor_launch_cb callback);

API int sal_ipc_adaptor_channel_close_with_provider(const char *plugin_uri);

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_CONVERGENCE_SAL_IPC_CLIENT_H__ */

