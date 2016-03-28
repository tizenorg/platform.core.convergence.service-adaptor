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
 * File: service-adaptor-client-message.h
 * Desc: Service Adaptor APIs
 * Created on: Oct, 2014
 * Auth: Yongjin Kim <youth.kim@samsung.com>
 *
 *****************************************************************************/
/**
 *	@file		service-adaptor-client-message.h
 *	@brief		Defines interface of Service Adaptor's Messaging
 *	@version	0.1
 */

#ifndef __SERVICE_ADAPTOR_CLIENT_MESSAGE_H__
#define __SERVICE_ADAPTOR_CLIENT_MESSAGE_H__

#include <stdbool.h>
#include "service-adaptor-client.h"
#include "service_adaptor_client_type.h"

typedef enum _service_adaptor_connection_policy_e {
	SERVICE_ADAPTOR_CONNECTION_POLICY_AUTO		= 0,
	SERVICE_ADAPTOR_CONNECTION_POLICY_CONNECT	= 1,
	SERVICE_ADAPTOR_CONNECTION_POLICY_DISCONNECT	= 2,
} service_adaptor_connection_policy_e;
/**
* @brief Describes infromation about violation users of wrong receivers
*/
typedef struct _service_adaptor_did_violation_users_s {
	long long int usera;
	long long int userb;
} service_adaptor_did_violation_users_s;

/**
* @brief Describes infromation about wrong receivers
*/
typedef struct _service_adaptor_wrong_receiver_s {
	long long int *invalid_receivers;		/**< specifies status as none*/
	unsigned int invalid_receivers_len;		/**< specifies status as none*/
	long long int *interrupted_receivers;		/**< specifies status as none*/
	unsigned int interrupted_receivers_len;		/**< specifies status as none*/
	long long int *disabled_receivers;		/**< specifies status as none*/
	unsigned int disabled_receivers_len;		/**< specifies status as none*/
	long long int *existing_chatmembers;		/**< specifies status as none*/
	unsigned int existing_chatmembers_len;		/**< specifies status as none*/
	service_adaptor_did_violation_users_s **did_violation_users;	/**< specifies status as none*/
	unsigned int did_violation_users_len;		/**< specifies status as none*/
	long long int *invitation_denieds;
	unsigned int invitation_denieds_len;

} service_adaptor_wrong_receiver_s;

/**
* @brief Describes infromation about chat message
*/
typedef struct _service_adaptor_chat_msg_s {
	long long int msg_id;		/**< specifies status as none*/
	int msg_type;			/**< specifies status as none*/
	char *chatmsg;			/**< specifies status as none*/
	int message_ttl;		/**< specifies status as none*/
} service_adaptor_chat_msg_s;

/**
* @brief Describes infromation about processed message
*/
typedef struct _service_adaptor_processed_msg_s {
	long long int msg_id;		/**< specifies status as none*/
	long long int sent_time;	/**< specifies status as none*/
} service_adaptor_processed_msg_s;

/**
* @brief Describes infromation about delivery ack
*/
typedef struct _service_adaptor_delivery_ack_s {
	long long int user_id;
	long long int msg_id;
	long long int timestamp;
} service_adaptor_delivery_ack_s;

/**
* @brief Describes infromation about read_ack
*/
typedef struct _service_adaptor_read_ack_s {
	long long int user_id;		/**< specifies status as none*/
	long long int msg_id;		/**< specifies status as none*/
	long long int timestamp;	/**< specifies status as none*/
} service_adaptor_read_ack_s;

/**
* @brief Describes infromation about ordered chat member
*/
typedef struct _service_adaptor_ordered_chat_member_s {
	long long int user_id;		/**< specifies status as none*/
	bool available;			/**< specifies status as none*/
	char *name;			/**< specifies status as none*/
} service_adaptor_ordered_chat_member_s;

/**
* @brief Describes infromation about inbox message
*/
typedef struct _service_adaptor_inbox_message_s {
	long long int msg_id;		/**< specifies status as none*/
	int msg_type;			/**< specifies status as none*/
	long long int sender;		/**< specifies status as none*/
	long long int receiver;		/**< specifies status as none*/
	long long int sent_time;	/**< specifies status as none*/
	char *chat_msg;			/**< specifies status as none*/
	long long int chatroom_id;	/**< specifies status as none*/
	int chat_type;			/**< specifies status as none*/
	int message_ttl;		/**< specifies status as none*/
} service_adaptor_inbox_message_s;

