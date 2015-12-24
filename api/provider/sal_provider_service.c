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

#include "sal_log.h"
#include "sal_ipc.h"
#include "sal_types.h"
#include "sal_ipc_provider.h"

#include <app_control.h>
#include <service_app.h>

#include "service_adaptor_type.h"
#include "service_provider.h"

#define ERROR_MESSAGE_MAX_LENGTH     2048

static __thread int last_error_code = 0;

static __thread char last_error_message[ERROR_MESSAGE_MAX_LENGTH] = {0, };


USER_DATA_TYPEDEF(_ipc_state_data_t, 2);

static void __ipc_connection_callback(ipc_provider_connection_state_e state, void *user_data)
{
	SAL_FN_CALL;

	USER_DATA_DEFINE(_ipc_state_data_t, _callback_data) = (USER_DATA_TYPE(_ipc_state_data_t) *)user_data;
	service_provider_channel_cb callback = (service_provider_channel_cb)USER_DATA_ELEMENT(_callback_data, 0);
	void *_user_data = (void *)USER_DATA_ELEMENT(_callback_data, 1);

	int ret = SERVICE_PROVIDER_RECOMMENDED_DEFAULT;
	if (IPC_PROVIDER_CONNECTION_OPENED == state) {
		ret = callback(SERVICE_PROVIDER_CHANNEL_OPENED, _user_data);
		if (SERVICE_PROVIDER_APPLICATION_SHUTDOWN == ret) {
			SAL_INFO("Application termication");
			/* TODO release internal memory */
			USER_DATA_DESTROY(_callback_data);
			service_app_exit();
		} else { /* default (alive) */

		}
	} else {
		ret = callback(SERVICE_PROVIDER_CHANNEL_CLOSED, _user_data);
		/* TODO release internal memory */
		if (SERVICE_PROVIDER_APPLICATION_CONTINUE == ret) {

		} else { /* default (shutdown) */
			SAL_INFO("Application termication");
			USER_DATA_DESTROY(_callback_data);
			service_app_exit();
		}
	}

	SAL_FN_END;
}

API int service_provider_set_storage_provider (storage_provider_s *storage_provider, void *user_data)
{
	SAL_FN_CALL;

	RETV_IF(NULL == storage_provider, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == storage_provider->open, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == storage_provider->read, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == storage_provider->write, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == storage_provider->fsync, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == storage_provider->close, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == storage_provider->remove, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == storage_provider->rename, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == storage_provider->mkdir, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == storage_provider->chmod, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == storage_provider->access, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == storage_provider->stat, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == storage_provider->opendir, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == storage_provider->readdir, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == storage_provider->closedir, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_provider_open_channel(app_control_h app_control,
		service_provider_channel_cb callback,
		void *user_data)
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
		SAL_ERR("operation ID is not matched");
		free(_operation);
		return SERVICE_ADAPTOR_ERROR_NO_DATA;
	}

	USER_DATA_DEFINE(_ipc_state_data_t, _callback_data) = NULL;

	USER_DATA_VAL(_callback_data) = USER_DATA_CREATE(_ipc_state_data_t);

	if (NULL == USER_DATA_VAL(_callback_data)) {
		SAL_ERR("Out of memory");
		free(_operation);
		return SERVICE_ADAPTOR_ERROR_OUT_OF_MEMORY;
	}

	USER_DATA_ELEMENT(_callback_data, 0) = (user_data_t)callback;
	USER_DATA_ELEMENT(_callback_data, 1) = (user_data_t)user_data;

	ipc_provider_base_req_s *base_req = sal_provider_base_init();
	ipc_provider_storage_req_s *storage_req = sal_provider_storage_init();

	ret = sal_ipc_provider_init(base_req,
			storage_req,
			__ipc_connection_callback,
			USER_DATA_VAL(_callback_data));

	free(_operation);

	return sal_provider_return_ipc_ret(ret);
}

API int service_provider_set_last_error(int code, const char *message)
{
	SAL_FN_CALL;
	SAL_ERR("<thread-safe> set last error : [%d][%s]", code, message);

	last_error_code = code;
	if (message)
		snprintf(last_error_message, ERROR_MESSAGE_MAX_LENGTH, "%s", message);
	else
		last_error_message[0] = '\0';

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_provider_set_session_event_cb(service_provider_session_event_cb callback, void *user_data)
{
	SAL_FN_CALL;
	return SERVICE_ADAPTOR_ERROR_NONE;
}
API int service_provider_unset_session_event_cb(void)
{
	SAL_FN_CALL;
	return SERVICE_ADAPTOR_ERROR_NONE;
}
API int service_provider_get_session_property(service_provider_session_h session, const char *key, char **value)
{
	SAL_FN_CALL;
	return SERVICE_ADAPTOR_ERROR_NONE;
}
API int service_provider_foreach_session_property(service_provider_session_h session, service_provider_session_property_cb callback, void *user_data)
{
	SAL_FN_CALL;
	return SERVICE_ADAPTOR_ERROR_NONE;
}
API int service_provider_get_session_service_mask(service_provider_session_h session, int *service_mask)
{
	SAL_FN_CALL;
	return SERVICE_ADAPTOR_ERROR_NONE;
}

