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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <dlfcn.h>
#include <glib.h>

#include "shop-adaptor.h"
#include "shop-adaptor-log.h"

/**
 * Shop adaptor plugin
 */
typedef struct shop_adaptor_plugin_s {
	shop_adaptor_h			adaptor;			/* Adaptor */
	char				*path;				/* Plugin library path */
	shop_adaptor_plugin_handle_h	handle;				/* Plugin handle */
	void				*dl_handle;			/* Plugin library handle */
	int				ref_counter;			/* Plugin reference counter */
	GMutex				ref_counter_mutex;		/* Plugin reference counter mutex */
	shop_adaptor_plugin_listener_h	plugin_listener;		/* Plugin callback listener */
	GMutex				plugin_listener_mutex;		/* Plugin callback listener mutex */
} shop_adaptor_plugin_t;

/**
 * Shop adaptor
 */
typedef struct shop_adaptor_s {
	GMutex	shop_adaptor_mutex;		/* Adaptor mutex */
	int	started;			/* Started flag */
	char	*plugins_dir;			/* Plugins directory path */
	GList	*plugins;			/* List of loaded plugins */
	GMutex	plugins_mutex;			/* Plugin list mutex */
	GList	*adaptor_listeners;		/* List of vservice channel listener (for now not effective) */
	GMutex	adaptor_listeners_mutex;	/* Listener list mutex */
} shop_adaptor_t;

/**
 * Creates plugin
 */
static shop_adaptor_plugin_h shop_adaptor_create_plugin(const char *plugin_path);

/**
 * Destroys plugin and deletes all resources associated with it
 */
static void shop_adaptor_destroy_plugin(shop_adaptor_plugin_h plugin);

/**
 * Loads plugins from selected directory
 */
static int shop_adaptor_load_plugins_from_directory(shop_adaptor_h adaptor, const char *dir_path);

/**
 * Checks if plugin is loaded by selected plugin adaptor
 */
static int shop_adaptor_has_plugin(shop_adaptor_h adaptor, shop_adaptor_plugin_h plugin);

/**
 * Increases adaptor's plugin references counter
 */
static void shop_adaptor_plugin_ref(shop_adaptor_plugin_h);

/**
 * Decreases adaptor's plugin references counter
 */
static void shop_adaptor_plugin_unref(shop_adaptor_plugin_h);

/* //------------------------------------------------------------------------
   // Functions implementations
   //------------------------------------------------------------------------ */

/* //////////////////////////////////////////////////////
   // Mandatory: External adaptor management function
   ////////////////////////////////////////////////////// */

EXPORT_API
shop_adaptor_h shop_adaptor_create(const char *plugins_dir)
{
	shop_adaptor_h shop_adaptor = (shop_adaptor_h) malloc(sizeof(shop_adaptor_t));
	if (NULL == shop_adaptor) {
	    return NULL;
	}

	shop_adaptor->started = 0;
	shop_adaptor->plugins_dir = strdup(plugins_dir);

	g_mutex_init(&shop_adaptor->shop_adaptor_mutex);
	g_mutex_init(&shop_adaptor->plugins_mutex);
	g_mutex_init(&shop_adaptor->adaptor_listeners_mutex);

	g_mutex_lock(&shop_adaptor->adaptor_listeners_mutex);
	shop_adaptor->adaptor_listeners = NULL;
	g_mutex_unlock(&shop_adaptor->adaptor_listeners_mutex);

	g_mutex_lock(&shop_adaptor->plugins_mutex);
	shop_adaptor->plugins = NULL;
	g_mutex_unlock(&shop_adaptor->plugins_mutex);

	return shop_adaptor;

}

