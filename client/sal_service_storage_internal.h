/*
 * Service Storage Internal
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

#ifndef __SERVICE_STORAGE_INTERNAL_H__
#define __SERVICE_STORAGE_INTERNAL_H__

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#include <glib.h>

#include "sal_service_adaptor.h"
#include "sal_service_storage.h"

typedef struct _service_storage_cloud_file_s
{
	service_plugin_h plugin;
	service_storage_cloud_file_cb callback;
	void *user_data;

	bool is_dir;
	char *dir_path;
	char *local_path;
	char *cloud_path;
	unsigned long long size;
	char *operation;

	GList *files;
} service_storage_cloud_file_s;

/*==================================================================================================
                                         FUNCTION PROTOTYPES
==================================================================================================*/

int service_storage_cloud_start(service_storage_cloud_file_h file);
int service_storage_cloud_stop(service_storage_cloud_file_h file);

#ifdef __cplusplus
}
#endif

#endif /* __SERVICE_STORAGE_INTERNAL_H__ */