/**
* @brief Describes infromation about end chat
*/
typedef struct _service_adaptor_end_chat_s {
	long long int chatroom_id;	/**< specifies status as none*/
	bool deny_invitation;
} service_adaptor_end_chat_s;

/**
* @brief Describes infromation about phone number
*/
typedef struct _service_adaptor_phone_number_s {
	char *phonenumber;		/**< specifies status as none*/
	char *ccc;			/**< specifies status as none*/
} service_adaptor_phone_number_s;

/**
* @brief Describes infromation about chat id
*/
typedef struct _service_adaptor_chat_id_s {
	long long int chatid;		/**< specifies status as none*/
	char *msisdn;			/**< specifies status as none*/
} service_adaptor_chat_id_s;

#ifdef NOT_IN_USE

/**
* @brief Callback for service_adaptor_request_channel_auth API
*
* @param[in]	handle		specifies Service Adaptor handle. use this handle to get internal data
* @param[in]	request_id	specifies request id
* @param[in]	error_code	specifies error code
* @param[in]	server_data	specifies server_data passed in API
* @return	void
* @pre	service_adaptor_request_channel_auth will invoke this callback.
* @see
*/
typedef void(*service_adaptor_reply_channel_auth_cb)(service_adaptor_h handle,
						long long int request_id,
						service_adaptor_error_s *error_code,
						void *server_data);

/**
* @brief Callback for service_adaptor_request_client_echo API
*
* @param[in]	handle		specifies Service Adaptor handle. use this handle to get internal data
* @param[in]	request_id	specifies request id
* @param[in]	error_code	specifies error code
* @param[in]	server_data	specifies server_data passed in API
* @return	void
* @pre	service_adaptor_request_client_echo will invoke this callback.
* @see
*/
typedef void(*service_adaptor_reply_client_echo_cb)(service_adaptor_h handle,
						long long int request_id,
						service_adaptor_error_s *error_code,
						void *server_data);

#endif /* NOT_IN_USE */

typedef void (*service_adaptor_reply_channel_disconnected_cb)(service_adaptor_h handle,
						service_adaptor_error_s *error_code,
						void *server_data);

/**
* @brief Callback for service_adaptor_request_create_chatroom API
*
* @param[in]	handle		specifies Service Adaptor handle. use this handle to get internal data
* @param[in]	request_id	specifies request id
* @param[in]	chatroom_id	specifies created chatroom Id
* @param[in]	wrong_receiver	specifies invalid_receivers and disabled_receivers
* @param[in]	error_code	specifies error code
* @param[in]	server_data	specifies server_data passed in API
* @return	void
* @pre	service_adaptor_request_create_chatroom will invoke this callback.
* @see
*/
typedef void(*service_adaptor_reply_create_chatroom_cb)(service_adaptor_h handle,
						long long int request_id,
						long long int chatroom_id,
						int default_message_ttl,
						service_adaptor_wrong_receiver_s *wrong_receiver,
						service_adaptor_error_s *error_code,
						void *server_data);

typedef void (*service_adaptor_reply_change_chatroom_meta_cb)(service_adaptor_h handle,
						long long int request_id,
						long long int chatroom_id,
						service_adaptor_error_s *error_code,
						void *server_data);


/**
* @brief Callback for service_adaptor_request_chat API
*
* @param[in]	handle		specifies Service Adaptor handle. use this handle to get internal data
* @param[in]	request_id	specifies request id
* @param[in]	chatroom_id	specifies created chatroom Id
* @param[in]	processed_msgs	specifies processed messages
* @param[in]	processed_msgs_len	specifies length of processed_msgs
* @param[in]	error_code	specifies error code
* @param[in]	server_data	specifies server_data passed in API
* @return	void
* @pre	service_adaptor_request_chat will invoke this callback.
* @see
*/
typedef void(*service_adaptor_reply_chat_cb)(service_adaptor_h handle,
						long long int request_id,
						long long int chatroom_id,
						service_adaptor_processed_msg_s **processed_msgs,
						unsigned int processed_msgs_len,
						service_adaptor_error_s *error_code,
						void *server_data);

