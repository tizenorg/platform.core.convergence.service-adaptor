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
 * File: service-adaptor-client-auth.c
 * Desc:
 * Created on: Oct, 2014
 * Auth: Yongjin Kim <youth.kim@samsung.com>
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "private/service-adaptor-client-auth.h"
#include "service_adaptor_client_type.h"
#include "service_adaptor_client_log.h"
#include "dbus_client.h"
#include "dbus_client_auth.h"

#include "util/service_adaptor_client_util.h"
/**	@brief	Gets Auth Plugin List
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_get_auth_plugin(service_adaptor_h handle,
						service_adaptor_plugin_h **plugins,
						unsigned int *plugins_len,
						service_adaptor_error_s **error_code)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;
	GList *plugin_list = NULL;
	int len = 0;

	if ((NULL == handle) || (NULL == plugins) || (0 == plugins_len)) {
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER, "Invalid Argument");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	ret = _dbus_get_auth_plugin_list(&plugin_list, handle->imsi ? handle->imsi : "", &error);
	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		_assign_error_code(&error, error_code);
	}

	int size = g_list_length(plugin_list);
	service_adaptor_plugin_h *plugins_array = (service_adaptor_plugin_h *) calloc(size, sizeof(service_adaptor_plugin_h));

	if (NULL == plugins_array) {
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_UNKNOWN, "Memory allocation failed");
		return SERVICE_ADAPTOR_ERROR_UNKNOWN;
	}

	for (GList *list = g_list_first(plugin_list); list != NULL; list = g_list_next(list)) {
		service_adaptor_plugin_h plugin = (service_adaptor_plugin_h) list->data;

		if (NULL == plugin) {
			continue;
		}

		plugins_array[len] = (service_adaptor_plugin_h) calloc(1, sizeof(service_adaptor_plugin_s));
		if (NULL != plugins_array[len]) {
			plugins_array[len]->name = _safe_strdup(plugin->name);
			plugins_array[len]->login = plugin->login;
		}
		len++;
	}

	*plugins = plugins_array;
	*plugins_len = size;

	sac_api_end(ret);
	return ret;
}

/**     @brief  Logins Auth Plugin using appId and appSecret
 *      @return int
 *      @remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_set_auth_plugin(service_adaptor_h handle,
						service_adaptor_plugin_h plugin_handle)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_plugin_h plugin = NULL;

	if ((NULL == handle) || (NULL == plugin_handle) || (NULL == plugin_handle->name)) {
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	plugin = (service_adaptor_plugin_h) calloc(1, sizeof(service_adaptor_plugin_s));

	if (NULL == plugin) {
		return SERVICE_ADAPTOR_ERROR_UNKNOWN;
	}

	plugin->name = _safe_strdup(plugin_handle->name);
	plugin->login = plugin_handle->login;

	handle->plugin = plugin;

	sac_api_end(ret);
	return ret;
}

/**	@brief	Renuests Channel Auth
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_set_auth(service_adaptor_h handle,
						const char *service_name,
						const char *app_id,
						const char *app_secret,
						unsigned int service_id,
						service_adaptor_error_s **error_code,
						void *user_data)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;

	if ((NULL == handle) || (NULL == handle->imsi) || (NULL == handle->plugin) || (NULL == handle->plugin->name) || (NULL == service_name) || (NULL == app_id) || (NULL == app_secret)) {
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER, "Invalid Argument");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	if (NULL != handle->service_name) {
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_STATE, "Duplicated handle usage (Destroy and create new handle)");
		return SERVICE_ADAPTOR_ERROR_INVALID_STATE;
	}

	char *_service_name = strdup(service_name);
	char *_app_id = strdup(app_id);
	if ((NULL == _service_name) || (NULL == _app_id)) {
		free(_service_name);
		free(_app_id);
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_UNKNOWN, "Memory allocation failed");
		return SERVICE_ADAPTOR_ERROR_UNKNOWN;
	}

	sac_debug("set_auth mutex lock");
	g_mutex_lock(&handle->set_auth_mutex);
	ret = _dbus_set_auth(service_name, handle->imsi, handle->plugin->name, app_id, app_secret, service_id, user_data, &error);
	g_mutex_unlock(&handle->set_auth_mutex);
	sac_debug("set_auth mutex unlock");

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		sac_info("set_auth API failed: %d (%s)", ret, _service_name);
		free(_service_name);
		free(_app_id);
		_assign_error_code(&error, error_code);
	} else {
		sac_info("set_auth API success (%s)", _service_name);
		handle->service_name = _service_name;
		handle->app_id = _app_id;
		handle->service_id = service_id;
	}

	sac_api_end(ret);
	return ret;
}

