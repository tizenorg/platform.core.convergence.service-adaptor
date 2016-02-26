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
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <plugin_message.h>

#include "storage-adaptor.h"
#include "storage-adaptor-log.h"

#define PLUGIN_MESSAGE_PROTOCOL_MAX_BUF_SIZE		40960
#define PLUGIN_MESSAGE_LISTENER_CMD_APPEND_FD		"append;"
#define PLUGIN_MESSAGE_LISTENER_CMD_STOP		"stop;"

#define STORAGE_PLUGIN_INTERFACE_CREATE_CONTEXT		"create_context"
#define STORAGE_PLUGIN_INTERFACE_DESTROY_CONTEXT	"destroy_context"
#define STORAGE_PLUGIN_INTERFACE_MAKE_DIRECTORY		"make_directory"
#define STORAGE_PLUGIN_INTERFACE_GET_LIST		"get_list"
#define STORAGE_PLUGIN_INTERFACE_REMOVE_DIRECTORY	"remove_directory"
#define STORAGE_PLUGIN_INTERFACE_UPLOAD_FILE_SYNC	"upload_file_sync"
#define STORAGE_PLUGIN_INTERFACE_DOWNLOAD_FILE_SYNC	"download_file_sync"
#define STORAGE_PLUGIN_INTERFACE_DELETE_FILE		"delete_file"
#define STORAGE_PLUGIN_INTERFACE_MOVE_DIRECTORY		"move_directory"
#define STORAGE_PLUGIN_INTERFACE_MOVE_FILE		"move_file"
#define STORAGE_PLUGIN_INTERFACE_SET_TRANSFER_STATE	"set_transfer_state"
#define STORAGE_PLUGIN_INTERFACE_GET_TRANSFER_STATE	"get_transfer_state"
#define STORAGE_PLUGIN_INTERFACE_GET_ROOT_FOLDER_PATH	"get_root_folder_path"

/* for 2.4 public functions */
#define STORAGE_PLUGIN_INTERFACE_START_UPLOAD_TASK	"start_upload_task"
#define STORAGE_PLUGIN_INTERFACE_START_DOWNLOAD_TASK	"start_download_task"
#define STORAGE_PLUGIN_INTERFACE_START_DOWNLOAD_THUMB_TASK	"start_download_thumb_task"
#define STORAGE_PLUGIN_INTERFACE_CANCEL_UPLOAD_TASK	"cancel_upload_task"
#define STORAGE_PLUGIN_INTERFACE_CANCEL_DOWNLOAD_TASK	"cancel_download_task"
#define STORAGE_PLUGIN_INTERFACE_CANCEL_DOWNLOAD_THUMB_TASK	"cancel_download_thumb_task"

#define STORAGE_PLUGIN_CALLBACK_DOWNLOAD_FILE_ASYNC_CB	"download_async_cb"
#define STORAGE_PLUGIN_CALLBACK_UPLOAD_FILE_ASYNC_CB	"upload_async_cb"
#define STORAGE_PLUGIN_CALLBACK_PROGRESS_CB		"progress_cb"

#define IF_IS_PLUGIN_THAN_RETURN_NULL()		do {if (!g_process_identity) return NULL; } while (0)
#define SAFE_ADD_STRING(x)			(x) ? (x) : ("")

typedef enum {
	PLUGIN_TYPE_INHOUSE	= 0,
	PLUGIN_TYPE_3RD_PARTY	= 1,
} storage_plugin_type_e;


#ifndef FORK_PLUGIN_ARCHITECTURE
GHashTable	*g_file_uid_list = NULL;
#endif

/**
 * Storage adaptor plugin
 */
typedef struct storage_adaptor_plugin_s {
	storage_adaptor_h			adaptor;		/* Adaptor */
	char					*path;			/* Plugin library path */
	storage_adaptor_plugin_handle_h		handle;			/* Plugin handle */
	void					*dl_handle;		/* Plugin library handle */
	int					ref_counter;		/* Plugin reference counter */
	GMutex					ref_counter_mutex;	/* Plugin reference counter mutex */
	storage_adaptor_plugin_listener_h	plugin_listener;	/* Plugin callback listener */
	GMutex					plugin_listener_mutex;	/* Plugin callback listener mutex */

	GMutex					message_mutex;
	storage_plugin_type_e			type;
	int					pid;
	int					rd;
	int					wd;
	GList					*contexts;
	GMutex					contexts_mutex;
} storage_adaptor_plugin_t;

/**
 * Storage adaptor
 */
typedef struct storage_adaptor_s {
	GMutex	storage_adaptor_mutex;		/* Adaptor mutex */
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
} storage_adaptor_t;

static int g_process_identity = -1;

static storage_adaptor_plugin_h g_child_plugin = NULL;

/**
 * Creates plugin
 */
static storage_adaptor_plugin_h storage_adaptor_create_plugin(const char *plugin_path);

/**
 * Destroys plugin and deletes all resources associated with it
 */
static void storage_adaptor_destroy_plugin(storage_adaptor_plugin_h plugin);

/**
 * Loads plugins from selected directory
 */
static int storage_adaptor_load_plugins_from_directory(storage_adaptor_h adaptor,
						const char *dir_path);

/**
 * Checks if plugin is loaded by selected plugin adaptor
 */
static int storage_adaptor_has_plugin(storage_adaptor_h adaptor,
						storage_adaptor_plugin_h plugin);

/*	TDB Temp */

#define GET_PLUGIN_PID() getpid()

int _get_plugin_fd_from_file_uid(long long int file_uid)
{
	return ((int) (0xffff & file_uid));
}

long long int _get_file_uid_from_plugin_fd(int plugin_fd)
{
	long long int plugin_section = 0LL;
	#ifdef FORK_PLUGIN_ARCHITECTURE
		/* TODO it must be changed to another index (not support 64bit) */
		plugin_section = ((long long int) GET_PLUGIN_PID()) << (sizeof(int)*8);
	#endif

	return (plugin_section | (long long int)plugin_fd);
}

/**
 * Increases adaptor's plugin references counter
 */
void storage_adaptor_plugin_ref(storage_adaptor_plugin_h);

/**
 * Decreases adaptor's plugin references counter
 */
void storage_adaptor_plugin_unref(storage_adaptor_plugin_h);


/* ///////////////////////////////////////////////////////////////////////////////
   /////////////  Internal function prototype (for forked plugin)  ///////////////
   /////////////////////////////////////////////////////////////////////////////// */


/* To be used by adaptor */
void *_storage_adaptor_plugin_message_collector(void *data);
void __storage_adaptor_transfer_message(const char *msg);
int __storage_adaptor_parse_message_cmd(storage_adaptor_h adaptor, char *msg);
void _storage_adaptor_send_cmd_add_fd(storage_adaptor_h adaptor, int fd);
void _storage_adaptor_send_cmd_stop_listen(storage_adaptor_h adaptor);

static int storage_adaptor_send_message_to_plugin_sync(storage_adaptor_plugin_h plugin,
						plugin_message_h send_message,
						plugin_message_h *receive_message);

/* To be used by adaptor (virtual plugin handle) */
storage_adaptor_plugin_handle_h __storage_adaptor_create_3rd_party_plugin_handle(const char *plugin_uri);

storage_error_code_t storage_plugin_send_create_context(storage_adaptor_plugin_context_h *context,
							const char *app_id,
							const char *app_secret,
							const char *access_token,
							const char *cid,
							const char *uid);

storage_error_code_t storage_plugin_send_destroy_context(storage_adaptor_plugin_context_h context);

storage_error_code_t storage_plugin_send_set_server_info(storage_adaptor_plugin_context_h context,
							GHashTable *server_info,
							void *request,
							storage_adaptor_error_code_h *error,
							void *response);

storage_error_code_t storage_plugin_send_make_directory(storage_adaptor_plugin_context_h context,
							const char *parent_folder_storage_path,
							const char *folder_name,
							void *request,
							storage_adaptor_file_info_h *file_info,
							storage_adaptor_error_code_h *error,
							void *response);

storage_error_code_t storage_plugin_send_remove_directory(storage_adaptor_plugin_context_h context,
							const char *parent_folder_storage_path,
							const char *folder_name,
							void *request,
							storage_adaptor_file_info_h *file_info,
							storage_adaptor_error_code_h *error,
							void *response);

storage_error_code_t storage_plugin_send_get_list(storage_adaptor_plugin_context_h context,
							const char *parent_folder_storage_path,
							const char *folder_name,
							void *request,
							storage_adaptor_file_info_h **file_info_list,
							int *file_info_list_len,
							storage_adaptor_error_code_h *error,
							void *response);

storage_error_code_t storage_plugin_send_upload_file_sync(storage_adaptor_plugin_context_h context,
							const char *parent_folder_storage_path,
							const char *file_name,
							const char *upload_file_local_path,
							const int publish,
							void *request,
							storage_adaptor_file_info_h *file_info,
							storage_adaptor_error_code_h *error,
							void *response);

storage_error_code_t storage_plugin_send_download_file_sync(storage_adaptor_plugin_context_h context,
							const char *parent_folder_storage_path,
							const char *file_name,
							const char *download_file_local_path,
							void *request,
							storage_adaptor_error_code_h *error,
							void *response);

storage_error_code_t storage_plugin_send_delete_file(storage_adaptor_plugin_context_h context,
							const char *parent_folder_storage_path,
							const char *file_name,
							void *request,
							storage_adaptor_file_info_h *file_info,
							storage_adaptor_error_code_h *error,
							void *response);

storage_error_code_t storage_plugin_send_move_directory(storage_adaptor_plugin_context_h context,
							const char *parent_folder_storage_path,
							const char *folder_name,
							const char *dest_parent_folder_storage_path,
							const char *new_folder_name,
							void *request,
							storage_adaptor_file_info_h *file_info,
							storage_adaptor_error_code_h *error,
							void *response);

storage_error_code_t storage_plugin_send_move_file(storage_adaptor_plugin_context_h context,
							const char *parent_folder_storage_path,
							const char *file_name,
							const char *dest_parent_folder_storage_path,
							const char *new_file_name,
							void *request,
							storage_adaptor_file_info_h *file_info,
							storage_adaptor_error_code_h *error,
							void *response);

storage_error_code_t storage_plugin_send_set_transfer_state(storage_adaptor_plugin_context_h context,
							void *transfer_request_id,
							storage_adaptor_transfer_state_e state,
							void *request,
							storage_adaptor_error_code_h *error,
							void *response);

storage_error_code_t storage_plugin_send_get_transfer_state(storage_adaptor_plugin_context_h context,
							void *transfer_request_id,
							void *request,
							storage_adaptor_transfer_state_e *state,
							storage_adaptor_error_code_h *error,
							void *response);

storage_error_code_t storage_plugin_send_get_root_folder_path(storage_adaptor_plugin_context_h context,
							void *request,
							char **root_folder_path,
							storage_adaptor_error_code_h *error,
							void *response);

storage_error_code_t storage_plugin_send_start_upload_task(storage_adaptor_plugin_context_h context,
							int fd,
							const char *upload_dir,
							const char *file_path,
							bool need_progress,
							storage_adaptor_error_code_h *error,
							void *user_data);

storage_error_code_t storage_plugin_send_start_download_task(storage_adaptor_plugin_context_h context,
							const char *storage_dir,
							const char *file_path,
							int fd,
							bool need_progress,
							storage_adaptor_error_code_h *error,
							void *user_data);

storage_error_code_t storage_plugin_send_start_download_thumb_task(storage_adaptor_plugin_context_h context,
							const char *storage_dir,
							const char *file_path,
							int fd,
							int thumbnail_size,
							bool need_progress,
							storage_adaptor_error_code_h *error,
							void *user_data);

storage_error_code_t storage_plugin_send_cancel_upload_task(storage_adaptor_plugin_context_h context,
							int fd,
							storage_adaptor_error_code_h *error);

storage_error_code_t storage_plugin_send_cancel_download_task(storage_adaptor_plugin_context_h context,
							int fd,
							storage_adaptor_error_code_h *error);

storage_error_code_t storage_plugin_send_cancel_download_thumb_task(storage_adaptor_plugin_context_h context,
							int fd,
							storage_adaptor_error_code_h *error);

/* To be used by forked plugin */
void *_storage_plugin_request_collector(void *data);
storage_adaptor_plugin_context_h __storage_plugin_get_context_by_context_id(storage_adaptor_plugin_h plugin, int context_id);
void __storage_plugin_progress_command(storage_adaptor_plugin_h plugin, char *order, char **result);


storage_adaptor_file_info_h _get_file_info_from_message_array(plugin_message_array_h message_array, int index);

int _message_array_set_file_info(plugin_message_array_h message_array, int index, storage_adaptor_file_info_h file_info);

/**
 * Definition of callback function variables for vservice channel (= infra adaptor) (example)
 */
/* private feature */
storage_adaptor_service_download_file_async_reply_cb	_service_adaptor_download_file_async_reply	= NULL;
storage_adaptor_service_upload_file_async_reply_cb	_service_adaptor_upload_file_async_reply	= NULL;
storage_adaptor_service_file_transfer_progress_reply_cb	_service_adaptor_file_transfer_progress_reply	= NULL;

/* public feature */
storage_adaptor_service_download_state_changed_reply_cb	_service_adaptor_download_state_changed_reply	= NULL;
storage_adaptor_service_upload_state_changed_reply_cb	_service_adaptor_upload_state_changed_reply	= NULL;
storage_adaptor_service_task_progress_reply_cb	_service_adaptor_task_progress_reply	= NULL;

/**
 * Gets a message from plugin (callback) when a sms message is received (sample)
 */
/* private feature */
void storage_adaptor_download_file_async_reply_cb(void *request_id,
						char *download_file_local_path,
						storage_adaptor_error_code_h error,
						void *response)
{
	if (NULL != _service_adaptor_download_file_async_reply) {
		_service_adaptor_download_file_async_reply(request_id,
				download_file_local_path, error, response);
	}
}

void storage_adaptor_upload_file_async_reply_cb(void *request_id,
						storage_adaptor_file_info_h file_info,
						storage_adaptor_error_code_h error,
						void *response)
{
	if (NULL != _service_adaptor_upload_file_async_reply) {
		_service_adaptor_upload_file_async_reply(request_id,
				file_info, error, response);
	}
}

void storage_adaptor_file_transfer_progress_reply_cb(void *request_id,
						unsigned long long progress_size_byte,
						unsigned long long total_size_byte,
						storage_adaptor_error_code_h error,
						void *response)
{
	if (NULL != _service_adaptor_file_transfer_progress_reply) {
		_service_adaptor_file_transfer_progress_reply(request_id,
				progress_size_byte, total_size_byte, error, response);
	}
}

/* public feature */
void storage_adaptor_download_state_changed_reply_cb(int file_descriptor,
						storage_adaptor_transfer_state_e state,
						storage_adaptor_error_code_h error,
						void *user_data)
{
	if ((state == STORAGE_ADAPTOR_TRANSFER_STATE_FINISHED)
			|| (state == STORAGE_ADAPTOR_TRANSFER_STATE_CANCELED)
			|| (state == STORAGE_ADAPTOR_TRANSFER_STATE_FAILED)) {
		close(file_descriptor);
	}

	if (NULL != _service_adaptor_download_state_changed_reply) {
		long long int file_uid = _get_file_uid_from_plugin_fd(file_descriptor);
		_service_adaptor_download_state_changed_reply(file_uid,
				state, error, user_data);
	}
}

void storage_adaptor_upload_state_changed_reply_cb(int file_descriptor,
						storage_adaptor_transfer_state_e state,
						storage_adaptor_file_info_h file_info,
						storage_adaptor_error_code_h error,
						void *user_data)
{
	if ((state == STORAGE_ADAPTOR_TRANSFER_STATE_FINISHED)
			|| (state == STORAGE_ADAPTOR_TRANSFER_STATE_CANCELED)
			|| (state == STORAGE_ADAPTOR_TRANSFER_STATE_FAILED)) {
		close(file_descriptor);
	}

	if (NULL != _service_adaptor_upload_state_changed_reply) {
		long long int file_uid = _get_file_uid_from_plugin_fd(file_descriptor);
		_service_adaptor_upload_state_changed_reply(file_uid,
				state, file_info, error, user_data);
	}
}

void storage_adaptor_task_progress_reply_cb(int file_descriptor,
						unsigned long long progress_size_byte,
						unsigned long long total_size_byte,
						storage_adaptor_error_code_h error,
						void *user_data)
{
	if (NULL != _service_adaptor_task_progress_reply) {
		long long int file_uid = _get_file_uid_from_plugin_fd(file_descriptor);
		_service_adaptor_task_progress_reply(file_uid,
				progress_size_byte, total_size_byte);
	}
}

/* //------------------------------------------------------------------------
   // Functions implementations
   //------------------------------------------------------------------------ */

/* //////////////////////////////////////////////////////////
   // Adaptor Defined Plugin Function
   ////////////////////////////////////////////////////////// */

