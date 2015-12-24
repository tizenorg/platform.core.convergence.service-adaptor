/*
 * Auth Adaptor Client
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

#include <stdio.h>
#include <string.h>
#include <glib.h>

#include <app.h>
#include "sal_service_auth.h"
/*
#include "service_adaptor_errors.h"
#include "service_adaptor_internal.h"
#include "sal_service_adaptor.h"
#include "sal_service_adaptor_internal.h"
#include "sal_service_task.h"
#include "sal_service_task_internal.h"
#include "sal_service_auth_internal.h"
#include "sal_ipc_client_auth.h"
*/
/******************************************************************************
 * Global variables and defines
 ******************************************************************************/

/******************************************************************************
 * Private interface
 ******************************************************************************/

/******************************************************************************
 * Private interface definition
 ******************************************************************************/
/*
int service_auth_oauth1_start(service_auth_oauth1_h oauth1)
{
	SAL_FN_CALL;

	RETV_IF(NULL == oauth1, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == oauth1->plugin, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	char *uri = NULL;

	ret = service_plugin_get_uri(oauth1->plugin, &uri);
	RETVM_IF(SERVICE_ADAPTOR_ERROR_NONE != ret, ret, "service_plugin_get_uri() Failed(%d)", ret);

	service_auth_oauth1_h auth_oauth1 = NULL;
	ret = ipc_service_auth_oauth1(uri, oauth1, &auth_oauth1);
	RETVM_IF(SERVICE_ADAPTOR_ERROR_NONE != ret, ret, "ipc_service_auth_oauth1() Failed(%d)", ret);

	RETV_IF(NULL == oauth1->callback, SERVICE_ADAPTOR_ERROR_NONE);

	oauth1->callback(SERVICE_ADAPTOR_ERROR_NONE, auth_oauth1, oauth1->user_data);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

int service_auth_oauth1_stop(service_auth_oauth1_h oauth1)
{
	SAL_FN_CALL;

	RETV_IF(NULL == oauth1, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	return SERVICE_ADAPTOR_ERROR_NONE;
}
*/
/******************************************************************************
 * Public interface definition
 ******************************************************************************/

/*
API int service_auth_oauth1_create(service_plugin_h plugin, service_auth_oauth1_h *oauth1)
{
	SAL_FN_CALL;

	RETV_IF(NULL == plugin, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == oauth1, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	service_auth_oauth1_h auth_oauth1 = (service_auth_oauth1_h) g_malloc0(sizeof(service_auth_oauth1_s));
	auth_oauth1->plugin = plugin;

	*oauth1 = auth_oauth1;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_auth_oauth1_oauth1_clone(service_auth_oauth1_h src_oauth1, service_auth_oauth1_h *dst_oauth1)
{
	SAL_FN_CALL;

	RETV_IF(NULL == src_oauth1, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == dst_oauth1, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	service_auth_oauth1_h auth_oauth1 = (service_auth_oauth1_h) g_malloc0(sizeof(service_auth_oauth1_s));
	auth_oauth1->plugin = src_oauth1->plugin;
	auth_oauth1->callback = src_oauth1->callback;
	auth_oauth1->access_token = strdup(src_oauth1->access_token);
	auth_oauth1->operation = strdup(src_oauth1->operation);

	*dst_oauth1 = auth_oauth1;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_auth_oauth1_oauth1_destroy(service_auth_oauth1_h oauth1)
{
	SAL_FN_CALL;

	RETV_IF(NULL == oauth1, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	SAL_FREE(oauth1->access_token);
	SAL_FREE(oauth1->operation);
	SAL_FREE(oauth1);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_auth_oauth1_set_callback(service_auth_oauth1_h oauth1, service_auth_oauth1_cb callback, void *user_data)
{
	SAL_FN_CALL;

	RETV_IF(NULL == oauth1, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == callback, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	oauth1->callback = callback;
	oauth1->user_data = user_data;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_auth_oauth1_unset_callback(service_auth_oauth1_h oauth1)
{
	SAL_FN_CALL;

	RETV_IF(NULL == oauth1, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	oauth1->callback = NULL;
	oauth1->user_data = NULL;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_auth_oauth1_set_access_token(service_auth_oauth1_h oauth1, const char *access_token)
{
	SAL_FN_CALL;

	RETV_IF(NULL == oauth1, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == access_token, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	oauth1->access_token = strdup(access_token);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_auth_oauth1_get_access_token(service_auth_oauth1_h oauth1, char **access_token)
{
	SAL_FN_CALL;

	RETV_IF(NULL == oauth1, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == access_token, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	*access_token = strdup(oauth1->access_token);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_auth_oauth1_set_operation(service_auth_oauth1_h oauth1, const char *operation)
{
	SAL_FN_CALL;

	RETV_IF(NULL == oauth1, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == operation, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	oauth1->operation = strdup(operation);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_auth_oauth1_get_operation(service_auth_oauth1_h oauth1, char **operation)
{
	SAL_FN_CALL;

	RETV_IF(NULL == oauth1, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == operation, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	*operation = strdup(oauth1->operation);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_auth_oauth1_create_task(service_auth_oauth1_h oauth1, service_task_h *task)
{
	SAL_FN_CALL;

	RETV_IF(NULL == oauth1, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == task, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	service_task_h service_task = (service_task_h) g_malloc0(sizeof(service_task_s));
	service_task->oauth1 = oauth1;

	*task = service_task;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API int service_auth_oauth1_destroy_task(service_task_h task)
{
	SAL_FN_CALL;

	RETV_IF(NULL == task, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	SAL_FREE(task);

	return SERVICE_ADAPTOR_ERROR_NONE;
}
*/
