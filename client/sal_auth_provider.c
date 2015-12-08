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

#include <stdio.h>
#include <string.h>
#include <glib.h>

#include <app.h>

#include "service_adaptor_errors.h"
#include "service_adaptor_internal.h"
#include "sal_service_provider.h"
#include "sal_auth_provider.h"

API int auth_provider_create(auth_provider_h *provider)
{
	SAL_FN_CALL;

	auth_provider_h auth_provider = (auth_provider_h) g_malloc0(sizeof(auth_provider_s));

	auth_provider->oauth1_get_access_token = NULL;
	auth_provider->oauth1_get_extra_data = NULL;

	auth_provider->oauth2_get_access_token = NULL;
	auth_provider->oauth2_get_extra_data = NULL;

	*provider = auth_provider;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API app_control_h auth_provider_message(auth_provider_h provider, const char *operation, void *user_data)
{
	SAL_FN_CALL;

	app_control_h reply = NULL;

	RETV_IF(NULL == provider, reply);

	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	if (0 == strcmp(operation, OAUTH1_0_GET_ACCESS_TOKEN_URI)) {
		app_control_create(&reply);

		char *access_token = NULL;
		ret = provider->oauth1_get_access_token(&access_token);

		if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
			SAL_ERR("oauth1_get_access_token() Fail (%d)", ret);
			app_control_add_extra_data(reply, PLUGIN_RESULT_KEY, PLUGIN_RESULT_VALUE_FAILURE);
			return reply;
		}

		app_control_add_extra_data(reply, PLUGIN_RESULT_KEY, PLUGIN_RESULT_VALUE_SUCCESS);
		app_control_add_extra_data(reply, OAUTH1_0_ACCESS_TOKEN_KEY, access_token);
	}

	return reply;
}

API int auth_provider_add_extra_data(auth_provider_h provider, app_control_h reply)
{
	SAL_FN_CALL;

	RETV_IF(NULL == provider, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == reply, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	app_control_add_extra_data(reply, PLUGIN_KEY_AUTH, PLUGIN_VALUE_TRUE);

	if (NULL != provider->oauth1_get_access_token) {
		app_control_add_extra_data(reply, OAUTH1_0_GET_ACCESS_TOKEN_URI, PLUGIN_VALUE_TRUE);
	}

	if (NULL != provider->oauth1_get_extra_data) {
		app_control_add_extra_data(reply, OAUTH1_0_GET_EXTRA_DATA_URI, PLUGIN_VALUE_TRUE);
	}

	if (NULL != provider->oauth2_get_access_token) {
		app_control_add_extra_data(reply, OAUTH2_0_GET_ACCESS_TOKEN_URI, PLUGIN_VALUE_TRUE);
	}

	if (NULL != provider->oauth2_get_extra_data) {
		app_control_add_extra_data(reply, OAUTH2_0_GET_EXTRA_DATA_URI, PLUGIN_VALUE_TRUE);
	}

	return SERVICE_ADAPTOR_ERROR_NONE;
}