/**
* @brief Callback for service_adaptor_request_allow_chat API
*
* @param[in]	handle		specifies Service Adaptor handle. use this handle to get internal data
* @param[in]	request_id	specifies request id
* @param[in]	chatroom_id	specifies created chatroom Id
* @param[in]	read_acks	specifies read_acks
* @param[in]	read_acks_len	specifies length of read_acks
* @param[in]	ordered_chat_members		specifies ordered chat members
* @param[in]	ordered_chat_members_len	specifies length of ordered_chat_members
* @param[in]	error_code	specifies error code
* @param[in]	server_data	specifies server_data passed in API
* @return	void
* @pre	service_adaptor_request_allow_chat will invoke this callback.
* @see
*/
typedef void(*service_adaptor_reply_allow_chat_cb)(service_adaptor_h handle,
						long long int request_id,
						long long int chatroom_id,
						service_adaptor_delivery_ack_s **delivery_acks,
						unsigned int delivery_acks_len,
						unsigned long long last_delivery_ack_timestamp,
						service_adaptor_read_ack_s **read_acks,
						unsigned int read_acks_len,
						unsigned long long last_read_ack_timestamp,
						service_adaptor_ordered_chat_member_s **ordered_chat_members,
						unsigned int ordered_chat_members_len,
						const char *chatroom_title,
						int default_message_ttl,
						service_adaptor_error_s *error_code,
						void *server_data);

/**
* @brief Callback for service_adaptor_request_all_unread_message API
*
* @param[in]	handle		specifies Service Adaptor handle. use this handle to get internal data
* @param[in]	request_id	specifies request id
* @param[in]	error_code	specifies error code
* @param[in]	server_data	specifies server_data passed in API
* @return	void
* @pre	service_adaptor_request_all_unread_message will invoke this callback.
* @see
*/
typedef void(*service_adaptor_reply_all_unread_message_cb)(service_adaptor_h handle,
						long long int request_id,
						service_adaptor_error_s *error_code,
						void *server_data);

/**
* @brief Callback for service_adaptor_register_forward_online_message_listener API
*
* @param[in]	handle		specifies Service Adaptor handle. use this handle to get internal data
* @param[in]	request_id	specifies request id
* @param[in]	chatroom_id	specifies created chatroom Id
* @param[in]	chat_type	specifies type of chat
* @param[in]	inbox_msg	specifies inbox message
* @param[in]	error_code	specifies error code
* @param[in]	server_data	specifies server_data passed in API
* @return	void
* @pre	service_adaptor_register_forward_online_message_listener will invoke this callback.
* @see
*/
typedef void(*service_adaptor_request_forward_online_message_cb)(service_adaptor_h handle,
						long long int request_id,
						long long int chatroom_id,
						int chat_type,
						service_adaptor_inbox_message_s *inbox_msg,
						bool skip_reply,
						service_adaptor_error_s *error_code,
						void *server_data);

/**
* @brief Callback for service_adaptor_register_forward_online_message_listener API
*
* @param[in]	handle		specifies Service Adaptor handle. use this handle to get internal data
* @param[in]	request_id	specifies request id
* @param[in]	inbox_msgs	specifies inbox message
* @param[in]	inbox_msgs_len	specifies length of inbox message
* @param[in]	next_pagination_key	specifies pagination function's next page value
* @param[in]	error_code	specifies error code
* @param[in]	server_data	specifies server_data passed in API
* @return	void
* @pre	service_adaptor_register_forward_online_message_listener will invoke this callback.
* @see
*/
typedef void(*service_adaptor_request_forward_unread_message_cb)(service_adaptor_h handle,
						long long int request_id,
						service_adaptor_inbox_message_s **inbox_msgs,
						unsigned int inbox_msgs_len,
						const char *next_pagination_key,
						service_adaptor_error_s *error_code,
						void *server_data);

/**
* @brief Callback for service_adaptor_request_read_message API
*
* @param[in]	handle		specifies Service Adaptor handle. use this handle to get internal data
* @param[in]	request_id	specifies request id
* @param[in]	chatroom_id	specifies created chatroom Id
* @param[in]	error_code	specifies error code
* @param[in]	server_data	specifies server_data passed in API
* @return	void
* @pre	service_adaptor_request_read_message will invoke this callback.
* @see
*/
typedef void(*service_adaptor_reply_read_message_cb)(service_adaptor_h handle,
						long long int request_id,
						long long int chatroom_id,
						service_adaptor_error_s *error_code,
						void *server_data);

