/*
 * Auth Adaptor
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

#ifndef __AUTH_ADAPTOR_H__
#define __AUTH_ADAPTOR_H__

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <glib.h>
#include <glib-object.h>
#include <glib-unix.h>

#include "service_adaptor_errors.h"
#include "oauth1_service.h"
#include "oauth2_service.h"

/**
 * @file auth_adaptor.h
 */

/**
 * @ingroup
 * @defgroup
 *
 * @brief
 *
 * @section
 *  \#include <auth_adaptor.h>
 *
 * <BR>
 * @{
 */

#define URI_AUTH	"auth"
#define URI_OAUTH1_0	"auth/oauth1.0"
#define URI_OAUTH2_0	"auth/oauth2.0"

/**
 * @brief Describes infromation about Auth Spec
 */
typedef enum _auth_spec_e
{
	AUTH_SPEC_OAUTH1_0	= (1 << 0),
	AUTH_SPEC_OAUTH2_0	= (1 << 1),
} auth_spec_e;

/**
 * @brief Describes infromation about Auth Plugin
 */
typedef struct _auth_plugin_s
{
	char *uri;
	char *name;
	char *package;

	oauth1_service_h oauth1;
	oauth2_service_h oauth2;

	GMutex mutex;
	GCond cond;
	int login;
} auth_plugin_s;
typedef struct _auth_plugin_s *auth_plugin_h;

/**
 * @brief Describes infromation about Auth Adaptor
 */
typedef struct _auth_adaptor_s
{
	GList *plugins;		// auth_plugin_h

	GMutex mutex;
	int start;
} auth_adaptor_s;
typedef struct _auth_adaptor_s *auth_adaptor_h;

/**
 * @brief Describes infromation about Auth Adaptor Listener
 */
typedef struct _auth_adaptor_listener_s
{
	void (*login_cb)(const char *uri, void *user_data);
} auth_adaptor_listener_s;
typedef struct _auth_adaptor_listener_s *auth_adaptor_listener_h;

auth_adaptor_h auth_adaptor_create();
service_adaptor_error_e auth_adaptor_destroy(auth_adaptor_h auth);
service_adaptor_error_e auth_adaptor_start(auth_adaptor_h auth);
service_adaptor_error_e auth_adaptor_stop(auth_adaptor_h auth);
service_adaptor_error_e auth_adaptor_register_listener(auth_adaptor_h auth, auth_adaptor_listener_h listener);
service_adaptor_error_e auth_adaptor_unregister_listener(auth_adaptor_h auth, auth_adaptor_listener_h listener);
service_adaptor_error_e auth_adaptor_create_plugin(const char *uri, const char *name, const char *package,  auth_plugin_h *plugin);
service_adaptor_error_e auth_adaptor_destroy_plugin(auth_plugin_h plugin);
service_adaptor_error_e auth_adaptor_register_plugin_service(auth_plugin_h plugin, GHashTable *service);
service_adaptor_error_e auth_adaptor_unregister_plugin_service(auth_plugin_h plugin);
service_adaptor_error_e auth_adaptor_add_plugin(auth_adaptor_h auth, auth_plugin_h plugin);
service_adaptor_error_e auth_adaptor_remove_plugin(auth_adaptor_h auth, auth_plugin_h plugin);
auth_plugin_h auth_adaptor_get_plugin(auth_adaptor_h auth, const char *uri);
char *auth_adaptor_get_uri(auth_adaptor_h auth, const char *package);
service_adaptor_error_e auth_adaptor_ref_plugin(auth_adaptor_h auth, const char *uri);
service_adaptor_error_e auth_adaptor_unref_plugin(auth_adaptor_h auth, const char *uri);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __AUTH_ADAPTOR_H__ */
