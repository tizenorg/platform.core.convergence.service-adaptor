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

#ifndef __MESSAGE_ADAPTOR_H__
#define __MESSAGE_ADAPTOR_H__

#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <pthread.h>


#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif

#include <glib.h>

/**
 * Message adaptor error code
 */
typedef enum message_error_code_e {
	MESSAGE_ADAPTOR_ERROR_NONE                     =  0,
	MESSAGE_ADAPTOR_ERROR_LAUNCH                    = 1,    /**< 1 ~ 99: internal error*/
	MESSAGE_ADAPTOR_ERROR_INIT                      = 2,
	MESSAGE_ADAPTOR_ERROR_DEINIT                    = 3,
	MESSAGE_ADAPTOR_ERROR_CREATE                    = 4,
	MESSAGE_ADAPTOR_ERROR_DESTROY                   = 5,
	MESSAGE_ADAPTOR_ERROR_START                     = 6,
	MESSAGE_ADAPTOR_ERROR_STOP                      = 7,
	MESSAGE_ADAPTOR_ERROR_CONNECT                   = 8,
	MESSAGE_ADAPTOR_ERROR_DISCONNECT                = 9,
	MESSAGE_ADAPTOR_ERROR_NOT_FOUND                 = 10,
	MESSAGE_ADAPTOR_ERROR_CORRUPTED                 = 11,
	MESSAGE_ADAPTOR_ERROR_UNSUPPORTED               = 12,
	MESSAGE_ADAPTOR_ERROR_INVALID_HANDLE            = 13,
	MESSAGE_ADAPTOR_ERROR_INVALID_ARGUMENT          = 14,
	MESSAGE_ADAPTOR_ERROR_INVALID_ARGUMENT_TYPE     = 15,
	MESSAGE_ADAPTOR_ERROR_NOT_AUTHORIZED            = 16,
	MESSAGE_ADAPTOR_ERROR_ADAPTOR_INTERNAL          = 17,
	MESSAGE_ADAPTOR_ERROR_PLUGIN_INTERNAL           = 18,	/* input error code and message issued from curl or http or message_plugin_internal_error_code_e(defined by developer manually) */
	MESSAGE_ADAPTOR_ERROR_SERVER_INTERNAL           = 19,	/* input error code and message issued from server. */
	MESSAGE_ADAPTOR_ERROR_DBUS                      = 20,
	MESSAGE_ADAPTOR_ERROR_TIME_OUT			= 21,
	MESSAGE_ADAPTOR_ERROR_MAX
} message_error_code_t;

/**
 * @ brief Message plugin internal error code
 * @ details When a plugin returns MESSAGE_ADAPTOR_ERROR_PLUGIN_INTERNAL, input this number to message_adaptor_error_code_s.code
 */
typedef enum _message_plugin_internal_error_code_e {
	MESSAGE_PLUGIN_ERROR_HTTP_BAD_REQUEST           = 400,
	MESSAGE_PLUGIN_ERROR_HTTP_UNAUTHORIZED          = 401,
	MESSAGE_PLUGIN_ERROR_HTTP_FORBIDDEN             = 403,
	MESSAGE_PLUGIN_ERROR_HTTP_NOT_FOUND             = 404,
	MESSAGE_PLUGIN_ERROR_HTTP_METHOD_NOT_ALLOWED    = 405,
	MESSAGE_PLUGIN_ERROR_HTTP_BAD_GATEWAY           = 502,
	MESSAGE_PLUGIN_ERROR_HTTP_SERVICE_UNAVAILBLE    = 503,
	MESSAGE_PLUGIN_ERROR_HTTP_INSUFFICIENT_STORAGE  = 507,
	MESSAGE_PLUGIN_ERROR_HTTP_ETC                   = 598,
	MESSAGE_PLUGIN_ERROR_HTTP_UNKNOWN               = 599,

	MESSAGE_PLUGIN_ERROR_NETWORK_DEVICE_OFFLINE	= 601,
	MESSAGE_PLUGIN_ERROR_NETWORK_DEVICE_CONFUSED	= 602,
	MESSAGE_PLUGIN_ERROR_NETWORK_SOCKET_ISSUE	= 603,
	MESSAGE_PLUGIN_ERROR_NETWORK_SERVER_NOT_RESPONSE = 604,
	MESSAGE_PLUGIN_ERROR_NEWTORK_ETC		= 648,
	MESSAGE_PLUGIN_ERROR_NEWTORK_UNKNOWN		= 649,

	MESSAGE_PLUGIN_ERROR_AUTH_FAILED		= 701,
	MESSAGE_PLUGIN_ERROR_AUTH_ETC                   = 718,
	MESSAGE_PLUGIN_ERROR_AUTH_UNKNOWN               = 719,

	MESSAGE_PLUGIN_ERROR_MEMORY_ALLOCATION_FAILED   = 801,
	MESSAGE_PLUGIN_ERROR_MEMORY_ETC                 = 808,
	MESSAGE_PLUGIN_ERROR_MEMORY_UNKNOWN             = 809,

	MESSAGE_PLUGIN_ERROR_THREAD_CREATE_FAILED       = 821,
	MESSAGE_PLUGIN_ERROR_THREAD_STOPPED             = 822,
	MESSAGE_PLUGIN_ERROR_THREAD_ETC                 = 828,
	MESSAGE_PLUGIN_ERROR_THREAD_UNNOWN              = 829,

	MESSAGE_PLUGIN_ERROR_ETC                        = 998,
	MESSAGE_PLUGIN_ERROR_UNKNOWN                    = 999,
} message_plugin_internal_error_code_e;

typedef enum _message_connection_policy_e {
	MESSAGE_CONNECTION_POLICY_AUTO		= 0,
	MESSAGE_CONNECTION_POLICY_CONNECT	= 1,
	MESSAGE_CONNECTION_POLICY_DISCONNECT	= 2,
} message_connection_policy_e;

/**
 * @ brief Message plugin's TCP connection state flag
 */
typedef enum message_connection_state_e {
	MESSAGE_CONNECTION_STATE_INIT		= 0,	/* init value (after create_context) */
	MESSAGE_CONNECTION_STATE_READY		= 1,	/* thread running (after connect) before channel_auth */
	MESSAGE_CONNECTION_STATE_CONNECT	= 2,	/* connection authenticated (after channel_auth_reply) */
	MESSAGE_CONNECTION_STATE_DISCONNECTED	= 3,	/* connection was stopped explicitly (after disconnect) */
	MESSAGE_CONNECTION_STATE_INTERRUPTED	= 4,	/* connection was stopped inadventently (by network/server/etc issue) */
	MESSAGE_CONNECTION_STATE_MAX		= 5,
} message_connection_state_t;

/**
 * @ brief Message adaptor plugin handle
 */
typedef struct message_adaptor_plugin_s *message_adaptor_plugin_h;

/**
 * @ brief Message adaptor
 */
typedef struct message_adaptor_s *message_adaptor_h;

/**
 * @ brief Message adaptor error code
 */
typedef struct message_adaptor_error_code_s {
	char *code;
	char *msg;
} message_adaptor_error_code_t;
typedef struct message_adaptor_error_code_s *message_adaptor_error_code_h;

/**
 * @ brief Message adaptor violated user structure
 */
typedef struct message_adaptor_did_violation_users_s {
	long long int usera;
	long long int userb;
} message_adaptor_did_violation_users_t;
typedef struct message_adaptor_did_violation_users_s *message_adaptor_did_violation_users_h;

/**
 * @ brief Message adaptor wrong receiver structure
 */
typedef struct {
	long long int *invalid_receivers;
	unsigned int invalid_receivers_len;
	long long int *interrupted_receivers;
	unsigned int interrupted_receivers_len;
	long long int *disabled_receivers;
	unsigned int disabled_receivers_len;
	long long int *existing_chatmember;
	unsigned int existing_chatmembers_len;
	struct message_adaptor_did_violation_users_s * did_violation_users;
	unsigned int did_violation_users_len;
	long long int *invitation_denieds;
	unsigned int invitation_denieds_len;
} message_adaptor_wrong_receiver_s;

/**
 * @ brief Message adaptor chat message structure
 */
typedef struct {
	long long int msg_id;
	int msg_type;
	char *chatmsg;
	int message_ttl;
} message_adaptor_chat_msg_s;

/**
 * @ brief Message adaptor processed message structure
 */
typedef struct {
	long long int msg_id;
	long long int sent_time;
} message_adaptor_processed_msg_s;

/**
 * @ brief Message adaptor deliveryAck structure
 */
typedef struct {
	long long int userId;
	long long int msgId;
	long long int timestamp;
} message_adaptor_delivery_ack_s;