/**
* @brief Callback for service_adaptor_request_invite_chat API
*
* @param[in]	handle		specifies Service Adaptor handle. use this handle to get internal data
* @param[in]	request_id	specifies request id
* @param[in]	chatroom_id	specifies created chatroom Id
* @param[in]	sent_time	specifies "InviteRequest" time received from server
* @param[in]	wrong_receiver	specifies invalid_receivers and disabled_receivers
* @param[in]	error_code	specifies error code
* @param[in]	server_data	specifies server_data passed in API
* @return	void
* @pre	service_adaptor_request_invite_chat will invoke this callback.
* @see
*/
typedef void(*service_adaptor_reply_invite_chat_cb)(service_adaptor_h handle,
						long long int request_id,
						long long int chatroom_id,
						long long int sent_time,
						service_adaptor_wrong_receiver_s *wrong_receiver,
						service_adaptor_error_s *error_code,
						void *server_data);

/**
* @brief Callback for service_adaptor_request_end_chat API
*
* @param[in]	handle		specifies Service Adaptor handle. use this handle to get internal data
* @param[in]	request_id	specifies request id
* @param[in]	error_code	specifies error code
* @param[in]	server_data	specifies server_data passed in API
* @return	void
* @pre	service_adaptor_request_end_chat will invoke this callback.
* @see
*/
typedef void(*service_adaptor_reply_end_chat_cb)(service_adaptor_h handle,
						long long int request_id,
						service_adaptor_error_s *error_code,
						void *server_data);

/**
* @brief Callback for service_adaptor_request_unseal_message API
*
* @param[in]	handle		specifies Service Adaptor handle. use this handle to get internal data
* @param[in]	request_id	specifies request id
* @param[in]	chatroom_id	specifies chatroom id
* @param[in]	error_code	specifies error code
* @param[in]	server_data	specifies server_data passed in API
* @return	void
* @pre	service_adaptor_request_unseal_message will invoke this callback.
* @see
*/
typedef void(*service_adaptor_reply_unseal_message_cb)(service_adaptor_h handle,
						long long int request_id,
						long long int chatroom_id,
						service_adaptor_error_s *error_code,
						void *server_data);


/**
* @brief Callback for service_adaptor_request_save_call_log API
*
* @param[in]	handle		specifies Service Adaptor handle. use this handle to get internal data
* @param[in]	request_id	specifies request id
* @param[in]	error_code	specifies error code
* @param[in]	server_data	specifies server_data passed in API
* @return	void
* @pre	service_adaptor_request_save_call_log will invoke this callback.
* @see
*/
typedef void(*service_adaptor_reply_save_call_log_cb)(service_adaptor_h handle,
						long long int request_id,
						service_adaptor_error_s *error_code,
						void *server_data);

/**
* @brief Callback for service_adaptor_request_current_time API
*
* @param[in]	handle		specifies Service Adaptor handle. use this handle to get internal data
* @param[in]	request_id	specifies request id
* @param[in]	current_time_millis	specifies current time (ms)
* @param[in]	error_code	specifies error code
* @param[in]	server_data	specifies server_data passed in API
* @return	void
* @pre	service_adaptor_request_current_time will invoke this callback.
* @see
*/
typedef void(*service_adaptor_reply_current_time_cb)(service_adaptor_h handle,
						long long int request_id,
						long long int current_time_millis,
						service_adaptor_error_s *error_code,
						void *server_data);

/*==================================================================================================
					FUNCTION PROTOTYPES
==================================================================================================*/

#ifdef NOT_IN_USE
/**
* @brief Requests Channel Auth
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	request_id	specifies packet Id
* @param[in]	uid		specifies DUID
* @param[in]	duid		specifies DUID
* @param[in]	appid		specifies appid
* @param[in]	access_token	specifies access token
* @param[in]	callback	the callback function to invoke
* @param[out]	error_code	specifies error code
* @param[in]	user_data	specifies user_data (json) passed in API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_RESULT_SUCCEEDED if Successful
*/
int service_adaptor_request_channel_auth(service_adaptor_h handle,
						long long int request_id,
						long long int uid,
						long long int duid,
						const char *appid,
						const char *access_token,
						service_adaptor_reply_channel_auth_cb callback,
						service_adaptor_error_s **error_code,
						void *user_data);

