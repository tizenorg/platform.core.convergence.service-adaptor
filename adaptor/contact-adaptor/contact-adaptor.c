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

#include "contact-adaptor.h"
#include "contact-adaptor-log.h"

/**
 * Contact adaptor plugin
 */
typedef struct contact_adaptor_plugin_s {
	contact_adaptor_h			adaptor;		/* Adaptor */
	char					*path;			/* Plugin library path */
	contact_adaptor_plugin_handle_h		handle;			/* Plugin handle */
	void					*dl_handle;		/* Plugin library handle */
	int					ref_counter;		/* Plugin reference counter */
	GMutex					ref_counter_mutex;	/* Plugin reference counter mutex */
	contact_adaptor_plugin_listener_h	plugin_listener;	/* Plugin callback listener */
	GMutex					plugin_listener_mutex;	/* Plugin callback listener mutex */
} contact_adaptor_plugin_t;

/**
 * Contact adaptor
 */
typedef struct contact_adaptor_s {
	GMutex	contact_adaptor_mutex;		/* Adaptor mutex */
	int	started;			/* Started flag */
	char	*plugins_dir;			/* Plugins directory path */
	GList	*plugins;			/* List of loaded plugins */
	GMutex	plugins_mutex;			/* Plugin list mutex */
	GList	*adaptor_listeners;		/* List of vservice channel listener (for now not effective) */
	GMutex	adaptor_listeners_mutex;	/* Listener list mutex */
} contact_adaptor_t;

/**
 * Creates plugin
 */
static contact_adaptor_plugin_h contact_adaptor_create_plugin(const char *plugin_path);

/**
 * Destroys plugin and deletes all resources associated with it
 */
static void contact_adaptor_destroy_plugin(contact_adaptor_plugin_h plugin);

/**
 * Loads plugins from selected directory
 */
static int contact_adaptor_load_plugins_from_directory(contact_adaptor_h adaptor,
						const char *dir_path);

/**
 * Checks if plugin is loaded by selected plugin adaptor
 */
static int contact_adaptor_has_plugin(contact_adaptor_h adaptor,
						contact_adaptor_plugin_h plugin);

/**
 * Increases adaptor's plugin references counter
 */
static void contact_adaptor_plugin_ref(contact_adaptor_plugin_h);

/**
 * Decreases adaptor's plugin references counter
 */
static void contact_adaptor_plugin_unref(contact_adaptor_plugin_h);

/**
 * On message received callback for service adaptor
 */
contact_adaptor_service_on_message_received_cb _service_adaptor_on_message_received = NULL;

/**
 * Callback on message received from plugin
 */
void
contact_adaptor_on_message_received(void *user_data)
{
	if (NULL != _service_adaptor_on_message_received) {
		_service_adaptor_on_message_received(user_data);
	}
}

/* /////////////////////////////////////////////////////////////
   // Plugin create / destroy / ref. count / get plugin name
   ///////////////////////////////////////////////////////////// */

