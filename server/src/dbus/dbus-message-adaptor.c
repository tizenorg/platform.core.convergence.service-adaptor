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
#include <glib.h>
#include <gio/gio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/syscall.h>

#include "service-adaptor.h"
#include "service-adaptor-message.h"
#include "service-adaptor-type.h"
#include "service-adaptor-log.h"
#include "dbus-message-adaptor.h"
#include "dbus-server.h"
#include "dbus-server-type.h"
#include "dbus-util.h"

static char __MESSAGE_ADAPTOR_ERROR_NONE[] = "0";
static char __MESSAGE_ADAPTOR_ERROR_NOT_FOUND[] = "10";
static char __MESSAGE_ADAPTOR_ERROR_CONNECTION[] = "601";

void __get_create_chatroom_req_type(GVariant *parameters,
						char **service_name,
						int64_t *request_id,
						int *chat_type,
						int64_t **receivers,
						unsigned int *receivers_len,
						char **chatroom_title)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *req_struct[private_service_adaptor_create_chatroom_req_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_create_chatroom_req_s_type_length; j++) {
		req_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*service_name = ipc_g_variant_dup_string(req_struct[idx++]);
	*request_id = g_variant_get_int64(req_struct[idx++]);
	*chat_type = g_variant_get_int32(req_struct[idx++]);

	gsize list_count = g_variant_n_children(req_struct[idx]);
	*receivers = (int64_t *) calloc(list_count, sizeof(int64_t));

	if (NULL != (*receivers)) {
		for (gsize i = 0; i < list_count; i++) {
			GVariant *info_struct;
			GVariant *info_entry_v = g_variant_get_child_value(req_struct[idx], i);

			info_struct = g_variant_get_child_value(info_entry_v, 0);
			(*receivers)[i] = g_variant_get_int64(info_struct);
		}
		idx++;
		*receivers_len = g_variant_get_uint32(req_struct[idx++]);
	} else {
		*receivers_len = 0U;
		idx += 2;
	}

	*chatroom_title = ipc_g_variant_dup_string(req_struct[idx++]);

	for (size_t j = 0; j < private_service_adaptor_create_chatroom_req_s_type_length; j++) {
		g_variant_unref(req_struct[j]);
	}
}

void __get_change_chatroom_meta_req_type(GVariant *parameters,
						char **service_name,
						int64_t *request_id,
						int64_t *chatroom_id,
						char **chatroom_title,
						int *default_message_ttl)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *req_struct[private_service_adaptor_change_chatroom_meta_req_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_change_chatroom_meta_req_s_type_length; j++) {
		req_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*service_name = ipc_g_variant_dup_string(req_struct[idx++]);
	*request_id = g_variant_get_int64(req_struct[idx++]);
	*chatroom_id = g_variant_get_int64(req_struct[idx++]);
	*chatroom_title = ipc_g_variant_dup_string(req_struct[idx++]);
	*default_message_ttl = g_variant_get_int32(req_struct[idx++]);

	for (size_t j = 0; j < private_service_adaptor_change_chatroom_meta_req_s_type_length; j++) {
		g_variant_unref(req_struct[j]);
	}
}

void __get_chat_req_type(GVariant *parameters,
						char **service_name,
						int64_t *request_id,
						int64_t *chatroom_id,
						message_adaptor_chat_msg_s ***chat_msgs,
						unsigned int *chat_msgs_len)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *req_struct[private_service_adaptor_chat_req_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_chat_req_s_type_length; j++) {
		req_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*service_name = ipc_g_variant_dup_string(req_struct[idx++]);
	*request_id = g_variant_get_int64(req_struct[idx++]);
	*chatroom_id = g_variant_get_int64(req_struct[idx++]);

	gsize list_count = g_variant_n_children(req_struct[idx]);
	*chat_msgs = (message_adaptor_chat_msg_s **) calloc(list_count, sizeof(message_adaptor_chat_msg_s *));

	if (NULL != (*chat_msgs)) {
		for (gsize i = 0; i < list_count; i++) {
			GVariant *info_struct[private_service_adaptor_chat_msg_s_type_length];
			GVariant *info_entry_v = g_variant_get_child_value(req_struct[idx], i);

			for (size_t j = 0; j < private_service_adaptor_chat_msg_s_type_length; j++) {
				info_struct[j] = g_variant_get_child_value(info_entry_v, j);
			}

			int idx2 = 0;
			(*chat_msgs)[i] = (message_adaptor_chat_msg_s *) calloc(1, sizeof(message_adaptor_chat_msg_s));
			if (NULL != ((*chat_msgs)[i])) {
				(*chat_msgs)[i]->msg_id = g_variant_get_int64(info_struct[idx2++]);
				(*chat_msgs)[i]->msg_type = g_variant_get_int32(info_struct[idx2++]);
				(*chat_msgs)[i]->chatmsg = ipc_g_variant_dup_string(info_struct[idx2++]);
				(*chat_msgs)[i]->message_ttl = g_variant_get_int32(info_struct[idx2++]);
			}

			for (size_t j = 0; j < private_service_adaptor_chat_msg_s_type_length; j++) {
				g_variant_unref(info_struct[j]);
			}
			g_variant_unref(info_entry_v);
		}
		idx++;
		*chat_msgs_len = g_variant_get_uint32(req_struct[idx++]);
	} else {
		*chat_msgs_len = 0U;
		idx += 2;
	}

	for (size_t j = 0; j < private_service_adaptor_chat_req_s_type_length; j++) {
		g_variant_unref(req_struct[j]);
	}
}

void __get_allow_chat_req_type(GVariant *parameters,
						char **service_name,
						int64_t *request_id,
						int64_t *chatroom_id,
						bool *is_auto_allow,
						int *max_count,
						bool *need_delivery_acks,
						unsigned long long *delivery_acks_timestamp,
						bool *need_read_ack,
						unsigned long long *last_read_ack_timestamp,
						bool *need_ordered_chat_member_list)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *req_struct[private_service_adaptor_allow_chat_req_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_allow_chat_req_s_type_length; j++) {
		req_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*service_name = ipc_g_variant_dup_string(req_struct[idx++]);
	*request_id = g_variant_get_int64(req_struct[idx++]);
	*chatroom_id = g_variant_get_int64(req_struct[idx++]);
	*is_auto_allow = g_variant_get_boolean(req_struct[idx++]);
	*max_count = g_variant_get_int32(req_struct[idx++]);
	*need_delivery_acks = g_variant_get_boolean(req_struct[idx++]);
	*delivery_acks_timestamp = g_variant_get_uint64(req_struct[idx++]);
	*need_read_ack = g_variant_get_boolean(req_struct[idx++]);
	*last_read_ack_timestamp = g_variant_get_uint64(req_struct[idx++]);
	*need_ordered_chat_member_list = g_variant_get_boolean(req_struct[idx++]);

	for (size_t j = 0; j < private_service_adaptor_allow_chat_req_s_type_length; j++) {
		g_variant_unref(req_struct[j]);
	}
}

void __get_all_unread_message_req_type(GVariant *parameters,
						char **service_name,
						int64_t *request_id,
						int *max_count)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *req_struct[private_service_adaptor_all_unread_message_req_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_all_unread_message_req_s_type_length; j++) {
		req_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*service_name = ipc_g_variant_dup_string(req_struct[idx++]);
	*request_id = g_variant_get_int64(req_struct[idx++]);
	*max_count = g_variant_get_int32(req_struct[idx++]);

	for (size_t j = 0; j < private_service_adaptor_all_unread_message_req_s_type_length; j++) {
		g_variant_unref(req_struct[j]);
	}
}

void __get_forward_online_message_res_type(GVariant *parameters,
						char **service_name,
						int64_t *request_id,
						int64_t *chatroom_id,
						bool *mark_as_read)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *req_struct[private_service_adaptor_forward_online_message_res_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_forward_online_message_res_s_type_length; j++) {
		req_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*service_name = ipc_g_variant_dup_string(req_struct[idx++]);
	*request_id = g_variant_get_int64(req_struct[idx++]);
	*chatroom_id = g_variant_get_int64(req_struct[idx++]);
	*mark_as_read = g_variant_get_boolean(req_struct[idx++]);

	for (size_t j = 0; j < private_service_adaptor_forward_online_message_res_s_type_length; j++) {
		g_variant_unref(req_struct[j]);
	}
}

void __get_forward_unread_message_res_type(GVariant *parameters,
						char **service_name,
						int64_t *request_id,
						char **next_pagination_key,
						int *max_count)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *req_struct[private_service_adaptor_forward_unread_message_res_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_forward_unread_message_res_s_type_length; j++) {
		req_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*service_name = ipc_g_variant_dup_string(req_struct[idx++]);
	*request_id = g_variant_get_int64(req_struct[idx++]);
	*next_pagination_key = ipc_g_variant_dup_string(req_struct[idx++]);
	*max_count = g_variant_get_int32(req_struct[idx++]);

	for (size_t j = 0; j < private_service_adaptor_forward_unread_message_res_s_type_length; j++) {
		g_variant_unref(req_struct[j]);
	}
}

void __get_read_message_req_type(GVariant *parameters,
						char **service_name,
						int64_t *request_id,
						int64_t *chatroom_id,
						message_adaptor_inbox_message_s **inbox_msg)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *req_struct[private_service_adaptor_read_message_req_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_read_message_req_s_type_length; j++) {
		req_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*service_name = ipc_g_variant_dup_string(req_struct[idx++]);
	*request_id = g_variant_get_int64(req_struct[idx++]);
	*chatroom_id = g_variant_get_int64(req_struct[idx++]);

	GVariant *req_info_struct[private_service_adaptor_inbox_message_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_inbox_message_s_type_length; j++) {
		req_info_struct[j] = g_variant_get_child_value(req_struct[idx], j);
	}

	int idx2 = 0;
	*inbox_msg = (message_adaptor_inbox_message_s *) calloc(1, sizeof(message_adaptor_inbox_message_s));
	if (NULL != (*inbox_msg)) {
		(*inbox_msg)->msgId = g_variant_get_int64(req_info_struct[idx2++]);
		(*inbox_msg)->msgType = g_variant_get_int32(req_info_struct[idx2++]);
		(*inbox_msg)->sender = g_variant_get_int64(req_info_struct[idx2++]);
		(*inbox_msg)->receiver = g_variant_get_int64(req_info_struct[idx2++]);
		(*inbox_msg)->sentTime = g_variant_get_int64(req_info_struct[idx2++]);
		(*inbox_msg)->chatMsg = ipc_g_variant_dup_string(req_info_struct[idx2++]);
		(*inbox_msg)->chatroomId = g_variant_get_int64(req_info_struct[idx2++]);
		(*inbox_msg)->chatType = g_variant_get_int32(req_info_struct[idx2++]);
	}

	for (size_t j = 0; j < private_service_adaptor_inbox_message_s_type_length; j++) {
		g_variant_unref(req_info_struct[j]);
	}

	for (size_t j = 0; j < private_service_adaptor_read_message_req_s_type_length; j++) {
		g_variant_unref(req_struct[j]);
	}
}

void __get_invite_chat_req_type(GVariant *parameters,
						char **service_name,
						int64_t *request_id,
						int64_t *chatroom_id,
						int64_t **inviting_members,
						unsigned int *inviting_members_len)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *req_struct[private_service_adaptor_invite_chat_req_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_invite_chat_req_s_type_length; j++) {
		req_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*service_name = ipc_g_variant_dup_string(req_struct[idx++]);
	*request_id = g_variant_get_int64(req_struct[idx++]);
	*chatroom_id = g_variant_get_int64(req_struct[idx++]);

	gsize list_count = g_variant_n_children(req_struct[idx]);
	*inviting_members = (int64_t *) calloc(list_count, sizeof(int64_t));

	if (NULL != (*inviting_members)) {
		for (gsize i = 0; i < list_count; i++) {
			GVariant *info_entry_v = g_variant_get_child_value(req_struct[idx], i);
			GVariant *info_struct = g_variant_get_child_value(info_entry_v, 0);

			(*inviting_members)[i] = g_variant_get_int64(info_struct);

			g_variant_unref(info_struct);
			g_variant_unref(info_entry_v);
		}
		idx++;

		*inviting_members_len = g_variant_get_uint32(req_struct[idx++]);
	} else {
		*inviting_members_len = 0U;
		idx += 2;
	}

	for (size_t j = 0; j < private_service_adaptor_invite_chat_req_s_type_length; j++) {
		g_variant_unref(req_struct[j]);
	}
}

void __get_end_chat_req_type(GVariant *parameters,
						char **service_name,
						int64_t *request_id,
						message_adaptor_end_chat_s ***end_chats,
						unsigned int *end_chats_len)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *req_struct[private_service_adaptor_end_chat_req_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_end_chat_req_s_type_length; j++) {
		req_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*service_name = ipc_g_variant_dup_string(req_struct[idx++]);
	*request_id = g_variant_get_int64(req_struct[idx++]);

	gsize list_count = g_variant_n_children(req_struct[idx]);
	*end_chats = (message_adaptor_end_chat_s **) calloc(list_count, sizeof(message_adaptor_end_chat_s *));

	if (NULL != (*end_chats)) {
		for (gsize i = 0; i < list_count; i++) {
			GVariant *info_struct[private_service_adaptor_end_chat_s_type_length];
			GVariant *info_entry_v = g_variant_get_child_value(req_struct[idx], i);

			for (size_t j = 0; j < private_service_adaptor_end_chat_s_type_length; j++) {
				info_struct[j] = g_variant_get_child_value(info_entry_v, j);
			}

			int idx2 = 0;
			(*end_chats)[i] = (message_adaptor_end_chat_s *) calloc(1, sizeof(message_adaptor_end_chat_s));
			if (NULL != (*end_chats)[i]) {
				(*end_chats)[i]->chatroom_id = g_variant_get_int64(info_struct[idx2++]);
				(*end_chats)[i]->deny_invitation = g_variant_get_boolean(info_struct[idx2++]);
			}

			for (size_t j = 0; j < private_service_adaptor_end_chat_s_type_length; j++) {
				g_variant_unref(info_struct[j]);
			}
			g_variant_unref(info_entry_v);
		}
		idx++;

		*end_chats_len = g_variant_get_uint32(req_struct[idx++]);
	} else {
		*end_chats_len = 0U;
		idx += 2;
	}

	for (size_t j = 0; j < private_service_adaptor_end_chat_req_s_type_length; j++) {
		g_variant_unref(req_struct[j]);
	}
}