storage_error_code_t storage_plugin_open_file(storage_adaptor_plugin_context_h context,
						const char *file_path,
						storage_adaptor_file_access_mode_e mode,
						int *file_descriptor,
						storage_adaptor_error_code_h *error)
{
	storage_error_code_t ret = STORAGE_ADAPTOR_ERROR_NONE;

	if ((NULL == file_descriptor) || (NULL == file_path)) {
		if (NULL != error) {
			*error = storage_adaptor_create_error_code((int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
					"Invalid parameter");
		}
		ret = STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	} else {
		int fd;
		if (STORAGE_ADAPTOR_FILE_ACCESS_READ == mode) {
			fd = open(file_path, mode);
		} else if (STORAGE_ADAPTOR_FILE_ACCESS_WRITE == mode) {
			fd = open(file_path, mode, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		} else {
			if (NULL != error) {
				*error = storage_adaptor_create_error_code((int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
						"Invalid parameter (file mode)");
			}
			ret = STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
			return ret;
		}

		if (fd < 0) {
			ret = STORAGE_ADAPTOR_ERROR_PLUGIN_INTERNAL;
			int64_t error_code;
			if (EEXIST == errno) {
				error_code = (int64_t) STORAGE_PLUGIN_ERROR_FILE_AREADY_EXIST;
			} else if (EACCES == errno) {
				error_code = (int64_t) STORAGE_PLUGIN_ERROR_FILE_ACCESS_DENIED;
			} else {
				error_code = (int64_t) STORAGE_PLUGIN_ERROR_FILE_OPEN_FAILED;
			}
			if (NULL != error) {
				*error = storage_adaptor_create_error_code(error_code,
						"File open failed");
			}
		} else {
			*file_descriptor = fd;
		}
	}
	return ret;
}

storage_error_code_t storage_plugin_close_file(storage_adaptor_plugin_context_h context,
						int file_descriptor,
						storage_adaptor_error_code_h *error)
{
	storage_error_code_t ret = STORAGE_ADAPTOR_ERROR_NONE;

	int r = close(file_descriptor);
	if (r) {
		storage_adaptor_debug("close ret : %d", r);
	}

	return ret;
}


/* //////////////////////////////////////////////////////
   // Mandatory: External adaptor management function
   ////////////////////////////////////////////////////// */
storage_adaptor_h storage_adaptor_create(const char *plugins_dir)
{
	if (NULL == plugins_dir) {
		storage_adaptor_error("Invalid argument""(plugins_dir: %p", plugins_dir);
		return NULL;
	}

	storage_adaptor_h storage_adaptor = (storage_adaptor_h) malloc(sizeof(storage_adaptor_t));

	if (NULL == storage_adaptor) {
		storage_adaptor_error("Critical : Memory allocation failed");
		return NULL;
	}

	/* for forked plugin */
	if (pipe(storage_adaptor->rd_cmd) == -1) {
		free(storage_adaptor);
		return NULL;
	}
	g_mutex_init(&storage_adaptor->rd_mutex);
	storage_adaptor->rd_list = NULL;

	storage_adaptor->started = 0;
	storage_adaptor->plugins_dir = strdup(plugins_dir);

	g_mutex_init(&storage_adaptor->storage_adaptor_mutex);
	g_mutex_init(&storage_adaptor->plugins_mutex);
	g_mutex_init(&storage_adaptor->adaptor_listeners_mutex);

	g_mutex_lock(&storage_adaptor->adaptor_listeners_mutex);
	storage_adaptor->adaptor_listeners = NULL;
	g_mutex_unlock(&storage_adaptor->adaptor_listeners_mutex);

	g_mutex_lock(&storage_adaptor->plugins_mutex);
	storage_adaptor->plugins = NULL;
	g_mutex_unlock(&storage_adaptor->plugins_mutex);

	#ifndef FORK_PLUGIN_ARCHITECTURE
		g_file_uid_list = g_hash_table_new_full(g_int_hash, g_int_equal, NULL, free);
	#endif

	return storage_adaptor;
}

void storage_adaptor_destroy(storage_adaptor_h adaptor)
{
	if (NULL == adaptor) {
		storage_adaptor_error("Invalid argument""(adaptor: %p)", adaptor);
		return ;
	}

	g_mutex_lock(&adaptor->storage_adaptor_mutex);
	if (0 != adaptor->started) {
		storage_adaptor_error("Storage adaptor is running. Forcing stop before destroy");
		storage_adaptor_stop(adaptor);
	}

	g_mutex_lock(&adaptor->plugins_mutex);
	if (NULL != adaptor->plugins) {
		g_list_free_full(adaptor->plugins, (GDestroyNotify) storage_adaptor_plugin_unref);
		adaptor->plugins = NULL;
	}
	g_mutex_unlock(&adaptor->plugins_mutex);

	g_mutex_lock(&adaptor->adaptor_listeners_mutex);
	if (NULL != adaptor->adaptor_listeners) {
		g_list_free(adaptor->adaptor_listeners);
		adaptor->adaptor_listeners = NULL;
	}
	g_mutex_unlock(&adaptor->adaptor_listeners_mutex);

	_service_adaptor_download_file_async_reply	= NULL;
	_service_adaptor_upload_file_async_reply	= NULL;
	_service_adaptor_file_transfer_progress_reply	= NULL;
	_service_adaptor_download_state_changed_reply	= NULL;
	_service_adaptor_upload_state_changed_reply	= NULL;
	_service_adaptor_task_progress_reply	= NULL;

	free(adaptor->plugins_dir);
	adaptor->plugins_dir = NULL;

	g_mutex_unlock(&adaptor->storage_adaptor_mutex);

	#ifndef FORK_PLUGIN_ARCHITECTURE
		g_hash_table_destroy(g_file_uid_list);
	#endif

	/* For forked plugin */
	g_list_free(adaptor->rd_list);
	close(adaptor->rd_cmd[0]);
	close(adaptor->rd_cmd[1]);

	free(adaptor);
}

int storage_adaptor_start(storage_adaptor_h adaptor)
{
	storage_adaptor_debug("Starting storage adaptor");
	if (NULL == adaptor) {
		storage_adaptor_error("Invalid argument""(adaptor: %p)", adaptor);
		return STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	g_mutex_lock(&adaptor->storage_adaptor_mutex);
	int result = STORAGE_ADAPTOR_ERROR_NONE;
	if (0 != adaptor->started) {
		storage_adaptor_error("Storage adaptor is already started");
		result = STORAGE_ADAPTOR_ERROR_START;
	} else {
		adaptor->started = 1;

		pthread_t pid;
		if (pthread_create(&pid, NULL, _storage_adaptor_plugin_message_collector, (void *)adaptor)) {
			adaptor->started = 0;
			storage_adaptor_error("Could not create 3rd party plugin listener");
			result = STORAGE_ADAPTOR_ERROR_NOT_FOUND;
		} else if (STORAGE_ADAPTOR_ERROR_NONE != (result = storage_adaptor_load_plugins_from_directory(adaptor, adaptor->plugins_dir))) {
			_storage_adaptor_send_cmd_stop_listen(adaptor);
			adaptor->started = 0;
			storage_adaptor_error("Could not load plugins from directory");
			result = STORAGE_ADAPTOR_ERROR_NOT_FOUND;
		} else {
			adaptor->plugin_listener = pid;
			storage_adaptor_info("Storage adaptor started successfully");
		}
	}
	g_mutex_unlock(&adaptor->storage_adaptor_mutex);

	return result;
}

int storage_adaptor_stop(storage_adaptor_h adaptor)
{
	if (NULL == adaptor) {
		storage_adaptor_error("Invalid argument""(adaptor: %p)", adaptor);
		return STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	g_mutex_lock(&adaptor->storage_adaptor_mutex);

	/* For forked plugin */
	storage_adaptor_debug("stop plugin listener");
	_storage_adaptor_send_cmd_stop_listen(adaptor);
	pthread_join(adaptor->plugin_listener, NULL);

	int result = STORAGE_ADAPTOR_ERROR_NONE;
	if (0 == adaptor->started) {
		result = STORAGE_ADAPTOR_ERROR_START;
	} else {
		if (NULL != adaptor->plugins) {
			g_mutex_lock(&adaptor->plugins_mutex);
			g_list_free_full(adaptor->plugins, (GDestroyNotify) storage_adaptor_plugin_unref);
			adaptor->plugins = NULL;
			g_mutex_unlock(&adaptor->plugins_mutex);
		}
		adaptor->started = 0;
		storage_adaptor_debug("Storage adaptor stopped");
	}

	g_mutex_unlock(&adaptor->storage_adaptor_mutex);
	return result;
}

int storage_adaptor_register_listener(storage_adaptor_h adaptor,
						storage_adaptor_listener_h listener)
{
	if ((NULL == adaptor) || (NULL == listener)) {
		storage_adaptor_error("Invalid argument""(adaptor: %p, listener: %p)", adaptor, listener);
		return STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	g_mutex_lock(&adaptor->adaptor_listeners_mutex);

	adaptor->adaptor_listeners = g_list_append(adaptor->adaptor_listeners, listener);

	g_mutex_unlock(&adaptor->adaptor_listeners_mutex);

	_service_adaptor_download_file_async_reply =
			(storage_adaptor_service_download_file_async_reply_cb) listener->download_file_async_reply;
	_service_adaptor_upload_file_async_reply =
			(storage_adaptor_service_upload_file_async_reply_cb) listener->upload_file_async_reply;
	_service_adaptor_file_transfer_progress_reply =
			(storage_adaptor_service_file_transfer_progress_reply_cb) listener->file_transfer_progress_reply;
	_service_adaptor_download_state_changed_reply =
			(storage_adaptor_service_download_state_changed_reply_cb) listener->download_state_changed_reply;
	_service_adaptor_upload_state_changed_reply =
			(storage_adaptor_service_upload_state_changed_reply_cb) listener->upload_state_changed_reply;
	_service_adaptor_task_progress_reply =
			(storage_adaptor_service_task_progress_reply_cb) listener->task_progress_reply;

	return STORAGE_ADAPTOR_ERROR_NONE;
}

int storage_adaptor_unregister_listener(storage_adaptor_h adaptor,
						storage_adaptor_listener_h listener)
{
	if ((NULL == adaptor) || (NULL == listener)) {
		storage_adaptor_error("Invalid argument""(adaptor: %p, listener: %p)", adaptor, listener);
		return STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	g_mutex_lock(&adaptor->adaptor_listeners_mutex);

	if (NULL == g_list_find(adaptor->adaptor_listeners, listener)) {
		g_mutex_unlock(&adaptor->adaptor_listeners_mutex);
		storage_adaptor_error("Could not find listener");
		return STORAGE_ADAPTOR_ERROR_NOT_FOUND;
	}

	adaptor->adaptor_listeners = g_list_remove(adaptor->adaptor_listeners, listener);

	g_mutex_unlock(&adaptor->adaptor_listeners_mutex);

	_service_adaptor_download_file_async_reply	= NULL;
	_service_adaptor_upload_file_async_reply	= NULL;
	_service_adaptor_file_transfer_progress_reply	= NULL;
	_service_adaptor_download_state_changed_reply	= NULL;
	_service_adaptor_upload_state_changed_reply	= NULL;
	_service_adaptor_task_progress_reply	= NULL;

	return STORAGE_ADAPTOR_ERROR_NONE;
}

/* /////////////////////////////////////////////////////////////
   // Plugin create / destroy / ref. count / get plugin name
   ///////////////////////////////////////////////////////////// */
static storage_adaptor_plugin_h storage_adaptor_create_plugin(const char *plugin_path)
{
	if (NULL == plugin_path) {
		storage_adaptor_error("Invalid argument (plugin_path is null)");
		return NULL;
	}

	void *dl_handle = dlopen(plugin_path, RTLD_LAZY);
	if (NULL == dl_handle) {
		storage_adaptor_error("Could not load plugin %s: %s", plugin_path, dlerror());
		return NULL;
	}

	storage_adaptor_plugin_handle_h (*get_adaptee_handle)(void) = NULL;

	get_adaptee_handle = (storage_adaptor_plugin_handle_h (*)(void)) (dlsym(dl_handle, "create_plugin_handle"));
	if (NULL == get_adaptee_handle) {
		dlclose(dl_handle);
		storage_adaptor_error("Could not get function pointer to create_plugin_handle");
		return NULL;
	}

	plugin_req_enter();
	storage_adaptor_plugin_handle_h handle = get_adaptee_handle();
	plugin_req_exit_void();

	if (NULL == handle) {
		dlclose(dl_handle);
		storage_adaptor_error("Could not get adaptee handle");
		return NULL;
	}
	/* TBD not fixed */
	handle->open_file = storage_plugin_open_file;
	handle->close_file = storage_plugin_close_file;

	storage_adaptor_plugin_h plugin = (storage_adaptor_plugin_h) calloc(1, sizeof(storage_adaptor_plugin_t));
	if (NULL == plugin) {
		dlclose(dl_handle);
		storage_adaptor_error("Could not create plugin object");
		return NULL;
	}

	storage_adaptor_plugin_listener_h listener =
			(storage_adaptor_plugin_listener_h) calloc(1, sizeof(storage_adaptor_plugin_listener_t));

	if (NULL == listener) {
		free(plugin);
		dlclose(dl_handle);
		storage_adaptor_error("Could not create listener object");
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

	listener->storage_adaptor_download_file_async_reply	= storage_adaptor_download_file_async_reply_cb;
	listener->storage_adaptor_upload_file_async_reply	= storage_adaptor_upload_file_async_reply_cb;
	listener->storage_adaptor_file_transfer_progress_reply	= storage_adaptor_file_transfer_progress_reply_cb;
	listener->storage_adaptor_download_state_changed_reply	= storage_adaptor_download_state_changed_reply_cb;
	listener->storage_adaptor_upload_state_changed_reply	= storage_adaptor_upload_state_changed_reply_cb;
	listener->storage_adaptor_task_progress_reply	= storage_adaptor_task_progress_reply_cb;


	plugin_req_enter();
	plugin->handle->set_listener(listener);
	plugin_req_exit_void();

	g_mutex_lock(&plugin->plugin_listener_mutex);
	plugin->plugin_listener = listener;
	g_mutex_unlock(&plugin->plugin_listener_mutex);

	return plugin;
}

static void storage_adaptor_destroy_plugin(storage_adaptor_plugin_h plugin)
{
	if (NULL == plugin) {
		storage_adaptor_error("Invalid argument""(plugin: %p)", plugin);
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

static int storage_adaptor_load_plugins_from_directory(storage_adaptor_h adaptor,
						const char *dir_path)
{
	char *plugin_path = NULL;
	DIR *dir = NULL;
	struct dirent dir_entry, *result = NULL;

	storage_adaptor_debug("Starting load plugins from directory");

	if ((NULL == adaptor) || (NULL == dir_path)) {
		storage_adaptor_error("Invalid argument""(adaptor: %p, dir_path: %p)", adaptor, dir_path);
		return STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	dir = opendir(dir_path);
	if (NULL == dir) {
		storage_adaptor_error("Could not open dir path (%s)", dir_path);
		return STORAGE_ADAPTOR_ERROR_NOT_FOUND;
	}

	int ret = STORAGE_ADAPTOR_ERROR_NONE;
	while (0 == (readdir_r(dir, &dir_entry, &result))) {

		if (NULL == result) {
			storage_adaptor_error("Could not open directory %s", plugin_path);
			break;
		}

		if (dir_entry.d_type & DT_DIR) {
			continue;
		}

		plugin_path = g_strconcat(dir_path, "/", dir_entry.d_name, NULL);
		storage_adaptor_plugin_h plugin = storage_adaptor_create_plugin(plugin_path);

		if (NULL != plugin) {
			storage_adaptor_debug("Loaded plugin: %s", plugin_path);
			plugin->adaptor = adaptor;
			storage_adaptor_plugin_ref(plugin);
			g_mutex_lock(&adaptor->plugins_mutex);
			adaptor->plugins = g_list_append(adaptor->plugins, plugin);
			g_mutex_unlock(&adaptor->plugins_mutex);
		} else {
			storage_adaptor_error("Could not load plugin %s", plugin_path);
		}

		g_free(plugin_path);
		plugin_path = NULL;
	}

	storage_adaptor_debug("End load plugins from directory");
	closedir(dir);
	return ret;
}

static int storage_adaptor_has_plugin(storage_adaptor_h adaptor,
						storage_adaptor_plugin_h plugin)
{
	if ((NULL == adaptor) || (NULL == plugin)) {
		storage_adaptor_error("Invalid argument""(adaptor: %p, plugin: %p)", adaptor, plugin);
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

void storage_adaptor_plugin_ref(storage_adaptor_plugin_h plugin)
{
	if (NULL == plugin) {
		storage_adaptor_error("Invalid argument""(plugin: %p)", plugin);
		return;
	}

	g_mutex_lock(&plugin->ref_counter_mutex);
	plugin->ref_counter = plugin->ref_counter + 1;
	if (NULL != plugin->handle) {
		storage_adaptor_info("plugin name : %s, ref_counter: %d",
				plugin->handle->plugin_uri, plugin->ref_counter);
	} else {
		storage_adaptor_info("ref_counter: %d", plugin->ref_counter);
	}
	g_mutex_unlock(&plugin->ref_counter_mutex);
}

void storage_adaptor_plugin_unref(storage_adaptor_plugin_h plugin)
{
	if (NULL == plugin) {
		storage_adaptor_error("Invalid argument""(plugin: %p)", plugin);
		return ;
	}

	int should_destroy = 0;

	g_mutex_lock(&plugin->ref_counter_mutex);
	plugin->ref_counter = plugin->ref_counter - 1;
	if (NULL != plugin->handle) {
		storage_adaptor_info("plugin name : %s, ref_counter: %d",
				plugin->handle->plugin_uri, plugin->ref_counter);
	} else {
		storage_adaptor_info("ref_counter: %d", plugin->ref_counter);
	}
	if (0 >= plugin->ref_counter) {
		should_destroy = 1;
	}
	g_mutex_unlock(&plugin->ref_counter_mutex);

	if (should_destroy) {
		storage_adaptor_debug("Plugin is being destroyed");
		storage_adaptor_destroy_plugin(plugin);
	}
}

/* For 3rd party plugin packages */
int storage_adaptor_load_plugin_from_package(storage_adaptor_h adaptor,
						const char *package_id,
						const char *plugin_path)
{
	int adaptor_fd[2];
	int plugin_fd[2];

	if (pipe(adaptor_fd) == -1) {
		storage_adaptor_debug("pipe creation error, can not load plugin package");
	} else if (pipe(plugin_fd) == -1) {
		close(adaptor_fd[0]);
		close(adaptor_fd[1]);
		storage_adaptor_debug("pipe creation error[2], can not load plugin package");
	} else {
		g_process_identity = fork();
		if (0 == g_process_identity) {	/* child */
			storage_adaptor_debug_func("[CHILD PROCESS] forked success (PID : %d, id : %d)", (int)getpid());

			storage_adaptor_plugin_h plugin = NULL;
			plugin = storage_adaptor_create_plugin(plugin_path);
			if (NULL == plugin) {
				storage_adaptor_error("[CHILD PROCESS] Load plugin failed");
				exit(1);
			}
			g_child_plugin = plugin;
			plugin->rd = plugin_fd[0];
			close(plugin_fd[1]);
			plugin->wd = adaptor_fd[1];
			close(adaptor_fd[0]);
			void *temp = _storage_plugin_request_collector((void *)plugin);
			storage_adaptor_debug_func("[CHILD PROCESS] exit %p", temp);
			exit(0);
		} else if (0 < g_process_identity) {	/* parent */
			storage_adaptor_debug_func("[PARENT PROCESS] forked success (PID : %d)", (int)getpid());
			storage_adaptor_plugin_h _plugin = (storage_adaptor_plugin_h) calloc(1, sizeof(storage_adaptor_plugin_t));
			if (NULL == _plugin) {
				storage_adaptor_error("[PARENT PROCESS] memory allocation failed");
				exit(1);
			}

			_plugin->ref_counter = 0;
			g_mutex_init(&_plugin->ref_counter_mutex);
			g_mutex_init(&_plugin->message_mutex);

			_plugin->handle = __storage_adaptor_create_3rd_party_plugin_handle(package_id);

			_plugin->type = PLUGIN_TYPE_3RD_PARTY;
			_plugin->pid = g_process_identity;
			_plugin->rd = adaptor_fd[0];
			close(adaptor_fd[1]);
			_plugin->wd = plugin_fd[1];
			close(plugin_fd[0]);

			_storage_adaptor_send_cmd_add_fd(adaptor, _plugin->rd);

			_plugin->adaptor = adaptor;
			storage_adaptor_plugin_ref(_plugin);
			g_mutex_lock(&adaptor->plugins_mutex);
			adaptor->plugins = g_list_append(adaptor->plugins, _plugin);
			g_mutex_unlock(&adaptor->plugins_mutex);
		} else {
			close(adaptor_fd[0]);
			close(adaptor_fd[1]);
			close(plugin_fd[0]);
			close(plugin_fd[1]);
			storage_adaptor_debug("fork error, can not load plugin package");
		}
	}

	return 0;
}

/* //////////////////////////////////////////////////////
   // Plugin context create / destroy
   ////////////////////////////////////////////////////// */
storage_adaptor_plugin_context_h storage_adaptor_create_plugin_context(storage_adaptor_plugin_h plugin,
						const char *app_id,
						const char *app_secret,
						const char *access_token,
						const char *cid,
						const char *uid,
						const char *service_name)
{
	storage_adaptor_debug("Starting storage_adaptor_create_plugin_context");

	if (NULL == plugin) {
		storage_adaptor_error("Invalid argument""(plugin: %p)", plugin);
		return NULL;
	}

	if (NULL != plugin->handle) {
		storage_adaptor_plugin_context_h plugin_context = NULL;

		if (plugin->type == PLUGIN_TYPE_3RD_PARTY) {
			plugin_context = (storage_adaptor_plugin_context_h) calloc(1, sizeof(storage_adaptor_plugin_context_t));
			if (NULL == plugin_context) {
				return NULL;
			}
			plugin_context->plugin_handle = plugin;
		}

		plugin_req_enter();
		plugin->handle->create_context(&plugin_context, SAFE_ADD_STRING(app_id), SAFE_ADD_STRING(app_secret),
				SAFE_ADD_STRING(access_token), SAFE_ADD_STRING(cid), SAFE_ADD_STRING(uid));
		plugin_req_exit_void();

		if (NULL == plugin_context) {
			storage_adaptor_error("Create context failed");
			return NULL;
		}

		/* For forked plugin */
		g_mutex_lock(&plugin->contexts_mutex);
		plugin->contexts = g_list_append(plugin->contexts, (gpointer)plugin_context);
		g_mutex_unlock(&plugin->contexts_mutex);

		plugin_context->plugin_uri = strdup(plugin->handle->plugin_uri);
		plugin_context->service_name = strdup(service_name ? service_name : "");
		return plugin_context;
	} else {
		storage_adaptor_error("Plugin handle is null");
	}

	storage_adaptor_debug("End storage_adaptor_create_plugin_context");
	return NULL;
}

void storage_adaptor_destroy_plugin_context(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h plugin_context)
{
	if ((NULL == plugin) || (NULL == plugin_context)) {
		storage_adaptor_error("Invalid argument""(plugin: %p, plugin_context: %p)", plugin, plugin_context);
		return;
	}

	free(plugin_context->plugin_uri);
	plugin_context->plugin_uri = NULL;
/*	free(plugin_context->service_name); */
/*	plugin_context->service_name = NULL; */

	if (NULL != plugin->handle) {
		plugin_req_enter();
		plugin->handle->destroy_context(plugin_context);
		plugin_req_exit_void();
	} else {
		storage_adaptor_error("Plugin handle is null");
	}
}

/* //////////////////////////////////////////////////////
   // Get plugin by plugin name
   ////////////////////////////////////////////////////// */
storage_adaptor_plugin_h storage_adaptor_get_plugin_by_name(storage_adaptor_h adaptor,
						const char *plugin_uri)
{
	storage_adaptor_debug("Starting storage_adaptor_get_plugin_by_name");

	if ((NULL == adaptor) || (NULL == plugin_uri)) {
		storage_adaptor_error("Invalid argument""(adaptor: %p, plugin_uri: %p)", adaptor, plugin_uri);
		return NULL;
	}

	storage_adaptor_plugin_h plugin = NULL;
	g_mutex_lock(&adaptor->plugins_mutex);
	int count = g_list_length(adaptor->plugins);
	int i = 0;
	for (i = 0; i < count; i++) {
		storage_adaptor_plugin_h temp_plugin = g_list_nth_data(adaptor->plugins, i);
		if (NULL != temp_plugin) {
			if (0 == strcmp(temp_plugin->handle->plugin_uri, plugin_uri)) {
				storage_adaptor_plugin_ref(temp_plugin);
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
int storage_adaptor_load_plugin(storage_adaptor_h adaptor,
						const char *plugin_path)
{
	if ((NULL == adaptor) || (NULL == plugin_path)) {
		storage_adaptor_error("Invalid argument""(adaptor: %p, plugin_path: %p)", adaptor, plugin_path);
		return STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (0 == adaptor->started) {
		storage_adaptor_error("Storage adaptor is not started");
		return STORAGE_ADAPTOR_ERROR_START;
	}

	storage_adaptor_plugin_h plugin = storage_adaptor_create_plugin(plugin_path);
	if (NULL == plugin) {
		storage_adaptor_error("Could not load plugin %s", plugin_path);
		return STORAGE_ADAPTOR_ERROR_CREATE;
	}

	plugin->adaptor = adaptor;
	storage_adaptor_plugin_ref(plugin);

	g_mutex_lock(&adaptor->plugins_mutex);
	adaptor->plugins = g_list_append(adaptor->plugins, plugin);
	g_mutex_unlock(&adaptor->plugins_mutex);

	return STORAGE_ADAPTOR_ERROR_NONE;
}

int storage_adaptor_unload_plugin(storage_adaptor_h adaptor,
						storage_adaptor_plugin_h plugin)
{
	if ((NULL == adaptor) || (NULL == plugin)) {
		storage_adaptor_error("Invalid argument""(adaptor: %p, plugin: %p)", adaptor, plugin);
		return STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (0 == adaptor->started) {
		storage_adaptor_error("Storage adaptor is not started");
		return STORAGE_ADAPTOR_ERROR_START;
	}

	if (!storage_adaptor_has_plugin(adaptor, plugin)) {
		storage_adaptor_error("Storage adaptor has no plugin");
		return STORAGE_ADAPTOR_ERROR_NOT_FOUND;
	}

	plugin->adaptor = NULL;

	g_mutex_lock(&adaptor->plugins_mutex);
	adaptor->plugins = g_list_remove(adaptor->plugins, plugin);
	g_mutex_unlock(&adaptor->plugins_mutex);

	storage_adaptor_plugin_unref(plugin);

	return STORAGE_ADAPTOR_ERROR_NONE;
}

GList *storage_adaptor_get_plugins(storage_adaptor_h adaptor)
{
	if (NULL == adaptor) {
		storage_adaptor_error("Invalid argument""(adaptor: %p)", adaptor);
		return NULL;
	}

	GList *plugins = NULL;

	g_mutex_lock(&adaptor->plugins_mutex);
	int plugins_count = g_list_length(adaptor->plugins);
	int i = 0;
	for (i = 0; i < plugins_count; i++) {
		storage_adaptor_plugin_h plugin = g_list_nth_data(adaptor->plugins, i);
		if (NULL != plugin) {
			storage_adaptor_plugin_ref(plugin);
			plugins = g_list_append(plugins, plugin);
		}
	}
	g_mutex_unlock(&adaptor->plugins_mutex);

	return plugins;
}

/* ////////////////////////////////////////////////////////////
   // Adaptor Etc Functions
   //////////////////////////////////////////////////////////// */

/* Get plugin name by plugin */
void storage_adaptor_get_plugin_uri(storage_adaptor_plugin_h plugin,
						char **plugin_uri)
{
	if ((NULL == plugin) || (NULL == plugin_uri)) {
		storage_adaptor_error("Invalid argument""(plugin: %p)", plugin);
		return;
	}
	if ((NULL != plugin->handle) && (NULL != plugin->handle->plugin_uri)) {
		*plugin_uri = strdup(plugin->handle->plugin_uri);
	}
}

/**
 * Refresh access token
 */
EXPORT_API
storage_error_code_t storage_adaptor_refresh_access_token(storage_adaptor_plugin_context_h context,
						const char *new_access_token)
{
	if ((NULL == context) || (NULL == new_access_token) || (0 >= strlen(new_access_token))) {
		return STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}
	storage_adaptor_debug("New access token : %s", new_access_token);

	free(context->access_token);
	context->access_token = NULL;
	context->access_token = strdup(new_access_token);

	return STORAGE_ADAPTOR_ERROR_NONE;
}


EXPORT_API
storage_error_code_t storage_adaptor_refresh_uid(storage_adaptor_plugin_context_h context,
						const char *new_uid)
{
	if ((NULL == context) || (NULL == new_uid) || (0 >= strlen(new_uid))) {
		return STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}
	storage_adaptor_debug("New uid : %s", new_uid);

	free(context->uid);
	context->uid = NULL;
	context->uid = strdup(new_uid);

	return STORAGE_ADAPTOR_ERROR_NONE;
}



storage_adaptor_error_code_h storage_adaptor_create_error_code(const int64_t code,
						const char *msg)
{
	if (NULL == msg) {
		return NULL;
	}
	storage_adaptor_error_code_h error_code =
			(storage_adaptor_error_code_h) calloc(1, sizeof(storage_adaptor_error_code_t));
	if (NULL != error_code) {
		error_code->code = code;
		error_code->msg = strdup(msg);
	}

	return error_code;
}

void storage_adaptor_destroy_error_code(storage_adaptor_error_code_h *error_code)
{
	if ((NULL != error_code) && (NULL != (*error_code))) {
		free((*error_code)->msg);
		(*error_code)->msg = NULL;
		free(*error_code);
		*error_code = NULL;
	}

}

storage_adaptor_file_info_h storage_adaptor_create_file_info(void)
{
	storage_adaptor_file_info_h _file_info = NULL;
	_file_info = (storage_adaptor_file_info_h) calloc(1, sizeof(storage_adaptor_file_info_t));

	storage_adaptor_media_meta_s *_media_meta = NULL;
	_media_meta = (storage_adaptor_media_meta_s *) calloc(1, sizeof(storage_adaptor_media_meta_s));

	storage_adaptor_cloud_meta_s *_cloud_meta = NULL;
	_cloud_meta = (storage_adaptor_cloud_meta_s *) calloc(1, sizeof(storage_adaptor_cloud_meta_s));

	if ((NULL == _file_info) || (NULL == _media_meta) || (NULL == _cloud_meta)) {
		free(_file_info);
		free(_media_meta);
		free(_cloud_meta);

		return NULL;
	}

	_media_meta->mime_type		= NULL;
	_media_meta->title		= NULL;
	_media_meta->album		= NULL;
	_media_meta->artist		= NULL;
	_media_meta->genere		= NULL;
	_media_meta->recorded_date	= NULL;
	_media_meta->width		= -1;
	_media_meta->height		= -1;
	_media_meta->duration		= -1;
	_media_meta->copyright		= NULL;
	_media_meta->track_num		= NULL;
	_media_meta->description	= NULL;
	_media_meta->composer		= NULL;
	_media_meta->year		= NULL;
	_media_meta->bitrate		= -1;
	_media_meta->samplerate		= -1;
	_media_meta->channel		= -1;
	_media_meta->extra_media_meta	= NULL;

	_cloud_meta->service_name	= NULL;
	_cloud_meta->usage_byte		= 0ULL;
	_cloud_meta->quota_byte		= 0ULL;
	_cloud_meta->extra_cloud_meta	= NULL;

	_file_info->plugin_uri		= NULL;
	_file_info->object_id		= NULL;
	_file_info->storage_path	= NULL;
	_file_info->file_size		= 0ULL;

	/* private only!! */
	_file_info->revision		= -1;
	_file_info->timestamp		= 0ULL;
	_file_info->type		= NULL;
	_file_info->deleted		= -1;
	_file_info->expired_time	= 0ULL;
	_file_info->download_count	= -0U;
	_file_info->max_download_count	= -0U;
	_file_info->file_info_index	= -1;
	_file_info->tag			= NULL;
	_file_info->file_share_token	= NULL;

	/* public */
	_file_info->created_time	= 0ULL;
	_file_info->modified_time       = 0ULL;
	_file_info->file_info_index     = -1;
	_file_info->content_type	= STORAGE_ADAPTOR_CONTENT_TYPE_DEFAULT;
	_file_info->media_meta		= _media_meta;
	_file_info->cloud_meta		= _cloud_meta;
	_file_info->extra_file_info	= NULL;

	return _file_info;
}

int storage_adaptor_destroy_file_info(storage_adaptor_file_info_h *file_info)
{
	if (NULL == file_info) {
		return 1;
	}

	if (NULL == *file_info) {
		return 0;
	}
	storage_adaptor_file_info_h _file_info = *file_info;

	free(_file_info->plugin_uri);
	free(_file_info->object_id);
	free(_file_info->storage_path);
	free(_file_info->extra_file_info);
	free(_file_info->type);
	free(_file_info->tag);

	storage_adaptor_media_meta_s *_media_meta = _file_info->media_meta;

	if (NULL != _media_meta) {
		free(_media_meta->mime_type);
		free(_media_meta->title);
		free(_media_meta->album);
		free(_media_meta->artist);
		free(_media_meta->genere);
		free(_media_meta->recorded_date);
		free(_media_meta->copyright);
		free(_media_meta->track_num);
		free(_media_meta->description);
		free(_media_meta->composer);
		free(_media_meta->year);
		free(_media_meta->extra_media_meta);
	}

	storage_adaptor_cloud_meta_s *_cloud_meta = _file_info->cloud_meta;

	if (NULL != _cloud_meta) {
		free(_cloud_meta->service_name);
		free(_cloud_meta->extra_cloud_meta);
	}


	if (NULL != _file_info->file_share_token) {
		free(_file_info->file_share_token->public_token);
		_file_info->file_share_token->public_token = NULL;

		free(_file_info->file_share_token->auth_code);
		_file_info->file_share_token->auth_code = NULL;

		free(_file_info->file_share_token);
	}

	_file_info->plugin_uri		= NULL;
	_file_info->object_id		= NULL;
	_file_info->storage_path	= NULL;
	_file_info->revision		= -1;
	_file_info->timestamp		= 0ULL;
	_file_info->type		= NULL;
	_file_info->file_size		= 0ULL;
	_file_info->deleted		= -1;
	_file_info->expired_time	= 0ULL;
	_file_info->download_count	= 0U;
	_file_info->max_download_count	= 0U;
	_file_info->file_info_index	= -1;
	_file_info->tag			= NULL;
	_file_info->file_share_token	= NULL;


	free((*file_info)->media_meta);
	free((*file_info)->cloud_meta);
	free(*file_info);
	*file_info = NULL;

	return STORAGE_ADAPTOR_ERROR_NONE;
}

void __assign_error_code(storage_adaptor_error_code_h *error, const int64_t code, const char *msg)
{
	if (NULL != error) {
		*error = storage_adaptor_create_error_code(code, msg);
	}
}

/* ////////////////////////////////////////////////////////////
   // Adaptor Plugin call Functions
   //////////////////////////////////////////////////////////// */



/* ////////////////////// Public feature //////////////////////////// */

EXPORT_API
storage_error_code_t storage_adaptor_open_file(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *file_path,
						storage_adaptor_file_access_mode_e mode,
						long long int *file_uid,
						storage_adaptor_error_code_h *error)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, file_uid, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (file_uid)"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->open_file, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (open file)"));

	int plugin_fd = 0;
	storage_error_code_t ret = plugin->handle->open_file(context, file_path, mode, &plugin_fd, error);
	if (STORAGE_ADAPTOR_ERROR_NONE == ret) {
		*file_uid = _get_file_uid_from_plugin_fd(plugin_fd);

		#ifndef FORK_PLUGIN_ARCHITECTURE
		long long int *buf = (long long int *) calloc(1, sizeof(long long int));
		if (NULL != buf) {
			*buf = *file_uid;
			g_hash_table_insert(g_file_uid_list, (void *)&plugin_fd, (void *)buf);
		}
		#endif

		#ifdef DEBUG_ADAPTOR_PARAMS
			storage_adaptor_debug_func("plugin fd (%d), file uid(%lld)", plugin_fd, *file_uid);
		#endif
	}

	return ret;
}

EXPORT_API
storage_error_code_t storage_adaptor_close_file(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						long long int file_uid,
						storage_adaptor_error_code_h *error)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->close_file, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (close file)"));

	int plugin_fd = _get_plugin_fd_from_file_uid(file_uid);
	storage_error_code_t ret = plugin->handle->close_file(context, plugin_fd, error);

	#ifndef FORK_PLUGIN_ARCHITECTURE
		g_hash_table_remove(g_file_uid_list, (void *)&plugin_fd);
	#endif

	#ifdef DEBUG_ADAPTOR_PARAMS
		storage_adaptor_debug_func("plugin fd (%d), file uid(%lld)", plugin_fd, file_uid);
	#endif

	return ret;
}

EXPORT_API
storage_error_code_t storage_adaptor_start_upload_task(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						long long int src_file_descriptor,		/* read only opened */
						const char *upload_dir_path,
						const char *file_name,
						bool need_progress,
						storage_adaptor_error_code_h *error,
						void *user_data)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->start_upload_task, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (start_upload_task)"));

	storage_error_code_t ret = STORAGE_ADAPTOR_ERROR_NONE;

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s START ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] file_uid (%lld)", src_file_descriptor);
	storage_adaptor_debug_func("[in] path (%s / %s)", upload_dir_path, file_name);
	storage_adaptor_debug_func("[in] need progress (%d)", need_progress ? 1 : 0);
#endif

	int plugin_fd = _get_plugin_fd_from_file_uid(src_file_descriptor);
	plugin_req_enter();
	ret = plugin->handle->start_upload_task(context, plugin_fd,
			upload_dir_path, file_name, need_progress, error, user_data);
	plugin_req_exit(ret, plugin, error);

	return ret;
}

EXPORT_API
storage_error_code_t storage_adaptor_start_download_task(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *storage_dir_path,
						const char *file_name,
						long long int dst_file_descriptor,		/* write only opened */
						bool need_progress,
						storage_adaptor_error_code_h *error,
						void *user_data)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->start_download_task, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (start_download_task)"));

	storage_error_code_t ret = STORAGE_ADAPTOR_ERROR_NONE;

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s START ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] file_uid (%lld)", dst_file_descriptor);
	storage_adaptor_debug_func("[in] path (%s / %s)", storage_dir_path, file_name);
	storage_adaptor_debug_func("[in] need progress (%d)", need_progress ? 1 : 0);
#endif

	int plugin_fd = _get_plugin_fd_from_file_uid(dst_file_descriptor);
	plugin_req_enter();
	ret = plugin->handle->start_download_task(context, storage_dir_path, file_name,
			plugin_fd, need_progress, error, user_data);
	plugin_req_exit(ret, plugin, error);

	return ret;
}

EXPORT_API
storage_error_code_t storage_adaptor_start_download_thumb_task(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *storage_dir_path,
						const char *file_name,
						long long int dst_file_descriptor,		/* write only opened */
						int thumbnail_size,			/* level (defined plugin SPEC) */
						bool need_progress,
						storage_adaptor_error_code_h *error,
						void *user_data)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->start_download_thumb_task, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (start_download_thumb_task)"));

	storage_error_code_t ret = STORAGE_ADAPTOR_ERROR_NONE;

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s START ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] file_uid (%lld)", dst_file_descriptor);
	storage_adaptor_debug_func("[in] path (%s / %s)", storage_dir_path, file_name);
	storage_adaptor_debug_func("[in] need progress (%d)", need_progress ? 1 : 0);
#endif

	int plugin_fd = _get_plugin_fd_from_file_uid(dst_file_descriptor);
	plugin_req_enter();
	ret = plugin->handle->start_download_thumb_task(context, storage_dir_path, file_name,
			plugin_fd, thumbnail_size, need_progress, error, user_data);
	plugin_req_exit(ret, plugin, error);

	return ret;
}


EXPORT_API
storage_error_code_t storage_adaptor_cancel_upload_task(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						long long int file_uid,
						storage_adaptor_error_code_h *error)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->cancel_upload_task, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (cancel_upload_task)"));

	storage_error_code_t ret = STORAGE_ADAPTOR_ERROR_NONE;

	int plugin_fd = _get_plugin_fd_from_file_uid(file_uid);
	plugin_req_enter();
	ret = plugin->handle->cancel_upload_task(context, plugin_fd, error);
	plugin_req_exit(ret, plugin, error);

	return ret;
}

EXPORT_API
storage_error_code_t storage_adaptor_cancel_download_task(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						long long int file_uid,
						storage_adaptor_error_code_h *error)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->cancel_download_task, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (cancel_download_task)"));

	storage_error_code_t ret = STORAGE_ADAPTOR_ERROR_NONE;

	int plugin_fd = _get_plugin_fd_from_file_uid(file_uid);
	plugin_req_enter();
	ret = plugin->handle->cancel_download_task(context, plugin_fd, error);
	plugin_req_exit(ret, plugin, error);

	return ret;

}

EXPORT_API
storage_error_code_t storage_adaptor_cancel_download_thumb_task(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						long long int file_uid,
						storage_adaptor_error_code_h *error)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->cancel_download_thumb_task, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (cancel_download_thumb_task)"));

	storage_error_code_t ret = STORAGE_ADAPTOR_ERROR_NONE;

	int plugin_fd = _get_plugin_fd_from_file_uid(file_uid);
	plugin_req_enter();
	ret = plugin->handle->cancel_download_thumb_task(context, plugin_fd, error);
	plugin_req_exit(ret, plugin, error);

	return ret;

}

/* ////////////////////// Common feature //////////////////////////// */

/**
* @brief Set server information for Storage Plugin
*
* @param[in]	plugin				specifies Storage Adaptor Plugin handle
* @param[in]	context				specifies Storage Adaptor Plugin Context handle
* @param[in]	server_info			specifies server information for Storage Plugin
* @param[in]	request				specifies optional parameter
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
storage_error_code_t storage_adaptor_set_server_info(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						GHashTable *server_info,
						void *request,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->set_server_info, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (set_server_info)"));

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s START ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] Context [app_id(%s) access_token(%s) uid(%s)]",
			context->app_id, context->access_token, context->uid);
	storage_adaptor_debug_func("[in] server_info [addr(%p)]", server_info);
	storage_adaptor_debug_func("[in] request [addr(%p)]", request);
#endif

	plugin_req_enter();
	storage_error_code_t ret = plugin->handle->set_server_info(
			context, server_info, request, error, response);
	plugin_req_exit(ret, plugin, error);

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("[out] return code (%d)", ret);
	if ((NULL != error) && (NULL != *error)) {
		storage_adaptor_debug_func("[out] error->code (%llu)", (*error)->code);
		storage_adaptor_debug_func("[out] error->msg (%s)", (*error)->msg);
	}
	storage_adaptor_debug_func("[out] response [addr(%p)]", response);
	storage_adaptor_debug_func("========== %s END ==========", __FUNCTION__);
#endif

	return ret;
}

/**
* @brief Makes a directory at cloud
* @param[in]    upload_file_local_path          specifies local path of the file to be uploaded
*
* @param[in]    plugin                          specifies Storage Adaptor Plugin handle
* @param[in]    context                         specifies Storage Adaptor Plugin Context handle
* @param[in]    parent_folder_storage_path      specifies path to locate the folder you want to create
* @param[in]    folder_name                     specifies folder name to be created at cloud
* @param[in]    request                         specifies optional parameter
* @param[out]   file_info                       specifies Storage Adaptor File Info handle
* @param[out]   error                           specifies error code
* @param[out]   response                        specifies optional parameter
* @return 0 on success, otherwise a negative error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t storage_adaptor_make_directory(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *parent_folder_storage_path,
						const char *folder_name,
						void *request,
						storage_adaptor_file_info_h *file_info,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->make_directory, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (make_directory)"));

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s START ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] Context [app_id(%s) access_token(%s) uid(%s)]",
			context->app_id, context->access_token, context->uid);
	storage_adaptor_debug_func("[in] parent_folder_storage_path (%s)", parent_folder_storage_path);
	storage_adaptor_debug_func("[in] folder_name (%s)", folder_name);
	storage_adaptor_debug_func("[in] request [addr(%p)]", request);
#endif

	plugin_req_enter();
	storage_error_code_t ret = plugin->handle->make_directory(
			context, parent_folder_storage_path, folder_name, request, file_info, error, response);
	plugin_req_exit(ret, plugin, error);

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("[out] return code (%d)", ret);
	if ((NULL != file_info) && (NULL != (*file_info))) {
		storage_adaptor_debug_func("[out] file_info->storage_path (%s)", (*file_info)->storage_path);
	}
	if ((NULL != error) && (NULL != *error)) {
		storage_adaptor_debug_func("[out] error->code (%llu)", (*error)->code);
		storage_adaptor_debug_func("[out] error->msg (%s)", (*error)->msg);
	}
	storage_adaptor_debug_func("[out] response [addr(%p)]", response);
	storage_adaptor_debug_func("========== %s END ==========", __FUNCTION__);
#endif

	return ret;
}

/**
* @brief Removes a directory at cloud
*
* @param[in]    plugin                          specifies Storage Adaptor Plugin handle
* @param[in]    context                         specifies Storage Adaptor Plugin Context handle
* @param[in]    parent_folder_storage_path      specifies parent folder path of folder you want to delete
* @param[in]    folder_name                     specifies folder name to be deleted from cloud
* @param[in]    request                         specifies optional parameter
* @param[out]   file_info                       specifies Storage Adaptor File Info handle
* @param[out]   error                           specifies error code
* @param[out]   response                        specifies optional parameter
* @return 0 on success, otherwise a negative error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t storage_adaptor_remove_directory(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *parent_folder_storage_path,
						const char *folder_name,
						void *request,
						storage_adaptor_file_info_h *file_info,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->remove_directory, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (remove_directory)"));

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] Context [app_id(%s) access_token(%s) uid(%s)]",
			context->app_id, context->access_token, context->uid);
	storage_adaptor_debug_func("[in] parent_folder_storage_path (%s)", parent_folder_storage_path);
	storage_adaptor_debug_func("[in] folder_name (%s)", folder_name);
	storage_adaptor_debug_func("[in] request [addr(%p)]", request);
#endif

	plugin_req_enter();
	storage_error_code_t ret = plugin->handle->remove_directory(context, parent_folder_storage_path,
			folder_name, request, file_info, error, response);
	plugin_req_exit(ret, plugin, error);

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("[out] return code (%d)", ret);
	if ((NULL != file_info) && (NULL != (*file_info))) {
		storage_adaptor_debug_func("[out] file_info->storage_path (%s)", (*file_info)->storage_path);
	}
	if ((NULL != error) && (NULL != *error)) {
		storage_adaptor_debug_func("[out] error->code (%llu)", (*error)->code);
		storage_adaptor_debug_func("[out] error->msg (%s)", (*error)->msg);
	}
	storage_adaptor_debug_func("[out] response [addr(%p)]", response);
	storage_adaptor_debug_func("========== %s END ==========", __FUNCTION__);
#endif

	return ret;
}

/**
* @brief Requests folder and file list in a folder
*
* @param[in]    plugin                          specifies Storage Adaptor Plugin handle
* @param[in]    context                         specifies Storage Adaptor Plugin Context handle
* @param[in]    parent_folder_storage_path      specifies parent folder path of folder you want to get list
* @param[in]    folder_name                     specifies folder name you want to get list
* @param[in]    request                         specifies optional parameter
* @param[out]   file_info_list                  specifies Storage Adaptor File Info handle
* @param[out]   file_info_list_len              specifies length of the file_info_list
* @param[out]   error                           specifies error code
* @param[out]   response                        specifies optional parameter
* @return 0 on success, otherwise a negative error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t storage_adaptor_list(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *parent_folder_storage_path,
						const char *folder_name,
						void *request,
						storage_adaptor_file_info_h **file_info_list,
						int *file_info_list_len,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->list, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (list)"));

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] Context [app_id(%s) access_token(%s) uid(%s)]",
			context->app_id, context->access_token, context->uid);
	storage_adaptor_debug_func("[in] parent_folder_storage_path (%s)", parent_folder_storage_path);
	storage_adaptor_debug_func("[in] folder_name (%s)", folder_name);
	storage_adaptor_debug_func("[in] request [addr(%p)]", request);
#endif

	plugin_req_enter();
	storage_error_code_t ret = plugin->handle->list(context, parent_folder_storage_path,
			folder_name, request, file_info_list, file_info_list_len, error, response);
	plugin_req_exit(ret, plugin, error);

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("[out] return code (%d)", ret);
	storage_adaptor_debug_func("[out] file_info_list_len (%d)", *file_info_list_len);
	if ((NULL != error) && (NULL != *error)) {
		storage_adaptor_debug_func("[out] error->code (%llu)", (*error)->code);
		storage_adaptor_debug_func("[out] error->msg (%s)", (*error)->msg);
	}
	storage_adaptor_debug_func("[out] response [addr(%p)]", response);
	storage_adaptor_debug_func("========== %s END ==========", __FUNCTION__);
#endif

	return ret;
}

/**
* @brief Uploads a file to cloud (Sync)
*
* @param[in]    plugin                          specifies Storage Adaptor Plugin handle
* @param[in]    context                         specifies Storage Adaptor Plugin Context handle
* @param[in]    parent_folder_storage_path      specifies folder path of file you want to upload
* @param[in]    file_name                       specifies file name to be uploaded to cloud
* @param[in]    upload_file_local_path          specifies local path of the file to be uploaded
* @param[in]    publish                         specifies Allow to share file with no authentication
* @param[in]    request                         specifies optional parameter
* @param[out]   file_info                       specifies Storage Adaptor File Info handle
* @param[out]   error                           specifies error code
* @param[out]   response                        specifies optional parameter
* @return 0 on success, otherwise a negative error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t storage_adaptor_upload_file_sync(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *parent_folder_storage_path,
						const char *file_name,
						const char *upload_file_local_path,
						const int publish,
						void *request,
						storage_adaptor_file_info_h *file_info,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->upload_file_sync, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (upload_file_sync)"));

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] Context [app_id(%s) access_token(%s) uid(%s)]",
			context->app_id, context->access_token, context->uid);
	storage_adaptor_debug_func("[in] parent_folder_storage_path (%s)", parent_folder_storage_path);
	storage_adaptor_debug_func("[in] file_name (%s)", file_name);
	storage_adaptor_debug_func("[in] upload_file_local_path (%s)", upload_file_local_path);
	storage_adaptor_debug_func("[in] publish (%d)", publish);
	storage_adaptor_debug_func("[in] request [addr(%p)]", request);
#endif

	plugin_req_enter();
	storage_error_code_t ret = plugin->handle->upload_file_sync(context, parent_folder_storage_path,
			file_name, upload_file_local_path, publish, request, file_info, error, response);
	plugin_req_exit(ret, plugin, error);

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("[out] return code (%d)", ret);
	if ((NULL != file_info) && (NULL != (*file_info))) {
		storage_adaptor_debug_func("[out] file_info->storage_path (%s)", (*file_info)->storage_path);
		if ((1 == publish) && (NULL != (*file_info)->file_share_token)) {
			storage_adaptor_debug_func("[out] file_info->file_share_token->public_token (%s)",
					(*file_info)->file_share_token->public_token);
			storage_adaptor_debug_func("[out] file_info->file_share_token->auth_code (%s)",
					(*file_info)->file_share_token->auth_code);
		}
	}
	if ((NULL != error) && (NULL != *error)) {
		storage_adaptor_debug_func("[out] error->code (%llu)", (*error)->code);
		storage_adaptor_debug_func("[out] error->msg (%s)", (*error)->msg);
	}
	storage_adaptor_debug_func("[out] response [addr(%p)]", response);
	storage_adaptor_debug_func("========== %s END ==========", __FUNCTION__);
#endif

	return ret;
}

/**
* @brief Downloads a file to local (Sync)
*
* @param[in]    plugin                          specifies Storage Adaptor Plugin handle
* @param[in]    context                         specifies Storage Adaptor Plugin Context handle
* @param[in]    parent_folder_storage_path      specifies folder path of file you want to download
* @param[in]    file_name                       specifies file name to be downloaded to local
* @param[in]    download_file_local_path        specifies local path to download
* @param[in]    request                         specifies optional parameter
* @param[out]   error                           specifies error code
* @param[out]   response                        specifies optional parameter
* @return 0 on success, otherwise a negative error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t storage_adaptor_download_file_sync(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *parent_folder_storage_path,
						const char *file_name,
						const char *download_file_local_path,
						void *request,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->download_file_sync, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (download_file_sync)"));

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] Context [app_id(%s) access_token(%s) uid(%s)]",
			context->app_id, context->access_token, context->uid);
	storage_adaptor_debug_func("[in] parent_folder_storage_path (%s)", parent_folder_storage_path);
	storage_adaptor_debug_func("[in] file_name (%s)", file_name);
	storage_adaptor_debug_func("[in] download_file_local_path (%s)", download_file_local_path);
	storage_adaptor_debug_func("[in] request [addr(%p)]", request);
#endif

	plugin_req_enter();
	storage_error_code_t ret = plugin->handle->download_file_sync(context, parent_folder_storage_path,
			file_name, download_file_local_path, request, error, response);
	plugin_req_exit(ret, plugin, error);

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("[out] return code (%d)", ret);
	if ((NULL != error) && (NULL != *error)) {
		storage_adaptor_debug_func("[out] error->code (%llu)", (*error)->code);
		storage_adaptor_debug_func("[out] error->msg (%s)", (*error)->msg);
	}
	storage_adaptor_debug_func("[out] response [addr(%p)]", response);
	storage_adaptor_debug_func("========== %s END ==========", __FUNCTION__);
#endif

	return ret;
}

storage_error_code_t storage_adaptor_download_thumbnail(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *folder_path,
						const char *file_name,
						const char *download_path,
						int thumbnail_size,
						void *request,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->download_thumbnail, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (download_thumbnail)"));

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] Context [app_id(%s) access_token(%s) uid(%s)]",
			context->app_id, context->access_token, context->uid);
	storage_adaptor_debug_func("[in] folder_path (%s)", folder_path);
	storage_adaptor_debug_func("[in] file_name (%s)", file_name);
	storage_adaptor_debug_func("[in] download_path (%s)", download_path);
	storage_adaptor_debug_func("[in] thumbnail_size (%d)", thumbnail_size);
	storage_adaptor_debug_func("[in] request [addr(%p)]", request);
#endif

	plugin_req_enter();
	storage_error_code_t ret = plugin->handle->download_thumbnail(context, folder_path,
			file_name, download_path, thumbnail_size, request, error, response);
	plugin_req_exit(ret, plugin, error);

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("[out] return code (%d)", ret);
	if ((NULL != error) && (NULL != *error)) {
		storage_adaptor_debug_func("[out] error->code (%llu)", (*error)->code);
		storage_adaptor_debug_func("[out] error->msg (%s)", (*error)->msg);
	}
	storage_adaptor_debug_func("[out] response [addr(%p)]", response);
	storage_adaptor_debug_func("========== %s END ==========", __FUNCTION__);
#endif

	return ret;
}



/**
* @brief Removes a file at cloud
*
* @param[in]    plugin                          specifies Storage Adaptor Plugin handle
* @param[in]    context                         specifies Storage Adaptor Plugin Context handle
* @param[in]    parent_folder_storage_path      specifies folder path of file you want to delete
* @param[in]    file_name                       specifies file name to be deleted from cloud
* @param[in]    request                         specifies optional parameter
* @param[out]   file_info                       specifies Storage Adaptor File Info handle
* @param[out]   error                           specifies error code
* @param[out]   response                        specifies optional parameter
* @return 0 on success, otherwise a negative error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t storage_adaptor_delete_file(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *parent_folder_storage_path,
						const char *file_name,
						void *request,
						storage_adaptor_file_info_h *file_info,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->delete_file, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (delete_file)"));

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] Context [app_id(%s) access_token(%s) uid(%s)]",
			context->app_id, context->access_token, context->uid);
	storage_adaptor_debug_func("[in] parent_folder_storage_path (%s)", parent_folder_storage_path);
	storage_adaptor_debug_func("[in] file_name (%s)", file_name);
	storage_adaptor_debug_func("[in] request [addr(%p)]", request);
#endif

	plugin_req_enter();
	storage_error_code_t ret = plugin->handle->delete_file(context,	parent_folder_storage_path,
			file_name, request, file_info, error, response);
	plugin_req_exit(ret, plugin, error);

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("[out] return code (%d)", ret);
	if ((NULL != file_info) && (NULL != (*file_info))) {
		storage_adaptor_debug_func("[out] file_info->storage_path (%s)", (*file_info)->storage_path);
	}
	if ((NULL != error) && (NULL != *error)) {
		storage_adaptor_debug_func("[out] error->code (%llu)", (*error)->code);
		storage_adaptor_debug_func("[out] error->msg (%s)", (*error)->msg);
	}
	storage_adaptor_debug_func("[out] response [addr(%p)]", response);
	storage_adaptor_debug_func("========== %s END ==========", __FUNCTION__);
#endif

	return ret;
}

/**
* @brief Move a folder into destination folder
*
* @param[in]    plugin                          specifies Storage Adaptor Plugin handle
* @param[in]    context                         specifies Storage Adaptor Plugin Context handle
* @param[in]    parent_folder_storage_path      specifies parent folder path of folder you want to move
* @param[in]    folder_name                     specifies folder name to be moved
* @param[in]    dest_parent_folder_storage_path specifies new parent folder path
* @param[in]    new_folder_name                 specifies new folder name
* @param[in]    request                         specifies optional parameter
* @param[out]   file_info                       specifies Storage Adaptor File Info handle
* @param[out]   error                           specifies error code
* @param[out]   response                        specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t storage_adaptor_move_directory(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *parent_folder_storage_path,
						const char *folder_name,
						const char *dest_parent_folder_storage_path,
						const char *new_folder_name,
						void *request,
						storage_adaptor_file_info_h *file_info,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->move_directory, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (move_directory)"));

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] Context [app_id(%s) access_token(%s) uid(%s)]",
			context->app_id, context->access_token, context->uid);
	storage_adaptor_debug_func("[in] parent_folder_storage_path (%s)", parent_folder_storage_path);
	storage_adaptor_debug_func("[in] folder_name (%s)", folder_name);
	storage_adaptor_debug_func("[in] dest_parent_folder_storage_path (%s)", dest_parent_folder_storage_path);
	storage_adaptor_debug_func("[in] new_folder_name (%s)", new_folder_name);
	storage_adaptor_debug_func("[in] request [addr(%p)]", request);
#endif

	plugin_req_enter();
	storage_error_code_t ret = plugin->handle->move_directory(context, parent_folder_storage_path,
			folder_name, dest_parent_folder_storage_path, new_folder_name, request, file_info, error, response);
	plugin_req_exit(ret, plugin, error);

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("[out] return code (%d)", ret);
	if ((NULL != file_info) && (NULL != (*file_info))) {
		storage_adaptor_debug_func("[out] file_info->storage_path (%s)", (*file_info)->storage_path);
	}
	if ((NULL != error) && (NULL != *error)) {
		storage_adaptor_debug_func("[out] error->code (%llu)", (*error)->code);
		storage_adaptor_debug_func("[out] error->msg (%s)", (*error)->msg);
	}
	storage_adaptor_debug_func("[out] response [addr(%p)]", response);
	storage_adaptor_debug_func("========== %s END ==========", __FUNCTION__);
#endif

	return ret;
}

/**
* @brief Move a file into destination folder
*
* @param[in]    plugin                          specifies Storage Adaptor Plugin handle
* @param[in]    context                         specifies Storage Adaptor Plugin Context handle
* @param[in]    parent_folder_storage_path      specifies folder path of file you want to move
* @param[in]    file_name                       specifies file name to be moved
* @param[in]    dest_parent_folder_storage_path specifies new folder path
* @param[in]    new_file_name                   specifies new file name
* @param[in]    request                         specifies optional parameter
* @param[out]   file_info                       specifies Storage Adaptor File Info handle
* @param[out]   error                           specifies error code
* @param[out]   response                        specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t storage_adaptor_move_file(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *parent_folder_storage_path,
						const char *file_name,
						const char *dest_parent_folder_storage_path,
						const char *new_file_name,
						void *request,
						storage_adaptor_file_info_h *file_info,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->move_file, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (move_file)"));

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] Context [app_id(%s) access_token(%s) uid(%s)]",
			context->app_id, context->access_token, context->uid);
	storage_adaptor_debug_func("[in] parent_folder_storage_path (%s)", parent_folder_storage_path);
	storage_adaptor_debug_func("[in] file_name (%s)", file_name);
	storage_adaptor_debug_func("[in] dest_parent_folder_storage_path (%s)", dest_parent_folder_storage_path);
	storage_adaptor_debug_func("[in] new_file_name (%s)", new_file_name);
	storage_adaptor_debug_func("[in] request [addr(%p)]", request);
#endif

	plugin_req_enter();
	storage_error_code_t ret = plugin->handle->move_file(context, parent_folder_storage_path,
			file_name, dest_parent_folder_storage_path, new_file_name, request, file_info, error, response);
	plugin_req_exit(ret, plugin, error);

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("[out] return code (%d)", ret);
	if ((NULL != file_info) && (NULL != (*file_info))) {
		storage_adaptor_debug_func("[out] file_info->storage_path (%s)", (*file_info)->storage_path);
	}
	if ((NULL != error) && (NULL != *error)) {
		storage_adaptor_debug_func("[out] error->code (%llu)", (*error)->code);
		storage_adaptor_debug_func("[out] error->msg (%s)", (*error)->msg);
	}
	storage_adaptor_debug_func("[out] response [addr(%p)]", response);
	storage_adaptor_debug_func("========== %s END ==========", __FUNCTION__);
#endif

	return ret;
}

/**
* @brief Get state of file transfer request
*
* @param[in]    plugin                          specifies Storage Adaptor Plugin handle
* @param[in]    context                         specifies Storage Adaptor Plugin Context handle
* @param[in]    transfer_request_id             specifies unique id for file transfer request
* @param[in]    request                         specifies optional parameter
* @param[out]   state                          specifies current state of transfer request
* @param[out]   error                           specifies error code
* @param[out]   response                        specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t storage_adaptor_get_transfer_state(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						void *transfer_request_id,
						void *request,
						storage_adaptor_transfer_state_e *state,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->get_transfer_state, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (get_transfer_state)"));

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] Context [app_id(%s) access_token(%s) cid(%s) uid(%s)]",
			context->app_id, context->access_token, context->cid, context->uid);
	storage_adaptor_debug_func("[in] request [addr(%p)]", request);
#endif

	plugin_req_enter();
	storage_error_code_t ret = plugin->handle->get_transfer_state(context,	transfer_request_id,
			request, state, error, response);
	plugin_req_exit(ret, plugin, error);

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("[out] return code (%d)", ret);
	if (NULL != state) {
		storage_adaptor_debug_func("[out] state (%d)", *state);
	}
	if ((NULL != error) && (NULL != *error)) {
		storage_adaptor_debug_func("[out] error->code (%llu)", (*error)->code);
		storage_adaptor_debug_func("[out] error->msg (%s)", (*error)->msg);
	}
	storage_adaptor_debug_func("[out] response [addr(%p)]", response);
	storage_adaptor_debug_func("========== %s END ==========", __FUNCTION__);
#endif

	return ret;
}

/**
* @brief Set state of file transfer request
*
* @param[in]    plugin                          specifies Storage Adaptor Plugin handle
* @param[in]    context                         specifies Storage Adaptor Plugin Context handle
* @param[in]    transfer_request_id             specifies unique id for file transfer request
* @param[in]    state                          specifies state to set
* @param[in]    request                         specifies optional parameter
* @param[out]   error                           specifies error code
* @param[out]   response                        specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t storage_adaptor_set_transfer_state(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						void *transfer_request_id,
						storage_adaptor_transfer_state_e state,
						void *request,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->set_transfer_state, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (set_transfer_state)"));

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] Context [app_id(%s) access_token(%s) uid(%s)]",
			context->app_id, context->access_token, context->uid);
	storage_adaptor_debug_func("[in] state(%d)", state);
	storage_adaptor_debug_func("[in] request [addr(%p)]", request);
#endif

	plugin_req_enter();
	storage_error_code_t ret = plugin->handle->set_transfer_state(context, transfer_request_id,
			state, request, error, response);
	plugin_req_exit(ret, plugin, error);

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("[out] return code (%d)", ret);
	if ((NULL != error) && (NULL != *error)) {
		storage_adaptor_debug_func("[out] error->code (%llu)", (*error)->code);
		storage_adaptor_debug_func("[out] error->msg (%s)", (*error)->msg);
	}
	storage_adaptor_debug_func("[out] response [addr(%p)]", response);
	storage_adaptor_debug_func("========== %s END ==========", __FUNCTION__);
#endif

	return ret;
}

storage_error_code_t storage_adaptor_get_root_folder_path(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						void *request,
						char **root_folder_path,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, root_folder_path, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (root_folder_path)"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->get_root_folder_path, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (get_root_folder_path)"));

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] Context [app_id(%s) access_token(%s) uid(%s)]",
			context->app_id, context->access_token, context->uid);
	storage_adaptor_debug_func("[in] request [addr(%p)]", request);
#endif

	plugin_req_enter();
	storage_error_code_t ret = plugin->handle->get_root_folder_path(context, request, root_folder_path, error, response);
	plugin_req_exit(ret, plugin, error);

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("[out] return code (%d)", ret);
	if (NULL != *root_folder_path) {
		storage_adaptor_debug_func("[out] root_folder_path (%s)", *root_folder_path);
	}
	if ((NULL != error) && (NULL != *error)) {
		storage_adaptor_debug_func("[out] error->code (%llu)", (*error)->code);
		storage_adaptor_debug_func("[out] error->msg (%s)", (*error)->msg);
	}
	storage_adaptor_debug_func("[out] response [addr(%p)]", response);
	storage_adaptor_debug_func("========== %s END ==========", __FUNCTION__);
#endif

	return ret;
}

/* ////////////////////// Private feature //////////////////////////// */

/**
* @brief Uploads a file to cloud (Async)
*
* @param[in]    plugin                          specifies Storage Adaptor Plugin handle
* @param[in]    context                         specifies Storage Adaptor Plugin Context handle
* @param[in]    parent_folder_storage_path      specifies folder path of file you want to upload
* @param[in]    file_name                       specifies file name to be uploaded to cloud
* @param[in]    upload_file_local_path          specifies local path of the file to be uploaded
* @param[in]    publish                         specifies Allow to share file with no authentication
* @param[in]    request                         specifies optional parameter
* @param[out]   transfer_request_id             specifies
* @param[out]   error                           specifies error code
* @return 0 on success, otherwise a negative error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t storage_adaptor_upload_file_async(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *parent_folder_storage_path,
						const char *file_name,
						const char *upload_file_local_path,
						const int publish,
						void *request,
						void *transfer_request_id,
						storage_adaptor_error_code_h *error)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->upload_file_async, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (upload_file_async)"));

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] Context [app_id(%s) access_token(%s) cid(%s) uid(%s)]",
			context->app_id, context->access_token, context->cid, context->uid);
	storage_adaptor_debug_func("[in] parent_folder_storage_path (%s)", parent_folder_storage_path);
	storage_adaptor_debug_func("[in] file_name (%s)", file_name);
	storage_adaptor_debug_func("[in] upload_file_local_path (%s)", upload_file_local_path);
	storage_adaptor_debug_func("[in] publish (%d)", publish);
	storage_adaptor_debug_func("[in] request [addr(%p)]", request);
#endif

	plugin_req_enter();
	storage_error_code_t ret = plugin->handle->upload_file_async(context, parent_folder_storage_path,
			file_name, upload_file_local_path, publish, request, transfer_request_id);
	plugin_req_exit(ret, plugin, error);

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("[out] return code (%d)", ret);
	storage_adaptor_debug_func("========== %s END ==========", __FUNCTION__);
#endif

	return ret;
}

/**
* @brief Downloads a file to local (Async)
*
* @param[in]    plugin                          specifies Storage Adaptor Plugin handle
* @param[in]    context                         specifies Storage Adaptor Plugin Context handle
* @param[in]    parent_folder_storage_path      specifies folder path of file you want to download
* @param[in]    file_name                       specifies file name to be downloaded to local
* @param[in]    download_file_local_path        specifies local path to download
* @param[in]    request                         specifies optional parameter
* @param[out]   transfer_request_id             specifies
* @param[out]   error                           specifies error code
* @return 0 on success, otherwise a negative error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t storage_adaptor_download_file_async(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *parent_folder_storage_path,
						const char *file_name,
						const char *download_file_local_path,
						void *request,
						void *transfer_request_id,
						storage_adaptor_error_code_h *error)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->download_file_async, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (download_file_async)"));

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] Context [app_id(%s) access_token(%s) cid(%s) uid(%s)]",
			context->app_id, context->access_token, context->cid, context->uid);
	storage_adaptor_debug_func("[in] parent_folder_storage_path (%s)", parent_folder_storage_path);
	storage_adaptor_debug_func("[in] file_name (%s)", file_name);
	storage_adaptor_debug_func("[in] download_file_local_path (%s)", download_file_local_path);
	storage_adaptor_debug_func("[in] request (addr : %p)", request);
	storage_adaptor_debug_func("[in] request [addr(%p)]", request);
#endif

	plugin_req_enter();
	storage_error_code_t ret = plugin->handle->download_file_async(context, parent_folder_storage_path,
			file_name, download_file_local_path, request, transfer_request_id);
	plugin_req_exit(ret, plugin, error);

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("[out] return code (%d)", ret);
	storage_adaptor_debug_func("========== %s END ==========", __FUNCTION__);
#endif

	return ret;
}


/**
* @brief Sets metadata of file at cloud
*
* @param[in]    plugin                          specifies Storage Adaptor Plugin handle
* @param[in]    context                         specifies Storage Adaptor Plugin Context handle
* @param[in]    parent_folder_storage_path      specifies folder path of file you want to set meta data
* @param[in]    file_name                       specifies file name to be updated meta data
* @param[in]    meta_data                       specifies meta data (A pair of Key, Value)
* @param[in]    request                         specifies optional parameter
* @param[out]   file_info                       specifies Storage Adaptor File Info handle
* @param[out]   error                           specifies error code
* @param[out]   response                        specifies optional parameter
* @return 0 on success, otherwise a negative error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t storage_adaptor_set_meta(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *parent_folder_storage_path,
						const char *file_name,
						const void *meta_data,
						void *request,
						storage_adaptor_file_info_h *file_info,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->set_meta, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (set_meta)"));

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] Context [app_id(%s) access_token(%s) cid(%s) uid(%s)]",
			context->app_id, context->access_token, context->cid, context->uid);
	storage_adaptor_debug_func("[in] parent_folder_storage_path (%s)", parent_folder_storage_path);
	storage_adaptor_debug_func("[in] file_name (%s)", file_name);
	storage_adaptor_debug_func("[in] request [addr(%p)]", request);
#endif

	plugin_req_enter();
	storage_error_code_t ret = plugin->handle->set_meta(context, parent_folder_storage_path,
			file_name, meta_data, request, file_info, error, response);
	plugin_req_exit(ret, plugin, error);

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("[out] return code (%d)", ret);
	if ((NULL != file_info) && (NULL != (*file_info))) {
		storage_adaptor_debug_func("[out] file_info->storage_path (%s)", (*file_info)->storage_path);
	}
	if ((NULL != error) && (NULL != *error)) {
		storage_adaptor_debug_func("[out] error->code (%llu)", (*error)->code);
		storage_adaptor_debug_func("[out] error->msg (%s)", (*error)->msg);
	}
	storage_adaptor_debug_func("[out] response [addr(%p)]", response);
	storage_adaptor_debug_func("========== %s END ==========", __FUNCTION__);
#endif

	return ret;
}

/**
* @brief Gets metatdata of file at cloud
*
* @param[in]    plugin                          specifies Storage Adaptor Plugin handle
* @param[in]    context                         specifies Storage Adaptor Plugin Context handle
* @param[in]    parent_folder_storage_path      specifies folder path of file you want to get meta data
* @param[in]    file_name                       specifies file name
* @param[in]    request                         specifies optional parameter
* @param[out]   file_info                       specifies Storage Adaptor File Info handle
* @param[out]   meta_data                       specifies meta data (A pair of Key, Value)
* @param[out]   error                           specifies error code
* @param[out]   response                        specifies optional parameter
* @return 0 on success, otherwise a negative error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t storage_adaptor_get_meta(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *parent_folder_storage_path,
						const char *file_name,
						void *request,
						storage_adaptor_file_info_h *file_info,
						void **meta_data,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->get_meta, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (get_meta)"));

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] Context [app_id(%s) access_token(%s) cid(%s) uid(%s)]",
			context->app_id, context->access_token, context->cid, context->uid);
	storage_adaptor_debug_func("[in] parent_folder_storage_path (%s)", parent_folder_storage_path);
	storage_adaptor_debug_func("[in] file_name (%s)", file_name);
	storage_adaptor_debug_func("[in] request [addr(%p)]", request);
#endif

	plugin_req_enter();
	storage_error_code_t ret = plugin->handle->get_meta(context, parent_folder_storage_path,
			file_name, request, file_info, meta_data, error, response);
	plugin_req_exit(ret, plugin, error);

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("[out] return code (%d)", ret);
	if ((NULL != file_info) && (NULL != (*file_info))) {
		storage_adaptor_debug_func("[out] file_info->storage_path (%s)", (*file_info)->storage_path);
	}
	if ((NULL != error) && (NULL != *error)) {
		storage_adaptor_debug_func("[out] error->code (%llu)", (*error)->code);
		storage_adaptor_debug_func("[out] error->msg (%s)", (*error)->msg);
	}
	storage_adaptor_debug_func("[out] response [addr(%p)]", response);
	storage_adaptor_debug_func("========== %s END ==========", __FUNCTION__);
#endif

	return ret;
}

/**
* @brief Set up Multi Channel Upload
*
* @param[in]    plugin                          specifies Storage Adaptor Plugin handle
* @param[in]    context                         specifies Storage Adaptor Plugin Context handle
* @param[in]    parent_folder_storage_path      specifies folder path of file you want to upload
* @param[in]    file_name                       specifies file name to be uploaded to cloud
* @param[in]    upload_file_local_path          specifies local path of the file to be uploaded
* @param[in]    chunk_size_byte                 specifies size of chunk
* @param[in]    request                         specifies optional parameter
* @param[out]   mupload_key                     specifies Multi Channel Upload key
* @param[out]   chunk_count                     specifies total number of chunks
* @param[out]   file_info                       specifies Storage Adaptor File Info handle
* @param[out]   error                           specifies error code
* @param[out]   response                        specifies optional parameter
* @return 0 on success, otherwise a negative error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t storage_adaptor_start_mupload(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *parent_folder_storage_path,
						const char *file_name,
						const char *upload_file_local_path,
						const unsigned long long chunk_size,
						void *request,
						char **mupload_key,
						int *chunk_count,
						storage_adaptor_file_info_h *file_info,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));

	storage_adaptor_check_param_equal(NULL, plugin->handle->start_mupload, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (start_mupload)"));


#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] Context [app_id(%s) access_token(%s) cid(%s) uid(%s)]",
			context->app_id, context->access_token, context->cid, context->uid);
	storage_adaptor_debug_func("[in] file_name (%s)", file_name);
	storage_adaptor_debug_func("[in] chunk_size (%llu)", chunk_size);
	storage_adaptor_debug_func("[in] request [addr(%p)]", request);
#endif

	plugin_req_enter();
	storage_error_code_t ret = plugin->handle->start_mupload(context, parent_folder_storage_path,
			file_name, upload_file_local_path, chunk_size, request, mupload_key, chunk_count,
			file_info, error, response);
	plugin_req_exit(ret, plugin, error);

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("[out] return code (%d)", ret);
	if ((NULL != file_info) && (NULL != (*file_info))) {
		storage_adaptor_debug_func("[out] file_info->storage_path (%s)", (*file_info)->storage_path);
	}
	if ((NULL != error) && (NULL != *error)) {
		storage_adaptor_debug_func("[out] error->code (%llu)", (*error)->code);
		storage_adaptor_debug_func("[out] error->msg (%s)", (*error)->msg);
	}
	storage_adaptor_debug_func("[out] response [addr(%p)]", response);
	storage_adaptor_debug_func("========== %s END ==========", __FUNCTION__);
#endif

	return ret;
}

/**
* @brief Uploads a chunk to cloud
*
* @param[in]    plugin                          specifies Storage Adaptor Plugin handle
* @param[in]    context                         specifies Storage Adaptor Plugin Context handle
* @param[in]    mupload_key                     specifies Multi Channel Upload key
* @param[in]    chunk_number                    specifies number of chunk (Starting at 1)
* @param[in]    request                         specifies optional parameter
* @param[out]   file_info                       specifies Storage Adaptor File Info handle
* @param[out]   error                           specifies error code
* @param[out]   response                        specifies optional parameter
* @return 0 on success, otherwise a negative error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t storage_adaptor_upload_mupload(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *mupload_key,
						const int chunk_number,
						void *request,
						storage_adaptor_file_info_h *file_info,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->upload_mupload, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (upload_mupload)"));

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] Context [app_id(%s) access_token(%s) cid(%s) uid(%s)]",
			context->app_id, context->access_token, context->cid, context->uid);
	storage_adaptor_debug_func("[in] mupload_key (%s)", mupload_key);
	storage_adaptor_debug_func("[in] chunk_number (%d)", chunk_number);
	storage_adaptor_debug_func("[in] request [addr(%p)]", request);
#endif

	plugin_req_enter();
	storage_error_code_t ret = plugin->handle->upload_mupload(context, mupload_key,
			chunk_number, request, file_info, error, response);
	plugin_req_exit(ret, plugin, error);

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("[out] return code (%d)", ret);
	if ((NULL != file_info) && (NULL != (*file_info))) {
		storage_adaptor_debug_func("[out] file_info->storage_path (%s)", (*file_info)->storage_path);
	}
	if ((NULL != error) && (NULL != *error)) {
		storage_adaptor_debug_func("[out] error->code (%llu)", (*error)->code);
		storage_adaptor_debug_func("[out] error->msg (%s)", (*error)->msg);
	}
	storage_adaptor_debug_func("[out] response [addr(%p)]", response);
	storage_adaptor_debug_func("========== %s END ==========", __FUNCTION__);
#endif

	return ret;
}

/**
* @brief Ends Multi channel Upload
*
* @param[in]    plugin                          specifies Storage Adaptor Plugin handle
* @param[in]    context                         specifies Storage Adaptor Plugin Context handle
* @param[in]    mupload_key                     specifies Multi Channel Upload key
* @param[in]    publish                         specifies Allow to share file with no authentication
* @param[in]    request                         specifies optional parameter
* @param[out]   file_info                       specifies Storage Adaptor File Info handle
* @param[out]   error                           specifies error code
* @param[out]   response                        specifies optional parameter
* @return 0 on success, otherwise a negative error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t storage_adaptor_end_mupload(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *mupload_key,
						const int publish,
						void *request,
						storage_adaptor_file_info_h *file_info,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->end_mupload, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (end_mupoad)"));

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] Context [app_id(%s) access_token(%s) cid(%s) uid(%s)]",
			context->app_id, context->access_token, context->cid, context->uid);
	storage_adaptor_debug_func("[in] mupload_key (%s)", mupload_key);
	storage_adaptor_debug_func("[in] publish (%d)", publish);
	storage_adaptor_debug_func("[in] request [addr(%p)]", request);
#endif

	plugin_req_enter();
	storage_error_code_t ret = plugin->handle->end_mupload(context, mupload_key,
			publish, request, file_info, error, response);
	plugin_req_exit(ret, plugin, error);

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("[out] return code (%d)", ret);
	if ((NULL != file_info) && (NULL != (*file_info))) {
		storage_adaptor_debug_func("[out] file_info->storage_path (%s)", (*file_info)->storage_path);
	}
	if ((NULL != error) && (NULL != *error)) {
		storage_adaptor_debug_func("[out] error->code (%llu)", (*error)->code);
		storage_adaptor_debug_func("[out] error->msg (%s)", (*error)->msg);
	}
	storage_adaptor_debug_func("[out] response [addr(%p)]", response);
	storage_adaptor_debug_func("========== %s END ==========", __FUNCTION__);
#endif

	return ret;
}

/**
* @brief Requests list of chunks uploaded
*
* @param[in]    plugin                          specifies Storage Adaptor Plugin handle
* @param[in]    context                         specifies Storage Adaptor Plugin Context handle
* @param[in]    mupload_key                     specifies Multi Channel Upload key
* @param[in]    request                         specifies optional parameter
* @param[out]   file_info_list                  specifies Storage Adaptor File Info handle
* @param[out]   file_info_list_len              specifies length of the file_info_list
* @param[out]   error                           specifies error code
* @param[out]   response                        specifies optional parameter
* @return 0 on success, otherwise a negative error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t storage_adaptor_list_mupload(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *mupload_key,
						void *request,
						storage_adaptor_file_info_h **file_info_list,
						int *file_info_list_len,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->list_mupload, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (list_mupload)"));

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] Context [app_id(%s) access_token(%s) cid(%s) uid(%s)]",
			context->app_id, context->access_token, context->cid, context->uid);
	storage_adaptor_debug_func("[in] mupload_key (%s)", mupload_key);
	storage_adaptor_debug_func("[in] request [addr(%p)]", request);
#endif

	plugin_req_enter();
	storage_error_code_t ret = plugin->handle->list_mupload(context, mupload_key,
			request, file_info_list, file_info_list_len, error, response);
	plugin_req_exit(ret, plugin, error);

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("[out] return code (%d)", ret);
	if (NULL != file_info_list_len) {
		storage_adaptor_debug_func("[out] file_info_list_len (%d)", *file_info_list_len);
	}
	if ((NULL != error) && (NULL != *error)) {
		storage_adaptor_debug_func("[out] error->code (%llu)", (*error)->code);
		storage_adaptor_debug_func("[out] error->msg (%s)", (*error)->msg);
	}
	storage_adaptor_debug_func("[out] response [addr(%p)]", response);
	storage_adaptor_debug_func("========== %s END ==========", __FUNCTION__);
#endif

	return ret;
}

/**
* @brief Cancels all operations
*
* @param[in]    plugin                          specifies Storage Adaptor Plugin handle
* @param[in]    context                         specifies Storage Adaptor Plugin Context handle
* @param[in]    mupload_key                     specifies Multi Channel Upload key
* @param[in]    request                         specifies optional parameter
* @param[out]   file_info                       specifies Storage Adaptor File Info handle
* @param[out]   error                           specifies error code
* @param[out]   response                        specifies optional parameter
* @return 0 on success, otherwise a negative error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t storage_adaptor_cancel_mupload(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *mupload_key,
						void *request,
						storage_adaptor_file_info_h *file_info,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->cancel_mupload, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (cancel_mupload)"));

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] Context [app_id(%s) access_token(%s) cid(%s) uid(%s)]",
			context->app_id, context->access_token, context->cid, context->uid);
	storage_adaptor_debug_func("[in] mupload_key (%s)", mupload_key);
	storage_adaptor_debug_func("[in] request [addr(%p)]", request);
#endif

	plugin_req_enter();
	storage_error_code_t ret = plugin->handle->cancel_mupload(context, mupload_key,
			request, file_info, error, response);
	plugin_req_exit(ret, plugin, error);

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("[out] return code (%d)", ret);
	if ((NULL != file_info) && (NULL != (*file_info))) {
		storage_adaptor_debug_func("[out] file_info->storage_path (%s)", (*file_info)->storage_path);
	}
	if ((NULL != error) && (NULL != *error)) {
		storage_adaptor_debug_func("[out] error->code (%llu)", (*error)->code);
		storage_adaptor_debug_func("[out] error->msg (%s)", (*error)->msg);
	}
	storage_adaptor_debug_func("[out] response [addr(%p)]", response);
	storage_adaptor_debug_func("========== %s END ==========", __FUNCTION__);
#endif

	return ret;
}

/**
* @brief Starts Transaction
*
* @param[in]    plugin                          specifies Storage Adaptor Plugin handle
* @param[in]    context                         specifies Storage Adaptor Plugin Context handle
* @param[in]    request                         specifies optional parameter
* @param[out]   tx_key                          specifies Transaction key
* @param[out]   error                           specifies error code
* @param[out]   response                        specifies optional parameter
* @return 0 on success, otherwise a negative error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t storage_adaptor_start_transaction(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						void *request,
						char **tx_key,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->start_transaction, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (start_transacion)"));

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] Context [app_id(%s) access_token(%s) cid(%s) uid(%s)]",
			context->app_id, context->access_token, context->cid, context->uid);
	storage_adaptor_debug_func("[in] request [addr(%p)]", request);
#endif

	plugin_req_enter();
	storage_error_code_t ret = plugin->handle->start_transaction(context, request,
			tx_key, error, response);
	plugin_req_exit(ret, plugin, error);

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("[out] return code (%d)", ret);
	if (NULL != tx_key) {
		storage_adaptor_debug_func("[out] tx_key (%s)", *tx_key);
	}
	if ((NULL != error) && (NULL != *error)) {
		storage_adaptor_debug_func("[out] error->code (%llu)", (*error)->code);
		storage_adaptor_debug_func("[out] error->msg (%s)", (*error)->msg);
	}
	storage_adaptor_debug_func("[out] response [addr(%p)]", response);
	storage_adaptor_debug_func("========== %s END ==========", __FUNCTION__);
#endif

	return ret;
}

/**
* @brief Uploads a file
*
* @param[in]    plugin                          specifies Storage Adaptor Plugin handle
* @param[in]    context                         specifies Storage Adaptor Plugin Context handle
* @param[in]    tx_key                          specifies Transaction key
* @param[in]    tx_seq                          specifies Transaction sequesnce (Starting at 1)
* @param[in]    parent_folder_storage_path      specifies folder path of file you want to upload
* @param[in]    file_name                       specifies file name to be uploaded to cloud
* @param[in]    upload_file_local_path          specifies local path of the file to be uploaded
* @param[in]    request                         specifies optional parameter
* @param[out]   file_info                       specifies Storage Adaptor File Info handle
* @param[out]   error                           specifies error code
* @param[out]   response                        specifies optional parameter
* @return 0 on success, otherwise a negative error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t storage_adaptor_upload_file_transaction(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *tx_key,
						const int tx_seq,
						const char *parent_folder_storage_path,
						const char *file_name,
						const char *upload_file_local_path,
						void *request,
						storage_adaptor_file_info_h *file_info,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->upload_file_transaction, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (upload_file_transaction)"));

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] Context [app_id(%s) access_token(%s) cid(%s) uid(%s)]",
			context->app_id, context->access_token, context->cid, context->uid);
	storage_adaptor_debug_func("[in] tx_key (%s)", tx_key);
	storage_adaptor_debug_func("[in] tx_seq (%d)", tx_seq);
	storage_adaptor_debug_func("[in] parent_folder_storage_path (%s)", parent_folder_storage_path);
	storage_adaptor_debug_func("[in] file_name (%s)", file_name);
	storage_adaptor_debug_func("[in] upload_file_local_path (%s)", upload_file_local_path);
	storage_adaptor_debug_func("[in] request [addr(%p)]", request);
#endif

	plugin_req_enter();
	storage_error_code_t ret = plugin->handle->upload_file_transaction(context, tx_key,
			tx_seq, parent_folder_storage_path, file_name, upload_file_local_path, request, file_info, error, response);
	plugin_req_exit(ret, plugin, error);

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("[out] return code (%d)", ret);
	if ((NULL != file_info) && (NULL != (*file_info))) {
		storage_adaptor_debug_func("[out] file_info->storage_path (%s)", (*file_info)->storage_path);

	}
	if ((NULL != error) && (NULL != *error)) {
		storage_adaptor_debug_func("[out] error->code (%llu)", (*error)->code);
		storage_adaptor_debug_func("[out] error->msg (%s)", (*error)->msg);
	}
	storage_adaptor_debug_func("[out] response [addr(%p)]", response);
	storage_adaptor_debug_func("========== %s END ==========", __FUNCTION__);
#endif

	return ret;
}

/**
* @brief Updates a folder
*
* @param[in]    plugin                          specifies Storage Adaptor Plugin handle
* @param[in]    context                         specifies Storage Adaptor Plugin Context handle
* @param[in]    tx_key                          specifies Transaction key
* @param[in]    tx_seq                          specifies Transaction sequesnce (Starting at 1)
* @param[in]    parent_folder_storage_path      specifies folder path of folder you want to update
* @param[in]    folder_name                     specifies folder name to be updated
* @param[in]    request                         specifies optional parameter
* @param[out]   file_info                       specifies Storage Adaptor File Info handle
* @param[out]   error                           specifies error code
* @param[out]   response                        specifies optional parameter
* @return 0 on success, otherwise a negative error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t storage_adaptor_set_dir_transaction(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *tx_key,
						const int tx_seq,
						const char *parent_folder_storage_path,
						const char *folder_name,
						void *request,
						storage_adaptor_file_info_h *file_info,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->set_dir_transaction, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (set_dir_transaction)"));

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] Context [app_id(%s) access_token(%s) cid(%s) uid(%s)]",
			context->app_id, context->access_token, context->cid, context->uid);
	storage_adaptor_debug_func("[in] tx_key (%s)", tx_key);
	storage_adaptor_debug_func("[in] tx_seq (%d)", tx_seq);
	storage_adaptor_debug_func("[in] parent_folder_storage_path (%s)", parent_folder_storage_path);
	storage_adaptor_debug_func("[in] folder_name (%s)", folder_name);
	storage_adaptor_debug_func("[in] request [addr(%p)]", request);
#endif

	plugin_req_enter();
	storage_error_code_t ret = plugin->handle->set_dir_transaction(context,	tx_key,
			tx_seq,	parent_folder_storage_path, folder_name, request, file_info, error, response);
	plugin_req_exit(ret, plugin, error);

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("[out] return code (%d)", ret);
	if ((NULL != file_info) && (NULL != (*file_info))) {
		storage_adaptor_debug_func("[out] file_info->storage_path (%s)", (*file_info)->storage_path);
	}
	if ((NULL != error) && (NULL != *error)) {
		storage_adaptor_debug_func("[out] error->code (%llu)", (*error)->code);
		storage_adaptor_debug_func("[out] error->msg (%s)", (*error)->msg);
	}
	storage_adaptor_debug_func("[out] response [addr(%p)]", response);
	storage_adaptor_debug_func("========== %s END ==========", __FUNCTION__);
#endif

	return ret;
}

/**
* @brief Removes a file
*
* @param[in]    plugin                          specifies Storage Adaptor Plugin handle
* @param[in]    context                         specifies Storage Adaptor Plugin Context handle
* @param[in]    tx_key                          specifies Transaction key
* @param[in]    tx_seq                          specifies Transaction sequesnce (Starting at 1)
* @param[in]    parent_folder_storage_path      specifies folder path of file you want to delete
* @param[in]    file_name                       specifies file name to be deleted from cloud
* @param[in]    request                         specifies optional parameter
* @param[in]    upload_file_local_path          specifies local path of the file to be uploaded
* @param[out]   file_info                       specifies Storage Adaptor File Info handle
* @param[out]   error                           specifies error code
* @param[out]   response                        specifies optional parameter
* @return 0 on success, otherwise a negative error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t storage_adaptor_delete_file_transaction(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *tx_key,
						const int tx_seq,
						const char *parent_folder_storage_path,
						const char *file_name,
						void *request,
						storage_adaptor_file_info_h *file_info,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->delete_file_transaction, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (delete_file_transaction)"));

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] Context [app_id(%s) access_token(%s) cid(%s) uid(%s)]",
			context->app_id, context->access_token, context->cid, context->uid);
	storage_adaptor_debug_func("[in] tx_key (%s)", tx_key);
	storage_adaptor_debug_func("[in] tx_seq (%d)", tx_seq);
	storage_adaptor_debug_func("[in] parent_folder_storage_path (%s)", parent_folder_storage_path);
	storage_adaptor_debug_func("[in] file_name (%s)", file_name);
	storage_adaptor_debug_func("[in] request [addr(%p)]", request);
#endif

	plugin_req_enter();
	storage_error_code_t ret = plugin->handle->delete_file_transaction(context, tx_key,
			tx_seq,	parent_folder_storage_path, file_name, request, file_info, error, response);
	plugin_req_exit(ret, plugin, error);

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("[out] return code (%d)", ret);
	if ((NULL != file_info) && (NULL != (*file_info))) {
		storage_adaptor_debug_func("[out] file_info->storage_path (%s)", (*file_info)->storage_path);
	}
	if ((NULL != error) && (NULL != *error)) {
		storage_adaptor_debug_func("[out] error->code (%llu)", (*error)->code);
		storage_adaptor_debug_func("[out] error->msg (%s)", (*error)->msg);
	}
	storage_adaptor_debug_func("[out] response [addr(%p)]", response);
	storage_adaptor_debug_func("========== %s END ==========", __FUNCTION__);
#endif

	return ret;
}

/**
* @brief Removes a folder
*
* @param[in]    plugin                          specifies Storage Adaptor Plugin handle
* @param[in]    context                         specifies Storage Adaptor Plugin Context handle
* @param[in]    tx_key                          specifies Transaction key
* @param[in]    tx_seq                          specifies Transaction sequesnce (Starting at 1)
* @param[in]    parent_folder_storage_path      specifies folder path of folder you want to delete
* @param[in]    folder_name                     specifies folder name to be deleted
* @param[in]    request                         specifies optional parameter
* @param[out]   file_info                       specifies Storage Adaptor File Info handle
* @param[out]   error                           specifies error code
* @param[out]   response                        specifies optional parameter
* @return 0 on success, otherwise a negative error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t storage_adaptor_delete_dir_transaction(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *tx_key,
						const int tx_seq,
						const char *parent_folder_storage_path,
						const char *folder_name,
						void *request,
						storage_adaptor_file_info_h *file_info,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->delete_dir_transaction, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (delete_dir_transaction)"));

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] Context [app_id(%s) access_token(%s) cid(%s) uid(%s)]",
			context->app_id, context->access_token, context->cid, context->uid);
	storage_adaptor_debug_func("[in] tx_key (%s)", tx_key);
	storage_adaptor_debug_func("[in] tx_seq (%d)", tx_seq);
	storage_adaptor_debug_func("[in] parent_folder_storage_path (%s)", parent_folder_storage_path);
	storage_adaptor_debug_func("[in] folder_name (%s)", folder_name);
	storage_adaptor_debug_func("[in] request [addr(%p)]", request);
#endif

	plugin_req_enter();
	storage_error_code_t ret = plugin->handle->delete_dir_transaction(context, tx_key,
			tx_seq, parent_folder_storage_path, folder_name, request, file_info, error, response);
	plugin_req_exit(ret, plugin, error);

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("[out] return code (%d)", ret);
	if ((NULL != file_info) && (NULL != (*file_info))) {
		storage_adaptor_debug_func("[out] file_info->storage_path (%s)", (*file_info)->storage_path);
	}
	if ((NULL != error) && (NULL != *error)) {
		storage_adaptor_debug_func("[out] error->code (%llu)", (*error)->code);
		storage_adaptor_debug_func("[out] error->msg (%s)", (*error)->msg);
	}
	storage_adaptor_debug_func("[out] response [addr(%p)]", response);
	storage_adaptor_debug_func("========== %s END ==========", __FUNCTION__);
#endif

	return ret;
}

/**
* @brief Ends Transaction
*
* @param[in]    plugin                          specifies Storage Adaptor Plugin handle
* @param[in]    context                         specifies Storage Adaptor Plugin Context handle
* @param[in]    tx_key                          specifies Transaction key
* @param[in]    tx_count                        specifies Transaction order count
* @param[in]    request                         specifies optional parameter
* @param[out]   file_info_list                  specifies Storage Adaptor File Info handle
* @param[out]   file_info_list_len              specifies length of the file_info_list
* @param[out]   error                           specifies error code
* @param[out]   response                        specifies optional parameter
* @return 0 on success, otherwise a negative error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t storage_adaptor_end_transaction(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *tx_key,
						const int tx_count,
						void *request,
						storage_adaptor_file_info_h **file_info_list,
						int *file_info_list_len,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->end_transaction, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (end_transaction)"));

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] Context [app_id(%s) access_token(%s) cid(%s) uid(%s)]",
			context->app_id, context->access_token, context->cid, context->uid);
	storage_adaptor_debug_func("[in] tx_key (%s)", tx_key);
	storage_adaptor_debug_func("[in] tx_count (%d)", tx_count);
	storage_adaptor_debug_func("[in] request [addr(%p)]", request);
#endif

	plugin_req_enter();
	storage_error_code_t ret = plugin->handle->end_transaction(context, tx_key,
			tx_count, request, file_info_list, file_info_list_len, error, response);
	plugin_req_exit(ret, plugin, error);

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("[out] return code (%d)", ret);
	if (NULL != file_info_list_len) {
		storage_adaptor_debug_func("[out] file_info_list_len (%d)", *file_info_list_len);
	}
	if ((NULL != error) && (NULL != *error)) {
		storage_adaptor_debug_func("[out] error->code (%llu)", (*error)->code);
		storage_adaptor_debug_func("[out] error->msg (%s)", (*error)->msg);
	}
	storage_adaptor_debug_func("[out] response [addr(%p)]", response);
	storage_adaptor_debug_func("========== %s END ==========", __FUNCTION__);
#endif

	return ret;
}

/**
* @brief Requests Transaction list
*
* @param[in]    plugin                          specifies Storage Adaptor Plugin handle
* @param[in]    context                         specifies Storage Adaptor Plugin Context handle
* @param[in]    tx_key                          specifies Transaction key
* @param[in]    request                         specifies optional parameter
* @param[out]   file_info_list                  specifies Storage Adaptor File Info handle
* @param[out]   file_info_list_len              specifies length of the file_info_list
* @param[out]   error                           specifies error code
* @param[out]   response                        specifies optional parameter
* @return 0 on success, otherwise a negative error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t storage_adaptor_list_transaction(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *tx_key,
						void *request,
						storage_adaptor_file_info_h **file_info_list,
						int *file_info_list_len,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->list_transaction, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (list_transaction)"));

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] Context [app_id(%s) access_token(%s) cid(%s) uid(%s)]",
			context->app_id, context->access_token, context->cid, context->uid);
	storage_adaptor_debug_func("[in] tx_key (%s)", tx_key);
	storage_adaptor_debug_func("[in] request [addr(%p)]", request);
#endif

	plugin_req_enter();
	storage_error_code_t ret = plugin->handle->list_transaction(context, tx_key,
			request, file_info_list, file_info_list_len, error, response);
	plugin_req_exit(ret, plugin, error);

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("[out] return code (%d)", ret);
	if (NULL != file_info_list_len) {
		storage_adaptor_debug_func("[out] file_info_list_len (%d)", *file_info_list_len);
	}
	if ((NULL != error) && (NULL != *error)) {
		storage_adaptor_debug_func("[out] error->code (%llu)", (*error)->code);
		storage_adaptor_debug_func("[out] error->msg (%s)", (*error)->msg);
	}
	storage_adaptor_debug_func("[out] response [addr(%p)]", response);
	storage_adaptor_debug_func("========== %s END ==========", __FUNCTION__);
#endif

	return ret;
}

/**
* @brief Cancels all transactions
*
* @param[in]    plugin                          specifies Storage Adaptor Plugin handle
* @param[in]    context                         specifies Storage Adaptor Plugin Context handle
* @param[in]    tx_key                          specifies Transaction key
* @param[in]    request                         specifies optional parameter
* @param[out]   file_info_list                  specifies Storage Adaptor File Info handle
* @param[out]   file_info_list_len              specifies length of the file_info_list
* @param[out]   error                           specifies error code
* @param[out]   response                        specifies optional parameter
* @return 0 on success, otherwise a negative error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t storage_adaptor_cancel_transaction(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *tx_key,
						void *request,
						storage_adaptor_file_info_h **file_info_list,
						int *file_info_list_len,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->cancel_transaction, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (cancel_transaction)"));

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] Context [app_id(%s) access_token(%s) cid(%s) uid(%s)]",
			context->app_id, context->access_token, context->cid, context->uid);
	storage_adaptor_debug_func("[in] tx_key (%s)", tx_key);
	storage_adaptor_debug_func("[in] request [addr(%p)]", request);
#endif

	plugin_req_enter();
	storage_error_code_t ret = plugin->handle->cancel_transaction(context, tx_key,
			request, file_info_list, file_info_list_len, error, response);
	plugin_req_exit(ret, plugin, error);

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("[out] return code (%d)", ret);
	if (NULL != file_info_list_len) {
		storage_adaptor_debug_func("[out] file_info_list_len (%d)", *file_info_list_len);
	}
	if ((NULL != error) && (NULL != *error)) {
		storage_adaptor_debug_func("[out] error->code (%llu)", (*error)->code);
		storage_adaptor_debug_func("[out] error->msg (%s)", (*error)->msg);
	}
	storage_adaptor_debug_func("[out] response [addr(%p)]", response);
	storage_adaptor_debug_func("========== %s END ==========", __FUNCTION__);
#endif

	return ret;
}

/**
* @brief Uploads multiple files to cloud
*
* @param[in]    plugin                          specifies Storage Adaptor Plugin handle
* @param[in]    context                         specifies Storage Adaptor Plugin Context handle
* @param[in]    parent_folder_storage_path      specifies folder path of files you want to upload
* @param[in]    upload_file_local_path_list     specifies local path list of the files to be uploaded
* @param[in]    upload_list_len                 specifies total number of files to be uploaded
* @param[in]    request                         specifies optional parameter
* @param[out]   error                           specifies error code
* @param[out]   response                        specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t storage_adaptor_multi_file_upload(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *parent_folder_storage_path,
						const char **upload_file_local_path_list,
						const int upload_list_len,
						void *request,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->multi_file_upload, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (multi_file_upload)"));

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] Context [app_id(%s) access_token(%s) cid(%s) uid(%s)]",
			context->app_id, context->access_token, context->cid, context->uid);
	storage_adaptor_debug_func("[in] parent_folder_storage_path (%s)", parent_folder_storage_path);
	storage_adaptor_debug_func("[in] upload_list_len (%d)", upload_list_len);
	storage_adaptor_debug_func("[in] request [addr(%p)]", request);
#endif

	plugin_req_enter();
	storage_error_code_t ret = plugin->handle->multi_file_upload(context, parent_folder_storage_path,
			upload_file_local_path_list, upload_list_len, request, error, response);
	plugin_req_exit(ret, plugin, error);

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("[out] return code (%d)", ret);
	if ((NULL != error) && (NULL != *error)) {
		storage_adaptor_debug_func("[out] error->code (%llu)", (*error)->code);
		storage_adaptor_debug_func("[out] error->msg (%s)", (*error)->msg);
	}
	storage_adaptor_debug_func("[out] response [addr(%p)]", response);
	storage_adaptor_debug_func("========== %s END ==========", __FUNCTION__);
#endif

	return ret;
}

/**
* @brief Downloads multiple files in a folder
*
* @param[in]    plugin                          specifies Storage Adaptor Plugin handle
* @param[in]    context                         specifies Storage Adaptor Plugin Context handle
* @param[in]    parent_folder_storage_path      specifies folder path of files you want to download
* @param[in]    file_name_list                  specifies file name list to be downloaded
* @param[in]    file_name_list_len              specifies total number of files to be downloaded
* @param[in]    download_folder_local_path      specifies local folder path to download files
* @param[in]    request                         specifies optional parameter
* @param[out]   file_info_list                  specifies Storage Adaptor File Info handle
* @param[out]   file_info_list_len              specifies length of the file_info_list
* @param[out]   error                           specifies error code
* @param[out]   response                        specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t storage_adaptor_multi_file_download(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *parent_folder_storage_path,
						const char *file_name_list,
						const int file_name_list_len,
						const char *download_folder_local_path,
						void *request,
						storage_adaptor_file_info_h **file_info_list,
						int *file_info_list_len,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->multi_file_download, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (multi_file_download)"));

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] Context [app_id(%s) access_token(%s) cid(%s) uid(%s)]",
			context->app_id, context->access_token, context->cid, context->uid);
	storage_adaptor_debug_func("[in] parent_folder_storage_path (%s)", parent_folder_storage_path);
	storage_adaptor_debug_func("[in] download_list_len (%d)", file_name_list_len);
	storage_adaptor_debug_func("[in] download_folder_local_path (%s)", download_folder_local_path);
	storage_adaptor_debug_func("[in] request [addr(%p)]", request);
#endif

	plugin_req_enter();
	storage_error_code_t ret = plugin->handle->multi_file_download(context,	parent_folder_storage_path,
			file_name_list, file_name_list_len, download_folder_local_path, request, file_info_list,
			file_info_list_len, error, response);
	plugin_req_exit(ret, plugin, error);

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("[out] return code (%d)", ret);
	if (NULL != file_info_list_len) {
		storage_adaptor_debug_func("[out] file_info_list_len (%d)", *file_info_list_len);
	}
	if ((NULL != error) && (NULL != *error)) {
		storage_adaptor_debug_func("[out] error->code (%llu)", (*error)->code);
		storage_adaptor_debug_func("[out] error->msg (%s)", (*error)->msg);
	}
	storage_adaptor_debug_func("[out] response [addr(%p)]", response);
	storage_adaptor_debug_func("========== %s END ==========", __FUNCTION__);
#endif

	return ret;
}

/**
* @brief Requests current server timestamp
*
* @param[in]    plugin                          specifies Storage Adaptor Plugin handle
* @param[in]    context                         specifies Storage Adaptor Plugin Context handle
* @param[in]    request                         specifies optional parameter
* @param[out]   timestamp                       specifies server timestamp
* @param[out]   error                           specifies error code
* @param[out]   response                        specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t storage_adaptor_get_timestamp(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						void *request,
						unsigned long long *timestamp,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->get_timestamp, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (get_timestamp)"));

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] Context [app_id(%s) access_token(%s) cid(%s) uid(%s)]",
			context->app_id, context->access_token, context->cid, context->uid);
	storage_adaptor_debug_func("[in] request [addr(%p)]", request);
#endif

	plugin_req_enter();
	storage_error_code_t ret = plugin->handle->get_timestamp(context, request,
			timestamp, error, response);
	plugin_req_exit(ret, plugin, error);

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("[out] return code (%d)", ret);
	if (NULL != timestamp) {
		storage_adaptor_debug_func("[out] timestamp (%llu)", *timestamp);
	}
	if ((NULL != error) && (NULL != *error)) {
		storage_adaptor_debug_func("[out] error->code (%llu)", (*error)->code);
		storage_adaptor_debug_func("[out] error->msg (%s)", (*error)->msg);
	}
	storage_adaptor_debug_func("[out] response [addr(%p)]", response);
	storage_adaptor_debug_func("========== %s END ==========", __FUNCTION__);
#endif

	return ret;
}

/**
* @brief Requests a file info by public token
*
* @param[in]    plugin                          specifies Storage Adaptor Plugin handle
* @param[in]    context                         specifies Storage Adaptor Plugin Context handle
* @param[in]    public_token                    specifies token for Download, Get API
						(when terminal upload file and add publish=true parameter, or
* @param[in]    auth_code                       specifies Authentication code for public APIs
* @param[in]    request                         specifies optional parameter
* @param[out]   file_info                       specifies Storage Adaptor File Info handle
* @param[out]   error                           specifies error code
* @param[out]   response                        specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t storage_adaptor_get_file_info_by_public_token(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *public_token,
						const char *auth_code,
						void *request,
						storage_adaptor_file_info_h *file_info,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->get_file_info_by_public_token, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (get_file_info_by_public_token)"));

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] Context [app_id(%s) access_token(%s) cid(%s) uid(%s)]",
			context->app_id, context->access_token, context->cid, context->uid);
	storage_adaptor_debug_func("[in] public_token (%s)", public_token);
	storage_adaptor_debug_func("[in] auth_code (%s)", auth_code);
	storage_adaptor_debug_func("[in] request [addr(%p)]", request);
#endif

	plugin_req_enter();
	storage_error_code_t ret = plugin->handle->get_file_info_by_public_token(context, public_token,
			auth_code, request, file_info, error, response);
	plugin_req_exit(ret, plugin, error);

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("[out] return code (%d)", ret);
	if ((NULL != file_info) && (NULL != (*file_info))) {
		storage_adaptor_debug_func("[out] file_info->storage_path (%s)", (*file_info)->storage_path);
	}
	if ((NULL != error) && (NULL != *error)) {
		storage_adaptor_debug_func("[out] error->code (%llu)", (*error)->code);
		storage_adaptor_debug_func("[out] error->msg (%s)", (*error)->msg);
	}
	storage_adaptor_debug_func("[out] response [addr(%p)]", response);
	storage_adaptor_debug_func("========== %s END ==========", __FUNCTION__);
#endif

	return ret;
}

/**
* @brief Downloads a file by public token (Sync)
*
* @param[in]    plugin                          specifies Storage Adaptor Plugin handle
* @param[in]    context                         specifies Storage Adaptor Plugin Context handle
* @param[in]    public_token                    specifies token for Download, Get API
						(when terminal upload file and add publish=true parameter, or
* @param[in]    auth_code                       specifies Authentication code for public APIs
* @param[in]    download_file_local_path        specifies local path to download
* @param[in]    request                         specifies optional parameter
* @param[out]   error                           specifies error code
* @param[out]   response                        specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t storage_adaptor_download_file_sync_by_public_token(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *public_token,
						const char *auth_code,
						const char *download_file_local_path,
						void *request,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->download_file_sync_by_public_token, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (download_file_sync_by_public_token)"));

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] Context [app_id(%s) access_token(%s) cid(%s) uid(%s)]",
			context->app_id, context->access_token, context->cid, context->uid);
	storage_adaptor_debug_func("[in] public_token (%s)", public_token);
	storage_adaptor_debug_func("[in] auth_code (%s)", auth_code);
	storage_adaptor_debug_func("[in] download_file_local_path (%s)", download_file_local_path);
	storage_adaptor_debug_func("[in] request [addr(%p)]", request);
#endif

	plugin_req_enter();
	storage_error_code_t ret = plugin->handle->download_file_sync_by_public_token(context, public_token,
			auth_code, download_file_local_path, request, error, response);
	plugin_req_exit(ret, plugin, error);

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("[out] return code (%d)", ret);
	if ((NULL != error) && (NULL != *error)) {
		storage_adaptor_debug_func("[out] error->code (%llu)", (*error)->code);
		storage_adaptor_debug_func("[out] error->msg (%s)", (*error)->msg);
	}
	storage_adaptor_debug_func("[out] response [addr(%p)]", response);
	storage_adaptor_debug_func("========== %s END ==========", __FUNCTION__);
#endif

	return ret;
}

/**
* @brief Downloads a file by public token (Async)
*
* @param[in]    plugin                          specifies Storage Adaptor Plugin handle
* @param[in]    context                         specifies Storage Adaptor Plugin Context handle
* @param[in]    public_token                    specifies token for Download, Get API
						(when terminal upload file and add publish=true parameter, or
* @param[in]    auth_code                       specifies Authentication code for public APIs
* @param[in]    download_file_local_path        specifies local path to download
* @param[in]    request                         specifies optional parameter
* @param[out]   transfer_request_id             specifies
* @param[out]   error                           specifies error code
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t storage_adaptor_download_file_async_by_public_token(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *public_token,
						const char *auth_code,
						const char *download_file_local_path,
						void *request,
						void *transfer_request_id,
						storage_adaptor_error_code_h *error)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->download_file_async_by_public_token, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (download_file_async_by_public_token)"));

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] Context [app_id(%s) access_token(%s) cid(%s) uid(%s)]",
			context->app_id, context->access_token, context->cid, context->uid);
	storage_adaptor_debug_func("[in] public_token (%s)", public_token);
	storage_adaptor_debug_func("[in] auth_code (%s)", auth_code);
	storage_adaptor_debug_func("[in] download_file_local_path (%s)", download_file_local_path);
	storage_adaptor_debug_func("[in] request [addr(%p)]", request);
#endif

	plugin_req_enter();
	storage_error_code_t ret = plugin->handle->download_file_async_by_public_token(context, public_token,
			auth_code, download_file_local_path, request, transfer_request_id);
	plugin_req_exit(ret, plugin, error);

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("[out] return code (%d)", ret);
	storage_adaptor_debug_func("[out] request_id [addr(%p)]", transfer_request_id);
	storage_adaptor_debug_func("========== %s END ==========", __FUNCTION__);
#endif

	return ret;
}

/**
* @brief Authenticates public auth code
*
* @param[in]    plugin                          specifies Storage Adaptor Plugin handle
* @param[in]    context                         specifies Storage Adaptor Plugin Context handle
* @param[in]    public_token                    specifies token for Download, Get API
						(when terminal upload file and add publish=true parameter, or
* @param[in]    auth_code                       specifies Authentication code for public APIs
* @param[in]    request                         specifies optional parameter
* @param[out]   error                           specifies error code
* @param[out]   response                        specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t storage_adaptor_auth_public_authcode_by_public_token(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *public_token,
						const char *auth_code,
						void *request,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->auth_public_authcode_by_public_token, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (auth_public_authcode_by_public_token)"));

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] Context [app_id(%s) access_token(%s) cid(%s) uid(%s)]",
			context->app_id, context->access_token, context->cid, context->uid);
	storage_adaptor_debug_func("[in] public_token (%s)", public_token);
	storage_adaptor_debug_func("[in] auth_code (%s)", auth_code);
	storage_adaptor_debug_func("[in] request [addr(%p)]", request);
#endif

	plugin_req_enter();
	storage_error_code_t ret = plugin->handle->auth_public_authcode_by_public_token(context, public_token,
			auth_code, request, error, response);
	plugin_req_exit(ret, plugin, error);

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("[out] return code (%d)", ret);
	if ((NULL != error) && (NULL != *error)) {
		storage_adaptor_debug_func("[out] error->code (%llu)", (*error)->code);
		storage_adaptor_debug_func("[out] error->msg (%s)", (*error)->msg);
	}
	storage_adaptor_debug_func("[out] response [addr(%p)]", response);
	storage_adaptor_debug_func("========== %s END ==========", __FUNCTION__);
#endif

	return ret;
}

/**
* @brief Removes multiple files in a folder
*
* @param[in]    plugin                          specifies Storage Adaptor Plugin handle
* @param[in]    context                         specifies Storage Adaptor Plugin Context handle
* @param[in]    parent_folder_storage_path      specifies folder path of files you want to delete
* @param[in]    file_name_list                  specifies file name list to be deleted
* @param[in]    file_name_list_len              specifies total number of files to be deleted
* @param[in]    request                         specifies optional parameter
* @param[out]   file_info_list                  specifies Storage Adaptor File Info handle
* @param[out]   file_info_list_len              specifies length of the file_info_list
* @param[out]   error                           specifies error code
* @param[out]   response                        specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t storage_adaptor_delete_multi_file_in_folder(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *parent_folder_storage_path,
						const char **file_name_list,
						const int file_name_list_len,
						void *request,
						storage_adaptor_file_info_h **file_info_list,
						int *file_info_list_len,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->delete_multi_file_in_folder, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (delete_multi_file_in_folder)"));

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] Context [app_id(%s) access_token(%s) cid(%s) uid(%s)]",
			context->app_id, context->access_token, context->cid, context->uid);
	storage_adaptor_debug_func("[in] parent_folder_storage_path (%s)", parent_folder_storage_path);
	storage_adaptor_debug_func("[in] file_name_list_len (%d)", file_name_list_len);
	storage_adaptor_debug_func("[in] request [addr(%p)]", request);
#endif

	plugin_req_enter();
	storage_error_code_t ret = plugin->handle->delete_multi_file_in_folder(context,	parent_folder_storage_path,
			file_name_list, file_name_list_len, request, file_info_list, file_info_list_len, error, response);
	plugin_req_exit(ret, plugin, error);

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("[out] return code (%d)", ret);
	if (NULL != file_info_list_len) {
		storage_adaptor_debug_func("[out] file_info_list_len (%d)", *file_info_list_len);
	}
	if ((NULL != error) && (NULL != *error)) {
		storage_adaptor_debug_func("[out] error->code (%llu)", (*error)->code);
		storage_adaptor_debug_func("[out] error->msg (%s)", (*error)->msg);
	}
	storage_adaptor_debug_func("[out] response [addr(%p)]", response);
	storage_adaptor_debug_func("========== %s END ==========", __FUNCTION__);
#endif

	return ret;
}

/**
* @brief Requests policy for upload
*
* @param[in]    plugin                          specifies Storage Adaptor Plugin handle
* @param[in]    context                         specifies Storage Adaptor Plugin Context handle
* @param[in]    request                         specifies optional parameter
* @param[out]   allowed_extension               specifies
* @param[out]   allowed_extension_len           specifies length of allowed_extension
* @param[out]   error                           specifies error code
* @param[out]   response                        specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t storage_adaptor_get_policy(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						void *request,
						char ***allowed_extension,
						int *allowed_extension_len,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->get_policy, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (get_policy)"));

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] Context [app_id(%s) access_token(%s) cid(%s) uid(%s)]",
			context->app_id, context->access_token, context->cid, context->uid);
	storage_adaptor_debug_func("[in] request [addr(%p)]", request);
#endif

	plugin_req_enter();
	storage_error_code_t ret = plugin->handle->get_policy(context, request,
			allowed_extension, allowed_extension_len, error, response);
	plugin_req_exit(ret, plugin, error);

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("[out] return code (%d)", ret);
	if (NULL != allowed_extension_len) {
		storage_adaptor_debug_func("[out] allowed_extension_len (%d)", *allowed_extension_len);
	}
	if ((NULL != error) && (NULL != *error)) {
		storage_adaptor_debug_func("[out] error->code (%llu)", (*error)->code);
		storage_adaptor_debug_func("[out] error->msg (%s)", (*error)->msg);
	}
	storage_adaptor_debug_func("[out] response [addr(%p)]", response);
	storage_adaptor_debug_func("========== %s END ==========", __FUNCTION__);
#endif

	return ret;
}

/**
* @brief Requests quota of user
*
* @param[in]    plugin                          specifies Storage Adaptor Plugin handle
* @param[in]    context                         specifies Storage Adaptor Plugin Context handle
* @param[in]    request                         specifies optional parameter
* @param[out]   total_usage                     specifies total usage of user
* @param[out]   total_quota                     specifies total quota of user
* @param[out]   error                           specifies error code
* @param[out]   response                        specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t storage_adaptor_get_quota(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						void *request,
						unsigned long long *total_usage,
						unsigned long long *total_quota,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->get_quota, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (get_quota)"));

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] Context [app_id(%s) access_token(%s) cid(%s) uid(%s)]",
			context->app_id, context->access_token, context->cid, context->uid);
	storage_adaptor_debug_func("[in] request [addr(%p)]", request);
#endif

	plugin_req_enter();
	storage_error_code_t ret = plugin->handle->get_quota(context, request,
			total_usage, total_quota, error, response);
	plugin_req_exit(ret, plugin, error);

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("[out] return code (%d)", ret);
	if (NULL != total_usage) {
		storage_adaptor_debug_func("[out] total_usage (%llu)", *total_usage);
	}
	if (NULL != total_quota) {
		storage_adaptor_debug_func("[out] total_quota (%llu)", *total_quota);
	}
	if ((NULL != error) && (NULL != *error)) {
		storage_adaptor_debug_func("[out] error->code (%llu)", (*error)->code);
		storage_adaptor_debug_func("[out] error->msg (%s)", (*error)->msg);
	}
	storage_adaptor_debug_func("[out] response [addr(%p)]", response);
	storage_adaptor_debug_func("========== %s END ==========", __FUNCTION__);
#endif

	return ret;
}

/**
* @brief Requests Redirect URL mapped with public token (Not yet supported)
*
* @param[in]    plugin                          specifies Storage Adaptor Plugin handle
* @param[in]    context                         specifies Storage Adaptor Plugin Context handle
* @param[in]    public_token                    specifies token for Download, Get API
						(when terminal upload file and add publish=true parameter, or
* @param[in]    request                         specifies optional parameter
* @param[out]   error                           specifies error code
* @param[out]   response                        specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t storage_adaptor_redirect_url_by_public_token(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *public_token,
						void *request,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->redirect_url_by_public_token, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (redirect_url_by_public_token)"));

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] Context [app_id(%s) access_token(%s) cid(%s) uid(%s)]",
			context->app_id, context->access_token, context->cid, context->uid);
	storage_adaptor_debug_func("[in] public_token (%s)", public_token);
	storage_adaptor_debug_func("[in] request [addr(%p)]", request);
#endif

	plugin_req_enter();
	storage_error_code_t ret = plugin->handle->redirect_url_by_public_token(context, public_token,
			request, error, response);
	plugin_req_exit(ret, plugin, error);

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("[out] return code (%d)", ret);
	if ((NULL != error) && (NULL != *error)) {
		storage_adaptor_debug_func("[out] error->code (%llu)", (*error)->code);
		storage_adaptor_debug_func("[out] error->msg (%s)", (*error)->msg);
	}
	storage_adaptor_debug_func("[out] response [addr(%p)]", response);
	storage_adaptor_debug_func("========== %s END ==========", __FUNCTION__);
#endif

	return ret;
}

/**
* @brief Creates Upload URL (Not yet supported)
*
* @param[in]    plugin                          specifies Storage Adaptor Plugin handle
* @param[in]    context                         specifies Storage Adaptor Plugin Context handle
* @param[in]    parent_folder_storage_path      specifies folder path of files you want to upload
* @param[in]    file_name                       specifies file name to be uploaded
* @param[in]    x_upload_content_length         specifies length of content
* @param[in]    request                         specifies optional parameter
* @param[out]   error                           specifies error code
* @param[out]   response                        specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t storage_adaptor_create_resuming_upload_url(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *parent_folder_storage_path,
						const char *file_name,
						const unsigned long long x_upload_content_length,
						void *request,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->create_resuming_upload_url, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (create_resuming_upload_url)"));

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] Context [app_id(%s) access_token(%s) cid(%s) uid(%s)]",
			context->app_id, context->access_token, context->cid, context->uid);
	storage_adaptor_debug_func("[in] parent_folder_storage_path(%s)", parent_folder_storage_path);
	storage_adaptor_debug_func("[in] file_name(%s)", file_name);
	storage_adaptor_debug_func("[in] x_upload_content_length(%llu)", x_upload_content_length);
	storage_adaptor_debug_func("[in] request [addr(%p)]", request);
#endif

	plugin_req_enter();
	storage_error_code_t ret = plugin->handle->create_resuming_upload_url(context, parent_folder_storage_path,
			file_name, x_upload_content_length, request, error, response);
	plugin_req_exit(ret, plugin, error);

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("[out] return code (%d)", ret);
	if ((NULL != error) && (NULL != *error)) {
		storage_adaptor_debug_func("[out] error->code (%llu)", (*error)->code);
		storage_adaptor_debug_func("[out] error->msg (%s)", (*error)->msg);
	}
	storage_adaptor_debug_func("[out] response [addr(%p)]", response);
	storage_adaptor_debug_func("========== %s END ==========", __FUNCTION__);
#endif

	return ret;
}

/**
* @brief Creates chunk Upload URL (Not yet supported)
*
* @param[in]    plugin                          specifies Storage Adaptor Plugin handle
* @param[in]    context                         specifies Storage Adaptor Plugin Context handle
* @param[in]    mupload_key                     specifies Multi Channel Upload key
* @param[in]    chunk_number                    specifies number of chunk (Starting at 1)
* @param[in]    x_upload_content_length         specifies length of content
* @param[in]    request                         specifies optional parameter
* @param[out]   rupload_key                     specifies Resuming Upload key
* @param[out]   error                           specifies error code
* @param[out]   response                        specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t storage_adaptor_create_resuming_chunk_upload_url(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *mupload_key,
						const int chunk_number,
						const unsigned long long x_upload_content_length,
						void *request,
						char **rupload_key,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->create_resuming_chunk_upload_url, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (create_resuming_chunk_upload_url)"));

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] Context [app_id(%s) access_token(%s) cid(%s) uid(%s)]",
			context->app_id, context->access_token, context->cid, context->uid);
	storage_adaptor_debug_func("[in] mupload_key(%s)", mupload_key);
	storage_adaptor_debug_func("[in] chunk_number(%d)", chunk_number);
	storage_adaptor_debug_func("[in] x_upload_content_length(%llu)", x_upload_content_length);
	storage_adaptor_debug_func("[in] request [addr(%p)]", request);
#endif

	plugin_req_enter();
	storage_error_code_t ret = plugin->handle->create_resuming_chunk_upload_url(context, mupload_key,
			chunk_number, x_upload_content_length, request, rupload_key, error, response);
	plugin_req_exit(ret, plugin, error);

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("[out] return code (%d)", ret);
	if (NULL != rupload_key) {
		storage_adaptor_debug_func("[out] rupload_key (%s)", *rupload_key);
	}
	if ((NULL != error) && (NULL != *error)) {
		storage_adaptor_debug_func("[out] error->code (%llu)", (*error)->code);
		storage_adaptor_debug_func("[out] error->msg (%s)", (*error)->msg);
	}
	storage_adaptor_debug_func("[out] response [addr(%p)]", response);
	storage_adaptor_debug_func("========== %s END ==========", __FUNCTION__);
#endif

	return ret;
}

/**
* @brief Resumes Upload (Not yet supported)
*
* @param[in]    plugin                          specifies Storage Adaptor Plugin handle
* @param[in]    context                         specifies Storage Adaptor Plugin Context handle
* @param[in]    rupload_key                     specifies Resuming Upload key
* @param[in]    content_range                   specifies range of content
* @param[in]    content_length                  specifies length of content
* @param[in]    request                         specifies optional parameter
* @param[out]   error                           specifies error code
* @param[out]   response                        specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t storage_adaptor_resuming_upload(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *rupload_key,
						const char *change_range,
						const unsigned long long content_length,
						void *request,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->resuming_upload, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (resuming_upload)"));

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] Context [app_id(%s) access_token(%s) cid(%s) uid(%s)]",
			context->app_id, context->access_token, context->cid, context->uid);
	storage_adaptor_debug_func("[in] rupload_key(%s)", rupload_key);
	storage_adaptor_debug_func("[in] change_range(%d)", change_range);
	storage_adaptor_debug_func("[in] content_length(%llu)", content_length);
	storage_adaptor_debug_func("[in] request [addr(%p)]", request);
#endif

	plugin_req_enter();
	storage_error_code_t ret = plugin->handle->resuming_upload(context, rupload_key,
			change_range, content_length, request, error, response);
	plugin_req_exit(ret, plugin, error);

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("[out] return code (%d)", ret);
	if ((NULL != error) && (NULL != *error)) {
		storage_adaptor_debug_func("[out] error->code (%llu)", (*error)->code);
		storage_adaptor_debug_func("[out] error->msg (%s)", (*error)->msg);
	}
	storage_adaptor_debug_func("[out] response [addr(%p)]", response);
	storage_adaptor_debug_func("========== %s END ==========", __FUNCTION__);
#endif

	return ret;
}

/**
* @brief Get progress of file transfer request
*
* @param[in]    plugin                          specifies Storage Adaptor Plugin handle
* @param[in]    context                         specifies Storage Adaptor Plugin Context handle
* @param[in]    transfer_request_id             specifies unique id for file transfer request
* @param[in]    request                         specifies optional parameter
* @param[out]   progress_size                   specifies current progress size
* @param[out]   total_size                      specifies total size to transfer
* @param[out]   error                           specifies error code
* @param[out]   response                        specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t storage_adaptor_get_transfer_progress(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						void *transfer_request_id,
						void *request,
						unsigned long long *progress_size_byte,
						unsigned long long *total_size_byte,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (plugin)"));

	storage_adaptor_check_param_equal(NULL, plugin, STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT, "Invalid argument (context)"));

	storage_adaptor_check_param_equal(NULL, plugin->handle, STORAGE_ADAPTOR_ERROR_INVALID_HANDLE,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_INVALID_HANDLE, "Invalid plugin handle"));


	storage_adaptor_check_param_equal(NULL, plugin->handle->get_transfer_progress, STORAGE_ADAPTOR_ERROR_UNSUPPORTED,
		 __assign_error_code(error, (int64_t) STORAGE_ADAPTOR_ERROR_UNSUPPORTED, "Plugin does not support this API (get_transfer_progress)"));

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("========== %s ==========", __FUNCTION__);
	storage_adaptor_debug_func("[in] Context [app_id(%s) access_token(%s) cid(%s) uid(%s)]",
			context->app_id, context->access_token, context->cid, context->uid);
	storage_adaptor_debug_func("[in] tansfer_request_id [addr(%p)]", transfer_request_id);
	storage_adaptor_debug_func("[in] request [addr(%p)]", request);
#endif

	plugin_req_enter();
	storage_error_code_t ret = plugin->handle->get_transfer_progress(context, transfer_request_id,
			request, progress_size_byte, total_size_byte, error, response);
	plugin_req_exit(ret, plugin, error);

#ifdef DEBUG_ADAPTOR_PARAMS
	storage_adaptor_debug_func("[out] return code (%d)", ret);
	if (NULL != progress_size_byte) {
		storage_adaptor_debug_func("[out] progress size : %10llubyte", *progress_size_byte);
	}
	if (NULL != total_size_byte) {
		storage_adaptor_debug_func("[out] total    size : %10llubyte", *total_size_byte);
	}
	if ((NULL != error) && (NULL != *error)) {
		storage_adaptor_debug_func("[out] error->code (%llu)", (*error)->code);
		storage_adaptor_debug_func("[out] error->msg (%s)", (*error)->msg);
	}
	storage_adaptor_debug_func("[out] response [addr(%p)]", response);
	storage_adaptor_debug_func("========== %s END ==========", __FUNCTION__);
#endif

	return ret;
}














/* ///////////////////////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////////////////////
   ////////////  Internal function description (for forked plugin)  //////////////
   ///////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////// */

void *_storage_adaptor_plugin_message_collector(void *data)
{
	storage_adaptor_h adaptor = (storage_adaptor_h) data;

	storage_adaptor_info("3rd party plugin listener run");
	int i, lagest_fd = -1;
	fd_set read_set;
	struct timeval tv;
	tv.tv_sec = 10L;		/* TODO change to define or meaningful value */
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
/*			storage_adaptor_debug("select refrech by timeout(%ld sec) [id : %d]", tv.tv_sec, g_process_identity); */
			if (0L >= tv.tv_sec) {
/*				storage_adaptor_debug("Resets selector timeout sec"); */
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
						storage_adaptor_debug("Child process dead (Remove from listening queue)");
						dead_list = g_list_append(dead_list, (gpointer)fd);
						continue;
					}
					/* allocates and read buf data */
					memset(msg_buf, 0, PLUGIN_MESSAGE_PROTOCOL_MAX_BUF_SIZE);
					buf_size %= (PLUGIN_MESSAGE_PROTOCOL_MAX_BUF_SIZE - 1);
					rcv_len = read(fd, msg_buf, buf_size);
					storage_adaptor_debug("read message [%s][%d]", msg_buf, rcv_len);

					if (0 < rcv_len) {
						/* transfer data to adaptor */
						__storage_adaptor_transfer_message(msg_buf);
					} else {
						storage_adaptor_debug("Child process dead (Remove from listening queue)");
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
					storage_adaptor_debug("Parent process dead : Listener break");
					break;
				}

				/* allocates and read buf data */
				memset(msg_buf, 0, PLUGIN_MESSAGE_PROTOCOL_MAX_BUF_SIZE);
				buf_size %= (PLUGIN_MESSAGE_PROTOCOL_MAX_BUF_SIZE - 1);
				rcv_len = read(fd, msg_buf, buf_size);
				storage_adaptor_debug("read message [%s][%d]", msg_buf, rcv_len);

				if (0 >= rcv_len) {
					storage_adaptor_debug("Parent process dead : Listener break");
					break;
				}

				/* parse cmd message (e.g. append read_fd / change timeout sec / stop listener) */
				int cmd_ret = __storage_adaptor_parse_message_cmd(adaptor, msg_buf);
				if (0 > cmd_ret) {
					storage_adaptor_info("3rd party plugin listener stopped by adaptor cmd");
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
			storage_adaptor_error("plugin message listener error (errno : %d)", errno);
		}
	}
	storage_adaptor_info("3rd party plugin listener stopped");

	return data;
}

void __storage_adaptor_transfer_message(const char *msg)
{
	plugin_message_h t_msg = NULL;
	int ret = 0;
	ret = plugin_message_deserialize(msg, &t_msg);
	if (!ret) {
		pmnumber req_type, req_id;
		ret = plugin_message_get_value_number(t_msg, PLUGIN_MESSAGE_ELEMENT_MESSAGE_TYPE, &req_type);
		if (!ret && (PLUGIN_MESSAGE_TYPE_FUNCTION == req_type)) {
			storage_adaptor_debug("Message function type : function");
			ret = plugin_message_get_value_number(t_msg, PLUGIN_MESSAGE_ELEMENT_REQUEST_ID, &req_id);
			if (!ret) {
				storage_adaptor_debug("Send plugin data to requester");
				int hooked_fd = (int) req_id;
				int len = strlen(msg);
				ret = write(hooked_fd, &len, sizeof(int));
				ret = write(hooked_fd, msg, sizeof(char) * len);
			} else {
				storage_adaptor_debug("Couldn't get request id");
			}
		} else if (!ret && (PLUGIN_MESSAGE_TYPE_CALLBACK == req_type)) {
			storage_adaptor_debug("Message function type : callback");
			/*TODO call callback function */
			char *callback_name = NULL;
			ret = plugin_message_get_value_string(t_msg, PLUGIN_MESSAGE_ELEMENT_FUNCTION_NAME, &callback_name);
			storage_adaptor_error("Callback name : %s", callback_name);
			if (NULL == callback_name) {
				storage_adaptor_error("Function name parsing error");
			} else if (0 == strncmp(STORAGE_PLUGIN_CALLBACK_DOWNLOAD_FILE_ASYNC_CB,
						callback_name,
						strlen(STORAGE_PLUGIN_CALLBACK_DOWNLOAD_FILE_ASYNC_CB))) {
				pmnumber fd, state, ret_code;
				char *ret_msg = NULL;
				int param_idx = 1;
				storage_adaptor_error_code_h error = NULL;
				ret = plugin_message_get_value_number(t_msg, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, &ret_code);
				ret = plugin_message_get_value_string(t_msg, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, &ret_msg);
				if (NULL != ret_msg) {
					error = storage_adaptor_create_error_code((int64_t) ret_code, ret_msg);
				}
				ret = plugin_message_get_param_number(t_msg, param_idx++, &fd);
				ret = plugin_message_get_param_number(t_msg, param_idx++, &state);

				storage_adaptor_download_state_changed_reply_cb((int)fd, (storage_adaptor_transfer_state_e)state, error, NULL);
				storage_adaptor_destroy_error_code(&error);
			} else if (0 == strncmp(STORAGE_PLUGIN_CALLBACK_UPLOAD_FILE_ASYNC_CB,
						callback_name,
						strlen(STORAGE_PLUGIN_CALLBACK_UPLOAD_FILE_ASYNC_CB))) {
				pmnumber fd, state, ret_code;
				char *ret_msg = NULL;
				int param_idx = 1;
				storage_adaptor_error_code_h error = NULL;
				storage_adaptor_file_info_h file_info = NULL;
				ret = plugin_message_get_value_number(t_msg, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, &ret_code);
				ret = plugin_message_get_value_string(t_msg, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, &ret_msg);
				if (NULL != ret_msg) {
					error = storage_adaptor_create_error_code((int64_t) ret_code, ret_msg);
				}
				ret = plugin_message_get_param_number(t_msg, param_idx++, &fd);
				ret = plugin_message_get_param_number(t_msg, param_idx++, &state);

				plugin_message_array_h file_info_message = NULL;
				ret = plugin_message_get_param_array(t_msg, param_idx++, &file_info_message);
				if ((0 == ret) && (NULL != file_info_message)) {
					file_info = _get_file_info_from_message_array(file_info_message, param_idx++);
				}

				storage_adaptor_upload_state_changed_reply_cb((int)fd,
						(storage_adaptor_transfer_state_e)state, file_info, error, NULL);

				storage_adaptor_destroy_file_info(&file_info);
				plugin_message_array_destroy(file_info_message);
				storage_adaptor_destroy_error_code(&error);
			} else if (0 == strncmp(STORAGE_PLUGIN_CALLBACK_PROGRESS_CB,
						callback_name,
						strlen(STORAGE_PLUGIN_CALLBACK_PROGRESS_CB))) {
				pmnumber fd, progress, total, ret_code;
				char *ret_msg = NULL;
				int param_idx = 1;
				storage_adaptor_error_code_h error = NULL;
				ret = plugin_message_get_value_number(t_msg, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, &ret_code);
				ret = plugin_message_get_value_string(t_msg, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, &ret_msg);
				if (NULL != ret_msg) {
					error = storage_adaptor_create_error_code((int64_t) ret_code, ret_msg);
				}
				ret = plugin_message_get_param_number(t_msg, param_idx++, &fd);
				ret = plugin_message_get_param_number(t_msg, param_idx++, &progress);
				ret = plugin_message_get_param_number(t_msg, param_idx++, &total);

				storage_adaptor_task_progress_reply_cb((int)fd,
						(unsigned long long)progress, (unsigned long long)total, error, NULL);
				storage_adaptor_destroy_error_code(&error);
			} else {
				storage_adaptor_error("Invalid callback name : %s", callback_name);
			}
			free(callback_name);
		} else {
			storage_adaptor_warning("Received message parsing fail.");
		}
		plugin_message_destroy(t_msg);
	}
}

int __storage_adaptor_parse_message_cmd(storage_adaptor_h adaptor, char *msg)
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


void _storage_adaptor_send_cmd_add_fd(storage_adaptor_h adaptor, int fd)
{
	char cmd_buf[256] = {0, };
	snprintf(cmd_buf, 255, "%s%d", PLUGIN_MESSAGE_LISTENER_CMD_APPEND_FD, fd);
	int len = strlen(cmd_buf);
	int wr_ret;

	g_mutex_lock(&adaptor->rd_mutex);
	wr_ret = write(adaptor->rd_cmd[1], &len, sizeof(int));
	wr_ret = write(adaptor->rd_cmd[1], cmd_buf, sizeof(char) * len);
	g_mutex_unlock(&adaptor->rd_mutex);
	storage_adaptor_debug("writed (%d)(%s)", wr_ret, cmd_buf);
}

void _storage_adaptor_send_cmd_stop_listen(storage_adaptor_h adaptor)
{
	char cmd_buf[256] = {0, };
	snprintf(cmd_buf, 255, "%s", PLUGIN_MESSAGE_LISTENER_CMD_STOP);
	int len = strlen(cmd_buf);
	int wr_ret;

	g_mutex_lock(&adaptor->rd_mutex);
	wr_ret = write(adaptor->rd_cmd[1], &len, sizeof(int));
	wr_ret = write(adaptor->rd_cmd[1], cmd_buf, sizeof(char) * len);
	g_mutex_unlock(&adaptor->rd_mutex);
	storage_adaptor_debug("writed (%d)(%s)", wr_ret, cmd_buf);
}

static int storage_adaptor_send_message_to_plugin_sync(storage_adaptor_plugin_h plugin,
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
		memset(read_buf, 0, PLUGIN_MESSAGE_PROTOCOL_MAX_BUF_SIZE);
		len %= (PLUGIN_MESSAGE_PROTOCOL_MAX_BUF_SIZE - 1);
		if (0 < len) {
			io_ret = read(sync_hook[0], read_buf, len);
		}
		storage_adaptor_debug("io ret : %d", io_ret);
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

storage_adaptor_plugin_handle_h __storage_adaptor_create_3rd_party_plugin_handle(const char *plugin_uri)
{
	storage_adaptor_plugin_handle_h handle = (storage_adaptor_plugin_handle_h) calloc(1, sizeof(storage_adaptor_plugin_handle_t));

	if (NULL == handle) {
		return handle;
	}

	handle->create_context = storage_plugin_send_create_context;
	handle->destroy_context = storage_plugin_send_destroy_context;
	handle->set_server_info = storage_plugin_send_set_server_info;
	handle->make_directory = storage_plugin_send_make_directory;
	handle->remove_directory = storage_plugin_send_remove_directory;
	handle->list = storage_plugin_send_get_list;
	handle->upload_file_sync = storage_plugin_send_upload_file_sync;
	handle->download_file_sync = storage_plugin_send_download_file_sync;
	handle->delete_file = storage_plugin_send_delete_file;
	handle->move_directory = storage_plugin_send_move_directory;
	handle->move_file = storage_plugin_send_move_file;
	handle->set_transfer_state = storage_plugin_send_set_transfer_state;
	handle->get_transfer_state = storage_plugin_send_get_transfer_state;
	handle->get_root_folder_path = storage_plugin_send_get_root_folder_path;

	handle->start_upload_task = storage_plugin_send_start_upload_task;
	handle->start_download_task = storage_plugin_send_start_download_task;
	handle->start_download_thumb_task = storage_plugin_send_start_download_thumb_task;
	handle->cancel_upload_task = storage_plugin_send_cancel_upload_task;
	handle->cancel_download_task = storage_plugin_send_cancel_download_task;
	handle->cancel_download_thumb_task = storage_plugin_send_cancel_download_thumb_task;

	handle->plugin_uri = strdup(plugin_uri);

	return handle;
}

storage_adaptor_file_info_h _get_file_info_from_message_array(plugin_message_array_h message_array, int index)
{
	storage_adaptor_file_info_h file_info = NULL;

	if (NULL == message_array) {
		return file_info;
	}

/*
	//TODO
	_media_meta->mime_type		= NULL;
	_media_meta->title		= NULL;
	_media_meta->album		= NULL;
	_media_meta->artist		= NULL;
	_media_meta->genere		= NULL;
	_media_meta->recorded_date	= NULL;
	_media_meta->width		= -1;
	_media_meta->height		= -1;
	_media_meta->duration		= -1;
	_media_meta->copyright		= NULL;
	_media_meta->track_num		= NULL;
	_media_meta->description	= NULL;
	_media_meta->composer		= NULL;
	_media_meta->year		= NULL;
	_media_meta->bitrate		= -1;
	_media_meta->samplerate		= -1;
	_media_meta->channel		= -1;
	_media_meta->extra_media_meta	= NULL;

	_cloud_meta->service_name	= NULL;
	_cloud_meta->usage_byte		= 0ULL;
	_cloud_meta->quota_byte		= 0ULL;
	_cloud_meta->extra_cloud_meta	= NULL;
*/

	char *plugin_uri		= NULL;
	char *object_id			= NULL;
	char *storage_path		= NULL;
	pmnumber file_size		= 0LL;
	pmnumber created_time		= 0LL;
	pmnumber modified_time		= 0LL;
	pmnumber file_info_index	= -1LL;
	pmnumber content_type		= (pmnumber)STORAGE_ADAPTOR_CONTENT_TYPE_DEFAULT;
	char *extra_file_info		= NULL;

	int ret = 0;
	ret = plugin_message_array_get_element(message_array, index, &plugin_uri, &object_id, &storage_path,
			&file_size, &created_time, &modified_time, &file_info_index, &content_type, &extra_file_info);

	if (0 == ret) {
		file_info = storage_adaptor_create_file_info();

		if (NULL != file_info) {
			file_info->plugin_uri		= plugin_uri;
			file_info->object_id		= object_id;
			file_info->storage_path		= storage_path;
			file_info->file_size		= (unsigned long long) file_size;
			file_info->created_time		= (unsigned long long) created_time;
			file_info->modified_time	= (unsigned long long) modified_time;
			file_info->file_info_index	= (long long int) file_info_index;
			file_info->content_type		= (int) content_type;
			file_info->extra_file_info	= extra_file_info;
		} else {
			free(plugin_uri);
			free(object_id);
			free(extra_file_info);
		}
	}

	return file_info;
}

int _message_array_set_file_info(plugin_message_array_h message_array, int index, storage_adaptor_file_info_h file_info)
{
	int ret = STORAGE_ADAPTOR_ERROR_NONE;
	if ((NULL == message_array) || (NULL == file_info)) {
		return -1;
	}

/*
	//TODO
	_media_meta->mime_type		= NULL;
	_media_meta->title		= NULL;
	_media_meta->album		= NULL;
	_media_meta->artist		= NULL;
	_media_meta->genere		= NULL;
	_media_meta->recorded_date	= NULL;
	_media_meta->width		= -1;
	_media_meta->height		= -1;
	_media_meta->duration		= -1;
	_media_meta->copyright		= NULL;
	_media_meta->track_num		= NULL;
	_media_meta->description	= NULL;
	_media_meta->composer		= NULL;
	_media_meta->year		= NULL;
	_media_meta->bitrate		= -1;
	_media_meta->samplerate		= -1;
	_media_meta->channel		= -1;
	_media_meta->extra_media_meta	= NULL;

	_cloud_meta->service_name	= NULL;
	_cloud_meta->usage_byte		= 0ULL;
	_cloud_meta->quota_byte		= 0ULL;
	_cloud_meta->extra_cloud_meta	= NULL;
*/

	char *plugin_uri		= SAFE_ADD_STRING(file_info->plugin_uri);
	char *object_id			= SAFE_ADD_STRING(file_info->object_id);
	char *storage_path		= SAFE_ADD_STRING(file_info->storage_path);
	pmnumber file_size		= (pmnumber) file_info->file_size;
	pmnumber created_time		= (pmnumber) file_info->created_time;
	pmnumber modified_time		= (pmnumber) file_info->modified_time;
	pmnumber file_info_index	= (pmnumber) file_info->file_info_index;
	pmnumber content_type		= (pmnumber) file_info->content_type;
	char *extra_file_info		= SAFE_ADD_STRING(file_info->extra_file_info);

	ret = plugin_message_array_add_element(message_array, plugin_uri, object_id, storage_path,
			file_size, created_time, modified_time, file_info_index, content_type, extra_file_info);

	return ret;
}

storage_error_code_t storage_plugin_send_create_context(storage_adaptor_plugin_context_h *context,
							const char *app_id,
							const char *app_secret,
							const char *access_token,
							const char *cid,
							const char *uid)
{
	storage_adaptor_plugin_h plugin = NULL;
	plugin = (*context)->plugin_handle;

	int ret = 0;
	plugin_message_h message = NULL;
	ret = plugin_message_create(&message);

	if (ret == 0) {
		(*context)->context_id = (int) (intptr_t)(*context);

		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_CONTEXT_ID,
				(pmnumber) (*context)->context_id);
		plugin_message_set_value_string(message, PLUGIN_MESSAGE_ELEMENT_FUNCTION_NAME,
				STORAGE_PLUGIN_INTERFACE_CREATE_CONTEXT);

		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_MESSAGE_TYPE,
				(pmnumber) PLUGIN_MESSAGE_TYPE_FUNCTION);

		int param_index = 1;
		plugin_message_set_param_string(message, param_index++, app_id);
		plugin_message_set_param_string(message, param_index++, app_secret);
		plugin_message_set_param_string(message, param_index++, access_token);
		plugin_message_set_param_string(message, param_index++, cid);
		plugin_message_set_param_string(message, param_index++, uid);

		plugin_message_h result_message = NULL;
		ret = storage_adaptor_send_message_to_plugin_sync(plugin, message, &result_message);

		if (0 == ret) {
			pmnumber ret_code;
			plugin_message_get_value_number(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, &ret_code);

			ret = (int) ret_code;

			char *ret_msg = NULL;
			if (STORAGE_ADAPTOR_ERROR_NONE == ret) {
				storage_adaptor_debug("Create context successed");
			} else {
				plugin_message_get_value_string(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE, &ret_msg);
				storage_adaptor_debug("Create context failed (%d)(%s)", ret, ret_msg);
				free(ret_msg);
				ret_msg = NULL;

				free(*context);
				(*context) = NULL;
			}
			plugin_message_destroy(result_message);
		}
	} else {
		ret = STORAGE_ADAPTOR_ERROR_PLUGIN_INTERNAL;
	}
	plugin_message_destroy(message);

	return ret;
}

storage_error_code_t storage_plugin_send_destroy_context(storage_adaptor_plugin_context_h context)
{
	storage_adaptor_plugin_h plugin = NULL;
	plugin = context->plugin_handle;

	int ret = 0;
	plugin_message_h message = NULL;
	ret = plugin_message_create(&message);

	if (ret == 0) {
		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_CONTEXT_ID,
				(pmnumber) context->context_id);
		plugin_message_set_value_string(message, PLUGIN_MESSAGE_ELEMENT_FUNCTION_NAME,
				STORAGE_PLUGIN_INTERFACE_DESTROY_CONTEXT);

		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_MESSAGE_TYPE,
				(pmnumber) PLUGIN_MESSAGE_TYPE_FUNCTION);

		plugin_message_h result_message = NULL;
		ret = storage_adaptor_send_message_to_plugin_sync(plugin, message, &result_message);

		if (0 == ret) {
			pmnumber ret_code;
			plugin_message_get_value_number(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, &ret_code);

			ret = (int) ret_code;

			char *ret_msg = NULL;
			if (STORAGE_ADAPTOR_ERROR_NONE == ret) {
				storage_adaptor_debug("Destroy context successed");
			} else {
				plugin_message_get_value_string(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE, &ret_msg);
				storage_adaptor_debug("Destroy context failed (%d)(%s)", ret, ret_msg);
				free(ret_msg);
				ret_msg = NULL;

				storage_adaptor_debug("Force release memory by adaptor process");
				free(context->access_token);
				free(context);
				context = NULL;
			}
			plugin_message_destroy(result_message);
		}
	} else {
		ret = STORAGE_ADAPTOR_ERROR_PLUGIN_INTERNAL;
	}
	plugin_message_destroy(message);

	return ret;
}

storage_error_code_t storage_plugin_send_set_server_info(storage_adaptor_plugin_context_h context,
							GHashTable *server_info,
							void *request,
							storage_adaptor_error_code_h *error,
							void *response)
{
	return STORAGE_ADAPTOR_ERROR_UNSUPPORTED;
}

storage_error_code_t storage_plugin_send_make_directory(storage_adaptor_plugin_context_h context,
							const char *parent_folder_storage_path,
							const char *folder_name,
							void *request,
							storage_adaptor_file_info_h *file_info,
							storage_adaptor_error_code_h *error,
							void *response)
{
	storage_adaptor_plugin_h plugin = NULL;
	plugin = context->plugin_handle;

	int ret = 0;
	plugin_message_h message = NULL;
	ret = plugin_message_create(&message);

	if (ret == 0) {
		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_CONTEXT_ID,
				(pmnumber) context->context_id);
		plugin_message_set_value_string(message, PLUGIN_MESSAGE_ELEMENT_FUNCTION_NAME,
				STORAGE_PLUGIN_INTERFACE_MAKE_DIRECTORY);

		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_MESSAGE_TYPE,
				(pmnumber) PLUGIN_MESSAGE_TYPE_FUNCTION);

		int param_index = 1;
		plugin_message_set_param_string(message, param_index++, parent_folder_storage_path);
		plugin_message_set_param_string(message, param_index++, folder_name);

		plugin_message_h result_message = NULL;
		ret = storage_adaptor_send_message_to_plugin_sync(plugin, message, &result_message);

		if (0 == ret) {
			pmnumber ret_code;
			plugin_message_get_value_number(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, &ret_code);

			ret = (int) ret_code;
			char *ret_msg = NULL;
			if (STORAGE_ADAPTOR_ERROR_NONE == ret) {
				storage_adaptor_debug("Make directory successed");

				if (NULL != file_info) {
					storage_adaptor_debug("Get file info");
					int param_idx = 1;
					int param_ret;
					plugin_message_array_h file_info_message = NULL;
					param_ret = plugin_message_get_param_array(result_message, param_idx++, &file_info_message);
					if ((0 == param_ret) && (NULL != file_info_message)) {
						*file_info = _get_file_info_from_message_array(file_info_message, param_idx++);
					}
					plugin_message_array_destroy(file_info_message);
				}
			} else {
				plugin_message_get_value_string(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE, &ret_msg);
				storage_adaptor_debug("Make directory failed (%d)(%s)", ret, ret_msg);
				if (NULL != error) {
					storage_adaptor_error_code_h error_code = storage_adaptor_create_error_code(ret, ret_msg);
					*error = error_code;
				}
				free(ret_msg);
				ret_msg = NULL;
			}
			plugin_message_destroy(result_message);
		}
	} else {
		ret = STORAGE_ADAPTOR_ERROR_PLUGIN_INTERNAL;
	}
	plugin_message_destroy(message);

	return ret;
}


