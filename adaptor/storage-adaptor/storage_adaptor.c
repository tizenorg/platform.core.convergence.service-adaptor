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
#include <glib.h>

#include "storage_adaptor.h"
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

static void _storage_adaptor_free_plugin(storage_plugin_h plugin)
{
}

/******************************************************************************
 * Public interface definition
 ******************************************************************************/

API storage_adaptor_h storage_adaptor_create()
{
	SAL_FN_CALL;

	storage_adaptor_h storage = (storage_adaptor_h) g_malloc0(sizeof(storage_adaptor_s));

	g_mutex_init(&storage->mutex);

	return storage;
}

API service_adaptor_error_e storage_adaptor_destroy(storage_adaptor_h storage)
{
	SAL_FN_CALL;

	RETV_IF(NULL == storage, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	g_mutex_lock(&storage->mutex);

	if (0 != storage->start) {
		storage_adaptor_stop(storage);
	}

	SAL_FREE(storage);

	g_mutex_unlock(&storage->mutex);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API service_adaptor_error_e storage_adaptor_start(storage_adaptor_h storage)
{
	SAL_FN_CALL;

	RETV_IF(NULL == storage, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	g_mutex_lock(&storage->mutex);

	storage->start = 1;

	g_mutex_unlock(&storage->mutex);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API service_adaptor_error_e storage_adaptor_stop(storage_adaptor_h storage)
{
	SAL_FN_CALL;

	RETV_IF(NULL == storage, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	/* TODO: notify storage adaptor stop to each plugin */

	g_mutex_lock(&storage->mutex);

	storage->start = 0;

	g_mutex_unlock(&storage->mutex);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API service_adaptor_error_e storage_adaptor_register_listener(storage_adaptor_h storage, storage_adaptor_listener_h listener)
{
	SAL_FN_CALL;

	RETV_IF(NULL == storage, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == listener, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	/* TODO: register insert/ update/ delete callback for service-adaptor */

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API service_adaptor_error_e storage_adaptor_unregister_listener(storage_adaptor_h storage, storage_adaptor_listener_h listener)
{
	SAL_FN_CALL;

	RETV_IF(NULL == storage, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == listener, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	/* TODO: unregister insert/ update/ delete callback for service-adaptor */

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API service_adaptor_error_e storage_adaptor_create_plugin(const char *uri, const char *name, const char *package, storage_plugin_h *plugin)
{
	SAL_FN_CALL;

	RETV_IF(NULL == uri, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == name, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == package, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	storage_plugin_h storage_plugin = (storage_plugin_h) g_malloc0(sizeof(storage_plugin_s));
	storage_plugin->uri = strdup(uri);
	storage_plugin->name = strdup(name);
	storage_plugin->package = strdup(package);

	g_mutex_init(&storage_plugin->mutex);
	g_cond_init(&storage_plugin->cond);

	*plugin = storage_plugin;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API service_adaptor_error_e storage_adaptor_destroy_plugin(storage_plugin_h plugin)
{
	SAL_FN_CALL;

	RETV_IF(NULL == plugin, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	SAL_FREE(plugin->uri);
	SAL_FREE(plugin->name);
	SAL_FREE(plugin);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API service_adaptor_error_e storage_adaptor_register_plugin_service(storage_plugin_h plugin, GHashTable *service)
{
	SAL_FN_CALL;

	RETV_IF(NULL == plugin, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == service, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	plugin->cloud = (cloud_service_h) g_malloc0(sizeof(cloud_service_s));
	ret = cloud_register_service(plugin->cloud, service);

	if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
		SAL_INFO("could not find the function for cloud");
		SAL_FREE(plugin->cloud);
	}
/*
	plugin->posix = (posix_service_h) g_malloc0(sizeof(posix_service_s));
	ret = posix_register_service(plugin->posix, service);

	if (SERVICE_ADAPTOR_ERROR_NONE != ret)
	{
		SAL_INFO("could not find the function for posix");
		SAL_FREE(plugin->posix);
	}
	*/
	return SERVICE_ADAPTOR_ERROR_NONE;
}

API service_adaptor_error_e storage_adaptor_unregister_plugin_service(storage_plugin_h plugin)
{
	SAL_FN_CALL;

	RETV_IF(NULL == plugin, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	if (NULL != plugin->cloud) {
		cloud_unregister_service(plugin->cloud);
		SAL_FREE(plugin->cloud);
	}
	/*
	if (NULL != plugin->posix)
	{
		posix_unregister_service(plugin->posix);
		SAL_FREE(plugin->posix);
	}
	*/
	return SERVICE_ADAPTOR_ERROR_NONE;
}

API service_adaptor_error_e storage_adaptor_add_plugin(storage_adaptor_h storage, storage_plugin_h plugin)
{
	SAL_FN_CALL;

	RETV_IF(NULL == storage, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == plugin, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	g_mutex_lock(&storage->mutex);

	storage->plugins = g_list_append(storage->plugins, plugin);

	g_mutex_unlock(&storage->mutex);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API service_adaptor_error_e storage_adaptor_remove_plugin(storage_adaptor_h storage, storage_plugin_h plugin)
{
	SAL_FN_CALL;

	RETV_IF(NULL == storage, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == plugin, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	g_mutex_lock(&storage->mutex);

	RETV_IF(NULL == g_list_find(storage->plugins, plugin), SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	storage->plugins = g_list_remove(storage->plugins, plugin);

	_storage_adaptor_free_plugin(plugin);

	g_mutex_unlock(&storage->mutex);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API storage_plugin_h storage_adaptor_get_plugin(storage_adaptor_h storage, const char *uri)
{
	SAL_FN_CALL;

	RETV_IF(NULL == storage, NULL);
	RETV_IF(NULL == uri, NULL);

	g_mutex_lock(&storage->mutex);

	storage_plugin_h plugin = NULL;

	for (GList *list = g_list_first(storage->plugins); list != NULL; list = list->next) {
		storage_plugin_h this = (storage_plugin_h) list->data;

		if (0 == strcmp(this->uri, uri)) {
			plugin = this;
			break;
		}
	}

	g_mutex_unlock(&storage->mutex);

	return plugin;
}

API char *storage_adaptor_get_uri(storage_adaptor_h storage, const char *package)
{
	SAL_FN_CALL;

	RETV_IF(NULL == storage, NULL);
	RETV_IF(NULL == package, NULL);

	g_mutex_lock(&storage->mutex);

	char *uri = NULL;

	for (GList *list = g_list_first(storage->plugins); list != NULL; list = list->next) {
		storage_plugin_h this = (storage_plugin_h) list->data;

		if (0 == strcmp(this->package, package)) {
			uri = this->uri;
			break;
		}
	}

	g_mutex_unlock(&storage->mutex);

	return uri;
}

API service_adaptor_error_e storage_adaptor_ref_plugin(storage_adaptor_h storage, const char *uri)
{
	SAL_FN_CALL;

	RETV_IF(NULL == storage, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == uri, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	g_mutex_lock(&storage->mutex);

	int ret = SERVICE_ADAPTOR_ERROR_NO_DATA;

	for (GList *list = g_list_first(storage->plugins); list != NULL; list = list->next) {
		storage_plugin_h this = (storage_plugin_h) list->data;

		if (0 == strcmp(this->uri, uri)) {
			ret = SERVICE_ADAPTOR_ERROR_NONE;
			/* TODO: increase ref count */
			break;
		}
	}

	g_mutex_unlock(&storage->mutex);

	return ret;
}

API service_adaptor_error_e storage_adaptor_unref_plugin(storage_adaptor_h storage, const char *uri)
{
	SAL_FN_CALL;

	RETV_IF(NULL == storage, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == uri, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	g_mutex_lock(&storage->mutex);

	int ret = SERVICE_ADAPTOR_ERROR_NO_DATA;

	for (GList *list = g_list_first(storage->plugins); list != NULL; list = list->next) {
		storage_plugin_h this = (storage_plugin_h) list->data;

		if (0 == strcmp(this->uri, uri)) {
			ret = SERVICE_ADAPTOR_ERROR_NONE;
			/* TODO: decrease ref count */
			break;
		}
	}

	g_mutex_unlock(&storage->mutex);

	return ret;
}