void __get_unseal_message_req_type(GVariant *parameters,
						char **service_name,
						int64_t *request_id,
						int64_t *chatroom_id,
						int64_t *sender_id,
						int64_t *message_id,
						char **message_detail)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *req_struct[private_service_adaptor_unseal_message_req_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_unseal_message_req_s_type_length; j++) {
		req_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*service_name = ipc_g_variant_dup_string(req_struct[idx++]);
	*request_id = g_variant_get_int64(req_struct[idx++]);
	*chatroom_id = g_variant_get_int64(req_struct[idx++]);
	*sender_id = g_variant_get_int64(req_struct[idx++]);
	*message_id = g_variant_get_int64(req_struct[idx++]);
	*message_detail = ipc_g_variant_dup_string(req_struct[idx++]);

	for (size_t j = 0; j < private_service_adaptor_unseal_message_req_s_type_length; j++) {
		g_variant_unref(req_struct[j]);
	}
}

void __get_save_call_log_req_type(GVariant *parameters,
						char **service_name,
						int64_t *request_id,
						int64_t *chatroom_id,
						char **call_id,
						char **call_log_type,
						int64_t *call_sender_id,
						int64_t *call_receiver_id,
						int *conversaction_second)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *req_struct[private_service_adaptor_save_call_log_req_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_save_call_log_req_s_type_length; j++) {
		req_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*service_name = ipc_g_variant_dup_string(req_struct[idx++]);
	*request_id = g_variant_get_int64(req_struct[idx++]);
	*chatroom_id = g_variant_get_int64(req_struct[idx++]);
	*call_id = ipc_g_variant_dup_string(req_struct[idx++]);
	*call_log_type = ipc_g_variant_dup_string(req_struct[idx++]);
	*call_sender_id = g_variant_get_int64(req_struct[idx++]);
	*call_receiver_id = g_variant_get_int64(req_struct[idx++]);
	*conversaction_second = g_variant_get_int32(req_struct[idx++]);

	for (size_t j = 0; j < private_service_adaptor_save_call_log_req_s_type_length; j++) {
		g_variant_unref(req_struct[j]);
	}
}

void __get_current_time_req_type(GVariant *parameters,
						char **service_name,
						int64_t *request_id)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *req_struct[private_service_adaptor_current_time_req_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_current_time_req_s_type_length; j++) {
		req_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*service_name = ipc_g_variant_dup_string(req_struct[idx++]);
	*request_id = g_variant_get_int64(req_struct[idx++]);

	for (size_t j = 0; j < private_service_adaptor_current_time_req_s_type_length; j++) {
		g_variant_unref(req_struct[j]);
	}
}

void __get_get_connection_policy_req_type(GVariant *parameters,
						char **service_name)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *req_struct[private_service_adaptor_get_connection_policy_req_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_get_connection_policy_req_s_type_length; j++) {
		req_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*service_name = ipc_g_variant_dup_string(req_struct[idx++]);

	for (size_t j = 0; j < private_service_adaptor_get_connection_policy_req_s_type_length; j++) {
		g_variant_unref(req_struct[j]);
	}
}

void __get_set_connection_policy_req_type(GVariant *parameters,
						char **service_name,
						int *policy)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *req_struct[private_service_adaptor_set_connection_policy_req_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_set_connection_policy_req_s_type_length; j++) {
		req_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*service_name = ipc_g_variant_dup_string(req_struct[idx++]);
	*policy = g_variant_get_int32(req_struct[idx++]);

	for (size_t j = 0; j < private_service_adaptor_set_connection_policy_req_s_type_length; j++) {
		g_variant_unref(req_struct[j]);
	}
}



void __get_chat_id_list_req_type(GVariant *parameters,
						char **service_name,
						message_adaptor_phone_number_s ***phone_number,
						unsigned int *phone_number_len)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *req_struct[private_service_adaptor_chat_id_list_req_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_chat_id_list_req_s_type_length; j++) {
		req_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*service_name = ipc_g_variant_dup_string(req_struct[idx++]);

	gsize list_count = g_variant_n_children(req_struct[idx]);
	*phone_number = (message_adaptor_phone_number_s **) calloc(list_count, sizeof(message_adaptor_phone_number_s *));

	if (NULL != (*phone_number)) {
		for (gsize i = 0; i < list_count; i++) {
			GVariant *info_struct[private_service_adaptor_phone_number_s_type_length];
			GVariant *info_entry_v = g_variant_get_child_value(req_struct[idx], i);

			for (size_t j = 0; j < private_service_adaptor_phone_number_s_type_length; j++) {
				info_struct[j] = g_variant_get_child_value(info_entry_v, j);
			}

			int idx2 = 0;
			(*phone_number)[i] = (message_adaptor_phone_number_s *) calloc(1, sizeof(message_adaptor_phone_number_s));
			if (NULL != (*phone_number)[i]) {
				(*phone_number)[i]->phonenumber = ipc_g_variant_dup_string(info_struct[idx2++]);
				(*phone_number)[i]->ccc = ipc_g_variant_dup_string(info_struct[idx2++]);
			}

			for (size_t j = 0; j < private_service_adaptor_phone_number_s_type_length; j++) {
				g_variant_unref(info_struct[j]);
			}
		}
		idx++;

		*phone_number_len = g_variant_get_uint32(req_struct[idx++]);
	} else {
		*phone_number_len = 0U;
		idx += 2;
	}

	for (size_t j = 0; j < private_service_adaptor_chat_id_list_req_s_type_length; j++) {
		g_variant_unref(req_struct[j]);
	}
}

void __get_msisdn_list_req_type(GVariant *parameters,
						char **service_name,
						int64_t **chat_ids,
						unsigned int *chat_ids_len)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *req_struct[private_service_adaptor_msisdn_list_req_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_msisdn_list_req_s_type_length; j++) {
		req_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*service_name = ipc_g_variant_dup_string(req_struct[idx++]);

	gsize list_count = g_variant_n_children(req_struct[idx]);
	*chat_ids = (int64_t *) calloc(list_count, sizeof(int64_t));

	if (NULL != (*chat_ids)) {
		for (gsize i = 0; i < list_count; i++) {
			GVariant *info_entry_v = g_variant_get_child_value(req_struct[idx], i);
			GVariant *info_struct = g_variant_get_child_value(info_entry_v, 0);;

			(*chat_ids)[i] = g_variant_get_int64(info_struct);

			g_variant_unref(info_struct);
		}
		idx++;

		*chat_ids_len = g_variant_get_uint32(req_struct[idx++]);
	} else {
		*chat_ids_len = 0U;
		idx += 2;
	}

	for (size_t j = 0; j < private_service_adaptor_msisdn_list_req_s_type_length; j++) {
		g_variant_unref(req_struct[j]);
	}
}

GVariant *__create_chat_id_list_res_type(message_adaptor_chat_id_s **chat_ids,
						unsigned int chat_ids_len,
						message_adaptor_error_code_t *error_code)
{
	if (NULL == chat_ids) {
		chat_ids_len = 0;
	}

	GVariantBuilder *builder = g_variant_builder_new(G_VARIANT_TYPE(private_message_chat_id_list_type));

	for (gsize i = 0; i < chat_ids_len; i++) {
		g_variant_builder_open(builder, G_VARIANT_TYPE(private_service_adaptor_chat_id_s_type));
		g_variant_builder_add(builder, "x", chat_ids[i]->chatid);
		safe_g_variant_builder_add_string(builder, chat_ids[i]->msisdn);
		g_variant_builder_close(builder);
	}

	GVariant *response = g_variant_new(MAKE_RETURN_TYPE(private_service_adaptor_chat_id_list_res_s_type),
			builder, chat_ids_len, (uint64_t) atoi(__safe_add_string(error_code->code)), __safe_add_string(error_code->msg));

	g_variant_builder_unref(builder);

	return response;
}

