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

#include "push-adaptor.h"
#include "push-adaptor-log.h"

/**
 * Push adaptor plugin
 */
typedef struct push_adaptor_plugin_s {
	push_adaptor_h			adaptor;		/* Adaptor */
	char				*path;			/* Plugin library path */
	push_adaptor_plugin_handle_h	handle;			/* Plugin handle */
	void				*dl_handle;		/* Plugin library handle */
	int				ref_counter;		/* Plugin reference counter */
	GMutex				ref_counter_mutex;	/* Plugin reference counter mutex */
	push_adaptor_plugin_listener_h	plugin_listener;	/* Plugin callback listener */
	GMutex				plugin_listener_mutex;	/* Plugin callback listener mutex */
} push_adaptor_plugin_t;

/**
 * Push adaptor
 */
typedef struct push_adaptor_s {
	GMutex	push_adaptor_mutex;		/* Adaptor mutex */
	int	started;			/* Started flag */
	char	*plugins_dir;			/* Plugins directory path */
	GList	*plugins;			/* List of loaded plugins */
	GMutex	plugins_mutex;			/* Plugin list mutex */
	GList	*adaptor_listeners;		/* List of vservice channel listener (for now not effective) */
	GMutex	adaptor_listeners_mutex;	/* Listener list mutex */
} push_adaptor_t;

/**
 * Creates plugin
 */
static push_adaptor_plugin_h push_adaptor_create_plugin(const char *plugin_path);

/**
 * Destroys plugin and deletes all resources associated with it
 */
static void push_adaptor_destroy_plugin(push_adaptor_plugin_h plugin);

/**
 * Loads plugins from selected directory
 */
static int push_adaptor_load_plugins_from_directory(push_adaptor_h adaptor, const char *dir_path);

/**
 * Checks if plugin is loaded by selected plugin adaptor
 */
static int push_adaptor_has_plugin(push_adaptor_h adaptor, push_adaptor_plugin_h plugin);

/**
 * Increases adaptor's plugin references counter
 */
static void push_adaptor_plugin_ref(push_adaptor_plugin_h);

/**
 * Decreases adaptor's plugin references counter
 */
static void push_adaptor_plugin_unref(push_adaptor_plugin_h);

/**
 * On notification received callback for service adaptor
 */
push_adaptor_service_on_notification_received_cb _service_adaptor_on_notification_received = NULL;

/**
 * Called on push notification received from plugin
 */
void
push_adaptor_on_notification_received(push_adaptor_notification_data_h notification, void *user_data)
{
	push_adaptor_info("[Push MSG]: %s", notification->msg);

	if (NULL != _service_adaptor_on_notification_received) {
		_service_adaptor_on_notification_received(notification, user_data);
	}
}

/* //////////////////////////////////////////////////////
   // Mandatory: Internal adaptor management function
   ////////////////////////////////////////////////////// */

/**
 * Creates plugin
 */
