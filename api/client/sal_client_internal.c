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

#include "sal_log.h"
#include "sal_ipc.h"

#include "sal_client_internal.h"
#include <service_adaptor_type.h>

/******************************************************************************
 * Global variables and defines
 ******************************************************************************/

/******************************************************************************
 * Private interface
 ******************************************************************************/

/******************************************************************************
 * Private interface definition
 ******************************************************************************/

/******************************************************************************
 * Public interface definition
 ******************************************************************************/

API int sal_client_return_ipc_ret(int ipc_client_ret)
{
	switch (ipc_client_ret) {
	case SAL_ERROR_NONE:
		SAL_INFO("== API remote ret : %s", "SERVICE_ADAPTOR_ERROR_NONE");
		return SERVICE_ADAPTOR_ERROR_NONE;
	case SAL_ERROR_NOT_SUPPORTED:
		SAL_ERR("== API remote ret : %s", "SERVICE_ADAPTOR_ERROR_NOT_SUPPORTED");
		return SERVICE_ADAPTOR_ERROR_NOT_SUPPORTED;
	case SAL_ERROR_INVALID_PARAMETER:
		SAL_ERR("== API remote ret : %s", "SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	case SAL_ERROR_TIMED_OUT:
		SAL_ERR("== API remote ret : %s", "SERVICE_ADAPTOR_ERROR_TIMED_OUT");
		return SERVICE_ADAPTOR_ERROR_TIMED_OUT;
	case SAL_ERROR_NO_DATA:
		SAL_ERR("== API remote ret : %s", "SERVICE_ADAPTOR_ERROR_NO_DATA");
		return SERVICE_ADAPTOR_ERROR_NO_DATA;
	case SAL_ERROR_PERMISSION_DENIED:
		SAL_ERR("== API remote ret : %s", "SERVICE_ADAPTOR_ERROR_PERMISSION_DENIED");
		return SERVICE_ADAPTOR_ERROR_PERMISSION_DENIED;
	case SAL_ERROR_IPC_UNSTABLE:
		SAL_ERR("== API remote ret : %s", "SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE");
		return SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
	case SAL_ERROR_PLUGIN_FAILED:
		SAL_ERR("== API remote ret : %s", "SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED");
		return SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED;
	case SAL_ERROR_NOT_AUTHORIZED:
		SAL_ERR("== API remote ret : %s", "SERVICE_ADAPTOR_ERROR_NOT_AUTHORIZED");
		return SERVICE_ADAPTOR_ERROR_NOT_AUTHORIZED;
	case SAL_ERROR_INVALID_STATE:
		SAL_ERR("== API remote ret : %s", "SERVICE_ADAPTOR_ERROR_INVALID_STATE");
		return SERVICE_ADAPTOR_ERROR_INVALID_STATE;
	default:
		SAL_ERR("== API remote ret : %d, %s", ipc_client_ret, "SERVICE_ADAPTOR_ERROR_UNKNOWN");
		return SERVICE_ADAPTOR_ERROR_UNKNOWN;
	}
}
