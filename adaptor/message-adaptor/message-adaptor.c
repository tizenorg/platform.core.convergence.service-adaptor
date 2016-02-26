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


#include "message-adaptor.h"
#include "message-adaptor-log.h"


/**
 * Message adaptor plugin
 */
typedef struct message_adaptor_plugin_s {
	message_adaptor_h			adaptor;		/* Adaptor */
	char					*path;			/* Plugin library path */
	message_adaptor_plugin_handle_h		handle;			/* Plugin handle */
	void					*dl_handle;		/* Plugin library handle */
	int					ref_counter;		/* Plugin reference counter */
	GMutex					ref_counter_mutex;	/* Plugin reference counter mutex */
	message_adaptor_plugin_listener_h	plugin_listener;	/* Plugin callback listener */
	GMutex					plugin_listener_mutex;	/* Plugin callback listener mutex */
	int					connected;		/* connected flag */
	GMutex					plugin_connect_mutex;
	GCond					plugin_connect_cond;
} message_adaptor_plugin_t;

/**
 * Message adaptor
 */
typedef struct message_adaptor_s {
	GMutex	message_adaptor_mutex;		/* Adaptor mutex */
	int	started;			/* Started flag */
	char	*plugins_dir;			/* Plugins directory path */
	GList	*plugins;			/* List of loaded plugins */
	GMutex	plugins_mutex;			/* Plugin list mutex */
	GList	*adaptor_listeners;		/* List of vservice channel listener (for now not effective) */
	GMutex	adaptor_listeners_mutex;	/* Listener list mutex */
} message_adaptor_t;

/**
 * Creates plugin
 */
static message_adaptor_plugin_h message_adaptor_create_plugin(const char *plugin_path);

/**
 * Destroys plugin and deletes all resources associated with it
 */
static void message_adaptor_destroy_plugin(message_adaptor_plugin_h plugin);

/**
 * Loads plugins from selected directory
 */
static int message_adaptor_load_plugins_from_directory(message_adaptor_h adaptor, const char *dir_path);

/**
 * Checks if plugin is loaded by selected plugin adaptor
 */
static int message_adaptor_has_plugin(message_adaptor_h adaptor, message_adaptor_plugin_h plugin);

/**
 * Increases adaptor's plugin references counter
 */
static void message_adaptor_plugin_ref(message_adaptor_plugin_h);

/**
 * Decreases adaptor's plugin references counter
 */
static void message_adaptor_plugin_unref(message_adaptor_plugin_h);


/**
 * Definition of callback function variables for service adaptor
 */

message_adaptor_service_client_echo_cb _service_adaptor_service_client_echo_cb = NULL;
message_adaptor_service_create_chatroom_reply_cb _service_adaptor_service_create_chatroom_reply_cb = NULL;
message_adaptor_service_change_chatroom_meta_reply_cb _service_adaptor_service_change_chatroom_meta_reply_cb = NULL;
message_adaptor_service_chat_reply_cb _service_adaptor_service_chat_reply_cb = NULL;
message_adaptor_service_allow_chat_reply_cb _service_adaptor_service_allow_chat_reply_cb = NULL;
message_adaptor_service_get_all_unread_message_reply_cb _service_adaptor_service_get_all_unread_message_reply_cb = NULL;
message_adaptor_service_forward_online_message_request_cb _service_adaptor_service_forward_online_message_request_cb = NULL;
message_adaptor_service_forward_unread_message_request_cb _service_adaptor_service_forward_unread_message_request_cb = NULL;
message_adaptor_service_read_message_reply_cb _service_adaptor_service_read_message_reply_cb = NULL;
message_adaptor_service_invite_chat_reply_cb _service_adaptor_service_invite_chat_reply_cb = NULL;
message_adaptor_service_end_chat_reply_cb _service_adaptor_service_end_chat_reply_cb = NULL;
message_adaptor_service_unseal_message_reply_cb _service_adaptor_service_unseal_message_reply_cb = NULL;
message_adaptor_service_save_call_log_reply_cb _service_adaptor_service_save_call_log_reply_cb = NULL;
message_adaptor_service_current_time_reply_cb _service_adaptor_service_current_time_reply_cb = NULL;
message_adaptor_service_typing_updated_cb _service_adaptor_service_typing_updated_cb = NULL;
message_adaptor_service_completion_cb _service_adaptor_service_completion_cb = NULL;
/*
 * Required function for sample callback functions
 */

void
message_adaptor_client_echo_cb(message_adaptor_plugin_context_h context,
						long long int request_id,
						message_adaptor_error_code_t **error_code,
						void *server_data)
{
	plugin_req_id_print();
	if (_service_adaptor_service_client_echo_cb) {
		_service_adaptor_service_client_echo_cb(context, request_id, error_code, server_data);
	}
}

void
message_adaptor_create_chatroom_reply_cb(message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int chatroom_id,
						int default_message_ttl,
						message_adaptor_wrong_receiver_s *wrong_receiver,
						message_adaptor_error_code_t **error_code,
						void *server_data)
{
	plugin_req_id_print();
	if (_service_adaptor_service_create_chatroom_reply_cb) {
		_service_adaptor_service_create_chatroom_reply_cb(context, request_id,
				chatroom_id, default_message_ttl, wrong_receiver, error_code, server_data);
	}
}

void
message_adaptor_change_chatroom_meta_reply_cb(message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int chatroom_id,
						message_adaptor_error_code_t **error_code,
						void *server_data)

{
	plugin_req_id_print();
	if (_service_adaptor_service_change_chatroom_meta_reply_cb) {
		_service_adaptor_service_change_chatroom_meta_reply_cb(context, request_id,
				chatroom_id, error_code, server_data);
	}
}

void
message_adaptor_chat_reply_cb(message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int chatroom_id,
						message_adaptor_processed_msg_s **processed_msgs,
						unsigned int processed_msgs_len,
						message_adaptor_error_code_t **error_code,
						void *server_data)
{
	plugin_req_id_print();
	if (_service_adaptor_service_chat_reply_cb) {
		_service_adaptor_service_chat_reply_cb(context,
				request_id, chatroom_id, processed_msgs,
				processed_msgs_len, error_code, server_data);
	}
}


void
message_adaptor_allow_chat_reply_cb(message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int chatroom_id,
						message_adaptor_delivery_ack_s **deliveryacks,
						unsigned int deliveryacks_len,
						unsigned long long last_delivery_ack_timestamp,
						message_adaptor_read_ack_s **read_acks,
						unsigned int read_acks_len,
						unsigned long long last_read_ack_timestamp,
						message_adaptor_ordered_chat_member_s **ordered_chat_members,
						unsigned int ordered_chat_members_len,
						const char *chatroom_title,
						int default_message_ttl,
						message_adaptor_error_code_t **error_code,
						void *server_data)
{
	plugin_req_id_print();
	if (_service_adaptor_service_allow_chat_reply_cb) {
		_service_adaptor_service_allow_chat_reply_cb(context,
				request_id, chatroom_id,
				deliveryacks, deliveryacks_len, last_delivery_ack_timestamp,
				read_acks, read_acks_len, last_read_ack_timestamp,
				ordered_chat_members, ordered_chat_members_len,
				chatroom_title, default_message_ttl,
				error_code, server_data);
	}
}

void
message_adaptor_get_all_unread_message_reply_cb(message_adaptor_plugin_context_h context,
						long long int request_id,
						message_adaptor_error_code_t **error_code,
						void *server_data)
{
	plugin_req_id_print();
	if (_service_adaptor_service_get_all_unread_message_reply_cb) {
		_service_adaptor_service_get_all_unread_message_reply_cb(context,
				request_id, error_code, server_data);
	}
}

void
message_adaptor_forward_online_message_request_cb(message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int chatroom_id, int chat_type,
						message_inboxentry_t *inbox_msg,
						bool skip_reply,
						message_adaptor_error_code_t **error_code,
						void *server_data)
{
	plugin_req_id_print();
	if (_service_adaptor_service_forward_online_message_request_cb) {
		_service_adaptor_service_forward_online_message_request_cb(context,
				request_id, chatroom_id, chat_type,
				inbox_msg, skip_reply, error_code, server_data);
	}
}