EXPORT_API
void shop_adaptor_destroy(shop_adaptor_h adaptor)
{
	if (NULL == adaptor) {
		shop_adaptor_error("Invalid argument");
		return ;
	}

	g_mutex_lock(&adaptor->shop_adaptor_mutex);
	if (adaptor->started) {
		shop_adaptor_error("Shop adaptor is running. Forcing stop before destroy");
		shop_adaptor_stop(adaptor);
	}

	g_mutex_lock(&adaptor->plugins_mutex);
	if (NULL != adaptor->plugins) {
		g_list_free_full(adaptor->plugins, (GDestroyNotify) shop_adaptor_plugin_unref);
		adaptor->plugins = NULL;
	}
	g_mutex_unlock(&adaptor->plugins_mutex);

	g_mutex_lock(&adaptor->adaptor_listeners_mutex);
	if (NULL != adaptor->adaptor_listeners) {
		g_list_free(adaptor->adaptor_listeners);
		adaptor->adaptor_listeners = NULL;
	}
	g_mutex_unlock(&adaptor->adaptor_listeners_mutex);

	/*TODO */
	/*please add destroying lisners if it needed, currently it is not needed */

	free(adaptor->plugins_dir);
	adaptor->plugins_dir = NULL;

	g_mutex_unlock(&adaptor->shop_adaptor_mutex);

	free(adaptor);
}

