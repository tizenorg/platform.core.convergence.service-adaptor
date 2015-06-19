/*
 * oAuth 1.0 Service
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

#include "oauth1_service.h"
#include "auth_adaptor.h"
#include "service_adaptor_errors.h"
#include "service_adaptor_internal.h"

#include "sal_service_provider.h"
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

typedef int (*_get_access_token)(void *plugin, oauth1_cb callback, void *user_data);
typedef int (*_get_extra_data)(void *plugin, const char *key, oauth1_cb callback, void *user_data);

//******************************************************************************
//* Private interface
//******************************************************************************

//******************************************************************************
//* Private interface definition
//******************************************************************************

/**
 * @brief callback of service plugin
 *
 * @return      void.
 */
static void _oauth1_get_access_token_cb(app_control_h request, app_control_h reply, app_control_result_e result, void *user_data)
{
	SAL_FN_CALL;

	RET_IF(APP_CONTROL_RESULT_FAILED == result);
	RET_IF(NULL == user_data);

	char *ret_str = NULL;
	app_control_get_extra_data(reply, PLUGIN_RESULT_KEY, &ret_str);

	char *access_token = NULL;
	app_control_get_extra_data(reply, OAUTH1_0_ACCESS_TOKEN_KEY, &access_token);

	app_control_user_data_h app_control_user_data = (app_control_user_data_h) user_data;
	oauth1_cb callback = (oauth1_cb) app_control_user_data->callback;

	RET_IF(NULL == callback);

	// TODO: move this function for chaning result enum to general
	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	if (0 == strcmp(PLUGIN_RESULT_VALUE_FAILURE, ret_str))
	{
		ret = SERVICE_ADAPTOR_ERROR_INTERNAL;
	}

	oauth1_h oauth1 = (oauth1_h) g_malloc0(sizeof(oauth1_s));
	oauth1->access_token = strdup(access_token);

	callback(ret, oauth1, app_control_user_data->user_data);

	SAL_FN_END;
}

static int _oauth1_get_access_token(auth_plugin_h plugin, oauth1_cb callback, void *user_data)
{
	SAL_FN_CALL;

	RETV_IF(NULL == plugin, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	app_control_h request;
	app_control_create(&request);

	app_control_set_app_id(request, plugin->uri);
	app_control_set_operation(request, OAUTH1_0_GET_ACCESS_TOKEN_URI);

	app_control_user_data_h app_control_user_data = (app_control_user_data_h) g_malloc0(sizeof(app_control_user_data_s));
	app_control_user_data->callback = (void *) callback;
	app_control_user_data->user_data = user_data;

	int res = app_control_send_launch_request(request, _oauth1_get_access_token_cb, app_control_user_data);

	if (APP_CONTROL_ERROR_NONE != res)
	{
		return SERVICE_ADAPTOR_ERROR_SYSTEM;
	}

	return SERVICE_ADAPTOR_ERROR_NONE;
}

static int _oauth1_get_extra_data(auth_plugin_h plugin, const char *key, oauth1_cb callback, void *user_data)
{
	SAL_FN_CALL;

	RETV_IF(NULL == plugin, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

//******************************************************************************
//* Public interface definition
//******************************************************************************

API int oauth1_register_service(oauth1_service_h oauth1, GHashTable *service)
{
	SAL_FN_CALL;

	RETV_IF(NULL == oauth1, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	int ret = SERVICE_ADAPTOR_ERROR_NO_DATA;
	GHashTableIter iter;
	gpointer iter_key, iter_value;

	g_hash_table_iter_init(&iter, service);
	while (g_hash_table_iter_next(&iter, &iter_key, &iter_value))
	{
		if (0 == strcmp(iter_key, OAUTH1_0_GET_ACCESS_TOKEN_URI))
		{
			oauth1->oauth1_get_access_token = (_get_access_token) _oauth1_get_access_token;
			ret = SERVICE_ADAPTOR_ERROR_NONE;
		}
		else if (0 == strcmp(iter_key, OAUTH1_0_GET_EXTRA_DATA_URI))
		{
			oauth1->oauth1_get_extra_data = (_get_extra_data) _oauth1_get_extra_data;
			ret = SERVICE_ADAPTOR_ERROR_NONE;
		}
	}

	return ret;
}

API int oauth1_unregister_service(oauth1_service_h oauth1)
{
	SAL_FN_CALL;

	RETV_IF(NULL == oauth1, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	oauth1->oauth1_get_access_token = NULL;
	oauth1->oauth1_get_extra_data = NULL;

	return SERVICE_ADAPTOR_ERROR_NONE;
}
