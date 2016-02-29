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

#ifndef __DBUS_MESSAGE_ADAPTOR_H__
#define __DBUS_MESSAGE_ADAPTOR_H__

void message_adaptor_method_call(GDBusConnection *connection,
						const gchar *sender,
						const gchar *object_path,
						const gchar *interface_name,
						const gchar *method_name,
						GVariant *parameters,
						GDBusMethodInvocation *invocation,
						gpointer user_data);

service_adaptor_internal_error_code_e dbus_reply_create_chatroom_callback(int64_t request_id,
						int64_t chatroom_id,
						int default_message_ttl,
						message_adaptor_wrong_receiver_s *wrong_receiver,
						message_adaptor_error_code_t *error_code,
						void *server_data);

service_adaptor_internal_error_code_e dbus_reply_change_chatroom_meta_callback(int64_t request_id,
						int64_t chatroom_id,
						message_adaptor_error_code_t *error_code,
						void *server_data);

service_adaptor_internal_error_code_e dbus_reply_chat_callback(int64_t request_id,
						int64_t chatroom_id,
						message_adaptor_processed_msg_s **processed_msgs,
						unsigned int processed_msgs_len,
						message_adaptor_error_code_t *error_code,
						void *server_data);

service_adaptor_internal_error_code_e dbus_reply_allow_chat_callback(int64_t request_id,
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
						message_adaptor_error_code_t *error_code,
						void *server_data);

service_adaptor_internal_error_code_e dbus_reply_all_unread_message_callback(int64_t request_id,
						message_adaptor_error_code_t *error_code,
						void *server_data);

service_adaptor_internal_error_code_e dbus_request_forward_online_message_callback(int64_t request_id,
						int64_t chatroom_id,
						int chat_type,
						message_adaptor_inbox_message_s *inbox_msg,
						bool skip_reply,
						message_adaptor_error_code_t *error_code,
						void *server_data);

service_adaptor_internal_error_code_e dbus_request_forward_unread_message_callback(int64_t request_id,
						message_adaptor_inbox_message_s **inbox_msgs,
						unsigned int inbox_msgs_len,
						char *next_pagination_key,
						message_adaptor_error_code_t *error_code,
						void *server_data);

service_adaptor_internal_error_code_e dbus_reply_read_message_callback(int64_t request_id,
						int64_t chatroom_id,
						message_adaptor_error_code_t *error_code,
						void *server_data);

service_adaptor_internal_error_code_e dbus_reply_invite_chat_callback(int64_t request_id,
						int64_t chatroom_id,
						int64_t sent_time,
						message_adaptor_wrong_receiver_s *wrong_receiver,
						message_adaptor_error_code_t *error_code,
						void *server_data);

service_adaptor_internal_error_code_e dbus_reply_end_chat_callback(int64_t request_id,
						message_adaptor_error_code_t *error_code,
						void *server_data);

service_adaptor_internal_error_code_e dbus_reply_unseal_message_callback(int64_t request_id,
						int64_t chatroom_id,
						message_adaptor_error_code_t *error_code,
						void *server_data);

service_adaptor_internal_error_code_e dbus_reply_save_call_log_callback(int64_t request_id,
						message_adaptor_error_code_t *error_code,
						void *server_data);

service_adaptor_internal_error_code_e dbus_reply_current_time_callback(int64_t request_id,
						int64_t current_time_millis,
						message_adaptor_error_code_t *error_code,
						void *server_data);

service_adaptor_internal_error_code_e dbus_reply_message_channel_disconnected_callback(const char *service_name,
						message_adaptor_error_code_t *error_code);

#endif /* __DBUS_MESSAGE_ADAPTOR_H__ */
