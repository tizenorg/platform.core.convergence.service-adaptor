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

#include <stdlib.h>
#include <sys/time.h>

#include "service-adaptor.h"
#include "service-adaptor-message.h"
#include "service-adaptor-type.h"
#include "service-adaptor-log.h"
#include "dbus-ipc.h"
#include "dbus-server.h"
#include "dbus-service-adaptor.h"
#include "dbus-message-adaptor.h"
#include "message-adaptor.h"

/*#define MESSAGING_PLUGIN_PATH	"/usr/lib/message-adaptor/plugins"*/
#define STR_MESSAGE_ADAPTOR_ERROR_CONNECTION	"601"

/* if calling completion callback, it need to wait called message api in dbus-message-adaptor */


void _get_service_name_by_message_context(message_adaptor_plugin_context_h context,
						char **service_name)
{
	service_adaptor_debug("<Start> %s", __FUNCTION__);
	service_adaptor_h service_adaptor = service_adaptor_get_handle();
	GList *services = NULL;
	services = service_adaptor_get_services_by_plugin_uri(service_adaptor, context->plugin_uri);

	GList *iter = NULL;
	service_adaptor_service_context_s *service_context = NULL;
	service_adaptor_debug("Start services iter (%d)", g_list_length(services));
	for (iter = g_list_first(services); iter; iter = g_list_next(iter)) {
		if (NULL != iter->data) {
			service_context = (service_adaptor_service_context_s *)iter->data;
			if (service_context->message_context == context) {
				if (NULL == service_context->service_name) {
					service_adaptor_error("Service name is NULL");
				} else {
					*service_name = strdup(service_context->service_name);
					service_adaptor_debug("Service name found (%s)", *service_name);
				}
				break;
			}
		} else {
			service_adaptor_debug("iter data is NULL");
		}
	}
	g_list_free(services);
	service_adaptor_debug("<End> %s", __FUNCTION__);
}

/***********************************************************
 * Free Message adaptor callback
 **********************************************************/
void service_adaptor_message_adaptor_client_echo_cb(message_adaptor_plugin_context_h context,
						long long int request_id,
						message_adaptor_error_code_t **error_code,
						void *server_data)
{
	service_adaptor_info("[Message  client echo CB]");

	message_adaptor_error_code_t *error = NULL;
	service_adaptor_h service_adaptor = service_adaptor_get_handle();
	message_adaptor_h adaptor = service_adaptor_get_message_adaptor(service_adaptor);

	if ((NULL == adaptor) || (NULL == context)) {
		service_adaptor_error("[Message  client echo CB] Invalid Param");
		return;
	}

	if (MESSAGE_CONNECTION_POLICY_CONNECT == context->connection_policy) {
		message_adaptor_plugin_h plugin = message_adaptor_get_plugin_by_name(adaptor, context->plugin_uri);

		if (NULL == plugin) {
			service_adaptor_warning("[Message  client echo CB] Could not find a plugin");
			return;
		}

		service_adaptor_info("Reply Server echo");
		message_adaptor_client_echo_reply(plugin, context, request_id, &error, NULL);
		message_adaptor_destroy_error_code(&error);
	} else {
		service_adaptor_info("Skip Server echo");
	}

	service_adaptor_info("%s End", __FUNCTION__);
}

void service_adaptor_message_adaptor_create_chatroom_reply_cb(message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int chatroom_id,
						int default_message_ttl,
						message_adaptor_wrong_receiver_s *wrong_receiver,
						message_adaptor_error_code_t **error_code,
						void *server_data)
{
	service_adaptor_info("[Message  create chatroom reply CB]");

	message_adaptor_error_code_t *err = error_code ? (*error_code) : NULL;
	message_adaptor_error_code_t _error;
	_error.code = "0";
	_error.msg = NULL;

	if (NULL == err) {
		err = &_error;
	}

	service_adaptor_internal_error_code_e ret = SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
	ret = dbus_reply_create_chatroom_callback(request_id, chatroom_id, default_message_ttl,
			wrong_receiver, err, server_data);

	if (SERVICE_ADAPTOR_INTERNAL_ERROR_NONE != ret) {
		service_adaptor_error("[Message  create chatroom reply DBus CB]");
	}

	service_adaptor_info("%s End", __FUNCTION__);
}

void service_adaptor_message_adaptor_change_chatroom_meta_reply_cb(message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int chatroom_id,
						message_adaptor_error_code_t **error_code,
						void *server_data)
{
	service_adaptor_info("[Message change chatroom meta reply CB]");

	message_adaptor_error_code_t *err = error_code ? (*error_code) : NULL;
	message_adaptor_error_code_t _error;
	_error.code = "0";
	_error.msg = NULL;

	if (NULL == err) {
		err = &_error;
	}

	service_adaptor_internal_error_code_e ret = SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
	ret = dbus_reply_change_chatroom_meta_callback(request_id, chatroom_id, err, server_data);

	if (SERVICE_ADAPTOR_INTERNAL_ERROR_NONE != ret) {
		service_adaptor_error("[Message change chatroom meta reply DBus CB]");
	}

	service_adaptor_info("%s End", __FUNCTION__);
}