storage_error_code_t storage_plugin_send_remove_directory(storage_adaptor_plugin_context_h context,
							const char *parent_folder_storage_path,
							const char *folder_name,
							void *request,
							storage_adaptor_file_info_h *file_info,
							storage_adaptor_error_code_h *error,
							void *response)
{
	storage_adaptor_plugin_h plugin = NULL;
	plugin = context->plugin_handle;

	int ret = 0;
	plugin_message_h message = NULL;
	ret = plugin_message_create(&message);

	if (ret == 0) {
		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_CONTEXT_ID,
				(pmnumber) context->context_id);
		plugin_message_set_value_string(message, PLUGIN_MESSAGE_ELEMENT_FUNCTION_NAME,
				STORAGE_PLUGIN_INTERFACE_REMOVE_DIRECTORY);

		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_MESSAGE_TYPE,
				(pmnumber) PLUGIN_MESSAGE_TYPE_FUNCTION);

		int param_index = 1;
		plugin_message_set_param_string(message, param_index++, parent_folder_storage_path);
		plugin_message_set_param_string(message, param_index++, folder_name);

		plugin_message_h result_message = NULL;
		ret = storage_adaptor_send_message_to_plugin_sync(plugin, message, &result_message);

		if (0 == ret) {
			pmnumber ret_code;
			plugin_message_get_value_number(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, &ret_code);

			ret = (int) ret_code;
			char *ret_msg = NULL;
			if (STORAGE_ADAPTOR_ERROR_NONE == ret) {
				storage_adaptor_debug("Remove directory successed");

				if (NULL != file_info) {
					storage_adaptor_debug("Get file info");
					int param_idx = 1;
					int param_ret;
					plugin_message_array_h file_info_message = NULL;
					param_ret = plugin_message_get_param_array(result_message, param_idx++, &file_info_message);
					if ((0 == param_ret) && (NULL != file_info_message)) {
						*file_info = _get_file_info_from_message_array(file_info_message, param_idx++);
					}
					plugin_message_array_destroy(file_info_message);
				}
			} else {
				plugin_message_get_value_string(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE, &ret_msg);
				storage_adaptor_debug("Remove directory failed (%d)(%s)", ret, ret_msg);
				if (NULL != error) {
					storage_adaptor_error_code_h error_code = storage_adaptor_create_error_code(ret, ret_msg);
					*error = error_code;
				}
				free(ret_msg);
				ret_msg = NULL;
			}
			plugin_message_destroy(result_message);
		}
	} else {
		ret = STORAGE_ADAPTOR_ERROR_PLUGIN_INTERNAL;
	}
	plugin_message_destroy(message);

	return ret;
}

