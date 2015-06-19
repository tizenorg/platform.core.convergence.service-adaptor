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

#ifndef __STORAGE_ADAPTOR_H__
#define __STORAGE_ADAPTOR_H__

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <glib.h>

#include "service_adaptor_errors.h"
#include "cloud_service.h"
//#include "posix_service.h"

/**
 * @file storage_adaptor.h
 */

/**
 * @ingroup
 * @defgroup
 *
 * @brief
 *
 * @section
 *  \#include <storage_adaptor.h>
 *
 * <BR>
 * @{
 */

#define URI_STORAGE	"storage"
#define URI_CLOUD	"storage/cloud"
#define URI_POSIX	"storage/posix"

/**
 * @brief Describes infromation about Storage Spec
 */
typedef enum _storage_spec_e
{
	STORAGE_SPEC_CLOUD      = (1 << 0),
	STORAGE_SPEC_POSIX      = (1 << 1),
} storage_spec_e;

/**
 * @brief Describes infromation about Storage Plugin
 */
typedef struct _storage_plugin_s
{
	char *uri;
	char *name;
	char *package;

	cloud_service_h cloud;
//	posix_service_h posix;

	GMutex mutex;
	GCond cond;
} storage_plugin_s;
typedef struct _storage_plugin_s *storage_plugin_h;

/**
 * @brief Describes infromation about Storage Adaptor
 */
typedef struct _storage_adaptor_s
{
	GList *plugins;		// storage_plugin_h

	GMutex mutex;
	int start;
} storage_adaptor_s;
typedef struct _storage_adaptor_s *storage_adaptor_h;

/**
 * @brief Describes infromation about Storage Adaptor Listener
 */
typedef struct _storage_adaptor_listener_s
{
	void (*create_file_cb)(const char *uri, const char *path, void *user_data);
} storage_adaptor_listener_s;
typedef struct _storage_adaptor_listener_s *storage_adaptor_listener_h;

storage_adaptor_h storage_adaptor_create();
service_adaptor_error_e storage_adaptor_destroy(storage_adaptor_h storage);
service_adaptor_error_e storage_adaptor_start(storage_adaptor_h storage);
service_adaptor_error_e storage_adaptor_stop(storage_adaptor_h storage);
service_adaptor_error_e storage_adaptor_register_listener(storage_adaptor_h storage, storage_adaptor_listener_h listener);
service_adaptor_error_e storage_adaptor_unregister_listener(storage_adaptor_h storage, storage_adaptor_listener_h listener);
service_adaptor_error_e storage_adaptor_create_plugin(const char *uri, const char *name, const char *package, storage_plugin_h *plugin);
service_adaptor_error_e storage_adaptor_destroy_plugin(storage_plugin_h plugin);
service_adaptor_error_e storage_adaptor_register_plugin_service(storage_plugin_h plugin, GHashTable *service);
service_adaptor_error_e storage_adaptor_unregister_plugin_service(storage_plugin_h plugin);
service_adaptor_error_e storage_adaptor_add_plugin(storage_adaptor_h storage, storage_plugin_h plugin);
service_adaptor_error_e storage_adaptor_remove_plugin(storage_adaptor_h storage, storage_plugin_h plugin);
storage_plugin_h storage_adaptor_get_plugin(storage_adaptor_h storage, const char *uri);
char *storage_adaptor_get_uri(storage_adaptor_h storage, const char *package);
service_adaptor_error_e storage_adaptor_ref_plugin(storage_adaptor_h storage, const char *uri);
service_adaptor_error_e storage_adaptor_unref_plugin(storage_adaptor_h storage, const char *uri);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __STORAGE_ADAPTOR_H__ */
