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
#include "sal.h"
#include "sal_ipc_server_auth.h"
/*
#include "auth_adaptor.h"
#include "sal_service_auth.h"
#include "sal_service_auth_internal.h"
*/

/******************************************************************************
 * Private interface
 ******************************************************************************/

static void __chunk_cb(ipc_server_session_h session)


/******************************************************************************
 * Global variables and defines
 ******************************************************************************/

struct _dbus_interface_map
{
	char *method_name;
	void (*func)(void *data);
};

static struct _dbus_interface_map __interface_map[] = {
		NULL,
	};

static ipc_server_auth_req_s req_callbacks = {0, };

static ipc_server_auth_res_s response_methods = {
		__response_chunk,
	};

/******************************************************************************
 * Private interface definition
 ******************************************************************************/

/* request callbacks  */
static void __chunk_cb(ipc_server_session_h session)
{
	SAL_FN_CALL;

	SAL_FN_END;
}

/* response functions  */
static void __response_chunk(ipc_server_session_h session)
{
	SAL_FN_CALL;

	SAL_FN_END;
}


/******************************************************************************
 * Public interface definition
 ******************************************************************************/

API int ipc_server_auth_init(ipc_server_auth_req_s *auth_req)
{
	SAL_FN_CALL;

	RETV_IF(NULL == auth_req, SAL_ERROR_INTERNAL);
	/*RET_IF(NULL == base_req->chunk_cb);*/

	req_callbacks.chunk_cb	= auth_req->chunk_cb;

	SAL_FN_END;
	return SAL_ERROR_NONE;
}

API gboolean sal_server_auth_method_call(void *data)
{
	SAL_FN_CALL;

	ipc_server_session_h session = (ipc_server_session_h) data;
	sal_info("===== method called : %s =====", session->method_name);

	bool catched = false;
	for (int i = 0; __interface_map[i]; i++) {
		if (!strncmp(session->method_name, __interface_map[i].method_name,
				strlen(__interface_map[i].method_name))) {
			catched = true;
			__interface_map[i].func(session);
		}
	}

	if (false == catched) {
		/* TODO add error handling */
		sal_error("function does not matched (%s)", ipc_data->method_name);
	}

	SAL_FN_END;
	return FALSE;
}

API ipc_server_auth_res_s *ipc_server_get_auth_res_handle(void);
{
	return &response_methods;
}

/*
API void service_auth_method_call(GDBusConnection *connection,
		const gchar *sender,
		const gchar *object_path,
		const gchar *interface_name,
		const gchar *method_name,
		GVariant *parameters,
		GDBusMethodInvocation *invocation,
		gpointer user_data)
{
	SAL_FN_CALL;

	int ipc_ret = SAL_ERROR_NONE;
	char *ipc_msg = NULL;
	char *ipc_type = NULL;
	GVariant *ipc_data = NULL;

	char *uri = NULL;

	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);

	if (0 == g_strcmp0(method_name, DBUS_SERVICE_AUTH_OAUTH1_METHOD)) {
		int idx = 0;
		int size = service_auth_oauth1_req_s_type_length;
		GVariant *req_info[size];

		ipc_create_variant_info(in_parameters, size, (GVariant ***) &req_info);

		char *uri = ipc_insure_g_variant_dup_string(req_info[idx++]);
		service_auth_oauth1_h oauth1 = NULL;
		_get_oauth1(req_info[idx++], &oauth1);

		SAL_INFO("uri: %s", uri);

		ipc_ret = SAL_ERROR_INTERNAL;
		ipc_type = strdup(service_auth_oauth1_res_s_type);

		sal_h sal = sal_get_handle();
		TRYVM_IF(NULL == sal, ipc_ret = SAL_ERROR_INTERNAL, "sal_get_handle() Failed");

		auth_plugin_h plugin = auth_adaptor_get_plugin(sal->auth, uri);

		ipc_reply_data_h reply = (ipc_reply_data_h) g_malloc0(sizeof(ipc_reply_data_s));
		reply->invocation = invocation;
		reply->type = strdup(ipc_type);

		ipc_ret = _oauth1_execute_operation(plugin, oauth1, reply);
		TRY_IF(SAL_ERROR_NONE == ipc_ret, "oauth1_execute_operation() Request Successed");

		ipc_create_error_msg(ipc_ret, &ipc_msg);
		ipc_data = g_variant_new(ipc_make_return_type(ipc_type), SAL_IPC_STR(NULL), SAL_IPC_STR(NULL), ipc_ret, SAL_IPC_STR(ipc_msg));

		ipc_destroy_variant_info(req_info, size);
	}

	g_dbus_method_invocation_return_value(invocation, ipc_data);

catch:
	SAL_FREE(uri);
	SAL_FREE(ipc_msg);
	SAL_FREE(ipc_type);

	SAL_FN_END;
}
*/
