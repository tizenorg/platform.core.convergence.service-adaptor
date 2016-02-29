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
 * File: dbus-client-message.c
 * Desc:
 * Created on: Oct, 2014
 * Auth: Yongjin Kim <youth.kim@samsung.com>
 *
 *****************************************************************************/

#include <gio/gio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <glib-object.h>

#include <dbus-server.h>

#include "dbus_client.h"
#include "dbus_client_message.h"
#include "service_adaptor_client_type.h"
#include "service_adaptor_client_log.h"
#include "private/service-adaptor-client-message.h"

#include "util/service_adaptor_client_util.h"
/******************************************************************************
 * Global variables and defines
 ******************************************************************************/

/******************************************************************************
 * Private interface
 ******************************************************************************/

/******************************************************************************
 * Private interface definition
 ******************************************************************************/

#define __FREE_ERROR_CODE(err)	do { if (err) free(err->msg); free(err); (err) = NULL; } while (0)

#define __FREE_POINTER_ARRAY(__arr, __len)	do { \
						if ((NULL != (__arr)) && (0 < (__len))) { \
							for (int __idx = 0; __idx < (__len); __idx++) { \
								__SAFE_FREE(__arr[__idx]); \
							} \
							__SAFE_FREE((__arr)); \
						} } while (0)

#define __ipc_get_simple_error_code()	do { \
			GVariant *call_result_struct[2]; \
			call_result_struct[0] = g_variant_get_child_value(call_result, 0); \
			call_result_struct[1] = g_variant_get_child_value(call_result, 1); \
\
			uint64_t remote_call_result = g_variant_get_uint64(call_result_struct[0]); \
			if (SERVICE_ADAPTOR_ERROR_NONE != remote_call_result) { \
				error->code = remote_call_result; \
				error->msg = ipc_g_variant_dup_string(call_result_struct[1]); \
				ret = _get_result_code(remote_call_result); \
			} \
			g_variant_unref(call_result_struct[0]); \
			g_variant_unref(call_result_struct[1]); \
		} while (0)

void __free_wrong_receiver_s(service_adaptor_wrong_receiver_s *wrong_receiver)
{
	__SAFE_FREE(wrong_receiver->invalid_receivers);
	__SAFE_FREE(wrong_receiver->interrupted_receivers);
	__SAFE_FREE(wrong_receiver->disabled_receivers);
	__SAFE_FREE(wrong_receiver->existing_chatmembers);
	__FREE_POINTER_ARRAY(wrong_receiver->did_violation_users, wrong_receiver->did_violation_users_len);
	__SAFE_FREE(wrong_receiver->invitation_denieds);
}

void __free_ordered_chat_member_s_list(service_adaptor_ordered_chat_member_s **ordered_chat_members, unsigned int ordered_chat_members_len)
{
	if ((NULL != ordered_chat_members) && (0 < ordered_chat_members_len)) {
		for (int i = 0; i < ordered_chat_members_len; i++) {
			if (NULL != ordered_chat_members[i]) {
				__SAFE_FREE(ordered_chat_members[i]->name);
				__SAFE_FREE(ordered_chat_members[i]);
			}
		}
	}
}
void __free_inbox_message_s_list(service_adaptor_inbox_message_s **inbox_messages, unsigned int inbox_messages_len)
{
	if ((NULL != inbox_messages) && (0 < inbox_messages_len)) {
		for (int i = 0; i < inbox_messages_len; i++) {
			if (NULL != inbox_messages[i]) {
				__SAFE_FREE(inbox_messages[i]->chat_msg);
				__SAFE_FREE(inbox_messages[i]);
			}
		}
	}
}
void __get_create_chatroom_res_type(GVariant *parameters,
						int64_t *request_id,
						int64_t *chatroom_id,
						int *default_message_ttl,
						service_adaptor_wrong_receiver_s *wrong_receiver)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *res_struct[private_service_adaptor_create_chatroom_res_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_create_chatroom_res_s_type_length; j++) {
		res_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*request_id = g_variant_get_int64(res_struct[idx++]);
	*chatroom_id = g_variant_get_int64(res_struct[idx++]);
	*default_message_ttl = g_variant_get_int32(res_struct[idx++]);

	GVariant *res_info_struct[private_service_adaptor_wrong_receiver_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_wrong_receiver_s_type_length; j++) {
		res_info_struct[j] = g_variant_get_child_value(res_struct[idx], j);
	}

	int idx2 = 0;

	gsize invalid_count = g_variant_n_children(res_info_struct[idx2]);
	wrong_receiver->invalid_receivers = (int64_t *) calloc(invalid_count, sizeof(int64_t));

	if (NULL != wrong_receiver->invalid_receivers) {
		for (gsize i = 0; i < invalid_count; i++) {
			GVariant *info_entry_v = g_variant_get_child_value(res_info_struct[idx2], i);
			GVariant *info_struct = g_variant_get_child_value(info_entry_v, 0);

			wrong_receiver->invalid_receivers[i] = g_variant_get_int64(info_struct);

			g_variant_unref(info_struct);
		}
		idx2++;
		wrong_receiver->invalid_receivers_len = g_variant_get_uint32(res_info_struct[idx2++]);
	} else {
		wrong_receiver->invalid_receivers_len = 0U;
		idx2++;
		idx2++;
	}

	gsize interrupted_count = g_variant_n_children(res_info_struct[idx2]);
	wrong_receiver->interrupted_receivers = (int64_t *) calloc(interrupted_count, sizeof(int64_t));

	if (NULL != wrong_receiver->interrupted_receivers) {
		for (gsize i = 0; i < interrupted_count; i++) {
			GVariant *info_entry_v = g_variant_get_child_value(res_info_struct[idx2], i);
			GVariant *info_struct = g_variant_get_child_value(info_entry_v, 0);

			wrong_receiver->interrupted_receivers[i] = g_variant_get_int64(info_struct);

			g_variant_unref(info_struct);
		}
		idx2++;
		wrong_receiver->interrupted_receivers_len = g_variant_get_uint32(res_info_struct[idx2++]);
	} else {
		wrong_receiver->interrupted_receivers_len = 0U;
		idx2++;
		idx2++;
	}

	gsize disabled_count = g_variant_n_children(res_info_struct[idx2]);
	wrong_receiver->disabled_receivers = (int64_t *) calloc(disabled_count, sizeof(int64_t));

	if (NULL != wrong_receiver->disabled_receivers) {
		for (gsize i = 0; i < disabled_count; i++) {
			GVariant *info_entry_v = g_variant_get_child_value(res_info_struct[idx2], i);
			GVariant *info_struct = g_variant_get_child_value(info_entry_v, 0);

			wrong_receiver->disabled_receivers[i] = g_variant_get_int64(info_struct);

			g_variant_unref(info_struct);
		}
		idx2++;
		wrong_receiver->disabled_receivers_len = g_variant_get_uint32(res_info_struct[idx2++]);
	} else {
		wrong_receiver->disabled_receivers_len = 0U;
		idx2++;
		idx2++;
	}

	gsize existing_count = g_variant_n_children(res_info_struct[idx2]);
	wrong_receiver->existing_chatmembers = (int64_t *) calloc(existing_count, sizeof(int64_t));

	if (NULL != wrong_receiver->existing_chatmembers) {
		for (gsize i = 0; i < existing_count; i++) {
			GVariant *info_entry_v = g_variant_get_child_value(res_info_struct[idx2], i);
			GVariant *info_struct = g_variant_get_child_value(info_entry_v, 0);

			wrong_receiver->existing_chatmembers[i] = g_variant_get_int64(info_struct);

			g_variant_unref(info_struct);
		}
		idx2++;
		wrong_receiver->existing_chatmembers_len = g_variant_get_uint32(res_info_struct[idx2++]);
	} else {
		wrong_receiver->existing_chatmembers_len = 0U;
		idx2++;
		idx2++;
	}

	gsize did_violation_users_count = g_variant_n_children(res_info_struct[idx2]);
	wrong_receiver->did_violation_users = (service_adaptor_did_violation_users_s **) calloc(did_violation_users_count, sizeof(service_adaptor_did_violation_users_s *));

	if (NULL != wrong_receiver->did_violation_users) {
		for (gsize i = 0; i < did_violation_users_count; i++) {
			GVariant *info_entry_v = g_variant_get_child_value(res_info_struct[idx2], i);
			GVariant *info_struct[private_service_adaptor_did_violation_users_s_type_length];

			for (size_t j = 0; j < private_service_adaptor_did_violation_users_s_type_length; j++) {
				info_struct[j] = g_variant_get_child_value(info_entry_v, j);
			}

			int idx3 = 0;
			wrong_receiver->did_violation_users[i] = (service_adaptor_did_violation_users_s *) calloc(1, sizeof(service_adaptor_did_violation_users_s));
			if (NULL != wrong_receiver->did_violation_users[i]) {
				wrong_receiver->did_violation_users[i]->usera = g_variant_get_int64(info_struct[idx3++]);
				wrong_receiver->did_violation_users[i]->userb = g_variant_get_int64(info_struct[idx3++]);
			}

			for (size_t j = 0; j < private_service_adaptor_did_violation_users_s_type_length; j++) {
				g_variant_unref(info_struct[j]);
			}
		}
		idx2++;
		wrong_receiver->did_violation_users_len = g_variant_get_uint32(res_info_struct[idx2++]);
	} else {
		wrong_receiver->did_violation_users_len = 0U;
		idx2++;
		idx2++;
	}

	gsize denieds_count = g_variant_n_children(res_info_struct[idx2]);
	wrong_receiver->invitation_denieds = (int64_t *) calloc(denieds_count, sizeof(int64_t));

	if (NULL != wrong_receiver->invitation_denieds) {
		for (gsize i = 0; i < denieds_count; i++) {
			GVariant *info_entry_v = g_variant_get_child_value(res_info_struct[idx2], i);
			GVariant *info_struct = g_variant_get_child_value(info_entry_v, 0);

			wrong_receiver->invitation_denieds[i] = g_variant_get_int64(info_struct);

			g_variant_unref(info_struct);
		}
		idx2++;
		wrong_receiver->invitation_denieds_len = g_variant_get_uint32(res_info_struct[idx2++]);
	} else {
		wrong_receiver->invitation_denieds_len = 0U;
		idx2++;
		idx2++;
	}

	for (size_t j = 0; j < private_service_adaptor_wrong_receiver_s_type_length; j++) {
		g_variant_unref(res_info_struct[j]);
	}

	for (size_t j = 0; j < private_service_adaptor_create_chatroom_res_s_type_length; j++) {
		g_variant_unref(res_struct[j]);
	}
}