storage_error_code_t storage_plugin_send_get_list(storage_adaptor_plugin_context_h context,
							const char *parent_folder_storage_path,
							const char *folder_name,
							void *request,
							storage_adaptor_file_info_h **file_info_list,
							int *file_info_list_len,
							storage_adaptor_error_code_h *error,
							void *response)
{
	storage_adaptor_plugin_h plugin = NULL;
	plugin = context->plugin_handle;

	int ret = 0;
	plugin_message_h message = NULL;
	ret = plugin_message_create(&message);

	if (ret == 0) {
		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_CONTEXT_ID,
				(pmnumber) context->context_id);
		plugin_message_set_value_string(message, PLUGIN_MESSAGE_ELEMENT_FUNCTION_NAME,
				STORAGE_PLUGIN_INTERFACE_REMOVE_DIRECTORY);

		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_MESSAGE_TYPE,
				(pmnumber) PLUGIN_MESSAGE_TYPE_FUNCTION);

		int param_index = 1;
		plugin_message_set_param_string(message, param_index++, parent_folder_storage_path);
		plugin_message_set_param_string(message, param_index++, folder_name);

		plugin_message_h result_message = NULL;
		ret = storage_adaptor_send_message_to_plugin_sync(plugin, message, &result_message);

		if (0 == ret) {
			pmnumber ret_code;
			plugin_message_get_value_number(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, &ret_code);

			ret = (int) ret_code;
			char *ret_msg = NULL;
			if (STORAGE_ADAPTOR_ERROR_NONE == ret) {
				storage_adaptor_debug("Get list successed");

				int param_ret;
				pmnumber len;
				param_ret = plugin_message_get_param_number(result_message, 2, &len);
				if ((0 == ret) && (NULL != file_info_list) && (len > 0LL)) {
					storage_adaptor_debug("Get file info");
					plugin_message_array_h file_info_message = NULL;
					param_ret = plugin_message_get_param_array(result_message, 1, &file_info_message);
					if ((0 == param_ret) && (NULL != file_info_message)) {
						int i, l = (int) len;
						*file_info_list = (storage_adaptor_file_info_h *) calloc(l, sizeof(storage_adaptor_file_info_h));
						if (NULL != *file_info_list) {
							for (i = 0; i < l; i++) {
								(*file_info_list)[i] = _get_file_info_from_message_array(file_info_message, (i+1));
							}
							if (NULL != file_info_list_len) {
								*file_info_list_len = l;
							}
						}
						plugin_message_array_destroy(file_info_message);
					}
				}
			} else {
				plugin_message_get_value_string(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE, &ret_msg);
				storage_adaptor_debug("Get list failed (%d)(%s)", ret, ret_msg);
				if (NULL != error) {
					storage_adaptor_error_code_h error_code = storage_adaptor_create_error_code(ret, ret_msg);
					*error = error_code;
				}
				free(ret_msg);
				ret_msg = NULL;
			}
			plugin_message_destroy(result_message);
		}
	} else {
		ret = STORAGE_ADAPTOR_ERROR_PLUGIN_INTERNAL;
	}
	plugin_message_destroy(message);

	return ret;
}


