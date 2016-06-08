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
#include <stdint.h>
#include <string.h>
#include <dirent.h>
#include <dlfcn.h>
#include <glib.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include <plugin_message.h>

#include "auth-adaptor.h"
#include "auth-adaptor-log.h"

#define PLUGIN_MESSAGE_LISTENER_CMD_APPEND_FD		"append;"
#define PLUGIN_MESSAGE_LISTENER_CMD_STOP		"stop;"
#define PLUGIN_MESSAGE_PROTOCOL_MAX_BUF_SIZE		8192

#define AUTH_PLUGIN_INTERFACE_CREATE_CONTEXT            "create_context"
#define AUTH_PLUGIN_INTERFACE_DESTROY_CONTEXT           "destroy_context"
#define AUTH_PLUGIN_INTERFACE_DESTROY_HANDLE            "destroy_handle"
#define AUTH_PLUGIN_INTERFACE_IS_AUTH                   "is_auth"
#define AUTH_PLUGIN_INTERFACE_JOIN                      "join"
#define AUTH_PLUGIN_INTERFACE_LOGIN                     "login"
#define AUTH_PLUGIN_INTERFACE_REFRESH_ACCESS_TOKEN	"refresh"


#define IF_IS_PLUGIN_THAN_RETURN_NULL()		do {if (!g_process_identity) return NULL; } while (0)

#define SAFE_ADD_STRING(x)	((x) ? (x) : "")

typedef enum {
	PLUGIN_TYPE_INHOUSE	= 0,
	PLUGIN_TYPE_3RD_PARTY	= 1,
} auth_plugin_type_e;

/**
 * @brief Describes Auth adaptor plugin
 */
typedef struct auth_adaptor_plugin_s {
	auth_adaptor_h				adaptor;		/* Adaptor */
	char					*path;			/* Plugin library path */
	auth_adaptor_plugin_handle_h		handle;			/* Plugin handle */
	void					*dl_handle;		/* Plugin library handle */
	int					ref_counter;		/* Plugin reference counter */
	GMutex					ref_counter_mutex;	/* Plugin reference counter mutex */
	auth_adaptor_plugin_listener_h		plugin_listener;	/* Plugin callback listener */
	GMutex					plugin_listener_mutex;	/* Plugin callback listener mutex */

	GMutex					message_mutex;
	auth_plugin_type_e			type;
	int					pid;
	int					rd;
	int					wd;
	GList					*contexts;
	GMutex					contexts_mutex;
/*	GQueue					sended; */
} auth_adaptor_plugin_t;

typedef struct _plugin_message_context_s {
	int hooker;
	char *message;
} plugin_message_context_t;

/**
 * @brief Describes Auth adaptor
 */
typedef struct auth_adaptor_s {
	GMutex	auth_adaptor_mutex;		/* Adaptor mutex */
	int	started;			/* Started flag */
	char	*plugins_dir;			/* Plugins directory path */
	GList	*plugins;			/* List of loaded plugins */
	GMutex	plugins_mutex;			/* Plugin list mutex */
	GList	*adaptor_listeners;		/* List of vservice channel listener (for now not effective) */
	GMutex	adaptor_listeners_mutex;	/* Listener list mutex */

	int	rd_cmd[2];
	GList	*rd_list;
	GMutex	rd_mutex;
	pthread_t	plugin_listener;
} auth_adaptor_t;

static int g_process_identity = -1;

/**
 * @brief Creates plugin
 */
static auth_adaptor_plugin_h auth_adaptor_create_plugin(const char *plugin_path);

/**
 * @brief Destroys plugin and deletes all resources associated with it
 */
static void auth_adaptor_destroy_plugin(auth_adaptor_plugin_h plugin);

/**
 * @brief Loads plugins from selected directory
 */
static int auth_adaptor_load_plugins_from_directory(auth_adaptor_h adaptor,
						const char *dir_path);

/**
 * @brief Checks if plugin is loaded by selected plugin adaptor
 */
static int auth_adaptor_has_plugin(auth_adaptor_h adaptor,
						auth_adaptor_plugin_h plugin);

static auth_adaptor_error_code_h auth_adaptor_create_error_code(const int64_t code,
						const char *msg);
/**
 * Increases adaptor's plugin references counter
 */
static void auth_adaptor_plugin_ref(auth_adaptor_plugin_h);

/**
 * @brief Decreases adaptor's plugin references counter
 */
static void auth_adaptor_plugin_unref(auth_adaptor_plugin_h);

/*
void auth_adaptor_login_reply_cb(auth_adaptor_plugin_context_h context,
						auth_adaptor_error_code_h error_code,
						void *response)
{
	if (_service_adaptor_login_reply != NULL)
		_service_adaptor_login_reply(imsi, plugin_uri, app_id, msisdn, response);
}
*/

/* ////////////////////////////////////////////////////////////////////////////// */
/* ////////////  Internal function prototype (for forked plugin)  /////////////// */
/* ////////////////////////////////////////////////////////////////////////////// */


/* To be used by adaptor */
void *_auth_adaptor_plugin_message_collector(void *data);
void __auth_adaptor_transfer_message(const char *msg);
int __auth_adaptor_parse_message_cmd(auth_adaptor_h adaptor, char *msg);
void _auth_adaptor_send_cmd_add_fd(auth_adaptor_h adaptor, int fd);
void _auth_adaptor_send_cmd_stop_listen(auth_adaptor_h adaptor);

static int auth_adaptor_send_message_to_plugin_sync(auth_adaptor_plugin_h plugin,
						plugin_message_h send_message,
						plugin_message_h *receive_message);

/* To be used by adaptor (virtual plugin handle) */
auth_adaptor_plugin_handle_h __auth_adaptor_create_3rd_party_plugin_handle(const char *plugin_uri);

auth_error_code_t auth_plugin_send_create_context(auth_adaptor_plugin_context_h *context,
							const char *user_id,
							const char *user_password,
							const char *app_id,
							const char *app_secret,
							const char *service_name);

auth_error_code_t auth_plugin_send_destroy_context(auth_adaptor_plugin_context_h context);

auth_error_code_t auth_plugin_send_is_auth(auth_adaptor_plugin_context_h context,
							void *request,
							int *is_auth,
							auth_adaptor_error_code_h *error,
							void *response);

auth_error_code_t auth_plugin_send_join(auth_adaptor_plugin_context_h context,
							const char *device_id,
							void *request,
							auth_adaptor_error_code_h *error,
							void *response);

auth_error_code_t auth_plugin_send_login(auth_adaptor_plugin_context_h context,
							void *request,
							auth_adaptor_error_code_h *error,
							void *response);

auth_error_code_t auth_plugin_send_refresh_access_token(auth_adaptor_plugin_context_h context,
							void *request,
							auth_adaptor_error_code_h *error,
							void *response);


auth_error_code_t auth_plugin_send_set_service_status(auth_adaptor_plugin_context_h context,
							const int service_id,
							const int status,
							void *request,
							auth_adaptor_error_code_h *error,
							void *response);

auth_error_code_t auth_plugin_send_get_msisdn(auth_adaptor_plugin_context_h context,
							void *request,
							char **msisdn,
							auth_adaptor_error_code_h *error,
							void *response);

auth_error_code_t auth_plugin_send_get_service_status(auth_adaptor_plugin_context_h context,
		const int service_id,
		void *request,
		int *status,
		auth_adaptor_error_code_h *error,
		void *response);

auth_error_code_t auth_plugin_send_get_service_policy(auth_adaptor_plugin_context_h context,
		const int service_id,
		void *request,
		char **default_status,
		char **policy_feature,
		char **policy_version,
		char **policy_doc_url,
		auth_adaptor_error_code_h *error,
		void *response);

auth_error_code_t auth_plugin_send_get_server_info(auth_adaptor_plugin_context_h context,
		void *request,
		GHashTable **server_info,
		auth_adaptor_error_code_h *error,
		void *response);


/* To be used by forked plugin */
void *_auth_plugin_request_collector(void *data);
auth_adaptor_plugin_context_h __auth_plugin_get_context_by_context_id(auth_adaptor_plugin_h plugin, int context_id);
void __auth_plugin_progress_command(auth_adaptor_plugin_h plugin, char *order, char **result);


/* ------------------------------------------------------------------------
// Functions implementations
// ------------------------------------------------------------------------ */

/* ////////////////////////////////////////////////////// */
/* // Mandatory: External adaptor management function /// */
/* ////////////////////////////////////////////////////// */
auth_adaptor_h auth_adaptor_create(const char *plugins_dir)
{
	if (NULL == plugins_dir) {
		auth_adaptor_error("Invalid argument""(plugins_dir: %s)", plugins_dir);
		return NULL;
	}

	auth_adaptor_h auth_adaptor = (auth_adaptor_h) calloc(1, sizeof(auth_adaptor_t));

	if (NULL == auth_adaptor) {
		auth_adaptor_error("Critical : Memory allocation failed");
		return NULL;
	}

	/* for forked plugin */
	if (pipe(auth_adaptor->rd_cmd) == -1) {
		free(auth_adaptor);
		auth_adaptor = NULL;
		return NULL;
	}
	g_mutex_init(&auth_adaptor->rd_mutex);
	auth_adaptor->rd_list = NULL;
/*	auth_adaptor->rd_list = g_list_append(auth_adaptor->rd_list, (gpointer)auth_adaptor->rd_cmd[0]); */

	auth_adaptor->started = 0;
	auth_adaptor->plugins_dir = strdup(plugins_dir);

	g_mutex_init(&auth_adaptor->auth_adaptor_mutex);
	g_mutex_init(&auth_adaptor->plugins_mutex);
	g_mutex_init(&auth_adaptor->adaptor_listeners_mutex);

	g_mutex_lock(&auth_adaptor->adaptor_listeners_mutex);
	auth_adaptor->adaptor_listeners = NULL;
	g_mutex_unlock(&auth_adaptor->adaptor_listeners_mutex);

	g_mutex_lock(&auth_adaptor->plugins_mutex);
	auth_adaptor->plugins = NULL;
	g_mutex_unlock(&auth_adaptor->plugins_mutex);


	return auth_adaptor;
}

void auth_adaptor_destroy(auth_adaptor_h adaptor)
{
	if (NULL == adaptor) {
		auth_adaptor_error("Invalid argument""(adaptor: %p)", adaptor);
		return;
	}

	g_mutex_lock(&adaptor->auth_adaptor_mutex);
	if (adaptor->started) {
		auth_adaptor_error("Auth adaptor is running. Forcing stop before destroy");
		auth_adaptor_stop(adaptor);
	}

	g_mutex_lock(&adaptor->plugins_mutex);
	if (NULL != adaptor->plugins) {
		g_list_free_full(adaptor->plugins, (GDestroyNotify) auth_adaptor_plugin_unref);
		adaptor->plugins = NULL;
	}
	g_mutex_unlock(&adaptor->plugins_mutex);

	g_mutex_lock(&adaptor->adaptor_listeners_mutex);
	if (NULL != adaptor->adaptor_listeners) {
		g_list_free(adaptor->adaptor_listeners);
		adaptor->adaptor_listeners = NULL;
	}
	g_mutex_unlock(&adaptor->adaptor_listeners_mutex);
/*
	_service_adaptor_login_reply = NULL;
*/
	free(adaptor->plugins_dir);
	adaptor->plugins_dir = NULL;

	/* For forked plugin */
	g_list_free(adaptor->rd_list);
	close(adaptor->rd_cmd[0]);
	close(adaptor->rd_cmd[1]);

	free(adaptor);
}