void __get_change_chatroom_meta_res_type(GVariant *parameters,
						int64_t *request_id,
						int64_t *chatroom_id)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *res_struct[private_service_adaptor_change_chatroom_meta_res_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_change_chatroom_meta_res_s_type_length; j++) {
		res_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*request_id = g_variant_get_int64(res_struct[idx++]);
	*chatroom_id = g_variant_get_int64(res_struct[idx++]);

	for (size_t j = 0; j < private_service_adaptor_change_chatroom_meta_res_s_type_length; j++) {
		g_variant_unref(res_struct[j]);
	}
}

void __get_chat_res_type(GVariant *parameters,
						int64_t *request_id,
						int64_t *chatroom_id,
						service_adaptor_processed_msg_s ***processed_msgs,
						unsigned int *processed_msgs_len)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *res_struct[private_service_adaptor_chat_res_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_chat_res_s_type_length; j++) {
		res_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*request_id = g_variant_get_int64(res_struct[idx++]);
	*chatroom_id = g_variant_get_int64(res_struct[idx++]);

	gsize list_count = g_variant_n_children(res_struct[idx]);

	*processed_msgs = (service_adaptor_processed_msg_s **) calloc(list_count, sizeof(service_adaptor_processed_msg_s *));

	if (NULL != *processed_msgs) {
		for (gsize i = 0; i < list_count; i++) {
			GVariant *info_struct[private_service_adaptor_processed_msg_s_type_length];
			GVariant *info_entry_v = g_variant_get_child_value(res_struct[idx], i);

			(*processed_msgs)[i] = (service_adaptor_processed_msg_s *) calloc(1, sizeof(service_adaptor_processed_msg_s));

			if (NULL != (*processed_msgs)[i]) {
				for (size_t j = 0; j < private_service_adaptor_processed_msg_s_type_length; j++) {
					info_struct[j] = g_variant_get_child_value(info_entry_v, j);
				}

				int idx2 = 0;
				(*processed_msgs)[i]->msg_id = g_variant_get_int64(info_struct[idx2++]);
				(*processed_msgs)[i]->sent_time = g_variant_get_int64(info_struct[idx2++]);

				for (size_t j = 0; j < private_service_adaptor_processed_msg_s_type_length; j++) {
					g_variant_unref(info_struct[j]);
				}
			}
		}
		idx++;
		*processed_msgs_len = g_variant_get_uint32(res_struct[idx++]);
	} else {
		*processed_msgs_len = 0U;
		idx++;
		idx++;
	}

	for (size_t j = 0; j < private_service_adaptor_chat_res_s_type_length; j++) {
		g_variant_unref(res_struct[j]);
	}
}

void __get_allow_chat_res_type(GVariant *parameters,
						int64_t *request_id,
						int64_t *chatroom_id,
						service_adaptor_delivery_ack_s ***delivery_acks,
						unsigned int *delivery_acks_len,
						unsigned long long *last_delivery_acks_timestamp,
						service_adaptor_read_ack_s ***read_acks,
						unsigned int *read_acks_len,
						unsigned long long *last_read_acks_timestamp,
						service_adaptor_ordered_chat_member_s ***ordered_chat_members,
						unsigned int *ordered_chat_members_len,
						char **chatroom_title,
						int *default_message_ttl)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *res_struct[private_service_adaptor_allow_chat_res_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_allow_chat_res_s_type_length; j++) {
		res_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*request_id = g_variant_get_int64(res_struct[idx++]);
	*chatroom_id = g_variant_get_int64(res_struct[idx++]);

	gsize delivery_acks_count = g_variant_n_children(res_struct[idx]);
	*delivery_acks = (service_adaptor_delivery_ack_s **) calloc(delivery_acks_count, sizeof(service_adaptor_delivery_ack_s *));

	if (NULL != *delivery_acks) {
		for (gsize i = 0; i < delivery_acks_count; i++) {
			GVariant *info_struct[private_service_adaptor_delivery_ack_s_type_length];
			GVariant *info_entry_v = g_variant_get_child_value(res_struct[idx], i);

			(*delivery_acks)[i] = (service_adaptor_delivery_ack_s *) calloc(1, sizeof(service_adaptor_delivery_ack_s));

			if (NULL == (*delivery_acks)[i]) {
				continue;
			}

			for (size_t j = 0; j < private_service_adaptor_delivery_ack_s_type_length; j++) {
				info_struct[j] = g_variant_get_child_value(info_entry_v, j);
			}

			int idx2 = 0;
			(*delivery_acks)[i]->user_id = g_variant_get_int64(info_struct[idx2++]);
			(*delivery_acks)[i]->msg_id = g_variant_get_int64(info_struct[idx2++]);
			(*delivery_acks)[i]->timestamp = g_variant_get_uint64(info_struct[idx2++]);

			for (size_t j = 0; j < private_service_adaptor_delivery_ack_s_type_length; j++) {
				g_variant_unref(info_struct[j]);
			}
		}
		idx++;

		*delivery_acks_len = g_variant_get_uint32(res_struct[idx++]);
	} else {
		*delivery_acks_len = 0U;
		idx++;
		idx++;
	}
	*last_delivery_acks_timestamp = g_variant_get_uint64(res_struct[idx++]);

	gsize read_acks_count = g_variant_n_children(res_struct[idx]);
	*read_acks = (service_adaptor_read_ack_s **) calloc(read_acks_count, sizeof(service_adaptor_read_ack_s *));

	if (NULL != *read_acks) {
		for (gsize i = 0; i < read_acks_count; i++) {
			GVariant *info_struct[private_service_adaptor_read_ack_s_type_length];
			GVariant *info_entry_v = g_variant_get_child_value(res_struct[idx], i);

			(*read_acks)[i] = (service_adaptor_read_ack_s *) calloc(1, sizeof(service_adaptor_read_ack_s));
			if (NULL == (*read_acks)[i]) {
				continue;
			}

			for (size_t j = 0; j < private_service_adaptor_read_ack_s_type_length; j++) {
				info_struct[j] = g_variant_get_child_value(info_entry_v, j);
			}

			int idx2 = 0;
			(*read_acks)[i]->user_id = g_variant_get_int64(info_struct[idx2++]);
			(*read_acks)[i]->msg_id = g_variant_get_int64(info_struct[idx2++]);
			(*read_acks)[i]->timestamp = g_variant_get_uint64(info_struct[idx2++]);

			for (size_t j = 0; j < private_service_adaptor_read_ack_s_type_length; j++) {
				g_variant_unref(info_struct[j]);
			}
		}
		idx++;

		*read_acks_len = g_variant_get_uint32(res_struct[idx++]);
	} else {
		*read_acks_len = 0U;
	}
	*last_read_acks_timestamp = g_variant_get_uint64(res_struct[idx++]);

	gsize ordered_chat_member_count = g_variant_n_children(res_struct[idx]);
	*ordered_chat_members = (service_adaptor_ordered_chat_member_s **) calloc(ordered_chat_member_count, sizeof(service_adaptor_ordered_chat_member_s *));

	if (NULL != *ordered_chat_members) {
		for (gsize i = 0; i < ordered_chat_member_count; i++) {
			GVariant *info_struct[private_service_adaptor_ordered_chat_member_s_type_length];
			GVariant *info_entry_v = g_variant_get_child_value(res_struct[idx], i);

			(*ordered_chat_members)[i] = (service_adaptor_ordered_chat_member_s *) calloc(1, sizeof(service_adaptor_ordered_chat_member_s));

			if (NULL != (*ordered_chat_members)[i]) {
				for (size_t j = 0; j < private_service_adaptor_ordered_chat_member_s_type_length; j++) {
					info_struct[j] = g_variant_get_child_value(info_entry_v, j);
				}

				int idx2 = 0;
				(*ordered_chat_members)[i]->user_id = g_variant_get_int64(info_struct[idx2++]);
				(*ordered_chat_members)[i]->available = g_variant_get_boolean(info_struct[idx2++]);
				(*ordered_chat_members)[i]->name = ipc_g_variant_dup_string(info_struct[idx2++]);

				for (size_t j = 0; j < private_service_adaptor_ordered_chat_member_s_type_length; j++) {
					g_variant_unref(info_struct[j]);
				}
			}
		}
		idx++;
		*ordered_chat_members_len = g_variant_get_uint32(res_struct[idx++]);
	} else {
		*ordered_chat_members_len = 0U;
		idx++;
		idx++;
	}
	*chatroom_title = ipc_g_variant_dup_string(res_struct[idx++]);
	*default_message_ttl = g_variant_get_int32(res_struct[idx++]);

	for (size_t j = 0; j < private_service_adaptor_allow_chat_res_s_type_length; j++) {
		g_variant_unref(res_struct[j]);
	}
}

void __get_all_unread_message_res_type(GVariant *parameters,
						int64_t *request_id)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *res_struct[private_service_adaptor_all_unread_message_res_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_all_unread_message_res_s_type_length; j++) {
		res_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*request_id = g_variant_get_int64(res_struct[idx++]);

	for (size_t j = 0; j < private_service_adaptor_all_unread_message_res_s_type_length; j++) {
		g_variant_unref(res_struct[j]);
	}
}
void __get_channel_disconnected_res_type(GVariant *parameters,
						char **service_name)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *res_struct[private_service_adaptor_channel_disconnected_res_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_channel_disconnected_res_s_type_length; j++) {
		res_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*service_name = ipc_g_variant_dup_string(res_struct[idx++]);

	for (size_t j = 0; j < private_service_adaptor_channel_disconnected_res_s_type_length; j++) {
		g_variant_unref(res_struct[j]);
	}
}


void __get_forward_online_message_req_type(GVariant *parameters,
						int64_t *request_id,
						int64_t *chatroom_id,
						int *chat_type,
						service_adaptor_inbox_message_s *inbox_msg,
						bool *skip_reply)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *res_struct[private_service_adaptor_forward_online_message_req_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_forward_online_message_req_s_type_length; j++) {
		res_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*request_id = g_variant_get_int64(res_struct[idx++]);
	*chatroom_id = g_variant_get_int64(res_struct[idx++]);
	*chat_type = g_variant_get_int32(res_struct[idx++]);

	GVariant *res_info_struct[private_service_adaptor_inbox_message_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_inbox_message_s_type_length; j++) {
		res_info_struct[j] = g_variant_get_child_value(res_struct[idx], j);
	}

	int idx2 = 0;

	inbox_msg->msg_id = g_variant_get_int64(res_info_struct[idx2++]);
	inbox_msg->msg_type = g_variant_get_int32(res_info_struct[idx2++]);
	inbox_msg->sender = g_variant_get_int64(res_info_struct[idx2++]);
	inbox_msg->receiver = g_variant_get_int64(res_info_struct[idx2++]);
	inbox_msg->sent_time = g_variant_get_int64(res_info_struct[idx2++]);
	inbox_msg->chat_msg = ipc_g_variant_dup_string(res_info_struct[idx2++]);
	inbox_msg->chatroom_id = g_variant_get_int64(res_info_struct[idx2++]);
	inbox_msg->chat_type = g_variant_get_int32(res_info_struct[idx2++]);
	inbox_msg->message_ttl = g_variant_get_int32(res_info_struct[idx2++]);

	for (size_t j = 0; j < private_service_adaptor_inbox_message_s_type_length; j++) {
		g_variant_unref(res_info_struct[j]);
	}
	idx++;

	*skip_reply = g_variant_get_boolean(res_struct[idx++]);

	for (size_t j = 0; j < private_service_adaptor_forward_online_message_req_s_type_length; j++) {
		g_variant_unref(res_struct[j]);
	}
}

