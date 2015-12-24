/*
 * Auth Plugin Client
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

#ifndef __AUTH_PLUGIN_CLIENT_H__
#define __AUTH_PLUGIN_CLIENT_H__

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#include <app.h>

#include "service_adaptor_errors.h"

#define OAUTH1_0_ACCESS_TOKEN_KEY	"access_token"

#define OAUTH1_0_GET_ACCESS_TOKEN_URI	"http://tizen.org/service-adaptor/auth/oauth1.0/get_access_token"
#define OAUTH1_0_GET_EXTRA_DATA_URI	"http://tizen.org/service-adaptor/auth/oauth1.0/get_extra_data"

#define OAUTH2_0_GET_ACCESS_TOKEN_URI	"http://tizen.org/service-adaptor/auth/oauth2.0/get_access_token"
#define OAUTH2_0_GET_EXTRA_DATA_URI	"http://tizen.org/service-adaptor/auth/oauth2.0/get_extra_data"

/**
 * @brief Describes infromation about Auth Plugin Handle
 */
typedef struct _auth_provider_s
{
	// oAuth 1.0
	service_adaptor_error_e (*oauth1_get_access_token)(char **access_token);
	service_adaptor_error_e (*oauth1_get_extra_data)(const char *key, char **value);

	// oAuth 2.0
	service_adaptor_error_e (*oauth2_get_access_token)(char **access_token);
	service_adaptor_error_e (*oauth2_get_extra_data)(const char *key, char **value);
} auth_provider_s;
typedef struct _auth_provider_s *auth_provider_h;

int auth_provider_create(auth_provider_h *provider);
app_control_h auth_provider_message(auth_provider_h provider, const char *operation, void *user_data);
int auth_provider_add_extra_data(auth_provider_h provider, app_control_h reply);

#ifdef __cplusplus
}
#endif

#endif /* __AUTH_PLUGIN_CLIENT_H__ */
