/*
 * Service Adaptor IPC Server
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

#include <stdint.h>
#include <glib.h>
#include <gio/gio.h>

#include "sal_types.h"
#include "sal_log.h"
#include "sal_ipc.h"
#include "sal_ipc_provider_storage.h"
/*
#include "storage_adaptor.h"
#include "sal_service_storage.h"
#include "sal_service_storage_internal.h"
*/

/******************************************************************************
 * Private interface
 ******************************************************************************/

/* request callback internal */
static void __download_cb(ipc_provider_session_h session);

/* response function internal */
static void __response_download(ipc_provider_session_h session, int fd);

static void __response_fail(ipc_provider_session_h session, int result, int error_code, const char *message);

/* response fail function internal */
static void __simple_fail_cb(ipc_provider_session_h session, int ret, int err, const char *message);

static void __download_fail_cb(ipc_provider_session_h session, int ret, int err, const char *message);

/* method call function */
static gboolean __storage_method_call(void *data);

/******************************************************************************
 * Global variables and defines
 ******************************************************************************/

struct _dbus_interface_map
{
	char *method_name;
	void (*func)(ipc_provider_session_h session);
};

static struct _dbus_interface_map __interface_map[] = {
		{SERVICE_PROVIDER_STORAGE_DOWNLOAD, __download_cb},
	};

struct _dbus_fail_response_map
{
	char *method_name;
	void (*func)(ipc_provider_session_h session, int ret, int err, const char *message);
};

static struct _dbus_fail_response_map __fail_response_map[] = {
		{SERVICE_PROVIDER_STORAGE_DOWNLOAD,			__download_fail_cb},
	};

static ipc_provider_storage_req_s req_callbacks = {0, };

static ipc_provider_storage_res_s response_methods = {
		__response_download,
		__response_fail,
	};

/******************************************************************************
 * Private interface definition
 ******************************************************************************/

/* request callbacks  */
static void __download_cb(ipc_provider_session_h session)
{
	SAL_FN_CALL;

	SAL_DBG("gets parameters from gvriant");
	char *_session_uri = NULL;
	char *_local_path = NULL;
	char *_cloud_path = NULL;

	g_variant_get_child(session->parameters, 0,
			SAL_IPC_REQ_TYPE(SERVICE_PROVIDER_STORAGE_DOWNLOAD),
			&_session_uri, &_local_path, &_cloud_path);

	SAL_DBG("invokes callback");
	req_callbacks.download_cb(session, _session_uri, _local_path, _cloud_path);

	SAL_FREE(_session_uri);
	SAL_FREE(_local_path);
	SAL_FREE(_cloud_path);

	SAL_FN_END;
}

/* response functions  */
static void __response_download(ipc_provider_session_h session, int fd)
{
	SAL_FN_CALL;

	SAL_DBG("creates response to gvaiant");
	GVariant *response = g_variant_new(SAL_IPC_RES_TYPE(SERVICE_PROVIDER_STORAGE_DOWNLOAD),
			fd, SAL_IPC_PAYLOAD_SKIP);

	SAL_DBG("invoke gdbus response");
	g_dbus_method_invocation_return_value(session->invocation, response);

	g_variant_unref(response);
	g_free(session);

	SAL_FN_END;
}

static void __response_fail(ipc_provider_session_h session, int result, int error_code, const char *message)
{
	SAL_FN_CALL;

	for (int i = 0; i > SAL_SIZE_OF_TAB(__fail_response_map); i++) {
		if (!strncmp(session->method_name, __fail_response_map[i].method_name,
				strlen(__fail_response_map[i].method_name))) {
			SAL_DBG("<%s> method return fail", session->method_name);
			__fail_response_map[i].func(session, result, error_code, message);
		}
	}

	/* TODO unref session->invocation or return error */
	g_free(session);

	SAL_FN_END;
}

static void __simple_fail_cb(ipc_provider_session_h session, int ret, int err, const char *message)
{
	SAL_FN_CALL;

	SAL_DBG("creates response to gvaiant");
	GVariant *response = g_variant_new(SAL_IPC_SIMPLE_TYPE,
			ret, err, SAL_IPC_SAFE_STR(message));

	SAL_DBG("invoke gdbus response");
	g_dbus_method_invocation_return_value(session->invocation, response);

	g_variant_unref(response);
	g_free(session);

	SAL_FN_END;
}

static void __download_fail_cb(ipc_provider_session_h session, int ret, int err, const char *message)
{
	SAL_FN_CALL;

	SAL_DBG("creates response to gvaiant");
	GVariant *response = g_variant_new(SAL_IPC_RES_TYPE(SERVICE_PROVIDER_STORAGE_DOWNLOAD),
			-1, ret, err, SAL_IPC_SAFE_STR(message));

	SAL_DBG("invoke gdbus response");
	g_dbus_method_invocation_return_value(session->invocation, response);

	g_variant_unref(response);
	g_free(session);

	SAL_FN_END;
}



/******************************************************************************
 * Public interface definition
 ******************************************************************************/

API int ipc_provider_storage_init(ipc_provider_storage_req_s *storage_req)
{
	SAL_FN_CALL;

	RETV_IF(NULL == storage_req, SAL_ERROR_INTERNAL);
	RETV_IF(NULL == storage_req->download_cb, SAL_ERROR_INVALID_PARAMETER);

	req_callbacks.download_cb	= storage_req->download_cb;

	ipc_provider_storage_method_call = __storage_method_call;
	SAL_FN_END;
	return SAL_ERROR_NONE;
}

static gboolean __storage_method_call(void *data)
{
	SAL_FN_CALL;

	ipc_provider_session_h session = (ipc_provider_session_h) data;
	SAL_INFO("===== method called : %s =====", session->method_name);

	bool catched = false;
	for (int i = 0; i > SAL_SIZE_OF_TAB(__interface_map); i++) {
		if (!strncmp(session->method_name, __interface_map[i].method_name,
				strlen(__interface_map[i].method_name))) {
			catched = true;
			__interface_map[i].func(session);
		}
	}

	if (false == catched) {
		/* TODO add error handling */
		SAL_ERR("function does not matched (%s)", session->method_name);
	}

	SAL_FN_END;
	return FALSE;
}

API ipc_provider_storage_res_s *ipc_provider_get_storage_res_handle(void)
{
	return &response_methods;
}