void __get_forward_unread_message_req_type(GVariant *parameters,
						int64_t *request_id,
						service_adaptor_inbox_message_s ***inbox_msgs,
						unsigned int *inbox_msgs_len,
						char **next_pagination_key)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *res_struct[private_service_adaptor_forward_unread_message_req_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_forward_unread_message_req_s_type_length; j++) {
		res_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*request_id = g_variant_get_int64(res_struct[idx++]);

	gsize list_count = g_variant_n_children(res_struct[idx]);

	*inbox_msgs = (service_adaptor_inbox_message_s **) calloc(list_count, sizeof(service_adaptor_inbox_message_s *));

	if (NULL != *inbox_msgs) {
		for (gsize i = 0; i < list_count; i++) {
			GVariant *info_struct[private_service_adaptor_inbox_message_s_type_length];
			GVariant *info_entry_v = g_variant_get_child_value(res_struct[idx], i);

			(*inbox_msgs)[i] = (service_adaptor_inbox_message_s *) calloc(1, sizeof(service_adaptor_inbox_message_s));

			if (NULL != (*inbox_msgs)[i]) {
				for (size_t j = 0; j < private_service_adaptor_inbox_message_s_type_length; j++) {
					info_struct[j] = g_variant_get_child_value(info_entry_v, j);
				}

				int idx2 = 0;
				(*inbox_msgs)[i]->msg_id = g_variant_get_int64(info_struct[idx2++]);
				(*inbox_msgs)[i]->msg_type = g_variant_get_int32(info_struct[idx2++]);
				(*inbox_msgs)[i]->sender = g_variant_get_int64(info_struct[idx2++]);
				(*inbox_msgs)[i]->receiver = g_variant_get_int64(info_struct[idx2++]);
				(*inbox_msgs)[i]->sent_time = g_variant_get_int64(info_struct[idx2++]);
				(*inbox_msgs)[i]->chat_msg = ipc_g_variant_dup_string(info_struct[idx2++]);
				(*inbox_msgs)[i]->chatroom_id = g_variant_get_int64(info_struct[idx2++]);
				(*inbox_msgs)[i]->chat_type = g_variant_get_int32(info_struct[idx2++]);
				(*inbox_msgs)[i]->message_ttl = g_variant_get_int32(info_struct[idx2++]);

				for (size_t j = 0; j < private_service_adaptor_inbox_message_s_type_length; j++) {
					g_variant_unref(info_struct[j]);
				}
			}
		}
		idx++;
		*inbox_msgs_len = g_variant_get_uint32(res_struct[idx++]);
	} else {
		*inbox_msgs_len = 0U;
		idx++;
		idx++;
	}
	*next_pagination_key = ipc_g_variant_dup_string(res_struct[idx++]);

	for (size_t j = 0; j < private_service_adaptor_forward_unread_message_req_s_type_length; j++) {
		g_variant_unref(res_struct[j]);
	}
}

void __get_read_message_res_type(GVariant *parameters,
						int64_t *request_id,
						int64_t *chatroom_id)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *res_struct[private_service_adaptor_read_message_res_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_read_message_res_s_type_length; j++) {
		res_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*request_id = g_variant_get_int64(res_struct[idx++]);
	*chatroom_id = g_variant_get_int64(res_struct[idx++]);

	for (size_t j = 0; j < private_service_adaptor_read_message_res_s_type_length; j++) {
		g_variant_unref(res_struct[j]);
	}
}

void __get_invite_chat_res_type(GVariant *parameters,
						int64_t *request_id,
						int64_t *chatroom_id,
						int64_t *sent_time,
						service_adaptor_wrong_receiver_s *wrong_receiver)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *res_struct[private_service_adaptor_invite_chat_res_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_invite_chat_res_s_type_length; j++) {
		res_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*request_id = g_variant_get_int64(res_struct[idx++]);
	*chatroom_id = g_variant_get_int64(res_struct[idx++]);
	*sent_time = g_variant_get_int64(res_struct[idx++]);

	GVariant *res_info_struct[private_service_adaptor_wrong_receiver_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_wrong_receiver_s_type_length; j++) {
		res_info_struct[j] = g_variant_get_child_value(res_struct[idx], j);
	}

	int idx2 = 0;

	gsize invalid_count = g_variant_n_children(res_info_struct[idx2]);
	wrong_receiver->invalid_receivers = (int64_t *) calloc(invalid_count, sizeof(int64_t));

	if (NULL != wrong_receiver->invalid_receivers) {
		for (gsize i = 0; i < invalid_count; i++) {
			GVariant *info_entry_v = g_variant_get_child_value(res_info_struct[idx2], i);
			GVariant *info_struct = g_variant_get_child_value(info_entry_v, 0);

			wrong_receiver->invalid_receivers[i] = g_variant_get_int64(info_struct);

			g_variant_unref(info_struct);
		}
		idx2++;
		wrong_receiver->invalid_receivers_len = g_variant_get_uint32(res_info_struct[idx2++]);
	} else {
		wrong_receiver->invalid_receivers_len = 0U;
		idx2++;
		idx2++;
	}

	gsize interrupted_count = g_variant_n_children(res_info_struct[idx2]);
	wrong_receiver->interrupted_receivers = (int64_t *) calloc(interrupted_count, sizeof(int64_t));

	if (NULL != wrong_receiver->interrupted_receivers) {
		for (gsize i = 0; i < interrupted_count; i++) {
			GVariant *info_entry_v = g_variant_get_child_value(res_info_struct[idx2], i);
			GVariant *info_struct = g_variant_get_child_value(info_entry_v, 0);

			wrong_receiver->interrupted_receivers[i] = g_variant_get_int64(info_struct);

			g_variant_unref(info_struct);
		}
		idx2++;
		wrong_receiver->interrupted_receivers_len = g_variant_get_uint32(res_info_struct[idx2++]);
	} else {
		wrong_receiver->interrupted_receivers_len = 0U;
		idx2++;
		idx2++;
	}

	gsize disabled_count = g_variant_n_children(res_info_struct[idx2]);
	wrong_receiver->disabled_receivers = (int64_t *) calloc(disabled_count, sizeof(int64_t));

	if (NULL != wrong_receiver->disabled_receivers) {
		for (gsize i = 0; i < disabled_count; i++) {
			GVariant *info_entry_v = g_variant_get_child_value(res_info_struct[idx2], i);
			GVariant *info_struct = g_variant_get_child_value(info_entry_v, 0);

			wrong_receiver->disabled_receivers[i] = g_variant_get_int64(info_struct);

			g_variant_unref(info_struct);
		}
		idx2++;
		wrong_receiver->disabled_receivers_len = g_variant_get_uint32(res_info_struct[idx2++]);
	} else {
		wrong_receiver->disabled_receivers_len = 0U;
		idx2++;
		idx2++;
	}

	gsize existing_count = g_variant_n_children(res_info_struct[idx2]);
	wrong_receiver->existing_chatmembers = (int64_t *) calloc(existing_count, sizeof(int64_t));

	if (NULL != wrong_receiver->existing_chatmembers) {
		for (gsize i = 0; i < existing_count; i++) {
			GVariant *info_entry_v = g_variant_get_child_value(res_info_struct[idx2], i);
			GVariant *info_struct = g_variant_get_child_value(info_entry_v, 0);

			wrong_receiver->existing_chatmembers[i] = g_variant_get_int64(info_struct);

			g_variant_unref(info_struct);
		}
		idx2++;
		wrong_receiver->existing_chatmembers_len = g_variant_get_uint32(res_info_struct[idx2++]);
	} else {
		wrong_receiver->existing_chatmembers_len = 0U;
		idx2++;
		idx2++;
	}

	gsize did_violation_count = g_variant_n_children(res_info_struct[idx2]);
	wrong_receiver->did_violation_users = (service_adaptor_did_violation_users_s **) calloc(did_violation_count, sizeof(service_adaptor_did_violation_users_s *));

	if (NULL != wrong_receiver->did_violation_users) {
		for (gsize i = 0; i < did_violation_count; i++) {
			GVariant *info_entry_v = g_variant_get_child_value(res_info_struct[idx2], i);
			GVariant *info_struct[private_service_adaptor_did_violation_users_s_type_length];

			for (size_t j = 0; j < private_service_adaptor_did_violation_users_s_type_length; j++) {
				info_struct[j] = g_variant_get_child_value(info_entry_v, j);
			}

			int idx3 = 0;

			wrong_receiver->did_violation_users[i] = (service_adaptor_did_violation_users_s *) calloc(1, sizeof(service_adaptor_did_violation_users_s));
			if (NULL != wrong_receiver->did_violation_users[i]) {
				wrong_receiver->did_violation_users[i]->usera = g_variant_get_int64(info_struct[idx3++]);
				wrong_receiver->did_violation_users[i]->userb = g_variant_get_int64(info_struct[idx3++]);
			}

			for (size_t j = 0; j < private_service_adaptor_did_violation_users_s_type_length; j++) {
				g_variant_unref(info_struct[j]);
			}
		}
		idx2++;
		wrong_receiver->did_violation_users_len = g_variant_get_uint32(res_info_struct[idx2++]);
	} else {
		wrong_receiver->did_violation_users_len = 0U;
		idx2++;
		idx2++;
	}

	gsize denieds_count = g_variant_n_children(res_info_struct[idx2]);
	wrong_receiver->invitation_denieds = (int64_t *) calloc(denieds_count, sizeof(int64_t));

	if (NULL != wrong_receiver->invitation_denieds) {
		for (gsize i = 0; i < denieds_count; i++) {
			GVariant *info_entry_v = g_variant_get_child_value(res_info_struct[idx2], i);
			GVariant *info_struct = g_variant_get_child_value(info_entry_v, 0);

			wrong_receiver->invitation_denieds[i] = g_variant_get_int64(info_struct);

			g_variant_unref(info_struct);
		}
		idx2++;
		wrong_receiver->invitation_denieds_len = g_variant_get_uint32(res_info_struct[idx2++]);
	} else {
		wrong_receiver->invitation_denieds_len = 0U;
		idx2++;
		idx2++;
	}


	for (size_t j = 0; j < private_service_adaptor_wrong_receiver_s_type_length; j++) {
		g_variant_unref(res_info_struct[j]);
	}

	for (size_t j = 0; j < private_service_adaptor_invite_chat_res_s_type_length; j++) {
		g_variant_unref(res_struct[j]);
	}
}