storage_error_code_t storage_plugin_send_upload_file_sync(storage_adaptor_plugin_context_h context,
							const char *parent_folder_storage_path,
							const char *file_name,
							const char *upload_file_local_path,
							const int publish,
							void *request,
							storage_adaptor_file_info_h *file_info,
							storage_adaptor_error_code_h *error,
							void *response)
{
	storage_adaptor_plugin_h plugin = NULL;
	plugin = context->plugin_handle;

	int ret = 0;
	plugin_message_h message = NULL;
	ret = plugin_message_create(&message);

	if (ret == 0) {
		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_CONTEXT_ID,
				(pmnumber) context->context_id);
		plugin_message_set_value_string(message, PLUGIN_MESSAGE_ELEMENT_FUNCTION_NAME,
				STORAGE_PLUGIN_INTERFACE_UPLOAD_FILE_SYNC);

		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_MESSAGE_TYPE,
				(pmnumber) PLUGIN_MESSAGE_TYPE_FUNCTION);

		int param_index = 1;
		plugin_message_set_param_string(message, param_index++, parent_folder_storage_path);
		plugin_message_set_param_string(message, param_index++, file_name);
		plugin_message_set_param_string(message, param_index++, upload_file_local_path);
		plugin_message_set_param_number(message, param_index++, (pmnumber) publish);

		plugin_message_h result_message = NULL;
		ret = storage_adaptor_send_message_to_plugin_sync(plugin, message, &result_message);

		if (0 == ret) {
			pmnumber ret_code;
			plugin_message_get_value_number(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, &ret_code);

			ret = (int) ret_code;
			char *ret_msg = NULL;
			if (STORAGE_ADAPTOR_ERROR_NONE == ret) {
				storage_adaptor_debug("Upload sync successed");

				if (NULL != file_info) {
					storage_adaptor_debug("Get file info");
					int param_idx = 1;
					int param_ret;
					plugin_message_array_h file_info_message = NULL;
					param_ret = plugin_message_get_param_array(result_message, param_idx++, &file_info_message);
					if ((0 == param_ret) && (NULL != file_info_message)) {
						*file_info = _get_file_info_from_message_array(file_info_message, param_idx++);
					}
					plugin_message_array_destroy(file_info_message);
				}
			} else {
				plugin_message_get_value_string(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE, &ret_msg);
				storage_adaptor_debug("Upload sync failed (%d)(%s)", ret, ret_msg);
				if (NULL != error) {
					storage_adaptor_error_code_h error_code = storage_adaptor_create_error_code(ret, ret_msg);
					*error = error_code;
				}
				free(ret_msg);
				ret_msg = NULL;
			}
			plugin_message_destroy(result_message);
		}
	} else {
		ret = STORAGE_ADAPTOR_ERROR_PLUGIN_INTERNAL;
	}
	plugin_message_destroy(message);

	return ret;
}