void service_adaptor_message_adaptor_chat_reply_cb(message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int chatroom_id,
						message_adaptor_processed_msg_s **processed_msgs,
						unsigned int processed_msgs_len,
						message_adaptor_error_code_t **error_code,
						void *server_data)
{
	service_adaptor_info("[Message chat reply CB]");

	message_adaptor_error_code_t *err = error_code ? (*error_code) : NULL;
	message_adaptor_error_code_t _error;
	_error.code = "0";
	_error.msg = NULL;

	if (NULL == err) {
		err = &_error;
	}

	service_adaptor_internal_error_code_e ret = SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
	ret = dbus_reply_chat_callback(request_id, chatroom_id, processed_msgs, processed_msgs_len, err, server_data);

	if (SERVICE_ADAPTOR_INTERNAL_ERROR_NONE != ret) {
		service_adaptor_error("[Message chat reply DBus CB]");
	}

	service_adaptor_info("%s End", __FUNCTION__);
}

void service_adaptor_message_adaptor_allow_chat_reply_cb(message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int chatroom_id,
						message_adaptor_delivery_ack_s **delivery_acks,
						unsigned int delivery_acks_len,
						unsigned long long last_delivery_acks_timestamp,
						message_adaptor_read_ack_s **read_acks,
						unsigned int read_acks_len,
						unsigned long long last_read_acks_timestamp,
						message_adaptor_ordered_chat_member_s **ordered_chat_members,
						unsigned int ordered_chat_members_len,
						const char *chatroom_title,
						int default_message_ttl,
						message_adaptor_error_code_t **error_code,
						void *server_data)
{
	service_adaptor_info("[Message allow chat reply CB]");

	message_adaptor_error_code_t *err = error_code ? (*error_code) : NULL;
	message_adaptor_error_code_t _error;
	_error.code = "0";
	_error.msg = NULL;

	if (NULL == err) {
		err = &_error;
	}

	service_adaptor_internal_error_code_e ret = SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
	ret = dbus_reply_allow_chat_callback(request_id, chatroom_id,
			delivery_acks, delivery_acks_len, last_delivery_acks_timestamp,
			read_acks, read_acks_len, last_read_acks_timestamp,
			ordered_chat_members, ordered_chat_members_len,
			chatroom_title, default_message_ttl, err, server_data);

	if (SERVICE_ADAPTOR_INTERNAL_ERROR_NONE != ret) {
		service_adaptor_error("[Message allow chat reply DBus CB]");
	}

	service_adaptor_info("%s End", __FUNCTION__);
}

void service_adaptor_message_adaptor_get_all_unread_message_reply_cb(message_adaptor_plugin_context_h context,
						long long int request_id,
						message_adaptor_error_code_t **error_code,
						void *server_data)
{
	service_adaptor_info("[Message  get all unread message reply CB]");

	message_adaptor_error_code_t *err = error_code ? (*error_code) : NULL;
	message_adaptor_error_code_t _error;
	_error.code = "0";
	_error.msg = NULL;

	if (NULL == err) {
		err = &_error;
	}

	service_adaptor_internal_error_code_e ret = SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
	ret = dbus_reply_all_unread_message_callback(request_id, err, server_data);

	if (SERVICE_ADAPTOR_INTERNAL_ERROR_NONE != ret) {
		service_adaptor_error("[Message  get all unread message reply DBus CB]");
	}

	service_adaptor_info("%s End", __FUNCTION__);
}

void service_adaptor_message_adaptor_forward_online_message_request_cb(message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int chatroom_id,
						int chat_type,
						message_adaptor_inbox_message_s *inbox_msg,
						bool skip_reply,
						message_adaptor_error_code_t **error_code,
						void *server_data)
{
	service_adaptor_info("[Message  forward online message request CB]");

	message_adaptor_error_code_t *err = error_code ? (*error_code) : NULL;
	message_adaptor_error_code_t _error;
	_error.code = "0";
	_error.msg = NULL;

	if (NULL == err) {
		err = &_error;
	}

	service_adaptor_internal_error_code_e ret = SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
	ret = dbus_request_forward_online_message_callback(request_id, chatroom_id, chat_type, inbox_msg, skip_reply, err, server_data);

	if (SERVICE_ADAPTOR_INTERNAL_ERROR_NONE != ret) {
		service_adaptor_error("[Message  forward online message DBus CB]");
	}

	service_adaptor_info("[End Message  forward online message request CB]");
	service_adaptor_info("%s End", __FUNCTION__);
}

void service_adaptor_message_adaptor_forward_unread_message_request_cb(message_adaptor_plugin_context_h context,
						long long int request_id,
						message_adaptor_inbox_message_s ***inbox_msgs,
						unsigned int inbox_msgs_len,
						char **next_pagination_key,
						message_adaptor_error_code_t **error_code,
						void *server_data)
{
	service_adaptor_info("[Message  forward unread message request CB]");

	message_adaptor_error_code_t *err = error_code ? (*error_code) : NULL;
	message_adaptor_error_code_t _error;
	_error.code = "0";
	_error.msg = NULL;

	if (NULL == err) {
		err = &_error;
	}

	service_adaptor_internal_error_code_e ret = SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
	ret = dbus_request_forward_unread_message_callback(request_id, *inbox_msgs, inbox_msgs_len, *next_pagination_key, err, server_data);

	if (SERVICE_ADAPTOR_INTERNAL_ERROR_NONE != ret) {
		service_adaptor_error("[Message  forward unread message request DBus CB]");
	}

	service_adaptor_info("%s End", __FUNCTION__);
}

