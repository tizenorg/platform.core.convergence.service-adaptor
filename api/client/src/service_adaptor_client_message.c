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

/******************************************************************************
 * File: service-adaptor-client-messaging.c
 * Desc:
 * Created on: Oct, 2014
 * Auth: Yongjin Kim <youth.kim@samsung.com>
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "private/service-adaptor-client-message.h"
#include "service_adaptor_client_type.h"
#include "service_adaptor_client_log.h"
#include "dbus_client.h"
#include "dbus_client_message.h"

#include "util/service_adaptor_client_util.h"
/**	@brief	Requests Creating Chatroom
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_request_create_chatroom(service_adaptor_h handle,
						long long int request_id,
						int chat_type,
						long long int *receivers,
						unsigned int receivers_len,
						const char *chatroom_title,
						service_adaptor_reply_create_chatroom_cb callback,
						service_adaptor_error_s **error_code,
						void *user_data)
{
	sac_api_start_msg(request_id);
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;

	if ((NULL == handle) || (NULL == handle->plugin) || (0 > chat_type) || (1 < chat_type) ||
			(NULL == chatroom_title) || (NULL == receivers) || (1 > receivers_len)) {
		if (NULL != error_code) {
			service_adaptor_error_s *_error = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));
			if (NULL != _error) {
				_error->code = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
				_error->msg = strdup("Invalid Argument");
			}
			*error_code = _error;
		}

		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	_queue_add_task(request_id, (uint32_t) callback, handle, user_data);

	error.code = SERVICE_ADAPTOR_ERROR_NONE;
	error.msg = NULL;
	ret = _dbus_request_create_chatroom(handle->service_name, request_id, chat_type, receivers, receivers_len, chatroom_title, user_data, &error);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		service_adaptor_error_s *_error = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));
		if ((NULL != _error) && (NULL != error_code)) {
			_error->code = error.code;
			_error->msg = error.msg;
			*error_code = _error;
		} else {
			free(_error);
			free(error.msg);
		}
	}

	sac_api_end(ret);
	return ret;
}

/**	@brief	Requests Changing Chatroom metadata
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_request_change_chatroom_meta(service_adaptor_h handle,
						long long int request_id,
						long long int chatroom_id,
						const char *chatroom_title,
						int default_message_ttl,
						service_adaptor_reply_change_chatroom_meta_cb callback,
						service_adaptor_error_s **error_code,
						void *user_data)

{
	sac_api_start_msg(request_id);
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;

	if ((NULL == handle) || (NULL == handle->plugin)) {
		if (NULL != error_code) {
			service_adaptor_error_s *_error = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));
			if (NULL != _error) {
				_error->code = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
				_error->msg = strdup("Invalid Argument");
			}
			*error_code = _error;
		}

		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	_queue_add_task(request_id, (uint32_t) callback, handle, user_data);

	error.code = SERVICE_ADAPTOR_ERROR_NONE;
	error.msg = NULL;
	ret = _dbus_request_change_chatroom_meta(handle->service_name, request_id, chatroom_id, chatroom_title, default_message_ttl, user_data, &error);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		service_adaptor_error_s *_error = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));
		if ((NULL != _error) && (NULL != error_code)) {
			_error->code = error.code;
			_error->msg = error.msg;
			*error_code = _error;
		} else {
			free(_error);
			free(error.msg);
		}
	}

	sac_api_end(ret);
	return ret;
}


/**	@brief	Requests Chat
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_request_chat(service_adaptor_h handle,
						long long int request_id,
						long long int chatroom_id,
						service_adaptor_chat_msg_s **chat_msgs,
						unsigned int chat_msgs_len,
						service_adaptor_reply_chat_cb callback,
						service_adaptor_error_s **error_code,
						void *user_data)
{
	sac_api_start_msg(request_id);
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;

	if ((NULL == handle) || (NULL == handle->plugin) || (NULL == chat_msgs) || (1 > chat_msgs_len)) {
		if (NULL != error_code) {
			service_adaptor_error_s *_error = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));
			if (NULL != _error) {
				_error->code = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
				_error->msg = strdup("Invalid Argument");
			}
			*error_code = _error;
		}

		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	_queue_add_task(request_id, (uint32_t) callback, handle, user_data);

	error.code = SERVICE_ADAPTOR_ERROR_NONE;
	error.msg = NULL;
	ret = _dbus_request_chat(handle->service_name, request_id, chatroom_id, chat_msgs, chat_msgs_len, user_data, &error);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		service_adaptor_error_s *_error = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));
		if ((NULL != _error) && (NULL != error_code)) {
			_error->code = error.code;
			_error->msg = error.msg;
			*error_code = _error;
		} else {
			free(_error);
			free(error.msg);
		}
	}

	sac_api_end(ret);
	return ret;
}

/**	@brief	Requests Allow Chat
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_request_allow_chat(service_adaptor_h handle,
						long long int request_id,
						long long int chatroom_id,
						bool is_auto_allow,
						int max_count,
						bool need_delivery_ack,
						unsigned long long last_delivery_ack_timestamp,
						bool need_read_ack,
						unsigned long long last_read_ack_timestamp,
						bool need_ordered_chat_member_list,
						service_adaptor_reply_allow_chat_cb callback,
						service_adaptor_error_s **error_code,
						void *user_data)
{
	sac_api_start_msg(request_id);
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;

	if ((NULL == handle) || (NULL == handle->plugin)) {
		if (NULL != error_code) {
			service_adaptor_error_s *_error = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));
			if (NULL != _error) {
				_error->code = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
				_error->msg = strdup("Invalid Argument");
			}
			*error_code = _error;
		}

		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	_queue_add_task(request_id, (uint32_t) callback, handle, user_data);

	error.code = SERVICE_ADAPTOR_ERROR_NONE;
	error.msg = NULL;
	ret = _dbus_request_allow_chat(handle->service_name, request_id, chatroom_id, is_auto_allow, max_count,
			need_delivery_ack, last_delivery_ack_timestamp, need_read_ack, last_read_ack_timestamp,
			need_ordered_chat_member_list, user_data, &error);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		service_adaptor_error_s *_error = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));
		if ((NULL != _error) && (NULL != error_code)) {
			_error->code = error.code;
			_error->msg = error.msg;
			*error_code = _error;
		} else {
			free(_error);
			free(error.msg);
		}
	}

	sac_api_end(ret);
	return ret;
}

/**	@brief	Requests All Unread Message
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_request_all_unread_message(service_adaptor_h handle,
						long long int request_id,
						int max_count,
						service_adaptor_reply_all_unread_message_cb callback,
						service_adaptor_error_s **error_code,
						void *user_data)
{
	sac_api_start_msg(request_id);
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;

	if ((NULL == handle) || (NULL == handle->plugin)) {
		if (NULL != error_code) {
			service_adaptor_error_s *_error = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));
			if (NULL != _error) {
				_error->code = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
				_error->msg = strdup("Invalid Argument");
			}
			*error_code = _error;
		}

		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	_queue_add_task(request_id, (uint32_t) callback, handle, user_data);

	error.code = SERVICE_ADAPTOR_ERROR_NONE;
	error.msg = NULL;
	ret = _dbus_request_all_unread_message(handle->service_name, request_id, max_count, user_data, &error);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		service_adaptor_error_s *_error = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));
		if ((NULL != _error) && (NULL != error_code)) {
			_error->code = error.code;
			_error->msg = error.msg;
			*error_code = _error;
		} else {
			free(_error);
			free(error.msg);
		}
	}

	sac_api_end(ret);
	return ret;
}

/**	@brief	Registers Forward Online Message Listener
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_register_channel_disconnected_listener(service_adaptor_h handle,
						service_adaptor_reply_channel_disconnected_cb callback,
						void *user_data)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	_queue_add_task(TASK_MESSAGE_CHANNEL_DISCONNECTED_MESSAGE, (uint32_t) callback, handle, user_data);

	sac_api_end(ret);
	return ret;
}

/**	@brief	Unregisters Forward Online Message Listener
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_unregister_channel_disconnected_listener(service_adaptor_h handle,
						void *user_data)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	service_adaptor_task_h task = _queue_get_task(TASK_MESSAGE_CHANNEL_DISCONNECTED_MESSAGE);

	if (NULL == task) {
		return SERVICE_ADAPTOR_ERROR_UNKNOWN;
	}

	_queue_del_task(task);

	sac_api_end(ret);
	return ret;
}



/**	@brief	Registers Forward Online Message Listener
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_register_forward_online_message_listener(service_adaptor_h handle,
						service_adaptor_request_forward_online_message_cb callback,
						void *user_data)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	_queue_add_task(TASK_MESSAGE_FORWARD_ONLINE_MESSAGE, (uint32_t) callback, handle, user_data);

	sac_api_end(ret);
	return ret;
}

/**	@brief	Unregisters Forward Online Message Listener
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_unregister_forward_online_message_listener(service_adaptor_h handle,
						void *user_data)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	service_adaptor_task_h task = _queue_get_task(TASK_MESSAGE_FORWARD_ONLINE_MESSAGE);

	if (NULL == task) {
		return SERVICE_ADAPTOR_ERROR_UNKNOWN;
	}

	_queue_del_task(task);

	sac_api_end(ret);
	return ret;
}

/**	@brief	Requests Forward Online Message
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_reply_forward_online_message(service_adaptor_h handle,
						long long int request_id,
						long long int chatroom_id,
						bool mark_as_read,
						service_adaptor_error_s **error_code,
						void *user_data)
{
	sac_api_start_msg(request_id);
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;

	if ((NULL == handle) || (NULL == handle->plugin)) {
		if (NULL != error_code) {
			service_adaptor_error_s *_error = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));
			if (NULL != _error) {
				_error->code = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
				_error->msg = strdup("Invalid Argument");
			}
			*error_code = _error;
		}

		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	error.code = SERVICE_ADAPTOR_ERROR_NONE;
	error.msg = NULL;
	ret = _dbus_reply_forward_online_message(handle->service_name, request_id, chatroom_id, mark_as_read, user_data, &error);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		service_adaptor_error_s *_error = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));
		if ((NULL != _error) && (NULL != error_code)) {
			_error->code = error.code;
			_error->msg = error.msg;
			*error_code = _error;
		} else {
			free(_error);
			free(error.msg);
		}
	}

	sac_api_end(ret);
	return ret;
}

/**	@brief	Registers Forward Unread Message Listener
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_register_forward_unread_message_listener(service_adaptor_h handle,
						service_adaptor_request_forward_unread_message_cb callback,
						void *user_data)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	_queue_add_task(TASK_MESSAGE_FORWARD_UNREAD_MESSAGE, (uint32_t) callback, handle, user_data);

	sac_api_end(ret);
	return ret;
}

/**	@brief	Unregisters Forward Unread Message Listener
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_unregister_forward_unread_message_listener(service_adaptor_h handle,
						void *user_data)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	service_adaptor_task_h task = _queue_get_task(TASK_MESSAGE_FORWARD_UNREAD_MESSAGE);

	if (NULL == task) {
		return SERVICE_ADAPTOR_ERROR_UNKNOWN;
	}

	_queue_del_task(task);

	sac_api_end(ret);
	return ret;
}

/**	@brief	Requests Forward Unread Message
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_reply_forward_unread_message(service_adaptor_h handle,
						long long int request_id,
						const char *next_pagination_key,
						int max_count,
						service_adaptor_error_s **error_code,
						void *user_data)
{
	sac_api_start_msg(request_id);
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;

	if ((NULL == handle) || (NULL == handle->plugin)) {
		if (NULL != error_code) {
			service_adaptor_error_s *_error = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));
			if (NULL != _error) {
				_error->code = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
				_error->msg = strdup("Invalid Argument");
			}
			*error_code = _error;
		}

		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	error.code = SERVICE_ADAPTOR_ERROR_NONE;
	error.msg = NULL;
	ret = _dbus_reply_forward_unread_message(handle->service_name, request_id, next_pagination_key, max_count, user_data, &error);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		service_adaptor_error_s *_error = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));
		if ((NULL != _error) && (NULL != error_code)) {
			_error->code = error.code;
			_error->msg = error.msg;
			*error_code = _error;
		} else {
			free(_error);
			free(error.msg);
		}
	}

	sac_api_end(ret);
	return ret;
}

/**	@brief	Requests Read Message
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_request_read_message(service_adaptor_h handle,
						long long int request_id,
						long long int chatroom_id,
						service_adaptor_inbox_message_s *inbox_msg,
						service_adaptor_reply_read_message_cb callback,
						service_adaptor_error_s **error_code,
						void *user_data)
{
	sac_api_start_msg(request_id);
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;

	_queue_add_task(request_id, (uint32_t) callback, handle, user_data);

	if ((NULL == handle) || (NULL == handle->plugin) || (NULL == inbox_msg)) {
		if (NULL != error_code) {
			service_adaptor_error_s *_error = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));
			if (NULL != _error) {
				_error->code = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
				_error->msg = strdup("Invalid Argument");
			}
			*error_code = _error;
		}

		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	error.code = SERVICE_ADAPTOR_ERROR_NONE;
	error.msg = NULL;
	ret = _dbus_request_read_message(handle->service_name, request_id, chatroom_id, inbox_msg, user_data, &error);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		service_adaptor_error_s *_error = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));
		if ((NULL != _error) && (NULL != error_code)) {
			_error->code = error.code;
			_error->msg = error.msg;
			*error_code = _error;
		} else {
			free(_error);
			free(error.msg);
		}
	}

	sac_api_end(ret);
	return ret;
}

/**	@brief	Requests Invite Chat
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_request_invite_chat(service_adaptor_h handle,
						long long int request_id,
						long long int chatroom_id,
						long long int *inviting_members,
						unsigned int inviting_members_len,
						service_adaptor_reply_invite_chat_cb callback,
						service_adaptor_error_s **error_code,
						void *user_data)
{
	sac_api_start_msg(request_id);
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;

	if ((NULL == handle) || (NULL == handle->plugin) || (NULL == inviting_members) || (1 > inviting_members_len)) {
		if (NULL != error_code) {
			service_adaptor_error_s *_error = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));
			if (NULL != _error) {
				_error->code = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
				_error->msg = strdup("Invalid Argument");
			}
			*error_code = _error;
		}

		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	_queue_add_task(request_id, (uint32_t) callback, handle, user_data);

	error.code = SERVICE_ADAPTOR_ERROR_NONE;
	error.msg = NULL;
	ret = _dbus_request_invite_chat(handle->service_name, request_id, chatroom_id, inviting_members, inviting_members_len, user_data, &error);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		service_adaptor_error_s *_error = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));
		if ((NULL != _error) && (NULL != error_code)) {
			_error->code = error.code;
			_error->msg = error.msg;
			*error_code = _error;
		} else {
			free(_error);
			free(error.msg);
		}
	}

	sac_api_end(ret);
	return ret;
}

/**	@brief	Requests End Chat
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_request_end_chat(service_adaptor_h handle,
						long long int request_id,
						service_adaptor_end_chat_s **end_chats,
						unsigned int end_chats_len,
						service_adaptor_reply_end_chat_cb callback,
						service_adaptor_error_s **error_code,
						void *user_data)
{
	sac_api_start_msg(request_id);
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;

	if ((NULL == handle) || (NULL == handle->plugin) || (NULL == end_chats) || (1 > end_chats_len)) {
		if (NULL != error_code) {
			service_adaptor_error_s *_error = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));
			if (NULL != _error) {
				_error->code = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
				_error->msg = strdup("Invalid Argument");
			}
			*error_code = _error;
		}

		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	_queue_add_task(request_id, (uint32_t) callback, handle, user_data);

	error.code = SERVICE_ADAPTOR_ERROR_NONE;
	error.msg = NULL;
	ret = _dbus_request_end_chat(handle->service_name, request_id, end_chats, end_chats_len, user_data, &error);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		service_adaptor_error_s *_error = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));
		if ((NULL != _error) && (NULL != error_code)) {
			_error->code = error.code;
			_error->msg = error.msg;
			*error_code = _error;
		} else {
			free(_error);
			free(error.msg);
		}
	}

	sac_api_end(ret);
	return ret;
}

/**	@brief	Requests Unseal Message
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_request_unseal_message(service_adaptor_h handle,
						long long int request_id,
						long long int chatroom_id,
						long long int sender_id,
						long long int message_id,
						const char *message_detail,
						service_adaptor_reply_unseal_message_cb callback,
						service_adaptor_error_s **error_code,
						void *user_data)
{
	sac_api_start_msg(request_id);
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;

	if ((NULL == handle) || (NULL == handle->plugin) || (NULL == message_detail)) {
		if (NULL != error_code) {
			service_adaptor_error_s *_error = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));
			if (NULL != _error) {
				_error->code = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
				_error->msg = strdup("Invalid Argument");
			}
			*error_code = _error;
		}

		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	_queue_add_task(request_id, (uint32_t) callback, handle, user_data);

	error.code = SERVICE_ADAPTOR_ERROR_NONE;
	error.msg = NULL;
	ret = _dbus_request_unseal_message(handle->service_name, request_id, chatroom_id,
			sender_id, message_id, message_detail, user_data, &error);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		service_adaptor_error_s *_error = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));
		if ((NULL != _error) && (NULL != error_code)) {
			_error->code = error.code;
			_error->msg = error.msg;
			*error_code = _error;
		} else {
			free(_error);
			free(error.msg);
		}
	}

	sac_api_end(ret);
	return ret;
}

/**	@brief	Requests Save Call Log
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_request_save_call_log(service_adaptor_h handle,
						long long int request_id,
						long long int chatroom_id,
						const char *call_id,
						const char *call_log_type,
						long long int call_sender_id,
						long long int call_receiver_id,
						int conversaction_second,
						service_adaptor_reply_save_call_log_cb callback,
						service_adaptor_error_s **error_code,
						void *user_data)
{
	sac_api_start_msg(request_id);
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;

	if ((NULL == handle) || (NULL == handle->plugin)) {
		if (NULL != error_code) {
			service_adaptor_error_s *_error = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));
			if (NULL != _error) {
				_error->code = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
				_error->msg = strdup("Invalid Argument");
			}
			*error_code = _error;
		}

		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	_queue_add_task(request_id, (uint32_t) callback, handle, user_data);

	error.code = SERVICE_ADAPTOR_ERROR_NONE;
	error.msg = NULL;
	ret = _dbus_request_save_call_log(handle->service_name, request_id, chatroom_id,
			call_id, call_log_type, call_sender_id, call_receiver_id,
			conversaction_second, user_data, &error);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		service_adaptor_error_s *_error = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));
		if ((NULL != _error) && (NULL != error_code)) {
			_error->code = error.code;
			_error->msg = error.msg;
			*error_code = _error;
		} else {
			free(_error);
			free(error.msg);
		}
	}

	sac_api_end(ret);
	return ret;
}

/**	@brief	Requests Current Time
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_request_current_time(service_adaptor_h handle,
						long long int request_id,
						service_adaptor_reply_current_time_cb callback,
						service_adaptor_error_s **error_code,
						void *user_data)
{
	sac_api_start_msg(request_id);
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;

	if ((NULL == handle) || (NULL == handle->plugin)) {
		if (NULL != error_code) {
			service_adaptor_error_s *_error = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));
			if (NULL != _error) {
				_error->code = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
				_error->msg = strdup("Invalid Argument");
			}
			*error_code = _error;
		}

		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	_queue_add_task(request_id, (uint32_t) callback, handle, user_data);

	error.code = SERVICE_ADAPTOR_ERROR_NONE;
	error.msg = NULL;
	ret = _dbus_request_current_time(handle->service_name, request_id, user_data, &error);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		service_adaptor_error_s *_error = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));
		if ((NULL != _error) && (NULL != error_code)) {
			_error->code = error.code;
			_error->msg = error.msg;
			*error_code = _error;
		} else {
			free(_error);
			free(error.msg);
		}
	}

	sac_api_end(ret);
	return ret;
}

SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_request_get_connection_policy(service_adaptor_h handle,
						service_adaptor_connection_policy_e *policy,
						service_adaptor_error_s **error_code)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;

	if ((NULL == handle) || (NULL == handle->plugin) || (NULL == policy)) {
		if (NULL != error_code) {
			service_adaptor_error_s *_error = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));
			if (NULL != _error) {
				_error->code = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
				_error->msg = strdup("Invalid Argument");
			}
			*error_code = _error;
		}

		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	error.code = SERVICE_ADAPTOR_ERROR_NONE;
	error.msg = NULL;
	ret = _dbus_request_get_connection_policy(handle->service_name, policy, &error);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		service_adaptor_error_s *_error = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));
		if ((NULL != _error) && (NULL != error_code)) {
			_error->code = error.code;
			_error->msg = error.msg;
			*error_code = _error;
		} else {
			free(_error);
			free(error.msg);
		}
	}

	sac_api_end(ret);
	return ret;
}

SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_request_set_connection_policy(service_adaptor_h handle,
						service_adaptor_connection_policy_e *policy,
						service_adaptor_error_s **error_code)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;

	if ((NULL == handle) || (NULL == handle->plugin) || (NULL == policy)) {
		if (NULL != error_code) {
			service_adaptor_error_s *_error = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));
			if (NULL != _error) {
				_error->code = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
				_error->msg = strdup("Invalid Argument");
			}
			*error_code = _error;
		}

		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	error.code = SERVICE_ADAPTOR_ERROR_NONE;
	error.msg = NULL;
	ret = _dbus_request_set_connection_policy(handle->service_name, policy, &error);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		service_adaptor_error_s *_error = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));
		if ((NULL != _error) && (NULL != error_code)) {
			_error->code = error.code;
			_error->msg = error.msg;
			*error_code = _error;
		} else {
			free(_error);
			free(error.msg);
		}
	}

	sac_api_end(ret);
	return ret;
}



/**	@brief	Requests chat id based on phone number
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_get_chat_id_list(service_adaptor_h handle,
						service_adaptor_phone_number_s **phone_numbers,
						unsigned int phone_numbers_len,
						void *user_data,
						service_adaptor_chat_id_s ***chat_ids,
						unsigned int *chat_ids_len,
						service_adaptor_error_s **error_code,
						void **server_data)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;

	if ((NULL == handle) || (NULL == handle->plugin) || (NULL == phone_numbers) || (1 > phone_numbers_len) || (NULL == chat_ids) || (NULL == chat_ids_len)) {
		if (NULL != error_code) {
			service_adaptor_error_s *_error = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));
			if (NULL != _error) {
				_error->code = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
				_error->msg = strdup("Invalid Argument");
			}
			*error_code = _error;
		}

		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	if (service_adaptor_check_handle_validate(handle)) {
		service_adaptor_set_last_result(SERVICE_ADAPTOR_ERROR_INVALID_STATE, "Invalid handle (Please success set_auth first)");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	error.code = SERVICE_ADAPTOR_ERROR_NONE;
	error.msg = NULL;
	ret = _dbus_get_chat_id_list(handle->service_name, phone_numbers, phone_numbers_len, user_data, chat_ids, chat_ids_len, server_data, &error);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		service_adaptor_error_s *_error = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));
		if ((NULL != _error) && (NULL != error_code)) {
			_error->code = error.code;
			_error->msg = error.msg;
			*error_code = _error;
		} else {
			free(_error);
			free(error.msg);
		}
	}

	sac_api_end(ret);
	return ret;
}

/**	@brief	Requests MSISDN based on User ID
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_get_msisdn_list(service_adaptor_h handle,
						long long int *chat_ids,
						unsigned int chat_ids_len,
						void *user_data,
						service_adaptor_chat_id_s ***msisdns,
						unsigned int *msisdns_len,
						service_adaptor_error_s **error_code,
						void **server_data)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;

	if ((NULL == handle) || (NULL == handle->plugin) || (NULL == chat_ids) || (1 > chat_ids_len) || (NULL == msisdns) || (msisdns_len == NULL)) {
		if (NULL != error_code) {
			service_adaptor_error_s *_error = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));
			if (NULL != _error) {
				_error->code = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
				_error->msg = strdup("Invalid Argument");
			}
			*error_code = _error;
		}

		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	error.code = SERVICE_ADAPTOR_ERROR_NONE;
	error.msg = NULL;
	ret = _dbus_get_msisdn_list(handle->service_name, chat_ids, chat_ids_len, user_data, msisdns, msisdns_len, server_data, &error);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		service_adaptor_error_s *_error = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));
		if ((NULL != _error) && (NULL != error_code)) {
			_error->code = error.code;
			_error->msg = error.msg;
			*error_code = _error;
		} else {
			free(_error);
			free(error.msg);
		}
	}

	sac_api_end(ret);
	return ret;
}