void __get_end_chat_res_type(GVariant *parameters,
						int64_t *request_id)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *res_struct[private_service_adaptor_end_chat_res_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_end_chat_res_s_type_length; j++) {
		res_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*request_id = g_variant_get_int64(res_struct[idx++]);

	for (size_t j = 0; j < private_service_adaptor_end_chat_res_s_type_length; j++) {
		g_variant_unref(res_struct[j]);
	}
}

void __get_unseal_message_res_type(GVariant *parameters,
						int64_t *request_id,
						int64_t *chatroom_id)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *res_struct[private_service_adaptor_unseal_message_res_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_unseal_message_res_s_type_length; j++) {
		res_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*request_id = g_variant_get_int64(res_struct[idx++]);
	*chatroom_id = g_variant_get_int64(res_struct[idx++]);

	for (size_t j = 0; j < private_service_adaptor_unseal_message_res_s_type_length; j++) {
		g_variant_unref(res_struct[j]);
	}
}
void __get_save_call_log_res_type(GVariant *parameters,
						int64_t *request_id)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *res_struct[private_service_adaptor_save_call_log_res_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_save_call_log_res_s_type_length; j++) {
		res_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*request_id = g_variant_get_int64(res_struct[idx++]);

	for (size_t j = 0; j < private_service_adaptor_save_call_log_res_s_type_length; j++) {
		g_variant_unref(res_struct[j]);
	}
}

