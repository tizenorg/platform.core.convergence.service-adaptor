/*
 * Storage Plugin Client
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

#include <app.h>

#include "service_adaptor_errors.h"
#include "service_adaptor_internal.h"
#include "cloud_service.h"
#include "sal_service_provider.h"
#include "sal_storage_provider.h"

API int storage_provider_create(storage_provider_h *provider)
{
	SAL_FN_CALL;

	storage_provider_h storage_provider = (storage_provider_h) g_malloc0(sizeof(storage_provider_s));

	storage_provider->cloud_remove_file = NULL;

	*provider = storage_provider;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API app_control_h storage_provider_message(storage_provider_h provider, const char *operation, void *user_data)
{
	SAL_FN_CALL;

	app_control_h reply = NULL;

	RETV_IF(NULL == provider, reply);

	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	if (0 == strcmp(operation, CLOUD_REMOVE_FILE_URI)) {
		app_control_create(&reply);

		char *cloud_path = NULL;
		app_control_get_extra_data((app_control_h) user_data, CLOUD_CLOUD_PATH_KEY, &cloud_path);

		ret = provider->cloud_remove_file(cloud_path);

		if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
			SAL_ERR("cloud_remove_file() Fail (%d)", ret);
			app_control_add_extra_data(reply, PLUGIN_RESULT_KEY, PLUGIN_RESULT_VALUE_FAILURE);
			return reply;
		}

		app_control_add_extra_data(reply, PLUGIN_RESULT_KEY, PLUGIN_RESULT_VALUE_SUCCESS);
	}

	return reply;
}

API int storage_provider_add_extra_data(storage_provider_h provider, app_control_h reply)
{
	SAL_FN_CALL;

	RETV_IF(NULL == provider, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == reply, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	app_control_add_extra_data(reply, PLUGIN_KEY_STORAGE, PLUGIN_VALUE_TRUE);

	if (NULL != provider->cloud_remove_file) {
		app_control_add_extra_data(reply, CLOUD_REMOVE_FILE_URI, PLUGIN_VALUE_TRUE);
	}

	return SERVICE_ADAPTOR_ERROR_NONE;
}