void message_adaptor_method_call(GDBusConnection *connection,
						const gchar *sender,
						const gchar *object_path,
						const gchar *interface_name,
						const gchar *method_name,
						GVariant *parameters,
						GDBusMethodInvocation *invocation,
						gpointer user_data)
{
	service_adaptor_internal_error_code_e ret_code = SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;

	if (0 == g_strcmp0(method_name, PRIVATE_DBUS_REQUEST_CREATE_CHATROOM_METHOD)) {
		char *service_name = NULL;
		int64_t request_id = 0;
		int chat_type = 0;
		int64_t *receivers = NULL;
		unsigned int receivers_len = 0;
		char *chatroom_title = NULL;
		message_adaptor_error_code_t *error_code = NULL;
		message_adaptor_error_code_t _error;
		_error.code = _error.msg = NULL;

		__get_create_chatroom_req_type(parameters, &service_name, &request_id, &chat_type,
				&receivers, &receivers_len, &chatroom_title);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = __MESSAGE_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = "Can not get service context";

			g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)",
					(uint64_t) atoi(__safe_add_string(error_code->code)), __safe_add_string(error_code->msg)));

			free(service_name);
			free(receivers);
			free(chatroom_title);
			return;
		}

		message_adaptor_h adaptor = service_adaptor_get_message_adaptor(service_adaptor);
		message_adaptor_plugin_h plugin = NULL;

		if (NULL != service->message_context) {
			plugin = message_adaptor_get_plugin_by_name(adaptor, service->message_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			if (MESSAGE_CONNECTION_POLICY_DISCONNECT == service->message_context->connection_policy) {
				service_adaptor_debug_func("Connection disconnected. Convert Auto connection");
				ret_code = service_adaptor_message_set_connection(plugin, service->message_context, MESSAGE_CONNECTION_POLICY_AUTO, &error_code);
				service_adaptor_debug("set_connection res (%d)", ret_code);
				if (NULL != error_code) {
					service_adaptor_debug("rcode : %s, msg : %s", error_code->code, error_code->msg);
					free(error_code->code);
					error_code->code = strdup(__MESSAGE_ADAPTOR_ERROR_CONNECTION);
				} else if (SERVICE_ADAPTOR_INTERNAL_ERROR_NONE != ret_code) {
					error_code = message_adaptor_create_error_code(__MESSAGE_ADAPTOR_ERROR_CONNECTION, "Server connect failed");
				}
			}
			if (NULL == error_code) {
				ret_code = message_adaptor_create_chatroom_request(plugin,
						service->message_context, request_id, chat_type, &receivers, receivers_len, chatroom_title, &error_code, NULL);

				if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
					service_adaptor_info("API returns auth-error. Try refresh auth");
					service_adaptor_auth_refresh(service_adaptor, service_name, service->message_context->plugin_uri);

					service_adaptor_debug("Empty error_code already issued (%s: %s)", error_code ? error_code->code : NULL, error_code ? error_code->msg : NULL);
					message_adaptor_destroy_error_code(&error_code);

					service_adaptor_debug("Re-try API");
					ret_code = message_adaptor_create_chatroom_request(plugin,
							service->message_context, request_id, chat_type, &receivers, receivers_len, chatroom_title, &error_code, NULL);
				}
			}
		}

		if (NULL == error_code) {
			error_code = &_error;
			error_code->code = strdup(__MESSAGE_ADAPTOR_ERROR_NONE);
			error_code->msg = strdup("");
		}

		g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)",
				(uint64_t) atoi(__safe_add_string(error_code->code)), __safe_add_string(error_code->msg)));

		if (error_code != &_error) {
			message_adaptor_destroy_error_code(&error_code);
		} else {
			free(_error.code);
			free(_error.msg);
		}
		free(service_name);
		free(receivers);
		free(chatroom_title);
	} else if (0 == g_strcmp0(method_name, PRIVATE_DBUS_REQUEST_CHANGE_CHATROOM_META_METHOD)) {
		char *service_name = NULL;
		int64_t request_id = 0;
		int64_t chatroom_id = 0;
		char *chatroom_title = NULL;
		int default_message_ttl = 0;

		message_adaptor_error_code_t *error_code = NULL;
		message_adaptor_error_code_t _error;
		_error.code = _error.msg = NULL;

		__get_change_chatroom_meta_req_type(parameters, &service_name, &request_id, &chatroom_id,
				&chatroom_title, &default_message_ttl);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = __MESSAGE_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = "Can not get service context";

			g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)",
					(uint64_t) atoi(__safe_add_string(error_code->code)), __safe_add_string(error_code->msg)));

			free(service_name);
			free(chatroom_title);
			return;
		}

		message_adaptor_h adaptor = service_adaptor_get_message_adaptor(service_adaptor);
		message_adaptor_plugin_h plugin = NULL;

		if (NULL != service->message_context) {
			plugin = message_adaptor_get_plugin_by_name(adaptor, service->message_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			if (MESSAGE_CONNECTION_POLICY_DISCONNECT == service->message_context->connection_policy) {
				service_adaptor_debug_func("Connection disconnected. Convert Auto connection");
				ret_code = service_adaptor_message_set_connection(plugin, service->message_context, MESSAGE_CONNECTION_POLICY_AUTO, &error_code);
				service_adaptor_debug("set_connection res (%d)", ret_code);
				if (NULL != error_code) {
					service_adaptor_debug("rcode : %s, msg : %s", error_code->code, error_code->msg);
					free(error_code->code);
					error_code->code = strdup(__MESSAGE_ADAPTOR_ERROR_CONNECTION);
				} else if (SERVICE_ADAPTOR_INTERNAL_ERROR_NONE != ret_code) {
					error_code = message_adaptor_create_error_code(__MESSAGE_ADAPTOR_ERROR_CONNECTION, "Server connect failed");
				}
			}
			if (NULL == error_code) {
				ret_code = message_adaptor_change_chatroom_meta_request(plugin, service->message_context,
						request_id, chatroom_id, chatroom_title, default_message_ttl, &error_code, NULL);

				if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
					service_adaptor_info("API returns auth-error. Try refresh auth");
					service_adaptor_auth_refresh(service_adaptor, service_name, service->message_context->plugin_uri);

					service_adaptor_debug("Empty error_code already issued (%s: %s)", error_code ? error_code->code : NULL, error_code ? error_code->msg : NULL);
					message_adaptor_destroy_error_code(&error_code);

					service_adaptor_debug("Re-try API");
					ret_code = message_adaptor_change_chatroom_meta_request(plugin, service->message_context,
							request_id, chatroom_id, chatroom_title, default_message_ttl, &error_code, NULL);
				}
			}
		}

		if (NULL == error_code) {
			error_code = &_error;
			error_code->code = strdup(__MESSAGE_ADAPTOR_ERROR_NONE);
			error_code->msg = strdup("");
		}

		g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)",
				(uint64_t) atoi(__safe_add_string(error_code->code)), __safe_add_string(error_code->msg)));

		if (error_code != &_error) {
			message_adaptor_destroy_error_code(&error_code);
		} else {
			free(_error.code);
			free(_error.msg);
		}
		free(service_name);
		free(chatroom_title);
	} else if (0 == g_strcmp0(method_name, PRIVATE_DBUS_REQUEST_CHAT_METHOD)) {
		char *service_name = NULL;
		int64_t request_id = 0;
		int64_t chatroom_id = 0;
		message_adaptor_chat_msg_s **chat_msgs = NULL;
		unsigned int chat_msgs_len = 0;
		message_adaptor_error_code_t *error_code = NULL;
		message_adaptor_error_code_t _error;
		_error.code = _error.msg = NULL;

		__get_chat_req_type(parameters, &service_name, &request_id, &chatroom_id, &chat_msgs, &chat_msgs_len);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = __MESSAGE_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = "Can not get service context";

			g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)",
					(uint64_t) atoi(__safe_add_string(error_code->code)), __safe_add_string(error_code->msg)));

			free(service_name);
			if ((NULL != chat_msgs) && (0U < chat_msgs_len)) {
				for (int i = 0; i < chat_msgs_len; i++) {
					if (NULL != chat_msgs[i]) {
						free(chat_msgs[i]->chatmsg);
						free(chat_msgs[i]);
					}
				}
				free(chat_msgs);
			}
			return;
		}

		message_adaptor_h adaptor = service_adaptor_get_message_adaptor(service_adaptor);
		message_adaptor_plugin_h plugin = NULL;

		if (NULL != service->message_context) {
			plugin = message_adaptor_get_plugin_by_name(adaptor, service->message_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			if (MESSAGE_CONNECTION_POLICY_DISCONNECT == service->message_context->connection_policy) {
				service_adaptor_debug_func("Connection disconnected. Convert Auto connection");
				ret_code = service_adaptor_message_set_connection(plugin, service->message_context, MESSAGE_CONNECTION_POLICY_AUTO, &error_code);
				service_adaptor_debug("set_connection res (%d)", ret_code);
				if (NULL != error_code) {
					service_adaptor_debug("rcode : %s, msg : %s", error_code->code, error_code->msg);
					free(error_code->code);
					error_code->code = strdup(__MESSAGE_ADAPTOR_ERROR_CONNECTION);
				} else if (SERVICE_ADAPTOR_INTERNAL_ERROR_NONE != ret_code) {
					error_code = message_adaptor_create_error_code(__MESSAGE_ADAPTOR_ERROR_CONNECTION, "Server connect failed");
				}
			}
			if (NULL == error_code) {
				ret_code = message_adaptor_chat_request(plugin, service->message_context,
						request_id, chatroom_id, chat_msgs, chat_msgs_len, &error_code, NULL);

				if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
					service_adaptor_info("API returns auth-error. Try refresh auth");
					service_adaptor_auth_refresh(service_adaptor, service_name, service->message_context->plugin_uri);

					service_adaptor_debug("Empty error_code already issued (%s: %s)", error_code ? error_code->code : NULL, error_code ? error_code->msg : NULL);
					message_adaptor_destroy_error_code(&error_code);

					service_adaptor_debug("Re-try API");
					ret_code = message_adaptor_chat_request(plugin, service->message_context,
							request_id, chatroom_id, chat_msgs, chat_msgs_len, &error_code, NULL);
				}
			}
		}

		if (NULL == error_code) {
			error_code = &_error;
			error_code->code = strdup(__MESSAGE_ADAPTOR_ERROR_NONE);
			error_code->msg = strdup("");
		}


		g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)",
				(uint64_t) atoi(__safe_add_string(error_code->code)), __safe_add_string(error_code->msg)));

		if (error_code != &_error) {
			message_adaptor_destroy_error_code(&error_code);
		} else {
			free(_error.code);
			free(_error.msg);
		}

		free(service_name);
		if ((NULL != chat_msgs) && (0U < chat_msgs_len)) {
			for (int i = 0; i < chat_msgs_len; i++) {
				if (NULL != chat_msgs[i]) {
					free(chat_msgs[i]->chatmsg);
					free(chat_msgs[i]);
				}
			}
			free(chat_msgs);
		}
	} else if (0 == g_strcmp0(method_name, PRIVATE_DBUS_REQUEST_ALLOW_CHAT_METHOD)) {
		char *service_name = NULL;
		int64_t request_id = 0;
		int64_t chatroom_id = 0;
		bool is_auto_allow = false;
		int max_count = 0;
		bool need_delivery_acks = false;
		unsigned long long delivery_acks_timestamp = 0;
		bool need_read_ack = false;
		unsigned long long last_read_ack_timestamp = 0;
		bool need_ordered_chat_member_list = false;
		message_adaptor_error_code_t *error_code = NULL;
		message_adaptor_error_code_t _error;
		_error.code = _error.msg = NULL;

		__get_allow_chat_req_type(parameters, &service_name, &request_id, &chatroom_id, &is_auto_allow, &max_count,
				&need_delivery_acks, &delivery_acks_timestamp,
				&need_read_ack, &last_read_ack_timestamp,
				&need_ordered_chat_member_list);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = __MESSAGE_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = "Can not get service context";

			g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)",
					(uint64_t) atoi(__safe_add_string(error_code->code)), __safe_add_string(error_code->msg)));

			free(service_name);
			return;
		}

		message_adaptor_h adaptor = service_adaptor_get_message_adaptor(service_adaptor);
		message_adaptor_plugin_h plugin = NULL;

		if (NULL != service->message_context) {
			plugin = message_adaptor_get_plugin_by_name(adaptor, service->message_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			if (MESSAGE_CONNECTION_POLICY_DISCONNECT == service->message_context->connection_policy) {
				service_adaptor_debug_func("Connection disconnected. Convert Auto connection");
				ret_code = service_adaptor_message_set_connection(plugin, service->message_context, MESSAGE_CONNECTION_POLICY_AUTO, &error_code);
				service_adaptor_debug("set_connection res (%d)", ret_code);
				if (NULL != error_code) {
					service_adaptor_debug("rcode : %s, msg : %s", error_code->code, error_code->msg);
					free(error_code->code);
					error_code->code = strdup(__MESSAGE_ADAPTOR_ERROR_CONNECTION);
				} else if (SERVICE_ADAPTOR_INTERNAL_ERROR_NONE != ret_code) {
					error_code = message_adaptor_create_error_code(__MESSAGE_ADAPTOR_ERROR_CONNECTION, "Server connect failed");
				}
			}
			if (NULL == error_code) {
				ret_code = message_adaptor_allow_chat_request(plugin, service->message_context,
						request_id, chatroom_id, is_auto_allow, max_count, need_delivery_acks, delivery_acks_timestamp,
						need_read_ack, last_read_ack_timestamp, need_ordered_chat_member_list, &error_code, NULL);

				if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
					service_adaptor_info("API returns auth-error. Try refresh auth");
					service_adaptor_auth_refresh(service_adaptor, service_name, service->message_context->plugin_uri);

					service_adaptor_debug("Empty error_code already issued (%s: %s)", error_code ? error_code->code : NULL, error_code ? error_code->msg : NULL);
					message_adaptor_destroy_error_code(&error_code);

					service_adaptor_debug("Re-try API");
					ret_code = message_adaptor_allow_chat_request(plugin, service->message_context,
							request_id, chatroom_id, is_auto_allow, max_count, need_delivery_acks, delivery_acks_timestamp,
							need_read_ack, last_read_ack_timestamp, need_ordered_chat_member_list, &error_code, NULL);
				}
			}
		}

		if (NULL == error_code) {
			error_code = &_error;
			error_code->code = strdup(__MESSAGE_ADAPTOR_ERROR_NONE);
			error_code->msg = strdup("");
		}


		g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)",
				(uint64_t) atoi(__safe_add_string(error_code->code)), __safe_add_string(error_code->msg)));

		if (error_code != &_error) {
			message_adaptor_destroy_error_code(&error_code);
		} else {
			free(_error.code);
			free(_error.msg);
		}
		free(service_name);
	} else if (0 == g_strcmp0(method_name, PRIVATE_DBUS_REQUEST_ALL_UNREAD_MESSAGE_METHOD)) {
		char *service_name = NULL;
		int64_t request_id = 0;
		int max_count = 0;
		message_adaptor_error_code_t *error_code = NULL;
		message_adaptor_error_code_t _error;
		_error.code = _error.msg = NULL;

		__get_all_unread_message_req_type(parameters, &service_name, &request_id, &max_count);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = __MESSAGE_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = "Can not get service context";

			g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)",
					(uint64_t) atoi(__safe_add_string(error_code->code)), __safe_add_string(error_code->msg)));

			free(service_name);
			return;
		}

		message_adaptor_h adaptor = service_adaptor_get_message_adaptor(service_adaptor);
		message_adaptor_plugin_h plugin = NULL;

		if (NULL != service->message_context) {
			plugin = message_adaptor_get_plugin_by_name(adaptor, service->message_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			if (MESSAGE_CONNECTION_POLICY_DISCONNECT == service->message_context->connection_policy) {
				service_adaptor_debug_func("Connection disconnected. Convert Auto connection");
				ret_code = service_adaptor_message_set_connection(plugin, service->message_context, MESSAGE_CONNECTION_POLICY_AUTO, &error_code);
				service_adaptor_debug("set_connection res (%d)", ret_code);
				if (NULL != error_code) {
					service_adaptor_debug("rcode : %s, msg : %s", error_code->code, error_code->msg);
					free(error_code->code);
					error_code->code = strdup(__MESSAGE_ADAPTOR_ERROR_CONNECTION);
				} else if (SERVICE_ADAPTOR_INTERNAL_ERROR_NONE != ret_code) {
					error_code = message_adaptor_create_error_code(__MESSAGE_ADAPTOR_ERROR_CONNECTION, "Server connect failed");
				}
			}
			if (NULL == error_code) {
				ret_code = message_adaptor_get_all_unread_message_request(plugin, service->message_context,
						request_id, max_count, &error_code, NULL);

				if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
					service_adaptor_info("API returns auth-error. Try refresh auth");
					service_adaptor_auth_refresh(service_adaptor, service_name, service->message_context->plugin_uri);

					service_adaptor_debug("Empty error_code already issued (%s: %s)", error_code ? error_code->code : NULL, error_code ? error_code->msg : NULL);
					message_adaptor_destroy_error_code(&error_code);

					service_adaptor_debug("Re-try API");
					ret_code = message_adaptor_get_all_unread_message_request(plugin, service->message_context,
							request_id, max_count, &error_code, NULL);
				}
			}
		}

		if (NULL == error_code) {
			error_code = &_error;
			error_code->code = strdup(__MESSAGE_ADAPTOR_ERROR_NONE);
			error_code->msg = strdup("");
		}


		g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)",
				(uint64_t) atoi(__safe_add_string(error_code->code)), __safe_add_string(error_code->msg)));

		if (error_code != &_error) {
			message_adaptor_destroy_error_code(&error_code);
		} else {
			free(_error.code);
			free(_error.msg);
		}
		free(service_name);
	} else if (0 == g_strcmp0(method_name, PRIVATE_DBUS_REPLY_FORWARD_ONLINE_MESSAGE_METHOD)) {
		char *service_name = NULL;
		int64_t request_id = 0;
		int64_t chatroom_id = 0;
		bool mark_as_read = false;
		message_adaptor_error_code_t *error_code = NULL;
		message_adaptor_error_code_t _error;
		_error.code = _error.msg = NULL;

		__get_forward_online_message_res_type(parameters, &service_name, &request_id, &chatroom_id, &mark_as_read);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = __MESSAGE_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = "Can not get service context";

			g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)",
					(uint64_t) atoi(__safe_add_string(error_code->code)), __safe_add_string(error_code->msg)));

			free(service_name);
			return;
		}

		message_adaptor_h adaptor = service_adaptor_get_message_adaptor(service_adaptor);
		message_adaptor_plugin_h plugin = NULL;

		if (NULL != service->message_context) {
			plugin = message_adaptor_get_plugin_by_name(adaptor, service->message_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			if (MESSAGE_CONNECTION_POLICY_DISCONNECT == service->message_context->connection_policy) {
				service_adaptor_debug_func("Connection disconnected. Convert Auto connection");
				ret_code = service_adaptor_message_set_connection(plugin, service->message_context, MESSAGE_CONNECTION_POLICY_AUTO, &error_code);
				service_adaptor_debug("set_connection res (%d)", ret_code);
				if (NULL != error_code) {
					service_adaptor_debug("rcode : %s, msg : %s", error_code->code, error_code->msg);
					free(error_code->code);
					error_code->code = strdup(__MESSAGE_ADAPTOR_ERROR_CONNECTION);
				} else if (SERVICE_ADAPTOR_INTERNAL_ERROR_NONE != ret_code) {
					error_code = message_adaptor_create_error_code(__MESSAGE_ADAPTOR_ERROR_CONNECTION, "Server connect failed");
				}
			}
			if (NULL == error_code) {
				ret_code = message_adaptor_forward_online_message_reply(plugin, service->message_context,
						request_id, chatroom_id, mark_as_read, &error_code, NULL);

				if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
					service_adaptor_info("API returns auth-error. Try refresh auth");
					service_adaptor_auth_refresh(service_adaptor, service_name, service->message_context->plugin_uri);

					service_adaptor_debug("Empty error_code already issued (%s: %s)", error_code ? error_code->code : NULL, error_code ? error_code->msg : NULL);
					message_adaptor_destroy_error_code(&error_code);

					service_adaptor_debug("Re-try API");
					ret_code = message_adaptor_forward_online_message_reply(plugin, service->message_context,
							request_id, chatroom_id, mark_as_read, &error_code, NULL);
				}
			}
		}

		if (NULL == error_code) {
			error_code = &_error;
			error_code->code = strdup(__MESSAGE_ADAPTOR_ERROR_NONE);
			error_code->msg = strdup("");
		}

		g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)",
				(uint64_t) atoi(__safe_add_string(error_code->code)), __safe_add_string(error_code->msg)));

		if (error_code != &_error) {
			message_adaptor_destroy_error_code(&error_code);
		} else {
			free(_error.code);
			free(_error.msg);
		}
		free(service_name);
	} else if (0 == g_strcmp0(method_name, PRIVATE_DBUS_REPLY_FORWARD_UNREAD_MESSAGE_METHOD)) {
		char *service_name = NULL;
		int64_t request_id = 0;
		char *next_pagination_key = NULL;
		int max_count = 0;
		message_adaptor_error_code_t *error_code = NULL;
		message_adaptor_error_code_t _error;
		_error.code = _error.msg = NULL;

		__get_forward_unread_message_res_type(parameters, &service_name, &request_id, &next_pagination_key, &max_count);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = __MESSAGE_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = "Can not get service context";

			g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)",
					(uint64_t) atoi(__safe_add_string(error_code->code)), __safe_add_string(error_code->msg)));

			free(service_name);
			free(next_pagination_key);
			return;
		}

		message_adaptor_h adaptor = service_adaptor_get_message_adaptor(service_adaptor);
		message_adaptor_plugin_h plugin = NULL;

		if (NULL != service->message_context) {
			plugin = message_adaptor_get_plugin_by_name(adaptor, service->message_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			if (MESSAGE_CONNECTION_POLICY_DISCONNECT == service->message_context->connection_policy) {
				service_adaptor_debug_func("Connection disconnected. Convert Auto connection");
				ret_code = service_adaptor_message_set_connection(plugin, service->message_context, MESSAGE_CONNECTION_POLICY_AUTO, &error_code);
				service_adaptor_debug("set_connection res (%d)", ret_code);
				if (NULL != error_code) {
					service_adaptor_debug("rcode : %s, msg : %s", error_code->code, error_code->msg);
					free(error_code->code);
					error_code->code = strdup(__MESSAGE_ADAPTOR_ERROR_CONNECTION);
				} else if (SERVICE_ADAPTOR_INTERNAL_ERROR_NONE != ret_code) {
					error_code = message_adaptor_create_error_code(__MESSAGE_ADAPTOR_ERROR_CONNECTION, "Server connect failed");
				}
			}
			if (NULL == error_code) {
				ret_code = message_adaptor_forward_unread_message_reply(plugin, service->message_context,
						request_id, next_pagination_key, max_count, &error_code, NULL);

				if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
					service_adaptor_info("API returns auth-error. Try refresh auth");
					service_adaptor_auth_refresh(service_adaptor, service_name, service->message_context->plugin_uri);

					service_adaptor_debug("Empty error_code already issued (%s: %s)", error_code ? error_code->code : NULL, error_code ? error_code->msg : NULL);
					message_adaptor_destroy_error_code(&error_code);

					service_adaptor_debug("Re-try API");
					ret_code = message_adaptor_forward_unread_message_reply(plugin, service->message_context,
							request_id, next_pagination_key, max_count, &error_code, NULL);
				}
			}
		}

		if (NULL == error_code) {
			error_code = &_error;
			error_code->code = strdup(__MESSAGE_ADAPTOR_ERROR_NONE);
			error_code->msg = strdup("");
		}

		g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)",
				(uint64_t) atoi(__safe_add_string(error_code->code)), __safe_add_string(error_code->msg)));

		if (error_code != &_error) {
			message_adaptor_destroy_error_code(&error_code);
		} else {
			free(_error.code);
			free(_error.msg);
		}
		free(service_name);
		free(next_pagination_key);
	} else if (0 == g_strcmp0(method_name, PRIVATE_DBUS_REQUEST_READ_MESSAGE_METHOD)) {
		char *service_name = NULL;
		int64_t request_id = 0;
		int64_t chatroom_id = 0;
		message_adaptor_inbox_message_s *inbox_msg = NULL;
		message_adaptor_error_code_t *error_code = NULL;
		message_adaptor_error_code_t _error;
		_error.code = _error.msg = NULL;

		__get_read_message_req_type(parameters, &service_name, &request_id, &chatroom_id, &inbox_msg);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = __MESSAGE_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = "Can not get service context";

			g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)",
					(uint64_t) atoi(__safe_add_string(error_code->code)), __safe_add_string(error_code->msg)));

			free(service_name);
			if (NULL != inbox_msg) {
				free(inbox_msg->chatMsg);
				free(inbox_msg);
			}
			return;
		}

		message_adaptor_h adaptor = service_adaptor_get_message_adaptor(service_adaptor);
		message_adaptor_plugin_h plugin = NULL;

		if (NULL != service->message_context) {
			plugin = message_adaptor_get_plugin_by_name(adaptor, service->message_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			if (MESSAGE_CONNECTION_POLICY_DISCONNECT == service->message_context->connection_policy) {
				service_adaptor_debug_func("Connection disconnected. Convert Auto connection");
				ret_code = service_adaptor_message_set_connection(plugin, service->message_context, MESSAGE_CONNECTION_POLICY_AUTO, &error_code);
				service_adaptor_debug("set_connection res (%d)", ret_code);
				if (NULL != error_code) {
					service_adaptor_debug("rcode : %s, msg : %s", error_code->code, error_code->msg);
					free(error_code->code);
					error_code->code = strdup(__MESSAGE_ADAPTOR_ERROR_CONNECTION);
				} else if (SERVICE_ADAPTOR_INTERNAL_ERROR_NONE != ret_code) {
					error_code = message_adaptor_create_error_code(__MESSAGE_ADAPTOR_ERROR_CONNECTION, "Server connect failed");
				}
			}
			if (NULL == error_code) {
				ret_code = message_adaptor_read_message_request(plugin, service->message_context,
						request_id, chatroom_id, inbox_msg, &error_code, NULL);

				if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
					service_adaptor_info("API returns auth-error. Try refresh auth");
					service_adaptor_auth_refresh(service_adaptor, service_name, service->message_context->plugin_uri);

					service_adaptor_debug("Empty error_code already issued (%s: %s)", error_code ? error_code->code : NULL, error_code ? error_code->msg : NULL);
					message_adaptor_destroy_error_code(&error_code);

					service_adaptor_debug("Re-try API");
					ret_code = message_adaptor_read_message_request(plugin, service->message_context,
							request_id, chatroom_id, inbox_msg, &error_code, NULL);
				}
			}
		}

		if (NULL == error_code) {
			error_code = &_error;
			error_code->code = strdup(__MESSAGE_ADAPTOR_ERROR_NONE);
			error_code->msg = strdup("");
		}

		g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)",
				(uint64_t) atoi(__safe_add_string(error_code->code)), __safe_add_string(error_code->msg)));

		if (error_code != &_error) {
			message_adaptor_destroy_error_code(&error_code);
		} else {
			free(_error.code);
			free(_error.msg);
		}
		free(service_name);
		if (NULL != inbox_msg) {
			free(inbox_msg->chatMsg);
			free(inbox_msg);
		}
	} else if (0 == g_strcmp0(method_name, PRIVATE_DBUS_REQUEST_INVITE_CHAT_METHOD)) {
		char *service_name = NULL;
		int64_t request_id = 0;
		int64_t chatroom_id = 0;
		int64_t *inviting_members = NULL;
		unsigned int inviting_members_len = 0;
		message_adaptor_error_code_t *error_code = NULL;
		message_adaptor_error_code_t _error;
		_error.code = _error.msg = NULL;

		__get_invite_chat_req_type(parameters, &service_name, &request_id,
				&chatroom_id, &inviting_members, &inviting_members_len);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = __MESSAGE_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = "Can not get service context";

			g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)",
					(uint64_t) atoi(__safe_add_string(error_code->code)), __safe_add_string(error_code->msg)));

			free(service_name);
			free(inviting_members);
			return;
		}

		message_adaptor_h adaptor = service_adaptor_get_message_adaptor(service_adaptor);
		message_adaptor_plugin_h plugin = NULL;

		if (NULL != service->message_context) {
			plugin = message_adaptor_get_plugin_by_name(adaptor, service->message_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			if (MESSAGE_CONNECTION_POLICY_DISCONNECT == service->message_context->connection_policy) {
				service_adaptor_debug_func("Connection disconnected. Convert Auto connection");
				ret_code = service_adaptor_message_set_connection(plugin, service->message_context, MESSAGE_CONNECTION_POLICY_AUTO, &error_code);
				service_adaptor_debug("set_connection res (%d)", ret_code);
				if (NULL != error_code) {
					service_adaptor_debug("rcode : %s, msg : %s", error_code->code, error_code->msg);
					free(error_code->code);
					error_code->code = strdup(__MESSAGE_ADAPTOR_ERROR_CONNECTION);
				} else if (SERVICE_ADAPTOR_INTERNAL_ERROR_NONE != ret_code) {
					error_code = message_adaptor_create_error_code(__MESSAGE_ADAPTOR_ERROR_CONNECTION, "Server connect failed");
				}
			}
			if (NULL == error_code) {
				ret_code = message_adaptor_invite_chat_request(plugin, service->message_context,
						request_id, chatroom_id, inviting_members, inviting_members_len, &error_code, NULL);

				if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
					service_adaptor_info("API returns auth-error. Try refresh auth");
					service_adaptor_auth_refresh(service_adaptor, service_name, service->message_context->plugin_uri);

					service_adaptor_debug("Empty error_code already issued (%s: %s)", error_code ? error_code->code : NULL, error_code ? error_code->msg : NULL);
					message_adaptor_destroy_error_code(&error_code);

					service_adaptor_debug("Re-try API");
					ret_code = message_adaptor_invite_chat_request(plugin, service->message_context,
							request_id, chatroom_id, inviting_members, inviting_members_len, &error_code, NULL);
				}
			}
		}

		if (NULL == error_code) {
			error_code = &_error;
			error_code->code = strdup(__MESSAGE_ADAPTOR_ERROR_NONE);
			error_code->msg = strdup("");
		}

		g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)",
				(uint64_t) atoi(__safe_add_string(error_code->code)), __safe_add_string(error_code->msg)));

		if (error_code != &_error) {
			message_adaptor_destroy_error_code(&error_code);
		} else {
			free(_error.code);
			free(_error.msg);
		}

		free(service_name);
		free(inviting_members);
	} else if (0 == g_strcmp0(method_name, PRIVATE_DBUS_REQUEST_END_CHAT_METHOD)) {
		char *service_name = NULL;
		int64_t request_id = 0;
		message_adaptor_end_chat_s **end_chats = NULL;
		unsigned int end_chats_len = 0;
		message_adaptor_error_code_t *error_code = NULL;
		message_adaptor_error_code_t _error;
		_error.code = _error.msg = NULL;

		__get_end_chat_req_type(parameters, &service_name, &request_id, &end_chats, &end_chats_len);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = __MESSAGE_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = "Can not get service context";

			g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)",
					(uint64_t) atoi(__safe_add_string(error_code->code)), __safe_add_string(error_code->msg)));

			free(service_name);
			if ((NULL != end_chats) && (0U < end_chats_len)) {
				for (int i = 0; i < end_chats_len; i++) {
					free(end_chats[i]);
				}
				free(end_chats);
			}
			return;
		}

		message_adaptor_h adaptor = service_adaptor_get_message_adaptor(service_adaptor);
		message_adaptor_plugin_h plugin = NULL;

		if (NULL != service->message_context) {
			plugin = message_adaptor_get_plugin_by_name(adaptor, service->message_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			if (MESSAGE_CONNECTION_POLICY_DISCONNECT == service->message_context->connection_policy) {
				service_adaptor_debug_func("Connection disconnected. Convert Auto connection");
				ret_code = service_adaptor_message_set_connection(plugin, service->message_context, MESSAGE_CONNECTION_POLICY_AUTO, &error_code);
				service_adaptor_debug("set_connection res (%d)", ret_code);
				if (NULL != error_code) {
					service_adaptor_debug("rcode : %s, msg : %s", error_code->code, error_code->msg);
					free(error_code->code);
					error_code->code = strdup(__MESSAGE_ADAPTOR_ERROR_CONNECTION);
				} else if (SERVICE_ADAPTOR_INTERNAL_ERROR_NONE != ret_code) {
					error_code = message_adaptor_create_error_code(__MESSAGE_ADAPTOR_ERROR_CONNECTION, "Server connect failed");
				}
			}
			if (NULL == error_code) {
				ret_code = message_adaptor_end_chat_request(plugin, service->message_context,
						request_id, end_chats, end_chats_len, &error_code, NULL);

				if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
					service_adaptor_info("API returns auth-error. Try refresh auth");
					service_adaptor_auth_refresh(service_adaptor, service_name, service->message_context->plugin_uri);

					service_adaptor_debug("Empty error_code already issued (%s: %s)", error_code ? error_code->code : NULL, error_code ? error_code->msg : NULL);
					message_adaptor_destroy_error_code(&error_code);

					service_adaptor_debug("Re-try API");
					ret_code = message_adaptor_end_chat_request(plugin, service->message_context,
							request_id, end_chats, end_chats_len, &error_code, NULL);
				}
			}
		}

		if (NULL == error_code) {
			error_code = &_error;
			error_code->code = strdup(__MESSAGE_ADAPTOR_ERROR_NONE);
			error_code->msg = strdup("");
		}

		g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)",
				(uint64_t) atoi(__safe_add_string(error_code->code)), __safe_add_string(error_code->msg)));

		if (error_code != &_error) {
			message_adaptor_destroy_error_code(&error_code);
		} else {
			free(_error.code);
			free(_error.msg);
		}
		free(service_name);
		if ((NULL != end_chats) && (0U < end_chats_len)) {
			for (int i = 0; i < end_chats_len; i++) {
				free(end_chats[i]);
			}
			free(end_chats);
		}
	} else if (0 == g_strcmp0(method_name, PRIVATE_DBUS_REQUEST_UNSEAL_MESSAGE_METHOD)) {
		char *service_name = NULL;
		int64_t request_id = 0;
		int64_t chatroom_id = 0;
		int64_t sender_id = 0;
		int64_t message_id = 0;
		char *message_detail = NULL;
		message_adaptor_error_code_t *error_code = NULL;
		message_adaptor_error_code_t _error;
		_error.code = _error.msg = NULL;

		__get_unseal_message_req_type(parameters, &service_name, &request_id, &chatroom_id, &sender_id, &message_id, &message_detail);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = __MESSAGE_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = "Can not get service context";

			g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)", (uint64_t) atoi(__safe_add_string(error_code->code)), __safe_add_string(error_code->msg)));

			free(service_name);
			free(message_detail);
			return;
		}

		message_adaptor_h adaptor = service_adaptor_get_message_adaptor(service_adaptor);
		message_adaptor_plugin_h plugin = NULL;

		if (NULL != service->message_context) {
			plugin = message_adaptor_get_plugin_by_name(adaptor, service->message_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			if (MESSAGE_CONNECTION_POLICY_DISCONNECT == service->message_context->connection_policy) {
				service_adaptor_debug_func("Connection disconnected. Convert Auto connection");
				ret_code = service_adaptor_message_set_connection(plugin, service->message_context, MESSAGE_CONNECTION_POLICY_AUTO, &error_code);
				service_adaptor_debug("set_connection res (%d)", ret_code);
				if (NULL != error_code) {
					service_adaptor_debug("rcode : %s, msg : %s", error_code->code, error_code->msg);
					free(error_code->code);
					error_code->code = strdup(__MESSAGE_ADAPTOR_ERROR_CONNECTION);
				} else if (SERVICE_ADAPTOR_INTERNAL_ERROR_NONE != ret_code) {
					error_code = message_adaptor_create_error_code(__MESSAGE_ADAPTOR_ERROR_CONNECTION, "Server connect failed");
				}
			}
			if (NULL == error_code) {
				ret_code = message_adaptor_unseal_message_request(plugin, service->message_context,
						request_id, chatroom_id, sender_id, message_id, message_detail, &error_code, NULL);
			}
		}

		if (NULL == error_code) {
			error_code = &_error;
			error_code->code = strdup(__MESSAGE_ADAPTOR_ERROR_NONE);
			error_code->msg = strdup("");
		}

		g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)", (uint64_t) atoi(__safe_add_string(error_code->code)), __safe_add_string(error_code->msg)));

		if (error_code != &_error) {
			message_adaptor_destroy_error_code(&error_code);
		} else {
			free(_error.code);
			free(_error.msg);
		}
		free(service_name);
		free(message_detail);
	} else if (0 == g_strcmp0(method_name, PRIVATE_DBUS_REQUEST_SAVE_CALL_LOG_METHOD)) {
		char *service_name = NULL;
		int64_t request_id = 0;
		int64_t chatroom_id = 0;
		char *call_id = NULL;
		char *call_log_type = NULL;
		int64_t call_sender_id = 0;
		int64_t call_receiver_id = 0;
		int conversaction_second = 0;
		message_adaptor_error_code_t *error_code = NULL;
		message_adaptor_error_code_t _error;
		_error.code = _error.msg = NULL;

		__get_save_call_log_req_type(parameters, &service_name, &request_id, &chatroom_id, &call_id, &call_log_type, &call_sender_id, &call_receiver_id, &conversaction_second);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = __MESSAGE_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = "Can not get service context";

			g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)", (uint64_t) atoi(__safe_add_string(error_code->code)), __safe_add_string(error_code->msg)));

			free(service_name);
			free(call_id);
			free(call_log_type);
			return;
		}

		message_adaptor_h adaptor = service_adaptor_get_message_adaptor(service_adaptor);
		message_adaptor_plugin_h plugin = NULL;

		if (NULL != service->message_context) {
			plugin = message_adaptor_get_plugin_by_name(adaptor, service->message_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			if (MESSAGE_CONNECTION_POLICY_DISCONNECT == service->message_context->connection_policy) {
				service_adaptor_debug_func("Connection disconnected. Convert Auto connection");
				ret_code = service_adaptor_message_set_connection(plugin, service->message_context, MESSAGE_CONNECTION_POLICY_AUTO, &error_code);
				service_adaptor_debug("set_connection res (%d)", ret_code);
				if (NULL != error_code) {
					service_adaptor_debug("rcode : %s, msg : %s", error_code->code, error_code->msg);
					free(error_code->code);
					error_code->code = strdup(__MESSAGE_ADAPTOR_ERROR_CONNECTION);
				} else if (SERVICE_ADAPTOR_INTERNAL_ERROR_NONE != ret_code) {
					error_code = message_adaptor_create_error_code(__MESSAGE_ADAPTOR_ERROR_CONNECTION, "Server connect failed");
				}
			}
			if (NULL == error_code) {
				ret_code = message_adaptor_save_call_log_request(plugin, service->message_context, request_id, chatroom_id,
						call_id, call_log_type, call_sender_id, call_receiver_id, conversaction_second, &error_code, NULL);
			}
		}

		if (NULL == error_code) {
			error_code = &_error;
			error_code->code = strdup(__MESSAGE_ADAPTOR_ERROR_NONE);
			error_code->msg = strdup("");
		}

		g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)", (uint64_t) atoi(__safe_add_string(error_code->code)), __safe_add_string(error_code->msg)));

		if (error_code != &_error) {
			message_adaptor_destroy_error_code(&error_code);
		} else {
			free(_error.code);
			free(_error.msg);
		}
		free(service_name);
		free(call_id);
		free(call_log_type);
	} else if (0 == g_strcmp0(method_name, PRIVATE_DBUS_REQUEST_CURRENT_TIME_METHOD)) {
		char *service_name = NULL;
		int64_t request_id = 0;
		message_adaptor_error_code_t *error_code = NULL;
		message_adaptor_error_code_t _error;
		_error.code = _error.msg = NULL;

		__get_current_time_req_type(parameters, &service_name, &request_id);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = __MESSAGE_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = "Can not get service context";

			g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)",
					(uint64_t) atoi(__safe_add_string(error_code->code)), __safe_add_string(error_code->msg)));

			free(service_name);
			return;
		}

		message_adaptor_h adaptor = service_adaptor_get_message_adaptor(service_adaptor);
		message_adaptor_plugin_h plugin = NULL;

		if (NULL != service->message_context) {
			plugin = message_adaptor_get_plugin_by_name(adaptor, service->message_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			if (MESSAGE_CONNECTION_POLICY_DISCONNECT == service->message_context->connection_policy) {
				service_adaptor_debug_func("Connection disconnected. Convert Auto connection");
				ret_code = service_adaptor_message_set_connection(plugin, service->message_context, MESSAGE_CONNECTION_POLICY_AUTO, &error_code);
				service_adaptor_debug("set_connection res (%d)", ret_code);
				if (NULL != error_code) {
					service_adaptor_debug("rcode : %s, msg : %s", error_code->code, error_code->msg);
					free(error_code->code);
					error_code->code = strdup(__MESSAGE_ADAPTOR_ERROR_CONNECTION);
				} else if (SERVICE_ADAPTOR_INTERNAL_ERROR_NONE != ret_code) {
					error_code = message_adaptor_create_error_code(__MESSAGE_ADAPTOR_ERROR_CONNECTION, "Server connect failed");
				}
			}
			if (NULL == error_code) {
				ret_code = message_adaptor_current_time_request(plugin, service->message_context,
						request_id, &error_code, NULL);

				if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
					service_adaptor_info("API returns auth-error. Try refresh auth");
					service_adaptor_auth_refresh(service_adaptor, service_name, service->message_context->plugin_uri);

					service_adaptor_debug("Empty error_code already issued (%s: %s)", error_code ? error_code->code : NULL, error_code ? error_code->msg : NULL);
					message_adaptor_destroy_error_code(&error_code);

					service_adaptor_debug("Re-try API");
					ret_code = message_adaptor_current_time_request(plugin, service->message_context,
							request_id, &error_code, NULL);
				}
			}
		}

		if (NULL == error_code) {
			error_code = &_error;
			error_code->code = strdup(__MESSAGE_ADAPTOR_ERROR_NONE);
			error_code->msg = strdup("");
		}

		g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)",
				(uint64_t) atoi(__safe_add_string(error_code->code)), __safe_add_string(error_code->msg)));

		if (error_code != &_error) {
			message_adaptor_destroy_error_code(&error_code);
		} else {
			free(_error.code);
			free(_error.msg);
		}
		free(service_name);
	} else if (0 == g_strcmp0(method_name, PRIVATE_DBUS_REQUEST_GET_CONNECTION_POLICY_METHOD)) {
		char *service_name = NULL;
		message_adaptor_error_code_t *error_code = NULL;
		message_adaptor_error_code_t _error;
		_error.code = _error.msg = NULL;

		__get_get_connection_policy_req_type(parameters, &service_name);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if ((NULL == service) || (NULL == service->message_context)) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = __MESSAGE_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = "Can not get service context";

			g_dbus_method_invocation_return_value(invocation, g_variant_new(MAKE_RETURN_TYPE(private_service_adaptor_connection_policy_res_s_type),
					(int32_t) MESSAGE_CONNECTION_POLICY_DISCONNECT, (uint64_t) atoi(__safe_add_string(error_code->code)), __safe_add_string(error_code->msg)));

			free(service_name);
			return;
		}

		if (NULL == error_code) {
			error_code = &_error;
			error_code->code = strdup(__MESSAGE_ADAPTOR_ERROR_NONE);
			error_code->msg = strdup("");
		}

		g_dbus_method_invocation_return_value(invocation, g_variant_new(MAKE_RETURN_TYPE(private_service_adaptor_connection_policy_res_s_type),
				(int32_t) service->message_context->connection_policy, (uint64_t) atoi(__safe_add_string(error_code->code)), __safe_add_string(error_code->msg)));

		if (error_code != &_error) {
			message_adaptor_destroy_error_code(&error_code);
		} else {
			free(_error.code);
			free(_error.msg);
		}
		free(service_name);
	} else if (0 == g_strcmp0(method_name, PRIVATE_DBUS_REQUEST_SET_CONNECTION_POLICY_METHOD)) {
		char *service_name = NULL;
		int policy = -1;
		message_adaptor_error_code_t *error_code = NULL;
		message_adaptor_error_code_t _error;
		_error.code = _error.msg = NULL;

		__get_set_connection_policy_req_type(parameters, &service_name, &policy);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if ((NULL == service) || (NULL == service->message_context)) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = __MESSAGE_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = "Can not get service context";

			g_dbus_method_invocation_return_value(invocation, g_variant_new(MAKE_RETURN_TYPE(private_service_adaptor_connection_policy_res_s_type),
					(int32_t) MESSAGE_CONNECTION_POLICY_DISCONNECT, (uint64_t) atoi(__safe_add_string(error_code->code)), __safe_add_string(error_code->msg)));

			free(service_name);
			return;
		}

		service_adaptor_debug("get message_adaptor");
		message_adaptor_h adaptor = service_adaptor_get_message_adaptor(service_adaptor);
		message_adaptor_plugin_h plugin = NULL;

		if (NULL != service->message_context) {
			plugin = message_adaptor_get_plugin_by_name(adaptor, service->message_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			service_adaptor_debug("set message connection (%d)", policy);
			ret_code = service_adaptor_message_set_connection(plugin, service->message_context,
					policy, &error_code);
			service_adaptor_debug("res (%d), policy (%d)", ret_code, policy);
		}

		if (NULL == error_code) {
			error_code = &_error;
			error_code->code = strdup(__MESSAGE_ADAPTOR_ERROR_NONE);
			error_code->msg = strdup("");
		}

		g_dbus_method_invocation_return_value(invocation, g_variant_new(MAKE_RETURN_TYPE(private_service_adaptor_connection_policy_res_s_type),
				(int32_t) service->message_context->connection_policy, (uint64_t) atoi(__safe_add_string(error_code->code)), __safe_add_string(error_code->msg)));

		if (error_code != &_error) {
			message_adaptor_destroy_error_code(&error_code);
		} else {
			free(_error.code);
			free(_error.msg);
		}
		free(service_name);
	} else if (0 == g_strcmp0(method_name, PRIVATE_DBUS_GET_CHAT_ID_LIST_METHOD)) {
		char *service_name = NULL;
		message_adaptor_phone_number_s **phone_number = NULL;
		unsigned int phone_number_len = 0;
		message_adaptor_chat_id_s **chat_ids = NULL;
		unsigned int chat_ids_len = 0;
		void *user_data = NULL;
		void *server_data = NULL;
		message_adaptor_error_code_t *error_code = NULL;
		message_adaptor_error_code_t _error;
		_error.code = _error.msg = NULL;

		__get_chat_id_list_req_type(parameters, &service_name, &phone_number, &phone_number_len);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = __MESSAGE_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = "Can not get service context";

			GVariant *response = __create_chat_id_list_res_type(chat_ids, chat_ids_len, error_code);
			g_dbus_method_invocation_return_value(invocation, response);

			free(service_name);
			if ((NULL != phone_number) && (0U < phone_number_len)) {
				for (int pi = 0; pi < phone_number_len; pi++) {
					if (NULL != phone_number[pi]) {
						free(phone_number[pi]->phonenumber);
						free(phone_number[pi]->ccc);
						free(phone_number[pi]);
					}
				}
				free(phone_number);
			}

			return;
		}

		message_adaptor_h adaptor = service_adaptor_get_message_adaptor(service_adaptor);
		message_adaptor_plugin_h plugin = NULL;

		if (NULL != service->message_context) {
			plugin = message_adaptor_get_plugin_by_name(adaptor, service->message_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			ret_code = message_adaptor_request_chat_id(plugin, service->message_context,
					phone_number, phone_number_len, user_data,
					&chat_ids, &chat_ids_len, &error_code, &server_data);

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->message_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%s: %s)", error_code ? error_code->code : NULL, error_code ? error_code->msg : NULL);
				message_adaptor_destroy_error_code(&error_code);

				service_adaptor_debug("Re-try API");
				ret_code = message_adaptor_request_chat_id(plugin, service->message_context,
						phone_number, phone_number_len, user_data,
						&chat_ids, &chat_ids_len, &error_code, &server_data);
			}
		}

		if (NULL == error_code) {
			error_code = &_error;
			error_code->code = strdup(__MESSAGE_ADAPTOR_ERROR_NONE);
			error_code->msg = strdup("");
		}

		GVariant *response = __create_chat_id_list_res_type(chat_ids, chat_ids_len, error_code);
		g_dbus_method_invocation_return_value(invocation, response);

		if (error_code != &_error) {
			message_adaptor_destroy_error_code(&error_code);
		} else {
			free(_error.code);
			free(_error.msg);
		}

		free(service_name);
		if ((NULL != phone_number) && (0U < phone_number_len)) {
			for (int pi = 0; pi < phone_number_len; pi++) {
				if (NULL != phone_number[pi]) {
					free(phone_number[pi]->phonenumber);
					free(phone_number[pi]->ccc);
					free(phone_number[pi]);
				}
			}
			free(phone_number);
		}
		if ((NULL != chat_ids) && (0U < chat_ids_len)) {
			for (int ci = 0; ci < chat_ids_len; ci++) {
				if (NULL != chat_ids[ci]) {
					free(chat_ids[ci]->msisdn);
					free(chat_ids[ci]);
				}
			}
			free(chat_ids);
		}
	} else if (0 == g_strcmp0(method_name, PRIVATE_DBUS_GET_MSISDN_LIST_METHOD)) {
		char *service_name = NULL;
		int64_t *chat_ids = NULL;
		unsigned int chat_ids_len = 0;
		message_adaptor_chat_id_s **msisdns = NULL;
		unsigned int msisdns_len = 0;
		void *user_data = NULL;
		void *server_data = NULL;
		message_adaptor_error_code_t *error_code = NULL;
		message_adaptor_error_code_t _error;
		_error.code = _error.msg = NULL;

		__get_msisdn_list_req_type(parameters, &service_name, &chat_ids, &chat_ids_len);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = __MESSAGE_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = "Can not get service context";

			g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)",
					(uint64_t) atoi(__safe_add_string(error_code->code)), __safe_add_string(error_code->msg)));

			free(service_name);
			free(chat_ids);
			return;
		}

		message_adaptor_h adaptor = service_adaptor_get_message_adaptor(service_adaptor);
		message_adaptor_plugin_h plugin = NULL;

		if (NULL != service->message_context) {
			plugin = message_adaptor_get_plugin_by_name(adaptor, service->message_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
			ret_code = message_adaptor_request_msisdn(plugin, service->message_context,
					chat_ids, chat_ids_len, user_data,
					&msisdns, &msisdns_len, &error_code, &server_data);

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->message_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%s: %s)", error_code ? error_code->code : NULL, error_code ? error_code->msg : NULL);
				message_adaptor_destroy_error_code(&error_code);

				service_adaptor_debug("Re-try API");
				ret_code = message_adaptor_request_msisdn(plugin, service->message_context,
						chat_ids, chat_ids_len, user_data,
						&msisdns, &msisdns_len, &error_code, &server_data);
			}
		}

		if (NULL == error_code) {
			error_code = &_error;
			error_code->code = strdup(__MESSAGE_ADAPTOR_ERROR_NONE);
			error_code->msg = strdup("");
		}

		GVariant *response = __create_chat_id_list_res_type(msisdns, msisdns_len, error_code);
		g_dbus_method_invocation_return_value(invocation, response);

		if (error_code != &_error) {
			message_adaptor_destroy_error_code(&error_code);
		} else {
			free(_error.code);
			free(_error.msg);
		}

		free(service_name);
		free(chat_ids);
		if ((NULL != msisdns) && (0U < msisdns_len)) {
			for (int i = 0; i < msisdns_len; i++) {
				free(msisdns[i]->msisdn);
				free(msisdns[i]);
			}
			free(msisdns);
		}
	}
}