/**
* @brief Requests Client Echo
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	request_id	specifies packet Id
* @param[in]	callback	the callback function to invoke
* @param[out]	error_code	specifies error code
* @param[in]	user_data	specifies user_data (json) passed in API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_RESULT_SUCCEEDED if Successful
*/
int service_adaptor_request_client_echo(service_adaptor_h handle,
						long long int request_id,
						service_adaptor_reply_client_echo_cb callback,
						service_adaptor_error_s **error_code,
						void *user_data);

#endif /* NOT_IN_USE */

/**
* @brief Requests Creating Chatroom
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	request_id	specifies packet Id
* @param[in]	chat_type	specifies type of chat (Single: 0, Group: 1)
* @param[in]	receivers	specifies receivers
* @param[in]	receivers_len	specifies length of receivers
* @param[in]	callback	the callback function to invoke
* @param[out]	error_code	specifies error code
* @param[in]	user_data	specifies user_data (json) passed in API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_RESULT_SUCCEEDED if Successful
*/
int service_adaptor_request_create_chatroom(service_adaptor_h handle,
						long long int request_id,
						int chat_type,
						long long int *receivers,
						unsigned int receivers_len,
						const char *chatroom_title,
						service_adaptor_reply_create_chatroom_cb callback,
						service_adaptor_error_s **error_code,
						void *user_data);

int service_adaptor_request_change_chatroom_meta(service_adaptor_h handle,
						long long int request_id,
						long long int chatroom_id,
						const char *chatroom_title,
						int default_message_ttl,
						service_adaptor_reply_change_chatroom_meta_cb callback,
						service_adaptor_error_s **error_code,
						void *user_data);

/**
* @brief Requests Chat
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	request_id	specifies packet Id
* @param[in]	chatroom_id	specifies created chatroom Id
* @param[in]	chat_msgs	specifies information of chat message
* @param[in]	chat_msgs_len	specifies length of chat_msgs
* @param[in]	callback	the callback function to invoke
* @param[out]	error_code	specifies error code
* @param[in]	user_data	specifies user_data (json) passed in API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_RESULT_SUCCEEDED if Successful
*/
int service_adaptor_request_chat(service_adaptor_h handle,
						long long int request_id,
						long long int chatroom_id,
						service_adaptor_chat_msg_s **chat_msgs,
						unsigned int chat_msgs_len,
						service_adaptor_reply_chat_cb callback,
						service_adaptor_error_s **error_code,
						void *user_data);

/**
* @brief Allows Chat
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	request_id	specifies packet Id
* @param[in]	chatroom_id	specifies created chatroom Id
* @param[in]	max_count	specifies the number of message
* @param[in]	need_delivery_ack		specifies requesting delivery ask's data
* @param[in]	last_delivery_ack_timestamp	specifies last delivery ask's timestamp
* @param[in]	need_read_ack			specifies requesting read_ack's data
* @param[in]	last_read_ack_timestamp	specifies last read_ack's timestamp
* @param[in]	need_ordered_chat_member_list	specifies requesting ordered member list
* @param[in]	callback	the callback function to invoke
* @param[out]	error_code	specifies error code
* @param[in]	user_data	specifies user_data (json) passed in API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_RESULT_SUCCEEDED if Successful
*/
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
						void *user_data);

/**
* @brief Requests All Unread Message
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	request_id	specifies packet Id
* @param[in]	max_count	specifies the number of message
* @param[in]	callback	the callback function to invoke
* @param[out]	error_code	specifies error code
* @param[in]	user_data	specifies user_data (json) passed in API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_RESULT_SUCCEEDED if Successful
*/
int service_adaptor_request_all_unread_message(service_adaptor_h handle,
						long long int request_id,
						int max_count,
						service_adaptor_reply_all_unread_message_cb callback,
						service_adaptor_error_s **error_code,
						void *user_data);


int service_adaptor_register_channel_disconnected_listener(service_adaptor_h handle,
						service_adaptor_reply_channel_disconnected_cb callback,
						void *user_data);

int service_adaptor_unregister_channel_disconnected_listener(service_adaptor_h handle,
						void *user_data);