storage_error_code_t storage_plugin_send_download_file_sync(storage_adaptor_plugin_context_h context,
							const char *parent_folder_storage_path,
							const char *file_name,
							const char *download_file_local_path,
							void *request,
							storage_adaptor_error_code_h *error,
							void *response)
{
	storage_adaptor_plugin_h plugin = NULL;
	plugin = context->plugin_handle;

	int ret = 0;
	plugin_message_h message = NULL;
	ret = plugin_message_create(&message);

	if (ret == 0) {
		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_CONTEXT_ID,
				(pmnumber) context->context_id);
		plugin_message_set_value_string(message, PLUGIN_MESSAGE_ELEMENT_FUNCTION_NAME,
				STORAGE_PLUGIN_INTERFACE_DOWNLOAD_FILE_SYNC);

		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_MESSAGE_TYPE,
				(pmnumber) PLUGIN_MESSAGE_TYPE_FUNCTION);

		int param_index = 1;
		plugin_message_set_param_string(message, param_index++, parent_folder_storage_path);
		plugin_message_set_param_string(message, param_index++, file_name);
		plugin_message_set_param_string(message, param_index++, download_file_local_path);

		plugin_message_h result_message = NULL;
		ret = storage_adaptor_send_message_to_plugin_sync(plugin, message, &result_message);

		if (0 == ret) {
			pmnumber ret_code;
			plugin_message_get_value_number(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, &ret_code);

			ret = (int) ret_code;
			char *ret_msg = NULL;
			if (STORAGE_ADAPTOR_ERROR_NONE == ret) {
				storage_adaptor_debug("Download sync successed");
			} else {
				plugin_message_get_value_string(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE, &ret_msg);
				storage_adaptor_debug("Download sync failed (%d)(%s)", ret, ret_msg);
				if (NULL != error) {
					storage_adaptor_error_code_h error_code = storage_adaptor_create_error_code(ret, ret_msg);
					*error = error_code;
				}
				free(ret_msg);
				ret_msg = NULL;
			}
			plugin_message_destroy(result_message);
		}
	} else {
		ret = STORAGE_ADAPTOR_ERROR_PLUGIN_INTERNAL;
	}
	plugin_message_destroy(message);

	return ret;
}

storage_error_code_t storage_plugin_send_delete_file(storage_adaptor_plugin_context_h context,
							const char *parent_folder_storage_path,
							const char *file_name,
							void *request,
							storage_adaptor_file_info_h *file_info,
							storage_adaptor_error_code_h *error,
							void *response)
{
	storage_adaptor_plugin_h plugin = NULL;
	plugin = context->plugin_handle;

	int ret = 0;
	plugin_message_h message = NULL;
	ret = plugin_message_create(&message);

	if (ret == 0) {
		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_CONTEXT_ID,
				(pmnumber) context->context_id);
		plugin_message_set_value_string(message, PLUGIN_MESSAGE_ELEMENT_FUNCTION_NAME,
				STORAGE_PLUGIN_INTERFACE_DELETE_FILE);

		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_MESSAGE_TYPE,
				(pmnumber) PLUGIN_MESSAGE_TYPE_FUNCTION);

		int param_index = 1;
		plugin_message_set_param_string(message, param_index++, parent_folder_storage_path);
		plugin_message_set_param_string(message, param_index++, file_name);

		plugin_message_h result_message = NULL;
		ret = storage_adaptor_send_message_to_plugin_sync(plugin, message, &result_message);

		if (0 == ret) {
			pmnumber ret_code;
			plugin_message_get_value_number(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, &ret_code);

			ret = (int) ret_code;
			char *ret_msg = NULL;
			if (STORAGE_ADAPTOR_ERROR_NONE == ret) {
				storage_adaptor_debug("Delete file successed");

				if (NULL != file_info) {
					storage_adaptor_debug("Get file info");
					int param_idx = 1;
					int param_ret;
					plugin_message_array_h file_info_message = NULL;
					param_ret = plugin_message_get_param_array(result_message, param_idx++, &file_info_message);
					if ((0 == param_ret) && (NULL != file_info_message)) {
						*file_info = _get_file_info_from_message_array(file_info_message, param_idx++);
					}
					plugin_message_array_destroy(file_info_message);
				}
			} else {
				plugin_message_get_value_string(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE, &ret_msg);
				storage_adaptor_debug("Delete file failed (%d)(%s)", ret, ret_msg);
				if (NULL != error) {
					storage_adaptor_error_code_h error_code = storage_adaptor_create_error_code(ret, ret_msg);
					*error = error_code;
				}
				free(ret_msg);
				ret_msg = NULL;
			}
			plugin_message_destroy(result_message);
		}
	} else {
		ret = STORAGE_ADAPTOR_ERROR_PLUGIN_INTERNAL;
	}
	plugin_message_destroy(message);

	return ret;
}

storage_error_code_t storage_plugin_send_move_directory(storage_adaptor_plugin_context_h context,
							const char *parent_folder_storage_path,
							const char *folder_name,
							const char *dest_parent_folder_storage_path,
							const char *new_folder_name,
							void *request,
							storage_adaptor_file_info_h *file_info,
							storage_adaptor_error_code_h *error,
							void *response)
{
	storage_adaptor_plugin_h plugin = NULL;
	plugin = context->plugin_handle;

	int ret = 0;
	plugin_message_h message = NULL;
	ret = plugin_message_create(&message);

	if (ret == 0) {
		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_CONTEXT_ID,
				(pmnumber) context->context_id);
		plugin_message_set_value_string(message, PLUGIN_MESSAGE_ELEMENT_FUNCTION_NAME,
				STORAGE_PLUGIN_INTERFACE_MOVE_DIRECTORY);

		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_MESSAGE_TYPE,
				(pmnumber) PLUGIN_MESSAGE_TYPE_FUNCTION);

		int param_index = 1;
		plugin_message_set_param_string(message, param_index++, parent_folder_storage_path);
		plugin_message_set_param_string(message, param_index++, folder_name);
		plugin_message_set_param_string(message, param_index++, dest_parent_folder_storage_path);
		plugin_message_set_param_string(message, param_index++, new_folder_name);

		plugin_message_h result_message = NULL;
		ret = storage_adaptor_send_message_to_plugin_sync(plugin, message, &result_message);

		if (0 == ret) {
			pmnumber ret_code;
			plugin_message_get_value_number(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, &ret_code);

			ret = (int) ret_code;
			char *ret_msg = NULL;
			if (STORAGE_ADAPTOR_ERROR_NONE == ret) {
				storage_adaptor_debug("Move directory successed");

				if (NULL != file_info) {
					storage_adaptor_debug("Get file info");
					int param_idx = 1;
					int param_ret;
					plugin_message_array_h file_info_message = NULL;
					param_ret = plugin_message_get_param_array(result_message, param_idx++, &file_info_message);
					if ((0 == param_ret) && (NULL != file_info_message)) {
						*file_info = _get_file_info_from_message_array(file_info_message, param_idx++);
					}
					plugin_message_array_destroy(file_info_message);
				}
			} else {
				plugin_message_get_value_string(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE, &ret_msg);
				storage_adaptor_debug("Move directory failed (%d)(%s)", ret, ret_msg);
				if (NULL != error) {
					storage_adaptor_error_code_h error_code = storage_adaptor_create_error_code(ret, ret_msg);
					*error = error_code;
				}
				free(ret_msg);
				ret_msg = NULL;
			}
			plugin_message_destroy(result_message);
		}
	} else {
		ret = STORAGE_ADAPTOR_ERROR_PLUGIN_INTERNAL;
	}
	plugin_message_destroy(message);

	return ret;
}

storage_error_code_t storage_plugin_send_move_file(storage_adaptor_plugin_context_h context,
							const char *parent_folder_storage_path,
							const char *file_name,
							const char *dest_parent_folder_storage_path,
							const char *new_file_name,
							void *request,
							storage_adaptor_file_info_h *file_info,
							storage_adaptor_error_code_h *error,
							void *response)
{
	storage_adaptor_plugin_h plugin = NULL;
	plugin = context->plugin_handle;

	int ret = 0;
	plugin_message_h message = NULL;
	ret = plugin_message_create(&message);

	if (ret == 0) {
		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_CONTEXT_ID,
				(pmnumber) context->context_id);
		plugin_message_set_value_string(message, PLUGIN_MESSAGE_ELEMENT_FUNCTION_NAME,
				STORAGE_PLUGIN_INTERFACE_MOVE_FILE);

		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_MESSAGE_TYPE,
				(pmnumber) PLUGIN_MESSAGE_TYPE_FUNCTION);

		int param_index = 1;
		plugin_message_set_param_string(message, param_index++, parent_folder_storage_path);
		plugin_message_set_param_string(message, param_index++, file_name);
		plugin_message_set_param_string(message, param_index++, dest_parent_folder_storage_path);
		plugin_message_set_param_string(message, param_index++, new_file_name);

		plugin_message_h result_message = NULL;
		ret = storage_adaptor_send_message_to_plugin_sync(plugin, message, &result_message);

		if (0 == ret) {
			pmnumber ret_code;
			plugin_message_get_value_number(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, &ret_code);

			ret = (int) ret_code;
			char *ret_msg = NULL;
			if (STORAGE_ADAPTOR_ERROR_NONE == ret) {
				storage_adaptor_debug("Move file successed");

				if (NULL != file_info) {
					storage_adaptor_debug("Get file info");
					int param_idx = 1;
					int param_ret;
					plugin_message_array_h file_info_message = NULL;
					param_ret = plugin_message_get_param_array(result_message, param_idx++, &file_info_message);
					if ((0 == param_ret) && (NULL != file_info_message)) {
						*file_info = _get_file_info_from_message_array(file_info_message, param_idx++);
					}
					plugin_message_array_destroy(file_info_message);
				}
			} else {
				plugin_message_get_value_string(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE, &ret_msg);
				storage_adaptor_debug("Move file failed (%d)(%s)", ret, ret_msg);
				if (NULL != error) {
					storage_adaptor_error_code_h error_code = storage_adaptor_create_error_code(ret, ret_msg);
					*error = error_code;
				}
				free(ret_msg);
				ret_msg = NULL;
			}
			plugin_message_destroy(result_message);
		}
	} else {
		ret = STORAGE_ADAPTOR_ERROR_PLUGIN_INTERNAL;
	}
	plugin_message_destroy(message);

	return ret;
}