void service_adaptor_message_adaptor_read_message_reply_cb(message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int chatroom_id,
						message_adaptor_error_code_t **error_code,
						void *server_data)
{
	service_adaptor_info("[Message  read message reply CB]");

	message_adaptor_error_code_t *err = error_code ? (*error_code) : NULL;
	message_adaptor_error_code_t _error;
	_error.code = "0";
	_error.msg = NULL;

	if (NULL == err) {
		err = &_error;
	}

	service_adaptor_internal_error_code_e ret = SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
	ret = dbus_reply_read_message_callback(request_id, chatroom_id, err, server_data);

	if (SERVICE_ADAPTOR_INTERNAL_ERROR_NONE != ret) {
		service_adaptor_error("[Message  read message reply DBus CB]");
	}

	service_adaptor_info("%s End", __FUNCTION__);
}

void service_adaptor_message_adaptor_invite_chat_reply_cb(message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int chatroom_id,
						long long int sent_time,
						message_adaptor_wrong_receiver_s *wrong_receiver,
						message_adaptor_error_code_t **error_code,
						void *server_data)
{
	service_adaptor_info("[Message  invite chat reply CB]");

	message_adaptor_error_code_t *err = error_code ? (*error_code) : NULL;
	message_adaptor_error_code_t _error;
	_error.code = "0";
	_error.msg = NULL;

	if (NULL == err) {
		err = &_error;
	}

	service_adaptor_internal_error_code_e ret = SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
	ret = dbus_reply_invite_chat_callback(request_id, chatroom_id, sent_time, wrong_receiver, err, server_data);

	if (SERVICE_ADAPTOR_INTERNAL_ERROR_NONE != ret) {
		service_adaptor_error("[Message  invite chat reply DBus CB]");
	}

	service_adaptor_info("%s End", __FUNCTION__);
}

void service_adaptor_message_adaptor_end_chat_reply_cb(message_adaptor_plugin_context_h context,
						long long int request_id,
						message_adaptor_error_code_t **error_code,
						void *server_data)
{
	service_adaptor_info("[Message  end chat reply CB]");

	message_adaptor_error_code_t *err = error_code ? (*error_code) : NULL;
	message_adaptor_error_code_t _error;
	_error.code = "0";
	_error.msg = NULL;

	if (NULL == err) {
		err = &_error;
	}

	service_adaptor_internal_error_code_e ret = SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
	ret = dbus_reply_end_chat_callback(request_id, err, server_data);

	if (SERVICE_ADAPTOR_INTERNAL_ERROR_NONE != ret) {
		service_adaptor_error("[Message  end chat reply DBus CB]");
	}

	service_adaptor_info("%s End", __FUNCTION__);
}

void service_adaptor_message_adaptor_unseal_message_reply_cb(message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int chatroom_id,
						message_adaptor_error_code_t **error_code,
						void *server_data)
{
	service_adaptor_info("[Message  unseal message reply CB]");

	message_adaptor_error_code_t *err = error_code ? (*error_code) : NULL;
	message_adaptor_error_code_t _error;
	_error.code = "0";
	_error.msg = NULL;

	if (NULL == err) {
		err = &_error;
	}

	service_adaptor_internal_error_code_e ret = SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
	ret = dbus_reply_unseal_message_callback(request_id, chatroom_id, err, server_data);

	if (SERVICE_ADAPTOR_INTERNAL_ERROR_NONE != ret) {
		service_adaptor_error("[Message  unseal message reply DBus CB]");
	}

	service_adaptor_info("%s End", __FUNCTION__);
}


void service_adaptor_message_adaptor_save_call_log_reply_cb(message_adaptor_plugin_context_h context,
						long long int request_id,
						message_adaptor_error_code_t **error_code,
						void *server_data)
{
	service_adaptor_info("[Message  save call log reply CB]");

	message_adaptor_error_code_t *err = error_code ? (*error_code) : NULL;
	message_adaptor_error_code_t _error;
	_error.code = "0";
	_error.msg = NULL;

	if (NULL == err) {
		err = &_error;
	}

	service_adaptor_internal_error_code_e ret = SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
	ret = dbus_reply_save_call_log_callback(request_id, err, server_data);

	if (SERVICE_ADAPTOR_INTERNAL_ERROR_NONE != ret) {
		service_adaptor_error("[Message  save call log reply DBus CB]");
	}

	service_adaptor_info("%s End", __FUNCTION__);
}

void service_adaptor_message_adaptor_current_time_reply_cb(message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int current_time_millis,
						message_adaptor_error_code_t **error_code,
						void *server_data)
{
	service_adaptor_info("[Message  current time reply CB]");

	message_adaptor_error_code_t *err = error_code ? (*error_code) : NULL;
	message_adaptor_error_code_t _error;
	_error.code = "0";
	_error.msg = NULL;

	if (NULL == err) {
		err = &_error;
	}

	service_adaptor_internal_error_code_e ret = SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
	ret = dbus_reply_current_time_callback(request_id, current_time_millis, err, server_data);

	if (SERVICE_ADAPTOR_INTERNAL_ERROR_NONE != ret) {
		service_adaptor_error("[Message  current time reply DBus CB]");
	}

	service_adaptor_info("%s End", __FUNCTION__);
}