/**
* @brief Registers Forward Online Message Listener
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	callback	the callback function to invoke
* @param[in]	user_data	specifies user_data passed in API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_RESULT_SUCCEEDED if Successful
*/
int service_adaptor_register_forward_online_message_listener(service_adaptor_h handle,
						service_adaptor_request_forward_online_message_cb callback,
						void *user_data);

/**
* @brief Unregisters Forward Online Message Listener
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	user_data	specifies user_data passed in API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_RESULT_SUCCEEDED if Successful
*/
int service_adaptor_unregister_forward_online_message_listener(service_adaptor_h handle,
						void *user_data);

/**
* @brief Requests Forward Online Message
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	request_id	specifies packet Id
* @param[in]	chatroom_id	specifies created chatroom Id
* @param[in]	mark_as_read	specifies mark as read
* @param[out]	error_code	specifies error code
* @param[in]	user_data	specifies user_data (json) passed in API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_RESULT_SUCCEEDED if Successful
*/
int service_adaptor_reply_forward_online_message(service_adaptor_h handle,
						long long int request_id,
						long long int chatroom_id,
						bool mark_as_read,
						service_adaptor_error_s **error_code,
						void *user_data);

/**
* @brief Registers Forward Unread Message Listener
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	callback	the callback function to invoke
* @param[in]	user_data	specifies user_data passed in API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_RESULT_SUCCEEDED if Successful
*/
int service_adaptor_register_forward_unread_message_listener(service_adaptor_h handle,
						service_adaptor_request_forward_unread_message_cb callback,
						void *user_data);

/**
* @brief Unregisters Forward Unread Message Listener
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	user_data	specifies user_data passed in API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_RESULT_SUCCEEDED if Successful
*/
int service_adaptor_unregister_forward_unread_message_listener(service_adaptor_h handle,
						void *user_data);

/**
* @brief Requests Forward Unread Message
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	request_id	specifies packet Id
* @param[in]	next_pagination_key	specifies pagination function's next page value
* @param[in]	max_count	specifies the number of message
* @param[out]	error_code	specifies error code
* @param[in]	user_data	specifies user_data (json) passed in API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_RESULT_SUCCEEDED if Successful
*/
int service_adaptor_reply_forward_unread_message(service_adaptor_h handle,
						long long int request_id,
						const char *next_pagination_key,
						int max_count,
						service_adaptor_error_s **error_code,
						void *user_data);

/**
* @brief Requests Read Message
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	request_id	specifies packet Id
* @param[in]	chatroom_id	specifies created chatroom Id
* @param[in]	inbox_msg	specifies inbox message
* @param[in]	callback	the callback function to invoke
* @param[out]	error_code	specifies error code
* @param[in]	user_data	specifies user_data (json) passed in API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_RESULT_SUCCEEDED if Successful
*/
int service_adaptor_request_read_message(service_adaptor_h handle,
						long long int request_id,
						long long int chatroom_id,
						service_adaptor_inbox_message_s *inbox_msg,
						service_adaptor_reply_read_message_cb callback,
						service_adaptor_error_s **error_code,
						void *user_data);

/**
* @brief Requests Invite Chat
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	request_id	specifies packet Id
* @param[in]	chatroom_id	specifies created chatroom Id
* @param[in]	inviting_members	specifies invited user id
* @param[in]	inviting_members_len	specifies length of invited user id
* @param[in]	callback	the callback function to invoke
* @param[out]	error_code	specifies error code
* @param[in]	user_data	specifies user_data (json) passed in API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_RESULT_SUCCEEDED if Successful
*/
int service_adaptor_request_invite_chat(service_adaptor_h handle,
						long long int request_id,
						long long int chatroom_id,
						long long int *inviting_members,
						unsigned int inviting_members_len,
						service_adaptor_reply_invite_chat_cb callback,
						service_adaptor_error_s **error_code,
						void *user_data);

/**
* @brief Requests End Chat
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	request_id	specifies packet Id
* @param[in]	end_chats	specifies list of created chatroom id
* @param[in]	end_chats_len	specifies length of end_chats
* @param[in]	callback	the callback function to invoke
* @param[out]	error_code	specifies error code
* @param[in]	user_data	specifies user_data (json) passed in API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_RESULT_SUCCEEDED if Successful
*/
int service_adaptor_request_end_chat(service_adaptor_h handle,
						long long int request_id,
						service_adaptor_end_chat_s **end_chats,
						unsigned int end_chats_len,
						service_adaptor_reply_end_chat_cb callback,
						service_adaptor_error_s **error_code,
						void *user_data);