/**
 * @ brief Message adaptor read_ack structure
 */
typedef struct {
	long long int userId;
	long long int msgId;
	long long int timestamp;
} message_adaptor_read_ack_s;

/**
 * @ brief Message adaptor ordered chat member structure
 */
typedef struct {
	long long int userId;
	long long int available;
	char *name;
} message_adaptor_ordered_chat_member_s;

/**
 * @ brief Message adaptor inbox entry structure
 */
typedef struct _message_inboxentry {
	long long int msgId;
	int msgType;
	long long int sender;
	long long int receiver;
	long long int sentTime;
	char *chatMsg;
	long long int chatroomId;
	int chatType;
	int message_ttl;
} message_inboxentry_t;

typedef message_inboxentry_t message_adaptor_inbox_message_s;

/**
 * @ brief Message adaptor plugin context structure
 */
typedef struct message_adaptor_plugin_context_s {
	long long int duid;
	char *access_token;
	char *app_id;
	int service_id;
	char *app_key;
	char *uid;
	char *imei;
	char *imsi;

	GMutex connection_state_mutex;
	message_connection_state_t connection_state;
	message_connection_policy_e connection_policy;

	/* Encryption */
	unsigned char enc_key[32];
	unsigned char enc_vec[16];
	/* bool enc_key_updated; */
	unsigned char gpb_key[32];
	unsigned char gpb_vec[16];
	/* bool gpb_key_updated; */
	unsigned char exp_key[32];
	unsigned char exp_vec[16];
	/* bool exp_key_updated; */

	char *plugin_uri;

	GMutex plugin_data_mutex;
	void *plugin_data;
} message_adaptor_plugin_context_t;

typedef struct message_adaptor_plugin_context_s *message_adaptor_plugin_context_h;

/**
 * @ brief Message adaptor result code for internal use
 */
typedef enum message_plugin_result_code_e {
	MESSAGE_PLUGIN_RESULT_SUCCEDED = 0,
	MESSAGE_PLUGIN_RESULT_FAILED = -1,
	MESSAGE_PLUGIN_RESULT_CANCELED = -2
} message_plugin_result_code_t;

typedef struct curl_cb_data_s {
	char *data;
	int size;
} curl_cb_data_t;

/**
 * @ brief Message adaptor phone number structure
 */
typedef struct {
	char *phonenumber;
	char *ccc;
} message_adaptor_phone_number_s;

/**
 * @ brief Message adaptor chat id structure
 */
typedef struct {
	long long int chatid;
	char *msisdn;
} message_adaptor_chat_id_s;

/**
 * @ brief Message adaptor end chat structure
 */
typedef struct {
	long long int chatroom_id;
	bool deny_invitation;
} message_adaptor_end_chat_s;

void message_adaptor_destroy_chat_msg_s(message_adaptor_chat_msg_s *msg);
void message_adaptor_destroy_processed_msg_s(message_adaptor_processed_msg_s *msg);
void message_adaptor_destroy_delivery_ack_s(message_adaptor_delivery_ack_s *ack);
void message_adaptor_destroy_read_ack_s(message_adaptor_read_ack_s *ack);
void message_adaptor_destroy_ordered_chat_member_s(message_adaptor_ordered_chat_member_s *member);
void message_adaptor_destroy_inbox_message_s(message_adaptor_inbox_message_s *msg);
void message_adaptor_destroy_phone_number_s(message_adaptor_phone_number_s *num);
void message_adaptor_destroy_chat_id_s(message_adaptor_chat_id_s *id);
void message_adaptor_destroy_end_chat_s(message_adaptor_end_chat_s *msg);

/**
* @brief The handle for Message Plugin Listener
*/
typedef struct message_adaptor_plugin_listener_s *message_adaptor_plugin_listener_h;

/**
 * @ brief Message adaptor plugin handle
 */
typedef struct message_adaptor_plugin_handle_s {
	/* Mandatory functions to handle plugin in adaptor */
	/* struct message_adaptor_plugin_handle_s * (*create_plugin_handle)(void); */
	message_error_code_t (*create_context)(message_adaptor_plugin_context_h *context,
							char *duid,
							char *access_token,
							char *app_id,
							int service_id);

	message_error_code_t (*destroy_context)(message_adaptor_plugin_context_h context);

	message_error_code_t (*destroy_handle)(struct message_adaptor_plugin_handle_s *handle);
	message_error_code_t (*set_listener)(message_adaptor_plugin_listener_h listener);
	message_error_code_t (*unset_listener)(void);

	message_error_code_t (*set_server_info)(message_adaptor_plugin_context_h context,
							GHashTable *server_info,
							void *request,
							message_adaptor_error_code_h *error,
							void *response);

	message_error_code_t (*get_key)(message_adaptor_plugin_context_h handle,
							char **in_uid,
							char **in_gcmid,
							char **in_del_gcm_id,
							char **key,
							char **expiredkey,
							char **gpbauthkey,
							message_adaptor_error_code_t **error_code,
							void **server_data);

	message_error_code_t (*request_chat_id) (message_adaptor_plugin_context_h handle,
							char *uid,
							message_adaptor_phone_number_s **phone_numbers,
							unsigned int phone_numbers_len,
							void *user_data,
							message_adaptor_chat_id_s ***chat_ids,
							unsigned int *chat_ids_len,
							message_adaptor_error_code_t **error_code,
							void **server_data);

	message_error_code_t (*request_msisdn) (message_adaptor_plugin_context_h handle,
							char *uid,
							long long int *chat_ids,
							unsigned int chat_ids_len,
							void *user_data,
							message_adaptor_chat_id_s ***msisdns,
							unsigned int *msisdns_len,
							message_adaptor_error_code_t **error_code,
							void **server_data);

	message_error_code_t (*channel_auth_request)(message_adaptor_plugin_context_h context,
							long long int request_id,
							const char *uid,
							long long int duid,
							const char *appid,
							const char *access_token,
							int timeout_second,
							void *user_data,
							message_adaptor_error_code_t **error_code,
							void *server_data);

	message_error_code_t (*client_echo_reply) (message_adaptor_plugin_context_h context,
							long long int *request_id,
							message_adaptor_error_code_t **error_code,
							void *user_data);

	message_error_code_t (*create_chatroom_request)(message_adaptor_plugin_context_h context,
							long long int *request_id,
							int *chat_type,
							long long int **receivers,
							int *receivers_len,
							const char *chatroom_title,
							message_adaptor_error_code_t **error_code,
							void *user_data);

	message_error_code_t (*change_chatroom_meta_request)(message_adaptor_plugin_context_h context,
							long long int request_id,
							long long int chatroom_id,
							const char *chatroom_title,
							int default_message_ttl,
							message_adaptor_error_code_t **error_code,
							void *user_data);

	message_error_code_t (*chat_request)(message_adaptor_plugin_context_h context,\
							long long int *request_id,
							long long int *chatroom_id,
							message_adaptor_chat_msg_s *msgs,
							message_adaptor_error_code_t **error_code,
							void *user_data);

	message_error_code_t (*allow_chat_request)(message_adaptor_plugin_context_h context,
							long long int *request_id,
							long long int *chatroom_id,
							bool is_auto_allow,
							int max_count,
							bool need_delivery_ack,
							long long int delivery_ack_timestamp,
							bool need_read_ack,
							long long int last_read_ack_timestamp,
							bool need_ordered_chat_member_list,
							message_adaptor_error_code_t **error_code,
							void *user_data);

	message_error_code_t (*get_all_unread_message_request)(message_adaptor_plugin_context_h context,
							long long int *request_id,
							int *max_count,
							message_adaptor_error_code_t **error_code,
							void *user_data);


	message_error_code_t (*forward_online_message_reply)(message_adaptor_plugin_context_h context,
							long long int *request_id,
							long long int *chatroom_id,
							bool *mark_as_read,
							message_adaptor_error_code_t **error_code,
							void *user_data);

	message_error_code_t (*forward_unread_message_reply)(message_adaptor_plugin_context_h context,
							long long int *request_id,
							const char **next_pagination_key,
							int *max_count,
							message_adaptor_error_code_t **error_code,
							void *user_data);

	message_error_code_t (*read_message_request)(message_adaptor_plugin_context_h context,
							long long int *request_id,
							long long int *chatroom_id,
							message_inboxentry_t *inbox_msg,
							message_adaptor_error_code_t **error_code,
							void *user_data);

	message_error_code_t (*invite_request)(message_adaptor_plugin_context_h context,
							long long int *request_id,
							long long int *chatroom_id,
							long long int *inviting_members,
							int *inviting_members_len,
							message_adaptor_error_code_t **error_code,
							void *user_data);

	message_error_code_t (*end_chat_request)(message_adaptor_plugin_context_h context,
							long long int *request_id,
							message_adaptor_end_chat_s **end_chats,
							int *end_chats_len,
							message_adaptor_error_code_t **error_code,
							void *user_data);

	message_error_code_t (*unseal_message_request)(message_adaptor_plugin_context_h context,
							long long int *request_id,
							long long int *chatroom_id,
							long long int *sender_id,
							long long int *message_id,
							const char *message_detail,
							message_adaptor_error_code_t **error_code,
							void *user_data);

	message_error_code_t (*save_call_log_request)(message_adaptor_plugin_context_h context,
							long long int *request_id,
							long long int *chatroom_id,
							const char **call_id,
							const char **call_log_type,
							long long int *call_sender_id,
							long long int *call_receiver_id,
							int *conversaction_second,
							message_adaptor_error_code_t **error_code,
							void *user_data);

	message_error_code_t (*current_time_request)(message_adaptor_plugin_context_h context,
							long long int *request_id,
							message_adaptor_error_code_t **error_code,
							void *user_data);

	message_error_code_t (*is_typing)(message_adaptor_plugin_context_h context,
							long long int *request_id,
							long long int *chatroom_id,
							char **state,
							int *chat_type,
							int *refreshtime,
							message_adaptor_error_code_t **error_code,
							void *user_data);

	/* message_error_code_t (*message_set_key)(message_adaptor_plugin_context_h context, char *key, bool is_gpb); */
	message_error_code_t (*connect_to_server)(message_adaptor_plugin_context_h context);
	message_error_code_t (*decode_push_message)(message_adaptor_plugin_context_h context, char *in_msg, char **out_msg);
	message_error_code_t (*disconnect_to_server)(message_adaptor_plugin_context_h context);
	message_error_code_t (*get_connection_state)(message_adaptor_plugin_context_h context,
							message_connection_state_t *state);
	char *plugin_uri;	/* get from config file  */

} message_adaptor_plugin_handle_t;
typedef struct message_adaptor_plugin_handle_s *message_adaptor_plugin_handle_h;