void service_adaptor_message_adaptor_typing_updated_cb(message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int chatroom_id,
						long long int *sender,
						char **state,
						int *contentType,
						int *refershTime,
						message_adaptor_error_code_t **error_code,
						void *server_data)
{
	service_adaptor_info("[Message typing updated reply CB]");
	service_adaptor_info("%s End", __FUNCTION__);
}

void service_adaptor_message_adaptor_thread_completion_cb(message_adaptor_plugin_context_h context,
						message_connection_state_t state,
						message_adaptor_error_code_t **error_code,
						void *server_data)
{
	service_adaptor_info("[Message  thread completion CB]");
	service_adaptor_debug("connection state(%d)", state);

	if (MESSAGE_CONNECTION_STATE_INTERRUPTED != state) {
		service_adaptor_info("Message Connection Finished (without re-connect)");
		return;
	}

	service_adaptor_h service_adaptor = service_adaptor_get_handle();
	message_adaptor_h adaptor = service_adaptor_get_message_adaptor(service_adaptor);

	if ((NULL == adaptor) || (NULL == context)) {
		service_adaptor_error("[Message  Thread Completion CB] Invalid Param");
		return;
	}

	message_adaptor_plugin_h plugin = message_adaptor_get_plugin_by_name(adaptor, context->plugin_uri);

	if (NULL == plugin) {
		service_adaptor_error("[Message  Thread Completion CB] Could not find a plugin");
		return;
	}

	int ret = SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
	message_adaptor_error_code_h error = NULL;

	message_adaptor_error_code_t *err = error_code ? (*error_code) : NULL;
	message_adaptor_error_code_t _error;
	_error.code = "0";
	_error.msg = NULL;

	if (NULL == err) {
		err = &_error;
	}

	/* Disconnect by user policy */
	if (MESSAGE_CONNECTION_POLICY_CONNECT != context->connection_policy) {
		service_adaptor_info("Disconnect By user policy (%d)", context->connection_policy);
		context->connection_policy = MESSAGE_CONNECTION_POLICY_DISCONNECT;
		message_adaptor_disconnect(plugin, context, &error);
		char *service_name = NULL;
		_get_service_name_by_message_context(context, &service_name);
		dbus_reply_message_channel_disconnected_callback(service_name, err);
		free(service_name);

		if (NULL != error) {
			service_adaptor_debug("error code : code(%s) msg(%s)", error->code, error->msg);
			message_adaptor_destroy_error_code(&error);
		}
		return;
	}

	service_adaptor_debug("message connection create");
	ret = service_adaptor_message_connection_create(plugin, context, &error);

	if (NULL != error) {
		service_adaptor_debug("error code : code(%s) msg(%s)", error->code, error->msg);
		message_adaptor_destroy_error_code(&error);
	}

	switch (ret) {
	case SERVICE_ADAPTOR_INTERNAL_ERROR_NONE:	/* success */
		service_adaptor_debug("Message connection was resumed successfully");
		break;
	case SERVICE_ADAPTOR_INTERNAL_ERROR_INVALID_ARGUMENT:	/* fail : Noti this information to Client */
		service_adaptor_warning("msg_buffer was empty (can not get message plugin, context)");
	case SERVICE_ADAPTOR_INTERNAL_ERROR_CONNECT:
		service_adaptor_warning("msg_buffer was empty (can not get message plugin, context)");
	default:
		service_adaptor_debug("message connection disconnect");
		context->connection_policy = MESSAGE_CONNECTION_POLICY_DISCONNECT;
		int con_ret = message_adaptor_disconnect(plugin, context, &error);
		if (con_ret) {
			service_adaptor_error("message connection disconnect failed");
		} else {
			service_adaptor_error("message connection resumption stopped (disconnect res : %d)", con_ret);
		}

		service_adaptor_info("Set Connection policy, and call noti callback");
		char *service_name = NULL;
		_get_service_name_by_message_context(context, &service_name);
		dbus_reply_message_channel_disconnected_callback(service_name, error ? error : &_error);
		free(service_name);

		if (NULL != error) {
			service_adaptor_debug("error code : code(%s) msg(%s)", error->code, error->msg);
			message_adaptor_destroy_error_code(&error);
		}
		break;
	}

	service_adaptor_info("%s End", __FUNCTION__);
}