void __get_current_time_res_type(GVariant *parameters,
						int64_t *request_id,
						int64_t *current_time_millis)
{
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *res_struct[private_service_adaptor_current_time_res_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_current_time_res_s_type_length; j++) {
		res_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*request_id = g_variant_get_int64(res_struct[idx++]);
	*current_time_millis = g_variant_get_int64(res_struct[idx++]);

	for (size_t j = 0; j < private_service_adaptor_current_time_res_s_type_length; j++) {
		g_variant_unref(res_struct[j]);
	}
}

void __get_chat_id_list_res_type(GVariant *call_result_struct,
						service_adaptor_chat_id_s ***chat_ids,
						unsigned int *chat_ids_len,
						void **server_data)
{
	GVariant *res_info_struct[private_service_adaptor_chat_id_list_res_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_chat_id_list_res_s_type_length; j++) {
		res_info_struct[j] = g_variant_get_child_value(call_result_struct, j);
	}

	int idx = 0;
	gsize list_count = g_variant_n_children(res_info_struct[idx]);

	*chat_ids = (service_adaptor_chat_id_s **) calloc(list_count, sizeof(service_adaptor_chat_id_s *));

	if (NULL != *chat_ids) {
		for (gsize i = 0; i < list_count; i++) {
			GVariant *list_info_struct[private_service_adaptor_chat_id_s_type_length];
			GVariant *list_info_entry_v = g_variant_get_child_value(res_info_struct[idx], i);
			(*chat_ids)[i] = (service_adaptor_chat_id_s *) calloc(1, sizeof(service_adaptor_chat_id_s));

			if (NULL != (*chat_ids)[i]) {
				for (size_t j = 0; j < private_service_adaptor_chat_id_s_type_length; j++) {
					list_info_struct[j] = g_variant_get_child_value(list_info_entry_v, j);
				}

				int idx2 = 0;
				(*chat_ids)[i]->chatid = g_variant_get_int64(list_info_struct[idx2++]);
				(*chat_ids)[i]->msisdn = ipc_g_variant_dup_string(list_info_struct[idx2++]);

				for (size_t j = 0; j < private_service_adaptor_chat_id_s_type_length; j++) {
					g_variant_unref(list_info_struct[j]);
				}
			}
		}
		idx++;
		*chat_ids_len = g_variant_get_uint32(res_info_struct[idx++]);
	} else {
		*chat_ids_len = 0U;
		idx++;
		idx++;
	}

	for (size_t j = 0; j < private_service_adaptor_chat_id_list_res_s_type_length; j++) {
		g_variant_unref(res_info_struct[j]);
	}
}

/******************************************************************************
 * Public interface definition
 ******************************************************************************/

void on_message_signal(GDBusProxy *proxy,
						gchar *sender_name,
						gchar *signal_name,
						GVariant *parameters,
						gpointer user_data)
{
	if (0 == g_strcmp0(signal_name, PRIVATE_DBUS_REPLY_CREATE_CHATROOM_SIGNAL)) {
		int64_t request_id = 0;
		int64_t chatroom_id = 0;
		int default_message_ttl = 0;
		service_adaptor_wrong_receiver_s wrong_receiver;
		service_adaptor_error_s *error_code = NULL;

		__get_create_chatroom_res_type(parameters, &request_id, &chatroom_id, &default_message_ttl, &wrong_receiver);

		service_adaptor_task_h task = _queue_get_task(request_id);

		if (NULL == task) {
			__free_wrong_receiver_s(&wrong_receiver);
			return;
		}

		GVariant *call_result[2];
		call_result[0] = g_variant_get_child_value(parameters, 1);
		call_result[1] = g_variant_get_child_value(parameters, 2);

		uint64_t remote_call_result = g_variant_get_uint64(call_result[0]);

		if (SERVICE_ADAPTOR_ERROR_NONE != remote_call_result) {
			error_code = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));

			if (NULL != error_code) {
				error_code->code = remote_call_result;
				error_code->msg = ipc_g_variant_dup_string(call_result[1]);
			}
		}

		service_adaptor_reply_create_chatroom_cb callback = (service_adaptor_reply_create_chatroom_cb) task->callback;

		if (NULL != callback) {
			callback(task->handle, request_id, chatroom_id, default_message_ttl, &wrong_receiver, error_code, task->user_data);
		}

		_queue_del_task(task);
		__free_wrong_receiver_s(&wrong_receiver);
		__FREE_ERROR_CODE(error_code);
	} else if (0 == g_strcmp0(signal_name, PRIVATE_DBUS_REPLY_CHANGE_CHATROOM_META_SIGNAL)) {
		int64_t request_id = 0;
		int64_t chatroom_id = 0;
		service_adaptor_error_s *error_code = NULL;

		__get_change_chatroom_meta_res_type(parameters, &request_id, &chatroom_id);

		service_adaptor_task_h task = _queue_get_task(request_id);

		if (NULL == task) {
			return;
		}

		GVariant *call_result[2];
		call_result[0] = g_variant_get_child_value(parameters, 1);
		call_result[1] = g_variant_get_child_value(parameters, 2);

		uint64_t remote_call_result = g_variant_get_uint64(call_result[0]);

		if (SERVICE_ADAPTOR_ERROR_NONE != remote_call_result) {
			error_code = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));

			if (NULL != error_code) {
				error_code->code = remote_call_result;
				error_code->msg = ipc_g_variant_dup_string(call_result[1]);
			}
		}

		service_adaptor_reply_change_chatroom_meta_cb callback = (service_adaptor_reply_change_chatroom_meta_cb) task->callback;

		if (NULL != callback) {
			callback(task->handle, request_id, chatroom_id, error_code, task->user_data);
		}
		__FREE_ERROR_CODE(error_code);

		_queue_del_task(task);
	} else if (0 == g_strcmp0(signal_name, PRIVATE_DBUS_REPLY_CHAT_SIGNAL)) {
		int64_t request_id = 0;
		int64_t chatroom_id = 0;
		service_adaptor_processed_msg_s **processed_msgs = NULL;
		unsigned int processed_msgs_len = 0;
		service_adaptor_error_s *error_code = NULL;

		__get_chat_res_type(parameters, &request_id, &chatroom_id, &processed_msgs, &processed_msgs_len);

		service_adaptor_task_h task = _queue_get_task(request_id);

		if (NULL == task) {
			__FREE_POINTER_ARRAY(processed_msgs, processed_msgs_len);
			__SAFE_FREE(processed_msgs);
			return;
		}

		GVariant *call_result[2];
		call_result[0] = g_variant_get_child_value(parameters, 1);
		call_result[1] = g_variant_get_child_value(parameters, 2);

		uint64_t remote_call_result = g_variant_get_uint64(call_result[0]);

		if (SERVICE_ADAPTOR_ERROR_NONE != remote_call_result) {
			error_code = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));

			if (NULL != error_code) {
				error_code->code = remote_call_result;
				error_code->msg = ipc_g_variant_dup_string(call_result[1]);
			}
		}

		service_adaptor_reply_chat_cb callback = (service_adaptor_reply_chat_cb) task->callback;

		if (NULL != callback) {
			callback(task->handle, request_id, chatroom_id, processed_msgs, processed_msgs_len, error_code, task->user_data);
		}
		__FREE_POINTER_ARRAY(processed_msgs, processed_msgs_len);
		__SAFE_FREE(processed_msgs);
		__FREE_ERROR_CODE(error_code);

		_queue_del_task(task);
	} else if (0 == g_strcmp0(signal_name, PRIVATE_DBUS_REPLY_ALLOW_CHAT_SIGNAL)) {
		int64_t request_id = 0;
		int64_t chatroom_id = 0;
		service_adaptor_delivery_ack_s **delivery_acks = NULL;
		unsigned int delivery_acks_len = 0;
		unsigned long long last_delivery_acks_timestamp = 0;
		service_adaptor_read_ack_s **read_acks = NULL;
		unsigned int read_acks_len = 0;
		unsigned long long last_read_acks_timestamp = 0;
		service_adaptor_ordered_chat_member_s **ordered_chat_members = NULL;
		unsigned int ordered_chat_members_len = 0;
		char *chatroom_title = NULL;
		int default_message_ttl = 0;
		service_adaptor_error_s *error_code = NULL;


		__get_allow_chat_res_type(parameters, &request_id, &chatroom_id,
				&delivery_acks, &delivery_acks_len, &last_delivery_acks_timestamp,
				&read_acks, &read_acks_len, &last_read_acks_timestamp,
				&ordered_chat_members, &ordered_chat_members_len,
				&chatroom_title, &default_message_ttl);

		service_adaptor_task_h task = _queue_get_task(request_id);

		if (NULL == task) {
			__FREE_POINTER_ARRAY(delivery_acks, delivery_acks_len);
			__SAFE_FREE(delivery_acks);
			__FREE_POINTER_ARRAY(read_acks, read_acks_len);
			__SAFE_FREE(read_acks);
			__free_ordered_chat_member_s_list(ordered_chat_members, ordered_chat_members_len);
			__SAFE_FREE(ordered_chat_members);
			__SAFE_FREE(chatroom_title);
			return;
		}

		GVariant *call_result[2];
		call_result[0] = g_variant_get_child_value(parameters, 1);
		call_result[1] = g_variant_get_child_value(parameters, 2);

		uint64_t remote_call_result = g_variant_get_uint64(call_result[0]);

		if (SERVICE_ADAPTOR_ERROR_NONE != remote_call_result) {
			error_code = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));

			if (NULL != error_code) {
				error_code->code = remote_call_result;
				error_code->msg = ipc_g_variant_dup_string(call_result[1]);
			}
		}

		service_adaptor_reply_allow_chat_cb callback = (service_adaptor_reply_allow_chat_cb) task->callback;
		if (NULL != callback) {
			callback(task->handle, request_id, chatroom_id,
					delivery_acks, delivery_acks_len, last_delivery_acks_timestamp,
					read_acks, read_acks_len, last_read_acks_timestamp,
					ordered_chat_members, ordered_chat_members_len,
					chatroom_title, default_message_ttl, error_code, task->user_data);
		}
		__FREE_POINTER_ARRAY(delivery_acks, delivery_acks_len);
		__SAFE_FREE(delivery_acks);
		__FREE_POINTER_ARRAY(read_acks, read_acks_len);
		__SAFE_FREE(read_acks);
		__free_ordered_chat_member_s_list(ordered_chat_members, ordered_chat_members_len);
		__SAFE_FREE(ordered_chat_members);
		__SAFE_FREE(chatroom_title);
		__FREE_ERROR_CODE(error_code);

		_queue_del_task(task);
	} else if (0 == g_strcmp0(signal_name, PRIVATE_DBUS_REPLY_ALL_UNREAD_MESSAGE_SIGNAL)) {
		int64_t request_id = 0;
		service_adaptor_error_s *error_code = NULL;

		__get_all_unread_message_res_type(parameters, &request_id);

		service_adaptor_task_h task = _queue_get_task(request_id);

		if (NULL == task) {
			return;
		}

		GVariant *call_result[2];
		call_result[0] = g_variant_get_child_value(parameters, 1);
		call_result[1] = g_variant_get_child_value(parameters, 2);

		uint64_t remote_call_result = g_variant_get_uint64(call_result[0]);

		if (SERVICE_ADAPTOR_ERROR_NONE != remote_call_result) {
			error_code = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));

			if (NULL != error_code) {
				error_code->code = remote_call_result;
				error_code->msg = ipc_g_variant_dup_string(call_result[1]);
			}
		}

		service_adaptor_reply_all_unread_message_cb callback = (service_adaptor_reply_all_unread_message_cb) task->callback;

		if (NULL != callback) {
			callback(task->handle, request_id, error_code, task->user_data);
		}
		__FREE_ERROR_CODE(error_code);

		_queue_del_task(task);
	} else if (0 == g_strcmp0(signal_name, PRIVATE_DBUS_REQUEST_FORWARD_ONLINE_MESSAGE_SIGNAL)) {
		int64_t request_id = 0;
		int64_t chatroom_id = 0;
		int chat_type = 0;
		service_adaptor_inbox_message_s inbox_msg;
		bool skip_reply = false;
		service_adaptor_error_s *error_code = NULL;

		__get_forward_online_message_req_type(parameters, &request_id, &chatroom_id, &chat_type, &inbox_msg, &skip_reply);

		service_adaptor_task_h task = _queue_get_task(-100);

		if (NULL == task) {
			__SAFE_FREE(inbox_msg.chat_msg);
			return;
		}

		GVariant *call_result[2];
		call_result[0] = g_variant_get_child_value(parameters, 1);
		call_result[1] = g_variant_get_child_value(parameters, 2);

		uint64_t remote_call_result = g_variant_get_uint64(call_result[0]);

		if (SERVICE_ADAPTOR_ERROR_NONE != remote_call_result) {
			error_code = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));

			if (NULL != error_code) {
				error_code->code = remote_call_result;
				error_code->msg = ipc_g_variant_dup_string(call_result[1]);
			}
		}

		service_adaptor_request_forward_online_message_cb callback = (service_adaptor_request_forward_online_message_cb) task->callback;

		if (NULL != callback) {
			callback(task->handle, request_id, chatroom_id, chat_type, &inbox_msg, skip_reply, error_code, task->user_data);
		}
		__SAFE_FREE(inbox_msg.chat_msg);
		__FREE_ERROR_CODE(error_code);
	} else if (0 == g_strcmp0(signal_name, PRIVATE_DBUS_REQUEST_FORWARD_UNREAD_MESSAGE_SIGNAL)) {
		int64_t request_id = 0;
		service_adaptor_inbox_message_s **inbox_msgs = NULL;
		uint32_t inbox_msgs_len = 0;
		char *next_pagination_key = NULL;
		service_adaptor_error_s *error_code = NULL;

		__get_forward_unread_message_req_type(parameters, &request_id, &inbox_msgs, &inbox_msgs_len, &next_pagination_key);

		service_adaptor_task_h task = _queue_get_task(-101);

		if (NULL == task) {
			__SAFE_FREE(next_pagination_key);
			__free_inbox_message_s_list(inbox_msgs, inbox_msgs_len);
			__SAFE_FREE(inbox_msgs);
			return;
		}

		GVariant *call_result[2];
		call_result[0] = g_variant_get_child_value(parameters, 1);
		call_result[1] = g_variant_get_child_value(parameters, 2);

		uint64_t remote_call_result = g_variant_get_uint64(call_result[0]);

		if (SERVICE_ADAPTOR_ERROR_NONE != remote_call_result) {
			error_code = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));

			if (NULL != error_code) {
				error_code->code = remote_call_result;
				error_code->msg = ipc_g_variant_dup_string(call_result[1]);
			}
		}

		service_adaptor_request_forward_unread_message_cb callback = (service_adaptor_request_forward_unread_message_cb) task->callback;

		if (NULL != callback) {
			callback(task->handle, request_id, inbox_msgs, inbox_msgs_len, next_pagination_key, error_code, task->user_data);
		}
		__SAFE_FREE(next_pagination_key);
		__free_inbox_message_s_list(inbox_msgs, inbox_msgs_len);
		__SAFE_FREE(inbox_msgs);
		__FREE_ERROR_CODE(error_code);
	} else if (0 == g_strcmp0(signal_name, PRIVATE_DBUS_REPLY_CHANNEL_DISCONNECTED_SIGNAL)) {
		char *service_name = NULL;
		service_adaptor_error_s *error_code = NULL;

		__get_channel_disconnected_res_type(parameters, &service_name);

		service_adaptor_task_h task = _queue_get_task(-102);

		if (NULL == task) {
			__SAFE_FREE(service_name);
			return;
		}

		GVariant *call_result[2];
		call_result[0] = g_variant_get_child_value(parameters, 1);
		call_result[1] = g_variant_get_child_value(parameters, 2);

		uint64_t remote_call_result = g_variant_get_uint64(call_result[0]);

		if (SERVICE_ADAPTOR_ERROR_NONE != remote_call_result) {
			error_code = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));

			if (NULL != error_code) {
				error_code->code = remote_call_result;
				error_code->msg = ipc_g_variant_dup_string(call_result[1]);
			}
		}

		service_adaptor_reply_channel_disconnected_cb callback = (service_adaptor_reply_channel_disconnected_cb) task->callback;

		if ((NULL != callback) && (NULL != task->handle) && (NULL != ((service_adaptor_h)task->handle)->service_name) && (NULL != service_name)
				&& (0 == strncmp(((service_adaptor_h)task->handle)->service_name, service_name, strlen(((service_adaptor_h)task->handle)->service_name)))) {
			callback(task->handle, error_code, task->user_data);
		}
		__SAFE_FREE(service_name);
		__FREE_ERROR_CODE(error_code);
	} else if (0 == g_strcmp0(signal_name, PRIVATE_DBUS_REPLY_READ_MESSAGE_SIGNAL)) {
		int64_t request_id = 0;
		int64_t chatroom_id = 0;
		service_adaptor_error_s *error_code = NULL;

		__get_read_message_res_type(parameters, &request_id, &chatroom_id);

		service_adaptor_task_h task = _queue_get_task(request_id);

		if (NULL == task) {
			return;
		}

		GVariant *call_result[2];
		call_result[0] = g_variant_get_child_value(parameters, 1);
		call_result[1] = g_variant_get_child_value(parameters, 2);

		uint64_t remote_call_result = g_variant_get_uint64(call_result[0]);

		if (SERVICE_ADAPTOR_ERROR_NONE != remote_call_result) {
			error_code = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));

			if (NULL != error_code) {
				error_code->code = remote_call_result;
				error_code->msg = ipc_g_variant_dup_string(call_result[1]);
			}
		}

		service_adaptor_reply_read_message_cb callback = (service_adaptor_reply_read_message_cb) task->callback;
		if (NULL != callback) {
			callback(task->handle, request_id, chatroom_id, error_code, task->user_data);
		}
		__FREE_ERROR_CODE(error_code);

		_queue_del_task(task);
	} else if (0 == g_strcmp0(signal_name, PRIVATE_DBUS_REPLY_INVITE_CHAT_SIGNAL)) {
		int64_t request_id = 0;
		int64_t chatroom_id = 0;
		int64_t sent_time = 0;
		service_adaptor_wrong_receiver_s wrong_receiver;
		service_adaptor_error_s *error_code = NULL;

		__get_invite_chat_res_type(parameters, &request_id, &chatroom_id, &sent_time, &wrong_receiver);

		service_adaptor_task_h task = _queue_get_task(request_id);

		if (NULL == task) {
			__free_wrong_receiver_s(&wrong_receiver);
			return;
		}

		GVariant *call_result[2];
		call_result[0] = g_variant_get_child_value(parameters, 1);
		call_result[1] = g_variant_get_child_value(parameters, 2);

		uint64_t remote_call_result = g_variant_get_uint64(call_result[0]);

		if (SERVICE_ADAPTOR_ERROR_NONE != remote_call_result) {
			error_code = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));

			if (NULL != error_code) {
				error_code->code = remote_call_result;
				error_code->msg = ipc_g_variant_dup_string(call_result[1]);
			}
		}

		service_adaptor_reply_invite_chat_cb callback = (service_adaptor_reply_invite_chat_cb) task->callback;
		if (NULL != callback) {
			callback(task->handle, request_id, chatroom_id, sent_time, &wrong_receiver, error_code, task->user_data);
		}

		__free_wrong_receiver_s(&wrong_receiver);
		__FREE_ERROR_CODE(error_code);
		_queue_del_task(task);
	} else if (0 == g_strcmp0(signal_name, PRIVATE_DBUS_REPLY_END_CHAT_SIGNAL)) {
		int64_t request_id = 0;
		service_adaptor_error_s *error_code = NULL;

		__get_end_chat_res_type(parameters, &request_id);

		service_adaptor_task_h task = _queue_get_task(request_id);

		if (NULL == task) {
			return;
		}

		GVariant *call_result[2];
		call_result[0] = g_variant_get_child_value(parameters, 1);
		call_result[1] = g_variant_get_child_value(parameters, 2);

		uint64_t remote_call_result = g_variant_get_uint64(call_result[0]);

		if (SERVICE_ADAPTOR_ERROR_NONE != remote_call_result) {
			error_code = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));

			if (NULL != error_code) {
				error_code->code = remote_call_result;
				error_code->msg = ipc_g_variant_dup_string(call_result[1]);
			}
		}

		service_adaptor_reply_end_chat_cb callback = (service_adaptor_reply_end_chat_cb) task->callback;
		if (NULL != callback) {
			callback(task->handle, request_id, error_code, task->user_data);
		}
		__FREE_ERROR_CODE(error_code);

		_queue_del_task(task);
	} else if (0 == g_strcmp0(signal_name, PRIVATE_DBUS_REPLY_UNSEAL_MESSAGE_SIGNAL)) {
		int64_t request_id = 0;
		int64_t chatroom_id = 0;
		service_adaptor_error_s *error_code = NULL;

		__get_unseal_message_res_type(parameters, &request_id, &chatroom_id);

		service_adaptor_task_h task = _queue_get_task(request_id);

		if (NULL == task) {
			return;
		}

		GVariant *call_result[2];
		call_result[0] = g_variant_get_child_value(parameters, 1);
		call_result[1] = g_variant_get_child_value(parameters, 2);

		uint64_t remote_call_result = g_variant_get_uint64(call_result[0]);

		if (SERVICE_ADAPTOR_ERROR_NONE != remote_call_result) {
			error_code = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));

			if (NULL != error_code) {
				error_code->code = remote_call_result;
				error_code->msg = ipc_g_variant_dup_string(call_result[1]);
			}
		}

		service_adaptor_reply_unseal_message_cb callback = (service_adaptor_reply_unseal_message_cb) task->callback;
		if (NULL != callback) {
			callback(task->handle, request_id, chatroom_id, error_code, task->user_data);
		}
		__FREE_ERROR_CODE(error_code);

		_queue_del_task(task);
	} else if (0 == g_strcmp0(signal_name, PRIVATE_DBUS_REPLY_SAVE_CALL_LOG_SIGNAL)) {
		int64_t request_id = 0;
		service_adaptor_error_s *error_code = NULL;

		__get_save_call_log_res_type(parameters, &request_id);

		service_adaptor_task_h task = _queue_get_task(request_id);

		if (NULL == task) {
			return;
		}

		GVariant *call_result[2];
		call_result[0] = g_variant_get_child_value(parameters, 1);
		call_result[1] = g_variant_get_child_value(parameters, 2);

		uint64_t remote_call_result = g_variant_get_uint64(call_result[0]);

		if (SERVICE_ADAPTOR_ERROR_NONE != remote_call_result) {
			error_code = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));

			if (NULL != error_code) {
				error_code->code = remote_call_result;
				error_code->msg = ipc_g_variant_dup_string(call_result[1]);
			}
		}

		service_adaptor_reply_save_call_log_cb callback = (service_adaptor_reply_save_call_log_cb) task->callback;
		if (NULL != callback) {
			callback(task->handle, request_id, error_code, task->user_data);
		}
		__FREE_ERROR_CODE(error_code);

		_queue_del_task(task);
	} else if (0 == g_strcmp0(signal_name, PRIVATE_DBUS_REPLY_CURRENT_TIME_SIGNAL)) {
		int64_t request_id = 0;
		int64_t current_time_millis = 0;
		service_adaptor_error_s *error_code = NULL;

		__get_current_time_res_type(parameters, &request_id, &current_time_millis);

		service_adaptor_task_h task = _queue_get_task(request_id);

		if (NULL == task) {
			return;
		}

		GVariant *call_result[2];
		call_result[0] = g_variant_get_child_value(parameters, 1);
		call_result[1] = g_variant_get_child_value(parameters, 2);

		uint64_t remote_call_result = g_variant_get_uint64(call_result[0]);

		if (SERVICE_ADAPTOR_ERROR_NONE != remote_call_result) {
			error_code = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));

			if (NULL != error_code) {
				error_code->code = remote_call_result;
				error_code->msg = ipc_g_variant_dup_string(call_result[1]);
			}
		}

		service_adaptor_reply_current_time_cb callback = (service_adaptor_reply_current_time_cb) task->callback;
		if (NULL != callback) {
			callback(task->handle, request_id, current_time_millis, error_code, task->user_data);
		}
		__FREE_ERROR_CODE(error_code);

		_queue_del_task(task);
	}
}

