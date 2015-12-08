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

#include <stdio.h>
#include <glib.h>

#include "auth_adaptor.h"
#include "service_adaptor_internal.h"

/******************************************************************************
 * Global variables and defines
 ******************************************************************************/

/******************************************************************************
 * Private interface
 ******************************************************************************/

/******************************************************************************
 * Private interface definition
 ******************************************************************************/

static void _auth_adaptor_free_plugin(auth_plugin_h plugin)
{
}

/******************************************************************************
 * Public interface definition
 ******************************************************************************/

API auth_adaptor_h auth_adaptor_create()
{
	SAL_FN_CALL;

	auth_adaptor_h auth = (auth_adaptor_h) g_malloc0(sizeof(auth_adaptor_s));

	g_mutex_init(&auth->mutex);

	return auth;
}

API service_adaptor_error_e auth_adaptor_destroy(auth_adaptor_h auth)
{
	SAL_FN_CALL;

	RETV_IF(NULL == auth, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	g_mutex_lock(&auth->mutex);

	if (0 != auth->start) {
		auth_adaptor_stop(auth);
	}

	SAL_FREE(auth);

	g_mutex_unlock(&auth->mutex);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API service_adaptor_error_e auth_adaptor_start(auth_adaptor_h auth)
{
	SAL_FN_CALL;

	RETV_IF(NULL == auth, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	g_mutex_lock(&auth->mutex);

	auth->start = 1;

	g_mutex_unlock(&auth->mutex);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API service_adaptor_error_e auth_adaptor_stop(auth_adaptor_h auth)
{
	SAL_FN_CALL;

	RETV_IF(NULL == auth, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	/* TODO: notify auth adaptor stop to each plugin */

	g_mutex_lock(&auth->mutex);

	auth->start = 0;

	g_mutex_unlock(&auth->mutex);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API service_adaptor_error_e auth_adaptor_register_listener(auth_adaptor_h auth, auth_adaptor_listener_h listener)
{
	SAL_FN_CALL;

	RETV_IF(NULL == auth, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == listener, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	/* TODO: register insert/ update/ delete callback for service-adaptor */

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API service_adaptor_error_e auth_adaptor_unregister_listener(auth_adaptor_h auth, auth_adaptor_listener_h listener)
{
	SAL_FN_CALL;

	RETV_IF(NULL == auth, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == listener, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	/* TODO: unregister insert/ update/ delete callback for service-adaptor */

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API service_adaptor_error_e auth_adaptor_create_plugin(const char *uri, const char *name, const char *package, auth_plugin_h *plugin)
{
	SAL_FN_CALL;

	RETV_IF(NULL == uri, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == name, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == package, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	auth_plugin_h auth_plugin = (auth_plugin_h) g_malloc0(sizeof(auth_plugin_s));
	auth_plugin->uri = strdup(uri);
	auth_plugin->name = strdup(name);
	auth_plugin->package = strdup(package);

	g_mutex_init(&auth_plugin->mutex);
	g_cond_init(&auth_plugin->cond);

	*plugin = auth_plugin;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API service_adaptor_error_e auth_adaptor_destroy_plugin(auth_plugin_h plugin)
{
	SAL_FN_CALL;

	RETV_IF(NULL == plugin, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	SAL_FREE(plugin->uri);
	SAL_FREE(plugin->name);
	SAL_FREE(plugin);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API service_adaptor_error_e auth_adaptor_register_plugin_service(auth_plugin_h plugin, GHashTable *service)
{
	SAL_FN_CALL;

	RETV_IF(NULL == plugin, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == service, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	plugin->oauth1 = (oauth1_service_h) g_malloc0(sizeof(oauth1_service_s));
	ret = oauth1_register_service(plugin->oauth1, service);

	if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
		SAL_INFO("could not find the function for oauth 1.0");
		SAL_FREE(plugin->oauth1);
	}

	plugin->oauth2 = (oauth2_service_h) g_malloc0(sizeof(oauth2_service_s));
	ret = oauth2_register_service(plugin->oauth2, service);

	if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
		SAL_INFO("could not find the function for oauth 2.0");
		SAL_FREE(plugin->oauth2);
	}

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API service_adaptor_error_e auth_adaptor_unregister_plugin_service(auth_plugin_h plugin)
{
	SAL_FN_CALL;

	RETV_IF(NULL == plugin, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	if (NULL != plugin->oauth1) {
		oauth1_unregister_service(plugin->oauth1);
		SAL_FREE(plugin->oauth1);
	}

	if (NULL != plugin->oauth2) {
		oauth2_unregister_service(plugin->oauth2);
		SAL_FREE(plugin->oauth2);
	}

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API service_adaptor_error_e auth_adaptor_add_plugin(auth_adaptor_h auth, auth_plugin_h plugin)
{
	SAL_FN_CALL;

	RETV_IF(NULL == auth, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == plugin, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	g_mutex_lock(&auth->mutex);

	auth->plugins = g_list_append(auth->plugins, plugin);

	g_mutex_unlock(&auth->mutex);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API service_adaptor_error_e auth_adaptor_remove_plugin(auth_adaptor_h auth, auth_plugin_h plugin)
{
	SAL_FN_CALL;

	RETV_IF(NULL == auth, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == plugin, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	g_mutex_lock(&auth->mutex);

	RETV_IF(NULL == g_list_find(auth->plugins, plugin), SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	auth->plugins = g_list_remove(auth->plugins, plugin);

	_auth_adaptor_free_plugin(plugin);

	g_mutex_unlock(&auth->mutex);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API auth_plugin_h auth_adaptor_get_plugin(auth_adaptor_h auth, const char *uri)
{
	SAL_FN_CALL;

	RETV_IF(NULL == auth, NULL);
	RETV_IF(NULL == uri, NULL);

	g_mutex_lock(&auth->mutex);

	auth_plugin_h plugin = NULL;

	for (GList *list = g_list_first(auth->plugins); list != NULL; list = list->next) {
		auth_plugin_h this = (auth_plugin_h) list->data;

		if (0 == strcmp(this->uri, uri)) {
			plugin = this;
			break;
		}
	}

	g_mutex_unlock(&auth->mutex);

	return plugin;
}

API char *auth_adaptor_get_uri(auth_adaptor_h auth, const char *package)
{
	SAL_FN_CALL;

	RETV_IF(NULL == auth, NULL);
	RETV_IF(NULL == package, NULL);

	g_mutex_lock(&auth->mutex);

	char *uri = NULL;

	for (GList *list = g_list_first(auth->plugins); list != NULL; list = list->next) {
		auth_plugin_h this = (auth_plugin_h) list->data;

		if (0 == strcmp(this->package, package)) {
			uri = this->uri;
			break;
		}
	}

	g_mutex_unlock(&auth->mutex);

	return uri;
}

API service_adaptor_error_e auth_adaptor_ref_plugin(auth_adaptor_h auth, const char *uri)
{
	SAL_FN_CALL;

	RETV_IF(NULL == auth, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == uri, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	g_mutex_lock(&auth->mutex);

	int ret = SERVICE_ADAPTOR_ERROR_NO_DATA;

	for (GList *list = g_list_first(auth->plugins); list != NULL; list = list->next) {
		auth_plugin_h this = (auth_plugin_h) list->data;

		if (0 == strcmp(this->uri, uri)) {
			ret = SERVICE_ADAPTOR_ERROR_NONE;
			/* TODO: increase ref count */
			break;
		}
	}

	g_mutex_unlock(&auth->mutex);

	return ret;
}

API service_adaptor_error_e auth_adaptor_unref_plugin(auth_adaptor_h auth, const char *uri)
{
	SAL_FN_CALL;

	RETV_IF(NULL == auth, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == uri, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	g_mutex_lock(&auth->mutex);

	int ret = SERVICE_ADAPTOR_ERROR_NO_DATA;

	for (GList *list = g_list_first(auth->plugins); list != NULL; list = list->next) {
		auth_plugin_h this = (auth_plugin_h) list->data;

		if (0 == strcmp(this->uri, uri)) {
			ret = SERVICE_ADAPTOR_ERROR_NONE;
			/* TODO: decrease ref count */
			break;
		}
	}

	g_mutex_unlock(&auth->mutex);

	return ret;
}