service_adaptor_internal_error_code_e service_adaptor_message_connection_create(message_adaptor_plugin_h plugin,
						message_adaptor_plugin_context_h message_context,
						message_adaptor_error_code_t **error)
{
	if ((NULL == plugin) || (NULL == message_context)) {
		service_adaptor_error("Invailid argument (plugin [%p], context [%p])", plugin, message_context);
		return SERVICE_ADAPTOR_INTERNAL_ERROR_INVALID_ARGUMENT;
	}

	int try_count = 0;
	int MAX_TRY_COUNT = 1;
	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	service_adaptor_debug("Try 'message_adaptor_connect'");

	do {
		ret = SERVICE_ADAPTOR_ERROR_NONE;
		ret = message_adaptor_connect(plugin, message_context, error);

		if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
			service_adaptor_warning("message adaptor connection failed(try count : %d)", ++try_count);
			if ((NULL != error) && (NULL != *error)) {
				service_adaptor_info("message connection fail error code [%s][%s]",
						(*error)->code, (*error)->msg);
				message_adaptor_destroy_error_code(error);
			}

			if (MAX_TRY_COUNT <= try_count) {
				service_adaptor_error("message connection resumption stopped (last connect res : %d)", ret);
				return SERVICE_ADAPTOR_INTERNAL_ERROR_CONNECT;
			}
		}
	} while (SERVICE_ADAPTOR_ERROR_NONE != ret);

	MAX_TRY_COUNT = 2;
	try_count = 0;
	message_adaptor_error_code_t *key_error_code = NULL;

	service_adaptor_debug("Try 'message_adaptor_get_key'");

	do {
		ret = SERVICE_ADAPTOR_ERROR_NONE;
		ret = message_adaptor_get_key(plugin, message_context,
				NULL, NULL, NULL, NULL, NULL, &key_error_code, NULL);

		if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
			service_adaptor_warning("Could not get encrypt key from server(try count : %d)", ++try_count);
			if (NULL != key_error_code) {
				service_adaptor_info("message get-key fail error code [%s][%s]",
						key_error_code->code, key_error_code->msg);
				message_adaptor_destroy_error_code(&key_error_code);
			}

			if (MESSAGE_ADAPTOR_ERROR_NOT_AUTHORIZED == ret) {
				service_adaptor_info("Access token invalid. Try refresh auth");
				char *service_name = NULL;
				_get_service_name_by_message_context(message_context, &service_name);
				service_adaptor_debug("service name : %s", service_name);

				service_adaptor_auth_refresh(service_adaptor_get_handle(), service_name, message_context->plugin_uri);
				free(service_name);
			} else {
				++try_count;
			}

			if (MAX_TRY_COUNT <= try_count) {
				service_adaptor_error("message connection resumption stopped (last get_key res : %d)", ret);
				return SERVICE_ADAPTOR_INTERNAL_ERROR_CONNECT;
			}
		}
	} while (SERVICE_ADAPTOR_ERROR_NONE != ret);

	MAX_TRY_COUNT = 1;
	try_count = 0;
	struct timeval tv;
	long long int req_id = 0;

	service_adaptor_debug("Try 'message_adaptor_channel_auth_request'");

	do {
		gettimeofday(&tv, NULL);
		req_id = (long long int)tv.tv_usec;

		ret = SERVICE_ADAPTOR_ERROR_NONE;
		int timeout_sec = 5;		/* TODO It will be changed to meaningful value */
		int additional_timeout_sec = 2;	/* TODO It will be changed to meaningful value */
		ret = message_adaptor_channel_auth_request(plugin, message_context, req_id, timeout_sec, NULL, error, NULL);

		if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
			service_adaptor_warning("message adaptor channel-auth request failed(try count : %d)", ++try_count);
			if ((NULL != error) && (NULL != *error)) {
				service_adaptor_info("message channel-auth error code [%s][%s]",
						(*error)->code, (*error)->msg);
				message_adaptor_destroy_error_code(error);
			}

			if (MAX_TRY_COUNT <= try_count) {
				service_adaptor_error("message connection resumption stopped (last channel_auth res : %d)", ret);
				return SERVICE_ADAPTOR_INTERNAL_ERROR_CONNECT;
			}
			if (MESSAGE_ADAPTOR_ERROR_TIME_OUT == ret) {
				timeout_sec += additional_timeout_sec;
				service_adaptor_info("Increase timeout seconds (total: %d sec)", timeout_sec);
			}
		}
	} while (SERVICE_ADAPTOR_ERROR_NONE != ret);

	service_adaptor_debug("Message connection created successfully");

	service_adaptor_info("%s End", __FUNCTION__);

	return ret;
}

message_adaptor_h service_adaptor_get_message_adaptor(service_adaptor_h service_adaptor)
{
	service_adaptor_debug("Get message adaptor");

	if ((void *) NULL == service_adaptor) {
		service_adaptor_error("Invalid argument");
		return NULL;
	}

	return service_adaptor->message_handle;
}