/**	@brief
 *	@return	service_adaptor_error_s
 *	@remarks :
 */
int _dbus_request_create_chatroom(const char *service_name,
						long long int request_id,
						int chat_type,
						long long int *receivers,
						unsigned int receivers_len,
						const char *chatroom_title,
						void *user_data,
						service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();
	ipc_check_proxy(sac_interface_proxy);

	if ((NULL == service_name) || (NULL == receivers) || (1 > receivers_len)) {
		error->code = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
		error->msg = strdup("Invalid Param");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	GVariantBuilder *builder = g_variant_builder_new(G_VARIANT_TYPE("a(x)"));

	for (int i = 0; i < receivers_len; i++) {
		g_variant_builder_add(builder, "(x)", receivers[i]);
	}

	GVariant *request = g_variant_new("(" private_service_adaptor_create_chatroom_req_s_type ")", __safe_add_string(service_name),
			request_id, chat_type, builder, receivers_len, __safe_add_string(chatroom_title));

	g_variant_builder_unref(builder);

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			PRIVATE_DBUS_REQUEST_CREATE_CHATROOM_METHOD,
			request,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
			NULL,
			&g_error);

	if (NULL == call_result) {
		error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;

		if (NULL != g_error) {
			error->msg = __SAFE_STRDUP(g_error->message);
			g_error_free(g_error);
		}
	} else {
		if (FALSE == g_variant_is_of_type(call_result, G_VARIANT_TYPE("(ts)"))) {
			ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->msg = strdup("D-Bus return type error");
		} else {
			__ipc_get_simple_error_code();
		}

		g_variant_unref(call_result);
	}

	return ret;
}

/**	@brief
 *	@return	service_adaptor_error_s
 *	@remarks :
 */
int _dbus_request_change_chatroom_meta(const char *service_name,
						long long int request_id,
						long long int chatroom_id,
						const char *chatroom_title,
						int default_message_ttl,
						void *user_data,
						service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();
	ipc_check_proxy(sac_interface_proxy);

	if (NULL == service_name) {
		error->code = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
		error->msg = strdup("Invalid Param");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	GVariant *request = g_variant_new("(" private_service_adaptor_change_chatroom_meta_req_s_type ")", __safe_add_string(service_name),
			request_id, chatroom_id,  __safe_add_string(chatroom_title), default_message_ttl);

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			PRIVATE_DBUS_REQUEST_CHANGE_CHATROOM_META_METHOD,
			request,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
			NULL,
			&g_error);

	if (NULL == call_result) {
		error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;

		if (NULL != g_error) {
			error->msg = __SAFE_STRDUP(g_error->message);
			g_error_free(g_error);
		}
	} else {
		if (FALSE == g_variant_is_of_type(call_result, G_VARIANT_TYPE("(ts)"))) {
			ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->msg = strdup("D-Bus return type error");
		} else {
			__ipc_get_simple_error_code();
		}

		g_variant_unref(call_result);
	}

	return ret;
}


int _dbus_request_chat(const char *service_name,
						long long int request_id,
						long long int chatroom_id,
						service_adaptor_chat_msg_s **chat_msgs,
						unsigned int chat_msgs_len,
						void *user_data,
						service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();
	ipc_check_proxy(sac_interface_proxy);

	if ((NULL == service_name) || (chat_msgs == NULL) || (1 > chat_msgs_len)) {
		error->code = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
		error->msg = strdup("Invalid Param");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	GVariantBuilder *builder = g_variant_builder_new(G_VARIANT_TYPE("a" private_service_adaptor_chat_msg_s_type));

	for (int i = 0; i < chat_msgs_len; i++) {
		g_variant_builder_open(builder, G_VARIANT_TYPE(private_service_adaptor_chat_msg_s_type));
		g_variant_builder_add(builder, "x", chat_msgs[i]->msg_id);
		g_variant_builder_add(builder, "i", chat_msgs[i]->msg_type);
		__safe_g_variant_builder_add_string(builder, chat_msgs[i]->chatmsg);
		g_variant_builder_add(builder, "i", chat_msgs[i]->message_ttl);
		g_variant_builder_close(builder);
	}

	GVariant *request = g_variant_new("(" private_service_adaptor_chat_req_s_type ")", __safe_add_string(service_name), request_id, chatroom_id, builder, chat_msgs_len);

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			PRIVATE_DBUS_REQUEST_CHAT_METHOD,
			request,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
			NULL,
			&g_error);

	if (NULL == call_result) {
		error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;

		if (NULL != g_error) {
			error->msg = __SAFE_STRDUP(g_error->message);
			g_error_free(g_error);
		}
	} else {
		if (FALSE == g_variant_is_of_type(call_result, G_VARIANT_TYPE("(ts)"))) {
			ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->msg = strdup("D-Bus return type error");
		} else {
			__ipc_get_simple_error_code();
		}

		g_variant_unref(call_result);
	}

	return ret;
}

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
						service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();
	ipc_check_proxy(sac_interface_proxy);

	if ((NULL == service_name)) {
		error->code = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
		error->msg = strdup("Invalid Param");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	GVariant *request = g_variant_new("(" private_service_adaptor_allow_chat_req_s_type ")",
			__safe_add_string(service_name), request_id, chatroom_id, is_auto_allow, max_count,
			need_delivery_ack, last_delivery_ack_timestamp,
			need_read_ack, last_read_ack_timestamp, need_ordered_chat_member_list);

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			PRIVATE_DBUS_REQUEST_ALLOW_CHAT_METHOD,
			request,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
			NULL,
			&g_error);

	if (NULL == call_result) {
		error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;

		if (NULL != g_error) {
			error->msg = __SAFE_STRDUP(g_error->message);
			g_error_free(g_error);
		}
	} else {
		if (FALSE == g_variant_is_of_type(call_result, G_VARIANT_TYPE("(ts)"))) {
			ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->msg = strdup("D-Bus return type error");
		} else {
			__ipc_get_simple_error_code();
		}

		g_variant_unref(call_result);
	}

	return ret;
}