GVariant *__create_create_chatroom_res_type(int64_t request_id,
						int64_t chatroom_id,
						int default_message_ttl,
						message_adaptor_wrong_receiver_s *wrong_receiver,
						message_adaptor_error_code_t *error_code,
						void *server_data)
{
	message_adaptor_wrong_receiver_s _wrong_receiver;
	_wrong_receiver.invalid_receivers = NULL;
	_wrong_receiver.invalid_receivers_len = 0;
	_wrong_receiver.interrupted_receivers = NULL;
	_wrong_receiver.interrupted_receivers_len = 0;
	_wrong_receiver.disabled_receivers = NULL;
	_wrong_receiver.disabled_receivers_len = 0;
	_wrong_receiver.existing_chatmember = NULL;
	_wrong_receiver.existing_chatmembers_len = 0;
	_wrong_receiver.did_violation_users = NULL;
	_wrong_receiver.did_violation_users_len = 0;
	_wrong_receiver.invitation_denieds = NULL;
	_wrong_receiver.invitation_denieds_len = 0;

	if (NULL == wrong_receiver) {
		wrong_receiver = &_wrong_receiver;
	}

	GVariantBuilder *builder_invalid = g_variant_builder_new(G_VARIANT_TYPE("a(x)"));

	if (wrong_receiver->invalid_receivers) {
		for (gsize j = 0; j < wrong_receiver->invalid_receivers_len; j++) {
			g_variant_builder_add(builder_invalid, "(x)", wrong_receiver->invalid_receivers[j]);
		}
	}

	GVariantBuilder *builder_interrupted = g_variant_builder_new(G_VARIANT_TYPE("a(x)"));

	if (wrong_receiver->interrupted_receivers) {
		for (gsize j = 0; j < wrong_receiver->interrupted_receivers_len; j++) {
			g_variant_builder_add(builder_interrupted, "(x)", wrong_receiver->interrupted_receivers[j]);
		}
	}

	GVariantBuilder *builder_disabled = g_variant_builder_new(G_VARIANT_TYPE("a(x)"));

	if (wrong_receiver->disabled_receivers) {
		for (gsize j = 0; j < wrong_receiver->disabled_receivers_len; j++) {
			g_variant_builder_add(builder_disabled, "(x)", wrong_receiver->disabled_receivers[j]);
		}
	}

	GVariantBuilder *builder_existing = g_variant_builder_new(G_VARIANT_TYPE("a(x)"));

	if (wrong_receiver->existing_chatmember) {
		for (gsize j = 0; j < wrong_receiver->existing_chatmembers_len; j++) {
			g_variant_builder_add(builder_existing, "(x)", wrong_receiver->existing_chatmember[j]);
		}
	}

	GVariantBuilder *builder_did_violation_users = g_variant_builder_new(G_VARIANT_TYPE(private_message_did_violation_users_list_type));

	if (wrong_receiver->did_violation_users) {
		for (gsize j = 0; j < wrong_receiver->did_violation_users_len; j++) {
			g_variant_builder_open(builder_did_violation_users, G_VARIANT_TYPE(private_service_adaptor_did_violation_users_s_type));
			g_variant_builder_add(builder_did_violation_users, "x", wrong_receiver->did_violation_users[j].usera);
			g_variant_builder_add(builder_did_violation_users, "x", wrong_receiver->did_violation_users[j].userb);
			g_variant_builder_close(builder_did_violation_users);
		}
	}

	GVariantBuilder *builder_invitation = g_variant_builder_new(G_VARIANT_TYPE("a(x)"));

	if (wrong_receiver->invitation_denieds) {
		for (gsize j = 0; j < wrong_receiver->invitation_denieds_len; j++) {
			g_variant_builder_add(builder_invitation, "(x)", wrong_receiver->invitation_denieds[j]);
		}
	}

	GVariant *response = g_variant_new(MAKE_RETURN_TYPE(private_service_adaptor_create_chatroom_res_s_type), request_id,
			chatroom_id, default_message_ttl, builder_invalid, wrong_receiver->invalid_receivers_len,
			builder_interrupted, wrong_receiver->interrupted_receivers_len,
			builder_disabled, wrong_receiver->disabled_receivers_len,
			builder_existing, wrong_receiver->existing_chatmembers_len,
			builder_did_violation_users, wrong_receiver->did_violation_users_len,
			builder_invitation, wrong_receiver->invitation_denieds_len,
			(uint64_t) atoi(__safe_add_string(error_code->code)), __safe_add_string(error_code->msg));

	g_variant_builder_unref(builder_invalid);
	g_variant_builder_unref(builder_interrupted);
	g_variant_builder_unref(builder_disabled);
	g_variant_builder_unref(builder_existing);
	g_variant_builder_unref(builder_did_violation_users);

	return response;
}