service_adaptor_internal_error_code_e service_adaptor_connect_message_plugin(service_adaptor_h service_adaptor,
						service_adaptor_service_context_h service,
						char *ret_msg)
{
	service_adaptor_debug("Connect to message plugin");

	if ((NULL == service_adaptor) || (NULL == service)) {
		service_adaptor_error("Invalid parameter");
		snprintf(ret_msg, 2048, "message plugin connect failed [%d]", (int)__LINE__);
		return SERVICE_ADAPTOR_INTERNAL_ERROR_INVALID_ARGUMENT;
	}

	message_adaptor_h adaptor = service_adaptor_get_message_adaptor(service_adaptor);
	message_adaptor_plugin_h plugin = message_adaptor_get_plugin_by_name(adaptor, service->plugin_uri);

	if ((NULL == service->context_info) || (NULL == service->context_info->duid)
			|| (NULL == service->context_info->access_token)) {
		if (NULL != service->context_info) {
			service_adaptor_error("Invalid duid or access_token");
			service_adaptor_debug_secure("Invalid duid or access_token: %s, %s",
					service->context_info->duid, service->context_info->access_token);
		} else {
			service_adaptor_error("Message context is NULL");
		}
		snprintf(ret_msg, 2048, "message plugin connect failed [%d]", (int)__LINE__);
		return SERVICE_ADAPTOR_INTERNAL_ERROR_INVALID_ARGUMENT;
	}

	message_adaptor_plugin_context_h message_context = message_adaptor_create_plugin_context(plugin,
			service->plugin_uri, service->context_info->duid, service->context_info->access_token, service->context_info->app_id, service->context_info->service_id);
	/* TODO It can be included in service_adaptor_service_context_t */

	if (NULL == message_context) {
		service_adaptor_debug_secure("Could not get message plugin context: %s, %s",
				service->context_info->duid, service->context_info->access_token);
		snprintf(ret_msg, 2048, "message plugin connect failed [%d]", (int)__LINE__);
		return SERVICE_ADAPTOR_INTERNAL_ERROR_CORRUPTED;
	}

	/* TODO It should remove this part. */
	/* And then it can add conn info of message_context from conn info of same duid in message_adaptor_connect() */
	int service_count = g_list_length(service_adaptor->service_list);

	for (int i = 0; i < service_count; i++) {
		service_adaptor_service_context_h svc = g_list_nth_data(service_adaptor->service_list, i);

		if ((NULL != svc) && (NULL != svc->message_context) && (0 == strncmp(svc->context_info->duid, service->context_info->duid, strlen(service->context_info->duid)))) {
			message_connection_state_t state = MESSAGE_CONNECTION_STATE_INIT;
			message_adaptor_error_code_h msg_error = NULL;
			message_adaptor_get_connection_state(plugin, svc->message_context, &state, &msg_error);

			if (MESSAGE_CONNECTION_STATE_CONNECT == state) {
				service->message_context = svc->message_context;
				service->connected |= 0x0000100;
				message_adaptor_destroy_error_code(&msg_error);

				return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
			}

			message_adaptor_destroy_error_code(&msg_error);
		}
	}

	/* Set server info */
	int ret = 0;
	message_adaptor_error_code_h error = NULL;
	ret = message_adaptor_set_server_info(plugin, message_context, service->server_info, NULL, &error, NULL);
	if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
		service_adaptor_warning("Could not set message plugin server information: %d", ret);
		if (NULL != error) {
			service_adaptor_warning("[%s] %s", error->code, error->msg);
		}
		message_adaptor_destroy_error_code(&error);
	}