int auth_adaptor_start(auth_adaptor_h adaptor)
{
	auth_adaptor_debug("Starting auth adaptor");
	if (NULL == adaptor) {
		auth_adaptor_error("Invalid argument""(adaptor: %p)", adaptor);
		return AUTH_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	g_mutex_lock(&adaptor->auth_adaptor_mutex);
	int result = AUTH_ADAPTOR_ERROR_NONE;
	if (adaptor->started) {
		auth_adaptor_error("Auth adaptor is already started");
	} else {
		adaptor->started = 1;

		pthread_t pid;
		if (pthread_create(&pid, NULL, _auth_adaptor_plugin_message_collector, (void *)adaptor)) {
			adaptor->started = 0;
			auth_adaptor_error("Could not create 3rd party plugin listener");
			result = AUTH_ADAPTOR_ERROR_NOT_FOUND;
		} else if (AUTH_ADAPTOR_ERROR_NONE != (result = auth_adaptor_load_plugins_from_directory(adaptor, adaptor->plugins_dir))) {
			_auth_adaptor_send_cmd_stop_listen(adaptor);
			adaptor->started = 0;
			auth_adaptor_error("Could not load plugins from directory");
			result = AUTH_ADAPTOR_ERROR_NOT_FOUND;
		} else {
			adaptor->plugin_listener = pid;
			auth_adaptor_info("Auth adaptor started successfully");
		}
	}
	g_mutex_unlock(&adaptor->auth_adaptor_mutex);

	return result;
}

int auth_adaptor_stop(auth_adaptor_h adaptor)
{
	if (NULL == adaptor) {
		auth_adaptor_error("Invalid argument""(adaptor: %p)", adaptor);
		return AUTH_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	g_mutex_lock(&adaptor->auth_adaptor_mutex);

	auth_adaptor_debug("stop plugin listener");
	_auth_adaptor_send_cmd_stop_listen(adaptor);
	pthread_join(adaptor->plugin_listener, NULL);
	int result = AUTH_ADAPTOR_ERROR_NONE;
	if (0 == adaptor->started) {
		result = AUTH_ADAPTOR_ERROR_STOP;
	} else {
		if (NULL != adaptor->plugins) {
			g_mutex_lock(&adaptor->plugins_mutex);
			g_list_free_full(adaptor->plugins, (GDestroyNotify) auth_adaptor_plugin_unref);
			adaptor->plugins = NULL;
			g_mutex_unlock(&adaptor->plugins_mutex);
		}
		adaptor->started = 0;
		auth_adaptor_debug("Auth adaptor stopped");
	}

	g_mutex_unlock(&adaptor->auth_adaptor_mutex);

	return result;
}

int auth_adaptor_register_listener(auth_adaptor_h adaptor,
						auth_adaptor_listener_h listener)
{
	if ((NULL == adaptor) || (NULL == listener)) {
		auth_adaptor_error("Invalid argument""(adaptor: %p, listener: %p)", adaptor, listener);
		return AUTH_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	g_mutex_lock(&adaptor->adaptor_listeners_mutex);

	adaptor->adaptor_listeners = g_list_append(adaptor->adaptor_listeners, listener);

	g_mutex_unlock(&adaptor->adaptor_listeners_mutex);
/*
	_service_adaptor_login_reply =
			(auth_adaptor_service_login_reply_cb)listener->login_reply;
*/
	return AUTH_ADAPTOR_ERROR_NONE;
}

int auth_adaptor_unregister_listener(auth_adaptor_h adaptor,
						auth_adaptor_listener_h listener)
{
	if ((NULL == adaptor) || (NULL == listener)) {
		auth_adaptor_error("Invalid argument""(adaptor: %p, listener: %p)", adaptor, listener);
		return AUTH_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	g_mutex_lock(&adaptor->adaptor_listeners_mutex);

	if (NULL == g_list_find(adaptor->adaptor_listeners, listener)) {
		g_mutex_unlock(&adaptor->adaptor_listeners_mutex);
		auth_adaptor_error("Could not find listener");
		return AUTH_ADAPTOR_ERROR_NOT_FOUND;
	}

	adaptor->adaptor_listeners = g_list_remove(adaptor->adaptor_listeners, listener);

	g_mutex_unlock(&adaptor->adaptor_listeners_mutex);
/*
	_service_adaptor_login_reply = NULL;
*/
	return AUTH_ADAPTOR_ERROR_NONE;
}

/* /////////////////////////////////////////////////////////////
   // Plugin create / destroy / ref. count / get plugin name
   ///////////////////////////////////////////////////////////// */
static auth_adaptor_plugin_h auth_adaptor_create_plugin(const char *plugin_path)
{
	if (NULL == plugin_path) {
		auth_adaptor_error("Invalid argument""(plugin_path: %p)", plugin_path);
		return NULL;

	}

	void *dl_handle = dlopen(plugin_path, RTLD_LAZY);
	if (NULL == dl_handle) {
		auth_adaptor_error("Could not load plugin %s: %s", plugin_path, dlerror());
		return NULL;
	}

	auth_adaptor_plugin_handle_h (*get_adaptee_handle)(void) = NULL;

	get_adaptee_handle = (auth_adaptor_plugin_handle_h (*)(void))(dlsym(dl_handle, "create_plugin_handle"));
	if (NULL == get_adaptee_handle) {
		dlclose(dl_handle);
		auth_adaptor_error("Could not get function pointer to create_plugin_handle");
		return NULL;
	}

	plugin_req_enter();
	auth_adaptor_plugin_handle_h handle = get_adaptee_handle();
	plugin_req_exit_void();
	if (NULL == handle) {
		dlclose(dl_handle);
		auth_adaptor_error("Could not get adaptee handle");
		return NULL;
	}

	auth_adaptor_plugin_h plugin = (auth_adaptor_plugin_h) calloc(1, sizeof(auth_adaptor_plugin_t));
	if (NULL == plugin) {
		dlclose(dl_handle);
		auth_adaptor_error("Could not create plugin object");
		return NULL;
	}

	plugin->path = g_strdup(plugin_path);
	plugin->handle = handle;
	plugin->dl_handle = dl_handle;
	plugin->ref_counter = 0;

	plugin->type = PLUGIN_TYPE_INHOUSE;

	g_mutex_init(&plugin->ref_counter_mutex);
	g_mutex_init(&plugin->plugin_listener_mutex);
	g_mutex_init(&plugin->contexts_mutex);
	plugin->contexts = NULL;

	auth_adaptor_plugin_listener_h listener =
		(auth_adaptor_plugin_listener_h) calloc(1, sizeof(auth_adaptor_plugin_listener_t));
/*
	listener->auth_adaptor_login_reply		= auth_adaptor_login_reply_cb;
*/
	plugin_req_enter();
	plugin->handle->set_listener(listener);
	plugin_req_exit_void();

	g_mutex_lock(&plugin->plugin_listener_mutex);
	plugin->plugin_listener = listener;
	g_mutex_unlock(&plugin->plugin_listener_mutex);

	return plugin;
}

static void auth_adaptor_destroy_plugin(auth_adaptor_plugin_h plugin)
{
	if (NULL == plugin) {
		auth_adaptor_error("Invalid argument""(plugin: %p)", plugin);
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

	g_free(plugin->path);
	plugin->path = NULL;

	free(plugin);
}

static int auth_adaptor_load_plugins_from_directory(auth_adaptor_h adaptor,
						const char *dir_path)
{
	char *plugin_path = NULL;
	DIR *dir = NULL;
	struct dirent dir_entry, *result = NULL;

	auth_adaptor_debug("Starting load plugins from directory");

	if ((NULL == adaptor) || (NULL == dir_path)) {
		auth_adaptor_error("Invalid argument""(adaptor: %p, dir_path: %p)", adaptor, dir_path);
		return AUTH_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	dir = opendir(dir_path);
	if (NULL == dir) {
		auth_adaptor_error("Could not open dir path (%s)", dir_path);
		return AUTH_ADAPTOR_ERROR_NOT_FOUND;
	}

	int ret = AUTH_ADAPTOR_ERROR_NONE;
	while (0 == (readdir_r(dir, &dir_entry, &result))) {

		if (NULL == result) {
			auth_adaptor_error("Could not open directory %s", plugin_path);
			break;
		}

		if (dir_entry.d_type & DT_DIR) {
			continue;
		}

		plugin_path = g_strconcat(dir_path, "/", dir_entry.d_name, NULL);
		auth_adaptor_plugin_h plugin = auth_adaptor_create_plugin(plugin_path);

		if (NULL != plugin) {
			auth_adaptor_debug("Loaded plugin: %s", plugin_path);
			plugin->adaptor = adaptor;
			auth_adaptor_plugin_ref(plugin);
			g_mutex_lock(&adaptor->plugins_mutex);
			adaptor->plugins = g_list_append(adaptor->plugins, plugin);
			g_mutex_unlock(&adaptor->plugins_mutex);
		} else {
			auth_adaptor_error("Could not load plugin %s", plugin_path);
		}

		free(plugin_path);
		plugin_path = NULL;
	}

	auth_adaptor_debug("End load plugins from directory");
	closedir(dir);
	return ret;
}

static int auth_adaptor_has_plugin(auth_adaptor_h adaptor,
						auth_adaptor_plugin_h plugin)
{
	if ((NULL == adaptor) || (NULL == plugin)) {
		auth_adaptor_error("Invalid argument""(adaptor: %p, plugin: %p)", adaptor, plugin);
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

static void auth_adaptor_plugin_ref(auth_adaptor_plugin_h plugin)
{
	if (NULL == plugin) {
		auth_adaptor_error("Invalid argument""(plugin: %p)", plugin);
		return;
	}

	g_mutex_lock(&plugin->ref_counter_mutex);
	plugin->ref_counter = plugin->ref_counter + 1;
	if (NULL != plugin->handle) {
		auth_adaptor_info("plugin name: %s, ref_counter: %d",
				plugin->handle->plugin_uri, plugin->ref_counter);
	} else {
		auth_adaptor_info("ref_counter: %d", plugin->ref_counter);
	}
	g_mutex_unlock(&plugin->ref_counter_mutex);
}

static void auth_adaptor_plugin_unref(auth_adaptor_plugin_h plugin)
{
	if (NULL == plugin) {
		auth_adaptor_error("Invalid argument""(plugin: %p)", plugin);
		return;
	}

	int should_destroy = 0;

	g_mutex_lock(&plugin->ref_counter_mutex);
	plugin->ref_counter = plugin->ref_counter - 1;
	if (NULL != plugin->handle) {
		auth_adaptor_info("plugin name: %s, ref_counter: %d",
				plugin->handle->plugin_uri, plugin->ref_counter);
	} else {
		auth_adaptor_info("ref_counter: %d", plugin->ref_counter);
	}
	if (0 >= plugin->ref_counter) {
		should_destroy = 1;
	}
	g_mutex_unlock(&plugin->ref_counter_mutex);

	if (should_destroy) {
		auth_adaptor_debug("Plugin is being destroyed");
		auth_adaptor_destroy_plugin(plugin);
	}
}

/* For 3rd party plugin packages */
int auth_adaptor_load_plugin_from_package(auth_adaptor_h adaptor,
						const char *package_id,
						const char *plugin_path)
{
	int adaptor_fd[2];
	int plugin_fd[2];

	if (pipe(adaptor_fd) == -1) {
		auth_adaptor_debug("pipe creation error, can not load plugin package");
	} else if (pipe(plugin_fd) == -1) {
		close(adaptor_fd[0]);
		close(adaptor_fd[1]);
		auth_adaptor_debug("pipe creation error[2], can not load plugin package");
	} else {
		g_process_identity = fork();
		if (0 == g_process_identity) {	/* child */
			auth_adaptor_debug_func("[CHILD PROCESS] forked success (PID : %d, id : %d)", (int)getpid());

			auth_adaptor_plugin_h plugin = NULL;
			plugin = auth_adaptor_create_plugin(plugin_path);

			if (NULL == plugin) {
				auth_adaptor_error("[CHILD PROCESS] Load plugin failed");
				exit(1);
			}

			plugin->rd = plugin_fd[0];
			close(plugin_fd[1]);
			plugin->wd = adaptor_fd[1];
			close(adaptor_fd[0]);
			void *temp = _auth_plugin_request_collector((void *)plugin);
			auth_adaptor_debug_func("[CHILD PROCESS] exit %p", temp);
			exit(0);
		} else if (0 < g_process_identity) {	/* parent */
			auth_adaptor_debug_func("[PARENT PROCESS] forked success (PID : %d)", (int)getpid());
			auth_adaptor_plugin_h _plugin = (auth_adaptor_plugin_h) calloc(1, sizeof(auth_adaptor_plugin_t));
			if (NULL != _plugin) {
				_plugin->ref_counter = 0;
				g_mutex_init(&_plugin->ref_counter_mutex);
				g_mutex_init(&_plugin->message_mutex);

				_plugin->handle = __auth_adaptor_create_3rd_party_plugin_handle(package_id);

				_plugin->type = PLUGIN_TYPE_3RD_PARTY;
				_plugin->pid = g_process_identity;
				_plugin->rd = adaptor_fd[0];
				close(adaptor_fd[1]);
				_plugin->wd = plugin_fd[1];
				close(plugin_fd[0]);

				_auth_adaptor_send_cmd_add_fd(adaptor, _plugin->rd);

				_plugin->adaptor = adaptor;
				auth_adaptor_plugin_ref(_plugin);
				g_mutex_lock(&adaptor->plugins_mutex);
				adaptor->plugins = g_list_append(adaptor->plugins, _plugin);
				g_mutex_unlock(&adaptor->plugins_mutex);
			}
		} else {
			close(adaptor_fd[0]);
			close(adaptor_fd[1]);
			close(plugin_fd[0]);
			close(plugin_fd[1]);
			auth_adaptor_debug("fork error, can not load plugin package");
		}
	}

	return 0;
}

/* //////////////////////////////////////////////////////
   // Get plugin name by plugin
   ////////////////////////////////////////////////////// */
void auth_adaptor_get_plugin_uri(auth_adaptor_plugin_h plugin,
						char **plugin_uri)
{
	if ((NULL == plugin) || (NULL == plugin_uri)) {
		auth_adaptor_error("Invalid argument""(plugin: %p, uri: %p)", plugin, plugin_uri);
		return;
	}
	if (NULL != plugin->handle) {
		*plugin_uri = strdup(plugin->handle->plugin_uri);
	}
}

/* //////////////////////////////////////////////////////
   // Create / Destroy error code
   ////////////////////////////////////////////////////// */
static auth_adaptor_error_code_h auth_adaptor_create_error_code(const int64_t code,
						const char *msg)
{
	if (NULL == msg) {
		return NULL;
	}

	auth_adaptor_error_code_h error_code =
			(auth_adaptor_error_code_h) malloc(sizeof(auth_adaptor_error_code_t));

	if (NULL != error_code) {
		error_code->code = code;
		error_code->msg = strdup(msg);
	}

	return error_code;
}

void auth_adaptor_destroy_error_code(auth_adaptor_error_code_h *error_code)
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
auth_adaptor_plugin_context_h auth_adaptor_create_plugin_context(auth_adaptor_plugin_h plugin,
						const char *user_id,
						const char *user_password,
						const char *app_id,
						const char *app_secret,
						const char *imsi,
						const char *service_name)
{
	auth_adaptor_debug("Starting auth_adaptor_create_plugin_context");

	if ((NULL == plugin) || (NULL == service_name)) {
		auth_adaptor_error("Invalid argument""(plugin: %p, service_name: %s)",
				plugin, service_name);
		return NULL;
	}

	if (NULL != plugin->handle) {
		auth_adaptor_plugin_context_h plugin_context = NULL;
		if (plugin->type == PLUGIN_TYPE_3RD_PARTY) {
			plugin_context = (auth_adaptor_plugin_context_h) calloc(1, sizeof(auth_adaptor_plugin_context_t));
			if (NULL == plugin_context) {
				auth_adaptor_error("Create context failed");
				return NULL;
			} else {
				plugin_context->plugin_handle = plugin;
			}
		}

		plugin_req_enter();
		plugin->handle->create_context(&plugin_context, SAFE_ADD_STRING(user_id), SAFE_ADD_STRING(user_password), SAFE_ADD_STRING(app_id), SAFE_ADD_STRING(app_secret), SAFE_ADD_STRING(imsi));
		plugin_req_exit_void();

		if (NULL == plugin_context) {
			auth_adaptor_error("Create context failed");
			return NULL;
		} else {
			plugin_context->plugin_uri = strdup(plugin->handle->plugin_uri);
			plugin_context->service_name = strdup(service_name);
		}

		/* For forked plugin */
		g_mutex_lock(&plugin->contexts_mutex);
		plugin->contexts = g_list_append(plugin->contexts, (gpointer)plugin_context);
		g_mutex_unlock(&plugin->contexts_mutex);

		return plugin_context;
	} else {
		auth_adaptor_error("Plugin handle is null");
	}

	auth_adaptor_debug("End auth_adaptor_create_plugin_context");
	return NULL;
}

void auth_adaptor_destroy_plugin_context(auth_adaptor_plugin_h plugin,
						auth_adaptor_plugin_context_h plugin_context)
{
	if ((NULL == plugin) || (NULL == plugin_context)) {
		auth_adaptor_error("Invalid argument""(plugin: %p, plugin_context: %p)", plugin, plugin_context);
		return;
	}

	if (NULL != plugin->handle) {
		/* For forked plugin */
		g_mutex_lock(&plugin->contexts_mutex);
		plugin->contexts = g_list_remove(plugin->contexts, (gpointer)plugin_context);
		g_mutex_unlock(&plugin->contexts_mutex);

		plugin_req_enter();
		plugin->handle->destroy_context(plugin_context);
		plugin_req_exit_void();
	} else {
		auth_adaptor_error("Plugin handle is null");
	}

	return;
}

/* //////////////////////////////////////////////////////
   // Get plugin by plugin name
   ////////////////////////////////////////////////////// */
auth_adaptor_plugin_h auth_adaptor_get_plugin_by_name(auth_adaptor_h adaptor,
						const char *plugin_uri)
{
	auth_adaptor_debug("Starting auth_adaptor_get_plugin_by_name");

	if ((NULL == adaptor) || (NULL == plugin_uri)) {
		auth_adaptor_error("Invalid argument""(adaptor: %p, plugin_uri: %p)", adaptor, plugin_uri);
		return NULL;
	}

	auth_adaptor_plugin_h plugin = NULL;
	g_mutex_lock(&adaptor->plugins_mutex);

	int count = g_list_length(adaptor->plugins);
	int i = 0;
	for (i = 0; i < count; i++) {
		auth_adaptor_plugin_h temp_plugin = g_list_nth_data(adaptor->plugins, i);
		if (NULL != temp_plugin) {
			if (0 == strcmp(temp_plugin->handle->plugin_uri, plugin_uri)) {
				auth_adaptor_plugin_ref(temp_plugin);
				plugin = temp_plugin;
				g_mutex_unlock(&adaptor->plugins_mutex);
				return plugin;
			}
		}
	}
	g_mutex_unlock(&adaptor->plugins_mutex);

	return plugin;
}

/* //////////////////////////////////////////////////////
   // Plugin load / unload / get plugin list
   ////////////////////////////////////////////////////// */
int auth_adaptor_load_plugin(auth_adaptor_h adaptor,
						const char *plugin_path)
{
	if ((NULL == adaptor) || (NULL == plugin_path)) {
		auth_adaptor_error("Invalid argument""(adaptor: %p, plugin_path: %p)", adaptor, plugin_path);
		return AUTH_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (0 == adaptor->started) {
		auth_adaptor_error("Auth adaptor is not started");
		return AUTH_ADAPTOR_ERROR_START;
	}

	auth_adaptor_plugin_h plugin = auth_adaptor_create_plugin(plugin_path);
	if (NULL == plugin) {
		auth_adaptor_error("Could not load plugin %s", plugin_path);
		return AUTH_ADAPTOR_ERROR_CREATE;
	}

	plugin->adaptor = adaptor;
	auth_adaptor_plugin_ref(plugin);

	g_mutex_lock(&adaptor->plugins_mutex);
	adaptor->plugins = g_list_append(adaptor->plugins, plugin);
	g_mutex_unlock(&adaptor->plugins_mutex);

	return AUTH_ADAPTOR_ERROR_NONE;
}

int auth_adaptor_unload_plugin(auth_adaptor_h adaptor,
						auth_adaptor_plugin_h plugin)
{
	if ((NULL == adaptor) || (NULL == plugin)) {
		auth_adaptor_error("Invalid argument""(adaptor: %p, plugin: %p)", adaptor, plugin);
		return AUTH_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (0 == adaptor->started) {
		auth_adaptor_error("Auth adaptor is not started");
		return AUTH_ADAPTOR_ERROR_START;
	}

	if (0 == auth_adaptor_has_plugin(adaptor, plugin)) {
		auth_adaptor_error("Auth adaptor has no plugin");
		return AUTH_ADAPTOR_ERROR_NOT_FOUND;
	}

	plugin->adaptor = NULL;

	g_mutex_lock(&adaptor->plugins_mutex);
	adaptor->plugins = g_list_remove(adaptor->plugins, plugin);
	g_mutex_unlock(&adaptor->plugins_mutex);

	auth_adaptor_plugin_unref(plugin);

	return AUTH_ADAPTOR_ERROR_NONE;
}

GList *auth_adaptor_get_plugins(auth_adaptor_h adaptor)
{
	if (NULL == adaptor) {
		auth_adaptor_error("Invalid argument""(adaptor: %p)", adaptor);
		return NULL;
	}

	GList *plugins = NULL;

	g_mutex_lock(&adaptor->plugins_mutex);
	int plugins_count = g_list_length(adaptor->plugins);
	int i;
	for (i = 0; i < plugins_count; i++) {
		auth_adaptor_plugin_h plugin = g_list_nth_data(adaptor->plugins, i);
		if (NULL != plugin) {
			auth_adaptor_plugin_ref(plugin);
			plugins = g_list_append(plugins, plugin);
		}
	}
	g_mutex_unlock(&adaptor->plugins_mutex);

	return plugins;
}

/* ////////////////////////////////////////////////////////////
   // Adaptor get Element Functions
   //////////////////////////////////////////////////////////// */

/*
 * @return : Need free
 */
EXPORT_API
char *auth_adaptor_get_access_token_dup(auth_adaptor_plugin_context_h context)
{
	if (NULL == context) {
		auth_adaptor_warning("Invalid argument""(context: %p", context);
		return NULL;
	}

	if (NULL == context->access_token) {
		auth_adaptor_warning("Invalid argument""(context->access_token: %p", context->access_token);
		return NULL;
	}

	if (0 >= strlen(context->access_token)) {
		auth_adaptor_warning("Invalid argument""(context->access_token length: %d", strlen(context->access_token));
		return NULL;
	}

	return strdup(context->access_token);
}

/*
 * @return : Need free
 */
EXPORT_API
char *auth_adaptor_get_uid_dup(auth_adaptor_plugin_context_h context)
{
	if (NULL == context) {
		auth_adaptor_warning("Invalid argument""(context: %p", context);
		return NULL;
	}

	if (NULL == context->uid) {
		auth_adaptor_warning("Invalid argument""(context->uid: %p", context->uid);
		return NULL;
	}

	if (0 >= strlen(context->uid)) {
		auth_adaptor_warning("Invalid argument""(context->uid length: %d", strlen(context->uid));
		return NULL;
	}

	return strdup(context->uid);
}

/*
 * @return : Need free
 */
EXPORT_API
char *auth_adaptor_get_msisdn_dup(auth_adaptor_plugin_context_h context)
{
	if (NULL == context) {
		auth_adaptor_warning("Invalid argument""(context: %p", context);
		return NULL;
	}

	if (NULL == context->msisdn) {
		auth_adaptor_warning("Invalid argument""(context->msisdn: %p", context->msisdn);
		return NULL;
	}

	if (0 >= strlen(context->msisdn)) {
		auth_adaptor_warning("Invalid argument""(context->msisdn length: %d", strlen(context->msisdn));
		return NULL;
	}

	return strdup(context->msisdn);
}

/* ////////////////////////////////////////////////////////////
   // Adaptor Plugin call Functions
   //////////////////////////////////////////////////////////// */

auth_error_code_t _auth_adaptor_mandatory_param_check(auth_adaptor_plugin_h plugin,
						auth_adaptor_plugin_context_h context,
						auth_adaptor_error_code_h *error_code)
{
	auth_error_code_t ret = AUTH_ADAPTOR_ERROR_NONE;

	if ((NULL == plugin) || (NULL == context)) {
		auth_adaptor_error("Invalid argument""(plugin: %p, context: %p)", plugin, context);
		if (NULL != error_code) {
			*error_code = auth_adaptor_create_error_code(
					(int64_t) AUTH_ADAPTOR_ERROR_INVALID_ARGUMENT,
					"Invalid argument (plugin or context)");
		}

		return AUTH_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		auth_adaptor_error("Plugin handle is null");

		if (NULL != error_code) {
			*error_code = auth_adaptor_create_error_code(
					(int64_t) AUTH_ADAPTOR_ERROR_INVALID_HANDLE,
					"Plugin handle is null");
		}

		return AUTH_ADAPTOR_ERROR_INVALID_HANDLE;
	}

	return ret;
}

/**
* @brief Check Account Registration [Sync API]
*
* @param[in]	plugin		specifies Auth-adaptor Plugin handle
* @param[in]	context		specifies Auth-adaptor Plugin context
* @param[in]	request		specifies Optional value for specific Plugin (Recommend Json Object format)
* @param[out]	is_auth		specifies Registered Flag (Registered : 1, Not registered : 0)
* @param[out]	error_code	specifies Error code
* @param[out]	response	specifies Optional value from specific Plugin (Recommend Json Object format)
* @return 0 on success, otherwise a positive error value
* @retval error code defined in auth_error_code_e - AUTH_ADAPTOR_ERROR_NONE if Successful
*/
auth_error_code_t auth_adaptor_is_auth(auth_adaptor_plugin_h plugin,
						auth_adaptor_plugin_context_h context,
						void *request,
						int *is_auth,
						auth_adaptor_error_code_h *error_code, void *response)
{
	auth_error_code_t ret = _auth_adaptor_mandatory_param_check(plugin, context, error_code);
	if (AUTH_ADAPTOR_ERROR_NONE != ret) {
		return ret;
	}

	if ((NULL == is_auth)) {
		auth_adaptor_error("Invalid argument""(is_auth: %p)", is_auth);
		if (NULL != error_code) {
			*error_code = auth_adaptor_create_error_code(
					(int64_t) AUTH_ADAPTOR_ERROR_INVALID_ARGUMENT,
					"Invalid argument (is_auth pointer is null)");
		}

		return AUTH_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle->is_auth) {
		auth_adaptor_error("Plugin does not support this API");
		if (NULL != error_code) {
			*error_code = auth_adaptor_create_error_code(
					(int64_t) AUTH_ADAPTOR_ERROR_UNSUPPORTED,
					"Plugin does not support this API (is_auth)");
		}
		return AUTH_ADAPTOR_ERROR_UNSUPPORTED;
	}

	plugin_req_enter();
	ret = plugin->handle->is_auth(context, request, is_auth, error_code, response);
	plugin_req_exit(ret, plugin, error_code);

	return ret;
}

/**
* @brief Request Access Token (included auth_adaptor_context) [Sync API]
*
* @param[in]	plugin		specifies Auth-adaptor Plugin handle
* @param[in]	context		specifies Auth-adaptor Plugin context
* @param[in]	is_auth		specifies Registration Flag (Must be issued from "auth_adaptor_is_auth" Function)
* @param[in]	request		specifies Optional value for specific Plugin (Recommend Json Object format)
* @param[out]	error_code	specifies Error code
* @param[out]	response	specifies Optional value from specific Plugin (Recommend Json Object format)
* @return 0 on success, otherwise a positive error value
* @retval error code defined in auth_error_code_e - AUTH_ADAPTOR_ERROR_NONE if Successful
*/
auth_error_code_t auth_adaptor_login(auth_adaptor_plugin_h plugin,
						auth_adaptor_plugin_context_h context,
						int is_auth,
						void *request,
						auth_adaptor_error_code_h *error_code,
						void *response)
{
	auth_error_code_t ret = _auth_adaptor_mandatory_param_check(plugin, context, error_code);
	if (AUTH_ADAPTOR_ERROR_NONE != ret) {
		return ret;
	}

	if ((0 > is_auth) || (1 < is_auth)) {
		auth_adaptor_error("\"is_auth\" value must be issued from \"auth_adaptor_is_auth\"");

		if (NULL != error_code) {
			*error_code = auth_adaptor_create_error_code(
					(int64_t) AUTH_ADAPTOR_ERROR_INVALID_ARGUMENT,
					"Invalid argument (is_auth)");
		}

		return AUTH_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (0 == is_auth) {
		auth_adaptor_error("Device is not joined. Please register to ESU");

		if (NULL != error_code) {
			*error_code = auth_adaptor_create_error_code(
					(int64_t) AUTH_ADAPTOR_ERROR_PLUGIN_INTERNAL,
					"Device is not joined");
		}

		return AUTH_ADAPTOR_ERROR_PLUGIN_INTERNAL;
	}


	char *old_access_token = auth_adaptor_get_access_token_dup(context);
	char *old_uid = auth_adaptor_get_uid_dup(context);
	free(context->uid);
	free(context->access_token);
	context->access_token = NULL;
	context->uid = NULL;

	plugin_req_enter();
	ret = plugin->handle->login(context, request, error_code, response);
	plugin_req_exit(ret, plugin, error_code);

	if (AUTH_ADAPTOR_ERROR_NONE == ret) {
		auth_adaptor_info("Login Successed");

		if (NULL == context->access_token)
		/* || (NULL == context->uid)) */
		{
			auth_adaptor_error("Critical missmatching Error!!\n");
			auth_adaptor_error("Login returns success but access_token is NULL");
			ret = AUTH_ADAPTOR_ERROR_PLUGIN_INTERNAL;

			if (NULL != error_code) {
				*error_code = auth_adaptor_create_error_code((int64_t) AUTH_ADAPTOR_ERROR_PLUGIN_INTERNAL,
						"Plugin returns Login success but access_token is Empty");
			}
		}

	}
	if (NULL != context->access_token) {
		free(old_access_token);
	} else {
		auth_adaptor_error("Fill old access token by error");
		context->access_token = old_access_token;
	}
	if (NULL != context->uid) {
		free(old_uid);
	} else {
		auth_adaptor_error("Fill old uid by error");
		context->uid = old_uid;
	}

	return ret;
}

/**
* @brief Request for Refreshing Access Token (included auth_adaptor_context) [Sync API]
*
* @param[in]	plugin		specifies Auth-adaptor Plugin handle
* @param[in]	context		specifies Auth-adaptor Plugin context
* @param[in]	request		specifies Optional value for specific Plugin (Recommend Json Object format)
* @param[out]	error_code	specifies Error code
* @param[out]	response	specifies Optional value from specific Plugin (Recommend Json Object format)
* @return 0 on success, otherwise a positive error value
* @retval error code defined in auth_error_code_e - AUTH_ADAPTOR_ERROR_NONE if Successful
*/
auth_error_code_t auth_adaptor_login_refresh(auth_adaptor_plugin_h plugin,
						auth_adaptor_plugin_context_h context,
						void *request,
						auth_adaptor_error_code_h *error_code,
						void *response)
{
	auth_error_code_t ret = _auth_adaptor_mandatory_param_check(plugin, context, error_code);
	if (AUTH_ADAPTOR_ERROR_NONE != ret) {
		return ret;
	}

	if ((NULL == context->access_token) || (0 >= strlen(context->access_token))) {
		auth_adaptor_error("Access_token is empty. Please login first");

		if (NULL != error_code) {
			*error_code = auth_adaptor_create_error_code(
					(int64_t) AUTH_ADAPTOR_ERROR_INVALID_ARGUMENT,
					"Invalid argument (context->access_token)");
		}

		return AUTH_ADAPTOR_ERROR_NOT_AUTHORIZED;
	}

	char *old_access_token = auth_adaptor_get_access_token_dup(context);
	char *old_uid = auth_adaptor_get_uid_dup(context);
/*	free(context->uid); */
/*	free(context->access_token); */
/*	context->access_token = NULL; */
/*	context->uid = NULL; */
	plugin_req_enter();
	ret = plugin->handle->refresh_access_token(context, request, error_code, response);
	plugin_req_exit(ret, plugin, error_code);

	if (AUTH_ADAPTOR_ERROR_NONE == ret) {
		auth_adaptor_info("Login refresh Successed");
/*
		if ((NULL == context->access_token) || (NULL == context->uid))
		{
			auth_adaptor_error("Critical missmatching Error!!\n");
			auth_adaptor_error("Login_refresh returns success but access_token or uid is NULL");
			ret = AUTH_ADAPTOR_ERROR_PLUGIN_INTERNAL;

			if (NULL != error_code)
			{
				*error_code = auth_adaptor_create_error_code((int64_t) AUTH_ADAPTOR_ERROR_PLUGIN_INTERNAL,
						"Plugin returns Login_refresh success but access_token or uid is Empty");
			}
		}
*/
	}

	if (NULL != context->access_token) {
		free(old_access_token);
	} else {
		auth_adaptor_error("Fill old access token by error");
		context->access_token = old_access_token;
	}
	if (NULL != context->uid) {
		free(old_uid);
	} else {
		auth_adaptor_error("Fill old uid by error");
		context->uid = old_uid;
	}

	return ret;
}

/**
* @brief Request Account Registration [Sync API]
*
* @param[in]	plugin		specifies Auth-adaptor Plugin handle
* @param[in]	context		specifies Auth-adaptor Plugin context
* @param[in]	device_id	specifies Device Unique ID
* @param[in]	request		specifies Optional value for specific Plugin (Recommend Json Object format)
* @param[out]	error_code	specifies Error code
* @param[out]	response	specifies Optional value from specific Plugin (Recommend Json Object format)
* @return 0 on success, otherwise a positive error value
* @retval error code defined in auth_error_code_e - AUTH_ADAPTOR_ERROR_NONE if Successful
*/
auth_error_code_t auth_adaptor_join(auth_adaptor_plugin_h plugin,
						auth_adaptor_plugin_context_h context,
						const char *device_id,
						void *request,
						auth_adaptor_error_code_h *error_code,
						void *response)
{
	auth_error_code_t ret = _auth_adaptor_mandatory_param_check(plugin, context, error_code);
	if (AUTH_ADAPTOR_ERROR_NONE != ret) {
		return ret;
	}

	if (NULL == plugin->handle->join) {
		auth_adaptor_error("Plugin does not support this API");
		if (NULL != error_code) {
			*error_code = auth_adaptor_create_error_code(
					(int64_t) AUTH_ADAPTOR_ERROR_UNSUPPORTED,
					"Plugin does not support this API (is_auth)");
		}
		return AUTH_ADAPTOR_ERROR_UNSUPPORTED;
	}

	plugin_req_enter();
	ret = plugin->handle->join(context, device_id, request, error_code, response);
	plugin_req_exit(ret, plugin, error_code);

	return ret;
}

/**
* @brief Request Account Information (MSISDN) [Sync API]
*
* @param[in]	plugin		specifies Auth-adaptor Plugin handle
* @param[in]	context		specifies Auth-adaptor Plugin context
* @param[in]	request		specifies Optional value for specific Plugin (Recommend Json Object format)
* @param[out]	msisdn		specifies MSISDN
* @param[out]	error_code	specifies Error code
* @param[out]	response	specifies Optional value from specific Plugin (Recommend Json Object format)
* @return 0 on success, otherwise a positive error value
* @retval error code defined in auth_error_code_e - AUTH_ADAPTOR_ERROR_NONE if Successful
*/
auth_error_code_t auth_adaptor_get_msisdn(auth_adaptor_plugin_h plugin,
						auth_adaptor_plugin_context_h context,
						void *request,
						char **msisdn,
						auth_adaptor_error_code_h *error_code,
						void *response)
{
	auth_error_code_t ret = _auth_adaptor_mandatory_param_check(plugin, context, error_code);
	if (AUTH_ADAPTOR_ERROR_NONE != ret) {
		return ret;
	}

	plugin_req_enter();
	ret = plugin->handle->get_msisdn(context, request, msisdn, error_code, response);
	plugin_req_exit(ret, plugin, error_code);

	return ret;
}

/**
* @brief Request Changing Service Status (Enable, Disable or etc...) [Sync API]
*
* @param[in]	plugin		specifies Auth-adaptor Plugin handle
* @param[in]	context		specifies Auth-adaptor Plugin context
* @param[in]	service_id	specifies Service ID (The value is depended by Server)
* @param[in]	status		specifies Service Status (The value is depended by Server)
* @param[in]	request		specifies Optional value for specific Plugin (Recommend Json Object format)
* @param[out]	error_code	specifies Error code
* @param[out]	response	specifies Optional value from specific Plugin (Recommend Json Object format)
* @return 0 on success, otherwise a positive error value
* @retval error code defined in auth_error_code_e - AUTH_ADAPTOR_ERROR_NONE if Successful
*/
auth_error_code_t auth_adaptor_set_service_status(auth_adaptor_plugin_h plugin,
						auth_adaptor_plugin_context_h context,
						const int service_id,
						const int status,
						void *request,
						auth_adaptor_error_code_h *error_code,
						void *response)
{
	auth_error_code_t ret = _auth_adaptor_mandatory_param_check(plugin, context, error_code);
	if (AUTH_ADAPTOR_ERROR_NONE != ret) {
		return ret;
	}

	plugin_req_enter();
	ret = plugin->handle->set_service_status(context, service_id, status, request, error_code, response);
	plugin_req_exit(ret, plugin, error_code);

	return ret;
}

/**
* @brief Request Service Status [Sync API]
*
* @param[in]	plugin		specifies Auth-adaptor Plugin handle
* @param[in]	context		specifies Auth-adaptor Plugin context
* @param[in]	service_id	specifies Service ID (The value is depended by Server)
* @param[in]	request		specifies Optional value for specific Plugin (Recommend Json Object format)
* @param[out]	status		specifies Service Status (The value is depended by Server)
* @param[out]	error_code	specifies Error code
* @param[out]	response	specifies Optional value from specific Plugin (Recommend Json Object format)
* @return 0 on success, otherwise a positive error value
* @retval error code defined in auth_error_code_e - AUTH_ADAPTOR_ERROR_NONE if Successful
*/
auth_error_code_t auth_adaptor_get_service_status(auth_adaptor_plugin_h plugin,
						auth_adaptor_plugin_context_h context,
						const int service_id,
						void *request,
						int *status,
						auth_adaptor_error_code_h *error_code,
						void *response)
{
	auth_error_code_t ret = _auth_adaptor_mandatory_param_check(plugin, context, error_code);
	if (AUTH_ADAPTOR_ERROR_NONE != ret) {
		return ret;
	}

	plugin_req_enter();
	ret = plugin->handle->get_service_status(context, service_id, request, status, error_code, response);
	plugin_req_exit(ret, plugin, error_code);

	return ret;
}

/**
* @brief Request Service Policy [Sync API]
*
* @param[in]	plugin		specifies Auth-adaptor Plugin handle
* @param[in]	context		specifies Auth-adaptor Plugin context
* @param[in]	service_id	specifies Service ID (The value is depended by Server)
* @param[in]	request		specifies Optional value for specific Plugin (Recommend Json Object format)
* @param[in]	default_status	specifies Service default Status Policy
* @param[in]	policy_feature	specifies Service Policy Feature
* @param[in]	policy_version	specifies Service Policy Version
* @param[in]	policy_doc_url	specifies Service Policy Document URL
* @param[out]	error_code	specifies Error code
* @param[out]	response	specifies Optional value from specific Plugin (Recommend Json Object format)
* @return 0 on success, otherwise a positive error value
* @retval error code defined in auth_error_code_e - AUTH_ADAPTOR_ERROR_NONE if Successful
*/
auth_error_code_t auth_adaptor_get_service_policy(auth_adaptor_plugin_h plugin,
						auth_adaptor_plugin_context_h context,
						const int service_id,
						void *request,
						char **default_status,
						char **policy_feature,
						char **policy_version,
						char **policy_doc_url,
						auth_adaptor_error_code_h *error_code,
						void *response)
{
	auth_error_code_t ret = _auth_adaptor_mandatory_param_check(plugin, context, error_code);
	if (AUTH_ADAPTOR_ERROR_NONE != ret) {
		return ret;
	}

	plugin_req_enter();
	ret = plugin->handle->get_service_policy(context, service_id, request, default_status,
			policy_feature, policy_version, policy_doc_url, error_code, response);
	plugin_req_exit(ret, plugin, error_code);

	return ret;
}

/**
* @brief Request Permitted Server information for Account [Sync API]
*
* @param[in]	plugin		specifies Auth-adaptor Plugin handle
* @param[in]	context		specifies Auth-adaptor Plugin context
* @param[in]	request		specifies Optional value for specific Plugin (Recommend Json Object format)
* @param[out]	server_info	specifies server information (URL, Scheme, Port)
* @param[out]	error_code	specifies Error code
* @param[out]	response	specifies Optional value from specific Plugin (Recommend Json Object format)
* @return 0 on success, otherwise a positive error value
* @retval error code defined in auth_error_code_e - AUTH_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
auth_error_code_t auth_adaptor_get_server_info(auth_adaptor_plugin_h plugin,
						auth_adaptor_plugin_context_h context,
						void *request,
						GHashTable **server_info,
						auth_adaptor_error_code_h *error_code,
						void *response)
{
	auth_error_code_t ret = _auth_adaptor_mandatory_param_check(plugin, context, error_code);
	if (AUTH_ADAPTOR_ERROR_NONE != ret) {
		return ret;
	}

	plugin_req_enter();
	ret = plugin->handle->get_server_info(context, request, server_info, error_code, response);
	plugin_req_exit(ret, plugin, error_code);

	return ret;
}

EXPORT_API
auth_error_code_t auth_adaptor_external_request(auth_adaptor_plugin_h plugin,
						auth_adaptor_plugin_context_h context,
						const char *api_uri,
						const unsigned char *req_bundle_raw,
						int req_len,
						unsigned char **res_bundle_raw,
						int *res_len,
						auth_adaptor_error_code_h *error_code)
{
	auth_error_code_t ret = AUTH_ADAPTOR_ERROR_NONE;

	if (NULL == plugin) {
		auth_adaptor_error("Invalid argument""(plugin: %p)", plugin);
		if (NULL != error_code) {
			*error_code = auth_adaptor_create_error_code(
					(int64_t) AUTH_ADAPTOR_ERROR_INVALID_ARGUMENT,
					"Invalid argument (plugin)");
		}

		return AUTH_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		auth_adaptor_error("Plugin handle is null");

		if (NULL != error_code) {
			*error_code = auth_adaptor_create_error_code(
					(int64_t) AUTH_ADAPTOR_ERROR_INVALID_HANDLE,
					"Plugin handle is null");
		}

		return AUTH_ADAPTOR_ERROR_INVALID_HANDLE;
	}

	if (NULL ==  plugin->handle->external_request) {
		if (NULL != error_code) {
			*error_code = auth_adaptor_create_error_code(AUTH_ADAPTOR_ERROR_UNSUPPORTED,
					"API does not supported by plugin");
		}
		return AUTH_ADAPTOR_ERROR_UNSUPPORTED;
	}

	plugin_req_enter();
	ret = plugin->handle->external_request(context, api_uri,
			req_bundle_raw, req_len,
			res_bundle_raw, res_len, error_code);
	plugin_req_exit(ret, plugin, error_code);

	return ret;
}

/* ///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////  Internal function description (for forked plugin)  //////////////
///////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////// */

void *_auth_adaptor_plugin_message_collector(void *data)
{
	auth_adaptor_h adaptor = (auth_adaptor_h) data;

	auth_adaptor_info("3rd party plugin listener run");
	int i, lagest_fd = -1;
	fd_set read_set;
	struct timeval tv;
	tv.tv_sec = 10L;		/* TODO change to define or meaningful value */
	tv.tv_usec = 0L;		/* TODO change to define or meaningful value */
	char msg_buf[PLUGIN_MESSAGE_PROTOCOL_MAX_BUF_SIZE] = {0, };
	int buf_size, rcv_len;
	GList *dead_list = NULL;

	while (1) {
		/* Clears and sets fds for select */
		FD_ZERO(&read_set);
		FD_SET(adaptor->rd_cmd[0], &read_set);
		lagest_fd = adaptor->rd_cmd[0];

		/* Sets plugin fds for select */
		for (i = 0; i < g_list_length(adaptor->rd_list); i++) {
			int fd = (int) g_list_nth_data(adaptor->rd_list, i);
			FD_SET(fd, &read_set);
			if (lagest_fd < fd) {
				lagest_fd = fd;
			}
		}

		/* Select with timeout (for avoid blocking issue) */
		int stmt = select((lagest_fd + 1), &read_set, NULL, NULL, &tv);
		IF_IS_PLUGIN_THAN_RETURN_NULL();
		if (stmt == 0) {
/*			auth_adaptor_debug("select refrech by timeout(%ld sec) [id : %d]", tv.tv_sec, g_process_identity); */
			if (0L >= tv.tv_sec) {
/*				auth_adaptor_debug("Resets selector timeout sec"); */
				tv.tv_sec = 10L;
			}
			IF_IS_PLUGIN_THAN_RETURN_NULL();
		} else if (stmt > 0) {
			/* Checking message queue with Plugin processes. */
			for (i = 0; i < g_list_length(adaptor->rd_list); i++) {
				IF_IS_PLUGIN_THAN_RETURN_NULL();
				int fd = (int) g_list_nth_data(adaptor->rd_list, i);
				if (FD_ISSET(fd, &read_set)) {
					IF_IS_PLUGIN_THAN_RETURN_NULL();
					/* pre-read buf size */
					rcv_len = read(fd, &buf_size, sizeof(int));
					if (0 >= rcv_len) {
						auth_adaptor_debug("Child process dead (Remove from listening queue)");
						dead_list = g_list_append(dead_list, (gpointer)fd);
						continue;
					}
					/* allocates and read buf data */
					memset(msg_buf, 0, PLUGIN_MESSAGE_PROTOCOL_MAX_BUF_SIZE);
					buf_size %= PLUGIN_MESSAGE_PROTOCOL_MAX_BUF_SIZE - 1;
					rcv_len = read(fd, msg_buf, buf_size);
					auth_adaptor_debug("read message [%s][%d]", msg_buf, rcv_len);

					if (0 < rcv_len) {
						/* transfer data to adaptor */
						__auth_adaptor_transfer_message(msg_buf);
					} else {
						auth_adaptor_debug("Child process dead (Remove from listening queue)");
						dead_list = g_list_append(dead_list, (gpointer)fd);
					}
				}
			}

			/* Checking message queue with Adaptor internal command. */
			IF_IS_PLUGIN_THAN_RETURN_NULL();
			if (FD_ISSET(adaptor->rd_cmd[0], &read_set)) {
				int fd = adaptor->rd_cmd[0];
				IF_IS_PLUGIN_THAN_RETURN_NULL();
				/* pre-read buf size */
				rcv_len = read(fd, &buf_size, sizeof(int));

				if (0 >= rcv_len) {
					auth_adaptor_debug("Parent process dead : Listener break");
					break;
				}

				/* allocates and read buf data */
				memset(msg_buf, 0, PLUGIN_MESSAGE_PROTOCOL_MAX_BUF_SIZE);
				buf_size %= PLUGIN_MESSAGE_PROTOCOL_MAX_BUF_SIZE - 1;
				rcv_len = read(fd, msg_buf, buf_size);
				auth_adaptor_debug("read message [%s][%d]", msg_buf, rcv_len);

				if (0 >= rcv_len) {
					auth_adaptor_debug("Parent process dead : Listener break");
					break;
				}

				/* parse cmd message (e.g. append read_fd / change timeout sec / stop listener) */
				int cmd_ret = __auth_adaptor_parse_message_cmd(adaptor, msg_buf);
				if (0 > cmd_ret) {
					auth_adaptor_info("3rd party plugin listener stopped by adaptor cmd");
					break;
				}
			}

			/* Remove fd with disconnected plugin. */
			for (i = 0; i < g_list_length(dead_list); i++) {
				adaptor->rd_list = g_list_remove(adaptor->rd_list, (gpointer) g_list_nth_data(dead_list, i));
			}
			g_list_free(dead_list);
			dead_list = NULL;
		} else {
			auth_adaptor_error("plugin message listener error (errno : %d)", errno);
		}
	}
	auth_adaptor_info("3rd party plugin listener stopped");

	return data;
}

void __auth_adaptor_transfer_message(const char *msg)
{
	plugin_message_h t_msg = NULL;
	int ret = 0;
	ret = plugin_message_deserialize(msg, &t_msg);
	if (!ret) {
		pmnumber req_type, req_id;
		ret = plugin_message_get_value_number(t_msg, PLUGIN_MESSAGE_ELEMENT_MESSAGE_TYPE, &req_type);

		if (!ret && (PLUGIN_MESSAGE_TYPE_FUNCTION == req_type)) {
			auth_adaptor_debug("MEssage function type : function");
			ret = plugin_message_get_value_number(t_msg, PLUGIN_MESSAGE_ELEMENT_REQUEST_ID, &req_id);
			if (!ret) {
				auth_adaptor_debug("Send plugin data to requester");
				int hooked_fd = (int) req_id;
				int len = strlen(msg);
				ret = write(hooked_fd, &len, sizeof(int));
				if (0 < len) {
					ret = write(hooked_fd, msg, sizeof(char) * len);
				}
			} else {
				auth_adaptor_debug("Couldn't get request id");
			}
		} else if (!ret && (PLUGIN_MESSAGE_TYPE_CALLBACK == req_type)) {
			auth_adaptor_warning("Auth adaptor unsupport callback yet");
		} else {
			auth_adaptor_warning("Received message parsing fail.");
		}
		plugin_message_destroy(t_msg);
	}
}

int __auth_adaptor_parse_message_cmd(auth_adaptor_h adaptor, char *msg)
{
	char *cmd_data = NULL;
	if (0 == strncmp(PLUGIN_MESSAGE_LISTENER_CMD_APPEND_FD, msg, strlen(PLUGIN_MESSAGE_LISTENER_CMD_APPEND_FD))) {
		cmd_data = msg + strlen(PLUGIN_MESSAGE_LISTENER_CMD_APPEND_FD);
		int fd = atoi(cmd_data);

		adaptor->rd_list = g_list_append(adaptor->rd_list, (gpointer)fd);
	} else if (0 == strncmp(PLUGIN_MESSAGE_LISTENER_CMD_STOP, msg, strlen(PLUGIN_MESSAGE_LISTENER_CMD_STOP))) {
		return -1;
	}

	return 0;
}


void _auth_adaptor_send_cmd_add_fd(auth_adaptor_h adaptor, int fd)
{
	char cmd_buf[256] = {0, };
	snprintf(cmd_buf, 255, "%s%d", PLUGIN_MESSAGE_LISTENER_CMD_APPEND_FD, fd);
	int len = strlen(cmd_buf);
	int wr_ret;

	g_mutex_lock(&adaptor->rd_mutex);
	wr_ret = write(adaptor->rd_cmd[1], &len, sizeof(int));
	wr_ret = write(adaptor->rd_cmd[1], cmd_buf, sizeof(char) * len);
	g_mutex_unlock(&adaptor->rd_mutex);
	auth_adaptor_debug("writed (%d)(%s)", wr_ret, cmd_buf);
}

void _auth_adaptor_send_cmd_stop_listen(auth_adaptor_h adaptor)
{
	char cmd_buf[256] = {0, };
	snprintf(cmd_buf, 255, "%s", PLUGIN_MESSAGE_LISTENER_CMD_STOP);
	int len = strlen(cmd_buf);
	int wr_ret;

	g_mutex_lock(&adaptor->rd_mutex);
	wr_ret = write(adaptor->rd_cmd[1], &len, sizeof(int));
	wr_ret = write(adaptor->rd_cmd[1], cmd_buf, sizeof(char) * len);
	g_mutex_unlock(&adaptor->rd_mutex);
	auth_adaptor_debug("writed (%d)(%s)", wr_ret, cmd_buf);
}

static int auth_adaptor_send_message_to_plugin_sync(auth_adaptor_plugin_h plugin,
						plugin_message_h send_message,
						plugin_message_h *receive_message)
{
	int io_ret = 0;
	int wfd = plugin->wd;
	int sync_hook[2];

	if (pipe(sync_hook) != -1) {
		char read_buf[PLUGIN_MESSAGE_PROTOCOL_MAX_BUF_SIZE] = {0, };

		plugin_message_set_value_number(send_message, PLUGIN_MESSAGE_ELEMENT_REQUEST_ID, (pmnumber) sync_hook[1]);
		char *stream = NULL;
		io_ret = plugin_message_serialize(send_message, &stream);
		int len = strlen(stream);

		g_mutex_lock(&plugin->message_mutex);
		io_ret = write(wfd, &len, sizeof(len));
		io_ret = write(wfd, stream, sizeof(char) * len);
		g_mutex_unlock(&plugin->message_mutex);
		free(stream);
		stream = NULL;
		len = 0;

		io_ret = read(sync_hook[0], &len, sizeof(len));

		if (0 < len) {
			memset(read_buf, 0, PLUGIN_MESSAGE_PROTOCOL_MAX_BUF_SIZE);
			len %= PLUGIN_MESSAGE_PROTOCOL_MAX_BUF_SIZE - 1;
			io_ret = read(sync_hook[0], read_buf, len);
		}
		auth_adaptor_debug("io ret : %d", io_ret);
		close(sync_hook[0]);
		close(sync_hook[1]);

		plugin_message_h _rcv;
		if (0 < strlen(read_buf)) {
			io_ret = plugin_message_deserialize(read_buf, &_rcv);
			if (!io_ret) {
				*receive_message = _rcv;
			}
		}
	} else {
		io_ret = -1;
	}

	return io_ret;
}

auth_adaptor_plugin_handle_h __auth_adaptor_create_3rd_party_plugin_handle(const char *plugin_uri)
{
	auth_adaptor_plugin_handle_h handle = (auth_adaptor_plugin_handle_h) calloc(1, sizeof(auth_adaptor_plugin_handle_t));

	if (NULL != handle) {
		handle->create_context = auth_plugin_send_create_context;
		handle->destroy_context = auth_plugin_send_destroy_context;
		handle->is_auth = auth_plugin_send_is_auth;
		handle->login = auth_plugin_send_login;
		handle->refresh_access_token = auth_plugin_send_refresh_access_token;

		handle->set_service_status = auth_plugin_send_set_service_status;
		handle->get_msisdn = auth_plugin_send_get_msisdn;
		handle->get_service_status = auth_plugin_send_get_service_status;
		handle->get_service_policy = auth_plugin_send_get_service_policy;
		handle->get_server_info = auth_plugin_send_get_server_info;


		handle->plugin_uri = strdup(plugin_uri);
	}

	return handle;
}

auth_error_code_t auth_plugin_send_create_context(auth_adaptor_plugin_context_h *context,
							const char *user_id,
							const char *user_password,
							const char *app_id,
							const char *app_secret,
							const char *imsi)
{
	auth_adaptor_plugin_h plugin = NULL;
	plugin = (*context)->plugin_handle;

	int ret = 0;
	plugin_message_h message = NULL;
	ret = plugin_message_create(&message);

	if (ret == 0) {
		(*context)->context_id = (int) (intptr_t)(*context);

		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_CONTEXT_ID,
				(pmnumber) (*context)->context_id);
		plugin_message_set_value_string(message, PLUGIN_MESSAGE_ELEMENT_FUNCTION_NAME,
				AUTH_PLUGIN_INTERFACE_CREATE_CONTEXT);

		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_MESSAGE_TYPE,
				(pmnumber) PLUGIN_MESSAGE_TYPE_FUNCTION);

		int param_index = 1;
		plugin_message_set_param_string(message, param_index++, user_id);
		plugin_message_set_param_string(message, param_index++, user_password);
		plugin_message_set_param_string(message, param_index++, app_id);
		plugin_message_set_param_string(message, param_index++, app_secret);
		plugin_message_set_param_string(message, param_index++, imsi);

		plugin_message_h result_message = NULL;
		ret = auth_adaptor_send_message_to_plugin_sync(plugin, message, &result_message);

		if (0 == ret) {
			pmnumber ret_code;
			plugin_message_get_value_number(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, &ret_code);

			ret = (int) ret_code;

			char *ret_msg = NULL;
			if (AUTH_ADAPTOR_ERROR_NONE == ret) {
				auth_adaptor_debug("Create context successed");
			} else {
				plugin_message_get_value_string(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE, &ret_msg);
				auth_adaptor_debug("Create context failed (%d)(%s)", ret, ret_msg);
				free(ret_msg);
				ret_msg = NULL;

				free(*context);
				(*context) = NULL;
			}
			plugin_message_destroy(result_message);
		}
	} else {
		ret = AUTH_ADAPTOR_ERROR_PLUGIN_INTERNAL;
	}
	plugin_message_destroy(message);

	return ret;


}

auth_error_code_t auth_plugin_send_destroy_context(auth_adaptor_plugin_context_h context)
{
	auth_adaptor_plugin_h plugin = NULL;
	plugin = context->plugin_handle;

	int ret = 0;
	plugin_message_h message = NULL;
	ret = plugin_message_create(&message);

	if (ret == 0) {
		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_CONTEXT_ID,
				(pmnumber) context->context_id);
		plugin_message_set_value_string(message, PLUGIN_MESSAGE_ELEMENT_FUNCTION_NAME,
				AUTH_PLUGIN_INTERFACE_DESTROY_CONTEXT);

		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_MESSAGE_TYPE,
				(pmnumber) PLUGIN_MESSAGE_TYPE_FUNCTION);

		plugin_message_h result_message = NULL;
		ret = auth_adaptor_send_message_to_plugin_sync(plugin, message, &result_message);

		if (0 == ret) {
			pmnumber ret_code;
			plugin_message_get_value_number(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, &ret_code);

			ret = (int) ret_code;

			char *ret_msg = NULL;
			if (AUTH_ADAPTOR_ERROR_NONE == ret) {
				auth_adaptor_debug("Destroy context successed");
			} else {
				plugin_message_get_value_string(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE, &ret_msg);
				auth_adaptor_debug("Destroy context failed (%d)(%s)", ret, ret_msg);
				free(ret_msg);
				ret_msg = NULL;

				auth_adaptor_debug("Force release memory by adaptor process");
				free(context->access_token);
				free(context);
				context = NULL;
			}
			plugin_message_destroy(result_message);
		}
	} else {
		ret = AUTH_ADAPTOR_ERROR_PLUGIN_INTERNAL;
	}
	plugin_message_destroy(message);

	return ret;


}




auth_error_code_t auth_plugin_send_is_auth(auth_adaptor_plugin_context_h context,
							void *request,
							int *is_auth,
							auth_adaptor_error_code_h *error,
							void *response)
{
	auth_adaptor_plugin_h plugin = NULL;
	plugin = context->plugin_handle;

	int ret = 0;
	plugin_message_h message = NULL;
	ret = plugin_message_create(&message);

	if (ret == 0) {
		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_CONTEXT_ID, (pmnumber) context->context_id);
		plugin_message_set_value_string(message, PLUGIN_MESSAGE_ELEMENT_FUNCTION_NAME, AUTH_PLUGIN_INTERFACE_IS_AUTH);

		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_MESSAGE_TYPE, (pmnumber) PLUGIN_MESSAGE_TYPE_FUNCTION);

		plugin_message_h result_message = NULL;
		ret = auth_adaptor_send_message_to_plugin_sync(plugin, message, &result_message);

		if (0 == ret) {
			pmnumber _is_auth, ret_code;
			plugin_message_get_value_number(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, &ret_code);

			ret = (int) ret_code;

			char *ret_msg = NULL;
			if (AUTH_ADAPTOR_ERROR_NONE == ret) {
				auth_adaptor_debug("Is_auth successed");
				plugin_message_get_param_number(result_message, 1, &_is_auth);
				*is_auth = (int)_is_auth;
			} else {
				plugin_message_get_value_string(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE, &ret_msg);
				auth_adaptor_debug("Is_auth failed (%d)(%s)", ret, ret_msg);
				if (NULL != error) {
					auth_adaptor_error_code_h error_code = auth_adaptor_create_error_code(ret, ret_msg);
					*error = error_code;
				}
				free(ret_msg);
				ret_msg = NULL;
			}

			plugin_message_destroy(result_message);
		}
	} else {
		ret = AUTH_ADAPTOR_ERROR_PLUGIN_INTERNAL;
	}
	plugin_message_destroy(message);

	return ret;
}

auth_error_code_t auth_plugin_send_join(auth_adaptor_plugin_context_h context,
							const char *device_id,
							void *request,
							auth_adaptor_error_code_h *error,
							void *response)
{
	auth_adaptor_plugin_h plugin = NULL;
	plugin = context->plugin_handle;

	int ret = 0;
	plugin_message_h message = NULL;
	ret = plugin_message_create(&message);

	if (ret == 0) {
		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_CONTEXT_ID, (pmnumber) context->context_id);
		plugin_message_set_value_string(message, PLUGIN_MESSAGE_ELEMENT_FUNCTION_NAME, AUTH_PLUGIN_INTERFACE_JOIN);

		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_MESSAGE_TYPE, (pmnumber) PLUGIN_MESSAGE_TYPE_FUNCTION);

		plugin_message_h result_message = NULL;
		ret = auth_adaptor_send_message_to_plugin_sync(plugin, message, &result_message);

		if (0 == ret) {
			pmnumber ret_code;
			plugin_message_get_value_number(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, &ret_code);

			ret = (int) ret_code;

			char *ret_msg = NULL;
			if (AUTH_ADAPTOR_ERROR_NONE == ret) {
				auth_adaptor_debug("Join successed");
			} else {
				plugin_message_get_value_string(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE, &ret_msg);
				auth_adaptor_debug("Join failed (%d)(%s)", ret, ret_msg);
				if (NULL != error) {
					auth_adaptor_error_code_h error_code = auth_adaptor_create_error_code(ret, ret_msg);
					*error = error_code;
				}
				free(ret_msg);
				ret_msg = NULL;
			}

			plugin_message_destroy(result_message);
		}
	} else {
		ret = AUTH_ADAPTOR_ERROR_PLUGIN_INTERNAL;
	}
	plugin_message_destroy(message);

	return ret;
}



auth_error_code_t auth_plugin_send_login(auth_adaptor_plugin_context_h context,
							void *request,
							auth_adaptor_error_code_h *error,
							void *response)
{
	auth_adaptor_plugin_h plugin = NULL;
	plugin = context->plugin_handle;

	int ret = 0;
	plugin_message_h message = NULL;
	ret = plugin_message_create(&message);

	if (ret == 0) {
		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_CONTEXT_ID,
				(pmnumber) context->context_id);
		plugin_message_set_value_string(message, PLUGIN_MESSAGE_ELEMENT_FUNCTION_NAME,
				AUTH_PLUGIN_INTERFACE_LOGIN);

		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_MESSAGE_TYPE,
				(pmnumber) PLUGIN_MESSAGE_TYPE_FUNCTION);

		plugin_message_h result_message = NULL;
		ret = auth_adaptor_send_message_to_plugin_sync(plugin, message, &result_message);

		if (0 == ret) {
			pmnumber ret_code;
			plugin_message_get_value_number(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, &ret_code);

			ret = (int) ret_code;
			char *ret_msg = NULL;
			if (AUTH_ADAPTOR_ERROR_NONE == ret) {
				auth_adaptor_debug("Login successed");
				char *access_token = NULL;
				char *uid = NULL;
				int param_idx = 1;
				plugin_message_get_param_string(result_message, param_idx++, &access_token);
				plugin_message_get_param_string(result_message, param_idx++, &uid);
				auth_adaptor_debug("access token : %s", access_token);
				auth_adaptor_debug("uid : %s", uid);

				context->access_token = access_token;
				context->uid = uid;
			} else {
				plugin_message_get_value_string(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE, &ret_msg);
				auth_adaptor_debug("Login failed (%d)(%s)", ret, ret_msg);
				if (NULL != error) {
					auth_adaptor_error_code_h error_code = auth_adaptor_create_error_code(ret, ret_msg);
					*error = error_code;
				}
				free(ret_msg);
				ret_msg = NULL;
			}
			plugin_message_destroy(result_message);
		}
	} else {
		ret = AUTH_ADAPTOR_ERROR_PLUGIN_INTERNAL;
	}
	plugin_message_destroy(message);

	return ret;

}

auth_error_code_t auth_plugin_send_refresh_access_token(auth_adaptor_plugin_context_h context,
							void *request,
							auth_adaptor_error_code_h *error,
							void *response)
{
	auth_adaptor_plugin_h plugin = NULL;
	plugin = context->plugin_handle;

	int ret = 0;
	plugin_message_h message = NULL;
	ret = plugin_message_create(&message);

	if (ret == 0) {
		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_CONTEXT_ID,
				(pmnumber) context->context_id);
		plugin_message_set_value_string(message, PLUGIN_MESSAGE_ELEMENT_FUNCTION_NAME,
				AUTH_PLUGIN_INTERFACE_REFRESH_ACCESS_TOKEN);

		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_MESSAGE_TYPE,
				(pmnumber) PLUGIN_MESSAGE_TYPE_FUNCTION);

		plugin_message_h result_message = NULL;
		ret = auth_adaptor_send_message_to_plugin_sync(plugin, message, &result_message);

		if (0 == ret) {
			pmnumber ret_code;
			plugin_message_get_value_number(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, &ret_code);

			ret = (int) ret_code;
			char *ret_msg = NULL;
			if (AUTH_ADAPTOR_ERROR_NONE == ret) {
				auth_adaptor_debug("Login refresh successed");
				char *access_token = NULL;
				char *uid = NULL;
				int param_idx = 1;
				plugin_message_get_param_string(result_message, param_idx++, &access_token);
				plugin_message_get_param_string(result_message, param_idx++, &uid);
				auth_adaptor_debug("access token : %s", access_token);
				auth_adaptor_debug("uid : %s", uid);

				context->access_token = access_token;
				context->uid = uid;
			} else {
				plugin_message_get_value_string(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE, &ret_msg);
				auth_adaptor_debug("Login refresh failed (%d)(%s)", ret, ret_msg);
				if (NULL != error) {
					auth_adaptor_error_code_h error_code = auth_adaptor_create_error_code(ret, ret_msg);
					*error = error_code;
				}
				free(ret_msg);
				ret_msg = NULL;
			}
			plugin_message_destroy(result_message);
		}
	} else {
		ret = AUTH_ADAPTOR_ERROR_PLUGIN_INTERNAL;
	}
	plugin_message_destroy(message);

	return ret;

}
/*
auth_error_code_t auth_plugin_send_get_server_info(auth_adaptor_plugin_context_h context,
		void *request,
		GHashTable **server_info,
		auth_adaptor_error_code_h *error,
		void *response)

{
	auth_adaptor_plugin_h plugin = NULL;
	plugin = context->plugin_handle;

	int ret = 0;
	plugin_message_h message = NULL;
	ret = plugin_message_create(&message);

	if (ret == 0)
	{
		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_CONTEXT_ID,
				(pmnumber) context->context_id);
		plugin_message_set_value_string(message, PLUGIN_MESSAGE_ELEMENT_FUNCTION_NAME,
				AUTH_PLUGIN_INTERFACE_LOGIN);

		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_MESSAGE_TYPE,
				(pmnumber) PLUGIN_MESSAGE_TYPE_FUNCTION);

		plugin_message_h result_message = NULL;
		ret = auth_adaptor_send_message_to_plugin_sync(plugin, message, &result_message);

		if (0 == ret)
		{
			pmnumber ret_code;
			plugin_message_get_value_number(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, &ret_code);

			ret = (int) ret_code;
			char *ret_msg = NULL;
			if (AUTH_ADAPTOR_ERROR_NONE == ret)
			{
				auth_adaptor_debug("Get server info successed");
				char *server_info_raw = NULL;
				int param_idx = 1;
				plugin_message_get_param_string(result_message, param_idx++, &server_info_raw);

				if ((NULL != server_info) && (NULL != server_info_raw))
				{
					bundle *info_data = bundle_decode((bundle_raw)server_info_raw, strlen(server_info_raw));
					if (NULL != info_data)
					{
						GHashTable *ht = g_hash_table_new_full(g_str_hash, g_str_equal, free, free);
						bundle_foreach(info_data, server_info_iter, (void *) ht);
						*server_info = ht;
					}
				}

				free(server_info_raw);
			}
			else
			{
				plugin_message_get_value_string(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE, &ret_msg);
				auth_adaptor_debug("Login failed (%d)(%s)", ret, ret_msg);
				if (NULL != error)
				{
					auth_adaptor_error_code_h error_code = auth_adaptor_create_error_code(ret, ret_msg);
					*error = error_code;
				}
				free(ret_msg);
				ret_msg = NULL;
			}
			plugin_message_destroy(result_message);
		}
	}
	else
	{
		ret = AUTH_ADAPTOR_ERROR_PLUGIN_INTERNAL;
	}
	plugin_message_destroy(message);

	return ret;

}
*/