/**
 * Callback function variable for service channel (= service adaptor)
 */

/**
* @brief Callback for ClientEcho API(sent from remote server, referenced by Service Adaptor)
* @param[in]	request_id		specifies request id for received data
* @param[in]	error_code		specifies error code
* @param[in]	server_data	specifies additional reply data from server(unused)
* @pre  ClientEcho()(requested by remote server) will invoke this callback.
* @return	void.
*/
typedef void (*message_adaptor_service_client_echo_cb)(message_adaptor_plugin_context_h context,
						long long int request_id,
						message_adaptor_error_code_t **error_code,
						void *server_data);

/**
* @brief Callback for message_adaptor_create_chatroom_request API (referenced by Service Adaptor)
* @param[in]	request_id		specifies request id for received data
* @param[in]	chatroom_id		specifies chatroom id
* @param[in]	wrong_receiver	specifies wrong receivers information
* @param[in]	error_code		specifies error code
* @param[in]	server_data		specifies additional reply data from server(unused)
* @pre  message_adaptor_create_chatroom_request() will invoke this callback.
* @return	void.
*/
typedef void (*message_adaptor_service_create_chatroom_reply_cb)(message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int chatroom_id,
						int default_message_ttl,
						message_adaptor_wrong_receiver_s *wrong_receiver,
						message_adaptor_error_code_t **error_code,
						void *server_data);

typedef void (*message_adaptor_service_change_chatroom_meta_reply_cb)(message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int chatroom_id,
						message_adaptor_error_code_t **error_code,
						void *server_data);

/**
* @brief Callback for message_adaptor_chat_request API (referenced by Service Adaptor)
* @param[in]	request_id			specifies request id for received data
* @param[in]	chatroom_id			specifies chatroom id
* @param[in]	processed_msgs		specifies processed message information
* @param[in]	processed_msgs_len	specifies the number of processed message
* @param[in]	error_code			specifies error code
* @param[in]	server_data			specifies additional reply data from server(unused)
* @pre  message_adaptor_chat_request() will invoke this callback.
* @return	void.
*/
typedef void (*message_adaptor_service_chat_reply_cb)(message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int chatroom_id,
						message_adaptor_processed_msg_s **processed_msgs,
						unsigned int processed_msgs_len,
						message_adaptor_error_code_t **error_code,
						void *server_data);

/**
* @brief Callback for message_adaptor_allow_chat_request API (referenced by Service Adaptor)
* @param[in]	request_id					specifies request id for received data
* @param[in]	chatroom_id					specifies chatroom id
* @param[in]	deliveryacks					specifies last receipt messages information for each user
* @param[in]	deliveryacks_len				specifies the number of deliveryacks
* @param[in]	read_acks					specifies last read message information for each user
* @param[in]	read_acks_len				specifies the number of wartermarks
* @param[in]	ordered_chat_members			specifies chat member list aligned to participation order
* @param[in]	ordered_chat_members_len		specifies the number of ordered_chat_members
* @param[in]	error_code					specifies error code
* @param[in]	server_data					specifies additional reply data from server(unused)
* @pre  message_adaptor_allow_chat_request() will invoke this callback.
* @return	void.
*/
typedef void (*message_adaptor_service_allow_chat_reply_cb)(message_adaptor_plugin_context_h context,
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
						void *server_data);

/**
* @brief Callback for message_adaptor_get_all_unread_message_request API (referenced by Service Adaptor)
* @param[in]	request_id	specifies request id for received data
* @param[in]	error_code	specifies error code
* @param[in]	server_data	specifies additional reply data from server(unused)
* @pre  message_adaptor_get_all_unread_message_request() will invoke this callback.
* @return	void.
*/
typedef void (*message_adaptor_service_get_all_unread_message_reply_cb)(message_adaptor_plugin_context_h context,
						long long int request_id,
						message_adaptor_error_code_t **error_code,
						void *server_data);

/**
* @brief Callback for ForwardOnlineMessage API (sent from remote server, referenced by Service Adaptor)
* @details This function is used for transferring a message from server to client requested by user
*		  who sent request messages via message_adaptor_chat_request()
* @param[in]	request_id	specifies request id for received data
* @param[in]	chatroom_id	specifies chatroom id
* @param[in]	chat_type		specifies chat room type(0: SINGLE, 1: GROUP)
* @param[in]	inbox_msg	specifies one message transferred from sender to receiver
* @param[in]	error_code	specifies error code
* @param[in]	server_data	specifies additional reply data from server(unused)
* @pre  ForwardOnlineMessage()(requested by remote server) will invoke this callback.
* @return	void.
*/
typedef void (*message_adaptor_service_forward_online_message_request_cb)(message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int chatroom_id,
						int chat_type,
						message_inboxentry_t *inbox_msg,
						bool skip_reply,
						message_adaptor_error_code_t **error_code,
						void *server_data);

/**
* @brief Callback for ForwardUnreadMessage API (sent from remote server, referenced by Service Adaptor)
* @details This function is used for transferring unread messages from server to client requested by user
*		  who sent request messages via message_adaptor_get_all_unread_message_request() or
*		  message_adaptor_allow_chat_request()
* @param[in]	request_id			specifies request id for received data
* @param[in]	inbox_msgs			specifies messages transferred from sender to receiver
* @param[in]	inbox_msg_len		specifies the number of inbox message
* @param[in]	next_pagination_key	specifies value for using pagination(NULL for unused case)
* @param[in]	error_code			specifies error code
* @param[in]	server_data			specifies additional reply data from server(unused)
* @pre  ForwardUnreadMessage()(requested by remote server) will invoke this callback.
* @return	void.
*/
typedef void (*message_adaptor_service_forward_unread_message_request_cb) (message_adaptor_plugin_context_h context,
						long long int request_id,
						message_inboxentry_t ***inbox_msgs,
						unsigned int inbox_msgs_len,
						char **next_pagination_key,
						message_adaptor_error_code_t **error_code,
						void *server_data);

/**
* @brief Callback for message_adaptor_read_message_request API (referenced by Service Adaptor)
* @param[in]	request_id	specifies request id for received data
* @param[in]	chatroom_id	specifies chatroom id
* @param[in]	error_code	specifies error code
* @param[in]	server_data	specifies additional reply data from server(unused)
* @pre  message_adaptor_read_message_request() will invoke this callback.
* @return	void.
*/
typedef void (*message_adaptor_service_read_message_reply_cb) (message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int chatroom_id,
						message_adaptor_error_code_t **error_code,
						void *server_data);