static push_adaptor_plugin_h push_adaptor_create_plugin(const char *plugin_path)
{
	push_adaptor_debug("Create plugin");

	if (NULL == plugin_path) {
		push_adaptor_error("Invalid argument");
		return NULL;
	}

	void *dl_handle = dlopen(plugin_path, RTLD_LAZY);
	if (NULL == dl_handle) {
		push_adaptor_error("Could not load plugin %s: %s", plugin_path, dlerror());
		return NULL;
	}

	push_adaptor_plugin_handle_h (*get_adaptee_handle)(void) = NULL;

	get_adaptee_handle = (push_adaptor_plugin_handle_h (*)(void))(dlsym(dl_handle, "create_plugin_handle"));
	if (NULL == get_adaptee_handle) {
		dlclose(dl_handle);
		push_adaptor_error("Could not get function pointer to create_plugin_handle");
		return NULL;
	}

	plugin_req_enter();
	push_adaptor_plugin_handle_h handle = get_adaptee_handle();
	plugin_req_exit_void();

	if (NULL == handle) {
		dlclose(dl_handle);
		push_adaptor_error("Could not get adaptee handle");
		return NULL;
	}

	push_adaptor_plugin_h plugin = (push_adaptor_plugin_h) calloc(1, sizeof(push_adaptor_plugin_t));
	if (NULL == plugin) {
		dlclose(dl_handle);
		push_adaptor_error("Could not create plugin object");
		return NULL;
	}

	push_adaptor_plugin_listener_h listener =
		(push_adaptor_plugin_listener_h) calloc(1, sizeof(push_adaptor_plugin_listener_t));
	if (NULL == listener) {
		free(plugin);
		dlclose(dl_handle);
		push_adaptor_error("Could not create listener object");
		return NULL;
	}

	plugin->path = g_strdup(plugin_path);
	plugin->handle = handle;
	plugin->dl_handle = dl_handle;
	plugin->ref_counter = 0;

	g_mutex_init(&plugin->ref_counter_mutex);
	g_mutex_init(&plugin->plugin_listener_mutex);

	listener->_on_notification_received = push_adaptor_on_notification_received;

	plugin_req_enter();
	plugin->handle->set_listener(listener);
	plugin_req_exit_void();

	g_mutex_lock(&plugin->plugin_listener_mutex);
	plugin->plugin_listener = listener;
	g_mutex_unlock(&plugin->plugin_listener_mutex);

	return plugin;
}

/**
 * Destroys plugin and deletes all resources associated with it
 */