GVariant *__create_change_chatroom_meta_res_type(int64_t request_id,
						int64_t chatroom_id,
						message_adaptor_error_code_t *error_code,
						void *server_data)
{
	GVariant *response = g_variant_new(MAKE_RETURN_TYPE(private_service_adaptor_change_chatroom_meta_res_s_type), request_id, chatroom_id, (uint64_t) atoi(__safe_add_string(error_code->code)), __safe_add_string(error_code->msg));

	return response;
}

GVariant *__create_chat_res_type(int64_t request_id,
						int64_t chatroom_id,
						message_adaptor_processed_msg_s **processed_msgs,
						unsigned int processed_msgs_len,
						message_adaptor_error_code_t *error_code,
						void *server_data)
{
	if (NULL == processed_msgs) {
		processed_msgs_len = 0;
	}

	GVariantBuilder *builder = g_variant_builder_new(G_VARIANT_TYPE(private_message_processed_msg_list_type));

	for (gsize i = 0; i < processed_msgs_len; i++) {
		g_variant_builder_open(builder, G_VARIANT_TYPE(private_service_adaptor_processed_msg_s_type));
		g_variant_builder_add(builder, "x", processed_msgs[i]->msg_id);
		g_variant_builder_add(builder, "x", processed_msgs[i]->sent_time);
		g_variant_builder_close(builder);
	}

	GVariant *response = g_variant_new(MAKE_RETURN_TYPE(private_service_adaptor_chat_res_s_type), request_id,
			chatroom_id, builder, processed_msgs_len,
			(uint64_t) atoi(__safe_add_string(error_code->code)), __safe_add_string(error_code->msg));

	g_variant_builder_unref(builder);

	return response;
}

