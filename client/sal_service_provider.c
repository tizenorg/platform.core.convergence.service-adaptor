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

#include <stdio.h>
#include <string.h>
#include <glib.h>

#include <app.h>

#include "service_adaptor_errors.h"
#include "service_adaptor_internal.h"
#include "sal_service_provider.h"

API int service_provider_create(service_provider_h *provider)
{
	SAL_FN_CALL;

	service_provider_h service_provider = (service_provider_h) g_malloc0(sizeof(service_provider_s));

	service_provider->connect = NULL;
	service_provider->disconnect = NULL;

	*provider = service_provider;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_provider_destroy(service_provider_h provider)
{
	SAL_FN_CALL;

	/* TODO: free internal value of provider */
	SAL_FREE(provider);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_provider_set_auth_provider(service_provider_h provider, auth_provider_h auth_provider)
{
	SAL_FN_CALL;

	RETV_IF(NULL == provider, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == auth_provider, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	provider->auth_provider = auth_provider;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_provider_unset_auth_provider(service_provider_h provider)
{
	SAL_FN_CALL;

	RETV_IF(NULL == provider, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	provider->auth_provider = NULL;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_provider_set_storage_provider(service_provider_h provider, storage_provider_h storage_provider)
{
	SAL_FN_CALL;

	RETV_IF(NULL == provider, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == storage_provider, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	provider->storage_provider = storage_provider;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_provider_unset_storage_provider(service_provider_h provider)
{
	SAL_FN_CALL;

	RETV_IF(NULL == provider, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	provider->storage_provider = NULL;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_provider_message(service_provider_h provider, app_control_h app_control, void *user_data)
{
	SAL_FN_CALL;

	RETV_IF(NULL == provider, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	char *operation = NULL;
	app_control_get_operation(app_control, &operation);

	RETV_IF(NULL == operation, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	app_control_h reply = NULL;

	if (0 == strcmp(operation, PLUGIN_CONNECT_URI)) {
		app_control_create(&reply);
		ret = provider->connect();

		if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
			SAL_ERR("connect() Fail (%d)", ret);
			app_control_add_extra_data(reply, PLUGIN_RESULT_KEY, PLUGIN_RESULT_VALUE_FAILURE);
			goto catch;
		}

		app_control_add_extra_data(reply, PLUGIN_RESULT_KEY, PLUGIN_RESULT_VALUE_SUCCESS);

		auth_provider_add_extra_data(provider->auth_provider, reply);
		storage_provider_add_extra_data(provider->storage_provider, reply);

		/* TODO: another adaptor */
	} else if (0 == strcmp(operation, PLUGIN_DISCONNECT_URI)) {
		app_control_create(&reply);
		ret = provider->disconnect();

		if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
			SAL_ERR("connect() Fail (%d)", ret);
			app_control_add_extra_data(reply, PLUGIN_RESULT_KEY, PLUGIN_RESULT_VALUE_FAILURE);
			goto catch;
		}

		app_control_add_extra_data(reply, PLUGIN_RESULT_KEY, PLUGIN_RESULT_VALUE_SUCCESS);
	} else {
		reply = auth_provider_message(provider->auth_provider, operation, app_control);
		TRY_IF(NULL != reply, "auth_plugin_client_message() Finded");

		reply = storage_provider_message(provider->storage_provider, operation, app_control);
		TRY_IF(NULL != reply, "storage_plugin_client_message() Finded");
	}

catch:
	app_control_reply_to_launch_request(reply, app_control, APP_CONTROL_RESULT_SUCCEEDED);
	app_control_destroy(reply);

	return SERVICE_ADAPTOR_ERROR_NONE;
}