int _dbus_request_all_unread_message(const char *service_name,
						long long int request_id,
						int max_count,
						void *user_data,
						service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();
	ipc_check_proxy(sac_interface_proxy);

	if ((NULL == service_name)) {
		error->code = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
		error->msg = strdup("Invalid Param");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	GVariant *request = g_variant_new("(" private_service_adaptor_all_unread_message_req_s_type ")", __safe_add_string(service_name), request_id, max_count);

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			PRIVATE_DBUS_REQUEST_ALL_UNREAD_MESSAGE_METHOD,
			request,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
			NULL,
			&g_error);

	if (NULL == call_result) {
		error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;

		if (NULL != g_error) {
			error->msg = __SAFE_STRDUP(g_error->message);
			g_error_free(g_error);
		}
	} else {
		if (FALSE == g_variant_is_of_type(call_result, G_VARIANT_TYPE("(ts)"))) {
			ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->msg = strdup("D-Bus return type error");
		} else {
			__ipc_get_simple_error_code();
		}

		g_variant_unref(call_result);
	}

	return ret;
}

int _dbus_reply_forward_online_message(const char *service_name,
						long long int request_id,
						long long int chatroom_id,
						bool mark_as_read,
						void *user_data,
						service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();
	ipc_check_proxy(sac_interface_proxy);

	if ((NULL == service_name)) {
		error->code = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
		error->msg = strdup("Invalid Param");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	GVariant *request = g_variant_new("(" private_service_adaptor_forward_online_message_res_s_type ")", __safe_add_string(service_name), request_id, chatroom_id, mark_as_read);

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			PRIVATE_DBUS_REPLY_FORWARD_ONLINE_MESSAGE_METHOD,
			request,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
			NULL,
			&g_error);

	if (NULL == call_result) {
		error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;

		if (NULL != g_error) {
			error->msg = __SAFE_STRDUP(g_error->message);
			g_error_free(g_error);
		}
	} else {
		if (FALSE == g_variant_is_of_type(call_result, G_VARIANT_TYPE("(ts)"))) {
			ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->msg = strdup("D-Bus return type error");
		} else {
			__ipc_get_simple_error_code();
		}

		g_variant_unref(call_result);
	}

	return ret;
}

int _dbus_reply_forward_unread_message(const char *service_name,
						long long int request_id,
						const char *next_pagination_key,
						int max_count,
						void *user_data,
						service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();
	ipc_check_proxy(sac_interface_proxy);

	if ((NULL == service_name)) {
		error->code = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
		error->msg = strdup("Invalid Param");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	GVariant *request = g_variant_new("(" private_service_adaptor_forward_unread_message_res_s_type ")", __safe_add_string(service_name), request_id, __safe_add_string(next_pagination_key), max_count);

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			PRIVATE_DBUS_REPLY_FORWARD_UNREAD_MESSAGE_METHOD,
			request,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
			NULL,
			&g_error);

	if (NULL == call_result) {
		error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;

		if (NULL != g_error) {
			error->msg = __SAFE_STRDUP(g_error->message);
			g_error_free(g_error);
		}
	} else {
		if (FALSE == g_variant_is_of_type(call_result, G_VARIANT_TYPE("(ts)"))) {
			ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->msg = strdup("D-Bus return type error");
		} else {
			__ipc_get_simple_error_code();
		}

		g_variant_unref(call_result);
	}

	return ret;
}

int _dbus_request_read_message(const char *service_name,
						long long int request_id,
						long long int chatroom_id,
						service_adaptor_inbox_message_s *inbox_msg,
						void *user_data,
						service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();
	ipc_check_proxy(sac_interface_proxy);

	if ((NULL == service_name) || (NULL == inbox_msg)) {
		error->code = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
		error->msg = strdup("Invalid Param");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	GVariant *request = g_variant_new("(" private_service_adaptor_read_message_req_s_type ")", __safe_add_string(service_name), request_id, chatroom_id, inbox_msg->msg_id, inbox_msg->msg_type, inbox_msg->sender, inbox_msg->receiver, inbox_msg->sent_time, __safe_add_string(inbox_msg->chat_msg), inbox_msg->chatroom_id, inbox_msg->chat_type, inbox_msg->message_ttl);

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			PRIVATE_DBUS_REQUEST_READ_MESSAGE_METHOD,
			request,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
			NULL,
			&g_error);

	if (NULL == call_result) {
		error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;

		if (NULL != g_error) {
			error->msg = __SAFE_STRDUP(g_error->message);
			g_error_free(g_error);
		}
	} else {
		if (FALSE == g_variant_is_of_type(call_result, G_VARIANT_TYPE("(ts)"))) {
			ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->msg = strdup("D-Bus return type error");
		} else {
			__ipc_get_simple_error_code();
		}

		g_variant_unref(call_result);
	}

	return ret;
}

int _dbus_request_invite_chat(const char *service_name,
						long long int request_id,
						long long int chatroom_id,
						long long int *inviting_members,
						unsigned int inviting_members_len,
						void *user_data,
						service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();
	ipc_check_proxy(sac_interface_proxy);

	if ((NULL == service_name) || (NULL == inviting_members) || (1 > inviting_members_len)) {
		error->code = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
		error->msg = strdup("Invalid Param");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	GVariantBuilder *builder = g_variant_builder_new(G_VARIANT_TYPE("a(x)"));

	for (int i = 0; i < inviting_members_len; i++) {
		g_variant_builder_add(builder, "(x)", inviting_members[i]);
	}

	GVariant *request = g_variant_new("(" private_service_adaptor_invite_chat_req_s_type ")", __safe_add_string(service_name), request_id, chatroom_id, builder, inviting_members_len);

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			PRIVATE_DBUS_REQUEST_INVITE_CHAT_METHOD,
			request,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
			NULL,
			&g_error);

	if (NULL == call_result) {
		error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;

		if (NULL != g_error) {
			error->msg = __SAFE_STRDUP(g_error->message);
			g_error_free(g_error);
		}
	} else {
		if (FALSE == g_variant_is_of_type(call_result, G_VARIANT_TYPE("(ts)"))) {
			ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->msg = strdup("D-Bus return type error");
		} else {
			__ipc_get_simple_error_code();
		}

		g_variant_unref(call_result);
	}

	return ret;
}

int _dbus_request_end_chat(const char *service_name,
						long long int request_id,
						service_adaptor_end_chat_s **end_chats,
						unsigned int end_chats_len,
						void *user_data,
						service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();
	ipc_check_proxy(sac_interface_proxy);

	if ((NULL == service_name) || (NULL == end_chats) || (1 > end_chats_len)) {
		error->code = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
		error->msg = strdup("Invalid Param");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	GVariantBuilder *builder = g_variant_builder_new(G_VARIANT_TYPE("a" private_service_adaptor_end_chat_s_type));

	for (int i = 0; i < end_chats_len; i++) {
		g_variant_builder_add(builder, private_service_adaptor_end_chat_s_type, end_chats[i]->chatroom_id, end_chats[i]->deny_invitation);
	}

	GVariant *request = g_variant_new("(" private_service_adaptor_end_chat_req_s_type ")", __safe_add_string(service_name), request_id, builder, end_chats_len);

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			PRIVATE_DBUS_REQUEST_END_CHAT_METHOD,
			request,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
			NULL,
			&g_error);

	if (NULL == call_result) {
		error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;

		if (NULL != g_error) {
			error->msg = __SAFE_STRDUP(g_error->message);
			g_error_free(g_error);
		}
	} else {
		if (FALSE == g_variant_is_of_type(call_result, G_VARIANT_TYPE("(ts)"))) {
			ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->msg = strdup("D-Bus return type error");
		} else {
			__ipc_get_simple_error_code();
		}

		g_variant_unref(call_result);
	}

	return ret;
}


int _dbus_request_unseal_message(const char *service_name,
						long long int request_id,
						long long int chatroom_id,
						long long int sender_id,
						long long int message_id,
						const char *message_detail,
						void *user_data,
						service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();
	ipc_check_proxy(sac_interface_proxy);

	if ((NULL == service_name)) {
		error->code = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
		error->msg = strdup("Invalid Param");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	GVariant *request = g_variant_new("(" private_service_adaptor_unseal_message_req_s_type ")", __safe_add_string(service_name),
			request_id, chatroom_id, sender_id, message_id, __safe_add_string(message_detail));

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			PRIVATE_DBUS_REQUEST_UNSEAL_MESSAGE_METHOD,
			request,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
			NULL,
			&g_error);

	if (NULL == call_result) {
		error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;

		if (NULL != g_error) {
			error->msg = __SAFE_STRDUP(g_error->message);
			g_error_free(g_error);
		}
	} else {
		if (FALSE == g_variant_is_of_type(call_result, G_VARIANT_TYPE("(ts)"))) {
			ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->msg = strdup("D-Bus return type error");
		} else {
			__ipc_get_simple_error_code();
		}

		g_variant_unref(call_result);
	}

	return ret;
}


int _dbus_request_save_call_log(const char *service_name,
						long long int request_id,
						long long int chatroom_id,
						const char *call_id,
						const char *call_log_type,
						long long int call_sender_id,
						long long int call_receiver_id,
						int conversaction_second,
						void *user_data,
						service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();
	ipc_check_proxy(sac_interface_proxy);

	if ((NULL == service_name)) {
		error->code = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
		error->msg = strdup("Invalid Param");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	GVariant *request = g_variant_new("(" private_service_adaptor_save_call_log_req_s_type ")", __safe_add_string(service_name),
			request_id, chatroom_id, call_id, call_log_type, call_sender_id, call_receiver_id, conversaction_second);

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			PRIVATE_DBUS_REQUEST_SAVE_CALL_LOG_METHOD,
			request,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
			NULL,
			&g_error);

	if (NULL == call_result) {
		error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;

		if (NULL != g_error) {
			error->msg = __SAFE_STRDUP(g_error->message);
			g_error_free(g_error);
		}
	} else {
		if (FALSE == g_variant_is_of_type(call_result, G_VARIANT_TYPE("(ts)"))) {
			ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->msg = strdup("D-Bus return type error");
		} else {
			__ipc_get_simple_error_code();
		}

		g_variant_unref(call_result);
	}

	return ret;
}