void
message_adaptor_forward_unread_message_request_cb(message_adaptor_plugin_context_h context,
						long long int request_id,
						message_inboxentry_t ***inbox_msgs,
						unsigned int inbox_msgs_len,
						char **next_pagination_key,
						message_adaptor_error_code_t **error_code,
						void *server_data)
{
	plugin_req_id_print();
	if (_service_adaptor_service_forward_unread_message_request_cb) {
		_service_adaptor_service_forward_unread_message_request_cb(context,
				request_id, inbox_msgs, inbox_msgs_len,
				next_pagination_key, error_code, server_data);
	}
}

void
message_adaptor_read_message_reply_cb(message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int chatroom_id,
						message_adaptor_error_code_t **error_code,
						void *server_data)
{
	plugin_req_id_print();
	if (_service_adaptor_service_read_message_reply_cb) {
		_service_adaptor_service_read_message_reply_cb(context,
				request_id, chatroom_id, error_code, server_data);
	}
}

void
message_adaptor_invite_chat_reply_cb(message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int chatroom_id,
						long long int sent_time,
						message_adaptor_wrong_receiver_s *wrong_receiver,
						message_adaptor_error_code_t **error_code,
						void *server_data)
{
	plugin_req_id_print();
	if (_service_adaptor_service_invite_chat_reply_cb) {
		_service_adaptor_service_invite_chat_reply_cb(context,
				request_id, chatroom_id, sent_time,
				wrong_receiver, error_code, server_data);
	}
}

void
message_adaptor_end_chat_reply_cb(message_adaptor_plugin_context_h context,
						long long int request_id,
						message_adaptor_error_code_t **error_code,
						void *server_data)
{
	plugin_req_id_print();
	if (_service_adaptor_service_end_chat_reply_cb) {
		_service_adaptor_service_end_chat_reply_cb(context,
				request_id, error_code, server_data);
	}
}

void
message_adaptor_unseal_message_reply_cb(message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int chatroom_id,
						message_adaptor_error_code_t **error_code,
						void *server_data)
{
	plugin_req_id_print();
	if (_service_adaptor_service_unseal_message_reply_cb) {
		_service_adaptor_service_unseal_message_reply_cb(context,
				request_id, chatroom_id, error_code, server_data);
	}
}

void
message_adaptor_save_call_log_reply_cb(message_adaptor_plugin_context_h context,
						long long int request_id,
						message_adaptor_error_code_t **error_code,
						void *server_data)
{
	plugin_req_id_print();
	if (_service_adaptor_service_save_call_log_reply_cb) {
		_service_adaptor_service_save_call_log_reply_cb(context,
				request_id, error_code, server_data);
	}
}

void
message_adaptor_current_time_reply_cb(message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int current_time_millis,
						message_adaptor_error_code_t **error_code,
						void *server_data)
{
	plugin_req_id_print();
	if (_service_adaptor_service_current_time_reply_cb) {
		_service_adaptor_service_current_time_reply_cb(context,
				request_id, current_time_millis, error_code, server_data);
	}
}

void
message_adaptor_typing_updated_cb(message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int chatroom_id,
						long long int *sender,
						char **state,
						int *contentType,
						int *refreshTime,
						message_adaptor_error_code_t **error_code,
						void *server_data)
{
	plugin_req_id_print();
	if (_service_adaptor_service_typing_updated_cb) {
		_service_adaptor_service_typing_updated_cb(context,
				request_id, chatroom_id, sender,
				state, contentType, refreshTime,
				error_code, server_data);
	}
}

void
message_adaptor_completion_cb(message_adaptor_plugin_context_h context,
						message_connection_state_t state,
						message_adaptor_error_code_t **error_code,
						void *server_data)
{
	if (_service_adaptor_service_completion_cb) {
		_service_adaptor_service_completion_cb(context,
				state, error_code, server_data);
	}
}


/* //------------------------------------------------------------------------
   // Functions implementations
   //------------------------------------------------------------------------ */

/* //////////////////////////////////////////////////////
   // Mandatory: External adaptor management function
   ////////////////////////////////////////////////////// */

EXPORT_API
message_adaptor_h message_adaptor_create(const char *plugins_dir)
{
	message_adaptor_h message_adaptor = (message_adaptor_h) malloc(sizeof(message_adaptor_t));
	if (NULL == message_adaptor) {
		return NULL;
	}

	message_adaptor->started = 0;
	message_adaptor->plugins_dir = strdup(plugins_dir);

	g_mutex_init(&message_adaptor->message_adaptor_mutex);
	g_mutex_init(&message_adaptor->plugins_mutex);
	g_mutex_init(&message_adaptor->adaptor_listeners_mutex);

	g_mutex_lock(&message_adaptor->adaptor_listeners_mutex);
	message_adaptor->adaptor_listeners = NULL;
	g_mutex_unlock(&message_adaptor->adaptor_listeners_mutex);

	g_mutex_lock(&message_adaptor->plugins_mutex);
	message_adaptor->plugins = NULL;
	g_mutex_unlock(&message_adaptor->plugins_mutex);

	return message_adaptor;

}

EXPORT_API
void message_adaptor_destroy(message_adaptor_h adaptor)
{
	if (NULL == adaptor) {
		message_adaptor_error("Invalid argument");
		return ;
	}

	g_mutex_lock(&adaptor->message_adaptor_mutex);
	if (adaptor->started) {
		message_adaptor_error("Message adaptor is running. Forcing stop before destroy");
		message_adaptor_stop(adaptor);
	}

	g_mutex_lock(&adaptor->plugins_mutex);
	if (NULL != adaptor->plugins) {
		g_list_free_full(adaptor->plugins, (GDestroyNotify) message_adaptor_plugin_unref);
		adaptor->plugins = NULL;
	}
	g_mutex_unlock(&adaptor->plugins_mutex);

	g_mutex_lock(&adaptor->adaptor_listeners_mutex);
	if (NULL != adaptor->adaptor_listeners) {
		g_list_free(adaptor->adaptor_listeners);
		adaptor->adaptor_listeners = NULL;
	}
	g_mutex_unlock(&adaptor->adaptor_listeners_mutex);

	free(adaptor->plugins_dir);
	adaptor->plugins_dir = NULL;

	g_mutex_unlock(&adaptor->message_adaptor_mutex);

	free(adaptor);
}