/**
* @brief Callback for message_adaptor_invite_chat_request API (referenced by Service Adaptor)
* @param[in]	request_id		specifies request id for received data
* @param[in]	chatroom_id		specifies chatroom id
* @param[in]	sent_time		specifies time when server received request message by client
* @param[in]	wrong_receiver	specifies wrong receivers information
* @param[in]	error_code		specifies error code
* @param[in]	server_data		specifies additional reply data from server(unused)
* @pre  message_adaptor_invite_chat_request() will invoke this callback.
* @return	void.
*/
typedef void (*message_adaptor_service_invite_chat_reply_cb) (message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int chatroom_id,
						long long int sent_time,
						message_adaptor_wrong_receiver_s *wrong_receiver,
						message_adaptor_error_code_t **error_code,
						void *server_data);

/**
* @brief Callback for message_adaptor_end_chat_request API (referenced by Service Adaptor)
* @param[in]	request_id		specifies request id for received data
* @param[in]	error_code		specifies error code
* @param[in]	server_data		specifies additional reply data from server(unused)
* @pre  message_adaptor_end_chat_request() will invoke this callback.
* @return	void.
*/
typedef void (*message_adaptor_service_end_chat_reply_cb)(message_adaptor_plugin_context_h context,
						long long int request_id,
						message_adaptor_error_code_t **error_code,
						void *server_data);

/**
* @brief Callback for message_adaptor_unseal_message_request API (referenced by Service Adaptor)
* @param[in]	request_id		specifies request id for received data
* @param[in]	error_code		specifies error code
* @param[in]	server_data		specifies additional reply data from server(unused)
* @pre  message_adaptor_unseal_message_request() will invoke this callback.
* @return	void.
*/
typedef void (*message_adaptor_service_unseal_message_reply_cb)(message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int chatroom_id,
						message_adaptor_error_code_t **error_code,
						void *server_data);

/**
* @brief Callback for message_adaptor_save_call_log_request API (referenced by Service Adaptor)
* @param[in]	request_id		specifies request id for received data
* @param[in]	error_code		specifies error code
* @param[in]	server_data		specifies additional reply data from server(unused)
* @pre  message_adaptor_save_call_log_request() will invoke this callback.
* @return	void.
*/
typedef void (*message_adaptor_service_save_call_log_reply_cb)(message_adaptor_plugin_context_h context,
						long long int request_id,
						message_adaptor_error_code_t **error_code,
						void *server_data);


/**
* @brief Callback for message_adaptor_current_time_request API (referenced by Service Adaptor)
* @param[in]	request_id		specifies request id for received data
* @param[in]	current_time_millis	specifies current UTC time in milliseconds
* @param[in]	error_code		specifies error code
* @param[in]	server_data		specifies additional reply data from server(unused)
* @pre  message_adaptor_current_time_request() will invoke this callback.
* @return	void.
*/
typedef void (*message_adaptor_service_current_time_reply_cb)(message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int current_time_millis,
						message_adaptor_error_code_t **error_code,
						void *server_data);

typedef void (*message_adaptor_service_typing_updated_cb)(message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int chatroom_id,
						long long int *sender,
						char ** state,
						int *contentType,
						int *refershTime,
						message_adaptor_error_code_t **error_code,
						void *server_data);

/**
* @brief Callback for notification of termination of Listener(referenced by Service Adaptor)
* @details This function called when channel is disconnected by server
* @param[in]	error_code		specifies error code
* @param[in]	server_data		specifies additional reply data from server(unused)
* @return	void.
*/
typedef void (*message_adaptor_service_completion_cb)(message_adaptor_plugin_context_h context,
						message_connection_state_t state,
						message_adaptor_error_code_t **error_code,
						void *server_data);



typedef struct message_adaptor_listener_s {
	void (*client_echo_cb)(message_adaptor_plugin_context_h context,
							long long int request_id,
							message_adaptor_error_code_t **error_code,
							void *server_data);

	void (*create_chatroom_reply_cb)(message_adaptor_plugin_context_h context,
							long long int request_id,
							long long int chatroom_id,
							int default_message_ttl,
							message_adaptor_wrong_receiver_s *wrong_receiver,
							message_adaptor_error_code_t **error_code,
							void *server_data);

	void (*change_chatroom_meta_reply_cb)(message_adaptor_plugin_context_h context,
							long long int request_id,
							long long int chatroom_id,
							message_adaptor_error_code_t **error_code,
							void *server_data);

	void (*chat_reply_cb)(message_adaptor_plugin_context_h context,
							long long int request_id,
							long long int chatroom_id,
							message_adaptor_processed_msg_s **processed_msgs,
							unsigned int processed_msgs_len,
							message_adaptor_error_code_t **error_code,
							void *server_data);

	void (*allow_chat_reply_cb) (message_adaptor_plugin_context_h context,
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
							void *server_data);


	void (*get_all_unread_message_reply_cb) (message_adaptor_plugin_context_h context,
							long long int request_id,
							message_adaptor_error_code_t **error_code,
							void *server_data);

	void (*forward_online_message_request_cb)(message_adaptor_plugin_context_h context,
							long long int request_id,
							long long int chatroom_id,
							int chat_type,
							message_inboxentry_t *inbox_msg,
							bool skip_reply,
							message_adaptor_error_code_t **error_code,
							void *server_data);


	void (*forward_unread_message_request_cb)(message_adaptor_plugin_context_h context,
							long long int request_id,
							message_inboxentry_t ***inbox_msgs,
							unsigned int inbox_msgs_len,
							char **next_pagination_key,
							message_adaptor_error_code_t **error_code,
							void *server_data);

	void (*read_message_reply_cb) (message_adaptor_plugin_context_h context,
							long long int request_id,
							long long int chatroom_id,
							message_adaptor_error_code_t **error_code,
							void *server_data);

	void (*invite_chat_reply_cb) (message_adaptor_plugin_context_h context,
							long long int request_id,
							long long int chatroom_id,
							long long int sent_time,
							message_adaptor_wrong_receiver_s *wrong_receiver,
							message_adaptor_error_code_t **error_code,
							void *server_data);

	void (*end_chat_reply_cb) (message_adaptor_plugin_context_h context,
							long long int request_id,
							message_adaptor_error_code_t **error_code,
							void *server_data);

	void (*unseal_message_reply_cb) (message_adaptor_plugin_context_h context,
							long long int request_id,
							long long int chatroom_id,
							message_adaptor_error_code_t **error_code,
							void *server_data);

	void (*save_call_log_reply_cb) (message_adaptor_plugin_context_h context,
							long long int request_id,
							message_adaptor_error_code_t **error_code,
							void *server_data);

	void (*current_time_reply_cb)(message_adaptor_plugin_context_h context,
							long long int request_id,
							long long int current_time_millis,
							message_adaptor_error_code_t **error_code,
							void *server_data);

	void (*typing_updated_cb)(message_adaptor_plugin_context_h context,
							long long int request_id,
							long long int chatroom_id,
							long long int *sender,
							char ** state,
							int *contentType,
							int *refershTime,
							message_adaptor_error_code_t **error_code,
							void *server_data);

	void (*completion_cb)(message_adaptor_plugin_context_h context,
							message_connection_state_t state,
							message_adaptor_error_code_t **error_code,
							void *server_data);


} message_adaptor_listener_t;
typedef struct message_adaptor_listener_s *message_adaptor_listener_h;


/**
 * Message adaptor listener for plugins
 * Listener is used by plugins
 */


/**
* @brief Callback for ClientEcho API(sent from remote server)
* @param[in]	request_id		specifies request id for received data
* @param[in]	error_code		specifies error code
* @param[in]	server_data	specifies additional reply data from server(unused)
* @pre  ClientEcho()(requested by remote server) will invoke this callback.
* @return	void.
*/
typedef void (*message_adaptor_plugin_client_echo_cb)(message_adaptor_plugin_context_h context,
						long long int request_id,
						message_adaptor_error_code_t **error_code,
						void *server_data);

/**
* @brief Callback for message_adaptor_plugin_handle->create_chatroom_request API (referenced by Service Adaptor)
* @param[in]	request_id		specifies request id for received data
* @param[in]	chatroom_id		specifies chatroom id
* @param[in]	wrong_receiver	specifies wrong receivers information
* @param[in]	error_code		specifies error code
* @param[in]	server_data		specifies additional reply data from server(unused)
* @pre  message_adaptor_plugin_handle->create_chatroom_request() will invoke this callback.
* @return	void.
*/
typedef void (*message_adaptor_plugin_create_chatroom_reply_cb)(message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int chatroom_id,
						int default_message_ttl,
						message_adaptor_wrong_receiver_s *wrong_receiver,
						message_adaptor_error_code_t **error_code,
						void *server_data);

typedef void (*message_adaptor_plugin_change_chatroom_meta_reply_cb)(message_adaptor_plugin_context_h context,
							long long int request_id,
							long long int chatroom_id,
							message_adaptor_error_code_t **error_code,
							void *server_data);
