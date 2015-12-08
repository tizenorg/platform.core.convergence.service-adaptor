/*
 * Storage Adaptor
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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>
#include <glib.h>

#include <app.h>

#include "cloud_service.h"
#include "storage_adaptor.h"
#include "service_adaptor_errors.h"
#include "service_adaptor_internal.h"

#include "sal_service_provider.h"
#include "sal_storage_provider.h"

/******************************************************************************
 * Global variables and defines
 ******************************************************************************/

typedef struct _app_control_user_data_s {
	cloud_file_h file;

	void *callback;
	void *user_data;
} app_control_user_data_s;
typedef struct _app_control_user_data_s *app_control_user_data_h;

typedef int (*_remove_file)(void *plugin, const char *cloud_path, cloud_file_cb callback, void *user_data);

/******************************************************************************
 * Private interface
 ******************************************************************************/

/******************************************************************************
 * Private interface definition
 ******************************************************************************/

int _make_cloud_file_by_local(const char *local_path, cloud_file_h *file)
{
	SAL_FN_CALL;

	RETV_IF(NULL == local_path, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	struct stat file_info;
	RETV_IF(0 != stat(local_path, &file_info), SERVICE_ADAPTOR_ERROR_INTERNAL);

	char *path_tmp = strdup(local_path);
	char *dir_path = dirname(path_tmp);

	cloud_file_h cloud_file = (cloud_file_h) g_malloc0(sizeof(cloud_file_s));
	cloud_file->dir_path = dir_path;
	cloud_file->local_path = strdup(local_path);
	cloud_file->cloud_path = NULL;
	cloud_file->size = file_info.st_size;

	if (S_ISDIR(file_info.st_mode)) {
		cloud_file->is_dir = true;
	}

	*file = cloud_file;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

int _make_cloud_file_by_cloud(const char *cloud_path, cloud_file_h *file)
{
	SAL_FN_CALL;

	RETV_IF(NULL == cloud_path, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	cloud_file_h cloud_file = (cloud_file_h) g_malloc0(sizeof(cloud_file_s));
	cloud_file->cloud_path = strdup(cloud_path);

	*file = cloud_file;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

/**
 * @brief callback of service plugin
 *
 * @return      void.
 */
static void _cloud_remove_file_cb(app_control_h request, app_control_h reply, app_control_result_e result, void *user_data)
{
	SAL_FN_CALL;

	RET_IF(APP_CONTROL_RESULT_FAILED == result);
	RET_IF(NULL == user_data);

	char *ret_str = NULL;
	app_control_get_extra_data(reply, PLUGIN_RESULT_KEY, &ret_str);

	app_control_user_data_h app_control_user_data = (app_control_user_data_h) user_data;
	cloud_file_cb callback = (cloud_file_cb) app_control_user_data->callback;

	RET_IF(NULL == callback);

	/* TODO: move this function for chaning result enum to general */
	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	if (0 == strcmp(PLUGIN_RESULT_VALUE_FAILURE, ret_str)) {
		ret = SERVICE_ADAPTOR_ERROR_INTERNAL;
	}

	callback(ret, app_control_user_data->file, app_control_user_data->user_data);

	SAL_FN_END;
}

static int _cloud_remove_file(storage_plugin_h plugin, const char *cloud_path, cloud_file_cb callback, void *user_data)
{
	SAL_FN_CALL;

	RETV_IF(NULL == plugin, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cloud_path, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	app_control_h request;
	app_control_create(&request);

	app_control_set_app_id(request, plugin->uri);
	app_control_set_operation(request, CLOUD_REMOVE_FILE_URI);

	app_control_add_extra_data(request, CLOUD_CLOUD_PATH_KEY, cloud_path);

	cloud_file_h file = NULL;
	_make_cloud_file_by_cloud(cloud_path, &file);
	file->operation = strdup(CLOUD_REMOVE_FILE_URI);

	app_control_user_data_h app_control_user_data = (app_control_user_data_h) g_malloc0(sizeof(app_control_user_data_s));
	app_control_user_data->file = file;
	app_control_user_data->callback = (void *) callback;
	app_control_user_data->user_data = user_data;

	int res = app_control_send_launch_request(request, _cloud_remove_file_cb, app_control_user_data);

	if (APP_CONTROL_ERROR_NONE != res) {
		return SERVICE_ADAPTOR_ERROR_SYSTEM;
	}

	return SERVICE_ADAPTOR_ERROR_NONE;
}

/******************************************************************************
 * Public interface definition
 ******************************************************************************/

API int cloud_register_service(cloud_service_h cloud, GHashTable *service)
{
	SAL_FN_CALL;

	RETV_IF(NULL == cloud, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	int ret = SERVICE_ADAPTOR_ERROR_NO_DATA;
	GHashTableIter iter;
	gpointer iter_key, iter_value;

	g_hash_table_iter_init(&iter, service);
	while (g_hash_table_iter_next(&iter, &iter_key, &iter_value)) {
		if (0 == strcmp(iter_key, CLOUD_REMOVE_FILE_URI)) {
			cloud->cloud_remove_file = (_remove_file) _cloud_remove_file;
			ret = SERVICE_ADAPTOR_ERROR_NONE;
		}
	}

	return ret;
}

API int cloud_unregister_service(cloud_service_h cloud)
{
	SAL_FN_CALL;

	RETV_IF(NULL == cloud, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	cloud->cloud_remove_file = NULL;

	return SERVICE_ADAPTOR_ERROR_NONE;
}