EXPORT_API
int message_adaptor_start(message_adaptor_h adaptor)
{
	message_adaptor_debug("Starting message adaptor");
	if (NULL == adaptor) {
		message_adaptor_error("Invalid argument");
		return MESSAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	g_mutex_lock(&adaptor->message_adaptor_mutex);
	int result = MESSAGE_ADAPTOR_ERROR_NONE;
	if (adaptor->started) {
		message_adaptor_error("Message adaptor is already started");
		result = MESSAGE_ADAPTOR_ERROR_START;
	} else {
		adaptor->started = 1;
		result = message_adaptor_load_plugins_from_directory(adaptor, adaptor->plugins_dir);
		if (MESSAGE_ADAPTOR_ERROR_NONE != result) {
			adaptor->started = 0;
			message_adaptor_error("Could not load plugins from directory");
		} else {
			message_adaptor_debug("Message adaptor started successfully");
		}
	}
	g_mutex_unlock(&adaptor->message_adaptor_mutex);

	return result;
}

/**
 * Stops message adaptor.
 */
EXPORT_API
int message_adaptor_stop(message_adaptor_h adaptor)
{
	if (NULL == adaptor) {
		message_adaptor_error("Invalid argument");
		return MESSAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	g_mutex_lock(&adaptor->message_adaptor_mutex);
	int result = MESSAGE_ADAPTOR_ERROR_NONE;
	if (!adaptor->started) {
		result = MESSAGE_ADAPTOR_ERROR_START;
	} else {
		if (NULL != adaptor->plugins) {
			g_mutex_lock(&adaptor->plugins_mutex);
			g_list_free_full(adaptor->plugins, (GDestroyNotify) message_adaptor_plugin_unref);
			adaptor->plugins = NULL;
			g_mutex_unlock(&adaptor->plugins_mutex);
		}
		adaptor->started = 0;
		message_adaptor_debug("Message adaptor stopped");
	}

	g_mutex_unlock(&adaptor->message_adaptor_mutex);
	return result;
}

/**
 * Registers plugin state listener
 */
	EXPORT_API
int message_adaptor_register_listener(message_adaptor_h adaptor, message_adaptor_listener_h listener)
{
	if ((NULL == adaptor) || (NULL == listener)) {
		message_adaptor_error("Invalid argument");
		return MESSAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	g_mutex_lock(&adaptor->adaptor_listeners_mutex);

	adaptor->adaptor_listeners = g_list_append(adaptor->adaptor_listeners, listener);

	g_mutex_unlock(&adaptor->adaptor_listeners_mutex);

	_service_adaptor_service_client_echo_cb =
		(message_adaptor_service_client_echo_cb)listener->client_echo_cb;
	_service_adaptor_service_create_chatroom_reply_cb =
		(message_adaptor_service_create_chatroom_reply_cb)listener->create_chatroom_reply_cb;
	_service_adaptor_service_change_chatroom_meta_reply_cb =
		(message_adaptor_service_change_chatroom_meta_reply_cb)listener->change_chatroom_meta_reply_cb;
	_service_adaptor_service_chat_reply_cb =
		(message_adaptor_service_chat_reply_cb)listener->chat_reply_cb;
	_service_adaptor_service_allow_chat_reply_cb =
		(message_adaptor_service_allow_chat_reply_cb)listener->allow_chat_reply_cb;
	_service_adaptor_service_get_all_unread_message_reply_cb =
		(message_adaptor_service_get_all_unread_message_reply_cb)listener->get_all_unread_message_reply_cb;
	_service_adaptor_service_forward_online_message_request_cb =
		(message_adaptor_service_forward_online_message_request_cb)listener->forward_online_message_request_cb;
	_service_adaptor_service_forward_unread_message_request_cb =
		(message_adaptor_service_forward_unread_message_request_cb)listener->forward_unread_message_request_cb;
	_service_adaptor_service_read_message_reply_cb =
		(message_adaptor_service_read_message_reply_cb)listener->read_message_reply_cb;
	_service_adaptor_service_invite_chat_reply_cb =
		(message_adaptor_service_invite_chat_reply_cb)listener->invite_chat_reply_cb;
	_service_adaptor_service_end_chat_reply_cb =
		(message_adaptor_service_end_chat_reply_cb)listener->end_chat_reply_cb;
	_service_adaptor_service_unseal_message_reply_cb =
		(message_adaptor_service_unseal_message_reply_cb)listener->unseal_message_reply_cb;
	_service_adaptor_service_save_call_log_reply_cb =
		(message_adaptor_service_save_call_log_reply_cb)listener->save_call_log_reply_cb;
	_service_adaptor_service_current_time_reply_cb =
		(message_adaptor_service_current_time_reply_cb)listener->current_time_reply_cb;
	_service_adaptor_service_typing_updated_cb =
		(message_adaptor_service_typing_updated_cb)listener->typing_updated_cb;
	_service_adaptor_service_completion_cb =
		(message_adaptor_service_completion_cb)listener->completion_cb;

	return MESSAGE_ADAPTOR_ERROR_NONE;
}

/**
 * Unregisters plugin state listener
 */
EXPORT_API
int message_adaptor_unregister_listener(message_adaptor_h adaptor, message_adaptor_listener_h listener)
{
	if ((NULL == adaptor) || (NULL == listener)) {
		message_adaptor_error("Invalid argument");
		return MESSAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	g_mutex_lock(&adaptor->adaptor_listeners_mutex);

	if (NULL == g_list_find(adaptor->adaptor_listeners, listener)) {
		g_mutex_unlock(&adaptor->adaptor_listeners_mutex);
		message_adaptor_error("Could not find listener");
		return MESSAGE_ADAPTOR_ERROR_NOT_FOUND;
	}

	adaptor->adaptor_listeners = g_list_remove(adaptor->adaptor_listeners, listener);

	g_mutex_unlock(&adaptor->adaptor_listeners_mutex);

	_service_adaptor_service_client_echo_cb = NULL;
	_service_adaptor_service_create_chatroom_reply_cb = NULL;
	_service_adaptor_service_change_chatroom_meta_reply_cb = NULL;
	_service_adaptor_service_chat_reply_cb = NULL;
	_service_adaptor_service_allow_chat_reply_cb = NULL;
	_service_adaptor_service_get_all_unread_message_reply_cb = NULL;
	_service_adaptor_service_forward_online_message_request_cb = NULL;
	_service_adaptor_service_forward_unread_message_request_cb = NULL;
	_service_adaptor_service_read_message_reply_cb = NULL;
	_service_adaptor_service_invite_chat_reply_cb = NULL;
	_service_adaptor_service_end_chat_reply_cb = NULL;
	_service_adaptor_service_unseal_message_reply_cb = NULL;
	_service_adaptor_service_save_call_log_reply_cb = NULL;
	_service_adaptor_service_current_time_reply_cb = NULL;
	_service_adaptor_service_typing_updated_cb = NULL;
	_service_adaptor_service_completion_cb = NULL;

	return MESSAGE_ADAPTOR_ERROR_NONE;
}

/* /////////////////////////////////////////////////////////////
   // Plugin create / destroy / ref. count / get plugin name
   ///////////////////////////////////////////////////////////// */
static message_adaptor_plugin_h message_adaptor_create_plugin(const char *plugin_path)
{
	if (NULL == plugin_path) {
		message_adaptor_error("Invalid argument");
		return NULL;
	}

	void *dl_handle = dlopen(plugin_path, RTLD_LAZY);
	if (NULL == dl_handle) {
		message_adaptor_error("Could not load plugin %s: %s", plugin_path, dlerror());
		return NULL;
	}

	message_adaptor_plugin_handle_h (*get_adaptee_handle)(void) = NULL;

	get_adaptee_handle = (message_adaptor_plugin_handle_h (*)(void))(dlsym(dl_handle, "create_plugin_handle"));
	if (NULL == get_adaptee_handle) {
		dlclose(dl_handle);
		message_adaptor_error("Could not get function pointer to create_plugin_handle");
		return NULL;
	}

	plugin_req_enter();
	message_adaptor_plugin_handle_h handle = get_adaptee_handle();
	plugin_req_exit_void();
	if (NULL == handle) {
		dlclose(dl_handle);
		message_adaptor_error("Could not get adaptee handle");
		return NULL;
	}

	message_adaptor_plugin_h plugin = (message_adaptor_plugin_h) calloc(1, sizeof(message_adaptor_plugin_t));
	if (NULL == plugin) {
		dlclose(dl_handle);
		message_adaptor_error("Could not create plugin object");
		return NULL;
	}

	message_adaptor_plugin_listener_h listener =
		(message_adaptor_plugin_listener_h) calloc(1, sizeof(message_adaptor_plugin_listener_t));
	if (NULL == listener) {
		free(plugin);
		dlclose(dl_handle);
		message_adaptor_error("Could not create listener object");
		return NULL;
	}

	plugin->path = g_strdup(plugin_path);
	plugin->handle = handle;
	plugin->dl_handle = dl_handle;
	plugin->ref_counter = 0;

	g_mutex_init(&plugin->ref_counter_mutex);
	g_mutex_init(&plugin->plugin_listener_mutex);

	plugin->connected = 0;

	g_mutex_init(&plugin->plugin_connect_mutex);
	g_cond_init(&plugin->plugin_connect_cond);

	listener->message_adaptor_client_echo = message_adaptor_client_echo_cb;
	listener->message_adaptor_create_chatroom_reply = message_adaptor_create_chatroom_reply_cb;
	listener->message_adaptor_change_chatroom_meta_reply = message_adaptor_change_chatroom_meta_reply_cb;
	listener->message_adaptor_chat_reply = message_adaptor_chat_reply_cb;
	listener->message_adaptor_allow_chat_reply = message_adaptor_allow_chat_reply_cb;
	listener->message_adaptor_get_all_unread_message_reply = message_adaptor_get_all_unread_message_reply_cb;
	listener->message_adaptor_forward_online_message_request = message_adaptor_forward_online_message_request_cb;
	listener->message_adaptor_forward_unread_message_request = message_adaptor_forward_unread_message_request_cb;
	listener->message_adaptor_read_message_reply = message_adaptor_read_message_reply_cb;
	listener->message_adaptor_invite_chat_reply = message_adaptor_invite_chat_reply_cb;
	listener->message_adaptor_end_chat_reply = message_adaptor_end_chat_reply_cb;
	listener->message_adaptor_unseal_message_reply = message_adaptor_unseal_message_reply_cb;
	listener->message_adaptor_save_call_log_reply = message_adaptor_save_call_log_reply_cb;
	listener->message_adaptor_current_time_reply = message_adaptor_current_time_reply_cb;
	listener->message_adaptor_typing_updated = message_adaptor_typing_updated_cb;
	listener->message_adaptor_completion = message_adaptor_completion_cb;

	plugin_req_enter();
	plugin->handle->set_listener(listener);
	plugin_req_exit_void();

	g_mutex_lock(&plugin->plugin_listener_mutex);
	plugin->plugin_listener = listener;
	g_mutex_unlock(&plugin->plugin_listener_mutex);

	return plugin;
}

static void message_adaptor_destroy_plugin(message_adaptor_plugin_h plugin)
{
	if (NULL == plugin) {
		message_adaptor_error("Invalid argument");
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

static int message_adaptor_load_plugins_from_directory(message_adaptor_h adaptor, const char *dir_path)
{
	char *plugin_path = NULL;
	DIR *dir = NULL;
	struct dirent dir_entry, *result = NULL;

	message_adaptor_debug("Starting load plugins from directory");

	if ((NULL == adaptor) || (NULL == dir_path)) {
		message_adaptor_error("Invalid argument");
		return MESSAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	dir = opendir(dir_path);
	if (NULL == dir) {
		message_adaptor_error("Could not open dir path (%s)", dir_path);
		return MESSAGE_ADAPTOR_ERROR_NOT_FOUND;
	}

	int ret = MESSAGE_ADAPTOR_ERROR_NONE;
	while (0 == (readdir_r(dir, &dir_entry, &result))) {

		if (NULL == result) {
			message_adaptor_error("Could not open directory %s", plugin_path);
			break;
		}

		if (dir_entry.d_type & DT_DIR) {
			continue;
		}

		plugin_path = g_strconcat(dir_path, "/", dir_entry.d_name, NULL);
		message_adaptor_plugin_h plugin = message_adaptor_create_plugin(plugin_path);

		if (NULL != plugin) {
			message_adaptor_debug("Loaded plugin: %s", plugin_path);
			plugin->adaptor = adaptor;
			message_adaptor_plugin_ref(plugin);
			g_mutex_lock(&adaptor->plugins_mutex);
			adaptor->plugins = g_list_append(adaptor->plugins, plugin);
			g_mutex_unlock(&adaptor->plugins_mutex);
		} else {
			message_adaptor_error("Could not load plugin %s", plugin_path);
		}

		free(plugin_path);
		plugin_path = NULL;
	}

	message_adaptor_debug("End load plugins from directory");
	closedir(dir);
	return ret;
}


static int message_adaptor_has_plugin(message_adaptor_h adaptor, message_adaptor_plugin_h plugin)
{
	if ((NULL == adaptor) || (NULL == plugin)) {
		message_adaptor_error("Invalid argument");
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

static void message_adaptor_plugin_ref(message_adaptor_plugin_h plugin)
{
	if (NULL == plugin) {
		message_adaptor_error("Invalid argument");
		return;
	}

	g_mutex_lock(&plugin->ref_counter_mutex);
	plugin->ref_counter = plugin->ref_counter + 1;
	if (NULL != plugin->handle) {
		message_adaptor_info("plugin name : %s, ref_counter: %d", plugin->handle->plugin_uri, plugin->ref_counter);
	} else {
		message_adaptor_info("ref_counter : %d", plugin->ref_counter);
	}
	g_mutex_unlock(&plugin->ref_counter_mutex);
}

static void message_adaptor_plugin_unref(message_adaptor_plugin_h plugin)
{
	if (NULL == plugin) {
		message_adaptor_error("Invalid argument");
		return ;
	}

	int should_destroy = 0;

	g_mutex_lock(&plugin->ref_counter_mutex);
	plugin->ref_counter = plugin->ref_counter - 1;

	if (NULL != plugin->handle) {
		message_adaptor_info("plugin name : %s, ref_counter: %d", plugin->handle->plugin_uri, plugin->ref_counter);
	} else {
		message_adaptor_info("ref_counter : %d", plugin->ref_counter);
	}

	if (0 >= plugin->ref_counter) {
		should_destroy = 1;
	}
	g_mutex_unlock(&plugin->ref_counter_mutex);

	if (should_destroy) {
		message_adaptor_debug("Plugin is being destroyed");
		message_adaptor_destroy_plugin(plugin);
	}
}


/**
 * Refresh access token
 */
EXPORT_API
message_error_code_t message_adaptor_refresh_access_token(message_adaptor_plugin_context_h context,
						const char *new_access_token)
{
	if ((NULL == context) || (NULL == new_access_token) || (0 >= strlen(new_access_token))) {
		return MESSAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}
	if ((NULL == context->access_token) || (0 >= strlen(context->access_token))) {
		return MESSAGE_ADAPTOR_ERROR_NOT_AUTHORIZED;
	}

	free(context->access_token);
	context->access_token = NULL;
	context->access_token = strdup(new_access_token);

	return MESSAGE_ADAPTOR_ERROR_NONE;
}

EXPORT_API
message_error_code_t message_adaptor_refresh_uid(message_adaptor_plugin_context_h context,
						const char *new_uid)
{
	if ((NULL == context) || (NULL == new_uid) || (0 >= strlen(new_uid))) {
		return MESSAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}
	message_adaptor_debug("New uid : %s", new_uid);

	free(context->uid);
	context->uid = NULL;
	context->uid = strdup(new_uid);

	char *pend = NULL;
	context->duid = (long long int) strtoll(new_uid, &pend, 10);

	return MESSAGE_ADAPTOR_ERROR_NONE;
}

/* //////////////////////////////////////////////////////
   // Create / Destroy error code
   ////////////////////////////////////////////////////// */
message_adaptor_error_code_h message_adaptor_create_error_code(const char *code, const char *msg)
{
	if (NULL == code || NULL == msg) {
		return NULL;
	}

	message_adaptor_error_code_h error_code = (message_adaptor_error_code_h) malloc(sizeof(message_adaptor_error_code_t));

	if (NULL != error_code) {
		error_code->code = strdup(code);
		error_code->msg = strdup(msg);
	}

	return error_code;
}

void message_adaptor_destroy_error_code(message_adaptor_error_code_h *error_code)
{
	if ((NULL != error_code) && (NULL != (*error_code))) {
		free((*error_code)->msg);
		(*error_code)->msg = NULL;
		free(*error_code);
		*error_code = NULL;
	}
}

void _set_error_code(message_adaptor_error_code_h *error, const char *code, const char *msg)
{
	if (NULL == error) {
		return;
	}

	message_adaptor_error_code_h error_code = (message_adaptor_error_code_h) calloc(1, sizeof(message_adaptor_error_code_t));

	if (NULL != error_code) {
		error_code->code = strdup(code);
		error_code->msg = strdup(msg);
	}

	*error = error_code;
}

void message_adaptor_destroy_chat_msg_s(message_adaptor_chat_msg_s *msg)
{
	if (NULL == msg) {
		return;
	}
	free(msg->chatmsg);
	free(msg);
}
void message_adaptor_destroy_processed_msg_s(message_adaptor_processed_msg_s *msg)
{
	if (NULL == msg) {
		return;
	}
	free(msg);
}
void message_adaptor_destroy_delivery_ack_s(message_adaptor_delivery_ack_s *ack)
{
	if (NULL == ack) {
		return;
	}
	free(ack);
}
void message_adaptor_destroy_read_ack_s(message_adaptor_read_ack_s *ack)
{
	if (NULL == ack) {
		return;
	}
	free(ack);
}
void message_adaptor_destroy_ordered_chat_member_s(message_adaptor_ordered_chat_member_s *member)
{
	if (NULL == member) {
		return;
	}
	free(member->name);
	free(member);
}
void message_adaptor_destroy_inbox_message_s(message_adaptor_inbox_message_s *msg)
{
	if (NULL == msg) {
		return;
	}
	free(msg->chatMsg);
	free(msg);
}
void message_adaptor_destroy_phone_number_s(message_adaptor_phone_number_s *num)
{
	if (NULL == num) {
		return;
	}
	free(num->phonenumber);
	free(num->ccc);
	free(num);
}
void message_adaptor_destroy_chat_id_s(message_adaptor_chat_id_s *id)
{
	if (NULL == id) {
		return;
	}
	free(id->msisdn);
	free(id);
}
void message_adaptor_destroy_end_chat_s(message_adaptor_end_chat_s *msg)
{
	if (NULL == msg) {
		return;
	}
	free(msg);
}


/* //////////////////////////////////////////////////////
   // Plugin context create / destroy
   ////////////////////////////////////////////////////// */

message_adaptor_plugin_context_h message_adaptor_create_plugin_context(message_adaptor_plugin_h plugin,
						char *plugin_uri,
						char *duid,
						char *access_token,
						char *app_id,
						int service_id)
{
	message_adaptor_debug("Starting message_adaptor_create_plugin_context");

	if (NULL == plugin) {
		message_adaptor_error("Invalid argument");
		return NULL;
	}

	if (NULL != plugin->handle) {
		message_adaptor_plugin_context_h plugin_context = NULL;

		plugin_req_enter();
		plugin->handle->create_context(&plugin_context, duid, access_token, app_id, service_id);
		plugin_req_exit_void();

		if (NULL != plugin_context) {
			plugin_context->plugin_uri = strdup(plugin->handle->plugin_uri);
			plugin_context->connection_policy = MESSAGE_CONNECTION_POLICY_AUTO;
		} else {
			message_adaptor_error("plugin context info message_context set error");
			message_adaptor_error("plugin context info message_plugin set error");
		}
		return plugin_context;
	} else {
		message_adaptor_error("Plugin handle is null");
	}

	message_adaptor_debug("End message_adaptor_create_plugin_context");
	return NULL;
}

void message_adaptor_destroy_plugin_context(message_adaptor_plugin_h plugin, message_adaptor_plugin_context_h plugin_context)
{
	message_adaptor_warning("Destroy plugin context");

	if ((NULL == plugin) || (NULL == plugin_context)) {
		message_adaptor_error("Invalid argument");
		return;
	}

	if (NULL != plugin->handle) {
		plugin_req_enter();
		plugin->handle->destroy_context(plugin_context);
		plugin_req_exit_void();
	} else {
		message_adaptor_error("Plugin handle is null");
	}
}

message_error_code_t message_adaptor_set_connected(message_adaptor_plugin_h plugin, int connected)
{
	if (NULL == plugin) {
		message_adaptor_error("plugin is NULL");
		return MESSAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	g_mutex_lock(&plugin->plugin_connect_mutex);
	plugin->connected = connected;
	g_cond_signal(&plugin->plugin_connect_cond);
	g_mutex_unlock(&plugin->plugin_connect_mutex);

	return MESSAGE_ADAPTOR_ERROR_NONE;
}

message_error_code_t message_adaptor_wait_connected(message_adaptor_plugin_h plugin)
{
	if (NULL == plugin) {
		message_adaptor_error("plugin is NULL");
		return MESSAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	gint64 timeout = g_get_monotonic_time() + 10 * G_TIME_SPAN_SECOND;
	g_mutex_lock(&plugin->plugin_connect_mutex);

	while (0 == plugin->connected) {
		if (!g_cond_wait_until(&plugin->plugin_connect_cond, &plugin->plugin_connect_mutex, timeout)) {
			g_mutex_unlock(&plugin->plugin_connect_mutex);
			return MESSAGE_ADAPTOR_ERROR_CONNECT;
		}
	}

	g_mutex_unlock(&plugin->plugin_connect_mutex);

	return MESSAGE_ADAPTOR_ERROR_NONE;
}

/* //////////////////////////////////////////////////////
   // Get plugin by plugin name
   ////////////////////////////////////////////////////// */
message_adaptor_plugin_h message_adaptor_get_plugin_by_name(message_adaptor_h adaptor, const char *plugin_uri)
{
	message_adaptor_warning("Starting message_adaptor_get_plugin_by_name");

	if ((NULL == adaptor)) {
		message_adaptor_error("adaptor is NULL");
	}

	if ((NULL == plugin_uri)) {
		message_adaptor_error("adaptor is NULL");
	} else {
		message_adaptor_error("plugin name : %s", plugin_uri);
	}

	if ((NULL == adaptor) || (NULL == plugin_uri)) {
		message_adaptor_error("Invalid argument");
		return NULL;
	}

	message_adaptor_plugin_h plugin = NULL;
	g_mutex_lock(&adaptor->plugins_mutex);
	int count = g_list_length(adaptor->plugins);
	int i = 0;
	message_adaptor_error("count : %d", count);
	for (i = 0; i < count; i++) {
		message_adaptor_plugin_h temp_plugin = (message_adaptor_plugin_h)g_list_nth_data(adaptor->plugins, i);
		if (NULL != temp_plugin) {
			message_adaptor_error("temp_plugin name : %s", temp_plugin->handle->plugin_uri);
			if (0 == strcmp(temp_plugin->handle->plugin_uri, plugin_uri)) {
				message_adaptor_plugin_ref(temp_plugin);
				plugin = temp_plugin;
				g_mutex_unlock(&adaptor->plugins_mutex);
				return plugin;
			}
		} else {
			message_adaptor_error("NULL != temp_plugin");
		}
	}
	g_mutex_unlock(&adaptor->plugins_mutex);

	if (NULL == plugin) {
		message_adaptor_debug("Plugin is not found by name");
	}

	return plugin;
}

/* //////////////////////////////////////////////////////
   // Plugin load / unload / get plugin list
   ////////////////////////////////////////////////////// */
int message_adaptor_load_plugin(message_adaptor_h adaptor, const char *plugin_path)
{
	if ((NULL == adaptor) || (NULL == plugin_path)) {
		message_adaptor_error("Invalid argument");
		return MESSAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (!adaptor->started) {
		message_adaptor_error("Storage adaptor is not started");
		return MESSAGE_ADAPTOR_ERROR_START;
	}

	message_adaptor_plugin_h plugin = message_adaptor_create_plugin(plugin_path);
	if (NULL == plugin) {
		message_adaptor_error("Could not load plugin %s", plugin_path);
		return MESSAGE_ADAPTOR_ERROR_CREATE;
	}

	plugin->adaptor = adaptor;
	message_adaptor_plugin_ref(plugin);

	g_mutex_lock(&adaptor->plugins_mutex);
	adaptor->plugins = g_list_append(adaptor->plugins, plugin);
	g_mutex_unlock(&adaptor->plugins_mutex);

	return MESSAGE_ADAPTOR_ERROR_NONE;
}

int message_adaptor_unload_plugin(message_adaptor_h adaptor, message_adaptor_plugin_h plugin)
{
	if ((NULL == adaptor) || (NULL == plugin)) {
		message_adaptor_error("Invalid argument");
		return MESSAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (!adaptor->started) {
		message_adaptor_error("Storage adaptor is not started");
		return MESSAGE_ADAPTOR_ERROR_START;
	}

	if (!message_adaptor_has_plugin(adaptor, plugin)) {
		message_adaptor_error("Storage adaptor has no plugin");
		return MESSAGE_ADAPTOR_ERROR_NOT_FOUND;
	}

	plugin->adaptor = NULL;

	g_mutex_lock(&adaptor->plugins_mutex);
	adaptor->plugins = g_list_remove(adaptor->plugins, plugin);
	g_mutex_unlock(&adaptor->plugins_mutex);
	message_adaptor_plugin_unref(plugin);

	return MESSAGE_ADAPTOR_ERROR_NONE;
}

GList *message_adaptor_get_plugins(message_adaptor_h adaptor)
{
	if (NULL == adaptor) {
		message_adaptor_error("Invalid argument");
		return NULL;
	}

	GList *plugins = NULL;
	g_mutex_lock(&adaptor->plugins_mutex);
	int plugins_count = g_list_length(adaptor->plugins);
	int i;
	for (i = 0; i < plugins_count; i++) {
		message_adaptor_plugin_h plugin = (message_adaptor_plugin_h) g_list_nth_data(adaptor->plugins, i);
		if (NULL != plugin) {
			message_adaptor_plugin_ref(plugin);
			plugins = g_list_append(plugins, plugin);
		}
	}
	g_mutex_unlock(&adaptor->plugins_mutex);

	return plugins;

}

/* ////////////////////////////////////////////////////////////
   // Adaptor Plugin call Functions
   //////////////////////////////////////////////////////////// */

/**
* @brief Set server information for Message Plugin
*
* @param[in]	plugin				specifies Message Adaptor Plugin handle
* @param[in]	context				specifies Message Adaptor Plugin Context handle
* @param[in]	server_info			specifies server information for Message Plugin
* @param[in]	request				specifies optional parameter
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in message_error_code_t - MESSAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
message_error_code_t message_adaptor_set_server_info(message_adaptor_plugin_h plugin,
						message_adaptor_plugin_context_h context,
						GHashTable *server_info,
						void *request,
						message_adaptor_error_code_h *error_code,
						void *response)
{
	if ((NULL == plugin) || (NULL == context)) {
		message_adaptor_error("Invalid argument""(plugin: %p, context: %p)", plugin, context);

		_set_error_code(error_code, "14", "Invalid argument (plugin or context)");

		return MESSAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		message_adaptor_error("Plugin handle is null");
		_set_error_code(error_code, "13", "Plugin handle is null");

		return MESSAGE_ADAPTOR_ERROR_INVALID_HANDLE;
	}

	message_error_code_t ret;
	plugin_req_enter();
	ret = plugin->handle->set_server_info(context, server_info, request, error_code, response);
	plugin_req_exit(ret, plugin, error_code);

	return ret;
}


/*TODO fill this area */
EXPORT_API
message_error_code_t message_adaptor_get_key(message_adaptor_plugin_h plugin,
						message_adaptor_plugin_context_h context,
						char **in_gcmid,
						char **in_del_gcm_id,
						char **key,
						char **expiredkey,
						char **gpbauthkey,
						message_adaptor_error_code_t **error_code,
						void **server_data)
{
	if ((NULL == plugin) || (NULL == context)) {
		message_adaptor_error("Invalid argument");

		_set_error_code(error_code, "14", "Invalid argument (plugin or context)");

		return MESSAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		message_adaptor_error("Plugin handle is null");

		_set_error_code(error_code, "13", "Plugin handle is null");

		return MESSAGE_ADAPTOR_ERROR_INVALID_HANDLE;
	}

	if (NULL == context->uid) {
		message_adaptor_error("UID is null");

		_set_error_code(error_code, "14", "Invalid argument (uid)");

		return MESSAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	message_error_code_t ret;
	plugin_req_enter();
	ret = plugin->handle->get_key(context, &(context->uid), in_gcmid, in_del_gcm_id, key, expiredkey, gpbauthkey, error_code, server_data);
	plugin_req_exit(ret, plugin, error_code);

	return ret;
}

EXPORT_API
message_error_code_t message_adaptor_request_chat_id(message_adaptor_plugin_h plugin,
						message_adaptor_plugin_context_h context,
						message_adaptor_phone_number_s **phone_numbers,
						unsigned int phone_numbers_len,
						void *user_data,
						message_adaptor_chat_id_s ***chat_ids,
						unsigned int *chat_ids_len,
						message_adaptor_error_code_t **error_code,
						void **server_data)
{
	message_adaptor_info("%s() Start!!!", __FUNCTION__);

	if ((NULL == plugin) || (NULL == context)) {
		message_adaptor_error("Invalid argument");

		_set_error_code(error_code, "14", "Invalid argument (plugin or context)");

		return MESSAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		message_adaptor_error("Plugin handle is null");

		_set_error_code(error_code, "13", "Plugin handle is null");

		return MESSAGE_ADAPTOR_ERROR_INVALID_HANDLE;
	}

	if (NULL == context->uid) {
		message_adaptor_error("UID is null");

		_set_error_code(error_code, "14", "Invalid argument (uid)");

		return MESSAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	message_error_code_t ret;
	plugin_req_enter();
	ret = plugin->handle->request_chat_id(context, context->uid, phone_numbers, phone_numbers_len, user_data, chat_ids, chat_ids_len, error_code, server_data);
	plugin_req_exit(ret, plugin, error_code);

	return ret;
}

EXPORT_API
message_error_code_t message_adaptor_request_msisdn(message_adaptor_plugin_h plugin,
						message_adaptor_plugin_context_h context,
						long long int *chat_ids,
						unsigned int chat_ids_len,
						void *user_data,
						message_adaptor_chat_id_s ***msisdns,
						unsigned int *msisdns_len,
						message_adaptor_error_code_t **error_code,
						void **server_data)
{
	message_adaptor_info("%s() Start!!!", __FUNCTION__);

	if ((NULL == plugin) || (NULL == context)) {
		message_adaptor_error("Invalid argument");

		_set_error_code(error_code, "14", "Invalid argument (plugin or context)");

		return MESSAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		message_adaptor_error("Plugin handle is null");

		_set_error_code(error_code, "13", "Plugin handle is null");

		return MESSAGE_ADAPTOR_ERROR_INVALID_HANDLE;
	}

	if (NULL == context->uid) {
		message_adaptor_error("UID is null");

		_set_error_code(error_code, "14", "Invalid argument (uid)");

		return MESSAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	message_error_code_t ret;
	plugin_req_enter();
	ret = plugin->handle->request_msisdn(context, context->uid, chat_ids, chat_ids_len, user_data, msisdns, msisdns_len, error_code, server_data);
	plugin_req_exit(ret, plugin, error_code);

	return ret;
}

/*TODO fill this area */
EXPORT_API
message_error_code_t message_adaptor_channel_auth_request(message_adaptor_plugin_h plugin,
						message_adaptor_plugin_context_h context,
						long long int request_id,
						const int timeout_second,
						void *user_data,
						message_adaptor_error_code_t **error_code,
						void *server_data)
{
	if ((NULL == plugin) || (NULL == context)) {
		message_adaptor_error("Invalid argument");

		_set_error_code(error_code, "14", "Invalid argument (plugin or context)");

		return MESSAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		message_adaptor_error("Plugin handle is null");

		_set_error_code(error_code, "13", "Plugin handle is null");

		return MESSAGE_ADAPTOR_ERROR_INVALID_HANDLE;
	}
	plugin_req_id_print();

	message_error_code_t ret;
	plugin_req_enter();
	ret = plugin->handle->channel_auth_request(context, request_id, context->uid,
			context->duid, context->app_id, context->access_token,
			timeout_second, user_data, error_code, server_data);
	plugin_req_exit(ret, plugin, error_code);

	return ret;
}

EXPORT_API
message_error_code_t message_adaptor_client_echo_reply(message_adaptor_plugin_h plugin,
						message_adaptor_plugin_context_h context,
						long long int request_id,
						message_adaptor_error_code_t **error_code,
						void *server_data)
{
	if ((NULL == plugin) || (NULL == context)) {
		message_adaptor_error("Invalid argument");

		_set_error_code(error_code, "14", "Invalid argument (plugin or context)");

		return MESSAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		message_adaptor_error("Plugin handle is null");

		_set_error_code(error_code, "13", "Plugin handle is null");

		return MESSAGE_ADAPTOR_ERROR_INVALID_HANDLE;
	}
	plugin_req_id_print();

	message_error_code_t ret;
	plugin_req_enter();
	ret = plugin->handle->client_echo_reply(context, &request_id, error_code, server_data);
	plugin_req_exit(ret, plugin, error_code);

	return ret;
}

EXPORT_API
message_error_code_t message_adaptor_create_chatroom_request(message_adaptor_plugin_h plugin,
						message_adaptor_plugin_context_h context,
						long long int request_id,
						int chat_type,
						long long int **receivers,
						unsigned int receivers_len,
						const char *chatroom_title,
						message_adaptor_error_code_t **error_code,
						void *user_data)
{
	if ((NULL == plugin) || (NULL == context)) {
		message_adaptor_error("Invalid argument");

		_set_error_code(error_code, "14", "Invalid argument (plugin or context)");

		return MESSAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		message_adaptor_error("Plugin handle is null");

		_set_error_code(error_code, "13", "Plugin handle is null");

		return MESSAGE_ADAPTOR_ERROR_INVALID_HANDLE;
	}
	plugin_req_id_print();

	message_error_code_t ret;
	plugin_req_enter();
	ret = plugin->handle->create_chatroom_request(context, &request_id, &chat_type, receivers,
			(int *)&receivers_len, chatroom_title, error_code, user_data);
	plugin_req_exit(ret, plugin, error_code);

	return ret;
}

EXPORT_API
message_error_code_t message_adaptor_change_chatroom_meta_request(message_adaptor_plugin_h plugin,
						message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int chatroom_id,
						const char *chatroom_title,
						int default_message_ttl,
						message_adaptor_error_code_t **error_code,
						void *user_data)
{
	if ((NULL == plugin) || (NULL == context)) {
		message_adaptor_error("Invalid argument");

		_set_error_code(error_code, "14", "Invalid argument (plugin or context)");

		return MESSAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		message_adaptor_error("Plugin handle is null");

		_set_error_code(error_code, "13", "Plugin handle is null");

		return MESSAGE_ADAPTOR_ERROR_INVALID_HANDLE;
	}
	plugin_req_id_print();

	message_error_code_t ret;
	plugin_req_enter();
	ret = plugin->handle->change_chatroom_meta_request(context, request_id, chatroom_id, chatroom_title,
			default_message_ttl, error_code, user_data);
	plugin_req_exit(ret, plugin, error_code);

	return ret;
}


EXPORT_API
message_error_code_t message_adaptor_chat_request(message_adaptor_plugin_h plugin,
						message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int chatroom_id,
						message_adaptor_chat_msg_s **chat_msgs,
						unsigned int chat_msgs_len,
						message_adaptor_error_code_t **error_code,
						void *user_data)
{
	if ((NULL == plugin) || (NULL == context)) {
		message_adaptor_error("Invalid argument");

		_set_error_code(error_code, "14", "Invalid argument (plugin or context)");

		return MESSAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		message_adaptor_error("Plugin handle is null");

		_set_error_code(error_code, "13", "Plugin handle is null");

		return MESSAGE_ADAPTOR_ERROR_INVALID_HANDLE;
	}
	plugin_req_id_print();

	message_error_code_t ret;
	plugin_req_enter();
	ret = plugin->handle->chat_request(context, &request_id, &chatroom_id, chat_msgs[0], error_code, user_data);
	plugin_req_exit(ret, plugin, error_code);

	return ret;
}

EXPORT_API
message_error_code_t message_adaptor_allow_chat_request(message_adaptor_plugin_h plugin,
						message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int chatroom_id,
						bool is_auto_allow,
						int max_count,
						bool need_delivery_ack,
						long long int delivery_ack_timestamp,
						bool need_read_ack,
						long long int last_read_ack_timestamp,
						bool need_ordered_chat_member_list,
						message_adaptor_error_code_t **error_code,
						void *user_data)
{
	if ((NULL == plugin) || (NULL == context)) {
		message_adaptor_error("Invalid argument");

		_set_error_code(error_code, "14", "Invalid argument (plugin or context)");

		return MESSAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		message_adaptor_error("Plugin handle is null");

		_set_error_code(error_code, "13", "Plugin handle is null");

		return MESSAGE_ADAPTOR_ERROR_INVALID_HANDLE;
	}
	plugin_req_id_print();

	message_error_code_t ret;
	plugin_req_enter();
	ret = plugin->handle->allow_chat_request(context,
			&request_id, &chatroom_id, is_auto_allow, max_count,
			need_delivery_ack, delivery_ack_timestamp,
			need_read_ack, last_read_ack_timestamp,
			need_ordered_chat_member_list,
			error_code, user_data);
	plugin_req_exit(ret, plugin, error_code);

	return ret;
}


EXPORT_API
message_error_code_t message_adaptor_get_all_unread_message_request(message_adaptor_plugin_h plugin,
						message_adaptor_plugin_context_h context,
						long long int request_id,
						int max_count,
						message_adaptor_error_code_t **error_code,
						void *user_data)
{
	if ((NULL == plugin) || (NULL == context)) {
		message_adaptor_error("Invalid argument");

		_set_error_code(error_code, "14", "Invalid argument (plugin or context)");

		return MESSAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		message_adaptor_error("Plugin handle is null");

		_set_error_code(error_code, "13", "Plugin handle is null");

		return MESSAGE_ADAPTOR_ERROR_INVALID_HANDLE;
	}
	plugin_req_id_print();

	message_error_code_t ret;
	plugin_req_enter();
	ret = plugin->handle->get_all_unread_message_request(context, &request_id, &max_count, error_code, user_data);
	plugin_req_exit(ret, plugin, error_code);

	return ret;
}


EXPORT_API
message_error_code_t message_adaptor_forward_online_message_reply(message_adaptor_plugin_h plugin,
						message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int chatroom_id,
						bool mark_as_read,
						message_adaptor_error_code_t **error_code,
						void *user_data)
{
	if ((NULL == plugin) || (NULL == context)) {
		message_adaptor_error("Invalid argument");

		_set_error_code(error_code, "14", "Invalid argument (plugin or context)");

		return MESSAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		message_adaptor_error("Plugin handle is null");

		_set_error_code(error_code, "13", "Plugin handle is null");

		return MESSAGE_ADAPTOR_ERROR_INVALID_HANDLE;
	}
	plugin_req_id_print();

	message_error_code_t ret;
	plugin_req_enter();
	ret = plugin->handle->forward_online_message_reply(context, &request_id, &chatroom_id, &mark_as_read, error_code, user_data);
	plugin_req_exit(ret, plugin, error_code);

	return ret;
}

EXPORT_API
message_error_code_t message_adaptor_forward_unread_message_reply(message_adaptor_plugin_h plugin,
						message_adaptor_plugin_context_h context,
						long long int request_id,
						const char *next_pagination_key,
						int max_count,
						message_adaptor_error_code_t **error_code,
						void *user_data)
{
	if ((NULL == plugin) || (NULL == context)) {
		message_adaptor_error("Invalid argument");

		_set_error_code(error_code, "14", "Invalid argument (plugin or context)");

		return MESSAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		message_adaptor_error("Plugin handle is null");

		_set_error_code(error_code, "13", "Plugin handle is null");

		return MESSAGE_ADAPTOR_ERROR_INVALID_HANDLE;
	}
	plugin_req_id_print();

	message_error_code_t ret;
	plugin_req_enter();
	ret = plugin->handle->forward_unread_message_reply(context, &request_id, &next_pagination_key, &max_count, error_code, user_data);
	plugin_req_exit(ret, plugin, error_code);

	return ret;
}

EXPORT_API
message_error_code_t message_adaptor_read_message_request(message_adaptor_plugin_h plugin,
						message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int chatroom_id,
						message_inboxentry_t *inbox_msg,
						message_adaptor_error_code_t **error_code,
						void *user_data)
{
	if ((NULL == plugin) || (NULL == context)) {
		message_adaptor_error("Invalid argument");

		_set_error_code(error_code, "14", "Invalid argument (plugin or context)");

		return MESSAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		message_adaptor_error("Plugin handle is null");

		_set_error_code(error_code, "13", "Plugin handle is null");

		return MESSAGE_ADAPTOR_ERROR_INVALID_HANDLE;
	}
	plugin_req_id_print();

	message_error_code_t ret;
	plugin_req_enter();
	ret = plugin->handle->read_message_request(context, &request_id, &chatroom_id,
			inbox_msg, error_code, user_data);
	plugin_req_exit(ret, plugin, error_code);

	return ret;
}

EXPORT_API
message_error_code_t message_adaptor_invite_chat_request(message_adaptor_plugin_h plugin,
						message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int chatroom_id,
						long long int *inviting_members,
						unsigned int inviting_members_len,
						message_adaptor_error_code_t **error_code,
						void *user_data)
{
	if ((NULL == plugin) || (NULL == context)) {
		message_adaptor_error("Invalid argument");

		_set_error_code(error_code, "14", "Invalid argument (plugin or context)");

		return MESSAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		message_adaptor_error("Plugin handle is null");

		_set_error_code(error_code, "13", "Plugin handle is null");

		return MESSAGE_ADAPTOR_ERROR_INVALID_HANDLE;
	}
	plugin_req_id_print();

	message_error_code_t ret;
	plugin_req_enter();
	ret = plugin->handle->invite_request(context, &request_id, &chatroom_id,
			inviting_members, (int *)&inviting_members_len, error_code, user_data);
	plugin_req_exit(ret, plugin, error_code);

	return ret;
}

EXPORT_API
message_error_code_t message_adaptor_end_chat_request(message_adaptor_plugin_h plugin,
						message_adaptor_plugin_context_h context,
						long long int request_id,
						message_adaptor_end_chat_s **end_chats,
						unsigned int end_chats_len,
						message_adaptor_error_code_t **error_code,
						void *user_data)
{
	if ((NULL == plugin) || (NULL == context)) {
		message_adaptor_error("Invalid argument");

		_set_error_code(error_code, "14", "Invalid argument (plugin or context)");

		return MESSAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		message_adaptor_error("Plugin handle is null");

		_set_error_code(error_code, "13", "Plugin handle is null");

		return MESSAGE_ADAPTOR_ERROR_INVALID_HANDLE;
	}
	plugin_req_id_print();

	message_error_code_t ret;
	plugin_req_enter();
	ret = plugin->handle->end_chat_request(context, &request_id, end_chats, (int *)&end_chats_len, error_code, user_data);
	plugin_req_exit(ret, plugin, error_code);

	return ret;
}

EXPORT_API
message_error_code_t message_adaptor_unseal_message_request(message_adaptor_plugin_h plugin,
						message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int chatroom_id,
						long long int sender_id,
						long long int message_id,
						const char *message_detail,
						message_adaptor_error_code_t **error_code,
						void *user_data)
{
	if ((NULL == plugin) || (NULL == context)) {
		message_adaptor_error("Invalid argument");
		_set_error_code(error_code, "14", "Invalid argument (plugin or context)");

		return MESSAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		message_adaptor_error("Plugin handle is null");

		_set_error_code(error_code, "13", "Plugin handle is null");

		return MESSAGE_ADAPTOR_ERROR_INVALID_HANDLE;
	}
	plugin_req_id_print();

	message_error_code_t ret;
	plugin_req_enter();
	ret = plugin->handle->unseal_message_request(context, &request_id,
			&chatroom_id, &sender_id, &message_id, message_detail, error_code, user_data);
	plugin_req_exit(ret, plugin, error_code);

	return ret;
}

EXPORT_API
message_error_code_t message_adaptor_save_call_log_request(message_adaptor_plugin_h plugin,
						message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int chatroom_id,
						const char *call_id,
						const char *call_log_type,
						long long int call_sender_id,
						long long int call_receiver_id,
						int conversaction_second,
						message_adaptor_error_code_t **error_code,
						void *user_data)
{
	if ((NULL == plugin) || (NULL == context)) {
		message_adaptor_error("Invalid argument");
		_set_error_code(error_code, "14", "Invalid argument (plugin or context)");

		return MESSAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		message_adaptor_error("Plugin handle is null");
		_set_error_code(error_code, "13", "Plugin handle is null");

		return MESSAGE_ADAPTOR_ERROR_INVALID_HANDLE;
	}
	plugin_req_id_print();

	message_error_code_t ret;
	plugin_req_enter();
	ret = plugin->handle->save_call_log_request(context, &request_id, &chatroom_id, &call_id, &call_log_type, &call_sender_id, &call_receiver_id, &conversaction_second, error_code, user_data);
	plugin_req_exit(ret, plugin, error_code);

	return ret;
}

EXPORT_API
message_error_code_t message_adaptor_current_time_request(message_adaptor_plugin_h plugin,
						message_adaptor_plugin_context_h context,
						long long int request_id,
						message_adaptor_error_code_t **error_code,
						void *user_data)
{
	if ((NULL == plugin) || (NULL == context)) {
		message_adaptor_error("Invalid argument");

		_set_error_code(error_code, "14", "Invalid argument (plugin or context)");

		return MESSAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		message_adaptor_error("Plugin handle is null");

		_set_error_code(error_code, "13", "Plugin handle is null");

		return MESSAGE_ADAPTOR_ERROR_INVALID_HANDLE;
	}
	plugin_req_id_print();

	message_error_code_t ret;
	plugin_req_enter();
	ret = plugin->handle->current_time_request(context, &request_id, error_code, user_data);
	plugin_req_exit(ret, plugin, error_code);

	return ret;
}

EXPORT_API
message_error_code_t message_adaptor_is_typing(message_adaptor_plugin_h plugin,
						message_adaptor_plugin_context_h context,
						long long int *request_id,
						long long int *chatroom_id,
						char **state,
						int *chat_type,
						int *refreshtime,
						message_adaptor_error_code_t **error_code,
						void *user_data)
{
	if ((NULL == plugin) || (NULL == context)) {
		message_adaptor_error("Invalid argument");

		_set_error_code(error_code, "14", "Invalid argument (plugin or context)");

		return MESSAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		message_adaptor_error("Plugin handle is null");

		_set_error_code(error_code, "13", "Plugin handle is null");

		return MESSAGE_ADAPTOR_ERROR_INVALID_HANDLE;
	}

	message_error_code_t ret;
	plugin_req_enter();
	ret = plugin->handle->is_typing(context, request_id, chatroom_id,
			state, chat_type, refreshtime, error_code, user_data);
	plugin_req_exit(ret, plugin, error_code);

	return ret;
}

message_error_code_t message_adaptor_connect(message_adaptor_plugin_h plugin,
						message_adaptor_plugin_context_h context,
						message_adaptor_error_code_h *error_code)
{
	if ((NULL == plugin) || (NULL == context)) {
		message_adaptor_error("Invalid argument");

		_set_error_code(error_code, "14", "Invalid argument (plugin or context)");

		return MESSAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		message_adaptor_error("Plugin handle is null");

		_set_error_code(error_code, "13", "Plugin handle is null");

		return MESSAGE_ADAPTOR_ERROR_INVALID_HANDLE;
	}

	message_error_code_t ret;
	plugin_req_enter();
	ret = plugin->handle->connect_to_server(context);
	plugin_req_exit(ret, plugin, error_code);

	return ret;
}

message_error_code_t message_adaptor_disconnect(message_adaptor_plugin_h plugin,
						message_adaptor_plugin_context_h context,
						message_adaptor_error_code_h *error_code)
{
	if ((NULL == plugin) || (NULL == context)) {
		message_adaptor_error("Invalid argument");

		_set_error_code(error_code, "14", "Invalid argument (plugin or context)");

		return MESSAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		message_adaptor_error("Plugin handle is null");

		_set_error_code(error_code, "13", "Plugin handle is null");

		return MESSAGE_ADAPTOR_ERROR_INVALID_HANDLE;
	}

	message_error_code_t ret;
	plugin_req_enter();
	ret = plugin->handle->disconnect_to_server(context);
	plugin_req_exit(ret, plugin, error_code);

	return ret;
}


message_error_code_t message_adaptor_get_connection_state(message_adaptor_plugin_h plugin,
						message_adaptor_plugin_context_h context,
						message_connection_state_t *state,
						message_adaptor_error_code_h *error_code)
{
	if ((NULL == plugin) || (NULL == context)) {
		message_adaptor_error("Invalid argument");

		_set_error_code(error_code, "14", "Invalid argument (plugin or context)");

		return MESSAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		message_adaptor_error("Plugin handle is null");

		_set_error_code(error_code, "13", "Plugin handle is null");

		return MESSAGE_ADAPTOR_ERROR_INVALID_HANDLE;
	}

	message_error_code_t ret;
	plugin_req_enter();
	ret = plugin->handle->get_connection_state(context, state);
	plugin_req_exit(ret, plugin, error_code);

	return ret;
}

message_error_code_t message_adaptor_decode_push_message(message_adaptor_plugin_h plugin,
						message_adaptor_plugin_context_h context,
						char *in_msg,
						char **out_msg,
						message_adaptor_error_code_h *error_code)
{
	if ((NULL == plugin) || (NULL == context)) {
		message_adaptor_error("Invalid argument");

		_set_error_code(error_code, "14", "Invalid argument (plugin or context)");

		return MESSAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	if (NULL == plugin->handle) {
		message_adaptor_error("Plugin handle is null");

		_set_error_code(error_code, "13", "Plugin handle is null");

		return MESSAGE_ADAPTOR_ERROR_INVALID_HANDLE;
	}

	if (NULL == in_msg || NULL == out_msg) {
		message_adaptor_error("invalid argument : input/output message");

		_set_error_code(error_code, "14", "Invalid argument (in_msg or out_msg)");

		return MESSAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	message_error_code_t ret;
	plugin_req_enter();
	ret = plugin->handle->decode_push_message(context, in_msg, out_msg);
	plugin_req_exit(ret, plugin, error_code);

	return ret;
}