EXPORT_API
int shop_adaptor_start(shop_adaptor_h adaptor)
{
	shop_adaptor_debug("Starting shop adaptor");
	if (NULL == adaptor) {
		shop_adaptor_error("Invalid argument");
		return SHOP_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	g_mutex_lock(&adaptor->shop_adaptor_mutex);
	int result = SHOP_ADAPTOR_ERROR_NONE;
	if (adaptor->started) {
		shop_adaptor_error("Storage adaptor is already started");
		result = SHOP_ADAPTOR_ERROR_START;
	} else {
		adaptor->started = 1;
		result = shop_adaptor_load_plugins_from_directory(adaptor, adaptor->plugins_dir);
		if (SHOP_ADAPTOR_ERROR_NONE != result) {
			adaptor->started = 0;
			shop_adaptor_error("Could not load plugins from directory");
		} else {
			shop_adaptor_debug("Storage adaptor started successfully");
		}
	}
	g_mutex_unlock(&adaptor->shop_adaptor_mutex);

	return result;
}

/**
 * Stops shop adaptor.
 */
EXPORT_API
int shop_adaptor_stop(shop_adaptor_h adaptor)
{
	if (NULL == adaptor) {
		shop_adaptor_error("Invalid argument");
		return SHOP_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	g_mutex_lock(&adaptor->shop_adaptor_mutex);
	int result = SHOP_ADAPTOR_ERROR_NONE;
	if (!adaptor->started) {
		result = SHOP_ADAPTOR_ERROR_START;
	} else {
		if (NULL != adaptor->plugins) {
			g_mutex_lock(&adaptor->plugins_mutex);
			g_list_free_full(adaptor->plugins, (GDestroyNotify) shop_adaptor_plugin_unref);
			adaptor->plugins = NULL;
			g_mutex_unlock(&adaptor->plugins_mutex);
		}
		adaptor->started = 0;
		shop_adaptor_debug("Shop adaptor stopped");
	}

	g_mutex_unlock(&adaptor->shop_adaptor_mutex);
	return result;
}

/**
 * Registers plugin state listener
 */
EXPORT_API
int shop_adaptor_register_listener(shop_adaptor_h adaptor, shop_adaptor_listener_h listener)
{
	if ((NULL == adaptor) || (NULL == listener)) {
		shop_adaptor_error("Invalid argument");
		return SHOP_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	g_mutex_lock(&adaptor->adaptor_listeners_mutex);

	adaptor->adaptor_listeners = g_list_append(adaptor->adaptor_listeners, listener);

	g_mutex_unlock(&adaptor->adaptor_listeners_mutex);

	return SHOP_ADAPTOR_ERROR_NONE;
}

/**
 * Unregisters plugin state listener
 */
EXPORT_API
int shop_adaptor_unregister_listener(shop_adaptor_h adaptor, shop_adaptor_listener_h listener)
{
	if ((NULL == adaptor) || (NULL == listener)) {
		shop_adaptor_error("Invalid argument");
		return SHOP_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	g_mutex_lock(&adaptor->adaptor_listeners_mutex);

	if (NULL == g_list_find(adaptor->adaptor_listeners, listener)) {
		g_mutex_unlock(&adaptor->adaptor_listeners_mutex);
		shop_adaptor_error("Could not find listener");
		return SHOP_ADAPTOR_ERROR_NOT_FOUND;
	}

	adaptor->adaptor_listeners = g_list_remove(adaptor->adaptor_listeners, listener);

	g_mutex_unlock(&adaptor->adaptor_listeners_mutex);

	return SHOP_ADAPTOR_ERROR_NONE;
}

/* /////////////////////////////////////////////////////////////
   // Plugin create / destroy / ref. count / get plugin name
   ///////////////////////////////////////////////////////////// */
static shop_adaptor_plugin_h shop_adaptor_create_plugin(const char *plugin_path)
{
	if (NULL == plugin_path) {
		shop_adaptor_error("Invalid argument");
		return NULL;
	}

	void *dl_handle = dlopen(plugin_path, RTLD_LAZY);
	if (NULL == dl_handle) {
		shop_adaptor_error("Could not load plugin %s: %s", plugin_path, dlerror());
		return NULL;
	}

	shop_adaptor_plugin_handle_h (*get_adaptee_handle)(void) = NULL;

	get_adaptee_handle = (shop_adaptor_plugin_handle_h (*)(void)) (dlsym(dl_handle, "create_plugin_handle"));
	if (NULL == get_adaptee_handle) {
		dlclose(dl_handle);
		shop_adaptor_error("Could not get function pointer to create_plugin_handle");
		return NULL;
	}

	plugin_req_enter();
	shop_adaptor_plugin_handle_h handle = get_adaptee_handle();
	plugin_req_exit_void();

	if (NULL == handle) {
		dlclose(dl_handle);
		shop_adaptor_error("Could not get adaptee handle");
		return NULL;
	}

	shop_adaptor_plugin_h plugin = (shop_adaptor_plugin_h) malloc(sizeof(shop_adaptor_plugin_t));
	if (NULL == plugin) {
		dlclose(dl_handle);
		shop_adaptor_error("Could not create plugin object");
		return NULL;
	}

	plugin->path = g_strdup(plugin_path);
	plugin->handle = handle;
	plugin->dl_handle = dl_handle;
	plugin->ref_counter = 0;

	g_mutex_init(&plugin->ref_counter_mutex);
	g_mutex_init(&plugin->plugin_listener_mutex);

	shop_adaptor_plugin_listener_h listener =
			(shop_adaptor_plugin_listener_h) malloc(sizeof(shop_adaptor_plugin_listener_t));

	plugin_req_enter();
	plugin->handle->set_listener(listener);
	plugin_req_exit_void();
	g_mutex_lock(&plugin->plugin_listener_mutex);
	plugin->plugin_listener = listener;
	g_mutex_unlock(&plugin->plugin_listener_mutex);

	return plugin;
}

static void shop_adaptor_destroy_plugin(shop_adaptor_plugin_h plugin)
{
	if (NULL == plugin) {
		shop_adaptor_error("Invalid argument");
		return;
	}

	if (NULL != plugin->handle) {
		plugin->handle->destroy_handle(plugin->handle);

		g_mutex_lock(&plugin->plugin_listener_mutex);
		plugin_req_enter();
		plugin->handle->unset_listener();
		plugin_req_exit_void();
		g_mutex_unlock(&plugin->plugin_listener_mutex);

		plugin->handle = NULL;
	}

	if (NULL != plugin->dl_handle) {
		dlclose(plugin->dl_handle);
		plugin->dl_handle = NULL;
	}

	free(plugin->path);
	plugin->path = NULL;

	free(plugin);
}

static int shop_adaptor_load_plugins_from_directory(shop_adaptor_h adaptor, const char *dir_path)
{
	char *plugin_path = NULL;
	DIR *dir = NULL;
	struct dirent dir_entry, *result = NULL;

	shop_adaptor_debug("Starting load plugins from directory");

	if ((NULL == adaptor) || (NULL == dir_path)) {
		shop_adaptor_error("Invalid argument");
		return SHOP_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	dir = opendir(dir_path);
	if (NULL == dir) {
		shop_adaptor_error("Could not open dir path (%s)", dir_path);
		return SHOP_ADAPTOR_ERROR_NOT_FOUND;
	}

	int ret = SHOP_ADAPTOR_ERROR_NONE;
	while (0 == (readdir_r(dir, &dir_entry, &result))) {

		if (NULL == result) {
			shop_adaptor_error("Could not open directory %s", plugin_path);
			break;
		}

		if (dir_entry.d_type & DT_DIR) {
			continue;
		}

		plugin_path = g_strconcat(dir_path, "/", dir_entry.d_name, NULL);
		shop_adaptor_plugin_h plugin = shop_adaptor_create_plugin(plugin_path);

		if (NULL != plugin) {
			shop_adaptor_debug("Loaded plugin: %s", plugin_path);
			plugin->adaptor = adaptor;
			shop_adaptor_plugin_ref(plugin);
			g_mutex_lock(&adaptor->plugins_mutex);
			adaptor->plugins = g_list_append(adaptor->plugins, plugin);
			g_mutex_unlock(&adaptor->plugins_mutex);
		} else {
			shop_adaptor_error("Could not load plugin %s", plugin_path);
		}

		free(plugin_path);
		plugin_path = NULL;
	}

	shop_adaptor_debug("End load plugins from directory");
	closedir(dir);
	return ret;
}


static int shop_adaptor_has_plugin(shop_adaptor_h adaptor, shop_adaptor_plugin_h plugin)
{
	if ((NULL == adaptor) || (NULL == plugin)) {
		shop_adaptor_error("Invalid argument");
		return 0;
	}

	int result = 0;

	g_mutex_lock(&adaptor->plugins_mutex);
	if (NULL != g_list_find(adaptor->plugins, plugin)) {
		result = 1;
	}
	g_mutex_unlock(&adaptor->plugins_mutex);

	return result;
}

static void shop_adaptor_plugin_ref(shop_adaptor_plugin_h plugin)
{
	if (NULL == plugin) {
		shop_adaptor_error("Invalid argument");
		return;
	}

	g_mutex_lock(&plugin->ref_counter_mutex);
	plugin->ref_counter = plugin->ref_counter + 1;
	if (NULL != plugin->handle) {
		shop_adaptor_info("plugin name : %s, ref_counter: %d", plugin->handle->plugin_uri, plugin->ref_counter);
	} else {
		shop_adaptor_info("ref_counter : %d", plugin->ref_counter);
	}
	g_mutex_unlock(&plugin->ref_counter_mutex);
}

static void shop_adaptor_plugin_unref(shop_adaptor_plugin_h plugin)
{
	if (NULL == plugin) {
		shop_adaptor_error("Invalid argument");
		return ;
	}

	int should_destroy = 0;

	g_mutex_lock(&plugin->ref_counter_mutex);
	plugin->ref_counter = plugin->ref_counter - 1;

	if (NULL != plugin->handle) {
		shop_adaptor_info("plugin name : %s, ref_counter: %d", plugin->handle->plugin_uri, plugin->ref_counter);
	} else {
		shop_adaptor_info("ref_counter : %d", plugin->ref_counter);
	}

	if (0 >= plugin->ref_counter) {
		should_destroy = 1;
	}
	g_mutex_unlock(&plugin->ref_counter_mutex);

	if (should_destroy) {
		shop_adaptor_debug("Plugin is being destroyed");
		shop_adaptor_destroy_plugin(plugin);
	}
}

/**
 * Refresh access token
 */
EXPORT_API
shop_error_code_t shop_adaptor_refresh_access_token(shop_adaptor_plugin_context_h context,
						const char *new_access_token)
{
	if ((NULL == context) || (NULL == new_access_token) || (0 >= strlen(new_access_token))) {
		return SHOP_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}
	shop_adaptor_debug("New access token : %s", new_access_token);

	free(context->access_token);
	context->access_token = NULL;
	context->access_token = strdup(new_access_token);

	return SHOP_ADAPTOR_ERROR_NONE;
}

EXPORT_API
shop_error_code_t shop_adaptor_refresh_uid(shop_adaptor_plugin_context_h context,
		const char *new_uid)
{
	if ((NULL == context) || (NULL == new_uid) || (0 >= strlen(new_uid))) {
		return SHOP_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}
	shop_adaptor_debug("New uid : %s", new_uid);

	free(context->duid);
	context->duid = NULL;
	context->duid = strdup(new_uid);

	return SHOP_ADAPTOR_ERROR_NONE;
}

/* //////////////////////////////////////////////////////
   // Create / Destroy error code
   ////////////////////////////////////////////////////// */
shop_adaptor_error_code_h shop_adaptor_create_error_code(const int64_t code, const char *msg)
{
	if (NULL == msg) {
		return NULL;
	}

	shop_adaptor_error_code_h error_code =
			(shop_adaptor_error_code_h) malloc(sizeof(shop_adaptor_error_code_t));
	if (NULL == error_code) {
	    return NULL;
	}

	error_code->code = code;
	error_code->msg = strdup(msg);

	return error_code;
}

void shop_adaptor_destroy_error_code(shop_adaptor_error_code_h *error_code)
{
	if ((NULL != error_code) && (NULL != (*error_code))) {
		free((*error_code)->msg);
		(*error_code)->msg = NULL;
		free(*error_code);
		*error_code = NULL;
	}
}


/* //////////////////////////////////////////////////////
   // Plugin context create / destroy
   ////////////////////////////////////////////////////// */
shop_adaptor_plugin_context_h shop_adaptor_create_plugin_context(shop_adaptor_plugin_h plugin, char *plugin_uri, char *duid, char *access_token, char *app_id, char *apptype)
{
	shop_adaptor_debug("Starting shop_adaptor_create_plugin_context");

	if (NULL == plugin) {
		shop_adaptor_error("Invalid argument");
		return NULL;
	}

	if (NULL != plugin->handle) {
		shop_adaptor_plugin_context_h plugin_context = NULL;

		plugin_req_enter();
		plugin->handle->create_context(&plugin_context, duid, access_token, app_id, apptype);
		plugin_req_exit_void();

		plugin_context->plugin_uri = strdup(plugin->handle->plugin_uri);
		return plugin_context;
	} else {
		shop_adaptor_error("Plugin handle is null");
	}

	shop_adaptor_debug("End shop_adaptor_create_plugin_context");
	return NULL;
}

void shop_adaptor_destroy_plugin_context(shop_adaptor_plugin_h plugin, shop_adaptor_plugin_context_h plugin_context)
{
	shop_adaptor_warning("Starting shop_adaptor_get_plugin_by_name");

	if ((NULL == plugin) || (NULL == plugin_context)) {
		shop_adaptor_error("Invalid argument");
		return;
	}

	if (NULL != plugin->handle) {
		plugin_req_enter();
		plugin->handle->destroy_context(plugin_context);
		plugin_req_exit_void();
	} else {
		shop_adaptor_error("Plugin handle is null");
	}
}

/* //////////////////////////////////////////////////////
   // Get plugin by plugin name
   ////////////////////////////////////////////////////// */
shop_adaptor_plugin_h shop_adaptor_get_plugin_by_name(shop_adaptor_h adaptor, const char *plugin_name)
{
	shop_adaptor_warning("Starting shop_adaptor_get_plugin_by_name");

	if ((NULL == adaptor) || (NULL == plugin_name)) {
		shop_adaptor_error("Invalid argument");
		return NULL;
	}

	shop_adaptor_plugin_h plugin = NULL;
	g_mutex_lock(&adaptor->plugins_mutex);
	int count = g_list_length(adaptor->plugins);
	int i = 0;
	for (i = 0; i < count; i++) {
		shop_adaptor_plugin_h temp_plugin = (shop_adaptor_plugin_h)g_list_nth_data(adaptor->plugins, i);
		if (NULL != temp_plugin) {
			if (0 == strcmp(temp_plugin->handle->plugin_uri, plugin_name)) {
				shop_adaptor_plugin_ref(temp_plugin);
				plugin = temp_plugin;
				g_mutex_unlock(&adaptor->plugins_mutex);
				return plugin;
			}
		}
	}
	g_mutex_unlock(&adaptor->plugins_mutex);

	if (NULL == plugin) {
		shop_adaptor_debug("Plugin is not found by name");
	}

	return plugin;
}

/* //////////////////////////////////////////////////////
   // Plugin load / unload / get plugin list
   ////////////////////////////////////////////////////// */
int shop_adaptor_load_plugin(shop_adaptor_h adaptor, const char *plugin_path)
{
	if ((NULL == adaptor) || (NULL == plugin_path)) {
		shop_adaptor_error("Invalid argument");
		return SHOP_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (!adaptor->started) {
		shop_adaptor_error("Storage adaptor is not started");
		return SHOP_ADAPTOR_ERROR_START;
	}

	shop_adaptor_plugin_h plugin = shop_adaptor_create_plugin(plugin_path);
	if (NULL == plugin) {
		shop_adaptor_error("Could not load plugin %s", plugin_path);
		return SHOP_ADAPTOR_ERROR_CREATE;
	}

	plugin->adaptor = adaptor;
	shop_adaptor_plugin_ref(plugin);

	g_mutex_lock(&adaptor->plugins_mutex);
	adaptor->plugins = g_list_append(adaptor->plugins, plugin);
	g_mutex_unlock(&adaptor->plugins_mutex);

	return SHOP_ADAPTOR_ERROR_NONE;
}

int shop_adaptor_unload_plugin(shop_adaptor_h adaptor, shop_adaptor_plugin_h plugin)
{
	if ((NULL == adaptor) || (NULL == plugin)) {
		shop_adaptor_error("Invalid argument");
		return SHOP_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (!adaptor->started) {
		shop_adaptor_error("Storage adaptor is not started");
		return SHOP_ADAPTOR_ERROR_START;
	}

	if (!shop_adaptor_has_plugin(adaptor, plugin)) {
		shop_adaptor_error("Storage adaptor has no plugin");
		return SHOP_ADAPTOR_ERROR_NOT_FOUND;
	}

	plugin->adaptor = NULL;

	g_mutex_lock(&adaptor->plugins_mutex);
	adaptor->plugins = g_list_remove(adaptor->plugins, plugin);
	g_mutex_unlock(&adaptor->plugins_mutex);

	shop_adaptor_plugin_unref(plugin);

	return SHOP_ADAPTOR_ERROR_NONE;
}

GList *shop_adaptor_get_plugins(shop_adaptor_h adaptor)
{
	if (NULL == adaptor) {
		shop_adaptor_error("Invalid argument");
		return NULL;
	}

	GList *plugins = NULL;

	g_mutex_lock(&adaptor->plugins_mutex);
	int plugins_count = g_list_length(adaptor->plugins);
	int i;
	for (i = 0; i < plugins_count; i++) {
		shop_adaptor_plugin_h plugin = (shop_adaptor_plugin_h)g_list_nth_data(adaptor->plugins, i);
		if (NULL != plugin) {
			shop_adaptor_plugin_ref(plugin);
			plugins = g_list_append(plugins, plugin);
		}
	}
	g_mutex_unlock(&adaptor->plugins_mutex);

	return plugins;

}

EXPORT_API
shop_error_code_t shop_adaptor_set_server_info(shop_adaptor_plugin_h plugin,
						shop_adaptor_plugin_context_h context,
						GHashTable *server_info,
						shop_adaptor_error_code_h *error_code)
{
	if ((NULL == plugin) || (NULL == context)) {
		shop_adaptor_error("Invalid argument");

		*error_code = shop_adaptor_create_error_code((int64_t) SHOP_ADAPTOR_ERROR_INVALID_ARGUMENT,
								"Invalid argument (plugin or context)");

		return SHOP_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		shop_adaptor_error("Plugin handle is null");

		*error_code = shop_adaptor_create_error_code((int64_t) SHOP_ADAPTOR_ERROR_INVALID_HANDLE,
								"Plugin handle is null");

		return SHOP_ADAPTOR_ERROR_INVALID_HANDLE;
	}

	shop_error_code_t ret;
	plugin_req_enter();
	ret = plugin->handle->set_server_info(context, server_info, error_code);
	plugin_req_exit(ret, plugin, error_code);

	return ret;
}

/* ////////////////////////////////////////////////////////////
   // Adaptor Plugin call Functions
   //////////////////////////////////////////////////////////// */
EXPORT_API
shop_error_code_t shop_adaptor_get_item_list_v1(shop_adaptor_plugin_h plugin, shop_adaptor_plugin_context_h context, shop_adaptor_shop_info_s *info, void *user_data, shop_adaptor_shop_item_s ***items, unsigned int *items_len, shop_adaptor_error_code_h *error_code, void **server_data)
{
	if ((NULL == plugin) || (NULL == context)) {
		shop_adaptor_error("Invalid argument");

		*error_code = shop_adaptor_create_error_code(
				(int64_t) SHOP_ADAPTOR_ERROR_INVALID_ARGUMENT,
				"Invalid argument (plugin or context)");

		return SHOP_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		shop_adaptor_error("Plugin handle is null");

		*error_code = shop_adaptor_create_error_code(
				(int64_t) SHOP_ADAPTOR_ERROR_INVALID_HANDLE,
				"Plugin handle is null");

		return SHOP_ADAPTOR_ERROR_INVALID_HANDLE;
	}

	shop_error_code_t ret;
	plugin_req_enter();
	ret = plugin->handle->get_item_list_v1(context, info, user_data, items, items_len, error_code, server_data);
	plugin_req_exit(ret, plugin, error_code);

	return ret;
}

EXPORT_API
shop_error_code_t shop_adaptor_download_item_package_v1(shop_adaptor_plugin_h plugin, shop_adaptor_plugin_context_h context, shop_adaptor_shop_info_s *info, void *user_data, shop_adaptor_shop_item_s **item, shop_adaptor_error_code_h *error_code, void **server_data)
{
	if ((NULL == plugin) || (NULL == context)) {
		shop_adaptor_error("Invalid argument");

		*error_code = shop_adaptor_create_error_code(
				(int64_t) SHOP_ADAPTOR_ERROR_INVALID_ARGUMENT,
				"Invalid argument (plugin or context)");

		return SHOP_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		shop_adaptor_error("Plugin handle is null");

		*error_code = shop_adaptor_create_error_code(
				(int64_t) SHOP_ADAPTOR_ERROR_INVALID_HANDLE,
				"Plugin handle is null");

		return SHOP_ADAPTOR_ERROR_INVALID_HANDLE;
	}

	shop_error_code_t ret;
	plugin_req_enter();
	ret = plugin->handle->download_item_package_v1(context, info, user_data, item, error_code, server_data);
	plugin_req_exit(ret, plugin, error_code);

	return ret;
}

EXPORT_API
shop_error_code_t shop_adaptor_download_sticker_v1(shop_adaptor_plugin_h plugin, shop_adaptor_plugin_context_h context, shop_adaptor_shop_info_s *info, void *user_data, shop_adaptor_shop_item_s **item, shop_adaptor_error_code_h *error_code, void **server_data)
{
	if ((NULL == plugin) || (NULL == context)) {
		shop_adaptor_error("Invalid argument");

		*error_code = shop_adaptor_create_error_code(
				(int64_t) SHOP_ADAPTOR_ERROR_INVALID_ARGUMENT,
				"Invalid argument (plugin or context)");

		return SHOP_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		shop_adaptor_error("Plugin handle is null");

		*error_code = shop_adaptor_create_error_code(
				(int64_t) SHOP_ADAPTOR_ERROR_INVALID_HANDLE,
				"Plugin handle is null");

		return SHOP_ADAPTOR_ERROR_INVALID_HANDLE;
	}

	shop_error_code_t ret;
	plugin_req_enter();
	ret = plugin->handle->download_sticker_v1(context, info, user_data, item, error_code, server_data);
	plugin_req_exit(ret, plugin, error_code);

	return ret;
}

EXPORT_API
shop_error_code_t shop_adaptor_get_panel_url_v1(shop_adaptor_plugin_h plugin, shop_adaptor_plugin_context_h context, shop_adaptor_shop_info_s *info, void *user_data, shop_adaptor_shop_item_s **item, shop_adaptor_error_code_h *error_code, void **server_data)
{
	if ((NULL == plugin) || (NULL == context)) {
		shop_adaptor_error("Invalid argument");

		*error_code = shop_adaptor_create_error_code(
				(int64_t) SHOP_ADAPTOR_ERROR_INVALID_ARGUMENT,
				"Invalid argument (plugin or context)");

		return SHOP_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		shop_adaptor_error("Plugin handle is null");

		*error_code = shop_adaptor_create_error_code(
				(int64_t) SHOP_ADAPTOR_ERROR_INVALID_HANDLE,
				"Plugin handle is null");

		return SHOP_ADAPTOR_ERROR_INVALID_HANDLE;
	}

	if (NULL == info) {
		shop_adaptor_error("info handle is null");

		*error_code = shop_adaptor_create_error_code(
				(int64_t) SHOP_ADAPTOR_ERROR_INVALID_ARGUMENT,
				"Info handle is null");

		return SHOP_ADAPTOR_ERROR_INVALID_HANDLE;
	}

	shop_error_code_t ret;
	plugin_req_enter();
	ret = plugin->handle->get_panel_url_v1(context, info, user_data, item, error_code, server_data);
	plugin_req_exit(ret, plugin, error_code);

	return ret;
}