int _dbus_request_current_time(const char *service_name,
						long long int request_id,
						void *user_data,
						service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();
	ipc_check_proxy(sac_interface_proxy);

	if ((NULL == service_name)) {
		error->code = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
		error->msg = strdup("Invalid Param");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	GVariant *request = g_variant_new("(" private_service_adaptor_current_time_req_s_type ")", __safe_add_string(service_name), request_id);

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			PRIVATE_DBUS_REQUEST_CURRENT_TIME_METHOD,
			request,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
			NULL,
			&g_error);

	if (NULL == call_result) {
		error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;

		if (NULL != g_error) {
			error->msg = __SAFE_STRDUP(g_error->message);
			g_error_free(g_error);
		}
	} else {
		if (FALSE == g_variant_is_of_type(call_result, G_VARIANT_TYPE("(ts)"))) {
			ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->msg = strdup("D-Bus return type error");
		} else {
			__ipc_get_simple_error_code();
		}

		g_variant_unref(call_result);
	}

	return ret;
}

int _dbus_request_get_connection_policy(const char *service_name,
						service_adaptor_connection_policy_e *policy,
						service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();
	ipc_check_proxy(sac_interface_proxy);

	if ((NULL == service_name)) {
		error->code = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
		error->msg = strdup("Invalid Param");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	GVariant *request = g_variant_new("(" private_service_adaptor_get_connection_policy_req_s_type ")", __safe_add_string(service_name));

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			PRIVATE_DBUS_REQUEST_GET_CONNECTION_POLICY_METHOD,
			request,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
			NULL,
			&g_error);

	if (NULL == call_result) {
		error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;

		if (NULL != g_error) {
			error->msg = __SAFE_STRDUP(g_error->message);
			g_error_free(g_error);
		}
	} else {
		if (FALSE == g_variant_is_of_type(call_result,
				G_VARIANT_TYPE(MAKE_RETURN_TYPE(private_service_adaptor_connection_policy_res_s_type)))) {
			ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->msg = strdup("D-Bus return type error");
		} else {
			GVariant *call_result_struct[3];
			call_result_struct[0] = g_variant_get_child_value(call_result, 0);
			call_result_struct[1] = g_variant_get_child_value(call_result, 1);
			call_result_struct[2] = g_variant_get_child_value(call_result, 2);

			GVariant *gv_policy = g_variant_get_child_value(call_result_struct[0], 0);
			*policy = (int) g_variant_get_int32(gv_policy);
			g_variant_unref(gv_policy);

			uint64_t remote_call_result = g_variant_get_uint64(call_result_struct[1]);

			if (SERVICE_ADAPTOR_ERROR_NONE != remote_call_result) {
				error->code = remote_call_result;
				error->msg = ipc_g_variant_dup_string(call_result_struct[2]);
				ret = _get_result_code(remote_call_result);
			}

			g_variant_unref(call_result_struct[0]);
			g_variant_unref(call_result_struct[1]);
			g_variant_unref(call_result_struct[2]);
		}

		g_variant_unref(call_result);
	}

	return ret;
}

int _dbus_request_set_connection_policy(const char *service_name,
						service_adaptor_connection_policy_e *policy,
						service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();
	ipc_check_proxy(sac_interface_proxy);

	if ((NULL == service_name)) {
		error->code = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
		error->msg = strdup("Invalid Param");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	GVariant *request = g_variant_new("(" private_service_adaptor_set_connection_policy_req_s_type ")", __safe_add_string(service_name), (int32_t)*policy);

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			PRIVATE_DBUS_REQUEST_SET_CONNECTION_POLICY_METHOD,
			request,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
			NULL,
			&g_error);

	if (NULL == call_result) {
		error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;

		if (NULL != g_error) {
			error->msg = __SAFE_STRDUP(g_error->message);
			g_error_free(g_error);
		}
	} else {
		if (FALSE == g_variant_is_of_type(call_result,
				G_VARIANT_TYPE(MAKE_RETURN_TYPE(private_service_adaptor_connection_policy_res_s_type)))) {
			ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->msg = strdup("D-Bus return type error");
		} else {
			GVariant *call_result_struct[3];
			call_result_struct[0] = g_variant_get_child_value(call_result, 0);
			call_result_struct[1] = g_variant_get_child_value(call_result, 1);
			call_result_struct[2] = g_variant_get_child_value(call_result, 2);

			GVariant *gv_policy = g_variant_get_child_value(call_result_struct[0], 0);
			*policy = (int) g_variant_get_int32(gv_policy);
			g_variant_unref(gv_policy);

			uint64_t remote_call_result = g_variant_get_uint64(call_result_struct[1]);

			if (SERVICE_ADAPTOR_ERROR_NONE != remote_call_result) {
				error->code = remote_call_result;
				error->msg = ipc_g_variant_dup_string(call_result_struct[2]);
				ret = _get_result_code(remote_call_result);
			}

			g_variant_unref(call_result_struct[0]);
			g_variant_unref(call_result_struct[1]);
			g_variant_unref(call_result_struct[2]);
		}

		g_variant_unref(call_result);
	}

	return ret;
}

int _dbus_get_chat_id_list(const char *service_name,
						service_adaptor_phone_number_s **phone_numbers,
						unsigned int phone_numbers_len,
						void *user_data,
						service_adaptor_chat_id_s ***chat_ids,
						unsigned int *chat_ids_len,
						void **server_data,
						service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();
	ipc_check_proxy(sac_interface_proxy);

	if ((NULL == service_name) || (NULL == phone_numbers) || (1 > phone_numbers_len)) {
		error->code = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
		error->msg = strdup("Invalid Param");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	GVariantBuilder *builder = g_variant_builder_new(G_VARIANT_TYPE("a" private_service_adaptor_phone_number_s_type));

	for (int i = 0; i < phone_numbers_len; i++) {
		g_variant_builder_open(builder, G_VARIANT_TYPE(private_service_adaptor_phone_number_s_type));
		__safe_g_variant_builder_add_string(builder, phone_numbers[i]->phonenumber);
		__safe_g_variant_builder_add_string(builder, phone_numbers[i]->ccc);
		g_variant_builder_close(builder);
	}

	GVariant *request = g_variant_new("(" private_service_adaptor_chat_id_list_req_s_type ")", __safe_add_string(service_name), builder, phone_numbers_len);

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			PRIVATE_DBUS_GET_CHAT_ID_LIST_METHOD,
			request,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
			NULL,
			&g_error);

	if (NULL == call_result) {
		error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;

		if (NULL != g_error) {
			error->msg = __SAFE_STRDUP(g_error->message);
			g_error_free(g_error);
		}
	} else {
		if (FALSE == g_variant_is_of_type(call_result, G_VARIANT_TYPE(MAKE_RETURN_TYPE(private_service_adaptor_chat_id_list_res_s_type)))) {
			ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->msg = strdup("D-Bus return type error");
		} else {
			GVariant *call_result_struct[3];
			call_result_struct[0] = g_variant_get_child_value(call_result, 0);
			call_result_struct[1] = g_variant_get_child_value(call_result, 1);
			call_result_struct[2] = g_variant_get_child_value(call_result, 2);

			uint64_t remote_call_result = g_variant_get_uint64(call_result_struct[1]);

			if (SERVICE_ADAPTOR_ERROR_NONE != remote_call_result) {
				error->code = remote_call_result;
				error->msg = ipc_g_variant_dup_string(call_result_struct[2]);
				ret = _get_result_code(remote_call_result);
			} else {
				if ((NULL != chat_ids) && (NULL != chat_ids_len)) {
					__get_chat_id_list_res_type(call_result_struct[0], chat_ids, chat_ids_len, server_data);
				}
			}

			g_variant_unref(call_result_struct[0]);
			g_variant_unref(call_result_struct[1]);
			g_variant_unref(call_result_struct[2]);
		}

		g_variant_unref(call_result);
	}

	return ret;
}

int _dbus_get_msisdn_list(const char *service_name,
						long long int *chat_ids,
						unsigned int chat_ids_len,
						void *user_data,
						service_adaptor_chat_id_s ***msisdns,
						unsigned int *msisdns_len,
						void **server_data,
						service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();
	ipc_check_proxy(sac_interface_proxy);

	if ((NULL == service_name) || (NULL == chat_ids) || (1 > chat_ids_len)) {
		error->code = SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
		error->msg = strdup("Invalid Param");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	GVariantBuilder *builder = g_variant_builder_new(G_VARIANT_TYPE("a(x)"));

	for (int i = 0; i < chat_ids_len; i++) {
		g_variant_builder_add(builder, "(x)", chat_ids[i]);
	}

	GVariant *request = g_variant_new("(" private_service_adaptor_msisdn_list_req_s_type ")", __safe_add_string(service_name), builder, chat_ids_len);

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			PRIVATE_DBUS_GET_MSISDN_LIST_METHOD,
			request,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
			NULL,
			&g_error);

	if (NULL == call_result) {
		error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;

		if (NULL != g_error) {
			error->msg = __SAFE_STRDUP(g_error->message);
			g_error_free(g_error);
		}
	} else {
		if (FALSE == g_variant_is_of_type(call_result, G_VARIANT_TYPE(MAKE_RETURN_TYPE(private_service_adaptor_chat_id_list_res_s_type)))) {
			ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->msg = strdup("D-Bus return type error");
		} else {
			GVariant *call_result_struct[3];
			call_result_struct[0] = g_variant_get_child_value(call_result, 0);
			call_result_struct[1] = g_variant_get_child_value(call_result, 1);
			call_result_struct[2] = g_variant_get_child_value(call_result, 2);

			uint64_t remote_call_result = g_variant_get_uint64(call_result_struct[1]);

			if (SERVICE_ADAPTOR_ERROR_NONE != remote_call_result) {
				error->code = remote_call_result;
				error->msg = ipc_g_variant_dup_string(call_result_struct[2]);
				ret = _get_result_code(remote_call_result);
			} else {
				if ((NULL != msisdns) && (NULL != msisdns_len)) {
					__get_chat_id_list_res_type(call_result_struct[0], msisdns, msisdns_len, server_data);
				}
			}

			g_variant_unref(call_result_struct[0]);
			g_variant_unref(call_result_struct[1]);
			g_variant_unref(call_result_struct[2]);
		}

		g_variant_unref(call_result);
	}

	return ret;
}

