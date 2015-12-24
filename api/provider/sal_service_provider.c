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

#include <app_control.h>
#include <service_app.h>

#include "service_adaptor_type.h"
#include "service_provider.h"

USER_DATA_TYPEDEF(_ipc_state_data_t, 2);

static void __ipc_connection_callback(ipc_provider_connection_state_e state, void *user_data)
{
	SAL_FN_CALL;

	USER_DATA_DEFINE(_ipc_state_data_t, _callback_data) = USER_DATA_TYPE(_ipc_state_data_t)user_data;
	service_provider_channel_cb callback = (service_provider_channel_cb)USER_DATA_ELEMENT(_callback_data, 0);
	void *_user_data = (void *)USER_DATA_ELEMENT(_callback_data, 1);

	int ret = SERVICE_PROVIDER_RECOMMENDED_DEFAULT;
	if (IPC_PROVIDER_CONNECTION_OPENED == state) {
		ret = callback(SERVICE_PROVIDER_CHANNEL_OPENED, _user_data);
		if (SERVICE_PROVIDER_APPLICATION_SHUTDOWN == ret) {

			sal_info("Application termication");
			service_app_exit();
		} else { /* default (alive) */

		}
	} else {
		ret = callback(SERVICE_PROVIDER_CHANNEL_CLOSED, _user_data);
		if (SERVICE_PROVIDER_APPLICATION_CONTINUE == ret) {

		} else { /* default (shutdown) */

			sal_info("Application termication");
			service_app_exit();
		}
	}

	SAL_FN_END;
}



API int service_provider_open_channel(app_control_h app_control,
		service_provider_channel_cb callback,
		void *user_data);
{
	SAL_FN_CALL;

	RETV_IF(NULL == app_control, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == callback, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	int ret = 0;
	char *_operation = NULL;
	ret = app_control_get_operation(app_control, &_operation);

	RETVM_IF(APP_CONTROL_ERROR_OUT_OF_MEMORY == ret, SERVICE_ADAPTOR_ERROR_OUT_OF_MEMORY, "Out of Memory");
	RETVM_IF(NULL == _operation, SERVICE_ADAPTOR_ERROR_NO_DATA, "No operation data");

	if (0 != g_strcmp0(APP_CONTROL_OPERATION_SERVICE_PROVIDER_CHANNEL, _operation)) {
		sal_error("operation ID is not matched");
		free(_operation);
		return SERVICE_ADAPTOR_ERROR_NO_DATA;
	}

	USER_DATA_DEFINE(_ipc_state_data_t, _callback_data) = NULL;

	USER_DATA_VAL(_callback_data) = USER_DATA_CREATE(_ipc_state_data_t, _callback_data);

	if (NULL == USER_DATA_VAL(_callback_data)) {
		SAL_ERROR("Out of memory");
		free(_operation);
		return SERVICE_ADAPTOR_ERROR_OUT_OF_MEMORY;
	}

	USER_DATA_ELEMENT(_callback_data, 0) = (user_data_t)callback;
	USER_DATA_ELEMENT(_callback_data, 1) = (user_data_t)user_data;

	ret = sal_ipc_provider_init(
			__ipc_connection_callback,
			USER_DATA_VAL(_callback_data));

	free(_operation);

	return sal_provider_return_ipc_ret(ret);
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