auth_error_code_t auth_plugin_send_set_service_status(auth_adaptor_plugin_context_h context,
							const int service_id,
							const int status,
							void *request,
							auth_adaptor_error_code_h *error,
							void *response)
{
	return AUTH_ADAPTOR_ERROR_UNSUPPORTED;
}

auth_error_code_t auth_plugin_send_get_msisdn(auth_adaptor_plugin_context_h context,
							void *request,
							char **msisdn,
							auth_adaptor_error_code_h *error,
							void *response)
{
	return AUTH_ADAPTOR_ERROR_UNSUPPORTED;
}

auth_error_code_t auth_plugin_send_get_service_status(auth_adaptor_plugin_context_h context,
		const int service_id,
		void *request,
		int *status,
		auth_adaptor_error_code_h *error,
		void *response)
{
	return AUTH_ADAPTOR_ERROR_UNSUPPORTED;
}

auth_error_code_t auth_plugin_send_get_service_policy(auth_adaptor_plugin_context_h context,
		const int service_id,
		void *request,
		char **default_status,
		char **policy_feature,
		char **policy_version,
		char **policy_doc_url,
		auth_adaptor_error_code_h *error,
		void *response)
{
	return AUTH_ADAPTOR_ERROR_UNSUPPORTED;
}

auth_error_code_t auth_plugin_send_get_server_info(auth_adaptor_plugin_context_h context,
		void *request,
		GHashTable **server_info,
		auth_adaptor_error_code_h *error,
		void *response)
{
	return AUTH_ADAPTOR_ERROR_UNSUPPORTED;
}


