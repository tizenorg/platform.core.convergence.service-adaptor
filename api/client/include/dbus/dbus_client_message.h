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
 * File: dbus-client-message.h
 * Desc: D-Bbus IPC client APIs for message
 * Created on: Oct, 2014
 * Auth: Yongjin Kim <youth.kim@samsung.com>
 *
 *****************************************************************************/
/**
 *	@file		dbus-client-message.h
 *	@brief		Defines interface of D-Bus IPC
 *	@version	0.1
 */

#ifndef __DBUS_CLIENT_MESSAGE_H__
#define __DBUS_CLIENT_MESSAGE_H__

#include <glib.h>
#include <gio/gio.h>
#include "service_adaptor_client_type.h"
#include "private/service-adaptor-client-message.h"

void on_message_signal(GDBusProxy *proxy,
						gchar *sender_name,
						gchar *signal_name,
						GVariant *parameters,
						gpointer user_data);

/**
 * @brief
 * @param[out]
 * @param[out]
 * @return
 * @pre This function requires opened DBus connection by service-adaptor-client-messaging.c
 */
int _dbus_request_create_chatroom(const char *service_name,
						long long int request_id,
						int chat_type,
						long long int *receivers,
						unsigned int receivers_len,
						const char *chatroom_title,
						void *user_data,
						service_adaptor_error_s *error);

int _dbus_request_change_chatroom_meta(const char *service_name,
						long long int request_id,
						long long int chatroom_id,
						const char *chatroom_title,
						int default_message_ttl,
						void *user_data,
						service_adaptor_error_s *error);

int _dbus_request_chat(const char *service_name,
						long long int request_id,
						long long int chatroom_id,
						service_adaptor_chat_msg_s **chat_msgs,
						unsigned int chat_msgs_len,
						void *user_data,
						service_adaptor_error_s *error);

int _dbus_request_allow_chat(const char *service_name,
						long long int request_id,
						long long int chatroom_id,
						bool is_auto_allow,
						int max_count,
						bool need_delivery_ack,
						unsigned long long last_delivery_ack_timestamp,
						bool need_read_ack,
						unsigned long long last_read_ack_timestamp,
						bool need_ordered_chat_member_list,
						void *user_data,
						service_adaptor_error_s *error);

int _dbus_request_all_unread_message(const char *service_name,
						long long int request_id,
						int max_count,
						void *user_data,
						service_adaptor_error_s *error);

int _dbus_reply_forward_online_message(const char *service_name,
						long long int request_id,
						long long int chatroom_id,
						bool mark_as_read,
						void *user_data,
						service_adaptor_error_s *error);

int _dbus_reply_forward_unread_message(const char *service_name,
						long long int request_id,
						const char *next_pagination_key,
						int max_count,
						void *user_data,
						service_adaptor_error_s *error);

int _dbus_request_read_message(const char *service_name,
						long long int request_id,
						long long int chatroom_id,
						service_adaptor_inbox_message_s *inbox_msg,
						void *user_data,
						service_adaptor_error_s *error);

int _dbus_request_invite_chat(const char *service_name,
						long long int request_id,
						long long int chatroom_id,
						long long int *inviting_members,
						unsigned int inviting_members_len,
						void *user_data,
						service_adaptor_error_s *error);

int _dbus_request_end_chat(const char *service_name,
						long long int request_id,
						service_adaptor_end_chat_s **end_chats,
						unsigned int end_chats_len,
						void *user_data,
						service_adaptor_error_s *error);

int _dbus_request_unseal_message(const char *service_name,
						long long int request_id,
						long long int chatroom_id,
						long long int sender_id,
						long long int message_id,
						const char *message_detail,
						void *user_data,
						service_adaptor_error_s *error);

int _dbus_request_save_call_log(const char *service_name,
						long long int request_id,
						long long int chatroom_id,
						const char *call_id,
						const char *call_log_type,
						long long int call_sender_id,
						long long int call_receiver_id,
						int conversaction_second,
						void *user_data,
						service_adaptor_error_s *error);

int _dbus_request_current_time(const char *service_name,
						long long int request_id,
						void *user_data,
						service_adaptor_error_s *error);

int _dbus_request_get_connection_policy(const char *service_name,
						service_adaptor_connection_policy_e *policy,
						service_adaptor_error_s *error);

int _dbus_request_set_connection_policy(const char *service_name,
						service_adaptor_connection_policy_e *policy,
						service_adaptor_error_s *error);

int _dbus_get_chat_id_list(const char *service_name,
						service_adaptor_phone_number_s **phone_numbers,
						unsigned int phone_numbers_len,
						void *user_data,
						service_adaptor_chat_id_s ***chat_ids,
						unsigned int *chat_ids_len,
						void **server_data,
						service_adaptor_error_s *error);

int _dbus_get_msisdn_list(const char *service_name,
						long long int *chat_ids,
						unsigned int chat_ids_len,
						void *user_data,
						service_adaptor_chat_id_s ***msisdns,
						unsigned int *msisdns_len,
						void **server_data,
						service_adaptor_error_s *error);

#endif /* __DBUS_CLIENT_MESSAGE_H__ */