/*	ret = service_adaptor_message_connection_create(plugin, message_context, &error); */

	SERVICE_ADAPTOR_API_TIME_CHECK_PAUSE();
	SERVICE_ADAPTOR_PLUGIN_API_TIME_CHECK_START(SA_TIME_CHECK_FLAG_MESSAGE);
	ret = message_adaptor_connect(plugin, message_context, &error);
	SERVICE_ADAPTOR_PLUGIN_API_TIME_CHECK_PAUSE(SA_TIME_CHECK_FLAG_MESSAGE);
	SERVICE_ADAPTOR_API_TIME_CHECK_START();

	if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
		service_adaptor_error("message adaptor connection failed");
		message_context->connection_policy = MESSAGE_CONNECTION_POLICY_DISCONNECT;
		service->message_context = message_context;
		if (NULL != error) {
			snprintf(ret_msg, 2048, "message connect failed [%s][%s]", error->code, error->msg);
			if (0 == strncmp(STR_MESSAGE_ADAPTOR_ERROR_CONNECTION, error->code, strlen(STR_MESSAGE_ADAPTOR_ERROR_CONNECTION))) {
				ret = MESSAGE_PLUGIN_ERROR_NETWORK_DEVICE_OFFLINE;
			}
		} else {
			snprintf(ret_msg, 2048, "message TCP connect failed []");
		}
		message_adaptor_destroy_error_code(&error);
		return ret;
	}

	struct timeval tv;
	long long int req_id = 0;
	void *server_data = NULL;
	message_adaptor_error_code_t *key_error_code = NULL;
	message_adaptor_error_code_t *error_code = NULL;

	SERVICE_ADAPTOR_API_TIME_CHECK_PAUSE();
	SERVICE_ADAPTOR_PLUGIN_API_TIME_CHECK_START(SA_TIME_CHECK_FLAG_MESSAGE);
	ret = message_adaptor_get_key(plugin, message_context, NULL, NULL, NULL, NULL, NULL, &key_error_code, &server_data);
	SERVICE_ADAPTOR_PLUGIN_API_TIME_CHECK_PAUSE(SA_TIME_CHECK_FLAG_MESSAGE);
	SERVICE_ADAPTOR_API_TIME_CHECK_START();

	if (NULL != key_error_code) {
		service_adaptor_error("Could not get encrypt key from server,%s %s", key_error_code->code, key_error_code->msg);

/*		return SERVICE_ADAPTOR_INTERNAL_ERROR_INVALID_ARGUMENT; */
	}

	if (MESSAGE_ADAPTOR_ERROR_NOT_AUTHORIZED == ret) {
		service_adaptor_info("Access token invalid. Try refresh auth");
		message_adaptor_destroy_error_code(&key_error_code);

		service->message_context = message_context;
		SERVICE_ADAPTOR_API_TIME_CHECK_PAUSE();
		SERVICE_ADAPTOR_PLUGIN_API_TIME_CHECK_START(SA_TIME_CHECK_FLAG_AUTH);
		service_adaptor_auth_refresh_with_service_context(service_adaptor_get_handle(), service, message_context->plugin_uri);
		SERVICE_ADAPTOR_PLUGIN_API_TIME_CHECK_PAUSE(SA_TIME_CHECK_FLAG_AUTH);
		SERVICE_ADAPTOR_API_TIME_CHECK_START();
		ret = message_adaptor_get_key(plugin, message_context, NULL, NULL, NULL, NULL, NULL, &key_error_code, &server_data);
	}

	if (NULL != key_error_code) {
		service_adaptor_error("Could not get encrypt key from server,%s %s", key_error_code->code, key_error_code->msg);
		snprintf(ret_msg, 2048, "message get-key failed [%s][%s]", key_error_code->code, key_error_code->msg);

		if (0 == strncmp(STR_MESSAGE_ADAPTOR_ERROR_CONNECTION, key_error_code->code, strlen(STR_MESSAGE_ADAPTOR_ERROR_CONNECTION))) {
			ret = MESSAGE_PLUGIN_ERROR_NETWORK_DEVICE_OFFLINE;
		}

		message_adaptor_destroy_error_code(&key_error_code);
		message_context->connection_policy = MESSAGE_CONNECTION_POLICY_DISCONNECT;
		service->message_context = message_context;
/*		service->message_context = NULL; */

		return ret;
	} else if (MESSAGE_ADAPTOR_ERROR_NONE != ret) {
		snprintf(ret_msg, 2048, "message get-key failed [%s]", "No error message");
		ret = MESSAGE_PLUGIN_ERROR_NETWORK_DEVICE_OFFLINE;

		message_context->connection_policy = MESSAGE_CONNECTION_POLICY_DISCONNECT;
		service->message_context = message_context;
		return ret;
	}

	gettimeofday(&tv, NULL);
	req_id = (long long int)tv.tv_usec;
	int timeout_sec = 10;		/* TODO It will be changed to meaningful value */
	SERVICE_ADAPTOR_API_TIME_CHECK_PAUSE();
	SERVICE_ADAPTOR_PLUGIN_API_TIME_CHECK_START(SA_TIME_CHECK_FLAG_MESSAGE);
	ret = message_adaptor_channel_auth_request(plugin, message_context, req_id, timeout_sec, NULL, &error_code, NULL);
	SERVICE_ADAPTOR_PLUGIN_API_TIME_CHECK_PAUSE(SA_TIME_CHECK_FLAG_MESSAGE);
	SERVICE_ADAPTOR_API_TIME_CHECK_START();

	if (MESSAGE_ADAPTOR_ERROR_NONE == ret) {
		message_context->connection_policy = MESSAGE_CONNECTION_POLICY_AUTO;
		service->message_context = message_context;
		service->connected |= 0x0000100;
	} else {
		message_context->connection_policy = MESSAGE_CONNECTION_POLICY_DISCONNECT;
		service->message_context = message_context;
/*		service->message_context = NULL; */
		if (NULL != error_code) {
			service_adaptor_error("Channel create request error (%s): %s", error_code->code, error_code->msg);
			snprintf(ret_msg, 2048, "message channel-auth failed [%s][%s]", error_code->code, error_code->msg);

			if (0 == strncmp(STR_MESSAGE_ADAPTOR_ERROR_CONNECTION, error_code->code, strlen(STR_MESSAGE_ADAPTOR_ERROR_CONNECTION))) {
				ret = MESSAGE_PLUGIN_ERROR_NETWORK_DEVICE_OFFLINE;
			}

			message_adaptor_destroy_error_code(&error_code);
		} else {
			snprintf(ret_msg, 2048, "message channel-auth failed [%s]", "No error message");
			ret = MESSAGE_PLUGIN_ERROR_NETWORK_DEVICE_OFFLINE;
		}
		return ret;
	}

	service_adaptor_debug("Connected to message plugin");

	return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
}

service_adaptor_internal_error_code_e service_adaptor_disconnect_message_plugin(service_adaptor_h service_adaptor,
						service_adaptor_service_context_h service)
{
	service_adaptor_debug("Disconnect from message plugin");

	service_adaptor_debug("get message adaptor");
	message_adaptor_h message_adaptor = service_adaptor_get_message_adaptor(service_adaptor);
	if ((NULL != service->message_context) && (NULL != message_adaptor)) {
		service_adaptor_debug("disconnects message");
		message_adaptor_error_code_h message_error = NULL;
		message_adaptor_plugin_h message_plugin = message_adaptor_get_plugin_by_name(message_adaptor, service->message_context->plugin_uri);

		if (NULL == message_plugin) {
			service_adaptor_error("Cannot find plugin");
		} else {
			message_connection_state_t state = MESSAGE_CONNECTION_STATE_INIT;
			message_adaptor_get_connection_state(message_plugin, service->message_context, &state, &message_error);
			message_adaptor_destroy_error_code(&message_error);

			service_adaptor_debug_func("connection state (%d)", state);
			if ((MESSAGE_CONNECTION_STATE_CONNECT == state) ||
					(MESSAGE_CONNECTION_STATE_INTERRUPTED == state)) {
				service_adaptor_debug_func("Call message_adaptor_disconnect");
				message_adaptor_disconnect(message_plugin, service->message_context, &message_error);
			}

			service_adaptor_debug("dsetroys message context");
			message_adaptor_destroy_plugin_context(message_plugin, service->message_context);
			service->message_context = NULL;

			if (NULL != message_error) {
				service_adaptor_debug("error code : code(%s) msg(%s)", message_error->code, message_error->msg);
				message_adaptor_destroy_error_code(&message_error);
			}
		}
	}

	service_adaptor_debug("Disconnected from message plugin");

	return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
}