/**
* @brief Requests Unseal Message
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	request_id	specifies packet Id
* @param[in]	chatroom_id	specifies chatroom Id
* @param[in]	sender_id	specifies message sender Id for 'Unseal Message'
* @param[in]	message_id	specifies message Id for 'Unseal Message'
* @param[in]	callback	the callback function to invoke
* @param[out]	error_code	specifies error code
* @param[in]	user_data	specifies user_data (json) passed in API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_RESULT_SUCCEEDED if Successful
*/
int service_adaptor_request_unseal_message(service_adaptor_h handle,
						long long int request_id,
						long long int chatroom_id,
						long long int sender_id,
						long long int message_id,
						const char *message_detail,
						service_adaptor_reply_unseal_message_cb callback,
						service_adaptor_error_s **error_code,
						void *user_data);

/**
* @brief Requests Save Call Log
*
* @param[in]	handle			specifies Service Adaptor handle
* @param[in]	request_id		specifies packet Id
* @param[in]	chatroom_id		specifies chatroom Id
* @param[in]	call_id			specifies unique call Id
* @param[in]	call_log_type		specifies call log type string
* @param[in]	call_sender_id		specifies caller's unique Id
* @param[in]	call_receiver_id	specifies callee's unique Id
* @param[in]	conversaction_second	specifies calling time
* @param[in]	callback		the callback function to invoke
* @param[out]	error_code		specifies error code
* @param[in]	user_data		specifies user_data (json) passed in API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_RESULT_SUCCEEDED if Successful
*/
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
						void *user_data);

/**
* @brief Requests Current Time
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	request_id	specifies packet Id
* @param[in]	callback	the callback function to invoke
* @param[out]	error_code	specifies error code
* @param[in]	user_data	specifies user_data (json) passed in API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_RESULT_SUCCEEDED if Successful
*/
int service_adaptor_request_current_time(service_adaptor_h handle,
						long long int request_id,
						service_adaptor_reply_current_time_cb callback,
						service_adaptor_error_s **error_code,
						void *user_data);


int service_adaptor_request_get_connection_policy(service_adaptor_h handle,
						service_adaptor_connection_policy_e *policy,
						service_adaptor_error_s **error_code);

int service_adaptor_request_set_connection_policy(service_adaptor_h handle,
						service_adaptor_connection_policy_e *policy,
						service_adaptor_error_s **error_code);
/**
* @brief Requests chat id based on phone number
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	phone_numbers		list of phone number
* @param[in]	phone_numbers_len	length of phone_numbers
* @param[in]	user_data	specifies user_data (json) passed in API
* @param[out]	chat_ids	list of chat id
* @param[out]	chat_ids_len	length of chat_ids
* @param[out]	error_code	specifies error code
* @param[out]	server_data	specifies server_data (json) passed in API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_RESULT_SUCCEEDED if Successful
*/
int service_adaptor_get_chat_id_list(service_adaptor_h handle,
						service_adaptor_phone_number_s **phone_numbers,
						unsigned int phone_numbers_len,
						void *user_data,
						service_adaptor_chat_id_s ***chat_ids,
						unsigned int *chat_ids_len,
						service_adaptor_error_s **error_code,
						void **server_data);

/**
* @brief Requests MSISDN based on User ID
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	chat_ids	list of chat id
* @param[in]	chat_ids_len	length of chat_ids
* @param[in]	user_data	specifies user_data (json) passed in API
* @param[out]	msisdns		list of msisdn
* @param[out]	msisdns_len	length of msisdns
* @param[out]	error_code	specifies error code
* @param[out]	server_data	specifies server_data (json) passed in API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_RESULT_SUCCEEDED if Successful
*/
int service_adaptor_get_msisdn_list(service_adaptor_h handle,
						long long int *chat_ids,
						unsigned int chat_ids_len,
						void *user_data,
						service_adaptor_chat_id_s ***msisdns,
						unsigned int *msisdns_len,
						service_adaptor_error_s **error_code,
						void **server_data);

#endif /* __SERVICE_ADAPTOR_CLIENT_MESSAGE_H__ */
