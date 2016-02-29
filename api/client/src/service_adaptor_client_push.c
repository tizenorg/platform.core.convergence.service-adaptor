/*
* Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
*
* Licensed under the Apache License, Version 2.0 (the License);
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an AS IS BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

/******************************************************************************
 * File: service-adaptor-client-push.c
 * Desc:
 * Created on: Oct, 2014
 * Auth: Yongjin Kim <youth.kim@samsung.com>
 *
 *****************************************************************************/

#include <stdio.h>
#include <glib.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "private/service-adaptor-client-push.h"
#include "service_adaptor_client_type.h"
#include "service_adaptor_client_private.h"
#include "service_adaptor_client_log.h"
#include "dbus/dbus_client_push.h"

#include "util/service_adaptor_client_util.h"
/**	@brief	Registers a callback function to receive push notification from push service
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_connect_push_service(service_adaptor_h handle,
						service_adaptor_push_notification_cb callback,
						service_adaptor_error_s **error_code,
						void *user_data)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	if ((NULL == handle) || (NULL == callback)) {
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER, "Invalid Parameter");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	sac_debug("Connect push service : service_id(%d)", handle->service_id);

	_signal_queue_add_task(handle->service_id, (uint32_t) callback, handle, user_data);

	sac_api_end(ret);
	return ret;
}

/**	@brief	Deregisters the callback function that had been registered to push service
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_disconnect_push_service(service_adaptor_h handle)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	if (NULL == handle) {
		service_adaptor_set_last_result(SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER, "Invlid parameters");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	service_adaptor_task_h task = _signal_queue_get_task(handle->service_id);

	if (NULL == task) {
		return SERVICE_ADAPTOR_ERROR_NONE;
	}

	sac_debug("Disconnect push service : service_id(%d)", handle->service_id);

	_signal_queue_del_task(task);

	sac_api_end(ret);
	return ret;
}

SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_register_push_service(service_adaptor_h handle,
						const char *service_file_name)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;

	if ((NULL == handle) || (NULL == service_file_name)) {
		service_adaptor_set_last_result(SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER, "Invlid parameters");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	ret = _dbus_push_register(service_file_name, &error);

	if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
		sac_error("Error occured (%lld)(%s)", error.code, error.msg);
		service_adaptor_set_last_result(error.code, error.msg);
		free(error.msg);
	}

	sac_api_end(ret);
	return ret;
}

SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_deregister_push_service(service_adaptor_h handle,
						const char *service_file_name)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;

	if ((NULL == handle) || (NULL == service_file_name)) {
		service_adaptor_set_last_result(SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER, "Invlid parameters");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	ret = _dbus_push_deregister(service_file_name, &error);

	if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
		sac_error("Error occured (%lld)(%s)", error.code, error.msg);
		service_adaptor_set_last_result(error.code, error.msg);
		free(error.msg);
	}

	sac_api_end(ret);
	return ret;
}
