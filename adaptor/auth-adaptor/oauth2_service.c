/*
 * oAuth 2.0 Service
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
#include <glib.h>

#include <app.h>

#include "oauth2_service.h"
#include "auth_adaptor.h"
#include "service_adaptor_errors.h"
#include "service_adaptor_internal.h"

#include "sal_auth_provider.h"

//******************************************************************************
//* Global variables and defines
//******************************************************************************

typedef struct _app_control_user_data_s
{
	void *callback;
	void *user_data;
} app_control_user_data_s;
typedef struct _app_control_user_data_s *app_control_user_data_h;

typedef int (*_get_access_token)(void *plugin, char **access_token, void *user_data);
typedef int (*_get_extra_data)(void *plugin, const char *key, char **value, void *user_data);

//******************************************************************************
//* Private interface
//******************************************************************************

//******************************************************************************
//* Private interface definition
//******************************************************************************

static int _oauth2_get_access_token(auth_plugin_h plugin, char **access_token, void *user_data)
{
	SAL_FN_CALL;

	RETV_IF(NULL == plugin, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

static int _oauth2_get_extra_data(auth_plugin_h plugin, const char *key, char **value, void *user_data)
{
	SAL_FN_CALL;

	RETV_IF(NULL == plugin, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

//******************************************************************************
//* Public interface definition
//******************************************************************************

API int oauth2_register_service(oauth2_service_h oauth2, GHashTable *service)
{
	SAL_FN_CALL;

	RETV_IF(NULL == oauth2, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	int ret = SERVICE_ADAPTOR_ERROR_NO_DATA;
	GHashTableIter iter;
	gpointer iter_key, iter_value;

	g_hash_table_iter_init(&iter, service);
	while (g_hash_table_iter_next(&iter, &iter_key, &iter_value))
	{
		if (0 == strcmp(iter_key, OAUTH2_0_GET_ACCESS_TOKEN_URI))
		{
			oauth2->oauth2_get_access_token = (_get_access_token) _oauth2_get_access_token;
			ret = SERVICE_ADAPTOR_ERROR_NONE;
		}
		else if (0 == strcmp(iter_key, OAUTH2_0_GET_EXTRA_DATA_URI))
		{
			oauth2->oauth2_get_extra_data = (_get_extra_data) _oauth2_get_extra_data;
			ret = SERVICE_ADAPTOR_ERROR_NONE;
		}
	}

	return ret;
}

API int oauth2_unregister_service(oauth2_service_h oauth2)
{
	SAL_FN_CALL;

	RETV_IF(NULL == oauth2, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	oauth2->oauth2_get_access_token = NULL;
	oauth2->oauth2_get_extra_data = NULL;

	return SERVICE_ADAPTOR_ERROR_NONE;
}