/**
* @brief Callback for message_adaptor_plugin_handle->chat_request API (referenced by Service Adaptor)
* @param[in]	request_id			specifies request id for received data
* @param[in]	chatroom_id			specifies chatroom id
* @param[in]	processed_msgs		specifies processed message information
* @param[in]	processed_msgs_len	specifies the number of processed message
* @param[in]	error_code			specifies error code
* @param[in]	server_data			specifies additional reply data from server(unused)
* @pre  message_adaptor_plugin_handle->chat_request() will invoke this callback.
* @return	void.
*/
typedef void (*message_adaptor_plugin_chat_reply_cb)(message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int chatroom_id,
						message_adaptor_processed_msg_s **processed_msgs,
						unsigned int processed_msgs_len,
						message_adaptor_error_code_t **error_code,
						void *server_data);

/**
* @brief Callback for message_adaptor_plugin_handle->allow_chat_request API (referenced by Service Adaptor)
* @param[in]	request_id					specifies request id for received data
* @param[in]	chatroom_id					specifies chatroom id
* @param[in]	deliveryacks					specifies last receipt messages information for each user
* @param[in]	deliveryacks_len				specifies the number of deliveryacks
* @param[in]	read_acks					specifies last read message information for each user
* @param[in]	read_acks_len				specifies the number of wartermarks
* @param[in]	ordered_chat_members			specifies chat member list aligned to participation order
* @param[in]	ordered_chat_members_len		specifies the number of ordered_chat_members
* @param[in]	error_code					specifies error code
* @param[in]	server_data					specifies additional reply data from server(unused)
* @pre  message_adaptor_plugin_handle->allow_chat_request() will invoke this callback.
* @return	void.
*/
typedef void (*message_adaptor_plugin_allow_chat_reply_cb)(message_adaptor_plugin_context_h context,
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
						void *server_data);

/**
* @brief Callback for message_adaptor_plugin_handle->get_all_unread_message_request API (referenced by Service Adaptor)
* @param[in]	request_id	specifies request id for received data
* @param[in]	error_code	specifies error code
* @param[in]	server_data	specifies additional reply data from server(unused)
* @pre  message_adaptor_plugin_handle->get_all_unread_message_request() will invoke this callback.
* @return	void.
*/
typedef void (*message_adaptor_plugin_get_all_unread_message_reply_cb)(message_adaptor_plugin_context_h context,
						long long int request_id,
						message_adaptor_error_code_t **error_code,
						void *server_data);

/**
* @brief Callback for ForwardOnlineMessage API (sent from remote server, referenced by Service Adaptor)
* @details This function is used for transferring a message from server to client requested by user
*		  who sent request messages via message_adaptor_chat_request()
* @param[in]	request_id	specifies request id for received data
* @param[in]	chatroom_id	specifies chatroom id
* @param[in]	chat_type		specifies chat room type(0: SINGLE, 1: GROUP)
* @param[in]	inbox_msg	specifies one message transferred from sender to receiver
* @param[in]	error_code	specifies error code
* @param[in]	server_data	specifies additional reply data from server(unused)
* @pre  ForwardOnlineMessage()(requested by remote server) will invoke this callback.
* @return	void.
*/
typedef void (*message_adaptor_plugin_forward_online_message_request_cb)(message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int chatroom_id,
						int chat_type,
						message_inboxentry_t *inbox_msg,
						bool skip_reply,
						message_adaptor_error_code_t **error_code,
						void *server_data);

/**
* @brief Callback for ForwardUnreadMessage API (sent from remote server, referenced by Service Adaptor)
* @details This function is used for transferring unread messages from server to client requested by user
*		  who sent request messages via message_adaptor_get_all_unread_message_request() or
*		  message_adaptor_allow_chat_request()
* @param[in]	request_id			specifies request id for received data
* @param[in]	inbox_msgs			specifies messages transferred from sender to receiver
* @param[in]	inbox_msg_len		specifies the number of inbox message
* @param[in]	next_pagination_key	specifies value for using pagination(NULL for unused case)
* @param[in]	error_code			specifies error code
* @param[in]	server_data			specifies additional reply data from server(unused)
* @pre  ForwardUnreadMessage()(requested by remote server) will invoke this callback.
* @return	void.
*/
typedef void (*message_adaptor_plugin_forward_unread_message_request_cb)(message_adaptor_plugin_context_h context,
						long long int request_id,
						message_inboxentry_t ***inbox_msgs,
						unsigned int inbox_msgs_len,
						char **next_pagination_key,
						message_adaptor_error_code_t **error_code,
						void *server_data);

/**
* @brief Callback for message_adaptor_plugin_handle->read_message_request API (referenced by Service Adaptor)
* @param[in]	request_id	specifies request id for received data
* @param[in]	chatroom_id	specifies chatroom id
* @param[in]	error_code	specifies error code
* @param[in]	server_data	specifies additional reply data from server(unused)
* @pre  message_adaptor_plugin_handle->read_message_request() will invoke this callback.
* @return	void.
*/
typedef void (*message_adaptor_plugin_read_message_reply_cb)(message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int chatroom_id,
						message_adaptor_error_code_t **error_code,
						void *server_data);

/**
* @brief Callback for message_adaptor_plugin_handle->invite_chat_request API (referenced by Service Adaptor)
* @param[in]	request_id		specifies request id for received data
* @param[in]	chatroom_id		specifies chatroom id
* @param[in]	sent_time		specifies time when server received request message by client
* @param[in]	wrong_receiver	specifies wrong receivers information
* @param[in]	error_code		specifies error code
* @param[in]	server_data		specifies additional reply data from server(unused)
* @pre  message_adaptor_plugin_handle->invite_chat_request() will invoke this callback.
* @return	void.
*/
typedef void (*message_adaptor_plugin_invite_chat_reply_cb)(message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int chatroom_id,
						long long int sent_time,
						message_adaptor_wrong_receiver_s *wrong_receiver,
						message_adaptor_error_code_t **error_code,
						void *server_data);

/**
* @brief Callback for message_adaptor_plugin_handle->end_chat_request API (referenced by Service Adaptor)
* @param[in]	request_id		specifies request id for received data
* @param[in]	error_code		specifies error code
* @param[in]	server_data		specifies additional reply data from server(unused)
* @pre  message_adaptor_plugin_handle->end_chat_request() will invoke this callback.
* @return	void.
*/
typedef void (*message_adaptor_plugin_end_chat_reply_cb)(message_adaptor_plugin_context_h context,
						long long int request_id,
						message_adaptor_error_code_t **error_code,
						void *server_data);

/**
* @brief Callback for message_adaptor_plugin_handle->unseal_message_request API (referenced by Service Adaptor)
* @param[in]	request_id		specifies request id for received data
* @param[in]	chatroom_id		specifies chatroom id
* @param[in]	error_code		specifies error code
* @param[in]	server_data		specifies additional reply data from server(unused)
* @pre  message_adaptor_plugin_handle->end_chat_request() will invoke this callback.
* @return	void.
*/
typedef void (*message_adaptor_plugin_unseal_message_reply_cb)(message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int chatroom_id,
						message_adaptor_error_code_t **error_code,
						void *server_data);

/**
* @brief Callback for message_adaptor_plugin_handle->save_call_log_request API (referenced by Service Adaptor)
* @param[in]	request_id		specifies request id for received data
* @param[in]	error_code		specifies error code
* @param[in]	server_data		specifies additional reply data from server(unused)
* @pre  message_adaptor_plugin_handle->save_call_log_request() will invoke this callback.
* @return	void.
*/
typedef void (*message_adaptor_plugin_save_call_log_reply_cb)(message_adaptor_plugin_context_h context,
						long long int request_id,
						message_adaptor_error_code_t **error_code,
						void *server_data);


/**
* @brief Callback for  message_adaptor_plugin_handle->current_time_request API (referenced by Service Adaptor)
* @param[in]	request_id		specifies request id for received data
* @param[in]	current_time_millis	specifies current UTC time in milliseconds
* @param[in]	error_code		specifies error code
* @param[in]	server_data		specifies additional reply data from server(unused)
* @pre   message_adaptor_plugin_handle->current_time_request() will invoke this callback.
* @return	void.
*/
typedef void (*message_adaptor_plugin_current_time_reply_cb)(message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int current_time_millis,
						message_adaptor_error_code_t **error_code,
						void *server_data);

typedef void (*message_adaptor_plugin_typing_updated_cb)(message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int chatroom_id,
						long long int *sender,
						char ** state,
						int *contentType,
						int *refershTime,
						message_adaptor_error_code_t **error_code,
						void *server_data);