storage_error_code_t storage_plugin_send_set_transfer_state(storage_adaptor_plugin_context_h context,
							void *transfer_request_id,
							storage_adaptor_transfer_state_e state,
							void *request,
							storage_adaptor_error_code_h *error,
							void *response)
{
	storage_adaptor_plugin_h plugin = NULL;
	plugin = context->plugin_handle;

	int ret = 0;
	plugin_message_h message = NULL;
	ret = plugin_message_create(&message);

	if (ret == 0) {
		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_CONTEXT_ID,
				(pmnumber) context->context_id);
		plugin_message_set_value_string(message, PLUGIN_MESSAGE_ELEMENT_FUNCTION_NAME,
				STORAGE_PLUGIN_INTERFACE_SET_TRANSFER_STATE);
		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_MESSAGE_TYPE,
				(pmnumber) PLUGIN_MESSAGE_TYPE_FUNCTION);

		int param_index = 1;
		plugin_message_set_param_number(message, param_index++, (pmnumber)(intptr_t)transfer_request_id);
		plugin_message_set_param_number(message, param_index++, (pmnumber)state);

		plugin_message_h result_message = NULL;
		ret = storage_adaptor_send_message_to_plugin_sync(plugin, message, &result_message);

		if (0 == ret) {
			pmnumber ret_code;
			plugin_message_get_value_number(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, &ret_code);

			ret = (int) ret_code;
			char *ret_msg = NULL;
			if (STORAGE_ADAPTOR_ERROR_NONE == ret) {
				storage_adaptor_debug("Set transfer state successed");
			} else {
				plugin_message_get_value_string(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE, &ret_msg);
				storage_adaptor_debug("Set transfer state failed (%d)(%s)", ret, ret_msg);
				if (NULL != error) {
					storage_adaptor_error_code_h error_code = storage_adaptor_create_error_code(ret, ret_msg);
					*error = error_code;
				}
				free(ret_msg);
				ret_msg = NULL;
			}
			plugin_message_destroy(result_message);
		}
	} else {
		ret = STORAGE_ADAPTOR_ERROR_PLUGIN_INTERNAL;
	}
	plugin_message_destroy(message);

	return ret;
}

storage_error_code_t storage_plugin_send_get_transfer_state(storage_adaptor_plugin_context_h context,
							void *transfer_request_id,
							void *request,
							storage_adaptor_transfer_state_e *state,
							storage_adaptor_error_code_h *error,
							void *response)
{
	storage_adaptor_plugin_h plugin = NULL;
	plugin = context->plugin_handle;

	int ret = 0;
	plugin_message_h message = NULL;
	ret = plugin_message_create(&message);

	if (ret == 0) {
		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_CONTEXT_ID,
				(pmnumber) context->context_id);
		plugin_message_set_value_string(message, PLUGIN_MESSAGE_ELEMENT_FUNCTION_NAME,
				STORAGE_PLUGIN_INTERFACE_GET_TRANSFER_STATE);
		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_MESSAGE_TYPE,
				(pmnumber) PLUGIN_MESSAGE_TYPE_FUNCTION);

		int param_index = 1;
		plugin_message_set_param_number(message, param_index++, (pmnumber)(intptr_t)transfer_request_id);

		plugin_message_h result_message = NULL;
		ret = storage_adaptor_send_message_to_plugin_sync(plugin, message, &result_message);

		if (0 == ret) {
			pmnumber ret_code;
			plugin_message_get_value_number(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, &ret_code);

			ret = (int) ret_code;
			char *ret_msg = NULL;
			if (STORAGE_ADAPTOR_ERROR_NONE == ret) {
				storage_adaptor_debug("Get transfer state successed");
				pmnumber st;
				plugin_message_get_param_number(message, 1, &st);
				if (NULL != state) {
					*state = (storage_adaptor_transfer_state_e)st;
				}
			} else {
				plugin_message_get_value_string(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE, &ret_msg);
				storage_adaptor_debug("Get transfer state failed (%d)(%s)", ret, ret_msg);
				if (NULL != error) {
					storage_adaptor_error_code_h error_code = storage_adaptor_create_error_code(ret, ret_msg);
					*error = error_code;
				}
				free(ret_msg);
				ret_msg = NULL;
			}
			plugin_message_destroy(result_message);
		}
	} else {
		ret = STORAGE_ADAPTOR_ERROR_PLUGIN_INTERNAL;
	}
	plugin_message_destroy(message);

	return ret;
}

storage_error_code_t storage_plugin_send_get_root_folder_path(storage_adaptor_plugin_context_h context,
							void *request,
							char **root_folder_path,
							storage_adaptor_error_code_h *error,
							void *response)
{
	storage_adaptor_plugin_h plugin = NULL;
	plugin = context->plugin_handle;

	int ret = 0;
	plugin_message_h message = NULL;
	ret = plugin_message_create(&message);

	if (ret == 0) {
		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_CONTEXT_ID,
				(pmnumber) context->context_id);
		plugin_message_set_value_string(message, PLUGIN_MESSAGE_ELEMENT_FUNCTION_NAME,
				STORAGE_PLUGIN_INTERFACE_GET_ROOT_FOLDER_PATH);
		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_MESSAGE_TYPE,
				(pmnumber) PLUGIN_MESSAGE_TYPE_FUNCTION);

/*		int param_index = 1; */

		plugin_message_h result_message = NULL;
		ret = storage_adaptor_send_message_to_plugin_sync(plugin, message, &result_message);

		if (0 == ret) {
			pmnumber ret_code;
			plugin_message_get_value_number(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, &ret_code);

			ret = (int) ret_code;
			char *ret_msg = NULL;
			if (STORAGE_ADAPTOR_ERROR_NONE == ret) {
				storage_adaptor_debug("Get root folder path successed");
				char *path = NULL;
				plugin_message_get_param_string(message, 1, &path);
				if (NULL != root_folder_path) {
					*root_folder_path = path;
				}
			} else {
				plugin_message_get_value_string(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE, &ret_msg);
				storage_adaptor_debug("Get root folder path failed (%d)(%s)", ret, ret_msg);
				if (NULL != error) {
					storage_adaptor_error_code_h error_code = storage_adaptor_create_error_code(ret, ret_msg);
					*error = error_code;
				}
				free(ret_msg);
				ret_msg = NULL;
			}
			plugin_message_destroy(result_message);
		}
	} else {
		ret = STORAGE_ADAPTOR_ERROR_PLUGIN_INTERNAL;
	}
	plugin_message_destroy(message);

	return ret;
}

/* ///////////// for 2.4 public API */
storage_error_code_t storage_plugin_send_start_upload_task(storage_adaptor_plugin_context_h context,
							int fd,
							const char *upload_dir,
							const char *file_path,
							bool need_progress,
							storage_adaptor_error_code_h *error,
							void *user_data)
{
	storage_adaptor_plugin_h plugin = NULL;
	plugin = context->plugin_handle;

	int ret = 0;
	plugin_message_h message = NULL;
	ret = plugin_message_create(&message);

	if (ret == 0) {
		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_CONTEXT_ID,
				(pmnumber) context->context_id);
		plugin_message_set_value_string(message, PLUGIN_MESSAGE_ELEMENT_FUNCTION_NAME,
				STORAGE_PLUGIN_INTERFACE_START_UPLOAD_TASK);

		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_MESSAGE_TYPE,
				(pmnumber) PLUGIN_MESSAGE_TYPE_FUNCTION);

		int param_index = 1;
		plugin_message_set_param_number(message, param_index++, (pmnumber)fd);
		plugin_message_set_param_string(message, param_index++, upload_dir);
		plugin_message_set_param_string(message, param_index++, file_path);
		plugin_message_set_param_bool(message, param_index++, need_progress);

		param_index = 1;
		plugin_message_set_opt_param_number(message, param_index++, (pmnumber)(intptr_t)user_data);

		plugin_message_h result_message = NULL;
		ret = storage_adaptor_send_message_to_plugin_sync(plugin, message, &result_message);

		if ((0 == ret) && (NULL != result_message)) {
			pmnumber ret_code;
			plugin_message_get_value_number(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, &ret_code);

			ret = (int) ret_code;
			char *ret_msg = NULL;
			if (STORAGE_ADAPTOR_ERROR_NONE == ret) {
				storage_adaptor_debug("Upload file async successed");
			} else {
				plugin_message_get_value_string(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE, &ret_msg);
				storage_adaptor_debug("Upload file async failed (%d)(%s)", ret, ret_msg);
				if (NULL != error) {
					storage_adaptor_error_code_h error_code = storage_adaptor_create_error_code(ret, ret_msg);
					*error = error_code;
				}
				free(ret_msg);
				ret_msg = NULL;
			}
			plugin_message_destroy(result_message);
		}
	} else {
		ret = STORAGE_ADAPTOR_ERROR_PLUGIN_INTERNAL;
	}
	plugin_message_destroy(message);

	return ret;
}

storage_error_code_t storage_plugin_send_start_download_task(storage_adaptor_plugin_context_h context,
							const char *storage_dir,
							const char *file_path,
							int fd,
							bool need_progress,
							storage_adaptor_error_code_h *error,
							void *user_data)
{
	storage_adaptor_plugin_h plugin = NULL;
	plugin = context->plugin_handle;

	int ret = 0;
	plugin_message_h message = NULL;
	ret = plugin_message_create(&message);

	if (ret == 0) {
		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_CONTEXT_ID,
				(pmnumber) context->context_id);
		plugin_message_set_value_string(message, PLUGIN_MESSAGE_ELEMENT_FUNCTION_NAME,
				STORAGE_PLUGIN_INTERFACE_START_DOWNLOAD_TASK);

		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_MESSAGE_TYPE,
				(pmnumber) PLUGIN_MESSAGE_TYPE_FUNCTION);

		int param_index = 1;
		plugin_message_set_param_string(message, param_index++, storage_dir);
		plugin_message_set_param_string(message, param_index++, file_path);
		plugin_message_set_param_number(message, param_index++, (pmnumber)fd);
		plugin_message_set_param_bool(message, param_index++, need_progress);

		param_index = 1;
		plugin_message_set_opt_param_number(message, param_index++, (pmnumber)(intptr_t)user_data);

		plugin_message_h result_message = NULL;
		ret = storage_adaptor_send_message_to_plugin_sync(plugin, message, &result_message);

		if ((0 == ret) && (NULL != result_message)) {
			pmnumber ret_code;
			plugin_message_get_value_number(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, &ret_code);

			ret = (int) ret_code;
			char *ret_msg = NULL;
			if (STORAGE_ADAPTOR_ERROR_NONE == ret) {
				storage_adaptor_debug("Download file async successed");
			} else {
				plugin_message_get_value_string(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE, &ret_msg);
				storage_adaptor_debug("Download file async failed (%d)(%s)", ret, ret_msg);
				if (NULL != error) {
					storage_adaptor_error_code_h error_code = storage_adaptor_create_error_code(ret, ret_msg);
					*error = error_code;
				}
				free(ret_msg);
				ret_msg = NULL;
			}
			plugin_message_destroy(result_message);
		}
	} else {
		ret = STORAGE_ADAPTOR_ERROR_PLUGIN_INTERNAL;
	}
	plugin_message_destroy(message);

	return ret;
}

storage_error_code_t storage_plugin_send_start_download_thumb_task(storage_adaptor_plugin_context_h context,
							const char *storage_dir,
							const char *file_path,
							int fd,
							int thumbnail_size,
							bool need_progress,
							storage_adaptor_error_code_h *error,
							void *user_data)
{
	storage_adaptor_plugin_h plugin = NULL;
	plugin = context->plugin_handle;

	int ret = 0;
	plugin_message_h message = NULL;
	ret = plugin_message_create(&message);

	if (ret == 0) {
		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_CONTEXT_ID,
				(pmnumber) context->context_id);
		plugin_message_set_value_string(message, PLUGIN_MESSAGE_ELEMENT_FUNCTION_NAME,
				STORAGE_PLUGIN_INTERFACE_START_DOWNLOAD_THUMB_TASK);

		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_MESSAGE_TYPE,
				(pmnumber) PLUGIN_MESSAGE_TYPE_FUNCTION);

		int param_index = 1;
		plugin_message_set_param_string(message, param_index++, storage_dir);
		plugin_message_set_param_string(message, param_index++, file_path);
		plugin_message_set_param_number(message, param_index++, (pmnumber)fd);
		plugin_message_set_param_number(message, param_index++, (pmnumber)thumbnail_size);
		plugin_message_set_param_bool(message, param_index++, need_progress);

		param_index = 1;
		plugin_message_set_opt_param_number(message, param_index++, (pmnumber)(intptr_t)user_data);

		plugin_message_h result_message = NULL;
		ret = storage_adaptor_send_message_to_plugin_sync(plugin, message, &result_message);

		if ((0 == ret) && (NULL != result_message)) {
			pmnumber ret_code;
			plugin_message_get_value_number(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, &ret_code);

			ret = (int) ret_code;
			char *ret_msg = NULL;
			if (STORAGE_ADAPTOR_ERROR_NONE == ret) {
				storage_adaptor_debug("Download thumbnail async successed");
			} else {
				plugin_message_get_value_string(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE, &ret_msg);
				storage_adaptor_debug("Download thumbnail async failed (%d)(%s)", ret, ret_msg);
				if (NULL != error) {
					storage_adaptor_error_code_h error_code = storage_adaptor_create_error_code(ret, ret_msg);
					*error = error_code;
				}
				free(ret_msg);
				ret_msg = NULL;
			}
			plugin_message_destroy(result_message);
		}
	} else {
		ret = STORAGE_ADAPTOR_ERROR_PLUGIN_INTERNAL;
	}
	plugin_message_destroy(message);

	return ret;
}

storage_error_code_t storage_plugin_send_cancel_upload_task(storage_adaptor_plugin_context_h context,
							int fd,
							storage_adaptor_error_code_h *error)
{
	storage_adaptor_plugin_h plugin = NULL;
	plugin = context->plugin_handle;

	int ret = 0;
	plugin_message_h message = NULL;
	ret = plugin_message_create(&message);

	if (ret == 0) {
		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_CONTEXT_ID,
				(pmnumber) context->context_id);
		plugin_message_set_value_string(message, PLUGIN_MESSAGE_ELEMENT_FUNCTION_NAME,
				STORAGE_PLUGIN_INTERFACE_CANCEL_UPLOAD_TASK);

		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_MESSAGE_TYPE,
				(pmnumber) PLUGIN_MESSAGE_TYPE_FUNCTION);

		int param_index = 1;
		plugin_message_set_param_number(message, param_index++, (pmnumber)fd);

		plugin_message_h result_message = NULL;
		ret = storage_adaptor_send_message_to_plugin_sync(plugin, message, &result_message);

		if ((0 == ret) && (NULL != result_message)) {
			pmnumber ret_code;
			plugin_message_get_value_number(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, &ret_code);

			ret = (int) ret_code;
			char *ret_msg = NULL;
			if (STORAGE_ADAPTOR_ERROR_NONE == ret) {
				storage_adaptor_debug("Upload file async successed");
			} else {
				plugin_message_get_value_string(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE, &ret_msg);
				storage_adaptor_debug("Upload file async failed (%d)(%s)", ret, ret_msg);
				if (NULL != error) {
					storage_adaptor_error_code_h error_code = storage_adaptor_create_error_code(ret, ret_msg);
					*error = error_code;
				}
				free(ret_msg);
				ret_msg = NULL;
			}
			plugin_message_destroy(result_message);
		}
	} else {
		ret = STORAGE_ADAPTOR_ERROR_PLUGIN_INTERNAL;
	}
	plugin_message_destroy(message);

	return ret;
}

storage_error_code_t storage_plugin_send_cancel_download_task(storage_adaptor_plugin_context_h context,
							int fd,
							storage_adaptor_error_code_h *error)
{
	storage_adaptor_plugin_h plugin = NULL;
	plugin = context->plugin_handle;

	int ret = 0;
	plugin_message_h message = NULL;
	ret = plugin_message_create(&message);

	if (ret == 0) {
		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_CONTEXT_ID,
				(pmnumber) context->context_id);
		plugin_message_set_value_string(message, PLUGIN_MESSAGE_ELEMENT_FUNCTION_NAME,
				STORAGE_PLUGIN_INTERFACE_CANCEL_DOWNLOAD_TASK);

		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_MESSAGE_TYPE,
				(pmnumber) PLUGIN_MESSAGE_TYPE_FUNCTION);

		int param_index = 1;
		plugin_message_set_param_number(message, param_index++, (pmnumber)fd);

		plugin_message_h result_message = NULL;
		ret = storage_adaptor_send_message_to_plugin_sync(plugin, message, &result_message);

		if ((0 == ret) && (NULL != result_message)) {
			pmnumber ret_code;
			plugin_message_get_value_number(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, &ret_code);

			ret = (int) ret_code;
			char *ret_msg = NULL;
			if (STORAGE_ADAPTOR_ERROR_NONE == ret) {
				storage_adaptor_debug("Cancel upload file successed");
			} else {
				plugin_message_get_value_string(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE, &ret_msg);
				storage_adaptor_debug("Cancel upload file failed (%d)(%s)", ret, ret_msg);
				if (NULL != error) {
					storage_adaptor_error_code_h error_code = storage_adaptor_create_error_code(ret, ret_msg);
					*error = error_code;
				}
				free(ret_msg);
				ret_msg = NULL;
			}
			plugin_message_destroy(result_message);
		}
	} else {
		ret = STORAGE_ADAPTOR_ERROR_PLUGIN_INTERNAL;
	}
	plugin_message_destroy(message);

	return ret;
}

storage_error_code_t storage_plugin_send_cancel_download_thumb_task(storage_adaptor_plugin_context_h context,
							int fd,
							storage_adaptor_error_code_h *error)
{
	storage_adaptor_plugin_h plugin = NULL;
	plugin = context->plugin_handle;

	int ret = 0;
	plugin_message_h message = NULL;
	ret = plugin_message_create(&message);

	if (ret == 0) {
		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_CONTEXT_ID,
				(pmnumber) context->context_id);
		plugin_message_set_value_string(message, PLUGIN_MESSAGE_ELEMENT_FUNCTION_NAME,
				STORAGE_PLUGIN_INTERFACE_CANCEL_DOWNLOAD_THUMB_TASK);

		plugin_message_set_value_number(message, PLUGIN_MESSAGE_ELEMENT_MESSAGE_TYPE,
				(pmnumber) PLUGIN_MESSAGE_TYPE_FUNCTION);

		int param_index = 1;
		plugin_message_set_param_number(message, param_index++, (pmnumber)fd);

		plugin_message_h result_message = NULL;
		ret = storage_adaptor_send_message_to_plugin_sync(plugin, message, &result_message);

		if ((0 == ret) && (NULL != result_message)) {
			pmnumber ret_code;
			plugin_message_get_value_number(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, &ret_code);

			ret = (int) ret_code;
			char *ret_msg = NULL;
			if (STORAGE_ADAPTOR_ERROR_NONE == ret) {
				storage_adaptor_debug("Cancel download thumbnail successed");
			} else {
				plugin_message_get_value_string(result_message, PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE, &ret_msg);
				storage_adaptor_debug("Cancel download thumbnail failed (%d)(%s)", ret, ret_msg);
				if (NULL != error) {
					storage_adaptor_error_code_h error_code = storage_adaptor_create_error_code(ret, ret_msg);
					*error = error_code;
				}
				free(ret_msg);
				ret_msg = NULL;
			}
			plugin_message_destroy(result_message);
		}
	} else {
		ret = STORAGE_ADAPTOR_ERROR_PLUGIN_INTERNAL;
	}
	plugin_message_destroy(message);

	return ret;
}

void storage_plugin_receive_download_state_changed_cb(int file_descriptor,
						storage_adaptor_transfer_state_e state,
						storage_adaptor_error_code_h error,
						void *user_data)
{
	int ret = 0;
	plugin_message_h m_callback = NULL;

	if ((0 != plugin_message_create(&m_callback)) || (NULL == m_callback)) {
		LOGE("[%s/%d] Callback message create failed", __FUNCTION__, __LINE__);
		return;
	}

	const char *func_name = STORAGE_PLUGIN_CALLBACK_DOWNLOAD_FILE_ASYNC_CB;

	plugin_message_set_value_string(m_callback, PLUGIN_MESSAGE_ELEMENT_FUNCTION_NAME, func_name);
	plugin_message_set_value_number(m_callback, PLUGIN_MESSAGE_ELEMENT_MESSAGE_TYPE, PLUGIN_MESSAGE_TYPE_CALLBACK);

	if (NULL == error) {
		plugin_message_set_value_number(m_callback, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, (pmnumber)STORAGE_ADAPTOR_ERROR_NONE);
	} else {
		LOGD("[%s/%d] Callback's error value (%lld, %s)", __FUNCTION__, __LINE__, (long long int)error->code, error->msg);
		plugin_message_set_value_number(m_callback, PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE, (pmnumber)error->code);
		plugin_message_set_value_string(m_callback, PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE, error->msg);
	}

	int param_idx = 1;
	plugin_message_set_param_number(m_callback, param_idx++, (pmnumber)file_descriptor);
	plugin_message_set_param_number(m_callback, param_idx++, (pmnumber)state);

	char *result = NULL;
	int rcv_len;

	ret = plugin_message_serialize(m_callback, &result);

	if ((0 == ret) && (NULL != result)) {
		LOGD("[%s/%d] Send callback data message to adaptor", __FUNCTION__, __LINE__);
		int res_len = strlen(result);
		rcv_len = write(g_child_plugin->wd, &res_len, sizeof(int));
		rcv_len = write(g_child_plugin->wd, result, sizeof(char) * res_len);
		if (rcv_len <= 0) {
			LOGE("[%s/%d] pipe socket writing error", __FUNCTION__, __LINE__);
		}
	} else {
		LOGE("[%s/%d] Callback data message serialization failed", __FUNCTION__, __LINE__);
	}

	plugin_message_destroy(m_callback);
}

void storage_plugin_receive_upload_state_changed_cb(int file_descriptor,
						storage_adaptor_transfer_state_e state,
						storage_adaptor_file_info_h file_info,
						storage_adaptor_error_code_h error,
						void *user_data)
{
	int ret = 0;
	plugin_message_h m_callback = NULL;

	if ((0 != plugin_message_create(&m_callback)) || (NULL == m_callback)) {
		LOGE("[%s/%d] Callback message create failed", __FUNCTION__, __LINE__);
		return;
	}

	const char *func_name = STORAGE_PLUGIN_CALLBACK_UPLOAD_FILE_ASYNC_CB;

	plugin_message_set_value_string(m_callback, PLUGIN_MESSAGE_ELEMENT_FUNCTION_NAME, func_name);
	plugin_message_set_value_number(m_callback, PLUGIN_MESSAGE_ELEMENT_MESSAGE_TYPE, PLUGIN_MESSAGE_TYPE_CALLBACK);

	if (NULL == error) {
		plugin_message_set_value_number(m_callback, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, (pmnumber)STORAGE_ADAPTOR_ERROR_NONE);
	} else {
		LOGD("[%s/%d] Callback's error value (%lld, %s)", __FUNCTION__, __LINE__, (long long int)error->code, error->msg);
		plugin_message_set_value_number(m_callback, PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE, (pmnumber)error->code);
		plugin_message_set_value_string(m_callback, PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE, error->msg);
	}

	int param_idx = 1;
	plugin_message_set_param_number(m_callback, param_idx++, (pmnumber)file_descriptor);
	plugin_message_set_param_number(m_callback, param_idx++, (pmnumber)state);

	if (NULL != file_info) {
		LOGD("Insert file info to pipe message");
		int param_ret;
		plugin_message_array_h file_info_message = NULL;
		char message_array_type[20] = {0, };
		snprintf(message_array_type, 19, "%c%c%c%c%c%c%c%c%c", PLUGIN_DATA_TYPE_STRING,
							PLUGIN_DATA_TYPE_STRING,
							PLUGIN_DATA_TYPE_STRING,
							PLUGIN_DATA_TYPE_NUM,
							PLUGIN_DATA_TYPE_NUM,
							PLUGIN_DATA_TYPE_NUM,
							PLUGIN_DATA_TYPE_NUM,
							PLUGIN_DATA_TYPE_NUM,
							PLUGIN_DATA_TYPE_STRING);
		param_ret = plugin_message_array_create(message_array_type, &file_info_message);
		if (0 == param_ret) {
			_message_array_set_file_info(file_info_message, 1, file_info);
			param_ret = plugin_message_set_param_array(m_callback, param_idx++, file_info_message);
			plugin_message_array_destroy(file_info_message);
		}
	}

	char *result = NULL;

	ret = plugin_message_serialize(m_callback, &result);

	if ((0 == ret) && (NULL != result)) {
		LOGD("[%s/%d] Send callback data message to adaptor", __FUNCTION__, __LINE__);
		int res_len = strlen(result);
		int rcv_len = write(g_child_plugin->wd, &res_len, sizeof(int));
		rcv_len = write(g_child_plugin->wd, result, sizeof(char) * res_len);
		if (rcv_len <= 0) {
			LOGE("[%s/%d] pipe socket writing error", __FUNCTION__, __LINE__);
		}
	} else {
		LOGE("[%s/%d] Callback data message serialization failed", __FUNCTION__, __LINE__);
	}

	plugin_message_destroy(m_callback);

}

void storage_plugin_receive_file_progress_cb(int file_descriptor,
						unsigned long long progress_size_byte,
						unsigned long long total_size_byte,
						storage_adaptor_error_code_h error,
						void *user_data)
{
	int ret = 0;
	plugin_message_h m_callback = NULL;

	if ((0 != plugin_message_create(&m_callback)) || (NULL == m_callback)) {
		LOGE("[%s/%d] Callback message create failed", __FUNCTION__, __LINE__);
		return;
	}

	const char *func_name = STORAGE_PLUGIN_CALLBACK_PROGRESS_CB;

	plugin_message_set_value_string(m_callback, PLUGIN_MESSAGE_ELEMENT_FUNCTION_NAME, func_name);
	plugin_message_set_value_number(m_callback, PLUGIN_MESSAGE_ELEMENT_MESSAGE_TYPE, PLUGIN_MESSAGE_TYPE_CALLBACK);

	if (NULL == error) {
		plugin_message_set_value_number(m_callback, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, (pmnumber)STORAGE_ADAPTOR_ERROR_NONE);
	} else {
		LOGD("[%s/%d] Callback's error value (%lld, %s)", __FUNCTION__, __LINE__, (long long int)error->code, error->msg);
		plugin_message_set_value_number(m_callback, PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE, (pmnumber)error->code);
		plugin_message_set_value_string(m_callback, PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE, error->msg);
	}

	int param_idx = 1;
	plugin_message_set_param_number(m_callback, param_idx++, (pmnumber)file_descriptor);
	plugin_message_set_param_number(m_callback, param_idx++, (pmnumber)progress_size_byte);
	plugin_message_set_param_number(m_callback, param_idx++, (pmnumber)total_size_byte);

	char *result = NULL;

	ret = plugin_message_serialize(m_callback, &result);

	if ((0 == ret) && (NULL != result)) {
		LOGD("[%s/%d] Send callback data message to adaptor", __FUNCTION__, __LINE__);
		int res_len = strlen(result);
		int rcv_len = write(g_child_plugin->wd, &res_len, sizeof(int));
		rcv_len = write(g_child_plugin->wd, result, sizeof(char) * res_len);
		if (rcv_len <= 0) {
			LOGE("[%s/%d] pipe socket writing error", __FUNCTION__, __LINE__);
		}
	} else {
		LOGE("[%s/%d] Callback data message serialization failed", __FUNCTION__, __LINE__);
	}

	plugin_message_destroy(m_callback);

}


/* For forked plugin */
void *_storage_plugin_request_collector(void *data)
{
	storage_adaptor_plugin_h plugin = (storage_adaptor_plugin_h) data;

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
		buf_size %= (PLUGIN_MESSAGE_PROTOCOL_MAX_BUF_SIZE - 1);
		rcv_len = read(plugin->rd, msg_buf, buf_size);
		LOGD("read message [%s][len: %d]", msg_buf, rcv_len);

		if (rcv_len <= 0) {
			LOGD("shutdown by adaptor disconnected");
			return NULL;
		}

		char *result = NULL;
		__storage_plugin_progress_command(plugin, msg_buf, &result);

		if (NULL != result) {
			int res_len = strlen(result);
			rcv_len = write(plugin->wd, &res_len, sizeof(int));
			rcv_len = write(plugin->wd, result, sizeof(char) * res_len);
		}
		/* transfer data to adaptor */
	}
	return data;
}

storage_adaptor_plugin_context_h __storage_plugin_get_context_by_context_id(storage_adaptor_plugin_h plugin, int context_id)
{
	if (NULL == plugin) {
		return NULL;
	}

	/* For forked plugin */
	storage_adaptor_plugin_context_h ctx = NULL;
	int i, len;
	len = g_list_length(plugin->contexts);

	for (i = 0; i < len; i++) {
		ctx = (storage_adaptor_plugin_context_h) g_list_nth_data(plugin->contexts, i);

		if (context_id == ctx->context_id) {
			return ctx;
		}
	}
	return NULL;
}