/* For forked plugin */
void *_auth_plugin_request_collector(void *data)
{
	auth_adaptor_plugin_h plugin = (auth_adaptor_plugin_h) data;

	int rcv_len, buf_size;
	char msg_buf[PLUGIN_MESSAGE_PROTOCOL_MAX_BUF_SIZE] = {0, };

	while (1) {
		/* pre-read buf size */
		rcv_len = read(plugin->rd, &buf_size, sizeof(int));

		if (rcv_len <= 0) {
			LOGD("shutdown by adaptor disconnected");
			return NULL;
		}

		/* allocates and read buf data */
		memset(msg_buf, 0, PLUGIN_MESSAGE_PROTOCOL_MAX_BUF_SIZE);
		buf_size %= PLUGIN_MESSAGE_PROTOCOL_MAX_BUF_SIZE - 1;
		rcv_len = read(plugin->rd, msg_buf, buf_size);
		LOGD("read message [%s][len: %d]", msg_buf, rcv_len);

		if (rcv_len <= 0) {
			LOGD("shutdown by adaptor disconnected");
			return NULL;
		}

		char *result = NULL;
		__auth_plugin_progress_command(plugin, msg_buf, &result);

		if (NULL != result) {
			int res_len = strlen(result);
			rcv_len = write(plugin->wd, &res_len, sizeof(int));
			rcv_len = write(plugin->wd, result, sizeof(char) * res_len);
		}
		/* transfer data to adaptor */
		/*		__auth_adaptor_transfer_message(adaptor, msg_buf); */
	}
	return data;
}