GVariant *__create_allow_chat_res_type(int64_t request_id,
						int64_t chatroom_id,
						message_adaptor_delivery_ack_s **delivery_acks,
						unsigned int delivery_acks_len,
						int64_t last_delivery_ack_timestamp,
						message_adaptor_read_ack_s **read_acks,
						unsigned int read_acks_len,
						int64_t last_read_ack_timestamp,
						message_adaptor_ordered_chat_member_s **ordered_chat_members,
						unsigned int ordered_chat_members_len,
						const char *chatroom_title,
						int default_message_ttl,
						message_adaptor_error_code_t *error_code,
						void *server_data)
{
	if (NULL == delivery_acks) {
		delivery_acks_len = 0;
	}

	if (NULL == read_acks) {
		read_acks_len = 0;
	}

	if (NULL == ordered_chat_members) {
		ordered_chat_members_len = 0;
	}

	GVariantBuilder *builder_delivery_ack = g_variant_builder_new(G_VARIANT_TYPE(private_message_delivery_ack_list_type));

	for (gsize i = 0; i < delivery_acks_len; i++) {
		g_variant_builder_open(builder_delivery_ack, G_VARIANT_TYPE(private_service_adaptor_delivery_ack_s_type));
		g_variant_builder_add(builder_delivery_ack, "x", delivery_acks[i]->userId);
		g_variant_builder_add(builder_delivery_ack, "x", delivery_acks[i]->msgId);
		g_variant_builder_add(builder_delivery_ack, "t", delivery_acks[i]->timestamp);
		g_variant_builder_close(builder_delivery_ack);
	}

	GVariantBuilder *builder_read_ack = g_variant_builder_new(G_VARIANT_TYPE(private_message_read_ack_list_type));

	for (gsize i = 0; i < read_acks_len; i++) {
		g_variant_builder_open(builder_read_ack, G_VARIANT_TYPE(private_service_adaptor_read_ack_s_type));
		g_variant_builder_add(builder_read_ack, "x", read_acks[i]->userId);
		g_variant_builder_add(builder_read_ack, "x", read_acks[i]->msgId);
		g_variant_builder_add(builder_read_ack, "t", read_acks[i]->timestamp);
		g_variant_builder_close(builder_read_ack);
	}

	GVariantBuilder *builder_ordered_chat_member = g_variant_builder_new(G_VARIANT_TYPE(private_message_ordered_chat_member_list_type));

	for (gsize i = 0; i < ordered_chat_members_len; i++) {
		g_variant_builder_open(builder_ordered_chat_member, G_VARIANT_TYPE(private_service_adaptor_ordered_chat_member_s_type));
		g_variant_builder_add(builder_ordered_chat_member, "x", ordered_chat_members[i]->userId);
		g_variant_builder_add(builder_ordered_chat_member, "b", ordered_chat_members[i]->available);
		g_variant_builder_add(builder_ordered_chat_member, "s", ordered_chat_members[i]->name);
		g_variant_builder_close(builder_ordered_chat_member);
	}

	GVariant *response = g_variant_new(MAKE_RETURN_TYPE(private_service_adaptor_allow_chat_res_s_type), request_id,
			chatroom_id, builder_delivery_ack, delivery_acks_len, last_delivery_ack_timestamp,
			builder_read_ack, read_acks_len, last_read_ack_timestamp,
			builder_ordered_chat_member, ordered_chat_members_len,
			__safe_add_string(chatroom_title), default_message_ttl,
			(uint64_t) atoi(__safe_add_string(error_code->code)), __safe_add_string(error_code->msg));

	g_variant_builder_unref(builder_delivery_ack);
	g_variant_builder_unref(builder_read_ack);
	g_variant_builder_unref(builder_ordered_chat_member);

	return response;
}