message_adaptor_h service_adaptor_create_message()
{
	message_adaptor_h message_adaptor = message_adaptor_create(MESSAGE_PLUGIN_PATH);

	if (NULL == message_adaptor) {
		service_adaptor_error("Could not create message adaptor");
		return NULL;
	}

	service_adaptor_debug("Message adaptor created");

	return message_adaptor;
}

service_adaptor_internal_error_code_e service_adaptor_message_set_connection(message_adaptor_plugin_h plugin,
						message_adaptor_plugin_context_h message_context,
						message_connection_policy_e policy,
						message_adaptor_error_code_t **error_code)
{
	service_adaptor_debug("<Start> %s", __FUNCTION__);
	if ((NULL == plugin) || (NULL == message_context)) {
		service_adaptor_error("Invailid argument (plugin [%p], context [%p])", plugin, message_context);
		return SERVICE_ADAPTOR_INTERNAL_ERROR_INVALID_ARGUMENT;
	}

	service_adaptor_internal_error_code_e ret = SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
	message_adaptor_error_code_t *_error = NULL;

	if (MESSAGE_CONNECTION_POLICY_DISCONNECT == policy) {
		service_adaptor_info("Disconnect By user policy(%d)", policy);
		message_context->connection_policy = MESSAGE_CONNECTION_POLICY_DISCONNECT;
		message_adaptor_disconnect(plugin, message_context, &_error);
	} else if ((MESSAGE_CONNECTION_POLICY_CONNECT == policy) || (MESSAGE_CONNECTION_POLICY_AUTO == policy)) {
		service_adaptor_info("Connect By user policy(%d)", policy);
		if (MESSAGE_CONNECTION_POLICY_DISCONNECT == message_context->connection_policy) {
			message_context->connection_policy = policy;
			service_adaptor_debug("Try new connection create");
			ret = service_adaptor_message_connection_create(plugin, message_context, &_error);
			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NONE != ret) {
				service_adaptor_debug("New connection create failed, set disconnect");
				message_context->connection_policy = MESSAGE_CONNECTION_POLICY_DISCONNECT;
				message_adaptor_disconnect(plugin, message_context, &_error);
				message_adaptor_destroy_error_code(&_error);
				_error = message_adaptor_create_error_code("601", "Message Channel Connect failed");
				ret =  SERVICE_ADAPTOR_INTERNAL_ERROR_DISCONNECT;
			} else {
				message_context->connection_policy = policy;
			}
		} else {
			message_context->connection_policy = policy;
		}
	}

	if (error_code != NULL) {
		*error_code = _error;
	} else {
		message_adaptor_destroy_error_code(&_error);
	}

	service_adaptor_debug("<End> %s", __FUNCTION__);

	return ret;
}

message_adaptor_listener_h service_adaptor_register_message_listener(message_adaptor_h message_adaptor)
{
	if (NULL == message_adaptor) {
		service_adaptor_error("Could not create message adaptor");
		return NULL;
	}

	message_adaptor_listener_h message_listener =
		(message_adaptor_listener_h) malloc(sizeof(message_adaptor_listener_t));

	if ((void *) NULL == message_listener) {
		service_adaptor_error("Could not create message listener");
		return NULL;
	}

	message_listener->client_echo_cb = service_adaptor_message_adaptor_client_echo_cb;
	message_listener->create_chatroom_reply_cb = service_adaptor_message_adaptor_create_chatroom_reply_cb;
	message_listener->change_chatroom_meta_reply_cb = service_adaptor_message_adaptor_change_chatroom_meta_reply_cb;
	message_listener->chat_reply_cb = service_adaptor_message_adaptor_chat_reply_cb;
	message_listener->allow_chat_reply_cb = service_adaptor_message_adaptor_allow_chat_reply_cb;
	message_listener->get_all_unread_message_reply_cb = service_adaptor_message_adaptor_get_all_unread_message_reply_cb;
	message_listener->forward_online_message_request_cb = service_adaptor_message_adaptor_forward_online_message_request_cb;
	message_listener->forward_unread_message_request_cb = service_adaptor_message_adaptor_forward_unread_message_request_cb;
	message_listener->read_message_reply_cb = service_adaptor_message_adaptor_read_message_reply_cb;
	message_listener->invite_chat_reply_cb = service_adaptor_message_adaptor_invite_chat_reply_cb;
	message_listener->end_chat_reply_cb = service_adaptor_message_adaptor_end_chat_reply_cb;
	message_listener->unseal_message_reply_cb = service_adaptor_message_adaptor_unseal_message_reply_cb;
	message_listener->save_call_log_reply_cb = service_adaptor_message_adaptor_save_call_log_reply_cb;
	message_listener->current_time_reply_cb = service_adaptor_message_adaptor_current_time_reply_cb;
	message_listener->typing_updated_cb = service_adaptor_message_adaptor_typing_updated_cb;
	message_listener->completion_cb = service_adaptor_message_adaptor_thread_completion_cb;

	message_adaptor_register_listener(message_adaptor, message_listener);
	service_adaptor_debug("Message adaptor listener created");

	return message_listener;
}