/**
* @brief Callback for notification of termination of Listener(referenced by Service Adaptor)
* @details This function called when channel is disconnected by server
* @param[in]	error_code		specifies error code
* @param[in]	server_data		specifies additional reply data from server(unused)
* @return	void.
*/
typedef void (*message_adaptor_plugin_completion_cb)(message_adaptor_plugin_context_h context,
						message_connection_state_t state,
						message_adaptor_error_code_t **error_code,
						void *server_data);

/**
 * Message adaptor listener for plugins
 * Listener is used by plugins
 */
typedef struct message_adaptor_plugin_listener_s {
	message_adaptor_plugin_client_echo_cb message_adaptor_client_echo;
	message_adaptor_plugin_create_chatroom_reply_cb message_adaptor_create_chatroom_reply;
	message_adaptor_plugin_change_chatroom_meta_reply_cb message_adaptor_change_chatroom_meta_reply;
	message_adaptor_plugin_chat_reply_cb message_adaptor_chat_reply;
	message_adaptor_plugin_allow_chat_reply_cb message_adaptor_allow_chat_reply;
	message_adaptor_plugin_get_all_unread_message_reply_cb message_adaptor_get_all_unread_message_reply;
	message_adaptor_plugin_forward_online_message_request_cb message_adaptor_forward_online_message_request;
	message_adaptor_plugin_forward_unread_message_request_cb message_adaptor_forward_unread_message_request;
	message_adaptor_plugin_read_message_reply_cb message_adaptor_read_message_reply;
	message_adaptor_plugin_invite_chat_reply_cb message_adaptor_invite_chat_reply;
	message_adaptor_plugin_end_chat_reply_cb message_adaptor_end_chat_reply;
	message_adaptor_plugin_unseal_message_reply_cb message_adaptor_unseal_message_reply;
	message_adaptor_plugin_save_call_log_reply_cb message_adaptor_save_call_log_reply;
	message_adaptor_plugin_current_time_reply_cb message_adaptor_current_time_reply;
	message_adaptor_plugin_typing_updated_cb message_adaptor_typing_updated;
	message_adaptor_plugin_completion_cb message_adaptor_completion;

} message_adaptor_plugin_listener_t;

/**
 * Loads plugin from selected path
 */
EXPORT_API
int message_adaptor_load_plugin(message_adaptor_h adaptor, const char *plugin_path);

/**
 * Unloads selected plugin
 */
EXPORT_API
int message_adaptor_unload_plugin(message_adaptor_h adaptor, message_adaptor_plugin_h plugin);

EXPORT_API
message_error_code_t message_adaptor_set_connected(message_adaptor_plugin_h plugin, int connected);

EXPORT_API
message_error_code_t message_adaptor_wait_connected(message_adaptor_plugin_h plugin);

/**
 * Gets plugin name
 */
EXPORT_API
const char *message_adaptor_get_plugin_uri(message_adaptor_plugin_h plugin);

/**
 * Refresh access token
 */
EXPORT_API
message_error_code_t message_adaptor_refresh_access_token(message_adaptor_plugin_context_h context,
						const char *new_access_token);

/**
 * Refresh uid
 */
EXPORT_API
message_error_code_t message_adaptor_refresh_uid(message_adaptor_plugin_context_h context,
						const char *new_uid);

/**
 * Create error code
 */
EXPORT_API
message_adaptor_error_code_h message_adaptor_create_error_code(const char *code, const char *msg);

/**
 * Destroy error code
 */
EXPORT_API
void message_adaptor_destroy_error_code(message_adaptor_error_code_h *error_code);

/**
 * Creates message adaptor
 */
EXPORT_API
message_adaptor_h message_adaptor_create(const char *plugins_dir);

/**
 * Destroys message adaptor
 * Destroys message adaptor. If message adaptor was started it is stopped first.
 */
EXPORT_API
void message_adaptor_destroy(message_adaptor_h adaptor);

/**
 * Starts message adaptor
 * Starts message adaptor and loads plugins that were found in plugins search dir
 * specified in message_adaptor_create
 */
EXPORT_API
int message_adaptor_start(message_adaptor_h adaptor);

/**
 * Stops message adaptor.
 */
EXPORT_API
int message_adaptor_stop(message_adaptor_h adaptor);

/**
 * Registers plugin state listener
 */
EXPORT_API
int message_adaptor_register_listener(message_adaptor_h adaptor, message_adaptor_listener_h listener);

/**
 * Unregisters plugin state listener
 */
EXPORT_API
int message_adaptor_unregister_listener(message_adaptor_h adaptor, message_adaptor_listener_h listener);

/**
 * Creates plugin context.
 */
EXPORT_API
message_adaptor_plugin_context_h message_adaptor_create_plugin_context(message_adaptor_plugin_h plugin,
						char *plugin_uri,
						char *duid,
						char *access_token,
						char *app_id,
						int service_id);

/**
 * Destroys plugin context.
 */
EXPORT_API
void message_adaptor_destroy_plugin_context(message_adaptor_plugin_h plugin, message_adaptor_plugin_context_h plugin_context);

/**
 * Gets plugin with specified unique name
 */
EXPORT_API
message_adaptor_plugin_h message_adaptor_get_plugin_by_name(message_adaptor_h adaptor, const char *plugin_uri);

/**********************************************************/
/* Adaptor Plugin call Functions                          */
/**********************************************************/

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
						message_adaptor_error_code_h *error,
						void *response);
/**
* @brief Request encryption/decryption key from remote server
* @param[in]	plugin		specifies Message-adaptor Plugin handle
* @param[in]	context		specifies Message-adaptor Plugin context
* @param[in]	in_gcmid		specifies GCM Registration ID for use of Google Cloud Messaging
* @param[in]	in_del_gcm_id	specifies Paramter for deletion of gcmid in db
* @param[out]	key			specifies user key for en/decryption of messages
* @param[out]	expiredkey	specifies expired key
* @param[out]	gpbauthkey	specifies system key for en/decryption of ChannelAuth Request/Reply messages
* @param[out]	error_code	specifies error code
* @param[out]	server_data	specifies additional reply data from server(JSON format)
* @return 0 on success, otherwise a positive error value
* @retval error code defined in message_error_code_e - MESSAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
message_error_code_t message_adaptor_get_key(message_adaptor_plugin_h plugin,
						message_adaptor_plugin_context_h context,
						char **in_gcmid,
						char **in_del_gcm_id,
						char **key,
						char **expiredkey,
						char **gpbauthkey,
						message_adaptor_error_code_t **error_code,
						void **server_data);

/**
* @brief Request Chat ID corresponds to MSISDN to remote server
* @param[in]	plugin				specifies Message-adaptor Plugin handle
* @param[in]	context				specifies Message-adaptor Plugin context
* @param[in]	phone_numbers		specifies List of phone numbers to request chat id
* @param[in]	phone_numbers_len	specifies the number of phone numbers
* @param[in]	user_data			specifies additional input data(JSON format, unused)
* @param[out]	chat_ids				specifies chat ids correspond to requested phone numbers
* @param[out]	chat_ids_len			specifies the number of chat ids
* @param[out]	error_code			specifies error code
* @param[out]	server_data			specifies additional reply data from server(JSON format)
* @return 0 on success, otherwise a positive error value
* @retval error code defined in message_error_code_e - MESSAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
message_error_code_t message_adaptor_request_chat_id(message_adaptor_plugin_h plugin,
						message_adaptor_plugin_context_h handle,
						message_adaptor_phone_number_s **phone_numbers,
						unsigned int phone_numbers_len,
						void *user_data,
						message_adaptor_chat_id_s ***chat_ids,
						unsigned int *chat_ids_len,
						message_adaptor_error_code_t **error_code,
						void **server_data);

/**
* @brief Request MSISDN corresponds to CHAT ID to remote server
* @param[in]	plugin		specifies Message-adaptor Plugin handle
* @param[in]	context		specifies Message-adaptor Plugin context
* @param[in]	chat_ids		specifies chat ids correspond to requested phone numbers
* @param[in]	chat_ids_len	specifies the number of chat ids
* @param[in]	user_data	specifies additional input data(JSON format, unused)
* @param[out]	msisdn		specifies List of phone numbers to request chat id
* @param[out]	msisdn_len	specifies the number of phone numbers
* @param[out]	error_code	specifies error code
* @param[out]	server_data	specifies additional reply data from server(JSON format)
* @return 0 on success, otherwise a positive error value
* @retval error code defined in message_error_code_e - MESSAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
message_error_code_t message_adaptor_request_msisdn(message_adaptor_plugin_h plugin,
						message_adaptor_plugin_context_h handle,
						long long int *chat_ids,
						unsigned int chat_ids_len,
						void *user_data,
						message_adaptor_chat_id_s ***msisdns,
						unsigned int *msisdns_len,
						message_adaptor_error_code_t **error_code,
						void **server_data);


/**
* @brief Channel Authorization Request.
*
* @param[in]	plugin		specifies Message-adaptor Plugin handle
* @param[in]	context		specifies Message-adaptor Plugin context
* @param[in]	request_id	specifies unique request ID
* @param[in]	timeout_second	specifies timeout second for wating this function (default = 0 : as long as possible be allowed by plugin)
* @param[in]	user_data	specifies additional user input data (unused)
* @param[out]	error_code	specifies error code
* @param[out]	server_data	specifies additional server output data (unused)
* @return MESSAGE_ADAPTOR_ERROR_NONE (0) on success, otherwise a positive error value
* @retval error code defined in message_error_code_t
*/
EXPORT_API
message_error_code_t message_adaptor_channel_auth_request(message_adaptor_plugin_h plugin,
						message_adaptor_plugin_context_h context,
						long long int request_id,
						const int timeout_second,
						void *user_data,
						message_adaptor_error_code_t **error_code,
						void *server_data);