GVariant *__create_all_unread_message_res_type(int64_t request_id,
						message_adaptor_error_code_t *error_code,
						void *server_data)
{
	GVariant *response = g_variant_new(MAKE_RETURN_TYPE(private_service_adaptor_all_unread_message_res_s_type), request_id,
			(uint64_t) atoi(__safe_add_string(error_code->code)), __safe_add_string(error_code->msg));

	return response;
}

GVariant *__create_forward_online_message_req_type(int64_t request_id,
						int64_t chatroom_id,
						int chat_type,
						message_adaptor_inbox_message_s *inbox_msg,
						bool skip_reply,
						message_adaptor_error_code_t *error_code,
						void *server_data)
{
	service_adaptor_info("%s %s %d", __FUNCTION__, __FILE__, __LINE__);

	message_adaptor_inbox_message_s _inbox_msg;
	_inbox_msg.chatMsg = "";
	if (NULL == inbox_msg) {
		inbox_msg = &_inbox_msg;
	}

	GVariant *response = g_variant_new(MAKE_RETURN_TYPE(private_service_adaptor_forward_online_message_req_s_type), request_id,
			chatroom_id, chat_type, inbox_msg->msgId, inbox_msg->msgType, inbox_msg->sender,
			inbox_msg->receiver, inbox_msg->sentTime, __safe_add_string(inbox_msg->chatMsg),
			inbox_msg->chatroomId, inbox_msg->chatType, inbox_msg->message_ttl, skip_reply,
			(uint64_t) atoi(__safe_add_string(error_code->code)), __safe_add_string(error_code->msg));

	return response;
}

GVariant *__create_forward_unread_message_req_type(int64_t request_id,
						message_adaptor_inbox_message_s **inbox_msgs,
						unsigned int inbox_msgs_len,
						char *next_pagination_key,
						message_adaptor_error_code_t *error_code,
						void *server_data)
{
	GVariantBuilder *builder = g_variant_builder_new(G_VARIANT_TYPE(private_message_inbox_message_list_type));

	for (gsize i = 0; i < inbox_msgs_len; i++) {
		if (NULL == inbox_msgs[i]) {
			break;
		}

		g_variant_builder_open(builder, G_VARIANT_TYPE(private_service_adaptor_inbox_message_s_type));
		g_variant_builder_add(builder, "x", inbox_msgs[i]->msgId);
		g_variant_builder_add(builder, "i", inbox_msgs[i]->msgType);
		g_variant_builder_add(builder, "x", inbox_msgs[i]->sender);
		g_variant_builder_add(builder, "x", inbox_msgs[i]->receiver);
		g_variant_builder_add(builder, "x", inbox_msgs[i]->sentTime);
		safe_g_variant_builder_add_string(builder, inbox_msgs[i]->chatMsg);
		g_variant_builder_add(builder, "x", inbox_msgs[i]->chatroomId);
		g_variant_builder_add(builder, "i", inbox_msgs[i]->chatType);
		g_variant_builder_add(builder, "i", inbox_msgs[i]->message_ttl);
		g_variant_builder_close(builder);
	}

	GVariant *response = g_variant_new(MAKE_RETURN_TYPE(private_service_adaptor_forward_unread_message_req_s_type), request_id,
			builder, inbox_msgs_len, __safe_add_string(next_pagination_key),
			(uint64_t) atoi(__safe_add_string(error_code->code)), __safe_add_string(error_code->msg));

	g_variant_builder_unref(builder);

	return response;
}

GVariant *__create_read_message_res_type(int64_t request_id,
						int64_t chatroom_id,
						message_adaptor_error_code_t *error_code,
						void *server_data)
{
	GVariant *response = g_variant_new(MAKE_RETURN_TYPE(private_service_adaptor_read_message_res_s_type), request_id, chatroom_id,
			(uint64_t) atoi(__safe_add_string(error_code->code)), __safe_add_string(error_code->msg));

	return response;
}

GVariant *__create_invite_chat_res_type(int64_t request_id,
						int64_t chatroom_id,
						int64_t sent_time,
						message_adaptor_wrong_receiver_s *wrong_receiver,
						message_adaptor_error_code_t *error_code,
						void *server_data)
{
	message_adaptor_wrong_receiver_s _wrong_receiver;
	_wrong_receiver.invalid_receivers = NULL;
	_wrong_receiver.invalid_receivers_len = 0;
	_wrong_receiver.interrupted_receivers = NULL;
	_wrong_receiver.interrupted_receivers_len = 0;
	_wrong_receiver.disabled_receivers = NULL;
	_wrong_receiver.disabled_receivers_len = 0;
	_wrong_receiver.existing_chatmember = NULL;
	_wrong_receiver.existing_chatmembers_len = 0;
	_wrong_receiver.did_violation_users = NULL;
	_wrong_receiver.did_violation_users_len = 0;
	_wrong_receiver.invitation_denieds = NULL;
	_wrong_receiver.invitation_denieds_len = 0;

	if (NULL == wrong_receiver) {
		wrong_receiver = &_wrong_receiver;
	}

	GVariantBuilder *builder_invalid = g_variant_builder_new(G_VARIANT_TYPE("a(x)"));

	if (wrong_receiver->invalid_receivers) {
		for (gsize j = 0; j < wrong_receiver->invalid_receivers_len; j++) {
			g_variant_builder_add(builder_invalid, "(x)", wrong_receiver->invalid_receivers[j]);
		}
	}

	GVariantBuilder *builder_interrupted = g_variant_builder_new(G_VARIANT_TYPE("a(x)"));

	if (wrong_receiver->interrupted_receivers) {
		for (gsize j = 0; j < wrong_receiver->interrupted_receivers_len; j++) {
			g_variant_builder_add(builder_interrupted, "(x)", wrong_receiver->interrupted_receivers[j]);
		}
	}

	GVariantBuilder *builder_disabled = g_variant_builder_new(G_VARIANT_TYPE("a(x)"));

	if (wrong_receiver->disabled_receivers) {
		for (gsize j = 0; j < wrong_receiver->disabled_receivers_len; j++) {
			g_variant_builder_add(builder_disabled, "(x)", wrong_receiver->disabled_receivers[j]);
		}
	}

	GVariantBuilder *builder_existing = g_variant_builder_new(G_VARIANT_TYPE("a(x)"));

	if (wrong_receiver->existing_chatmember) {
		for (gsize j = 0; j < wrong_receiver->existing_chatmembers_len; j++) {
			g_variant_builder_add(builder_existing, "(x)", wrong_receiver->existing_chatmember[j]);
		}
	}

	GVariantBuilder *builder_did_violation_users = g_variant_builder_new(G_VARIANT_TYPE(private_message_did_violation_users_list_type));

	if (wrong_receiver->did_violation_users) {
		for (gsize j = 0; j < wrong_receiver->did_violation_users_len; j++) {
			g_variant_builder_open(builder_did_violation_users, G_VARIANT_TYPE(private_service_adaptor_did_violation_users_s_type));
			g_variant_builder_add(builder_did_violation_users, "x", wrong_receiver->did_violation_users[j].usera);
			g_variant_builder_add(builder_did_violation_users, "x", wrong_receiver->did_violation_users[j].userb);
			g_variant_builder_close(builder_did_violation_users);
		}
	}

	GVariantBuilder *builder_invitation = g_variant_builder_new(G_VARIANT_TYPE("a(x)"));

	if (wrong_receiver->invitation_denieds) {
		for (gsize j = 0; j < wrong_receiver->invitation_denieds_len; j++) {
			g_variant_builder_add(builder_invitation, "(x)", wrong_receiver->invitation_denieds[j]);
		}
	}


	GVariant *response = g_variant_new(MAKE_RETURN_TYPE(private_service_adaptor_invite_chat_res_s_type), request_id,
			chatroom_id, sent_time, builder_invalid, wrong_receiver->invalid_receivers_len,
			builder_interrupted, wrong_receiver->interrupted_receivers_len,
			builder_disabled, wrong_receiver->disabled_receivers_len,
			builder_existing, wrong_receiver->existing_chatmembers_len,
			builder_did_violation_users, wrong_receiver->did_violation_users_len,
			builder_invitation, wrong_receiver->invitation_denieds_len,
			(uint64_t) atoi(__safe_add_string(error_code->code)), __safe_add_string(error_code->msg));

	g_variant_builder_unref(builder_invalid);
	g_variant_builder_unref(builder_interrupted);
	g_variant_builder_unref(builder_disabled);
	g_variant_builder_unref(builder_existing);
	g_variant_builder_unref(builder_did_violation_users);

	return response;
}

GVariant *__create_end_chat_res_type(int64_t request_id,
						message_adaptor_error_code_t *error_code,
						void *server_data)
{
	GVariant *response = g_variant_new(MAKE_RETURN_TYPE(private_service_adaptor_end_chat_res_s_type), request_id,
			(uint64_t) atoi(__safe_add_string(error_code->code)), __safe_add_string(error_code->msg));

	return response;
}

GVariant *__create_unseal_message_res_type(int64_t request_id,
						int64_t chatroom_id,
						message_adaptor_error_code_t *error_code,
						void *server_data)
{
	GVariant *response = g_variant_new(MAKE_RETURN_TYPE(private_service_adaptor_unseal_message_res_s_type), request_id, chatroom_id, (uint64_t) atoi(__safe_add_string(error_code->code)), __safe_add_string(error_code->msg));

	return response;
}

GVariant *__create_save_call_log_res_type(int64_t request_id,
						message_adaptor_error_code_t *error_code,
						void *server_data)
{
	GVariant *response = g_variant_new(MAKE_RETURN_TYPE(private_service_adaptor_save_call_log_res_s_type), request_id, (uint64_t) atoi(__safe_add_string(error_code->code)), __safe_add_string(error_code->msg));

	return response;
}

GVariant *__create_current_time_res_type(int64_t request_id,
						int64_t current_time_millis,
						message_adaptor_error_code_t *error_code,
						void *server_data)
{
	GVariant *response = g_variant_new(MAKE_RETURN_TYPE(private_service_adaptor_current_time_res_s_type), request_id, current_time_millis,
			(uint64_t) atoi(__safe_add_string(error_code->code)), __safe_add_string(error_code->msg));

	return response;
}

GVariant *__create_message_channel_disconnected_res_type(const char *service_name,
						message_adaptor_error_code_t *error_code)
{
	GVariant *response = g_variant_new(MAKE_RETURN_TYPE(private_service_adaptor_channel_disconnected_res_s_type),
			__safe_add_string(service_name),
			(uint64_t) atoi(__safe_add_string(error_code->code)), __safe_add_string(error_code->msg));

	return response;
}

