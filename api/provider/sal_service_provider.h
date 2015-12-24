/*
 * Service Plugin Client
 *
 * Copyright (c) 2014 - 2015 Samsung Electronics Co., Ltd. All rights reserved.
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

#ifndef __SERVICE_PLUGIN_CLIENT_H__
#define __SERVICE_PLUGIN_CLIENT_H__

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#include <app.h>

#include "sal_auth_provider.h"
#include "sal_storage_provider.h"

#define PLUGIN_RESULT_KEY		"result"
#define PLUGIN_RESULT_VALUE_SUCCESS	"success"
#define PLUGIN_RESULT_VALUE_FAILURE	"failure"

#define PLUGIN_KEY_AUTH			"auth_plugin"
#define PLUGIN_KEY_STORAGE		"storage_plugin"
#define PLUGIN_VALUE_TRUE		"true"
#define PLUGIN_VALUE_FALSE		"false"

#define PLUGIN_CONNECT_URI		"connect"
#define PLUGIN_DISCONNECT_URI		"disconnect"

/**
 * @brief Describes infromation about Plugin Handle
 */
typedef struct _service_provider_s
{
	service_adaptor_error_e (*connect)(void);
	service_adaptor_error_e (*disconnect)(void);

	auth_provider_h auth_provider;
	storage_provider_h storage_provider;

	char *uri;
	char *name;
} service_provider_s;
typedef struct _service_provider_s *service_provider_h;

int service_provider_create(service_provider_h *provider);
int service_provider_destroy(service_provider_h provider);
int service_provider_set_auth_provider(service_provider_h provider, auth_provider_h auth_provider);
int service_provider_unset_auth_provider(service_provider_h provider);
int service_provider_set_storage_provider(service_provider_h provider, storage_provider_h storage_provider);
int service_provider_unset_storage_provider(service_provider_h provider);
int service_provider_message(service_provider_h provider, app_control_h app_control, void *user_data);

#ifdef __cplusplus
}
#endif

#endif /* __SERVICE_PLUGIN_CLIENT_H__ */