/**
* @brief Client Echo Reply.(Client -> Server)
*
* @param[in]	plugin		specifies Message-adaptor Plugin handle
* @param[in]	context		specifies Message-adaptor Plugin context
* @param[in]	request_id	specifies unique request ID
* @param[in]	error_code	specifies error code
* @param[in]	server_data	specifies server data to be sent(not used)
* @return MESSAGE_ADAPTOR_ERROR_NONE (0) on success, otherwise a positive error value
* @retval error code defined in message_error_code_t
*/
EXPORT_API
message_error_code_t message_adaptor_client_echo_reply(message_adaptor_plugin_h plugin,
						message_adaptor_plugin_context_h context,
						long long int request_id,
						message_adaptor_error_code_t **error_code,
						void *server_data);

/**
* @brief Create Chatroom Request.
*
* @param[in]	plugin		specifies Message-adaptor Plugin handle
* @param[in]	context		specifies Message-adaptor Plugin context
* @param[in]	request_id	specifies unique request ID
* @param[in]	chattype		specifies chat type (0 - SINGLE, 1 - GROUP)
* @param[in]	receivers		specifies the receivers devices ID
* @param[in]	receivers_len	specifies the number of receivers
* @param[in]	user_data	specifies additional user input data (unused)
* @param[out]	error_code	specifies error code
* @return MESSAGE_ADAPTOR_ERROR_NONE (0) on success, otherwise a positive error value
* @retval error code defined in message_error_code_t
*/
EXPORT_API
message_error_code_t message_adaptor_create_chatroom_request(message_adaptor_plugin_h plugin,
						message_adaptor_plugin_context_h context,
						long long int request_id,
						int chat_type,
						long long int **receivers,
						unsigned int receivers_len,
						const char *chatroom_title,
						message_adaptor_error_code_t **error_code,
						void *user_data);

/**
* @brief Change Chatroom meta Request.
*
* @param[in]	plugin		specifies Message-adaptor Plugin handle
* @param[in]	context		specifies Message-adaptor Plugin context
* @param[in]	request_id	specifies unique request ID
* @param[in]	chattype		specifies chat type (0 - SINGLE, 1 - GROUP)
* @param[in]	receivers		specifies the receivers devices ID
* @param[in]	receivers_len	specifies the number of receivers
* @param[in]	user_data	specifies additional user input data (unused)
* @param[out]	error_code	specifies error code
* @return MESSAGE_ADAPTOR_ERROR_NONE (0) on success, otherwise a positive error value
* @retval error code defined in message_error_code_t
*/
EXPORT_API
message_error_code_t message_adaptor_change_chatroom_meta_request(message_adaptor_plugin_h plugin,
						message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int chatroom_id,
						const char *chatroom_title,
						int default_message_ttl,
						message_adaptor_error_code_t **error_code,
						void *user_data);
/**
* @brief Chat Request.
*
* @param[in]	plugin			specifies Message-adaptor Plugin handle
* @param[in]	context			specifies Message-adaptor Plugin context
* @param[in]	request_id		specifies unique request ID
* @param[in]	chatroom_id		specifies chatroom ID
* @param[in]	chat_msgs		specifies message data
* @param[in]	chat_msgs_len	specifies the number of message data
* @param[in]	user_data		specifies additional user input data (unused)
* @param[out]	error_code		specifies error code
* @return MESSAGE_ADAPTOR_ERROR_NONE (0) on success, otherwise a positive error value
* @retval error code defined in message_error_code_t
*/
EXPORT_API
message_error_code_t message_adaptor_chat_request(message_adaptor_plugin_h plugin,
						message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int chatroom_id,
						message_adaptor_chat_msg_s **chat_msgs,
						unsigned int chat_msgs_len,
						message_adaptor_error_code_t **error_code,
						void *user_data);

/**
* @brief Allow Chat Request.
*
* @param[in]	plugin						specifies Message-adaptor Plugin handle
* @param[in]	context						specifies Message-adaptor Plugin context
* @param[in]	request_id					specifies unique request ID
* @param[in]	chatroom_id					specifies chatroom ID
* @param[in]	max_count					specifies the max number of ForwardUnreadMessage (default : 500)
* @param[in]	need_delivery_ack				specifies whether receive delivery ack data or not (0 - do not receive delivery ack data, 1 - receive delivery ack data)
* @param[in]	need_delivery_ack_timestamp			specifies latest receipt time of delivery ack data
* @param[in]	need_read_ack					specifies whether receive water mark data or not (0 - do not receive water mark data, 1 - receive read_ack data)
* @param[in]	last_read_ack_timestamp				specifies time of last read_ack
* @param[in]	need_ordered_chat_member_list			specifies whether need odered chat memebr list or not
* @param[in]	user_data					specifies additional user input data (unused)
* @param[out]	error_code					specifies error code
* @return MESSAGE_ADAPTOR_ERROR_NONE (0) on success, otherwise a positive error value
* @retval error code defined in message_error_code_t
*/
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
						void *user_data);

/**
* @brief Get All Unread Message Request.
*
* @param[in]	plugin		specifies Message-adaptor Plugin handle
* @param[in]	context		specifies Message-adaptor Plugin context
* @param[in]	request_id	specifies unique request ID
* @param[in]	max_count	specifies the max number of ForwardUnreadMessage (default : 500)
* @param[in]	user_data	specifies additional user input data (unused)
* @param[out]	error_code	specifies error code
* @return MESSAGE_ADAPTOR_ERROR_NONE (0) on success, otherwise a positive error value
* @retval error code defined in message_error_code_t
*/
EXPORT_API
message_error_code_t message_adaptor_get_all_unread_message_request(message_adaptor_plugin_h plugin,
						message_adaptor_plugin_context_h context,
						long long int request_id,
						int max_count,
						message_adaptor_error_code_t **error_code,
						void *user_data);

/**
* @brief Forward Online Message Reply.
*
* @param[in]	plugin		specifies Message-adaptor Plugin handle
* @param[in]	context		specifies Message-adaptor Plugin context
* @param[in]	request_id	specifies unique request ID
* @param[in]	chatroom_id	specifies chatroom ID
* @param[in]	mark_as_read	marks the received message as read
* @param[in]	user_data	specifies additional user input data (unused)
* @param[out]	error_code	specifies error code
* @return MESSAGE_ADAPTOR_ERROR_NONE (0) on success, otherwise a positive error value
* @retval error code defined in message_error_code_t
*/
EXPORT_API
message_error_code_t message_adaptor_forward_online_message_reply(message_adaptor_plugin_h plugin,
						message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int chatroom_id,
						bool mark_as_read,
						message_adaptor_error_code_t **error_code,
						void *user_data);

/**
* @brief Forward Unread Message Reply.
*
* @param[in]	plugin				specifies Message-adaptor Plugin handle
* @param[in]	context				specifies Message-adaptor Plugin context
* @param[in]	request_id			specifies unique request ID
* @param[in]	next_pagination_key	specifies the next pagination key
* @param[in]	max_count			specifies the max number of unread messages to be shown in one page(default : 500)
* @param[in]	user_data			specifies additional user input data (unused)
* @param[out]	error_code			specifies error code
* @return MESSAGE_ADAPTOR_ERROR_NONE (0) on success, otherwise a positive error value
* @retval error code defined in message_error_code_t
*/
EXPORT_API
message_error_code_t message_adaptor_forward_unread_message_reply(message_adaptor_plugin_h plugin,
						message_adaptor_plugin_context_h context,
						long long int request_id,
						const char *next_pagination_key,
						int max_count,
						message_adaptor_error_code_t **error_code,
						void *user_data);