auth_adaptor_plugin_context_h __auth_plugin_get_context_by_context_id(auth_adaptor_plugin_h plugin, int context_id)
{
	if (NULL == plugin) {
		return NULL;
	}

	/* For forked plugin */
	auth_adaptor_plugin_context_h ctx = NULL;
	int i, len;
	len = g_list_length(plugin->contexts);

	for (i = 0; i < len; i++) {
		ctx = (auth_adaptor_plugin_context_h) g_list_nth_data(plugin->contexts, i);

		if (context_id == ctx->context_id) {
			return ctx;
		}
	}
	return NULL;
}

void __auth_plugin_progress_command(auth_adaptor_plugin_h plugin, char *order, char **result)
{
	int ret = 0;
	plugin_message_h m_order = NULL;
	plugin_message_h m_result = NULL;

	if ((NULL == order) || (plugin_message_deserialize(order, &m_order))) {
		LOGE("[%s/%d] Message parse error", __FUNCTION__, __LINE__);
		return;
	} else if (plugin_message_create(&m_result)) {
		plugin_message_destroy(m_order);
		m_order = NULL;

		LOGE("[%s/%d] Message parse error", __FUNCTION__, __LINE__);
		return;
	}

	char *func_name = NULL;
	int context_id = 0;

	pmnumber ctx, req, type;

	plugin_message_get_value_number(m_order, PLUGIN_MESSAGE_ELEMENT_CONTEXT_ID, &ctx);
	plugin_message_set_value_number(m_result, PLUGIN_MESSAGE_ELEMENT_CONTEXT_ID, ctx);
	plugin_message_get_value_number(m_order, PLUGIN_MESSAGE_ELEMENT_REQUEST_ID, &req);
	plugin_message_set_value_number(m_result, PLUGIN_MESSAGE_ELEMENT_REQUEST_ID, req);
	plugin_message_get_value_string(m_order, PLUGIN_MESSAGE_ELEMENT_FUNCTION_NAME, &func_name);
	plugin_message_set_value_string(m_result, PLUGIN_MESSAGE_ELEMENT_FUNCTION_NAME, func_name);
	plugin_message_get_value_number(m_result, PLUGIN_MESSAGE_ELEMENT_MESSAGE_TYPE, &type);
	plugin_message_set_value_number(m_result, PLUGIN_MESSAGE_ELEMENT_MESSAGE_TYPE, type);
	plugin_message_set_value_number(m_result, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, (pmnumber)AUTH_ADAPTOR_ERROR_NONE);
	context_id = (int) ctx;
	auth_adaptor_plugin_context_h context = __auth_plugin_get_context_by_context_id(plugin, context_id);

	auth_adaptor_error_code_h error_code = NULL;

	if (0 == strncmp(AUTH_PLUGIN_INTERFACE_CREATE_CONTEXT,
			func_name, strlen(AUTH_PLUGIN_INTERFACE_CREATE_CONTEXT))) {
		LOGD(">>>>>> %s func start", func_name);
		char *user_id = NULL;
		char *user_password = NULL;
		char *app_id = NULL;
		char *app_secret = NULL;
		char *service_name = NULL;
		char *imsi = NULL;

		int param_idx = 1;
		plugin_message_get_param_string(m_order, param_idx++, &user_id);
		plugin_message_get_param_string(m_order, param_idx++, &user_password);
		plugin_message_get_param_string(m_order, param_idx++, &app_id);
		plugin_message_get_param_string(m_order, param_idx++, &app_secret);
		plugin_message_get_param_string(m_order, param_idx++, &imsi);

		LOGD("Call library function");
		context = auth_adaptor_create_plugin_context(plugin,
				user_id, user_password, app_id, app_secret, imsi, "");
		if (NULL == context) {
			LOGE("[%s<%s>/%d] Could not create context", __FUNCTION__, func_name, __LINE__);
			plugin_message_set_value_number(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_CODE,
					(pmnumber)AUTH_ADAPTOR_ERROR_PLUGIN_INTERNAL);
			plugin_message_set_value_string(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE,
					"Could not create context");
		} else {
			LOGD("[%s<%s>/%d] Created context successfuly", __FUNCTION__, func_name, __LINE__);
			context->context_id = context_id;

			plugin_message_set_value_number(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_CODE,
					(pmnumber)AUTH_ADAPTOR_ERROR_NONE);
		}

		free(user_id);
		free(user_password);
		free(app_id);
		free(app_secret);
		free(service_name);
		LOGD("<<<<<< %s func end", func_name);
	} else if (0 == strncmp(AUTH_PLUGIN_INTERFACE_DESTROY_CONTEXT,
			func_name, strlen(AUTH_PLUGIN_INTERFACE_DESTROY_CONTEXT))) {
		LOGD(">>>>>> %s func start", func_name);
		if (NULL == context) {
			LOGE("[%s<%s>/%d] Could not found context", __FUNCTION__, func_name, __LINE__);
			plugin_message_set_value_number(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_CODE,
					(pmnumber)AUTH_ADAPTOR_ERROR_INVALID_ARGUMENT);
			plugin_message_set_value_string(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE,
					"Invalid argument (context is NULL)");
		} else {
			LOGD("[%s<%s>/%d] function success", __FUNCTION__, func_name, __LINE__);
			auth_adaptor_destroy_plugin_context(plugin, context);

			plugin_message_set_value_number(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_CODE,
					(pmnumber)AUTH_ADAPTOR_ERROR_NONE);
		}
		LOGD("<<<<<< %s func end", func_name);
	} else if (0 == strncmp(AUTH_PLUGIN_INTERFACE_IS_AUTH,
			func_name, strlen(AUTH_PLUGIN_INTERFACE_IS_AUTH))) {
		LOGD(">>>>>> %s func start", func_name);
		int is_auth = 0;
		LOGD("Call library function");
		if (NULL == context) {
			LOGE("[%s<%s>/%d] Could not found context", __FUNCTION__, func_name, __LINE__);
			plugin_message_set_value_number(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_CODE,
					(pmnumber)AUTH_ADAPTOR_ERROR_INVALID_ARGUMENT);
			plugin_message_set_value_string(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE,
					"Invalid argument (context is NULL)");
		} else {
			ret = plugin->handle->is_auth(context, NULL, &is_auth, &error_code, NULL);
			if (AUTH_ADAPTOR_ERROR_NONE == ret) {
				int param_idx = 1;
				plugin_message_set_param_number(m_result, param_idx++, (pmnumber)is_auth);
				plugin_message_set_value_number(m_result, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, (pmnumber)ret);
			} else if (NULL != error_code) {
				plugin_message_set_value_number(m_result,
						PLUGIN_MESSAGE_ELEMENT_RESULT_CODE,
						(pmnumber)error_code->code);
				plugin_message_set_value_string(m_result,
						PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE,
						error_code->msg ? error_code->msg : "");
				free(error_code->msg);
				free(error_code);
			} else {
				plugin_message_set_value_number(m_result, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, (pmnumber)ret);
			}
		}
		LOGD("<<<<<< %s func end", func_name);
	} else if (0 == strncmp(AUTH_PLUGIN_INTERFACE_LOGIN,
			func_name, strlen(AUTH_PLUGIN_INTERFACE_LOGIN))) {
		LOGD(">>>>>> %s func start", func_name);
		LOGD("Call library function");
		if (NULL == context) {
			LOGE("[%s<%s>/%d] Could not found context", __FUNCTION__, func_name, __LINE__);
			plugin_message_set_value_number(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_CODE,
					(pmnumber)AUTH_ADAPTOR_ERROR_INVALID_ARGUMENT);
			plugin_message_set_value_string(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE,
					"Invalid argument (context is NULL)");
		} else {
			ret = plugin->handle->login(context, NULL, &error_code, NULL);
			if (AUTH_ADAPTOR_ERROR_NONE == ret) {
				int param_idx = 1;
				plugin_message_set_param_string(m_result, param_idx++, context->access_token ? context->access_token : "");
				plugin_message_set_param_string(m_result, param_idx++, context->uid ? context->uid : "");
				plugin_message_set_value_number(m_result, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, (pmnumber)ret);
			} else if (NULL != error_code) {
				plugin_message_set_value_number(m_result,
						PLUGIN_MESSAGE_ELEMENT_RESULT_CODE,
						(pmnumber)error_code->code);
				plugin_message_set_value_string(m_result,
						PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE,
						error_code->msg ? error_code->msg : "");
				free(error_code->msg);
				free(error_code);
			} else {
				plugin_message_set_value_number(m_result, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, (pmnumber)ret);
			}
		}
		LOGD("<<<<<< %s func end", func_name);
	} else if (0 == strncmp(AUTH_PLUGIN_INTERFACE_REFRESH_ACCESS_TOKEN,
			func_name, strlen(AUTH_PLUGIN_INTERFACE_REFRESH_ACCESS_TOKEN))) {
		LOGD(">>>>>> %s func start", func_name);
		LOGD("Call library function");
		if (NULL == context) {
			LOGE("[%s<%s>/%d] Could not found context", __FUNCTION__, func_name, __LINE__);
			plugin_message_set_value_number(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_CODE,
					(pmnumber)AUTH_ADAPTOR_ERROR_INVALID_ARGUMENT);
			plugin_message_set_value_string(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE,
					"Invalid argument (context is NULL)");
		} else {
			ret = plugin->handle->refresh_access_token(context, NULL, &error_code, NULL);
			if (AUTH_ADAPTOR_ERROR_NONE == ret) {
				int param_idx = 1;
				plugin_message_set_param_string(m_result, param_idx++, context->access_token ? context->access_token : "");
				plugin_message_set_param_string(m_result, param_idx++, context->uid ? context->uid : "");
				plugin_message_set_value_number(m_result, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, (pmnumber)ret);
			} else if (NULL != error_code) {
				plugin_message_set_value_number(m_result,
						PLUGIN_MESSAGE_ELEMENT_RESULT_CODE,
						(pmnumber)error_code->code);
				plugin_message_set_value_string(m_result,
						PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE,
						error_code->msg ? error_code->msg : "");
				free(error_code->msg);
				free(error_code);
			} else {
				plugin_message_set_value_number(m_result, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, (pmnumber)ret);
			}
		}
		LOGD("<<<<<< %s func end", func_name);
	} else {
		plugin_message_set_value_number(m_result,
				PLUGIN_MESSAGE_ELEMENT_RESULT_CODE,
				(pmnumber)AUTH_ADAPTOR_ERROR_UNSUPPORTED);
		plugin_message_set_value_string(m_result,
				PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE,
				"Unsupported operation");
	}

	free(func_name);

	char *result_data = NULL;
	plugin_message_serialize(m_result, &result_data);
	plugin_message_destroy(m_result);
	plugin_message_destroy(m_order);

	*result = result_data;
}