service_adaptor_internal_error_code_e dbus_reply_create_chatroom_callback(int64_t request_id,
						int64_t chatroom_id,
						int default_message_ttl,
						message_adaptor_wrong_receiver_s *wrong_receiver,
						message_adaptor_error_code_t *error_code,
						void *server_data)
{
	GError* error = NULL;
	GDBusConnection *dbus_connection = dbus_get_connection();

	if (NULL != dbus_connection) {
		GVariant *response = __create_create_chatroom_res_type(request_id, chatroom_id,
				default_message_ttl, wrong_receiver, error_code, server_data);

		g_dbus_connection_emit_signal(dbus_connection,
				NULL,
				SERVICE_ADAPTOR_OBJECT_PATH,
				SERVICE_ADAPTOR_INTERFACE,
				PRIVATE_DBUS_REPLY_CREATE_CHATROOM_SIGNAL,
				response,
				&error);

		if (NULL != error) {
			service_adaptor_debug("Unable to send msg: %s", error->message);
			return SERVICE_ADAPTOR_INTERNAL_ERROR_DBUS;
		}
	}

	return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
}

service_adaptor_internal_error_code_e dbus_reply_change_chatroom_meta_callback(int64_t request_id,
						int64_t chatroom_id,
						message_adaptor_error_code_t *error_code,
						void *server_data)
{
	GError* error = NULL;
	GDBusConnection *dbus_connection = dbus_get_connection();

	if (NULL != dbus_connection) {
		GVariant *response = __create_change_chatroom_meta_res_type(request_id, chatroom_id, error_code, server_data);

		g_dbus_connection_emit_signal(dbus_connection,
				NULL,
				SERVICE_ADAPTOR_OBJECT_PATH,
				SERVICE_ADAPTOR_INTERFACE,
				PRIVATE_DBUS_REPLY_CHANGE_CHATROOM_META_SIGNAL,
				response,
				&error);

		if (NULL != error) {
			service_adaptor_debug("Unable to send msg: %s", error->message);
			return SERVICE_ADAPTOR_INTERNAL_ERROR_DBUS;
		}
	}

	return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
}

service_adaptor_internal_error_code_e dbus_reply_chat_callback(int64_t request_id,
						int64_t chatroom_id,
						message_adaptor_processed_msg_s **processed_msgs,
						unsigned int processed_msgs_len,
						message_adaptor_error_code_t *error_code,
						void *server_data)
{
	GError* error = NULL;
	GDBusConnection *dbus_connection = dbus_get_connection();

	if (NULL != dbus_connection) {
		GVariant *response = __create_chat_res_type(request_id, chatroom_id,
				processed_msgs, processed_msgs_len, error_code, server_data);

		g_dbus_connection_emit_signal(dbus_connection,
				NULL,
				SERVICE_ADAPTOR_OBJECT_PATH,
				SERVICE_ADAPTOR_INTERFACE,
				PRIVATE_DBUS_REPLY_CHAT_SIGNAL,
				response,
				&error);

		if (NULL != error) {
			service_adaptor_debug("Unable to send msg: %s", error->message);
			return SERVICE_ADAPTOR_INTERNAL_ERROR_DBUS;
		}
	}

	return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
}

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
						void *server_data)
{
	GError* error = NULL;
	GDBusConnection *dbus_connection = dbus_get_connection();

	if (NULL != dbus_connection) {
		GVariant *response = __create_allow_chat_res_type(request_id, chatroom_id,
				delivery_acks, delivery_acks_len, last_delivery_acks_timestamp,
				read_acks, read_acks_len, last_read_acks_timestamp,
				ordered_chat_members, ordered_chat_members_len,
				chatroom_title, default_message_ttl,
				error_code, server_data);

		g_dbus_connection_emit_signal(dbus_connection,
				NULL,
				SERVICE_ADAPTOR_OBJECT_PATH,
				SERVICE_ADAPTOR_INTERFACE,
				PRIVATE_DBUS_REPLY_ALLOW_CHAT_SIGNAL,
				response,
				&error);

		if (NULL != error) {
			service_adaptor_debug("Unable to send msg: %s", error->message);
			return SERVICE_ADAPTOR_INTERNAL_ERROR_DBUS;
		}
	}

	return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
}

service_adaptor_internal_error_code_e dbus_reply_all_unread_message_callback(int64_t request_id,
						message_adaptor_error_code_t *error_code,
						void *server_data)
{
	GError* error = NULL;
	GDBusConnection *dbus_connection = dbus_get_connection();

	if (NULL != dbus_connection) {
		GVariant *response = __create_all_unread_message_res_type(request_id, error_code, server_data);

		g_dbus_connection_emit_signal(dbus_connection,
				NULL,
				SERVICE_ADAPTOR_OBJECT_PATH,
				SERVICE_ADAPTOR_INTERFACE,
				PRIVATE_DBUS_REPLY_ALL_UNREAD_MESSAGE_SIGNAL,
				response,
				&error);

		if (NULL != error) {
			service_adaptor_debug("Unable to send msg: %s", error->message);
			return SERVICE_ADAPTOR_INTERNAL_ERROR_DBUS;
		}
	}

	return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
}

service_adaptor_internal_error_code_e dbus_request_forward_online_message_callback(int64_t request_id,
						int64_t chatroom_id,
						int chat_type,
						message_adaptor_inbox_message_s *inbox_msg,
						bool skip_reply,
						message_adaptor_error_code_t *error_code,
						void *server_data)
{
	GError* error = NULL;
	GDBusConnection *dbus_connection = dbus_get_connection();

	service_adaptor_debug("%s Enter", __FUNCTION__);

	if (NULL != dbus_connection) {
		GVariant *response = __create_forward_online_message_req_type(request_id, chatroom_id,
				chat_type, inbox_msg, skip_reply, error_code, server_data);

		g_dbus_connection_emit_signal(dbus_connection,
				NULL,
				SERVICE_ADAPTOR_OBJECT_PATH,
				SERVICE_ADAPTOR_INTERFACE,
				PRIVATE_DBUS_REQUEST_FORWARD_ONLINE_MESSAGE_SIGNAL,
				response,
				&error);

		if (NULL != error) {
			service_adaptor_debug("Unable to send msg: %s", error->message);
			return SERVICE_ADAPTOR_INTERNAL_ERROR_DBUS;
		}
	}

	service_adaptor_debug("%s End", __FUNCTION__);

	return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
}

service_adaptor_internal_error_code_e dbus_request_forward_unread_message_callback(int64_t request_id,
						message_adaptor_inbox_message_s **inbox_msgs,
						unsigned int inbox_msgs_len,
						char *next_pagination_key,
						message_adaptor_error_code_t *error_code,
						void *server_data)
{
	GError* error = NULL;
	GDBusConnection *dbus_connection = dbus_get_connection();

	if (NULL != dbus_connection) {
		GVariant *response = __create_forward_unread_message_req_type(request_id, inbox_msgs,
				inbox_msgs_len, next_pagination_key, error_code, server_data);

		g_dbus_connection_emit_signal(dbus_connection,
				NULL,
				SERVICE_ADAPTOR_OBJECT_PATH,
				SERVICE_ADAPTOR_INTERFACE,
				PRIVATE_DBUS_REQUEST_FORWARD_UNREAD_MESSAGE_SIGNAL,
				response,
				&error);

		if (NULL != error) {
			service_adaptor_debug("Unable to send msg: %s", error->message);
			return SERVICE_ADAPTOR_INTERNAL_ERROR_DBUS;
		}
	}

	return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
}

service_adaptor_internal_error_code_e dbus_reply_read_message_callback(int64_t request_id,
						int64_t chatroom_id,
						message_adaptor_error_code_t *error_code,
						void *server_data)
{
	GError* error = NULL;
	GDBusConnection *dbus_connection = dbus_get_connection();

	if (NULL != dbus_connection) {
		GVariant *response = __create_read_message_res_type(request_id, chatroom_id, error_code, server_data);

		g_dbus_connection_emit_signal(dbus_connection,
				NULL,
				SERVICE_ADAPTOR_OBJECT_PATH,
				SERVICE_ADAPTOR_INTERFACE,
				PRIVATE_DBUS_REPLY_READ_MESSAGE_SIGNAL,
				response,
				&error);

		if (NULL != error) {
			service_adaptor_debug("Unable to send msg: %s", error->message);
			return SERVICE_ADAPTOR_INTERNAL_ERROR_DBUS;
		}
	}

	return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
}

service_adaptor_internal_error_code_e dbus_reply_invite_chat_callback(int64_t request_id,
						int64_t chatroom_id,
						int64_t sent_time,
						message_adaptor_wrong_receiver_s *wrong_receiver,
						message_adaptor_error_code_t *error_code,
						void *server_data)
{
	GError* error = NULL;
	GDBusConnection *dbus_connection = dbus_get_connection();

	if (NULL != dbus_connection) {
		GVariant *response = __create_invite_chat_res_type(request_id, chatroom_id,
				sent_time, wrong_receiver, error_code, server_data);

		g_dbus_connection_emit_signal(dbus_connection,
				NULL,
				SERVICE_ADAPTOR_OBJECT_PATH,
				SERVICE_ADAPTOR_INTERFACE,
				PRIVATE_DBUS_REPLY_INVITE_CHAT_SIGNAL,
				response,
				&error);

		if (NULL != error) {
			service_adaptor_debug("Unable to send msg: %s", error->message);
			return SERVICE_ADAPTOR_INTERNAL_ERROR_DBUS;
		}
	}

	return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
}

service_adaptor_internal_error_code_e dbus_reply_end_chat_callback(int64_t request_id,
						message_adaptor_error_code_t *error_code,
						void *server_data)
{
	GError* error = NULL;
	GDBusConnection *dbus_connection = dbus_get_connection();

	if (NULL != dbus_connection) {
		GVariant *response = __create_end_chat_res_type(request_id, error_code, server_data);

		g_dbus_connection_emit_signal(dbus_connection,
				NULL,
				SERVICE_ADAPTOR_OBJECT_PATH,
				SERVICE_ADAPTOR_INTERFACE,
				PRIVATE_DBUS_REPLY_END_CHAT_SIGNAL,
				response,
				&error);

		if (NULL != error) {
			service_adaptor_debug("Unable to send msg: %s", error->message);
			return SERVICE_ADAPTOR_INTERNAL_ERROR_DBUS;
		}
	}

	return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
}

service_adaptor_internal_error_code_e dbus_reply_unseal_message_callback(int64_t request_id,
						int64_t chatroom_id,
						message_adaptor_error_code_t *error_code,
						void *server_data)
{
	GError* error = NULL;
	GDBusConnection *dbus_connection = dbus_get_connection();

	if (NULL != dbus_connection) {
		GVariant *response = __create_unseal_message_res_type(request_id, chatroom_id, error_code, server_data);

		g_dbus_connection_emit_signal(dbus_connection,
				NULL,
				SERVICE_ADAPTOR_OBJECT_PATH,
				SERVICE_ADAPTOR_INTERFACE,
				PRIVATE_DBUS_REPLY_UNSEAL_MESSAGE_SIGNAL,
				response,
				&error);

		if (NULL != error) {
			service_adaptor_debug("Unable to send msg: %s", error->message);
			return SERVICE_ADAPTOR_INTERNAL_ERROR_DBUS;
		}
	}

	return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
}

service_adaptor_internal_error_code_e dbus_reply_save_call_log_callback(int64_t request_id,
						message_adaptor_error_code_t *error_code,
						void *server_data)
{
	GError* error = NULL;
	GDBusConnection *dbus_connection = dbus_get_connection();

	if (NULL != dbus_connection) {
		GVariant *response = __create_save_call_log_res_type(request_id, error_code, server_data);

		g_dbus_connection_emit_signal(dbus_connection,
				NULL,
				SERVICE_ADAPTOR_OBJECT_PATH,
				SERVICE_ADAPTOR_INTERFACE,
				PRIVATE_DBUS_REPLY_SAVE_CALL_LOG_SIGNAL,
				response,
				&error);

		if (NULL != error) {
			service_adaptor_debug("Unable to send msg: %s", error->message);
			return SERVICE_ADAPTOR_INTERNAL_ERROR_DBUS;
		}
	}

	return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
}

service_adaptor_internal_error_code_e dbus_reply_current_time_callback(int64_t request_id,
						int64_t current_time_millis,
						message_adaptor_error_code_t *error_code,
						void *server_data)
{
	GError* error = NULL;
	GDBusConnection *dbus_connection = dbus_get_connection();

	if (NULL != dbus_connection) {
		GVariant *response = __create_current_time_res_type(request_id, current_time_millis, error_code, server_data);

		g_dbus_connection_emit_signal(dbus_connection,
				NULL,
				SERVICE_ADAPTOR_OBJECT_PATH,
				SERVICE_ADAPTOR_INTERFACE,
				PRIVATE_DBUS_REPLY_CURRENT_TIME_SIGNAL,
				response,
				&error);

		if (NULL != error) {
			service_adaptor_debug("Unable to send msg: %s", error->message);
			return SERVICE_ADAPTOR_INTERNAL_ERROR_DBUS;
		}
	}

	return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
}

service_adaptor_internal_error_code_e dbus_reply_message_channel_disconnected_callback(const char *service_name,
						message_adaptor_error_code_t *error_code)
{
	GError* error = NULL;
	GDBusConnection *dbus_connection = dbus_get_connection();

	if (NULL != dbus_connection) {
		GVariant *response = __create_message_channel_disconnected_res_type(service_name, error_code);

		g_dbus_connection_emit_signal(dbus_connection,
				NULL,
				SERVICE_ADAPTOR_OBJECT_PATH,
				SERVICE_ADAPTOR_INTERFACE,
				PRIVATE_DBUS_REPLY_CHANNEL_DISCONNECTED_SIGNAL,
				response,
				&error);

		if (NULL != error) {
			service_adaptor_debug("Unable to send msg: %s", error->message);
			return SERVICE_ADAPTOR_INTERNAL_ERROR_DBUS;
		}
	}

	return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
}