/**
* @brief Read Message Request.
*
* @param[in]	plugin		specifies Message-adaptor Plugin handle
* @param[in]	context		specifies Message-adaptor Plugin context
* @param[in]	request_id	specifies unique request ID
* @param[in]	chatroom_id	specifies chatroom ID
* @param[in]	inbox_msg	specifies the message to be read
* @param[in]	user_data	specifies additional user input data (unused)
* @param[out]	error_code	specifies error code
* @return MESSAGE_ADAPTOR_ERROR_NONE (0) on success, otherwise a positive error value
* @retval error code defined in message_error_code_t
*/
EXPORT_API
message_error_code_t message_adaptor_read_message_request(message_adaptor_plugin_h plugin,
						message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int chatroom_id,
						message_inboxentry_t *inbox_msg,
						message_adaptor_error_code_t **error_code,
						void *user_data);

/**
* @brief Invite Request.
*
* @param[in]	plugin				specifies Message-adaptor Plugin handle
* @param[in]	context				specifies Message-adaptor Plugin context
* @param[in]	request_id			specifies unique request ID
* @param[in]	chatroom_id			specifies chatroom ID
* @param[in]	inviting_members		specifies members IDs to be invited to chatroom
* @param[in]	inviting_members_len	specifies the number of members IDs
* @param[in]	user_data			specifies additional user input data (unused)
* @param[out]	error_code			specifies error code
* @return MESSAGE_ADAPTOR_ERROR_NONE (0) on success, otherwise a positive error value
* @retval error code defined in message_error_code_t
*/
EXPORT_API
message_error_code_t message_adaptor_invite_chat_request(message_adaptor_plugin_h plugin,
						message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int chatroom_id,
						long long int *inviting_members,
						unsigned int inviting_members_len,
						message_adaptor_error_code_t **error_code,
						void *user_data);

/**
* @brief End Chat Request.
*
* @param[in]	plugin			specifies Message-adaptor Plugin handle
* @param[in]	context			specifies Message-adaptor Plugin context
* @param[in]	request_id		specifies unique request ID
* @param[in]	end_chats		specifies chatrooms IDs
* @param[in]	end_chats_len		specifies the number of chatrooms
* @param[in]	user_data			specifies additional user input data (unused)
* @param[out]	error_code			specifies error code
* @return MESSAGE_ADAPTOR_ERROR_NONE (0) on success, otherwise a positive error value
* @retval error code defined in message_error_code_t
*/
EXPORT_API
message_error_code_t message_adaptor_end_chat_request(message_adaptor_plugin_h plugin,
						message_adaptor_plugin_context_h context,
						long long int request_id,
						message_adaptor_end_chat_s **end_chats,
						unsigned int end_chats_len,
						message_adaptor_error_code_t **error_code,
						void *user_data);

/**
* @brief Unseal Message Request.
*
* @param[in]	plugin				specifies Message-adaptor Plugin handle
* @param[in]	context				specifies Message-adaptor Plugin context
* @param[in]	request_id			specifies unique request ID
* @param[in]	chatroom_id			specifies chatroom ID
* @param[in]	sender_id			specifies message sender ID for 'Unseal Message'
* @param[in]	message_id			specifies message ID for 'Unseal Message'
* @param[in]	user_data			specifies additional user input data (unused)
* @param[out]	error_code			specifies error code
* @return MESSAGE_ADAPTOR_ERROR_NONE (0) on success, otherwise a positive error value
* @retval error code defined in message_error_code_t
*/
EXPORT_API
message_error_code_t message_adaptor_unseal_message_request(message_adaptor_plugin_h plugin,
						message_adaptor_plugin_context_h context,
						long long int request_id,
						long long int chatroom_id,
						long long int sender_id,
						long long int message_id,
						const char *message_detail,
						message_adaptor_error_code_t **error_code,
						void *user_data);

/**
* @brief Save Call Log Request.
*
* @param[in]	plugin				specifies Message-adaptor Plugin handle
* @param[in]	context				specifies Message-adaptor Plugin context
* @param[in]	request_id			specifies unique request ID
* @param[in]	chatroom_id			specifies chatroom ID
* @param[in]	call_id				specifies unique call ID
* @param[in]	call_log_type			specifies call log type string
* @param[in]	call_sender_id			specifies caller's unique ID
* @param[in]	call_receiver_id		specifies callee's unique ID
* @param[in]	conversaction_second		specifies calling time
* @param[in]	user_data			specifies additional user input data (unused)
* @param[out]	error_code			specifies error code
* @return MESSAGE_ADAPTOR_ERROR_NONE (0) on success, otherwise a positive error value
* @retval error code defined in message_error_code_t
*/
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
						void *user_data);


/**
* @brief Current Time Request.
*
* @param[in]	plugin			specifies Message-adaptor Plugin handle
* @param[in]	context			specifies Message-adaptor Plugin context
* @param[in]	request_id		specifies unique request ID
* @param[in]	user_data		specifies additional user input data (unused)
* @param[out]	error_code		specifies error code
* @return MESSAGE_ADAPTOR_ERROR_NONE (0) on success, otherwise a positive error value
* @retval error code defined in message_error_code_t
*/
EXPORT_API
message_error_code_t message_adaptor_current_time_request(message_adaptor_plugin_h plugin,
						message_adaptor_plugin_context_h context,
						long long int request_id,
						message_adaptor_error_code_t **error_code,
						void *user_data);
EXPORT_API
message_error_code_t message_adaptor_is_typing(message_adaptor_plugin_h plugin,
						message_adaptor_plugin_context_h context,
						long long int *request_id,
						long long int *chatroom_id,
						char **state,
						int *chat_type,
						int *refreshtime,
						message_adaptor_error_code_t **error_code,
						void *user_data);
/*
EXPORT_API
message_error_code_t message_adaptor_plugin_set_key(message_adaptor_plugin_h plugin,
												message_adaptor_plugin_context_h context,
												char *key_str,
												bool is_gpb);
*/

/**
* @brief Establish a TCP server connection
* @details create socket and establish connection, then start recv listener
* @param[in]	plugin			specifies Message-adaptor Plugin handle
* @param[in]	context			specifies Message-adaptor Plugin context
* @param[out]	error_code		specifies error code
* @return MESSAGE_ADAPTOR_ERROR_NONE (0) on success, otherwise a positive error value
* @retval error code defined in message_error_code_t
*/
EXPORT_API
message_error_code_t message_adaptor_connect(message_adaptor_plugin_h plugin, message_adaptor_plugin_context_h context, message_adaptor_error_code_h *error_code);



/**
* @brief Disestablish a TCP server connection
* @details destroy socket and disestablish connection, then stop recv listener
* @param[in]	plugin			specifies Message-adaptor Plugin handle
* @param[in]	context			specifies Message-adaptor Plugin context
* @param[out]	error_code		specifies error code
* @return MESSAGE_ADAPTOR_ERROR_NONE (0) on success, otherwise a positive error value
* @retval error code defined in message_error_code_t
*/
EXPORT_API
message_error_code_t message_adaptor_disconnect(message_adaptor_plugin_h plugin, message_adaptor_plugin_context_h context, message_adaptor_error_code_h *error_code);

/**
* @brief get TCP server connection state
* @details get connection state flag
* @param[in]	plugin			specifies Message-adaptor Plugin handle
* @param[in]	context			specifies Message-adaptor Plugin context
* @param[out]	state			specifies TCP connection state
* @param[out]	error_code		specifies error code
* @return MESSAGE_ADAPTOR_ERROR_NONE (0) on success, otherwise a positive error value
* @retval error code defined in message_error_code_t
*/
EXPORT_API
message_error_code_t message_adaptor_get_connection_state(message_adaptor_plugin_h plugin, message_adaptor_plugin_context_h context, message_connection_state_t *state, message_adaptor_error_code_h *error_code);

/**
* @brief Decode Push Messages received from SPP Push server
* @param[in]	plugin		specifies Message-adaptor Plugin handle
* @param[in]	context		specifies Message-adaptor Plugin context
* @param[in]	in_msg		specifies Input Messages(json format, include encrypted fields)
* @param[out]	out_msg		specifies Output Messages(decrypted)
* @param[out]	error_code	specifies error to be sent to server
* @return MESSAGE_ADAPTOR_ERROR_NONE (0) on success, otherwise a positive error value
* @retval error code defined in message_error_code_t
*/

EXPORT_API
message_error_code_t message_adaptor_decode_push_message(message_adaptor_plugin_h plugin,
						message_adaptor_plugin_context_h context,
						char *in_msg,
						char **out_msg,
						message_adaptor_error_code_h *error_code);

#endif
