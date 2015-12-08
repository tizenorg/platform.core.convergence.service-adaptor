/*
 * Service Adaptor IPC Client
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

#include <glib.h>
#include <gio/gio.h>

#include "service_adaptor_errors.h"
#include "service_adaptor_internal.h"
#include "sal_ipc_client.h"
#include "sal_ipc_client_auth.h"
#include "sal_service_auth.h"
#include "sal_service_auth_internal.h"

/******************************************************************************
 * Global variables and defines
 ******************************************************************************/

/******************************************************************************
 * Private interface
 ******************************************************************************/

/******************************************************************************
 * Private interface definition
 ******************************************************************************/

int _get_oauth1(GVariant *reply_info, service_auth_oauth1_h *oauth1)
{
	service_auth_oauth1_h auth_oauth1 = (service_auth_oauth1_h) g_malloc0(sizeof(service_auth_oauth1_s));

	int info_size = service_auth_oauth1_res_s_type_length;
	GVariant *info[info_size];
	ipc_create_variant_info(reply_info, info_size, (GVariant ***) &info);

	int idx = 0;
	int auth_info_size = service_auth_oauth1_s_type_length;
	GVariant *auth_info[auth_info_size];
	ipc_create_variant_info(info[idx++], auth_info_size, (GVariant ***) &auth_info);

	int idx2 = 0;
	auth_oauth1->access_token = ipc_insure_g_variant_dup_string(auth_info[idx2++]);
	auth_oauth1->operation = ipc_insure_g_variant_dup_string(auth_info[idx2++]);

	ipc_destroy_variant_info(auth_info, auth_info_size);

	ipc_destroy_variant_info(info, info_size);

	*oauth1 = auth_oauth1;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

/******************************************************************************
 * Public interface definition
 ******************************************************************************/

API int ipc_service_auth_oauth1(const char *uri, service_auth_oauth1_h req_oauth1, service_auth_oauth1_h *res_oauth1)
{
	SAL_FN_CALL;

	RETV_IF(NULL == uri, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == req_oauth1, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	char *request_method = DBUS_SERVICE_AUTH_OAUTH1_METHOD;
	GVariant *request_data = g_variant_new("(" service_auth_oauth1_req_s_type ")", uri, SAL_IPC_STR(req_oauth1->access_token), SAL_IPC_STR(req_oauth1->operation));

	char *reply_type = service_auth_oauth1_res_s_type;
	int reply_size = RETURN_LENGTH + 1;
	GVariant *reply = NULL;

	ret = sal_ipc_client_call_request(request_method, request_data, reply_type, &reply);
	RETVM_IF(SERVICE_ADAPTOR_ERROR_NONE != ret, ret, "ipc_client_call_request() Failed(%d)", ret);

	GVariant *reply_info[reply_size];
	ipc_create_variant_info(reply, reply_size, (GVariant ***) &reply_info);

	int idx = 0;
	service_auth_oauth1_h oauth1 = NULL;
	_get_oauth1(reply_info[idx++], &oauth1);

	int ipc_ret = g_variant_get_int32(reply_info[idx++]);
	char *ipc_msg = ipc_insure_g_variant_dup_string(reply_info[idx++]);

	ipc_destroy_variant_info(reply_info, reply_size);

	g_variant_unref(reply);

	RETVM_IF(SERVICE_ADAPTOR_ERROR_NONE != ipc_ret, SERVICE_ADAPTOR_ERROR_INTERNAL, "IPC Result Failed(%d): %s", ipc_ret, ipc_msg);

	*res_oauth1 = oauth1;

	SAL_FREE(ipc_msg);

	return SERVICE_ADAPTOR_ERROR_NONE;
}