void __storage_plugin_progress_command(storage_adaptor_plugin_h plugin, char *order, char **result)
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
	plugin_message_get_value_number(m_order, PLUGIN_MESSAGE_ELEMENT_MESSAGE_TYPE, &type);
	plugin_message_set_value_number(m_result, PLUGIN_MESSAGE_ELEMENT_MESSAGE_TYPE, type);
	plugin_message_set_value_number(m_result, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, (pmnumber)STORAGE_ADAPTOR_ERROR_NONE);
	context_id = (int) ctx;
	storage_adaptor_plugin_context_h context = __storage_plugin_get_context_by_context_id(plugin, context_id);

	storage_adaptor_error_code_h error_code = NULL;
	storage_adaptor_file_info_h file_info = NULL;

	if (0 == strncmp(STORAGE_PLUGIN_INTERFACE_CREATE_CONTEXT,
			func_name, strlen(STORAGE_PLUGIN_INTERFACE_CREATE_CONTEXT))) {
		LOGD(">>>>>> %s func start", func_name);
		char *app_id = NULL;
		char *app_secret = NULL;
		char *access_token = NULL;
		char *cid = NULL;
		char *uid = NULL;
		char *service_name = NULL;

		int param_idx = 1;
		plugin_message_get_param_string(m_order, param_idx++, &app_id);
		plugin_message_get_param_string(m_order, param_idx++, &app_secret);
		plugin_message_get_param_string(m_order, param_idx++, &access_token);
		plugin_message_get_param_string(m_order, param_idx++, &cid);
		plugin_message_get_param_string(m_order, param_idx++, &uid);

		LOGD("Call library function");
		context = storage_adaptor_create_plugin_context(plugin,
				app_id, app_secret, access_token, cid, uid, "");

		if (NULL == context) {
			LOGE("[%s<%s>/%d] Could not create context", __FUNCTION__, func_name, __LINE__);
/*			error_code = storage_adaptor_create_error_code((int64_t)STORAGE_ADAPTOR_ERROR_PLUGIN_INTERNAL, */
/*					"Could not create context"); */
			plugin_message_set_value_number(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_CODE,
					(pmnumber)STORAGE_ADAPTOR_ERROR_PLUGIN_INTERNAL);
			plugin_message_set_value_string(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE,
					"Could not create context");
		} else {
			LOGD("[%s<%s>/%d] Created context successfuly", __FUNCTION__, func_name, __LINE__);
			context->context_id = context_id;

			plugin_message_set_value_number(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_CODE,
					(pmnumber)STORAGE_ADAPTOR_ERROR_NONE);
		}


		free(app_id);
		free(app_secret);
		free(access_token);
		free(uid);
		free(service_name);
		LOGD("<<<<<< %s func end", func_name);
	} else if (0 == strncmp(STORAGE_PLUGIN_INTERFACE_DESTROY_CONTEXT,
			func_name, strlen(STORAGE_PLUGIN_INTERFACE_DESTROY_CONTEXT))) {
		LOGD(">>>>>> %s func start", func_name);
		if (NULL == context) {
			LOGE("[%s<%s>/%d] Could not found context", __FUNCTION__, func_name, __LINE__);
		} else {
			LOGD("[%s<%s>/%d] function success", __FUNCTION__, func_name, __LINE__);
			storage_adaptor_destroy_plugin_context(plugin, context);

			plugin_message_set_value_number(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_CODE,
					(pmnumber)STORAGE_ADAPTOR_ERROR_NONE);
		}
		LOGD("<<<<<< %s func end", func_name);
	} else if (0 == strncmp(STORAGE_PLUGIN_INTERFACE_MAKE_DIRECTORY,
			func_name, strlen(STORAGE_PLUGIN_INTERFACE_MAKE_DIRECTORY))) {
		LOGD(">>>>>> %s func start", func_name);
		char *parent_path = NULL;
		char *folder_path = NULL;

		int param_idx = 1;
		plugin_message_get_param_string(m_order, param_idx++, &parent_path);
		plugin_message_get_param_string(m_order, param_idx++, &folder_path);

		LOGD("Call library function");
		ret = plugin->handle->make_directory(context, parent_path, folder_path,
				NULL, &file_info, &error_code, NULL);

		if (STORAGE_ADAPTOR_ERROR_NONE == ret) {
			LOGD("API success");
			int param_idx = 1;

			if (NULL != file_info) {
				LOGD("Insert file info to pipe message");
				int param_ret;
				plugin_message_array_h file_info_message = NULL;
				char message_array_type[20] = {0, };
				snprintf(message_array_type, 19, "%c%c%c%c%c%c%c%c%c", PLUGIN_DATA_TYPE_STRING,
									PLUGIN_DATA_TYPE_STRING,
									PLUGIN_DATA_TYPE_STRING,
									PLUGIN_DATA_TYPE_NUM,
									PLUGIN_DATA_TYPE_NUM,
									PLUGIN_DATA_TYPE_NUM,
									PLUGIN_DATA_TYPE_NUM,
									PLUGIN_DATA_TYPE_NUM,
									PLUGIN_DATA_TYPE_STRING);
				param_ret = plugin_message_array_create(message_array_type, &file_info_message);
				if (0 == param_ret) {
					_message_array_set_file_info(file_info_message, 1, file_info);
					param_ret = plugin_message_set_param_array(m_result, param_idx++, file_info_message);
					plugin_message_array_destroy(file_info_message);
				}
			}
			plugin_message_set_value_number(m_result, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, (pmnumber)ret);
		} else if (NULL != error_code) {
			LOGD("API failed, error_code[%lld / %s]", (long long int)error_code->code, error_code->msg);
			plugin_message_set_value_number(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_CODE,
					(pmnumber)error_code->code);
			plugin_message_set_value_string(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE,
					error_code->msg ? error_code->msg : "");
			free(error_code->msg);
			free(error_code);
		} else {
			LOGD("API failed ret_code[%d]", ret);
			plugin_message_set_value_number(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, (pmnumber)ret);
		}

		free(parent_path);
		free(folder_path);
		LOGD("<<<<<< %s func end", func_name);
	} else if (0 == strncmp(STORAGE_PLUGIN_INTERFACE_REMOVE_DIRECTORY,
			func_name, strlen(STORAGE_PLUGIN_INTERFACE_REMOVE_DIRECTORY))) {
		LOGD(">>>>>> %s func start", func_name);
		char *parent_path = NULL;
		char *folder_path = NULL;

		int param_idx = 1;
		plugin_message_get_param_string(m_order, param_idx++, &parent_path);
		plugin_message_get_param_string(m_order, param_idx++, &folder_path);

		LOGD("Call library function");
		ret = plugin->handle->remove_directory(context, parent_path, folder_path,
				NULL, &file_info, &error_code, NULL);

		if (STORAGE_ADAPTOR_ERROR_NONE == ret) {
			LOGD("API success");
			int param_idx = 1;

			if (NULL != file_info) {
				LOGD("Insert file info to pipe message");
				int param_ret;
				plugin_message_array_h file_info_message = NULL;
				char message_array_type[20] = {0, };
				snprintf(message_array_type, 19, "%c%c%c%c%c%c%c%c%c", PLUGIN_DATA_TYPE_STRING,
									PLUGIN_DATA_TYPE_STRING,
									PLUGIN_DATA_TYPE_STRING,
									PLUGIN_DATA_TYPE_NUM,
									PLUGIN_DATA_TYPE_NUM,
									PLUGIN_DATA_TYPE_NUM,
									PLUGIN_DATA_TYPE_NUM,
									PLUGIN_DATA_TYPE_NUM,
									PLUGIN_DATA_TYPE_STRING);
				param_ret = plugin_message_array_create(message_array_type, &file_info_message);
				if (0 == param_ret) {
					_message_array_set_file_info(file_info_message, 1, file_info);
					param_ret = plugin_message_set_param_array(m_result, param_idx++, file_info_message);
					plugin_message_array_destroy(file_info_message);
				}
			}
			plugin_message_set_value_number(m_result, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, (pmnumber)ret);
		} else if (NULL != error_code) {
			LOGD("API failed, error_code[%lld / %s]", (long long int)error_code->code, error_code->msg);
			plugin_message_set_value_number(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_CODE,
					(pmnumber)error_code->code);
			plugin_message_set_value_string(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE,
					error_code->msg ? error_code->msg : "");
			free(error_code->msg);
			free(error_code);
		} else {
			LOGD("API failed ret_code[%d]", ret);
			plugin_message_set_value_number(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, (pmnumber)ret);
		}

		free(parent_path);
		free(folder_path);
		LOGD("<<<<<< %s func end", func_name);
	} else if (0 == strncmp(STORAGE_PLUGIN_INTERFACE_GET_LIST,
			func_name, strlen(STORAGE_PLUGIN_INTERFACE_GET_LIST))) {
		LOGD(">>>>>> %s func start", func_name);
		char *parent_path = NULL;
		char *folder_path = NULL;

		int param_idx = 1;
		plugin_message_get_param_string(m_order, param_idx++, &parent_path);
		plugin_message_get_param_string(m_order, param_idx++, &folder_path);

		storage_adaptor_file_info_h *file_list = NULL;
		int file_list_len = 0;
		LOGD("Call library function");
		ret = plugin->handle->list(context, parent_path, folder_path,
				NULL, &file_list, &file_list_len, &error_code, NULL);

		if (STORAGE_ADAPTOR_ERROR_NONE == ret) {
			int param_idx = 1;

			if ((NULL != file_list) && (0 < file_list_len)) {
				LOGD("Insert file list to pipe message (length : %d)", file_list_len);
				int param_ret, i;
				plugin_message_array_h file_info_message = NULL;
				char message_array_type[20] = {0, };
				snprintf(message_array_type, 19, "%c%c%c%c%c%c%c%c%c", PLUGIN_DATA_TYPE_STRING,
									PLUGIN_DATA_TYPE_STRING,
									PLUGIN_DATA_TYPE_STRING,
									PLUGIN_DATA_TYPE_NUM,
									PLUGIN_DATA_TYPE_NUM,
									PLUGIN_DATA_TYPE_NUM,
									PLUGIN_DATA_TYPE_NUM,
									PLUGIN_DATA_TYPE_NUM,
									PLUGIN_DATA_TYPE_STRING);
				param_ret = plugin_message_array_create(message_array_type, &file_info_message);
				if (0 == param_ret) {
					for (i = 0; i < file_list_len; i++) {
						_message_array_set_file_info(file_info_message, (i + 1), file_list[i]);
						storage_adaptor_destroy_file_info(&(file_list[i]));
					}
					free(file_list);
					file_list = NULL;
					param_ret = plugin_message_set_param_array(m_result, param_idx++, file_info_message);
					param_ret = plugin_message_set_param_number(m_result, param_idx++, (pmnumber)file_list_len);
					plugin_message_array_destroy(file_info_message);
				}
			}
			plugin_message_set_value_number(m_result, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, (pmnumber)ret);
		} else if (NULL != error_code) {
			LOGD("API failed, error_code[%lld / %s]", (long long int)error_code->code, error_code->msg);
			plugin_message_set_value_number(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_CODE,
					(pmnumber)error_code->code);
			plugin_message_set_value_string(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE,
					error_code->msg ? error_code->msg : "");
			free(error_code->msg);
			free(error_code);
		} else {
			LOGD("API failed ret_code[%d]", ret);
			plugin_message_set_value_number(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, (pmnumber)ret);
		}

		free(parent_path);
		free(folder_path);
		LOGD("<<<<<< %s func end", func_name);
	} else if (0 == strncmp(STORAGE_PLUGIN_INTERFACE_UPLOAD_FILE_SYNC,
			func_name, strlen(STORAGE_PLUGIN_INTERFACE_UPLOAD_FILE_SYNC))) {
		LOGD(">>>>>> %s func start", func_name);
		char *parent_path = NULL;
		char *folder_path = NULL;
		char *local_path = NULL;
		pmnumber publish;

		int param_idx = 1;
		plugin_message_get_param_string(m_order, param_idx++, &parent_path);
		plugin_message_get_param_string(m_order, param_idx++, &folder_path);
		plugin_message_get_param_string(m_order, param_idx++, &local_path);
		plugin_message_get_param_number(m_order, param_idx++, &publish);

		LOGD("Call library function");
		ret = plugin->handle->upload_file_sync(context, parent_path, folder_path, local_path, (int)publish,
				NULL, &file_info, &error_code, NULL);

		if (STORAGE_ADAPTOR_ERROR_NONE == ret) {
			LOGD("API success");
			int param_idx = 1;

			if (NULL != file_info) {
				LOGD("Insert file info to pipe message");
				int param_ret;
				plugin_message_array_h file_info_message = NULL;
				char message_array_type[20] = {0, };
				snprintf(message_array_type, 19, "%c%c%c%c%c%c%c%c%c", PLUGIN_DATA_TYPE_STRING,
									PLUGIN_DATA_TYPE_STRING,
									PLUGIN_DATA_TYPE_STRING,
									PLUGIN_DATA_TYPE_NUM,
									PLUGIN_DATA_TYPE_NUM,
									PLUGIN_DATA_TYPE_NUM,
									PLUGIN_DATA_TYPE_NUM,
									PLUGIN_DATA_TYPE_NUM,
									PLUGIN_DATA_TYPE_STRING);
				param_ret = plugin_message_array_create(message_array_type, &file_info_message);
				if (0 == param_ret) {
					_message_array_set_file_info(file_info_message, 1, file_info);
					param_ret = plugin_message_set_param_array(m_result, param_idx++, file_info_message);
					plugin_message_array_destroy(file_info_message);
				}
			}
			plugin_message_set_value_number(m_result, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, (pmnumber)ret);
		} else if (NULL != error_code) {
			LOGD("API failed, error_code[%lld / %s]", (long long int)error_code->code, error_code->msg);
			plugin_message_set_value_number(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_CODE,
					(pmnumber)error_code->code);
			plugin_message_set_value_string(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE,
					error_code->msg ? error_code->msg : "");
			free(error_code->msg);
			free(error_code);
		} else {
			LOGD("API failed ret_code[%d]", ret);
			plugin_message_set_value_number(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, (pmnumber)ret);
		}

		free(parent_path);
		free(folder_path);
		free(local_path);
		LOGD("<<<<<< %s func end", func_name);
	} else if (0 == strncmp(STORAGE_PLUGIN_INTERFACE_DOWNLOAD_FILE_SYNC,
			func_name, strlen(STORAGE_PLUGIN_INTERFACE_DOWNLOAD_FILE_SYNC))) {
		LOGD(">>>>>> %s func start", func_name);
		char *parent_path = NULL;
		char *folder_path = NULL;
		char *local_path = NULL;

		int param_idx = 1;
		plugin_message_get_param_string(m_order, param_idx++, &parent_path);
		plugin_message_get_param_string(m_order, param_idx++, &folder_path);
		plugin_message_get_param_string(m_order, param_idx++, &local_path);

		LOGD("Call library function");
		ret = plugin->handle->download_file_sync(context, parent_path, folder_path, local_path,
				NULL, &error_code, NULL);

		if (STORAGE_ADAPTOR_ERROR_NONE == ret) {
			LOGD("API success");
			plugin_message_set_value_number(m_result, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, (pmnumber)ret);
		} else if (NULL != error_code) {
			LOGD("API failed, error_code[%lld / %s]", (long long int)error_code->code, error_code->msg);
			plugin_message_set_value_number(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_CODE,
					(pmnumber)error_code->code);
			plugin_message_set_value_string(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE,
					error_code->msg ? error_code->msg : "");
			free(error_code->msg);
			free(error_code);
		} else {
			LOGD("API failed ret_code[%d]", ret);
			plugin_message_set_value_number(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, (pmnumber)ret);
		}

		free(parent_path);
		free(folder_path);
		free(local_path);
		LOGD("<<<<<< %s func end", func_name);
	} else if (0 == strncmp(STORAGE_PLUGIN_INTERFACE_DELETE_FILE,
			func_name, strlen(STORAGE_PLUGIN_INTERFACE_DELETE_FILE))) {
		LOGD(">>>>>> %s func start", func_name);
		char *parent_path = NULL;
		char *folder_path = NULL;

		int param_idx = 1;
		plugin_message_get_param_string(m_order, param_idx++, &parent_path);
		plugin_message_get_param_string(m_order, param_idx++, &folder_path);

		LOGD("Call library function");
		ret = plugin->handle->delete_file(context, parent_path, folder_path,
				NULL, &file_info, &error_code, NULL);

		if (STORAGE_ADAPTOR_ERROR_NONE == ret) {
			LOGD("API success");
			int param_idx = 1;

			if (NULL != file_info) {
				LOGD("Insert file info to pipe message");
				int param_ret;
				plugin_message_array_h file_info_message = NULL;
				char message_array_type[20] = {0, };
				snprintf(message_array_type, 19, "%c%c%c%c%c%c%c%c%c", PLUGIN_DATA_TYPE_STRING,
									PLUGIN_DATA_TYPE_STRING,
									PLUGIN_DATA_TYPE_STRING,
									PLUGIN_DATA_TYPE_NUM,
									PLUGIN_DATA_TYPE_NUM,
									PLUGIN_DATA_TYPE_NUM,
									PLUGIN_DATA_TYPE_NUM,
									PLUGIN_DATA_TYPE_NUM,
									PLUGIN_DATA_TYPE_STRING);
				param_ret = plugin_message_array_create(message_array_type, &file_info_message);
				if (0 == param_ret) {
					_message_array_set_file_info(file_info_message, 1, file_info);
					param_ret = plugin_message_set_param_array(m_result, param_idx++, file_info_message);
					plugin_message_array_destroy(file_info_message);
				}
			}
			plugin_message_set_value_number(m_result, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, (pmnumber)ret);
		} else if (NULL != error_code) {
			LOGD("API failed, error_code[%lld / %s]", (long long int)error_code->code, error_code->msg);
			plugin_message_set_value_number(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_CODE,
					(pmnumber)error_code->code);
			plugin_message_set_value_string(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE,
					error_code->msg ? error_code->msg : "");
			free(error_code->msg);
			free(error_code);
		} else {
			LOGD("API failed ret_code[%d]", ret);
			plugin_message_set_value_number(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, (pmnumber)ret);
		}

		free(parent_path);
		free(folder_path);
		LOGD("<<<<<< %s func end", func_name);
	} else if (0 == strncmp(STORAGE_PLUGIN_INTERFACE_MOVE_DIRECTORY,
			func_name, strlen(STORAGE_PLUGIN_INTERFACE_MOVE_DIRECTORY))) {
		LOGD(">>>>>> %s func start", func_name);
		char *parent_path = NULL;
		char *folder_path = NULL;
		char *dst_parent_path = NULL;
		char *dst_folder_path = NULL;

		int param_idx = 1;
		plugin_message_get_param_string(m_order, param_idx++, &parent_path);
		plugin_message_get_param_string(m_order, param_idx++, &folder_path);
		plugin_message_get_param_string(m_order, param_idx++, &dst_parent_path);
		plugin_message_get_param_string(m_order, param_idx++, &dst_folder_path);

		LOGD("Call library function");
		ret = plugin->handle->move_directory(context, parent_path, folder_path, dst_parent_path, dst_folder_path,
				NULL, &file_info, &error_code, NULL);

		if (STORAGE_ADAPTOR_ERROR_NONE == ret) {
			LOGD("API success");
			int param_idx = 1;

			if (NULL != file_info) {
				LOGD("Insert file info to pipe message");
				int param_ret;
				plugin_message_array_h file_info_message = NULL;
				char message_array_type[20] = {0, };
				snprintf(message_array_type, 19, "%c%c%c%c%c%c%c%c%c", PLUGIN_DATA_TYPE_STRING,
									PLUGIN_DATA_TYPE_STRING,
									PLUGIN_DATA_TYPE_STRING,
									PLUGIN_DATA_TYPE_NUM,
									PLUGIN_DATA_TYPE_NUM,
									PLUGIN_DATA_TYPE_NUM,
									PLUGIN_DATA_TYPE_NUM,
									PLUGIN_DATA_TYPE_NUM,
									PLUGIN_DATA_TYPE_STRING);
				param_ret = plugin_message_array_create(message_array_type, &file_info_message);
				if (0 == param_ret) {
					_message_array_set_file_info(file_info_message, 1, file_info);
					param_ret = plugin_message_set_param_array(m_result, param_idx++, file_info_message);
					plugin_message_array_destroy(file_info_message);
				}
			}
			plugin_message_set_value_number(m_result, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, (pmnumber)ret);
		} else if (NULL != error_code) {
			LOGD("API failed, error_code[%lld / %s]", (long long int)error_code->code, error_code->msg);
			plugin_message_set_value_number(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_CODE,
					(pmnumber)error_code->code);
			plugin_message_set_value_string(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE,
					error_code->msg ? error_code->msg : "");
			free(error_code->msg);
			free(error_code);
		} else {
			LOGD("API failed ret_code[%d]", ret);
			plugin_message_set_value_number(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, (pmnumber)ret);
		}

		free(parent_path);
		free(folder_path);
		free(dst_parent_path);
		free(dst_folder_path);
		LOGD("<<<<<< %s func end", func_name);
	} else if (0 == strncmp(STORAGE_PLUGIN_INTERFACE_MOVE_FILE,
			func_name, strlen(STORAGE_PLUGIN_INTERFACE_MOVE_FILE))) {
		LOGD(">>>>>> %s func start", func_name);
		char *parent_path = NULL;
		char *file_path = NULL;
		char *dst_parent_path = NULL;
		char *dst_file_path = NULL;

		int param_idx = 1;
		plugin_message_get_param_string(m_order, param_idx++, &parent_path);
		plugin_message_get_param_string(m_order, param_idx++, &file_path);
		plugin_message_get_param_string(m_order, param_idx++, &dst_parent_path);
		plugin_message_get_param_string(m_order, param_idx++, &dst_file_path);

		LOGD("Call library function");
		ret = plugin->handle->move_file(context, parent_path, file_path, dst_parent_path, dst_file_path,
				NULL, &file_info, &error_code, NULL);

		if (STORAGE_ADAPTOR_ERROR_NONE == ret) {
			LOGD("API success");
			int param_idx = 1;

			if (NULL != file_info) {
				LOGD("Insert file info to pipe message");
				int param_ret;
				plugin_message_array_h file_info_message = NULL;
				char message_array_type[20] = {0, };
				snprintf(message_array_type, 19, "%c%c%c%c%c%c%c%c%c", PLUGIN_DATA_TYPE_STRING,
									PLUGIN_DATA_TYPE_STRING,
									PLUGIN_DATA_TYPE_STRING,
									PLUGIN_DATA_TYPE_NUM,
									PLUGIN_DATA_TYPE_NUM,
									PLUGIN_DATA_TYPE_NUM,
									PLUGIN_DATA_TYPE_NUM,
									PLUGIN_DATA_TYPE_NUM,
									PLUGIN_DATA_TYPE_STRING);
				param_ret = plugin_message_array_create(message_array_type, &file_info_message);
				if (0 == param_ret) {
					_message_array_set_file_info(file_info_message, 1, file_info);
					param_ret = plugin_message_set_param_array(m_result, param_idx++, file_info_message);
					plugin_message_array_destroy(file_info_message);
				}
			}
			plugin_message_set_value_number(m_result, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, (pmnumber)ret);
		} else if (NULL != error_code) {
			LOGD("API failed, error_code[%lld / %s]", (long long int)error_code->code, error_code->msg);
			plugin_message_set_value_number(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_CODE,
					(pmnumber)error_code->code);
			plugin_message_set_value_string(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE,
					error_code->msg ? error_code->msg : "");
			free(error_code->msg);
			free(error_code);
		} else {
			LOGD("API failed ret_code[%d]", ret);
			plugin_message_set_value_number(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, (pmnumber)ret);
		}

		free(parent_path);
		free(file_path);
		free(dst_parent_path);
		free(dst_file_path);
		LOGD("<<<<<< %s func end", func_name);
	} else if (0 == strncmp(STORAGE_PLUGIN_INTERFACE_START_UPLOAD_TASK,
			func_name, strlen(STORAGE_PLUGIN_INTERFACE_START_UPLOAD_TASK))) {
		LOGD(">>>>>> %s func start", func_name);
		pmnumber fd, user_data;
		char *parent_path = NULL;
		char *file_name = NULL;
		bool need_progress = false;

		int param_idx = 1;
		plugin_message_get_param_number(m_order, param_idx++, &fd);
		plugin_message_get_param_string(m_order, param_idx++, &parent_path);
		plugin_message_get_param_string(m_order, param_idx++, &file_name);
		plugin_message_get_param_bool(m_order, param_idx++, &need_progress);

		param_idx = 1;
		plugin_message_get_param_number(m_order, param_idx++, &user_data);

		LOGD("Call library function");
		ret = plugin->handle->start_upload_task(context, (int)fd, parent_path, file_name, need_progress,
				&error_code, (void *)(intptr_t)user_data);

		if (STORAGE_ADAPTOR_ERROR_NONE == ret) {
			LOGD("API success");

			plugin_message_set_value_number(m_result, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, (pmnumber)ret);
		} else if (NULL != error_code) {
			LOGD("API failed, error_code[%lld / %s]", (long long int)error_code->code, error_code->msg);
			plugin_message_set_value_number(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_CODE,
					(pmnumber)error_code->code);
			plugin_message_set_value_string(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE,
					error_code->msg ? error_code->msg : "");
			free(error_code->msg);
			free(error_code);
		} else {
			LOGD("API failed ret_code[%d]", ret);
			plugin_message_set_value_number(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, (pmnumber)ret);
		}

		free(parent_path);
		free(file_name);
		LOGD("<<<<<< %s func end", func_name);
	} else if (0 == strncmp(STORAGE_PLUGIN_INTERFACE_START_DOWNLOAD_TASK,
			func_name, strlen(STORAGE_PLUGIN_INTERFACE_START_DOWNLOAD_TASK))) {
		LOGD(">>>>>> %s func start", func_name);
		pmnumber fd, user_data;
		char *parent_path = NULL;
		char *file_name = NULL;
		bool need_progress = false;

		int param_idx = 1;
		plugin_message_get_param_string(m_order, param_idx++, &parent_path);
		plugin_message_get_param_string(m_order, param_idx++, &file_name);
		plugin_message_get_param_number(m_order, param_idx++, &fd);
		plugin_message_get_param_bool(m_order, param_idx++, &need_progress);

		param_idx = 1;
		plugin_message_get_param_number(m_order, param_idx++, &user_data);

		LOGD("Call library function");
		ret = plugin->handle->start_download_task(context, parent_path, file_name, (int)fd, need_progress,
				&error_code, (void *)(intptr_t)user_data);

		if (STORAGE_ADAPTOR_ERROR_NONE == ret) {
			LOGD("API success");

			plugin_message_set_value_number(m_result, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, (pmnumber)ret);
		} else if (NULL != error_code) {
			LOGD("API failed, error_code[%lld / %s]", (long long int)error_code->code, error_code->msg);
			plugin_message_set_value_number(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_CODE,
					(pmnumber)error_code->code);
			plugin_message_set_value_string(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE,
					error_code->msg ? error_code->msg : "");
			free(error_code->msg);
			free(error_code);
		} else {
			LOGD("API failed ret_code[%d]", ret);
			plugin_message_set_value_number(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, (pmnumber)ret);
		}

		free(parent_path);
		free(file_name);
		LOGD("<<<<<< %s func end", func_name);
	} else if (0 == strncmp(STORAGE_PLUGIN_INTERFACE_START_DOWNLOAD_THUMB_TASK,
			func_name, strlen(STORAGE_PLUGIN_INTERFACE_START_DOWNLOAD_THUMB_TASK))) {
		LOGD(">>>>>> %s func start", func_name);
		pmnumber fd, thumb_size, user_data;
		char *parent_path = NULL;
		char *file_name = NULL;
		bool need_progress = false;

		int param_idx = 1;
		plugin_message_get_param_string(m_order, param_idx++, &parent_path);
		plugin_message_get_param_string(m_order, param_idx++, &file_name);
		plugin_message_get_param_number(m_order, param_idx++, &fd);
		plugin_message_get_param_number(m_order, param_idx++, &thumb_size);
		plugin_message_get_param_bool(m_order, param_idx++, &need_progress);

		param_idx = 1;
		plugin_message_get_param_number(m_order, param_idx++, &user_data);

		LOGD("Call library function");
		ret = plugin->handle->start_download_thumb_task(context, parent_path, file_name, (int)fd, (int)thumb_size, need_progress,
				&error_code, (void *)(intptr_t)user_data);

		if (STORAGE_ADAPTOR_ERROR_NONE == ret) {
			LOGD("API success");

			plugin_message_set_value_number(m_result, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, (pmnumber)ret);
		} else if (NULL != error_code) {
			LOGD("API failed, error_code[%lld / %s]", (long long int)error_code->code, error_code->msg);
			plugin_message_set_value_number(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_CODE,
					(pmnumber)error_code->code);
			plugin_message_set_value_string(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE,
					error_code->msg ? error_code->msg : "");
			free(error_code->msg);
			free(error_code);
		} else {
			LOGD("API failed ret_code[%d]", ret);
			plugin_message_set_value_number(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, (pmnumber)ret);
		}

		free(parent_path);
		free(file_name);
		LOGD("<<<<<< %s func end", func_name);
	} else if (0 == strncmp(STORAGE_PLUGIN_INTERFACE_CANCEL_UPLOAD_TASK,
			func_name, strlen(STORAGE_PLUGIN_INTERFACE_CANCEL_UPLOAD_TASK))) {
		LOGD(">>>>>> %s func start", func_name);
		pmnumber fd;

		int param_idx = 1;
		plugin_message_get_param_number(m_order, param_idx++, &fd);

		LOGD("Call library function");
		ret = plugin->handle->cancel_upload_task(context, (int)fd, &error_code);

		if (STORAGE_ADAPTOR_ERROR_NONE == ret) {
			LOGD("API success");

			plugin_message_set_value_number(m_result, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, (pmnumber)ret);
		} else if (NULL != error_code) {
			LOGD("API failed, error_code[%lld / %s]", (long long int)error_code->code, error_code->msg);
			plugin_message_set_value_number(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_CODE,
					(pmnumber)error_code->code);
			plugin_message_set_value_string(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE,
					error_code->msg ? error_code->msg : "");
			free(error_code->msg);
			free(error_code);
		} else {
			LOGD("API failed ret_code[%d]", ret);
			plugin_message_set_value_number(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, (pmnumber)ret);
		}

		LOGD("<<<<<< %s func end", func_name);
	} else if (0 == strncmp(STORAGE_PLUGIN_INTERFACE_CANCEL_DOWNLOAD_TASK,
			func_name, strlen(STORAGE_PLUGIN_INTERFACE_CANCEL_DOWNLOAD_TASK))) {
		LOGD(">>>>>> %s func start", func_name);
		pmnumber fd;

		int param_idx = 1;
		plugin_message_get_param_number(m_order, param_idx++, &fd);

		LOGD("Call library function");
		ret = plugin->handle->cancel_download_task(context, (int)fd, &error_code);

		if (STORAGE_ADAPTOR_ERROR_NONE == ret) {
			LOGD("API success");

			plugin_message_set_value_number(m_result, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, (pmnumber)ret);
		} else if (NULL != error_code) {
			LOGD("API failed, error_code[%lld / %s]", (long long int)error_code->code, error_code->msg);
			plugin_message_set_value_number(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_CODE,
					(pmnumber)error_code->code);
			plugin_message_set_value_string(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE,
					error_code->msg ? error_code->msg : "");
			free(error_code->msg);
			free(error_code);
		} else {
			LOGD("API failed ret_code[%d]", ret);
			plugin_message_set_value_number(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, (pmnumber)ret);
		}

		LOGD("<<<<<< %s func end", func_name);
	} else if (0 == strncmp(STORAGE_PLUGIN_INTERFACE_CANCEL_DOWNLOAD_THUMB_TASK,
			func_name, strlen(STORAGE_PLUGIN_INTERFACE_CANCEL_DOWNLOAD_THUMB_TASK))) {
		LOGD(">>>>>> %s func start", func_name);
		pmnumber fd;

		int param_idx = 1;
		plugin_message_get_param_number(m_order, param_idx++, &fd);

		LOGD("Call library function");
		ret = plugin->handle->cancel_download_thumb_task(context, (int)fd, &error_code);

		if (STORAGE_ADAPTOR_ERROR_NONE == ret) {
			LOGD("API success");

			plugin_message_set_value_number(m_result, PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, (pmnumber)ret);
		} else if (NULL != error_code) {
			LOGD("API failed, error_code[%lld / %s]", (long long int)error_code->code, error_code->msg);
			plugin_message_set_value_number(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_CODE,
					(pmnumber)error_code->code);
			plugin_message_set_value_string(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE,
					error_code->msg ? error_code->msg : "");
			free(error_code->msg);
			free(error_code);
		} else {
			LOGD("API failed ret_code[%d]", ret);
			plugin_message_set_value_number(m_result,
					PLUGIN_MESSAGE_ELEMENT_RESULT_CODE, (pmnumber)ret);
		}

		LOGD("<<<<<< %s func end", func_name);
	} else { /* TODO Next */
		LOGD(">>>>>> %s func start", func_name);
		plugin_message_set_value_number(m_result,
				PLUGIN_MESSAGE_ELEMENT_RESULT_CODE,
				(pmnumber)STORAGE_ADAPTOR_ERROR_UNSUPPORTED);
		plugin_message_set_value_string(m_result,
				PLUGIN_MESSAGE_ELEMENT_RESULT_MESSAGE,
				"Unsupported operation");
		LOGD("<<<<<< %s func end", func_name);
	}

	storage_adaptor_destroy_file_info(&file_info);
	free(func_name);

	char *result_data = NULL;
	plugin_message_serialize(m_result, &result_data);
	plugin_message_destroy(m_result);
	plugin_message_destroy(m_order);

	*result = result_data;
}