static void push_adaptor_destroy_plugin(push_adaptor_plugin_h plugin)
{
	push_adaptor_debug("Destroy plugin");

	if (NULL == plugin) {
		push_adaptor_error("Invalid argument");
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

/**
 * Loads plugins from selected directory
 */
static int push_adaptor_load_plugins_from_directory(push_adaptor_h adaptor, const char *dir_path)
{
	push_adaptor_debug("Load plugins from directory");

	char *plugin_path = NULL;
	DIR *dir = NULL;
	struct dirent dir_entry, *result = NULL;

	if ((NULL == adaptor) || (NULL == dir_path)) {
		push_adaptor_error("Invalid argument");
		return PUSH_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	dir = opendir(dir_path);
	if (NULL == dir) {
		push_adaptor_error("Could not open dir path (%s)", dir_path);
		return PUSH_ADAPTOR_ERROR_NOT_FOUND;
	}

	int ret = PUSH_ADAPTOR_ERROR_NONE;
	while (0 == (readdir_r(dir, &dir_entry, &result))) {

		if (NULL == result) {
			push_adaptor_error("Could not open directory %s", plugin_path);
			break;
		}

		if (dir_entry.d_type & DT_DIR) {
			continue;
		}

		plugin_path = g_strconcat(dir_path, "/", dir_entry.d_name, NULL);
		push_adaptor_plugin_h plugin = push_adaptor_create_plugin(plugin_path);

		if (NULL != plugin) {
			push_adaptor_debug("Loaded plugin: %s", plugin_path);
			plugin->adaptor = adaptor;
			push_adaptor_plugin_ref(plugin);
			g_mutex_lock(&adaptor->plugins_mutex);
			adaptor->plugins = g_list_append(adaptor->plugins, plugin);
			g_mutex_unlock(&adaptor->plugins_mutex);
		} else {
			push_adaptor_error("Could not load plugin %s", plugin_path);
		}

		free(plugin_path);
		plugin_path = NULL;
	}

	push_adaptor_debug("End load plugins from directory");
	closedir(dir);
	return ret;
}

/**
 * Checks if plugin is loaded by selected plugin adaptor
 */
static int push_adaptor_has_plugin(push_adaptor_h adaptor, push_adaptor_plugin_h plugin)
{
	push_adaptor_debug("Find plugin in plugin list");

	if ((NULL == adaptor) || (NULL == plugin)) {
		push_adaptor_error("Invalid argument");
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

/**
 * Increases adaptor's plugin references counter
 */
static void push_adaptor_plugin_ref(push_adaptor_plugin_h plugin)
{
	push_adaptor_debug("Increase plugin reference count");

	if (NULL == plugin) {
		push_adaptor_error("Invalid argument");
		return;
	}

	g_mutex_lock(&plugin->ref_counter_mutex);
	plugin->ref_counter = plugin->ref_counter + 1;
	push_adaptor_info("ref_counter: %d", plugin->ref_counter);
	g_mutex_unlock(&plugin->ref_counter_mutex);
}

/**
 * Decreases adaptor's plugin references counter
 */
static void push_adaptor_plugin_unref(push_adaptor_plugin_h plugin)
{
	push_adaptor_debug("Decrease plugin reference count");

	if (NULL == plugin) {
		push_adaptor_error("Invalid argument");
		return ;
	}

	int should_destroy = 0;

	g_mutex_lock(&plugin->ref_counter_mutex);
	plugin->ref_counter = plugin->ref_counter - 1;
	push_adaptor_info("ref_counter: %d", plugin->ref_counter);
	if (0 >= plugin->ref_counter) {
		should_destroy = 1;
	}
	g_mutex_unlock(&plugin->ref_counter_mutex);

	if (should_destroy) {
		push_adaptor_debug("Plugin is being destroyed");
		push_adaptor_destroy_plugin(plugin);
	}
}

/* //////////////////////////////////////////////////////
   // Mandatory: External adaptor management function
   ////////////////////////////////////////////////////// */

/**
* @brief Creates Push Adaptor
*
* @param[in]    plugin_dir      specifies directory path where plugins are stored
* @return       push_adaptor_h on success, otherwise NULL value
*/
push_adaptor_h push_adaptor_create(const char *plugins_dir)
{
	push_adaptor_warning("Create push adaptor");

	push_adaptor_h push_adaptor = (push_adaptor_h) malloc(sizeof(push_adaptor_t));
	if (NULL == push_adaptor) {
		return NULL;
	}

	push_adaptor->started = 0;
	push_adaptor->plugins_dir = strdup(plugins_dir);

	g_mutex_init(&push_adaptor->push_adaptor_mutex);
	g_mutex_init(&push_adaptor->plugins_mutex);
	g_mutex_init(&push_adaptor->adaptor_listeners_mutex);

	g_mutex_lock(&push_adaptor->adaptor_listeners_mutex);
	push_adaptor->adaptor_listeners = NULL;
	g_mutex_unlock(&push_adaptor->adaptor_listeners_mutex);

	g_mutex_lock(&push_adaptor->plugins_mutex);
	push_adaptor->plugins = NULL;
	g_mutex_unlock(&push_adaptor->plugins_mutex);

	return push_adaptor;
}

/**
* @brief Destroys push adaptor. If push adaptor was started it is stopped first.
*
* @param[in]    adaptor         specifies push adaptor handle to be destroyed
* @return       void
*/
void push_adaptor_destroy(push_adaptor_h adaptor)
{
	push_adaptor_warning("Destroy push adaptor");
	if (NULL == adaptor) {
		push_adaptor_error("Invalid argument");
		return ;
	}

	g_mutex_lock(&adaptor->push_adaptor_mutex);
	if (adaptor->started) {
		push_adaptor_error("Push adaptor is running. Forcing stop before destroy");
		push_adaptor_stop(adaptor);
	}

	g_mutex_lock(&adaptor->plugins_mutex);
	if (NULL != adaptor->plugins) {
		g_list_free_full(adaptor->plugins, (GDestroyNotify) push_adaptor_plugin_unref);
		adaptor->plugins = NULL;
	}
	g_mutex_unlock(&adaptor->plugins_mutex);

	g_mutex_lock(&adaptor->adaptor_listeners_mutex);
	if (NULL != adaptor->adaptor_listeners) {
		g_list_free(adaptor->adaptor_listeners);
		adaptor->adaptor_listeners = NULL;
	}
	g_mutex_unlock(&adaptor->adaptor_listeners_mutex);

	_service_adaptor_on_notification_received = NULL;

	free(adaptor->plugins_dir);
	adaptor->plugins_dir = NULL;

	g_mutex_unlock(&adaptor->push_adaptor_mutex);

	free(adaptor);
}

/**
* @brief Starts push adaptor and loads plugins that were found in plugins search dir
* specified in push_adaptor_create
*
* @param[in]    adaptor         specifies push adaptor handle
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in push_error_code_e - PUSH_ADAPTOR_ERROR_NONE if successful
*/
int push_adaptor_start(push_adaptor_h adaptor)
{
	push_adaptor_warning("Start push adaptor");
	if (NULL == adaptor) {
		push_adaptor_error("Invalid argument");
		return PUSH_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	g_mutex_lock(&adaptor->push_adaptor_mutex);
	int result = PUSH_ADAPTOR_ERROR_NONE;
	if (adaptor->started) {
		push_adaptor_error("Push adaptor is already started");
		result = PUSH_ADAPTOR_ERROR_START;
	} else {
		adaptor->started = 1;
		result = push_adaptor_load_plugins_from_directory(adaptor, adaptor->plugins_dir);
		if (PUSH_ADAPTOR_ERROR_NONE != result) {
			adaptor->started = 0;
			push_adaptor_error("Could not load plugins from directory");
		} else {
			push_adaptor_debug("Push adaptor started successfully");
		}
	}
	g_mutex_unlock(&adaptor->push_adaptor_mutex);

	return result;
}

/**
* @brief Stops push adaptor
*
* @param[in]    adaptor         specifies push adaptor handle
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in push_error_code_e - PUSH_ADAPTOR_ERROR_NONE if successful
*/
int push_adaptor_stop(push_adaptor_h adaptor)
{
	push_adaptor_warning("Stop contact adaptor");

	if (NULL == adaptor) {
		push_adaptor_error("Invalid argument");
		return PUSH_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	g_mutex_lock(&adaptor->push_adaptor_mutex);
	int result = PUSH_ADAPTOR_ERROR_NONE;
	if (!adaptor->started) {
		result = PUSH_ADAPTOR_ERROR_START;
	} else {
		if (NULL != adaptor->plugins) {
			g_mutex_lock(&adaptor->plugins_mutex);
			g_list_free_full(adaptor->plugins, (GDestroyNotify) push_adaptor_plugin_unref);
			adaptor->plugins = NULL;
			g_mutex_unlock(&adaptor->plugins_mutex);
		}
		adaptor->started = 0;
		push_adaptor_debug("Push adaptor stopped");
	}

	g_mutex_unlock(&adaptor->push_adaptor_mutex);
	return result;
}

/**
* @brief Registers plugin state listener
*
* @param[in]    adaptor	        specifies push adaptor handle
* @param[in]    listener        specifies push adaptor listener handle
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in push_error_code_e - PUSH_ADAPTOR_ERROR_NONE if successful
*/
int push_adaptor_register_listener(push_adaptor_h adaptor, push_adaptor_listener_h listener)
{
	push_adaptor_warning("Register push adaptor listener");

	if ((NULL == adaptor) || (NULL == listener)) {
		push_adaptor_error("Invalid argument");
		return PUSH_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	g_mutex_lock(&adaptor->adaptor_listeners_mutex);
	adaptor->adaptor_listeners = g_list_append(adaptor->adaptor_listeners, listener);

	g_mutex_unlock(&adaptor->adaptor_listeners_mutex);

	_service_adaptor_on_notification_received =
			(push_adaptor_service_on_notification_received_cb) listener->_on_notification_received;

	return PUSH_ADAPTOR_ERROR_NONE;
}

/**
* @brief Unregisters plugin state listener
*
* @param[in]    adaptor	        specifies push adaptor handle
* @param[in]    listener        specifies push adaptor listener handle
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in push_error_code_e - PUSH_ADAPTOR_ERROR_NONE if successful
*/
int push_adaptor_unregister_listener(push_adaptor_h adaptor, push_adaptor_listener_h listener)
{
	push_adaptor_warning("Unregister push adaptor listener");

	if ((NULL == adaptor) || (NULL == listener)) {
		push_adaptor_error("Invalid argument");
		return PUSH_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	g_mutex_lock(&adaptor->adaptor_listeners_mutex);

	if (NULL == g_list_find(adaptor->adaptor_listeners, listener)) {
		g_mutex_unlock(&adaptor->adaptor_listeners_mutex);
		push_adaptor_error("Could not find listener");
		return PUSH_ADAPTOR_ERROR_NOT_FOUND;
	}

	adaptor->adaptor_listeners = g_list_remove(adaptor->adaptor_listeners, listener);

	g_mutex_unlock(&adaptor->adaptor_listeners_mutex);

	_service_adaptor_on_notification_received = NULL;

	return PUSH_ADAPTOR_ERROR_NONE;
}

/* //////////////////////////////////////////////////////
   // Create / Destroy error code
   ////////////////////////////////////////////////////// */

/**
* @brief Create error code
*
* @param[in]    code            specifies error code number
* @param[in]    msg             specifies error message
* @return       push_adaptor_error_code_h on success, otherwise NULL value
*/
push_adaptor_error_code_h push_adaptor_create_error_code(const int64_t code, const char *msg)
{
	if (NULL == msg) {
		return NULL;
	}

	push_adaptor_error_code_h error_code =
		(push_adaptor_error_code_h) malloc(sizeof(push_adaptor_error_code_t));
	if (NULL == error_code) {
		return NULL;
	}

	error_code->code = code;
	error_code->msg = strdup(msg);

	return error_code;
}

/**
* @brief Destroy error code
*
* @param[in]    error_code      specifies error code handle
* @return       void
*/
void push_adaptor_destroy_error_code(push_adaptor_error_code_h *error_code)
{
	if (NULL == *error_code) {
		return;
	}

	if (NULL != (*error_code)->msg) {
		free((*error_code)->msg);
		(*error_code)->msg = NULL;
	}

	free(*error_code);
	*error_code = NULL;
}

/* //////////////////////////////////////////////////////
   // Plugin context create / destroy
   ////////////////////////////////////////////////////// */
/**
* @brief Creates plugin context
*
* @param[in]    plugin          specifies push adaptor plugin handle
* @param[in]    push_app_id     specifies push service application ID
* @return       push_adaptor_plugin_context_h on success, otherwise NULL value
*/
push_adaptor_plugin_context_h push_adaptor_create_plugin_context(push_adaptor_plugin_h plugin,
						const char *plugin_uri,
						const char *push_app_id)
{
	push_adaptor_warning("Create plugin context");

	if (NULL == plugin) {
		push_adaptor_error("Invalid argument");
		return NULL;
	}

	if (NULL != plugin->handle) {
		push_adaptor_plugin_context_h plugin_context = NULL;

		plugin_req_enter();
		plugin->handle->create_context(&plugin_context, push_app_id);
		plugin_req_exit_void();

		if (NULL != plugin_context) {
			plugin_context->plugin_uri = strdup(plugin->handle->plugin_uri);
			plugin_context->state = PUSH_ADAPTOR_STATE_DISCONNECTED;
		}
		return plugin_context;
	} else {
		push_adaptor_error("Plugin handle is null");
	}

	return NULL;
}

/**
* @brief Destroys plugin context
*
* @param[in]    plugin          specifies push adaptor plugin handle
* @param[in]    context         specifies push adaptor plugin context handle
* @return       void
*/
void push_adaptor_destroy_plugin_context(push_adaptor_plugin_h plugin,
						push_adaptor_plugin_context_h plugin_context)
{
	push_adaptor_warning("Destroy plugin context");

	if ((NULL == plugin) || (NULL == plugin_context)) {
		push_adaptor_error("Invalid argument");
		return;
	}

	if (NULL != plugin->handle) {
		plugin_req_enter();
		plugin->handle->destroy_context(plugin_context);
		plugin_req_exit_void();
	} else {
		push_adaptor_error("Plugin handle is null");
	}
}

/* //////////////////////////////////////////////////////
   // Get plugin by plugin name
   ////////////////////////////////////////////////////// */

/**
* @brief Gets plugin with specified unique name
*
* @param[in]    adaptor         specifies push adaptor handle
* @param[in]    plugin_name     specifies plugin name to be searched for
* @return       push_adaptor_plugin_h on success, otherwise NULL value
*/
push_adaptor_plugin_h push_adaptor_get_plugin_by_name(push_adaptor_h adaptor,
						const char *plugin_name)
{
	push_adaptor_warning("Get plugin by name: %s", plugin_name);

	if ((NULL == adaptor) || (NULL == plugin_name)) {
		push_adaptor_error("Invalid argument");
		return NULL;
	}

	push_adaptor_plugin_h plugin = NULL;
	g_mutex_lock(&adaptor->plugins_mutex);
	int count = g_list_length(adaptor->plugins);
	int i = 0;
	for (i = 0; i < count; i++) {
		push_adaptor_plugin_h temp_plugin = g_list_nth_data(adaptor->plugins, i);
		if (NULL != temp_plugin) {
			if (0 == strcmp(temp_plugin->handle->plugin_uri, plugin_name)) {
				push_adaptor_plugin_ref(temp_plugin);
				plugin = temp_plugin;
				push_adaptor_debug("Plugin is found by name");
				break;
			}
		}
	}
	g_mutex_unlock(&adaptor->plugins_mutex);

	if (NULL == plugin)
		push_adaptor_debug("Plugin is not found by name");

	return plugin;
}

/* //////////////////////////////////////////////////////
   // Plugin load / unload / get plugin list
   ////////////////////////////////////////////////////// */

/**
* @brief Loads plugin from selected path
*
* @param[in]    adaptor         specifies push adaptor handle
* @param[in]    plugin_path     specifies plugin's saved path
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in push_error_code_e - PUSH_ADAPTOR_ERROR_NONE if successful
*/
int push_adaptor_load_plugin(push_adaptor_h adaptor,
						const char *plugin_path)
{
	push_adaptor_warning("Load plugin by plugin path: %s", plugin_path);

	if ((NULL == adaptor) || (NULL == plugin_path)) {
		push_adaptor_error("Invalid argument");
		return PUSH_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (!adaptor->started) {
		push_adaptor_error("Push adaptor is not started");
		return PUSH_ADAPTOR_ERROR_START;
	}

	push_adaptor_plugin_h plugin = push_adaptor_create_plugin(plugin_path);
	if (NULL == plugin) {
		push_adaptor_error("Could not load plugin %s", plugin_path);
		return PUSH_ADAPTOR_ERROR_CREATE;
	}

	plugin->adaptor = adaptor;
	push_adaptor_plugin_ref(plugin);

	g_mutex_lock(&adaptor->plugins_mutex);
	adaptor->plugins = g_list_append(adaptor->plugins, plugin);
	g_mutex_unlock(&adaptor->plugins_mutex);

	return PUSH_ADAPTOR_ERROR_NONE;
}

/**
* @brief Unloads selected plugin
*
* @param[in]    adaptor         specifies push adaptor handle
* @param[in]    plugin          specifies push adaptor plugin handle
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in push_error_code_e - PUSH_ADAPTOR_ERROR_NONE if successful
*/
int push_adaptor_unload_plugin(push_adaptor_h adaptor,
						push_adaptor_plugin_h plugin)
{
	push_adaptor_warning("Unload plugin");

	if ((NULL == adaptor) || (NULL == plugin)) {
		push_adaptor_error("Invalid argument");
		return PUSH_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (!adaptor->started) {
		push_adaptor_error("Push adaptor is not started");
		return PUSH_ADAPTOR_ERROR_START;
	}

	if (!push_adaptor_has_plugin(adaptor, plugin)) {
		push_adaptor_error("Push adaptor has no plugin");
		return PUSH_ADAPTOR_ERROR_NOT_FOUND;
	}

	plugin->adaptor = NULL;

	g_mutex_lock(&adaptor->plugins_mutex);
	adaptor->plugins = g_list_remove(adaptor->plugins, plugin);
	g_mutex_unlock(&adaptor->plugins_mutex);

	push_adaptor_plugin_unref(plugin);

	return PUSH_ADAPTOR_ERROR_NONE;
}

/**
* @brief Get plugin list
*
* @param[in]    adaptor         specifies push adaptor handle
* @return       GList pointer on success, otherwise NULL value
*/
GList *push_adaptor_get_plugins(push_adaptor_h adaptor)
{
	push_adaptor_warning("Get plugin list");

	if (NULL == adaptor) {
		push_adaptor_error("Invalid argument");
		return NULL;
	}

	GList *plugins = NULL;

	g_mutex_lock(&adaptor->plugins_mutex);
	int plugins_count = g_list_length(adaptor->plugins);
	int i;
	for (i = 0; i < plugins_count; i++) {
		push_adaptor_plugin_h plugin = g_list_nth_data(adaptor->plugins, i);
		if (NULL != plugin) {
			push_adaptor_plugin_ref(plugin);
			plugins = g_list_append(plugins, plugin);
		}
	}
	g_mutex_unlock(&adaptor->plugins_mutex);

	return plugins;
}

/* //////////////////////////////////////////////////////
   // External Push Adaptor APIs
   ////////////////////////////////////////////////////// */

/**
* @brief Set server information for Push Plugin
*
* @param[in]    plugin         specifies Push Adaptor Plugin handle
* @param[in]    context        specifies Push Adaptor Plugin Context handle
* @param[in]    server_info    specifies server information for Push Plugin
* @param[out]   error          specifies error code
* @return 0 on success, otherwise a positive error value
* @retval error code defined in push_error_code_t - PUSH_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
push_error_code_t push_adaptor_set_server_info(push_adaptor_plugin_h plugin,
						push_adaptor_plugin_context_h context,
						GHashTable *server_info,
						push_adaptor_error_code_h *error_code)
{
	if ((NULL == plugin) || (NULL == context)) {
		push_adaptor_error("Invalid argument");

		*error_code = push_adaptor_create_error_code((int64_t) PUSH_ADAPTOR_ERROR_INVALID_ARGUMENT,
								"Invalid argument (plugin or context)");

		return PUSH_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		push_adaptor_error("Plugin handle is null");

		*error_code = push_adaptor_create_error_code((int64_t) PUSH_ADAPTOR_ERROR_INVALID_HANDLE,
								"Plugin handle is null");

		return PUSH_ADAPTOR_ERROR_INVALID_HANDLE;
	}

/*      *error_code = push_adaptor_create_error_code((int64_t) PUSH_ADAPTOR_ERROR_NONE, */
/*				                      "Push adaptor error none"); */
	push_error_code_t ret;
	plugin_req_enter();
	ret = plugin->handle->set_server_info(context, server_info);
	plugin_req_exit(ret, plugin, error_code);

	return ret;
}

/**
* @brief Connects to push service with Push App ID handed over when creates plugin context
*
* @param[in]    plugin          specifies push adaptor plugin handle
* @param[in]    context         specifies push adaptor plugin context handle
* @param[out]   error           specifies error code
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in push_error_code_e - PUSH_ADAPTOR_ERROR_NONE if successful
*/
push_error_code_t push_adaptor_connect(push_adaptor_plugin_h plugin,
						push_adaptor_plugin_context_h context,
						push_adaptor_error_code_h *error_code)
{
	push_adaptor_warning("Connect to push adaptor");

	if ((NULL == plugin) || (NULL == context)) {
		push_adaptor_error("Invalid argument");

		*error_code = push_adaptor_create_error_code((int64_t) PUSH_ADAPTOR_ERROR_INVALID_ARGUMENT,
								"Invalid argument (plugin or context)");

		return PUSH_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		push_adaptor_error("Plugin handle is null");

		*error_code = push_adaptor_create_error_code((int64_t) PUSH_ADAPTOR_ERROR_INVALID_HANDLE,
								"Plugin handle is null");

		return PUSH_ADAPTOR_ERROR_INVALID_HANDLE;
	}

/*	*error_code = push_adaptor_create_error_code((int64_t) PUSH_ADAPTOR_ERROR_NONE, */
/*							"Push adaptor error none"); */

	push_error_code_t ret;
	plugin_req_enter();
	ret = plugin->handle->connect(context);
	plugin_req_exit(ret, plugin, error_code);

	return ret;
}

/**
* @brief Disconnects from push service with Push App ID handed over when creates plugin context
*
* @param[in]    plugin          specifies push adaptor plugin handle
* @param[in]    context         specifies push adaptor plugin context handle
* @param[out]   error           specifies error code
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in push_error_code_e - PUSH_ADAPTOR_ERROR_NONE if successful
*/
push_error_code_t push_adaptor_disconnect(push_adaptor_plugin_h plugin,
						push_adaptor_plugin_context_h context,
						push_adaptor_error_code_h *error_code)
{
	push_adaptor_warning("Disconnect from push adaptor");

	if ((NULL == plugin) || (NULL == context)) {
		push_adaptor_error("Invalid argument");

		*error_code = push_adaptor_create_error_code((int64_t) PUSH_ADAPTOR_ERROR_INVALID_ARGUMENT,
								"Invalid argument (plugin or context)");

		return PUSH_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		push_adaptor_error("Plugin handle is null");

		*error_code = push_adaptor_create_error_code((int64_t) PUSH_ADAPTOR_ERROR_INVALID_HANDLE,
								"Plugin handle is null");

		return PUSH_ADAPTOR_ERROR_INVALID_HANDLE;
	}

/*	*error_code = push_adaptor_create_error_code((int64_t) PUSH_ADAPTOR_ERROR_NONE, */
/*							"Push adaptor error none"); */

	push_error_code_t ret;
	plugin_req_enter();
	ret = plugin->handle->disconnect(context);
	plugin_req_exit(ret, plugin, error_code);

	return ret;
}

EXPORT_API
push_error_code_t push_adaptor_is_connected(push_adaptor_plugin_h plugin,
						push_adaptor_plugin_context_h context,
						int *is_connected)
{
	push_adaptor_info("Check push connection");

	if ((NULL == context) || (NULL == is_connected)) {
		return PUSH_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	*is_connected = context->state;

	return PUSH_ADAPTOR_ERROR_NONE;
}

/**
* @brief Asynchronous request to get unread notifications
*
* @param[in]    plugin          specifies push adaptor plugin handle
* @param[in]    context         specifies push adaptor plugin context handle
* @param[out]   error           specifies error code
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in push_error_code_e - PUSH_ADAPTOR_ERROR_NONE if successful
*/
push_error_code_t push_adaptor_request_unread_notification(push_adaptor_plugin_h plugin,
						push_adaptor_plugin_context_h context,
						push_adaptor_error_code_h *error_code)
{
	push_adaptor_warning("Request unread notification");

	if ((NULL == plugin) || (NULL == context)) {
		push_adaptor_error("Invalid argument");

		*error_code = push_adaptor_create_error_code((int64_t) PUSH_ADAPTOR_ERROR_INVALID_ARGUMENT,
								"Invalid argument (plugin or context)");

		return PUSH_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		push_adaptor_error("Plugin handle is null");

		*error_code = push_adaptor_create_error_code((int64_t) PUSH_ADAPTOR_ERROR_INVALID_HANDLE,
								"Plugin handle is null");

		return PUSH_ADAPTOR_ERROR_INVALID_HANDLE;
	}

/*	*error_code = push_adaptor_create_error_code((int64_t) PUSH_ADAPTOR_ERROR_NONE, */
/*							"Push adaptor error none"); */

	push_error_code_t ret;
	plugin_req_enter();
	ret = plugin->handle->request_unread_notification(context);
	plugin_req_exit(ret, plugin, error_code);

	return ret;
}