/**
* Creates plugin
*/
static contact_adaptor_plugin_h contact_adaptor_create_plugin(const char *plugin_path)
{
	contact_adaptor_debug("Create plugin");

	if (NULL == plugin_path) {
		contact_adaptor_error("Invalid argument");
		return NULL;
	}

	void *dl_handle = dlopen(plugin_path, RTLD_LAZY);
	if (NULL == dl_handle) {
		contact_adaptor_error("Could not load plugin %s: %s", plugin_path, dlerror());
		return NULL;
	}

	contact_adaptor_plugin_handle_h (*get_adaptee_handle)(void) = NULL;

	get_adaptee_handle = (contact_adaptor_plugin_handle_h (*)(void))(dlsym(dl_handle, "create_plugin_handle"));
	if (NULL == get_adaptee_handle) {
		dlclose(dl_handle);
		contact_adaptor_error("Could not get function pointer to create_plugin_handle");
		return NULL;
	}

	plugin_req_enter();
	contact_adaptor_plugin_handle_h handle = get_adaptee_handle();
	plugin_req_exit_void();

	if (NULL == handle) {
		dlclose(dl_handle);
		contact_adaptor_error("Could not get adaptee handle");
		return NULL;
	}

	contact_adaptor_plugin_h plugin = (contact_adaptor_plugin_h) calloc(1, sizeof(contact_adaptor_plugin_t));
	if (NULL == plugin) {
		dlclose(dl_handle);
		contact_adaptor_error("Could not create plugin object");
		return NULL;
	}

	contact_adaptor_plugin_listener_h listener =
		(contact_adaptor_plugin_listener_h) calloc(1, sizeof(contact_adaptor_plugin_listener_t));
	if (NULL == listener) {
		free(plugin);
		dlclose(dl_handle);
		contact_adaptor_error("Could not create listener object");
		return NULL;
	}

	plugin->path = g_strdup(plugin_path);
	plugin->handle = handle;
	plugin->dl_handle = dl_handle;
	plugin->ref_counter = 0;

	g_mutex_init(&plugin->ref_counter_mutex);
	g_mutex_init(&plugin->plugin_listener_mutex);

	listener->_on_message_received	= contact_adaptor_on_message_received;

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
static void contact_adaptor_destroy_plugin(contact_adaptor_plugin_h plugin)
{
	contact_adaptor_debug("Destroy plugin");

	if (NULL == plugin) {
		contact_adaptor_error("Invalid argument");
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
static int contact_adaptor_load_plugins_from_directory(contact_adaptor_h adaptor,
						const char *dir_path)
{
	contact_adaptor_debug("Load plugins from directory");

	char *plugin_path = NULL;
	DIR *dir = NULL;
	struct dirent dir_entry, *result = NULL;

	if ((NULL == adaptor) || (NULL == dir_path)) {
		contact_adaptor_error("Invalid argument");
		return CONTACT_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	dir = opendir(dir_path);
	if (NULL == dir) {
		contact_adaptor_error("Could not open dir path (%s)", dir_path);
		return CONTACT_ADAPTOR_ERROR_NOT_FOUND;
	}

	int ret = CONTACT_ADAPTOR_ERROR_NONE;
	while (0 == (readdir_r(dir, &dir_entry, &result))) {

		if (NULL == result) {
			contact_adaptor_error("Could not open directory %s", plugin_path);
			break;
		}

		if (dir_entry.d_type & DT_DIR) {
			continue;
		}

		plugin_path = g_strconcat(dir_path, "/", dir_entry.d_name, NULL);
		contact_adaptor_plugin_h plugin = contact_adaptor_create_plugin(plugin_path);

		if (NULL != plugin) {
			contact_adaptor_debug("Loaded plugin: %s", plugin_path);
			plugin->adaptor = adaptor;
			contact_adaptor_plugin_ref(plugin);
			g_mutex_lock(&adaptor->plugins_mutex);
			adaptor->plugins = g_list_append(adaptor->plugins, plugin);
			g_mutex_unlock(&adaptor->plugins_mutex);
		} else {
			contact_adaptor_error("Could not load plugin %s", plugin_path);
		}

		free(plugin_path);
		plugin_path = NULL;
	}

	contact_adaptor_debug("End load plugins from directory");
	closedir(dir);
	return ret;
}

/**
* Checks if plugin is loaded by selected plugin adaptor
*/
static int contact_adaptor_has_plugin(contact_adaptor_h adaptor,
						contact_adaptor_plugin_h plugin)
{
	contact_adaptor_debug("Find plugin in plugin list");

	if ((NULL == adaptor) || (NULL == plugin)) {
		contact_adaptor_error("Invalid argument");
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
static void contact_adaptor_plugin_ref(contact_adaptor_plugin_h plugin)
{
	contact_adaptor_debug("Increase plugin reference count");

	if (NULL == plugin) {
		contact_adaptor_error("Invalid argument");
		return;
	}

	g_mutex_lock(&plugin->ref_counter_mutex);
	plugin->ref_counter = plugin->ref_counter + 1;
	contact_adaptor_info("ref_counter: %d", plugin->ref_counter);
	g_mutex_unlock(&plugin->ref_counter_mutex);
}

/**
* Decreases adaptor's plugin references counter
*/
static void contact_adaptor_plugin_unref(contact_adaptor_plugin_h plugin)
{
	contact_adaptor_debug("Decrease plugin reference count");

	if (NULL == plugin) {
		contact_adaptor_error("Invalid argument");
		return ;
	}

	int should_destroy = 0;

	g_mutex_lock(&plugin->ref_counter_mutex);
	plugin->ref_counter = plugin->ref_counter - 1;
	contact_adaptor_info("ref_counter: %d", plugin->ref_counter);
	if (0 >= plugin->ref_counter) {
		should_destroy = 1;
	}
	g_mutex_unlock(&plugin->ref_counter_mutex);

	if (should_destroy) {
		contact_adaptor_debug("Plugin is being destroyed");
		contact_adaptor_destroy_plugin(plugin);
	}
}

/* //////////////////////////////////////////////////////
   // Mandatory: External adaptor management function
   ////////////////////////////////////////////////////// */

/**
* @brief Creates Contact Adaptor.
*
* @param[in]    plugin_dir      specifies directory path where plugins are stored
* @return       contact_adaptor_h on success, otherwise NULL value
*/
contact_adaptor_h contact_adaptor_create(const char *plugins_dir)
{
	contact_adaptor_warning("Create contact adaptor");

	contact_adaptor_h contact_adaptor = (contact_adaptor_h) malloc(sizeof(contact_adaptor_t));
	if (NULL == contact_adaptor) {
		return NULL;
	}

	contact_adaptor->started = 0;
	contact_adaptor->plugins_dir = strdup(plugins_dir);

	g_mutex_init(&contact_adaptor->contact_adaptor_mutex);
	g_mutex_init(&contact_adaptor->plugins_mutex);
	g_mutex_init(&contact_adaptor->adaptor_listeners_mutex);

	g_mutex_lock(&contact_adaptor->adaptor_listeners_mutex);
	contact_adaptor->adaptor_listeners = NULL;
	g_mutex_unlock(&contact_adaptor->adaptor_listeners_mutex);

	g_mutex_lock(&contact_adaptor->plugins_mutex);
	contact_adaptor->plugins = NULL;
	g_mutex_unlock(&contact_adaptor->plugins_mutex);

	return contact_adaptor;
}

/**
* @brief Destroys contact adaptor. If contact adaptor was started it is stopped first.
*
* @param[in]	adaptor		specifies contact adaptor handle to be destroyed
* @return	void
*/
void contact_adaptor_destroy(contact_adaptor_h adaptor)
{
	contact_adaptor_warning("Destroy contact adaptor");

	if (NULL == adaptor) {
		contact_adaptor_error("Invalid argument");
		return ;
	}

	g_mutex_lock(&adaptor->contact_adaptor_mutex);
	if (adaptor->started) {
		contact_adaptor_error("Contact adaptor is running. Forcing stop before destroy");
		contact_adaptor_stop(adaptor);
	}

	g_mutex_lock(&adaptor->plugins_mutex);
	if (NULL != adaptor->plugins) {
		g_list_free_full(adaptor->plugins, (GDestroyNotify) contact_adaptor_plugin_unref);
		adaptor->plugins = NULL;
	}
	g_mutex_unlock(&adaptor->plugins_mutex);

	g_mutex_lock(&adaptor->adaptor_listeners_mutex);
	if (NULL != adaptor->adaptor_listeners) {
		g_list_free(adaptor->adaptor_listeners);
		adaptor->adaptor_listeners = NULL;
	}
	g_mutex_unlock(&adaptor->adaptor_listeners_mutex);

	_service_adaptor_on_message_received	= NULL;

	free(adaptor->plugins_dir);
	adaptor->plugins_dir = NULL;

	g_mutex_unlock(&adaptor->contact_adaptor_mutex);

	free(adaptor);
}

/**
* @brief Starts contact adaptor and loads plugins that are found in contact_adaptor_create().
*
* @param[in]    adaptor		specifies contact adaptor handle
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in contact_error_code_e - CONTACT_ADAPTOR_ERROR_NONE if successful
*/
int contact_adaptor_start(contact_adaptor_h adaptor)
{
	contact_adaptor_warning("Start contact adaptor");

	if (NULL == adaptor) {
		contact_adaptor_error("Invalid argument");
		return CONTACT_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	g_mutex_lock(&adaptor->contact_adaptor_mutex);
	int result = CONTACT_ADAPTOR_ERROR_NONE;
	if (adaptor->started) {
		contact_adaptor_error("Contact adaptor is already started");
		result = CONTACT_ADAPTOR_ERROR_START;
	} else {
		adaptor->started = 1;
		result = contact_adaptor_load_plugins_from_directory(adaptor, adaptor->plugins_dir);
		if (CONTACT_ADAPTOR_ERROR_NONE != result) {
			adaptor->started = 0;
			contact_adaptor_error("Could not load plugins from directory");
		} else {
			contact_adaptor_debug("Contact adaptor started successfully");
		}
	}
	g_mutex_unlock(&adaptor->contact_adaptor_mutex);

	return result;
}

/**
* @brief Stops contact adaptor.
*
* @param[in]    adaptor		specifies contact adaptor handle
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in contact_error_code_e - CONTACT_ADAPTOR_ERROR_NONE if successful
*/
int contact_adaptor_stop(contact_adaptor_h adaptor)
{
	contact_adaptor_warning("Stop contact adaptor");

	if (NULL == adaptor) {
		contact_adaptor_error("Invalid argument");
		return CONTACT_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	g_mutex_lock(&adaptor->contact_adaptor_mutex);
	int result = CONTACT_ADAPTOR_ERROR_NONE;
	if (!adaptor->started) {
		result = CONTACT_ADAPTOR_ERROR_START;
	} else {
		if (NULL != adaptor->plugins) {
			g_mutex_lock(&adaptor->plugins_mutex);
			g_list_free_full(adaptor->plugins, (GDestroyNotify) contact_adaptor_plugin_unref);
			adaptor->plugins = NULL;
			g_mutex_unlock(&adaptor->plugins_mutex);
		}
		adaptor->started = 0;
		contact_adaptor_debug("Contact adaptor stopped");
	}

	g_mutex_unlock(&adaptor->contact_adaptor_mutex);
	return result;
}

/**
* @brief Registers plugin state listener
*
* @param[in]    adaptor		specifies contact adaptor handle
* @param[in]    listener	specifies contact adaptor listener handle
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in contact_error_code_e - CONTACT_ADAPTOR_ERROR_NONE if successful
*/
int contact_adaptor_register_listener(contact_adaptor_h adaptor,
						contact_adaptor_listener_h listener)
{
	contact_adaptor_warning("Register contact adaptor listener");

	if ((NULL == adaptor) || (NULL == listener)) {
		contact_adaptor_error("Invalid argument");
		return CONTACT_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	g_mutex_lock(&adaptor->adaptor_listeners_mutex);

	adaptor->adaptor_listeners = g_list_append(adaptor->adaptor_listeners, listener);

	g_mutex_unlock(&adaptor->adaptor_listeners_mutex);

	_service_adaptor_on_message_received =
			(contact_adaptor_service_on_message_received_cb) listener->_on_message_received;

	return CONTACT_ADAPTOR_ERROR_NONE;
}

/**
* @brief Unregisters plugin state listener
*
* @param[in]    adaptor		specifies contact adaptor handle
* @param[in]    listener	specifies contact adaptor listener handle
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in contact_error_code_e - CONTACT_ADAPTOR_ERROR_NONE if successful
*/
int contact_adaptor_unregister_listener(contact_adaptor_h adaptor,
						contact_adaptor_listener_h listener)
{
	contact_adaptor_warning("Deregister contact adaptor listener");

	if ((NULL == adaptor) || (NULL == listener)) {
		contact_adaptor_error("Invalid argument");
		return CONTACT_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	g_mutex_lock(&adaptor->adaptor_listeners_mutex);

	if (NULL == g_list_find(adaptor->adaptor_listeners, listener)) {
		g_mutex_unlock(&adaptor->adaptor_listeners_mutex);
		contact_adaptor_error("Could not find listener");
		return CONTACT_ADAPTOR_ERROR_NOT_FOUND;
	}

	adaptor->adaptor_listeners = g_list_remove(adaptor->adaptor_listeners, listener);

	g_mutex_unlock(&adaptor->adaptor_listeners_mutex);

	_service_adaptor_on_message_received	= NULL;

	return CONTACT_ADAPTOR_ERROR_NONE;
}

/* //////////////////////////////////////////////////////
   // Plugin context create / destroy
   ////////////////////////////////////////////////////// */

/**
* @brief Creates plugin context
*
* @param[in]    plugin		specifies contact adaptor plugin handle
* @param[in]    duid		specifies device unique ID
* @param[in]    access_token	specifies access token issued by Auth Adaptor
* @return       contact_adaptor_plugin_context_h on success, otherwise NULL value
*/
contact_adaptor_plugin_context_h contact_adaptor_create_plugin_context(contact_adaptor_plugin_h plugin,
						const char *duid,
						const char *access_token,
						const char *service_name)
{
	contact_adaptor_warning("Create plugin context");

	if ((NULL == plugin) || (NULL == duid) || (NULL == access_token) || (NULL == service_name)) {
		contact_adaptor_error("Invalid argument: %s, %s", duid, access_token);
		return NULL;
	}

	if (NULL != plugin->handle) {
		contact_adaptor_plugin_context_h plugin_context = NULL;

		plugin_req_enter();
		plugin->handle->create_context(&plugin_context, duid, access_token);
		plugin_req_exit_void();

		plugin_context->plugin_uri = strdup(plugin->handle->plugin_uri);
		plugin_context->service_name = strdup(service_name);
		return plugin_context;
	} else {
		contact_adaptor_error("Plugin handle is null");
	}

	return NULL;
}

/**
* @brief Destroys plugin context.
*
* @param[in]    plugin		specifies contact adaptor plugin handle
* @param[in]    context		specifies contact adaptor plugin context handle
* @return       void
*/
void contact_adaptor_destroy_plugin_context(contact_adaptor_plugin_h plugin,
						contact_adaptor_plugin_context_h plugin_context)
{
	contact_adaptor_warning("Destroy plugin context");

	if ((NULL == plugin) || (NULL == plugin_context)) {
		contact_adaptor_error("Invalid argument");
		return;
	}

	if (NULL != plugin->handle) {
		free(plugin_context->plugin_uri);
		plugin_context->plugin_uri = NULL;
		free(plugin_context->service_name);
		plugin_context->service_name = NULL;

		plugin_req_enter();
		plugin->handle->destroy_context(plugin_context);
		plugin_req_exit_void();
	} else {
		contact_adaptor_error("Plugin handle is null");
	}
}

/* //////////////////////////////////////////////////////
   // Get plugin by plugin name
   ////////////////////////////////////////////////////// */

/**
* @brief Gets plugin with specified unique name
*
* @param[in]    adaptor		specifies contact adaptor handle
* @param[in]    plugin_name     specifies plugin name to be searched for
* @return       contact_adaptor_plugin_h on success, otherwise NULL value
*/
contact_adaptor_plugin_h contact_adaptor_get_plugin_by_name(contact_adaptor_h adaptor,
						const char *plugin_name)
{
	contact_adaptor_warning("Get plugin by name: %s", plugin_name);

	if ((NULL == adaptor) || (NULL == plugin_name)) {
		contact_adaptor_error("Invalid argument");
		return NULL;
	}

	contact_adaptor_plugin_h plugin = NULL;
	g_mutex_lock(&adaptor->plugins_mutex);
	int count = g_list_length(adaptor->plugins);
	int i = 0;
	for (i = 0; i < count; i++) {
		contact_adaptor_plugin_h temp_plugin = g_list_nth_data(adaptor->plugins, i);
		if (NULL != temp_plugin) {
			if (0 == strcmp(temp_plugin->handle->plugin_uri, plugin_name)) {
				contact_adaptor_plugin_ref(temp_plugin);
				plugin = temp_plugin;
				contact_adaptor_debug("Plugin is found by name");
				g_mutex_unlock(&adaptor->plugins_mutex);
				return plugin;
			}
		}
	}
	g_mutex_unlock(&adaptor->plugins_mutex);

	if (NULL == plugin) {
		contact_adaptor_debug("Plugin is not found by name");
	}

	return plugin;
}

/* //////////////////////////////////////////////////////
   // Plugin load / unload / get plugin list
   ////////////////////////////////////////////////////// */

/**
* @brief Loads plugin from selected path
*
* @param[in]    adaptor         specifies contact adaptor handle
* @param[in]    plugin_path     specifies plugin's saved path
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in contact_error_code_e - CONTACT_ADAPTOR_ERROR_NONE if successful
*/
int contact_adaptor_load_plugin(contact_adaptor_h adaptor,
						const char *plugin_path)
{
	contact_adaptor_warning("Load plugin by plugin path: %s", plugin_path);

	if ((NULL == adaptor) || (NULL == plugin_path)) {
		contact_adaptor_error("Invalid argument");
		return CONTACT_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (!adaptor->started) {
		contact_adaptor_error("Contact adaptor is not started");
		return CONTACT_ADAPTOR_ERROR_START;
	}

	contact_adaptor_plugin_h plugin = contact_adaptor_create_plugin(plugin_path);
	if (NULL == plugin) {
		contact_adaptor_error("Could not load plugin %s", plugin_path);
		return CONTACT_ADAPTOR_ERROR_CREATE;
	}

	plugin->adaptor = adaptor;
	contact_adaptor_plugin_ref(plugin);

	g_mutex_lock(&adaptor->plugins_mutex);
	adaptor->plugins = g_list_append(adaptor->plugins, plugin);
	g_mutex_unlock(&adaptor->plugins_mutex);

	return CONTACT_ADAPTOR_ERROR_NONE;
}

/**
* @brief Unloads selected plugin
*
* @param[in]    adaptor         specifies contact adaptor handle
* @param[in]    plugin          specifies contact adaptor plugin handle
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in contact_error_code_e - CONTACT_ADAPTOR_ERROR_NONE if successful
*/
int contact_adaptor_unload_plugin(contact_adaptor_h adaptor,
						contact_adaptor_plugin_h plugin)
{
	contact_adaptor_warning("Unload plugin");

	if ((NULL == adaptor) || (NULL == plugin)) {
		contact_adaptor_error("Invalid argument");
		return CONTACT_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (!adaptor->started) {
		contact_adaptor_error("Contact adaptor is not started");
		return CONTACT_ADAPTOR_ERROR_START;
	}

	if (!contact_adaptor_has_plugin(adaptor, plugin)) {
		contact_adaptor_error("Contact adaptor has no plugin");
		return CONTACT_ADAPTOR_ERROR_NOT_FOUND;
	}

	plugin->adaptor = NULL;

	g_mutex_lock(&adaptor->plugins_mutex);
	adaptor->plugins = g_list_remove(adaptor->plugins, plugin);
	g_mutex_unlock(&adaptor->plugins_mutex);

	contact_adaptor_plugin_unref(plugin);

	return CONTACT_ADAPTOR_ERROR_NONE;
}

/**
* @brief Get plugin list of contact adaptor handle has
*
* @param[in]    adaptor         specifies contact adaptor handle
* @return       GList pointer on success, otherwise NULL value
*/
GList *contact_adaptor_get_plugins(contact_adaptor_h adaptor)
{
	contact_adaptor_warning("Get plugin list");

	if (NULL == adaptor) {
		contact_adaptor_error("Invalid argument");
		return NULL;
	}

	GList *plugins = NULL;

	g_mutex_lock(&adaptor->plugins_mutex);
	int plugins_count = g_list_length(adaptor->plugins);
	int i;
	for (i = 0; i < plugins_count; i++) {
		contact_adaptor_plugin_h plugin = g_list_nth_data(adaptor->plugins, i);
		if (NULL != plugin) {
			contact_adaptor_plugin_ref(plugin);
			plugins = g_list_append(plugins, plugin);
		}
	}
	g_mutex_unlock(&adaptor->plugins_mutex);

	return plugins;
}

/**
* @brief Refresh access token was issued from auth-adaptor
*
* @param[in]	context			specifies Contact Adaptor Plugin Context handle
* @param[in]	new_access_token	specifies New access token
* @return	contact_adaptor_error_code_h on success, otherwise NULL value
*/
EXPORT_API
contact_error_code_t contact_adaptor_refresh_access_token(contact_adaptor_plugin_context_h context,
						const char *new_access_token)
{
	if ((NULL == context) || (NULL == new_access_token) || (0 >= strlen(new_access_token))) {
		return CONTACT_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}
	contact_adaptor_debug("New access token : %s", new_access_token);

	free(context->access_token);
	context->access_token = NULL;
	context->access_token = strdup(new_access_token);

	return CONTACT_ADAPTOR_ERROR_NONE;
}

EXPORT_API
contact_error_code_t contact_adaptor_refresh_uid(contact_adaptor_plugin_context_h context,
						const char *new_uid)
{
	if ((NULL == context) || (NULL == new_uid) || (0 >= strlen(new_uid))) {
		return CONTACT_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}
	contact_adaptor_debug("New uid : %s", new_uid);

	free(context->duid);
	context->duid = NULL;
	context->duid = strdup(new_uid);

	return CONTACT_ADAPTOR_ERROR_NONE;
}


/* //////////////////////////////////////////////////////
   // Create / Destroy error code
   ////////////////////////////////////////////////////// */

/**
* @brief Create error code
*
* @param[in]    code            specifies error code number
* @param[in]    msg             specifies error message
* @return       contact_adaptor_error_code_h on success, otherwise NULL value
*/
contact_adaptor_error_code_h contact_adaptor_create_error_code(const int64_t code,
						const char *msg)
{
	if (NULL == msg) {
		return NULL;
	}

	contact_adaptor_error_code_h error_code = (contact_adaptor_error_code_h) malloc(sizeof(contact_adaptor_error_code_t));
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
void contact_adaptor_destroy_error_code(contact_adaptor_error_code_h *error_code)
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

void __contact_adaptor_destroy_char_array(char **arr, unsigned int len)
{
	if ((NULL != arr) && (0U < len)) {
		for (int i = 0; i < len; i++) {
			free(arr[i]);
		}
		free(arr);
	}
}

EXPORT_API
void contact_adaptor_destroy_contact_req_s(contact_adaptor_contact_req_h req)
{
	if (NULL != req) {
		if ((req->cts != NULL) && (0U < req->cts_len)) {
			for (int i = 0; i < req->cts_len; i++) {
				if (NULL != req->cts[i]) {
					free(req->cts[i]->tp);
					free(req->cts[i]->id);
					free(req->cts[i]->pn);
					free(req->cts[i]->nm);
					free(req->cts[i]->cc);
				}
				free(req->cts[i]);
			}
			free(req->cts);
		}
		free(req);
	}
}

EXPORT_API
void contact_adaptor_destroy_contact_res_s(contact_adaptor_contact_res_h res)
{
	if (NULL == res) {
		return;
	} else if ((res->cts == NULL) || (0U == res->cts_len)) {
		free(res);
		return;
	}

	for (int i = 0; i < res->cts_len; i++) {
		if (NULL != res->cts[i]) {
			free(res->cts[i]->duid);
			free(res->cts[i]->id);
			free(res->cts[i]->msisdn);
			free(res->cts[i]->ty);
			free(res->cts[i]->cc);
			free(res->cts[i]->pn);
			free(res->cts[i]->nm);
			/*
			   if ((NULL != res->cts[i]->evnt) && (0U > res->cts[i]->evnt_len)) {
			   for (int j = 0; j < res->cts[i]->evnt_len; j++) {
			   free(res->cts[i]->evnt[j]);
			   }
			   free(res->cts[i]->evnt);
			   }
			 */
			__contact_adaptor_destroy_char_array(res->cts[i]->evnt, res->cts[i]->evnt_len);
			/* free(res->cts[i]->img);*/

			__contact_adaptor_destroy_char_array(res->cts[i]->adrs, res->cts[i]->adrs_len);
			__contact_adaptor_destroy_char_array(res->cts[i]->mail, res->cts[i]->mail_len);
			/*
			   if ((NULL != res->cts[i]->adrs) && (0U > res->cts[i]->adrs_len)) {
			   for (int j = 0; j < res->cts[i]->adrs_len; j++) {
			   free(res->cts[i]->adrs[j]);
			   }
			   free(res->cts[i]->adrs);
			   }
			   if ((NULL != res->cts[i]->mail) && (0U > res->cts[i]->mail_len)) {
			   for (int j = 0; j < res->cts[i]->mail_len; j++) {
			   free(res->cts[i]->mail[j]);
			   }
			   free(res->cts[i]->mail);
			   }
			 */
			free(res->cts[i]->org);
			free(res->cts[i]->prsc);
			free(res->cts[i]->status);
		}
		free(res->cts[i]);
	}
	free(res->cts);
	free(res);
}

EXPORT_API
void contact_adaptor_destroy_profile_req_s(contact_adaptor_profile_req_h req)
{
	if (NULL == req) {
		return;
	}

	free(req->cc);
	free(req->pn);
	free(req->nm);
	__contact_adaptor_destroy_char_array(req->evnt, req->evnt_len);
	free(req->img);
	__contact_adaptor_destroy_char_array(req->adrs, req->adrs_len);
	__contact_adaptor_destroy_char_array(req->mail, req->mail_len);
	free(req->org);
	free(req->prsc);
	free(req->status);

	free(req);
}

EXPORT_API
void contact_adaptor_destroy_profile_res_s(contact_adaptor_profile_res_h res)
{
	if (NULL == res) {
		return;
	}

	free(res->nm);
	free(res->img);
	free(res->prsc);
	free(res->status);

	free(res);
}

EXPORT_API
void contact_adaptor_destroy_file_path_s(contact_adaptor_file_path_h path)
{
	if (NULL == path) {
		return;
	}

	__contact_adaptor_destroy_char_array(path->file_paths, path->file_paths_len);

	free(path);
}

EXPORT_API
void contact_adaptor_destroy_privacy_req_s(contact_adaptor_privacy_req_h req)
{
	if (NULL == req) {
		return;
	}

	if ((NULL != req->cts) && (0U < req->cts_len)) {
		for (int i = 0; i < req->cts_len; i++) {
			free(req->cts[i]->cc);
			free(req->cts[i]->pn);
			free(req->cts[i]);
		}
		free(req->cts);
	}
	free(req);
}

EXPORT_API
void contact_adaptor_destroy_privacy_res_s(contact_adaptor_privacy_res_h res)
{
	free(res);
}

EXPORT_API
void contact_adaptor_destroy_presence_info_s(contact_adaptor_presence_info_h info)
{
	if (NULL == info) {
		return;
	}

	free(info->prsc);
	free(info->status);
	free(info);
}

/* //////////////////////////////////////////////////////
   // Contact Adaptor External APIs
   ////////////////////////////////////////////////////// */

/**
* @brief Set server information for Contact Plugin
*
* @param[in]    plugin		specifies Contact Adaptor Plugin handle
* @param[in]    context	specifies Contact Adaptor Plugin Context handle
* @param[in]    server_info	specifies server information for Contact Plugin
* @param[in]    request	specifies optional parameter
* @param[out]   error		specifies error code
* @param[out]   response	specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in contact_error_code_t - CONTACT_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
contact_error_code_t contact_adaptor_set_server_info(contact_adaptor_plugin_h plugin,
						contact_adaptor_plugin_context_h context,
						GHashTable *server_info,
						void *user_data,
						contact_adaptor_error_code_h *error,
						void **server_data)
{
	if ((NULL == plugin) || (NULL == context)) {
		contact_adaptor_error("Invalid argument");

		*error = contact_adaptor_create_error_code((int64_t) CONTACT_ADAPTOR_ERROR_INVALID_ARGUMENT,
								"Invalid argument (plugin or context)");

		return CONTACT_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		contact_adaptor_error("Plugin handle is null");

		*error = contact_adaptor_create_error_code((int64_t) CONTACT_ADAPTOR_ERROR_INVALID_HANDLE,
								"Plugin handle is null");

		return CONTACT_ADAPTOR_ERROR_INVALID_HANDLE;
	}

	contact_error_code_t ret;
	plugin_req_enter();
	ret = plugin->handle->set_server_info(context, server_info, user_data, error, server_data);
	plugin_req_exit(ret, plugin, error);

	return ret;
}

/**
* @brief Resets contact information in Contact server and upload native contact information of device to
* the server
*
* @param[in]    plugin          specifies contact adaptor plugin handle
* @param[in]    context         specifies contact adaptor plugin context handle
* @param[in]    request         specifies contact adaptor contact API request handle
* @param[in]    user_data       specifies user side arbitrary data
* @param[out]   response        specifies contact adaptor contact API response handle
* @param[out]   error           specifies returned error code handle
* @param[out]   server_data     specifies server side arbitrary data
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in contact_error_code_e - CONTACT_ADAPTOR_ERROR_NONE if successful
*/
contact_error_code_t contact_adaptor_new_contact_list(contact_adaptor_plugin_h plugin,
						contact_adaptor_plugin_context_h context,
						contact_adaptor_contact_req_h request,
						void *user_data,
						contact_adaptor_contact_res_h *response,
						contact_adaptor_error_code_h *error,
						void **server_data)
{
	contact_adaptor_warning("New contact list");

	if ((NULL == plugin) || (NULL == context) || (NULL == request)) {
		contact_adaptor_error("Invalid argument");

		*error = contact_adaptor_create_error_code((int64_t) CONTACT_ADAPTOR_ERROR_INVALID_ARGUMENT,
							"Invalid argument (plugin or context or request)");

		return CONTACT_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		contact_adaptor_error("Plugin handle is null");

		*error = contact_adaptor_create_error_code((int64_t) CONTACT_ADAPTOR_ERROR_INVALID_HANDLE,
							"Plugin handle is null");

		return CONTACT_ADAPTOR_ERROR_INVALID_HANDLE;
	}

	contact_error_code_t ret;
	plugin_req_enter();
	ret = plugin->handle->new_contact_list(context, request, user_data, response, error, server_data);
	plugin_req_exit(ret, plugin, error);

	return ret;
}

/**
* @brief Synchronized native contact information of device with contact server according to type
* "type" field of each contact
*
* @param[in]    plugin          specifies contact adaptor plugin handle
* @param[in]    context         specifies contact adaptor plugin context handle
* @param[in]    request         specifies contact adaptor contact API request handle
* @param[in]    user_data       specifies user side arbitrary data
* @param[out]   response        specifies contact adaptor contact API response handle
* @param[out]   error           specifies returned error code handle
* @param[out]   server_data     specifies server side arbitrary data
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in contact_error_code_e - CONTACT_ADAPTOR_ERROR_NONE if successful
*/
contact_error_code_t contact_adaptor_set_contact_list(contact_adaptor_plugin_h plugin,
						contact_adaptor_plugin_context_h context,
						contact_adaptor_contact_req_h request,
						void *user_data,
						contact_adaptor_contact_res_h *response,
						contact_adaptor_error_code_h *error,
						void **server_data)
{
	contact_adaptor_warning("Set contact list");

	if ((NULL == plugin) || (NULL == context) || (NULL == request)) {
		contact_adaptor_error("Invalid argument");

		*error = contact_adaptor_create_error_code((int64_t) CONTACT_ADAPTOR_ERROR_INVALID_ARGUMENT,
							"Invalid argument (plugin or context or request)");

		return CONTACT_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		contact_adaptor_error("Plugin handle is null");

		*error = contact_adaptor_create_error_code((int64_t) CONTACT_ADAPTOR_ERROR_INVALID_HANDLE,
							"Plugin handle is null");

		return CONTACT_ADAPTOR_ERROR_INVALID_HANDLE;
	}

	contact_error_code_t ret;
	plugin_req_enter();
	ret = plugin->handle->set_contact_list(context, request, user_data,
						response, error, server_data);
	plugin_req_exit(ret, plugin, error);

	return ret;
}

/**
* @brief Gets profile and service registration information of each contact
*
* @param[in]    plugin          specifies contact adaptor plugin handle
* @param[in]    context         specifies contact adaptor plugin context handle
* @param[in]    request         specifies contact adaptor contact API request handle
* @param[in]    user_data       specifies user side arbitrary data
* @param[out]   response        specifies contact adaptor contact API response handle
* @param[out]   error           specifies returned error code handle
* @param[out]   server_data     specifies server side arbitrary data
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in contact_error_code_e - CONTACT_ADAPTOR_ERROR_NONE if successful
*/
contact_error_code_t contact_adaptor_get_contact_infos_latest(contact_adaptor_plugin_h plugin,
						contact_adaptor_plugin_context_h context,
						contact_adaptor_contact_req_h request,
						void *user_data,
						contact_adaptor_contact_res_h *response,
						contact_adaptor_error_code_h *error,
						void **server_data)
{
	contact_adaptor_warning("Get contact infos latest");

	if ((NULL == plugin) || (NULL == context)) {
		contact_adaptor_error("Invalid argument");

		*error = contact_adaptor_create_error_code((int64_t) CONTACT_ADAPTOR_ERROR_INVALID_ARGUMENT,
							"Invalid argument (plugin or context)");

		return CONTACT_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		contact_adaptor_error("Plugin handle is null");

		*error = contact_adaptor_create_error_code((int64_t) CONTACT_ADAPTOR_ERROR_INVALID_HANDLE,
							"Plugin handle is null");

		return CONTACT_ADAPTOR_ERROR_INVALID_HANDLE;
	}

	contact_error_code_t ret;
	plugin_req_enter();
	ret = plugin->handle->get_contact_infos_latest(context, request, user_data,
						response, error, server_data);
	plugin_req_exit(ret, plugin, error);

	return ret;
}

/**
* @brief Gets profile and service registration information of contact that have been updated since
* last update
*
* @param[in]    plugin          specifies contact adaptor plugin handle
* @param[in]    context         specifies contact adaptor plugin context handle
* @param[in]    request         specifies contact adaptor contact API request handle
* @param[in]    user_data       specifies user side arbitrary data
* @param[out]   response        specifies contact adaptor contact API response handle
* @param[out]   error           specifies returned error code handle
* @param[out]   server_data     specifies server side arbitrary data
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in contact_error_code_e - CONTACT_ADAPTOR_ERROR_NONE if successful
*/
contact_error_code_t contact_adaptor_get_contact_infos_polling(contact_adaptor_plugin_h plugin,
						contact_adaptor_plugin_context_h context,
						contact_adaptor_contact_req_h request,
						void *user_data,
						contact_adaptor_contact_res_h *response,
						contact_adaptor_error_code_h *error,
						void **server_data)
{
	contact_adaptor_warning("Get contact infos polling");

	if ((NULL == plugin) || (NULL == context) || (NULL == request)) {
		contact_adaptor_error("Invalid argument");

		*error = contact_adaptor_create_error_code((int64_t) CONTACT_ADAPTOR_ERROR_INVALID_ARGUMENT,
							"Invalid argument (plugin or context or request)");

		return CONTACT_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		contact_adaptor_error("Plugin handle is null");

		*error = contact_adaptor_create_error_code((int64_t) CONTACT_ADAPTOR_ERROR_INVALID_HANDLE,
							"Plugin handle is null");

		return CONTACT_ADAPTOR_ERROR_INVALID_HANDLE;
	}

	contact_error_code_t ret;
	plugin_req_enter();
	ret = plugin->handle->get_contact_infos_polling(context, request, user_data,
						response, error, server_data);
	plugin_req_exit(ret, plugin, error);

	return ret;
}

/**
* @brief Sets or updates device's profile to server
*
* @param[in]    plugin          specifies contact adaptor plugin handle
* @param[in]    context         specifies contact adaptor plugin context handle
* @param[in]    request         specifies contact adaptor profile API request handle
* @param[in]    user_data       specifies user side arbitrary data
* @param[out]   response        specifies contact adaptor profile API response handle
* @param[out]   error           specifies returned error code handle
* @param[out]   server_data     specifies server side arbitrary data
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in contact_error_code_e - CONTACT_ADAPTOR_ERROR_NONE if successful
*/
contact_error_code_t contact_adaptor_set_me_profile_with_push(contact_adaptor_plugin_h plugin,
						contact_adaptor_plugin_context_h context,
						contact_adaptor_profile_req_h request,
						void *user_data,
						contact_adaptor_profile_res_h *response,
						contact_adaptor_error_code_h *error,
						void **server_data)
{
	contact_adaptor_warning("Set me profile with push");

	if ((NULL == plugin) || (NULL == context) || (NULL == request)) {
		contact_adaptor_error("Invalid argument");

		*error = contact_adaptor_create_error_code((int64_t) CONTACT_ADAPTOR_ERROR_INVALID_ARGUMENT,
							"Invalid argument (plugin or context or request)");

		return CONTACT_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		contact_adaptor_error("Plugin handle is null");

		*error = contact_adaptor_create_error_code((int64_t) CONTACT_ADAPTOR_ERROR_INVALID_HANDLE,
							"Plugin handle is null");

		return CONTACT_ADAPTOR_ERROR_INVALID_HANDLE;
	}

	contact_error_code_t ret;
	plugin_req_enter();
	ret = plugin->handle->set_me_profile_with_push(context, request, user_data,
						response, error, server_data);
	plugin_req_exit(ret, plugin, error);

	return ret;
}

/**
* @brief Gets the profile information of a contact which is correspondent with country code and phone number
*
* @param[in]    plugin          specifies contact adaptor plugin handle
* @param[in]    context         specifies contact adaptor plugin context handle
* @param[in]    request         specifies contact adaptor profile API request handle
* @param[in]    user_data       specifies user side arbitrary data
* @param[out]   response        specifies contact adaptor profile API response handle
* @param[out]   error           specifies returned error code handle
* @param[out]   server_data     specifies server side arbitrary data
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in contact_error_code_e - CONTACT_ADAPTOR_ERROR_NONE if successful
*/
contact_error_code_t contact_adaptor_get_profile(contact_adaptor_plugin_h plugin,
						contact_adaptor_plugin_context_h context,
						contact_adaptor_profile_req_h request,
						void *user_data,
						contact_adaptor_profile_res_h *response,
						contact_adaptor_error_code_h *error,
						void **server_data)
{
	contact_adaptor_warning("Get profile");

	if ((NULL == plugin) || (NULL == context) || (NULL == request)) {
		contact_adaptor_error("Invalid argument");

		*error = contact_adaptor_create_error_code((int64_t) CONTACT_ADAPTOR_ERROR_INVALID_ARGUMENT,
							"Invalid argument (plugin or context or request)");

		return CONTACT_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		contact_adaptor_error("Plugin handle is null");

		*error = contact_adaptor_create_error_code((int64_t) CONTACT_ADAPTOR_ERROR_INVALID_HANDLE,
							"Plugin handle is null");

		return CONTACT_ADAPTOR_ERROR_INVALID_HANDLE;
	}

	contact_error_code_t ret;
	plugin_req_enter();
	ret = plugin->handle->get_profile(context, request, user_data, response, error, server_data);
	plugin_req_exit(ret, plugin, error);

	return ret;
}

/**
* @brief Uploads profile image meta to file server
*
* @param[in]    plugin          specifies contact adaptor plugin handle
* @param[in]    context         specifies contact adaptor plugin context handle
* @param[in]    request         specifies contact adaptor profile API image file request handle
* @param[in]    user_data       specifies user side arbitrary data
* @param[out]   response        specifies contact adaptor profile API image file response handle
* @param[out]   error           specifies returned error code handle
* @param[out]   server_data     specifies server side arbitrary data
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in contact_error_code_e - CONTACT_ADAPTOR_ERROR_NONE if successful
*/
contact_error_code_t contact_adaptor_set_me_profile_image_meta_with_push(contact_adaptor_plugin_h plugin,
						contact_adaptor_plugin_context_h context,
						contact_adaptor_contact_image_h	*imgs,
						unsigned int imgs_len,
						void *user_data,
						contact_adaptor_error_code_h *error,
						void **server_data)
{
	contact_adaptor_warning("Set me profile image meta with push");

	if ((NULL == plugin) || (NULL == context) || (NULL == imgs) || (0U == imgs_len)) {
		contact_adaptor_error("Invalid argument");

		*error = contact_adaptor_create_error_code((int64_t) CONTACT_ADAPTOR_ERROR_INVALID_ARGUMENT,
							"Invalid argument (plugin or context or request)");

		return CONTACT_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		contact_adaptor_error("Plugin handle is null");

		*error = contact_adaptor_create_error_code((int64_t) CONTACT_ADAPTOR_ERROR_INVALID_HANDLE,
							"Plugin handle is null");

		return CONTACT_ADAPTOR_ERROR_INVALID_HANDLE;
	}

	contact_error_code_t ret;
	plugin_req_enter();
	ret = plugin->handle->set_me_profile_image_meta_with_push(context, imgs, imgs_len, user_data,
						error, server_data);
	plugin_req_exit(ret, plugin, error);

	return ret;
}

/**
* @brief Deletes profile image meta from profile server
*
* @param[in]    plugin          specifies contact adaptor plugin handle
* @param[in]    context         specifies contact adaptor plugin context handle
* @param[in]    request         specifies contact adaptor profile API image file request handle
* @param[in]    user_data       specifies user side arbitrary data
* @param[out]   response        specifies contact adaptor profile API image file response handle
* @param[out]   error           specifies returned error code handle
* @param[out]   server_data     specifies server side arbitrary data
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in contact_error_code_e - CONTACT_ADAPTOR_ERROR_NONE if successful
*/
contact_error_code_t contact_adaptor_delete_me_profile_image_meta_with_push(contact_adaptor_plugin_h plugin,
						contact_adaptor_plugin_context_h context,
						void *user_data,
						contact_adaptor_error_code_h *error,
						void **server_data)
{
	contact_adaptor_warning("Delete me profile image meta with push");

	if ((NULL == plugin) || (NULL == context)) {
		contact_adaptor_error("Invalid argument");

		*error = contact_adaptor_create_error_code((int64_t) CONTACT_ADAPTOR_ERROR_INVALID_ARGUMENT,
							"Invalid argument (plugin or context or request)");

		return CONTACT_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		contact_adaptor_error("Plugin handle is null");

		*error = contact_adaptor_create_error_code((int64_t) CONTACT_ADAPTOR_ERROR_INVALID_HANDLE,
							"Plugin handle is null");

		return CONTACT_ADAPTOR_ERROR_INVALID_HANDLE;
	}

	contact_error_code_t ret;
	plugin_req_enter();
	ret = plugin->handle->delete_me_profile_image_meta_with_push(context, user_data,
						error, server_data);
	plugin_req_exit(ret, plugin, error);

	return ret;
}

/**
* @brief Sets the level of privacy
*
* @param[in]    plugin          specifies contact adaptor plugin handle
* @param[in]    context         specifies contact adaptor plugin context handle
* @param[in]    request         specifies contact adaptor privacy API request handle
* @param[in]    user_data       specifies user side arbitrary data
* @param[out]   response        specifies contact adaptor privacy API response handle
* @param[out]   error           specifies returned error code handle
* @param[out]   server_data     specifies server side arbitrary data
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in contact_error_code_e - CONTACT_ADAPTOR_ERROR_NONE if successful
*/
contact_error_code_t contact_adaptor_set_me_profile_privacy(contact_adaptor_plugin_h plugin,
						contact_adaptor_plugin_context_h context,
						contact_adaptor_privacy_req_h request,
						void *user_data,
						contact_adaptor_privacy_res_h *response,
						contact_adaptor_error_code_h *error,
						void **server_data)
{
	contact_adaptor_warning("Set me profile privacy");

	if ((NULL == plugin) || (NULL == context) || (NULL == request)) {
		contact_adaptor_error("Invalid argument");

		*error = contact_adaptor_create_error_code((int64_t) CONTACT_ADAPTOR_ERROR_INVALID_ARGUMENT,
							"Invalid argument (plugin or context or request)");

		return CONTACT_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		contact_adaptor_error("Plugin handle is null");

		*error = contact_adaptor_create_error_code((int64_t) CONTACT_ADAPTOR_ERROR_INVALID_HANDLE,
							"Plugin handle is null");

		return CONTACT_ADAPTOR_ERROR_INVALID_HANDLE;
	}

	contact_error_code_t ret;
	plugin_req_enter();
	ret = plugin->handle->set_me_profile_privacy(context, request, user_data,
						response, error, server_data);
	plugin_req_exit(ret, plugin, error);

	return ret;
}

/**
* @brief Gets my profile's privacy level
*
* @param[in]    plugin          specifies contact adaptor plugin handle
* @param[in]    context         specifies contact adaptor plugin context handle
* @param[in]    request         specifies contact adaptor privacy API request handle
* @param[in]    user_data       specifies user side arbitrary data
* @param[out]   response        specifies contact adaptor privacy API response handle
* @param[out]   error           specifies returned error code handle
* @param[out]   server_data     specifies server side arbitrary data
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in contact_error_code_e - CONTACT_ADAPTOR_ERROR_NONE if successful
*/
contact_error_code_t contact_adaptor_get_me_profile_privacy(contact_adaptor_plugin_h plugin,
						contact_adaptor_plugin_context_h context,
						contact_adaptor_privacy_req_h request,
						void *user_data,
						contact_adaptor_privacy_res_h *response,
						contact_adaptor_error_code_h *error,
						void **server_data)
{
	contact_adaptor_warning("Get me profile privacy");

	if ((NULL == plugin) || (NULL == context)) {
		contact_adaptor_error("Invalid argument");

		*error = contact_adaptor_create_error_code((int64_t) CONTACT_ADAPTOR_ERROR_INVALID_ARGUMENT,
							"Invalid argument (plugin or context or request)");

		return CONTACT_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		contact_adaptor_error("Plugin handle is null");

		*error = contact_adaptor_create_error_code((int64_t) CONTACT_ADAPTOR_ERROR_INVALID_HANDLE,
							"Plugin handle is null");

		return CONTACT_ADAPTOR_ERROR_INVALID_HANDLE;
	}

	contact_error_code_t ret;
	plugin_req_enter();
	ret = plugin->handle->get_me_profile_privacy(context, request, user_data,
						response, error, server_data);
	plugin_req_exit(ret, plugin, error);

	return ret;
}

/**
* @brief Sets my presence information
*
* @param[in]    plugin          specifies contact adaptor plugin handle
* @param[in]    context         specifies contact adaptor plugin context handle
* @param[in]    request         specifies contact adaptor presence API request handle
* @param[in]    user_data       specifies user side arbitrary data
* @param[out]   response        specifies contact adaptor presence API response handle
* @param[out]   error           specifies returned error code handle
* @param[out]   server_data     specifies server side arbitrary data
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in contact_error_code_e - CONTACT_ADAPTOR_ERROR_NONE if successful
*/
contact_error_code_t contact_adaptor_set_me_presence_with_push(contact_adaptor_plugin_h plugin,
						contact_adaptor_plugin_context_h context,
						contact_adaptor_presence_info_h request,
						void *user_data,
						contact_adaptor_presence_info_h *response,
						contact_adaptor_error_code_h *error,
						void **server_data)
{
	contact_adaptor_warning("Set me presence with push");

	if ((NULL == plugin) || (NULL == context) || (NULL == request)) {
		contact_adaptor_error("Invalid argument");

		*error = contact_adaptor_create_error_code((int64_t) CONTACT_ADAPTOR_ERROR_INVALID_ARGUMENT,
							"Invalid argument (plugin or context or request)");

		return CONTACT_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		contact_adaptor_error("Plugin handle is null");

		*error = contact_adaptor_create_error_code((int64_t) CONTACT_ADAPTOR_ERROR_INVALID_HANDLE,
							"Plugin handle is null");

		return CONTACT_ADAPTOR_ERROR_INVALID_HANDLE;
	}

	contact_error_code_t ret;
	plugin_req_enter();
	ret = plugin->handle->set_me_presence_with_push(context, request, user_data,
						response, error, server_data);
	plugin_req_exit(ret, plugin, error);

	return ret;
}

/**
* @brief Sets my presence on/off information
*
* @param[in]    plugin          specifies contact adaptor plugin handle
* @param[in]    context         specifies contact adaptor plugin context handle
* @param[in]    request         specifies contact adaptor presence API request handle
* @param[in]    user_data       specifies user side arbitrary data
* @param[out]   response        specifies contact adaptor presence API response handle
* @param[out]   error           specifies returned error code handle
* @param[out]   server_data     specifies server side arbitrary data
* @return       0 on success, otherwise a positive error value
* @retval       error code defined in contact_error_code_e - CONTACT_ADAPTOR_ERROR_NONE if successful
*/
contact_error_code_t contact_adaptor_set_me_presence_on_off_with_push(contact_adaptor_plugin_h plugin,
						contact_adaptor_plugin_context_h context,
						contact_adaptor_presence_info_h request,
						void *user_data,
						contact_adaptor_presence_info_h *response,
						contact_adaptor_error_code_h *error,
						void **server_data)
{
	contact_adaptor_warning("Set me presence on off with push");

	if ((NULL == plugin) || (NULL == context) || (NULL == request)) {
		contact_adaptor_error("Invalid argument");

		*error = contact_adaptor_create_error_code((int64_t) CONTACT_ADAPTOR_ERROR_INVALID_ARGUMENT,
							"Invalid argument (plugin or context or request)");

		return CONTACT_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		contact_adaptor_error("Plugin handle is null");

		*error = contact_adaptor_create_error_code((int64_t) CONTACT_ADAPTOR_ERROR_INVALID_HANDLE,
							"Plugin handle is null");

		return CONTACT_ADAPTOR_ERROR_INVALID_HANDLE;
	}

	contact_error_code_t ret;
	plugin_req_enter();
	ret = plugin->handle->set_me_presence_on_off_with_push(context, request, user_data,
								response, error, server_data);
	plugin_req_exit(ret, plugin, error);

	return ret;
}

EXPORT_API
contact_error_code_t contact_adaptor_set_me_profile_type(contact_adaptor_plugin_h plugin,
						contact_adaptor_plugin_context_h context,
						int req_type,
						void *user_data,
						char **url,
						contact_adaptor_error_code_h *error,
						void **server_data)
{
	contact_adaptor_info("%s", __FUNCTION__);

	if ((NULL == plugin) || (NULL == context)) {
		contact_adaptor_error("Invalid argument");

		*error = contact_adaptor_create_error_code((int64_t) CONTACT_ADAPTOR_ERROR_INVALID_ARGUMENT,
							"Invalid argument (plugin or context or request)");

		return CONTACT_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		contact_adaptor_error("Plugin handle is null");

		*error = contact_adaptor_create_error_code((int64_t) CONTACT_ADAPTOR_ERROR_INVALID_HANDLE,
							"Plugin handle is null");

		return CONTACT_ADAPTOR_ERROR_INVALID_HANDLE;
	}

	char *_url = NULL;
	contact_error_code_t ret;
	plugin_req_enter();
	ret = plugin->handle->set_me_profile_type(context, req_type, user_data,
								&_url, error, server_data);
	plugin_req_exit(ret, plugin, error);

	contact_adaptor_debug("url : %s", _url);
	if (NULL != url) {
		*url = _url;
	} else {
		free(_url);
	}

	return ret;
}


