/*
 * Resource Adaptor
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

#ifndef __RESOURCE_ADAPTOR_H__
#define __RESOURCE_ADAPTOR_H__

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

/**
 * @file resource_adaptor.h
 */

/**
 * @ingroup
 * @defgroup
 *
 * @brief
 *
 * @section
 *  \#include <resource_adaptor.h>
 *
 * <BR>
 * @{
 */

/**
 * @brief Handle of Resource Plugin
 */
typedef struct resource_plugin_s *resource_plugin_h;

/**
 * @brief Describes infromation about SmartThings Service
 * @key access token, ...
 */
 typedef struct _resource_plugin_smartthings_service_s
 {
	int (*smartthings_get_resource)(resource_plugin_h plugin, char **resource_uri, void *user_data);
 } resource_plugin_smartthings_service_s;
 typedef struct resource_plugin_smartthings_service_s *resource_plugin_smartthings_service_h;

/**
 * @brief Describes infromation about Resource Plugin
 */
typedef struct _resource_plugin_s
{
	char *uri;
	char *name;

	resource_plugin_smartthings_service_h smartthings;

	GMutex mutex;
	int start;
} resource_plugin_s;

/**
 * @brief Describes infromation about Resource Adaptor
 */
typedef struct _resource_adaptor_s
{
	GList *plugins;		// resource_plugin_h

	GMutex mutex;
	int start;
} resource_adaptor_s;
typedef struct resource_adaptor_s *resource_adaptor_h;

/**
 * @brief Describes infromation about Resource Adaptor Listener
 */
typedef struct _resource_adaptor_listener_s
{
	void (*login_cb)(char *uri, void *user_data);
} resource_adaptor_listener_s;
typedef struct resource_adaptor_listener_s *resource_adaptor_listener_h;

resource_adaptor_h resource_adaptor_create();
service_adaptor_error_e resource_adaptor_destroy(resource_adaptor_h resource);
service_adaptor_error_e resource_adaptor_start(resource_adaptor_h resource);
service_adaptor_error_e resource_adaptor_stop(resource_adaptor_h resource);
service_adaptor_error_e resource_adaptor_register_listener(resource_adaptor_h resource, resource_adaptor_listener_h listener);
service_adaptor_error_e resource_adaptor_unregister_listener(resource_adaptor_h resource, resource_adaptor_listener_h listener);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __RESOURCE_ADAPTOR_H__ */
