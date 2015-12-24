/*
 * Service Auth
 *
 * Copyright (c) 2014 - 2015 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this oauth1 except in compliance with the License.
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

#ifndef __SERVICE_AUTH_H__
#define __SERVICE_AUTH_H__

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C"
{
#endif
/*
#include "sal_service_adaptor.h"
#include "sal_service_task.h"

#define SERVICE_AUTH_OAUTH1_0_GET_ACCESS_TOKEN_URI	"http://tizen.org/service-adaptor/auth/oauth1.0/get_access_token"
#define SERVICE_AUTH_OAUTH1_0_GET_EXTRA_DATA_URI	"http://tizen.org/service-adaptor/auth/oauth1.0/get_extra_data"

#define SERVICE_AUTH_OAUTH2_0_GET_ACCESS_TOKEN_URI	"http://tizen.org/service-adaptor/auth/oauth2.0/get_access_token"
#define SERVICE_AUTH_OAUTH2_0_GET_EXTRA_DATA_URI	"http://tizen.org/service-adaptor/auth/oauth2.0/get_extra_data"

typedef struct _service_auth_oauth1_s *service_auth_oauth1_h;

typedef void (*service_auth_oauth1_cb)(int result, service_auth_oauth1_h oauth1, void *user_data);
typedef void (*service_auth_oauth1_get_access_token_cb)(int result, const char *access_token, void *user_data);
*/
/*==================================================================================================
                                         FUNCTION PROTOTYPES
==================================================================================================*/
/*
int service_auth_oauth1_create(service_plugin_h plugin, service_auth_oauth1_h *oauth1);
int service_auth_oauth1_clone(service_auth_oauth1_h src_oauth1, service_auth_oauth1_h *dst_oauth1);
int service_auth_oauth1_destroy(service_auth_oauth1_h oauth1);
int service_auth_oauth1_set_callback(service_auth_oauth1_h oauth1, service_auth_oauth1_cb callback, void *user_data);
int service_auth_oauth1_unset_callback(service_auth_oauth1_h oauth1);
int service_auth_oauth1_set_access_token(service_auth_oauth1_h oauth1, const char *access_token);
int service_auth_oauth1_get_access_token(service_auth_oauth1_h oauth1, char **access_token);
int service_auth_oauth1_set_operation(service_auth_oauth1_h oauth1, const char *operation);
int service_auth_oauth1_get_operation(service_auth_oauth1_h oauth1, char **operation);
int service_auth_oauth1_create_task(service_auth_oauth1_h oauth1, service_task_h *task);
int service_auth_oauth1_destroy_task(service_task_h task);
*/
#ifdef __cplusplus
}
#endif

#endif /* __SERVICE_AUTH_H__ */
