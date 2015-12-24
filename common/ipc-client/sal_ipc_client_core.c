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

#include "sal_types.h"
#include "sal_log.h"
#include "sal_ipc.h"
#include "sal_ipc_client.h"
#include "sal_ipc_client_core.h"

/******************************************************************************
 * Global variables and defines
 ******************************************************************************/

/******************************************************************************
 * Private interface
 ******************************************************************************/

/*
 * @param[in]	response	Gvariant response
 * @param[out]	list		plugin list
 */
static void __get_connect_res(GVariant *response, GList **list);

/*
 * @param[in]	response		Gvariant response
 * @param[out]	plugin_handle	identifier of plugin handle
 */
static void __get_plugin_start_res(GVariant *response, char **plugin_handle);

/******************************************************************************
 * Private interface definition
 ******************************************************************************/

static void __get_connect_res(GVariant *response, GList **list)
{
	SAL_FN_CALL;

	GVariantIter *plugin_iter = NULL;
	plugin_iter  = g_variant_iter_new(response);
	GList *_plugin_list = NULL;

	char *_plugin_uri = NULL;
	while (g_variant_iter_loop(plugin_iter, "(s)", &_plugin_uri)) {

		SAL_DBG("<iter> plugin_uri : %s", _plugin_uri);
		if (_plugin_uri && ('\0' != _plugin_uri))
			_plugin_list = g_list_append(_plugin_list, (void *)_plugin_uri);
		else
			free(_plugin_uri);
		_plugin_uri = NULL;
	}

	g_variant_iter_free(plugin_iter);
	plugin_iter = NULL;

/*
	if (0 == g_list_length(_plugin_list)) {
		ret = SAL_ERROR_NO_DATA;
	}
*/

	*list = _plugin_list;

	SAL_FN_END;
}

static void __get_plugin_start_res(GVariant *response, char **plugin_handle)
{
	SAL_FN_CALL;
	char *_str = NULL;
	g_variant_get(response, service_plugin_start_res_s_type, &_str);
	if (_str && ('\0' == _str[0]))
		SAL_FREE(_str);

	*plugin_handle = _str;
	SAL_FN_END;
}

/******************************************************************************
 * Public interface definition
 ******************************************************************************/

API int ipc_service_adaptor_connect(int pid, const char *uri, GList **plugins)
{
	SAL_FN_CALL;

	RETV_IF(NULL == uri, SAL_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == plugins, SAL_ERROR_INVALID_PARAMETER);

	int ret = SAL_ERROR_NONE;

	/* Call dbus request */
	char *request_method = DBUS_SERVICE_ADAPTOR_CONNECT_METHOD;
	GVariant *request_data = g_variant_new("(" service_adaptor_connect_req_s_type ")", pid, uri);

	char *reply_type = SAL_IPC_RETURN_TYPE(service_adaptor_connect_res_s_type);
	GVariant *reply = NULL;

	ret = sal_ipc_client_call_request(request_method, request_data, reply_type, &reply);
	RETVM_IF(SAL_ERROR_NONE != ret, ret, "ipc_client_call_request() Failed(%d)", ret);

	/* Parse response */
	GVariant *reply_data = NULL;
	ret = sal_ipc_client_get_data_response(reply, &reply_data);
	if (!ret && reply_data) {
		__get_connect_res(reply_data, plugins);
		g_variant_unref(reply_data);
	}

	SAL_FN_END;
	return ret;
}

API int ipc_service_adaptor_disconnect(int pid, const char *uri)
{
	SAL_FN_CALL;

	RETV_IF(NULL == uri, SAL_ERROR_INVALID_PARAMETER);

	int ret = SAL_ERROR_NONE;

	/* Call dbus request */
	char *request_method = DBUS_SERVICE_ADAPTOR_DISCONNECT_METHOD;
	GVariant *request_data = g_variant_new("(" service_adaptor_disconnect_s_type ")", pid, uri);

	char *reply_type = SAL_IPC_SIMPLE_TYPE;
	GVariant *reply = NULL;

	ret = sal_ipc_client_call_request(request_method, request_data, reply_type, &reply);
	RETVM_IF(SAL_ERROR_NONE != ret, ret, "ipc_client_call_request() Failed(%d)", ret);

	ret = sal_ipc_client_get_simple_response(reply);

	SAL_FN_END;
	return ret;
}


API int ipc_service_plugin_start(int pid, const char *uri, const char *plugin_uri, char **plugin_handle)
{
	SAL_FN_CALL;

	RETV_IF(NULL == uri, SAL_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == plugin_uri, SAL_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == plugin_handle, SAL_ERROR_INVALID_PARAMETER);

	int ret = SAL_ERROR_NONE;

	char *request_method = DBUS_SERVICE_PLUGIN_START_METHOD;
	GVariant *request_data = g_variant_new("(" service_plugin_start_req_s_type ")", pid, uri, plugin_uri);

	char *reply_type = SAL_IPC_RETURN_TYPE(service_plugin_start_res_s_type);
	GVariant *reply = NULL;

	ret = sal_ipc_client_call_request(request_method, request_data, reply_type, &reply);
	RETVM_IF(SAL_ERROR_NONE != ret, ret, "ipc_client_call_request() Failed(%d)", ret);

	/* Parse response */
	GVariant *reply_data = NULL;
	ret = sal_ipc_client_get_data_response(reply, &reply_data);
	if (!ret && reply_data) {
		__get_plugin_start_res(reply_data, plugin_handle);
		g_variant_unref(reply_data);
	}

	SAL_FN_END;
	return ret;
}

API int ipc_service_plugin_stop(const char *plugin_handle)
{
	SAL_FN_CALL;

	RETV_IF(NULL == plugin_handle, SAL_ERROR_INVALID_PARAMETER);

	int ret = SAL_ERROR_NONE;

	char *request_method = DBUS_SERVICE_PLUGIN_STOP_METHOD;
	GVariant *request_data = g_variant_new("(" service_plugin_stop_s_type ")", plugin_handle);

	char *reply_type = SAL_IPC_SIMPLE_TYPE;
	GVariant *reply = NULL;

	ret = sal_ipc_client_call_request(request_method, request_data, reply_type, &reply);
	RETVM_IF(SAL_ERROR_NONE != ret, ret, "ipc_client_call_request() Failed(%d)", ret);

	ret = sal_ipc_client_get_simple_response(reply);

	SAL_FN_END;
	return ret;
}

