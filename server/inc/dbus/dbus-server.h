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

#ifndef __DBUS_SERVER_H__
#define __DBUS_SERVER_H__

/**
 * Service Adaptor D-Bus server bus name.
 */
#define SERVICE_ADAPTOR_BUS_NAME "org.tizen.serviceadaptor.client"

/**
 * Service Adaptor D-Bus server object path.
 */
#define SERVICE_ADAPTOR_OBJECT_PATH "/org/tizen/serviceadaptor/client"

/**
 * Service Adaptor D-Bus interface.
 */
#define SERVICE_ADAPTOR_INTERFACE "org.tizen.serviceadaptor.client.interface"

/**
 * Service Adaptor Activation start key path
 */
#define SERVICE_ADAPTOR_START_KEY_PATH		"/opt/share/service-adaptor/.fingerprint"

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
///////////////////
///////////////////               private feature
///////////////////

#define private_service_adaptor_essential_s_type_length 1
#define private_service_adaptor_essential_s_type \
	"(" \
	"s" /* char * service_name */ \
	")"

#define private_service_adaptor_external_req_s_type_length 4
#define private_service_adaptor_external_req_s_type \
	"(" \
	"s" /* char * service_name */ \
	"i" /* int32 service_flag */ \
	"s" /* char * api_uri */ \
	service_adaptor_raw_data_s_type \
	")"

#define private_service_adaptor_plugin_s_type_length 2
#define private_service_adaptor_plugin_s_type \
	"(" \
	"s" /* char * name */ \
	"b" /* bool login */ \
	")"

#define private_service_adaptor_set_auth_s_type_length 8
#define private_service_adaptor_set_auth_s_type \
	"(" \
	"s" /* char * service_name */ \
	"s" /* char * imsi */ \
	"s" /* char * cluster_name */ \
	"s" /* char * app_id */ \
	"s" /* char * app_secret */ \
	"s" /* char * user_id */ \
	"s" /* char * user_password */ \
	"u" /* uint32 service_id */ \
	")"

#define private_service_adaptor_contact_info_req_s_type_length 5
#define private_service_adaptor_contact_info_req_s_type \
	"(" \
	"s" /* char * tp  */ \
	"s" /* char * id */ \
	"s" /* char * pn */ \
	"s" /* char * nm */ \
	"s" /* char * cc */ \
	")"

#define private_service_adaptor_contact_req_s_type_length 4
#define private_service_adaptor_contact_req_s_type \
	"(" \
	"s" /* char * service_name */ \
	"x" /* int64 tt */ \
	"a" private_service_adaptor_contact_info_req_s_type \
	"u" /* uint32 cts_len */ \
	")"

#define private_service_adaptor_contact_info_res_s_type_length 21
#define private_service_adaptor_contact_info_res_s_type \
	"(" \
	"s" /* char * duid  */ \
	"s" /* char * id */ \
	"s" /* char * msisdn */ \
	"s" /* char * ty */ \
	"s" /* char * cc */ \
	"s" /* char * pn */ \
	"s" /* char * nm */ \
	"a(s)" /* char ** evnt */ \
	"u" /* uint32 evnt_len */ \
	"a(is)" /* char * imgs */ \
	"u" /* uint32 imgs_len */ \
	"a(s)" /* char ** adrs */ \
	"u" /* uint32 adrs_len */ \
	"a(s)" /* char ** mail */ \
	"u" /* uint32 mail_len */ \
	"s" /* char * org */ \
	"s" /* char * prsc */ \
	"s" /* char * status */ \
	"u" /* uint32 sids */ \
	"i" /* int32_t profile_type */ \
	"s" /* char * profile_url */ \
	")"

#define private_service_adaptor_contact_res_s_type_length 3
#define private_service_adaptor_contact_res_s_type \
	"(" \
	"x" /* int64 tt */ \
	"a" private_service_adaptor_contact_info_res_s_type \
	"u" /* uint32 cts_len */ \
	")"

#define private_service_adaptor_profile_req_s_type_length 14
#define private_service_adaptor_profile_req_s_type \
	"(" \
	"s" /* char * service_name */ \
	"s" /* char * cc */ \
	"s" /* char * pn */ \
	"s" /* char * nm */ \
	"a(s)" /* char ** evnt */ \
	"u" /* uint32 evnt_len */ \
	"s" /* char * img */ \
	"a(s)" /* char ** adrs */ \
	"u" /* uint32 adrs_len */ \
	"a(s)" /* char ** mail */ \
	"u" /* uint32 mail_len */ \
	"s" /* char * org */ \
	"s" /* char * prsc */ \
	"s" /* char * status */ \
	")"

#define private_service_adaptor_profile_res_s_type_length 4
#define private_service_adaptor_profile_res_s_type \
	"(" \
	"s" /* char * nm */ \
	"s" /* char * img */ \
	"s" /* char * prsc */ \
	"s" /* char * status */ \
	")"

#define private_service_adaptor_privacy_info_req_s_type_length 2
#define private_service_adaptor_privacy_info_req_s_type \
	"(" \
	"s" /* char * cc */ \
	"s" /* char * pn */ \
	")"

#define private_service_adaptor_privacy_req_s_type_length 4
#define private_service_adaptor_privacy_req_s_type \
	"(" \
	"s" /* char * service_name */ \
	"u" /* uint32 lvl */ \
	"a" private_service_adaptor_privacy_info_req_s_type \
	"u" /* uint32 cts_len */ \
	")"

#define private_service_adaptor_privacy_res_s_type_length 2
#define private_service_adaptor_privacy_res_s_type \
	"(" \
	"u" /* uint32 lvl */ \
	"u" /* uint32 prscon */ \
	")"

#define private_service_adaptor_presence_req_s_type_length 4
#define private_service_adaptor_presence_req_s_type \
	"(" \
	"s" /* char * service_name */ \
	"s" /* char * prsc */ \
	"s" /* char * status */ \
	"u" /* uint32 prscon */ \
	")"

#define private_service_adaptor_contact_profile_image_req_s_type_length 3
#define private_service_adaptor_contact_profile_image_req_s_type \
	"(" \
	"s" /* char * service_name */ \
	"a(iis)" /* images array */ \
	"u" /* uint32 images length */ \
	")"

#define private_service_adaptor_set_me_profile_type_req_s_type_length 2
#define private_service_adaptor_set_me_profile_type_req_s_type \
	"(" \
	"s" /* */ \
	"i" /* */ \
	")"


#define private_service_adaptor_file_path_req_s_type_length 3
#define private_service_adaptor_file_path_req_s_type \
	"(" \
	"s" /* char * service_name */ \
	"a(s)" /* char ** file_paths */ \
	"u" /* uint32 file_paths_len */ \
	")"

#define private_service_adaptor_file_path_res_s_type_length 2
#define private_service_adaptor_file_path_res_s_type \
	"(" \
	"a(s)" /* char ** file_paths */ \
	"u" /* uint32 file_paths_len */ \
	")"

#define private_service_adaptor_file_s_type_length 1
#define private_service_adaptor_file_s_type \
	"(" \
	"i" /* int32 file_description */ \
	")"

#define private_service_adaptor_file_publish_s_type_length 1
#define private_service_adaptor_file_publish_s_type \
	"(" \
	"s" /* char * publish_url */ \
	")"

#define private_service_adaptor_file_transfer_req_s_type_length 3
#define private_service_adaptor_file_transfer_req_s_type \
	"(" \
	"s" /* char * service_name */ \
	"s" /* char * server_path */ \
	"s" /* char * download_path */ \
	")"

#define private_service_adaptor_file_status_req_s_type_length 2
#define private_service_adaptor_file_status_req_s_type \
	"(" \
	"s" /* char * service_name */ \
	"i" /* int32 file_description */ \
	")"

#define private_service_adaptor_privilege_check_req_s_type_length 2
#define private_service_adaptor_privilege_check_req_s_type \
	"(" \
	"s" /* char * service_name */ \
	"s" /* char * privilege_name */ \
	")"

#define private_service_adaptor_file_status_res_s_type_length 3
#define private_service_adaptor_file_status_res_s_type \
	"(" \
	"x" /* int64 total_size */ \
	"x" /* int64 transferred_size */ \
	"x" /* int64 status */ \
	")"

#define private_service_adaptor_did_violation_users_s_type_length 2
#define private_service_adaptor_did_violation_users_s_type \
	"(" \
	"x" /* int64 usera */ \
	"x" /* int64 userb */ \
	")"

#define private_service_adaptor_wrong_receiver_s_type_length 12
#define private_service_adaptor_wrong_receiver_s_type \
	"(" \
	"a(x)" /* int64 * invalid_receivers */ \
	"u" /* uint32 invalid_receivers_len */ \
	"a(x)" /* int64 * interrupted_receivers */ \
	"u" /* uint32 interrupted_receivers_len */ \
	"a(x)" /* int64 * disabled_receivers */ \
	"u" /* uint32 disabled_receivers_len */ \
	"a(x)" /* int64 * existing_chatmembers */ \
	"u" /* uint32 existing_chatmembers_len */ \
	"a" private_service_adaptor_did_violation_users_s_type \
	"u" /* uint32 did_violation_users_len */ \
	"a(x)" /* int64 * invitation_denieds */ \
	"u" /* uint32 invitation_denieds_len */ \
	")"

#define private_service_adaptor_chat_msg_s_type_length 4
#define private_service_adaptor_chat_msg_s_type \
	"(" \
	"x" /* int64 msg_id */ \
	"i" /* int msg_type */ \
	"s" /* char * chatmsg */ \
	"i" /* int32 message_ttl */ \
	")"

#define private_service_adaptor_processed_msg_s_type_length 2
#define private_service_adaptor_processed_msg_s_type \
	"(" \
	"x" /* int64 msg_id */ \
	"x" /* int64 sent_time */ \
	")"

#define private_service_adaptor_delivery_ack_s_type_length 3
#define private_service_adaptor_delivery_ack_s_type \
	"(" \
	"x" /* int64 user_id */ \
	"x" /* int64 msg_id */ \
	"t" /* int64 timestamp */ \
	")"

#define private_service_adaptor_read_ack_s_type_length 3
#define private_service_adaptor_read_ack_s_type \
	"(" \
	"x" /* int64 user_id */ \
	"x" /* int64 msg_id */ \
	"t" /* int64 timestamp */ \
	")"

#define private_service_adaptor_ordered_chat_member_s_type_length 3
#define private_service_adaptor_ordered_chat_member_s_type \
	"(" \
	"x" /* int64 user_id */ \
	"b" /* bool available */ \
	"s" /* char * name */ \
	")"

#define private_service_adaptor_inbox_message_s_type_length 9
#define private_service_adaptor_inbox_message_s_type \
	"(" \
	"x" /* int64 msg_id */ \
	"i" /* int msg_type */ \
	"x" /* int64 sender */ \
	"x" /* int64 receiver */ \
	"x" /* int64 sent_time */ \
	"s" /* char * chat_msg */ \
	"x" /* int64 chatroom_id */ \
	"i" /* int chat_type */ \
	"i" /* int32 message_ttl */ \
	")"

#define private_service_adaptor_end_chat_s_type_length 2
#define private_service_adaptor_end_chat_s_type \
	"(" \
	"x" /* int64 chatroom_id */ \
	"b" /* boolean deny_invitation */ \
	")"

#define private_service_adaptor_phone_number_s_type_length 2
#define private_service_adaptor_phone_number_s_type \
	"(" \
	"s" /* char * phonenumber */ \
	"s" /* char * ccc */ \
	")"

#define private_service_adaptor_chat_id_s_type_length 2
#define private_service_adaptor_chat_id_s_type \
	"(" \
	"x" /* int64 chatid */ \
	"s" /* char * msisdn */ \
	")"

#define private_service_adaptor_create_chatroom_req_s_type_length 6
#define private_service_adaptor_create_chatroom_req_s_type \
	"(" \
	"s" /* char * service_name */ \
	"x" /* int64 request_id */ \
	"i" /* int chat_type */ \
	"a(x)" /* int64 * receivers */ \
	"u" /* uint32 receivers_len */ \
	"s" /* char * chatroom_title */ \
	")"

#define private_service_adaptor_create_chatroom_res_s_type_length 4
#define private_service_adaptor_create_chatroom_res_s_type \
	"(" \
	"x" /* int64 request_id */ \
	"x" /* int64 chatroom_id */ \
	"i" /* int32 default_message_ttl */ \
	private_service_adaptor_wrong_receiver_s_type \
	")"

#define private_service_adaptor_change_chatroom_meta_req_s_type_length 5
#define private_service_adaptor_change_chatroom_meta_req_s_type \
	"(" \
	"s" /* char * service_name */ \
	"x" /* int64 request_id */ \
	"x" /* int64 chatroom_id */ \
	"s" /* char * chatroom_title */ \
	"i" /* int32 default_message_ttl */ \
	")"

#define private_service_adaptor_change_chatroom_meta_res_s_type_length 2
#define private_service_adaptor_change_chatroom_meta_res_s_type \
	"(" \
	"x" /* int64 request_id */ \
	"x" /* int64 chatroom_id */ \
	")"

#define private_service_adaptor_chat_req_s_type_length 5
#define private_service_adaptor_chat_req_s_type \
	"(" \
	"s" /* char * service_name */ \
	"x" /* int64 request_id */ \
	"x" /* int64 chatroom_id */ \
	"a" private_service_adaptor_chat_msg_s_type \
	"u" /* uint32 chat_msgs_len */ \
	")"

#define private_service_adaptor_chat_res_s_type_length 4
#define private_service_adaptor_chat_res_s_type \
	"(" \
	"x" /* int64 request_id */ \
	"x" /* int64 chatroom_id */ \
	"a" private_service_adaptor_processed_msg_s_type \
	"u" /* uint32 processed_msgs_len */ \
	")"

#define private_service_adaptor_allow_chat_req_s_type_length 10
#define private_service_adaptor_allow_chat_req_s_type \
	"(" \
	"s" /* char * service_name */ \
	"x" /* int64 request_id */ \
	"x" /* int64 chatroom_id */ \
	"b" /* bool is_auth_allow */ \
	"i" /* int max_count */ \
	"b" /* bool need_delivery_ack */ \
	"t" /* long need_delivery_ack_timestamp */ \
	"b" /* bool need_read_ack */ \
	"t" /* long last_read_ack_timestamp */ \
	"b" /* bool need_ordered_chat_member_list */ \
	")"

#define private_service_adaptor_allow_chat_res_s_type_length 12
#define private_service_adaptor_allow_chat_res_s_type \
	"(" \
	"x" /* int64 request_id */ \
	"x" /* int64 chatroom_id */ \
	"a" private_service_adaptor_delivery_ack_s_type \
	"u" /* uint32 delivery_acks_len */ \
	"t" /* uint64 last_delivery_acks_timestamp */ \
	"a" private_service_adaptor_read_ack_s_type \
	"u" /* uint32 read_acks_len */ \
	"t" /* uint64 last_read_acks_timestamp */ \
	"a" private_service_adaptor_ordered_chat_member_s_type \
	"u" /* uint32 ordered_chat_members_len */ \
	"s" /* char * chatroom_title */ \
	"i" /* int32 default_message_ttl */ \
	")"

#define private_service_adaptor_all_unread_message_req_s_type_length 3
#define private_service_adaptor_all_unread_message_req_s_type \
	"(" \
	"s" /* char * service_name */ \
	"x" /* int64 request_id */ \
	"i" /* int max_count */ \
	")"

#define private_service_adaptor_all_unread_message_res_s_type_length 1
#define private_service_adaptor_all_unread_message_res_s_type \
	"(" \
	"x" /* int64 request_id */ \
	")"

#define private_service_adaptor_forward_online_message_req_s_type_length 5
#define private_service_adaptor_forward_online_message_req_s_type \
	"(" \
	"x" /* int64 request_id */ \
	"x" /* int64 chatroom_id */ \
	"i" /* int chat_type */ \
	private_service_adaptor_inbox_message_s_type \
	"b" /* bool skip_reply */ \
	")"

#define private_service_adaptor_forward_online_message_res_s_type_length 4
#define private_service_adaptor_forward_online_message_res_s_type \
	"(" \
	"s" /* char * service_name */ \
	"x" /* int64 request_id */ \
	"x" /* int64 chatroom_id */ \
	"b" /* bool mark_as_read */ \
	")"

#define private_service_adaptor_forward_unread_message_req_s_type_length 4
#define private_service_adaptor_forward_unread_message_req_s_type \
	"(" \
	"x" /* int64 request_id */ \
	"a" private_service_adaptor_inbox_message_s_type \
	"u" /* uint32 inbox_msgs_len */ \
	"s" /* char * next_pagination_key */ \
	")"

#define private_service_adaptor_forward_unread_message_res_s_type_length 4
#define private_service_adaptor_forward_unread_message_res_s_type \
	"(" \
	"s" /* char * service_name */ \
	"x" /* int64 request_id */ \
	"s" /* char * next_pagination_key */ \
	"i" /* int max_count */ \
	")"

#define private_service_adaptor_read_message_req_s_type_length 4
#define private_service_adaptor_read_message_req_s_type \
	"(" \
	"s" /* char * service_name */ \
	"x" /* int64 request_id */ \
	"x" /* int64 chatroom_id */ \
	private_service_adaptor_inbox_message_s_type \
	")"

#define private_service_adaptor_read_message_res_s_type_length 2
#define private_service_adaptor_read_message_res_s_type \
	"(" \
	"x" /* int64 request_id */ \
	"x" /* int64 chatroom_id */ \
	")"

#define private_service_adaptor_invite_chat_req_s_type_length 5
#define private_service_adaptor_invite_chat_req_s_type \
	"(" \
	"s" /* char * service_name */ \
	"x" /* int64 request_id */ \
	"x" /* int64 chatroom_id */ \
	"a(x)" /* int64 *inviting_members */ \
	"u" /* uint32 inviting_members_len */ \
	")"

#define private_service_adaptor_invite_chat_res_s_type_length 4
#define private_service_adaptor_invite_chat_res_s_type \
	"(" \
	"x" /* int64 request_id */ \
	"x" /* int64 chatroom_id */ \
	"x" /* int64 sent_time */ \
	private_service_adaptor_wrong_receiver_s_type \
	")"

#define private_service_adaptor_end_chat_req_s_type_length 4
#define private_service_adaptor_end_chat_req_s_type \
	"(" \
	"s" /* char * service_name */ \
	"x" /* int64 request_id */ \
	"a" private_service_adaptor_end_chat_s_type \
	"u" /* uint32 end_chats_len */ \
	")"

#define private_service_adaptor_end_chat_res_s_type_length 1
#define private_service_adaptor_end_chat_res_s_type \
	"(" \
	"x" /* int64 request_id */ \
	")"

#define private_service_adaptor_unseal_message_req_s_type_length 6
#define private_service_adaptor_unseal_message_req_s_type \
	"(" \
	"s" /* char * service_name */ \
	"x" /* int64 request_id */ \
	"x" /* int64 chatroom_id */ \
	"x" /* int64 sender_id */ \
	"x" /* int64 message_id */ \
	"s" /* char * message_detail */ \
	")"

#define private_service_adaptor_unseal_message_res_s_type_length 2
#define private_service_adaptor_unseal_message_res_s_type \
	"(" \
	"x" /* int64 request_id */ \
	"x" /* int64 chatroom_id */ \
	")"

#define private_service_adaptor_save_call_log_req_s_type_length 8
#define private_service_adaptor_save_call_log_req_s_type \
	"(" \
	"s" /* char * service_name */ \
	"x" /* int64 request_id */ \
	"x" /* int64 chatroom_id */ \
	"s" /* char * call_id */ \
	"s" /* char * call_log_type */ \
	"x" /* int64 call_sender_id */ \
	"x" /* int64 call_receiver_id */ \
	"i" /* int conversaction_second */ \
	")"

#define private_service_adaptor_save_call_log_res_s_type_length 1
#define private_service_adaptor_save_call_log_res_s_type \
	"(" \
	"x" /* int64 request_id */ \
	")"

#define private_service_adaptor_current_time_req_s_type_length 2
#define private_service_adaptor_current_time_req_s_type \
	"(" \
	"s" /* char * service_name */ \
	"x" /* int64 request_id */ \
	")"

#define private_service_adaptor_current_time_res_s_type_length 2
#define private_service_adaptor_current_time_res_s_type \
	"(" \
	"x" /* int64 request_id */ \
	"x" /* int64 current_time_millis */ \
	")"

#define private_service_adaptor_get_connection_policy_req_s_type_length 1
#define private_service_adaptor_get_connection_policy_req_s_type \
	"(" \
	"s" /* char * service_name */ \
	")"

#define private_service_adaptor_set_connection_policy_req_s_type_length 2
#define private_service_adaptor_set_connection_policy_req_s_type \
	"(" \
	"s" /* char * service_name */ \
	"i" /* int32 connection_policy */ \
	")"

#define private_service_adaptor_connection_policy_res_s_type_length 1
#define private_service_adaptor_connection_policy_res_s_type \
	"(" \
	"i" /* int32 connection_policy */ \
	")"

#define private_service_adaptor_channel_disconnected_res_s_type_length 1
#define private_service_adaptor_channel_disconnected_res_s_type \
	"(" \
	"s" /* char * service_name */ \
	")"

#define private_service_adaptor_chat_id_list_req_s_type_length 3
#define private_service_adaptor_chat_id_list_req_s_type \
	"(" \
	"s" /* char * service_name */ \
	"a" private_service_adaptor_phone_number_s_type \
	"u" /* uint32 phone_number_len */ \
	")"

#define private_service_adaptor_chat_id_list_res_s_type_length 2
#define private_service_adaptor_chat_id_list_res_s_type \
	"(" \
	"a" private_service_adaptor_chat_id_s_type \
	"u" /* uint32 chat_ids_len */ \
	")"

#define private_service_adaptor_msisdn_list_req_s_type_length 3
#define private_service_adaptor_msisdn_list_req_s_type \
	"(" \
	"s" /* char * service_name */ \
	"a(x)" /* int64 * chat_ids */ \
	"u" /* uint32 chat_ids_len */ \
	")"

#define private_service_adaptor_msisdn_list_res_s_type_length 2
#define private_service_adaptor_msisdn_list_res_s_type \
	"(" \
	"a" private_service_adaptor_chat_id_s_type \
	"u" /* uint32 msisdns_len */ \
	")"

#define private_service_adaptor_shop_info_s_type_length 9
#define private_service_adaptor_shop_info_s_type \
	"(" \
	"i" /* int category_id */ \
	"i" /* long item_id */ \
	"i" /* long sticker_id */ \
	"s" /* char * lang_cd */ \
	"s" /* char * cntry_cd */ \
	"i" /* int rwidth */ \
	"i" /* int rheight */ \
	"i" /* int start_idx */ \
	"i" /* int count */ \
	")"

#define private_service_adaptor_shop_item_s_type_length 17
#define private_service_adaptor_shop_item_s_type \
	"(" \
	"i" /* long item_id */ \
	"i" /* int category_id */ \
	"a(i)" /* long * sticker_ids */ \
	"u" /* uint32 sticker_ids_len */ \
	"s" /* char * title */ \
	"s" /* char * character */ \
	"i" /* int version */ \
	"s" /* char * download_url */ \
	"s" /* char * panel_url */ \
	"s" /* char * sticker_url */ \
	"i" /* long file_size */ \
	"i" /* int count */ \
	"s" /* char * character_code */ \
	"x" /* int64 startdate */ \
	"x" /* int64 enddate */ \
	"x" /* int64 expired_date */ \
	"x" /* int64 valid_period */ \
	")"

#define private_service_adaptor_shop_req_s_type_length 2
#define private_service_adaptor_shop_req_s_type \
	"(" \
	"s" /* char * service_name */ \
	private_service_adaptor_shop_info_s_type \
	")"

#define private_service_adaptor_shop_res_s_type_length 2
#define private_service_adaptor_shop_res_s_type \
	"(" \
	"a" private_service_adaptor_shop_item_s_type \
	"u" /* uint32 items_len */ \
	")"

#define private_service_adaptor_file_progress_s_type_length 3
#define private_service_adaptor_file_progress_s_type \
	"(" \
	"i" /* int32 file_description */ \
	"t" /* uint64 progress_size */ \
	"t" /* uint64 total_size */ \
	")"

#define private_service_adaptor_file_transfer_completion_s_type_length 2
#define private_service_adaptor_file_transfer_completion_s_type \
	"(" \
	"i" /* int32 file_description */ \
	"s" /* char * publish_url */ \
	")"

#define private_service_adaptor_push_data_s_type_length 4
#define private_service_adaptor_push_data_s_type \
	"(" \
	"u" /* uint32 service_id */ \
	"s" /* char * data */ \
	"s" /* char * msg */ \
	"x" /* int64 time_stamp */ \
	")"

#define private_service_adaptor_push_register_req_s_type_length 1
#define private_service_adaptor_push_register_req_s_type \
	"(" \
	"s" /* char *service_file_name */ \
	")"

#define private_service_adaptor_push_deregister_req_s_type_length 1
#define private_service_adaptor_push_deregister_req_s_type \
	"(" \
	"s" /* char *service_file_name */ \
	")"

///////////////////
///////////////////               private feature
///////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
///////////////////
///////////////////               public feature
///////////////////


#define service_adaptor_essential_s_type_length 1
#define service_adaptor_essential_s_type \
	"(" \
	"s" /* char * service_name */ \
	")"

#define service_adaptor_plugin_s_type_length 2
#define service_adaptor_plugin_s_type \
	"(" \
	"s" /* char * plugin_uri */ \
	"i" /* int32_t installed_mask */ \
	")"

#define service_adaptor_is_auth_req_s_type_length 2
#define service_adaptor_is_auth_req_s_type \
	"(" \
	"s" /* char * plugin_uri */ \
	service_adaptor_raw_data_s_type \
	")"

#define service_adaptor_join_req_s_type_length 2
#define service_adaptor_join_req_s_type \
	"(" \
	"s" /* char * plugin_uri */ \
	service_adaptor_raw_data_s_type \
	")"

#define service_adaptor_set_auth_s_type_length 9
#define service_adaptor_set_auth_s_type \
	"(" \
	"a(y)" /* char array security_cookie */ \
	"a(y)" /* char array plugin_property */ \
	"s" /* char * service_name */ \
	"s" /* char * plugin_uri */ \
	"s" /* char * app_id */ \
	"s" /* char * app_secret */ \
	"s" /* char * user_id */ \
	"s" /* char * user_password */ \
	"i" /* int32_t enable_mask */ \
	")"

#define service_adaptor_raw_data_s_type \
	"a(y)" /* unsigned char * raw_data*/ \

#define service_adaptor_file_path_req_s_type_length 3
#define service_adaptor_file_path_req_s_type \
	"(" \
	"s" /* char * service_name */ \
	"a(s)" /* char ** file_paths */ \
	"u" /* uint32 file_paths_len */ \
	")"

#define service_adaptor_file_path_res_s_type_length 2
#define service_adaptor_file_path_res_s_type \
	"(" \
	"a(s)" /* char ** file_paths */ \
	"u" /* uint32 file_paths_len */ \
	")"

#define service_adaptor_file_s_type_length 1
#define service_adaptor_file_s_type \
	"(" \
	"i" /* int32 file_description */ \
	")"

#define service_adaptor_content_meta_s_type_length 18
#define service_adaptor_content_meta_s_type \
	"(" \
	"s" /* char * mime_type  */ \
	"s" /* char * title  */ \
	"s" /* char * album  */ \
	"s" /* char * artist  */ \
	"s" /* char * genere  */ \
	"s" /* char * recorded_date  */ \
	"i" /* int32  width */ \
	"i" /* int32  height */ \
	"i" /* int32  duration */ \
	"s" /* char * copyright  */ \
	"s" /* char * track_num  */ \
	"s" /* char * description  */ \
	"s" /* char * composer  */ \
	"s" /* char * year  */ \
	"i" /* int32  bitrate */ \
	"i" /* int32  samplerate */ \
	"i" /* int32  channel */ \
	"s" /* char * extra_media_meta  */ \
	")"

#define service_adaptor_cloud_meta_s_type_length 4
#define service_adaptor_cloud_meta_s_type \
	"(" \
	"s" /* char * service_name  */ \
	"t" /* uint64  usage_byte */ \
	"t" /* uint64  quota_byte */ \
	"s" /* char *  extra_cloud_meta */ \
	")"

#define service_adaptor_file_info_s_type_length 11
#define service_adaptor_file_info_s_type \
	"(" \
	"s" /* char * plugin_name  */ \
	"s" /* char * object_id  */ \
	"s" /* char * storage_path  */ \
	"t" /* uint64  file_size */ \
	"t" /* uint64  created_time */ \
	"t" /* uint64  modified_time */ \
	"i" /* int32  file_info_index */ \
	"i" /* int32  content_type */ \
	service_adaptor_content_meta_s_type \
	service_adaptor_cloud_meta_s_type \
	"s" /* char *  extra_file_info */ \
	")"

#define service_adaptor_file_publish_s_type_length 1
#define service_adaptor_file_publish_s_type \
	"(" \
	"s" /* char * publish_url */ \
	")"

#define service_adaptor_file_transfer_req_s_type_length 3
#define service_adaptor_file_transfer_req_s_type \
	"(" \
	"s" /* char * service_name */ \
	"s" /* char * server_path */ \
	"s" /* char * download_path */ \
	")"

#define service_adaptor_download_thumbnail_req_s_type_length 4
#define service_adaptor_download_thumbnail_req_s_type \
	"(" \
	"s" /* char * service_name */ \
	"s" /* char * server_path */ \
	"s" /* char * download_path */ \
	"i" /* int32 thumbnail_size */ \
	")"

#define service_adaptor_file_status_req_s_type_length 2
#define service_adaptor_file_status_req_s_type \
	"(" \
	"s" /* char * service_name */ \
	"i" /* int32 file_description */ \
	")"

#define service_adaptor_file_status_res_s_type_length 3
#define service_adaptor_file_status_res_s_type \
	"(" \
	"x" /* int64 total_size */ \
	"x" /* int64 transferred_size */ \
	"x" /* int64 status */ \
	")"
#define service_adaptor_get_root_folder_path_req_s_type_length 1
#define service_adaptor_get_root_folder_path_req_s_type \
	"(" \
	"s" /* char * service_name */ \
	")"
#define service_adaptor_get_root_folder_path_res_s_type_length 1
#define service_adaptor_get_root_folder_path_res_s_type \
	"(" \
	"s" /* char * root_folder_path */ \
	")"
#define service_adaptor_make_directory_req_s_type_length 2
#define service_adaptor_make_directory_req_s_type \
	"(" \
	"s" /* char * service_name */ \
	"s" /* char * folder_path */ \
	")"
#define service_adaptor_remove_file_req_s_type_length 2
#define service_adaptor_remove_file_req_s_type \
	"(" \
	"s" /* char * service_name */ \
	"s" /* char * file_path */ \
	")"
#define service_adaptor_remove_directory_req_s_type_length 2
#define service_adaptor_remove_directory_req_s_type \
	"(" \
	"s" /* char * service_name */ \
	"s" /* char * folder_path */ \
	")"
#define service_adaptor_move_file_req_s_type_length 3
#define service_adaptor_move_file_req_s_type \
	"(" \
	"s" /* char * service_name */ \
	"s" /* char * src_file_path */ \
	"s" /* char * dst_file_path */ \
	")"
#define service_adaptor_move_directory_req_s_type_length 3
#define service_adaptor_move_directory_req_s_type \
	"(" \
	"s" /* char * service_name */ \
	"s" /* char * src_folder_path */ \
	"s" /* char * dst_folder_path */ \
	")"
#define service_adaptor_get_file_list_req_s_type_length 2
#define service_adaptor_get_file_list_req_s_type \
	"(" \
	"s" /* char * service_name */ \
	"s" /* char * parent_path */ \
	")"
#define service_adaptor_open_upload_file_req_s_type_length 3
#define service_adaptor_open_upload_file_req_s_type \
	"(" \
	"s" /* char * service_name */ \
	"s" /* char * local_path */ \
	"s" /* char * upload_path */ \
	")"
#define service_adaptor_open_download_file_req_s_type_length 3
#define service_adaptor_open_download_file_req_s_type \
	"(" \
	"s" /* char * service_name */ \
	"s" /* char * storage_path */ \
	"s" /* char * local_path */ \
	")"
#define service_adaptor_open_download_thumbnail_req_s_type_length 4
#define service_adaptor_open_download_thumbnail_req_s_type \
	"(" \
	"s" /* char * service_name */ \
	"s" /* char * storage_path */ \
	"s" /* char * local_path */ \
	"i" /* int32_t thumbnail_size */ \
	")"
#define service_adaptor_close_file_req_s_type_length 2
#define service_adaptor_close_file_req_s_type \
	"(" \
	"s" /* char * service_name */ \
	service_adaptor_file_descriptor_s_type \
	")"
// jwkim async5
#define service_adaptor_start_upload_file_req_s_type_length 5
#define service_adaptor_start_upload_file_req_s_type \
	"(" \
	"s" /* char * service_name */ \
	service_adaptor_file_descriptor_s_type \
	"s" /* char * storage_path */ \
	"b" /* bool need_progress */ \
	"b" /* bool need_state */ \
	")"
#define service_adaptor_start_download_file_req_s_type_length 5
#define service_adaptor_start_download_file_req_s_type \
	"(" \
	"s" /* char * service_name */ \
	service_adaptor_file_descriptor_s_type \
	"s" /* char * storage_path */ \
	"b" /* bool need_progress */ \
	"b" /* bool need_state */ \
	")"
#define service_adaptor_start_download_thumbnail_req_s_type_length 6
#define service_adaptor_start_download_thumbnail_req_s_type \
	"(" \
	"s" /* char * service_name */ \
	service_adaptor_file_descriptor_s_type \
	"s" /* char * storage_path */ \
	"i" /* int32_t thumbnail_size */ \
	"b" /* bool need_progress */ \
	"b" /* bool need_state */ \
	")"

#define service_adaptor_cancel_file_task_req_s_type_length 2
#define service_adaptor_cancel_file_task_req_s_type \
	"(" \
	"s" /* char * service_name */ \
	service_adaptor_file_descriptor_s_type \
	")"

#define service_adaptor_file_descriptor_s_type_length 1
#define service_adaptor_file_descriptor_s_type \
	"(" \
	"x" /* int64_t file_uid */ \
	")"

#define service_adaptor_get_file_list_res_s_type_length 2
#define service_adaptor_get_file_list_res_s_type \
	"(" \
	"a" service_adaptor_file_info_s_type \
	"u" /* uint32 file_info_len */ \
	")"

#define service_adaptor_file_progress_s_type_length 3
#define service_adaptor_file_progress_s_type \
	"(" \
	service_adaptor_file_descriptor_s_type \
	"t" /* uint64 progress_size */ \
	"t" /* uint64 total_size */ \
	")"

#define service_adaptor_file_transfer_state_changed_s_type_length 2
#define service_adaptor_file_transfer_state_changed_s_type \
	"(" \
	service_adaptor_file_descriptor_s_type \
	"i" /* int32_t state */ \
	")"

#define service_adaptor_push_data_s_type_length 4
#define service_adaptor_push_data_s_type \
	"(" \
	"u" /* uint32 service_id */ \
	"s" /* char * data */ \
	"s" /* char * msg */ \
	"x" /* int64 time_stamp */ \
	")"

///////////////////
///////////////////               public feature
///////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
///////////////////
///////////////////               private feature
///////////////////

/**
 * array of structures
 */
#define private_plugin_list_type			"a(sb)"
#define private_contact_info_req_list_type		"a" private_service_adaptor_contact_info_req_s_type
#define private_contact_info_res_list_type		"a" private_service_adaptor_contact_info_res_s_type
#define private_shop_item_res_list_type			"a" private_service_adaptor_shop_item_s_type
#define private_message_chat_id_list_type		"a" private_service_adaptor_chat_id_s_type
#define private_message_processed_msg_list_type		"a" private_service_adaptor_processed_msg_s_type
#define private_message_did_violation_users_list_type	"a" private_service_adaptor_did_violation_users_s_type
#define private_message_delivery_ack_list_type		"a" private_service_adaptor_delivery_ack_s_type
#define private_message_read_ack_list_type		"a" private_service_adaptor_read_ack_s_type
#define private_message_ordered_chat_member_list_type	"a" private_service_adaptor_ordered_chat_member_s_type
#define private_message_inbox_message_list_type		"a" private_service_adaptor_inbox_message_s_type


///////////////////
///////////////////               private feature
///////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
///////////////////
///////////////////               public feature
///////////////////


#define plugin_list_type			"a" service_adaptor_plugin_s_type
#define storage_file_info_list_type		"a" service_adaptor_file_info_s_type

///////////////////
///////////////////               public feature
///////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
///////////////////
///////////////////               common
///////////////////


/**
 * append error code to the type
 */
#define MAKE_RETURN_TYPE(x)			"(" x "ts)"

/**
 * DBus APIs
 */
#define DBUS_SERVICE_ADAPTOR	"dbus_00"
#define DBUS_AUTH_ADAPTOR 	"dbus_01"
#define DBUS_CONTACT_ADAPTOR 	"dbus_02"
#define DBUS_MESSAGE_ADAPTOR 	"dbus_03"
#define DBUS_DISCOVERY_ADAPTOR 	"dbus_04"
#define DBUS_SHOP_ADAPTOR 	"dbus_05"
#define DBUS_STORAGE_ADAPTOR 	"dbus_06"
#define DBUS_PUSH_ADAPTOR 	"dbus_07"
#define DBUS_NAME_LENGTH	7


///////////////////
///////////////////               common
///////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
///////////////////
///////////////////               private feature
///////////////////


#define PRIVATE_DBUS_CONNECT_SERVICE_ADAPTOR_METHOD		DBUS_SERVICE_ADAPTOR "_private""_connect_service_adaptor"
#define PRIVATE_DBUS_EXTERNAL_REQ_METHOD			DBUS_SERVICE_ADAPTOR "_private" "_ext_req"

#define PRIVATE_DBUS_GET_AUTH_PLUGIN_LIST_METHOD		DBUS_AUTH_ADAPTOR "_private" "_get_auth_plugin_list"
#define PRIVATE_DBUS_SET_AUTH_METHOD				DBUS_AUTH_ADAPTOR "_private" "_set_auth"

#define PRIVATE_DBUS_SET_NEW_CONTACT_LIST_METHOD		DBUS_CONTACT_ADAPTOR "_private" "_set_new_contact_list"
#define PRIVATE_DBUS_SET_CONTACT_LIST_METHOD			DBUS_CONTACT_ADAPTOR "_private" "_set_contact_list"
#define PRIVATE_DBUS_GET_CONTACT_LIST_METHOD			DBUS_CONTACT_ADAPTOR "_private" "_get_contact_list"
#define PRIVATE_DBUS_GET_CONTACT_INFOS_POLLING_METHOD		DBUS_CONTACT_ADAPTOR "_private" "_get_contact_infos_polling"
#define PRIVATE_DBUS_SET_ME_PROFILE_WITH_PUSH_METHOD		DBUS_CONTACT_ADAPTOR "_private" "_set_me_profile_with_push"
#define PRIVATE_DBUS_GET_PROFILE_METHOD				DBUS_CONTACT_ADAPTOR "_private" "_get_profile"
#define PRIVATE_DBUS_SET_PROFILE_IMAGE_META_WITH_PUSH_METHOD	DBUS_CONTACT_ADAPTOR "_private" "_set_profile_image_meta_with_push"
#define PRIVATE_DBUS_DEL_ME_PROFILE_IMAGE_META_WITH_PUSH_METHOD	DBUS_CONTACT_ADAPTOR "_private" "_del_me_profile_image_meta_with_push"
#define PRIVATE_DBUS_SET_ME_PROFILE_PRIVACY_METHOD		DBUS_CONTACT_ADAPTOR "_private" "_set_me_profile_privacy"
#define PRIVATE_DBUS_GET_PROFILE_PRIVACY_METHOD			DBUS_CONTACT_ADAPTOR "_private" "_get_profile_privacy"
#define PRIVATE_DBUS_SET_ME_PRESENCE_WITH_PUSH_METHOD		DBUS_CONTACT_ADAPTOR "_private" "_set_me_presence_with_push"
#define PRIVATE_DBUS_SET_ME_PRESENCE_ON_OFF_WITH_PUSH_METHOD	DBUS_CONTACT_ADAPTOR "_private" "_set_me_presence_on_off_with_push"
#define PRIVATE_DBUS_SET_ME_PROFILE_TYPE_METHOD			DBUS_CONTACT_ADAPTOR "_private" "_set_me_profile_type"

#define PRIVATE_DBUS_REQUEST_CREATE_CHATROOM_METHOD		DBUS_MESSAGE_ADAPTOR "_private" "_request_create_chatroom"
#define PRIVATE_DBUS_REQUEST_CHANGE_CHATROOM_META_METHOD	DBUS_MESSAGE_ADAPTOR "_private" "_request_change_chatroom_meta"
#define PRIVATE_DBUS_REQUEST_CHAT_METHOD			DBUS_MESSAGE_ADAPTOR "_private" "_request_chat"
#define PRIVATE_DBUS_REQUEST_ALLOW_CHAT_METHOD			DBUS_MESSAGE_ADAPTOR "_private" "_request_allow_chat"
#define PRIVATE_DBUS_REQUEST_ALL_UNREAD_MESSAGE_METHOD		DBUS_MESSAGE_ADAPTOR "_private" "_request_all_unread_message"
#define PRIVATE_DBUS_REPLY_FORWARD_ONLINE_MESSAGE_METHOD	DBUS_MESSAGE_ADAPTOR "_private" "_reply_forward_online_message"
#define PRIVATE_DBUS_REPLY_FORWARD_UNREAD_MESSAGE_METHOD	DBUS_MESSAGE_ADAPTOR "_private" "_reply_forward_unread_message"
#define PRIVATE_DBUS_REQUEST_READ_MESSAGE_METHOD		DBUS_MESSAGE_ADAPTOR "_private" "_request_read_message"
#define PRIVATE_DBUS_REQUEST_INVITE_CHAT_METHOD			DBUS_MESSAGE_ADAPTOR "_private" "_request_invite_chat"
#define PRIVATE_DBUS_REQUEST_END_CHAT_METHOD			DBUS_MESSAGE_ADAPTOR "_private" "_request_end_chat"
#define PRIVATE_DBUS_REQUEST_UNSEAL_MESSAGE_METHOD		DBUS_MESSAGE_ADAPTOR "_private" "_request_unseal_message"
#define PRIVATE_DBUS_REQUEST_SAVE_CALL_LOG_METHOD		DBUS_MESSAGE_ADAPTOR "_private" "_request_save_call_log"
#define PRIVATE_DBUS_REQUEST_CURRENT_TIME_METHOD		DBUS_MESSAGE_ADAPTOR "_private" "_request_current_time"
#define PRIVATE_DBUS_REQUEST_GET_CONNECTION_POLICY_METHOD	DBUS_MESSAGE_ADAPTOR "_private" "_request_get_connection_policy"
#define PRIVATE_DBUS_REQUEST_SET_CONNECTION_POLICY_METHOD	DBUS_MESSAGE_ADAPTOR "_private" "_request_set_connection_policy"
#define PRIVATE_DBUS_GET_CHAT_ID_LIST_METHOD			DBUS_MESSAGE_ADAPTOR "_private" "_get_chat_id_list"
#define PRIVATE_DBUS_GET_MSISDN_LIST_METHOD			DBUS_MESSAGE_ADAPTOR "_private" "_get_msisdn_list"

#define PRIVATE_DBUS_GET_ITEM_LIST_METHOD			DBUS_SHOP_ADAPTOR "_private" "_get_item_list"
#define PRIVATE_DBUS_DOWNLOAD_ITEM_PACKAGE_METHOD		DBUS_SHOP_ADAPTOR "_private" "_download_item_package"
#define PRIVATE_DBUS_DOWNLOAD_STICKER_METHOD			DBUS_SHOP_ADAPTOR "_private" "_download_sticker"
#define PRIVATE_DBUS_GET_PANEL_URL_METHOD			DBUS_SHOP_ADAPTOR "_private" "_get_panel_url"

#define PRIVATE_DBUS_DOWNLOAD_FILE_METHOD			DBUS_STORAGE_ADAPTOR "_private" "_download_file"
#define PRIVATE_DBUS_DOWNLOAD_FILE_ASYNC_METHOD			DBUS_STORAGE_ADAPTOR "_private" "_download_file_async"
#define PRIVATE_DBUS_UPLOAD_FILE_METHOD				DBUS_STORAGE_ADAPTOR "_private" "_upload_file"
#define PRIVATE_DBUS_UPLOAD_FILE_ASYNC_METHOD			DBUS_STORAGE_ADAPTOR "_private" "_upload_file_async"
#define PRIVATE_DBUS_GET_FILE_STATUS_METHOD			DBUS_STORAGE_ADAPTOR "_private" "_get_file_status"
#define PRIVATE_DBUS_CANCEL_FILE_TRANSFER_METHOD		DBUS_STORAGE_ADAPTOR "_private" "_cancel_file_transfer"
#define PRIVATE_DBUS_PAUSE_FILE_TRANSFER_METHOD			DBUS_STORAGE_ADAPTOR "_private" "_pause_file_transfer"
#define PRIVATE_DBUS_RESUME_FILE_TRANSFER_METHOD		DBUS_STORAGE_ADAPTOR "_private" "_resume_file_transfer"
#define PRIVATE_DBUS_DOWNLOAD_FILE_PUBLISH_METHOD		DBUS_STORAGE_ADAPTOR "_private" "_download_file_publish"
#define PRIVATE_DBUS_DOWNLOAD_FILE_PUBLISH_ASYNC_METHOD		DBUS_STORAGE_ADAPTOR "_private" "_download_file_publish_async"
#define PRIVATE_DBUS_UPLOAD_FILE_PUBLISH_METHOD			DBUS_STORAGE_ADAPTOR "_private" "_upload_file_publish"
#define PRIVATE_DBUS_UPLOAD_FILE_PUBLISH_ASYNC_METHOD		DBUS_STORAGE_ADAPTOR "_private" "_upload_file_publish_async"
#define PRIVATE_DBUS_GET_PRIVILEGE_CHECK_RESULT_METHOD          DBUS_STORAGE_ADAPTOR "_private" "_get_privilege_check_result"

#define PRIVATE_DBUS_REPLY_CREATE_CHATROOM_SIGNAL		DBUS_MESSAGE_ADAPTOR "_private" "_reply_create_chatroom"
#define PRIVATE_DBUS_REPLY_CHANGE_CHATROOM_META_SIGNAL		DBUS_MESSAGE_ADAPTOR "_private" "_reply_change_chatroom_meta"
#define PRIVATE_DBUS_REPLY_CHAT_SIGNAL				DBUS_MESSAGE_ADAPTOR "_private" "_reply_chat"
#define PRIVATE_DBUS_REPLY_ALLOW_CHAT_SIGNAL			DBUS_MESSAGE_ADAPTOR "_private" "_reply_allow_chat"
#define PRIVATE_DBUS_REPLY_ALL_UNREAD_MESSAGE_SIGNAL		DBUS_MESSAGE_ADAPTOR "_private" "_reply_all_unread_message"
#define PRIVATE_DBUS_REQUEST_FORWARD_ONLINE_MESSAGE_SIGNAL	DBUS_MESSAGE_ADAPTOR "_private" "_request_forward_online_message"
#define PRIVATE_DBUS_REQUEST_FORWARD_UNREAD_MESSAGE_SIGNAL	DBUS_MESSAGE_ADAPTOR "_private" "_request_forward_unread_message"
#define PRIVATE_DBUS_REPLY_READ_MESSAGE_SIGNAL			DBUS_MESSAGE_ADAPTOR "_private" "_reply_read_message"
#define PRIVATE_DBUS_REPLY_INVITE_CHAT_SIGNAL			DBUS_MESSAGE_ADAPTOR "_private" "_reply_invite_chat"
#define PRIVATE_DBUS_REPLY_END_CHAT_SIGNAL			DBUS_MESSAGE_ADAPTOR "_private" "_reply_end_chat"
#define PRIVATE_DBUS_REPLY_UNSEAL_MESSAGE_SIGNAL		DBUS_MESSAGE_ADAPTOR "_private" "_reply_unseal_message"
#define PRIVATE_DBUS_REPLY_SAVE_CALL_LOG_SIGNAL			DBUS_MESSAGE_ADAPTOR "_private" "_reply_save_call_log"
#define PRIVATE_DBUS_REPLY_CURRENT_TIME_SIGNAL			DBUS_MESSAGE_ADAPTOR "_private" "_reply_current_time"
#define PRIVATE_DBUS_REPLY_CHANNEL_DISCONNECTED_SIGNAL		DBUS_MESSAGE_ADAPTOR "_private" "_reply_channel_disconnected"

#define PRIVATE_DBUS_STORAGE_FILE_PROGRESS_SIGNAL		DBUS_STORAGE_ADAPTOR "_private" "_storage_file_progress"
#define PRIVATE_DBUS_STORAGE_FILE_TRANSFER_COMPLETION_SIGNAL	DBUS_STORAGE_ADAPTOR "_private" "_storage_file_transfer_completion"

#define PRIVATE_DBUS_PUSH_DATA_SIGNAL				DBUS_PUSH_ADAPTOR "_private" "_push_data"
#define PRIVATE_DBUS_PUSH_REGISTER_METHOD			DBUS_PUSH_ADAPTOR "_private" "_push_register"
#define PRIVATE_DBUS_PUSH_DEREGISTER_METHOD			DBUS_PUSH_ADAPTOR "_private" "_push_deregister"

#define PRIVATE_DBUS_SERVICE_ADAPTOR_SIGNAL			DBUS_SERVICE_ADAPTOR "_private" "_service_adaptor_signal"


///////////////////
///////////////////               private feature
///////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
///////////////////
///////////////////               public feature
///////////////////

#define DBUS_CONNECT_SERVICE_ADAPTOR_METHOD		DBUS_SERVICE_ADAPTOR "_connect_service_adaptor"
#define DBUS_DISCONNECT_SERVICE_ADAPTOR_METHOD		DBUS_SERVICE_ADAPTOR "_disconnect_service_adaptor"

#define DBUS_GET_AUTH_PLUGIN_LIST_METHOD		DBUS_AUTH_ADAPTOR "_get_auth_plugin_list"
#define DBUS_IS_AUTH_METHOD				DBUS_AUTH_ADAPTOR "_is_auth"
#define DBUS_JOIN_METHOD				DBUS_AUTH_ADAPTOR "_join"
#define DBUS_SET_AUTH_METHOD				DBUS_AUTH_ADAPTOR "_set_auth"

#define DBUS_DOWNLOAD_FILE_METHOD			DBUS_STORAGE_ADAPTOR "_download_file"
#define DBUS_DOWNLOAD_THUMBNAIL_METHOD			DBUS_STORAGE_ADAPTOR "_download_thumbnail"
#define DBUS_UPLOAD_FILE_METHOD				DBUS_STORAGE_ADAPTOR "_upload_file"
#define DBUS_GET_ROOT_FOLDER_PATH_METHOD		DBUS_STORAGE_ADAPTOR "_get_root_folder_path"
#define DBUS_MAKE_DIRECTORY_METHOD			DBUS_STORAGE_ADAPTOR "_make_directory"
#define DBUS_REMOVE_FILE_METHOD				DBUS_STORAGE_ADAPTOR "_remove_file"
#define DBUS_REMOVE_DIRECTORY_METHOD			DBUS_STORAGE_ADAPTOR "_remove_directory"
#define DBUS_GET_METADATA_METHOD			DBUS_STORAGE_ADAPTOR "_get_metadata"
#define DBUS_GET_LIST_METHOD				DBUS_STORAGE_ADAPTOR "_get_list"
#define DBUS_MOVE_FILE_METHOD				DBUS_STORAGE_ADAPTOR "_move_file"
#define DBUS_MOVE_DIRECTORY_METHOD			DBUS_STORAGE_ADAPTOR "_move_directory"
#define DBUS_GET_FILE_LIST_METHOD			DBUS_STORAGE_ADAPTOR "_get_file_list"

#define DBUS_OPEN_UPLOAD_FILE_METHOD			DBUS_STORAGE_ADAPTOR "_open_upload_file"
#define DBUS_OPEN_DOWNLOAD_FILE_METHOD			DBUS_STORAGE_ADAPTOR "_open_download_file"
#define DBUS_OPEN_DOWNLOAD_THUMBNAIL_METHOD		DBUS_STORAGE_ADAPTOR "_open_download_thumbnail"
#define DBUS_CLOSE_FILE_METHOD				DBUS_STORAGE_ADAPTOR "_close_file"
#define DBUS_START_UPLOAD_FILE_METHOD			DBUS_STORAGE_ADAPTOR "_start_upload_file"
#define DBUS_START_DOWNLOAD_FILE_METHOD			DBUS_STORAGE_ADAPTOR "_start_download_file"
#define DBUS_START_DOWNLOAD_THUMBNAIL_METHOD		DBUS_STORAGE_ADAPTOR "_start_dowlnoad_thumbnail"
#define DBUS_CANCEL_UPLOAD_FILE_METHOD			DBUS_STORAGE_ADAPTOR "_cancel_upload_file"
#define DBUS_CANCEL_DOWNLOAD_FILE_METHOD		DBUS_STORAGE_ADAPTOR "_cancel_download_file"
#define DBUS_CANCEL_DOWNLOAD_THUMBNAIL_METHOD		DBUS_STORAGE_ADAPTOR "_cancel_download_thumbnail"

#define DBUS_STORAGE_FILE_PROGRESS_SIGNAL		DBUS_STORAGE_ADAPTOR "_storage_file_progress"
#define DBUS_STORAGE_FILE_TRANSFER_STATE_CHANGED_SIGNAL	DBUS_STORAGE_ADAPTOR "_storage_file_transfer_state_changed"
#define DBUS_STORAGE_FILE_TRANSFER_COMPLETION_SIGNAL	DBUS_STORAGE_ADAPTOR "_storage_file_transfer_completion"

#define DBUS_PUSH_DATA_SIGNAL				DBUS_PUSH_ADAPTOR "_push_data"

#define DBUS_SERVICE_ADAPTOR_SIGNAL			DBUS_SERVICE_ADAPTOR "_service_adaptor_signal"

/* Extention enum define */
#define SERVICE_ADAPTOR_FILE_TRANSFER_STATE_IN_PROGRESS 1
#define SERVICE_ADAPTOR_FILE_TRANSFER_STATE_COMPLETED 2
#define SERVICE_ADAPTOR_FILE_TRANSFER_STATE_CANCELED 3
#define SERVICE_ADAPTOR_FILE_TRANSFER_STATE_FAILED 4


///////////////////
///////////////////               public feature
///////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
///////////////////
///////////////////               private feature
///////////////////

typedef enum
{
	SERVICE_ADAPTOR_PROTOCOL_RETURN_CODE_NONE			= 0,
	SERVICE_ADAPTOR_PROTOCOL_RETURN_CODE_COMMON_NO_DATA		= 101,
	SERVICE_ADAPTOR_PROTOCOL_RETURN_CODE_COMMON_TIMED_OUT		= 102,
	SERVICE_ADAPTOR_PROTOCOL_RETURN_CODE_COMMON_NOT_SUPPORTED	= 103,
	SERVICE_ADAPTOR_PROTOCOL_RETURN_CODE_COMMON_PERMISSION_DENIED	= 104,
	SERVICE_ADAPTOR_PROTOCOL_RETURN_CODE_AUTH_NOT_AUTHORIZED	= 201,
	SERVICE_ADAPTOR_PROTOCOL_RETURN_CODE_MESSAGE_NETWORK		= 601,
} service_adaptor_protocol_return_code_e;

typedef enum
{
	SERVICE_ADAPTOR_PROTOCOL_SERVICE_TYPE_AUTH		= (0x01 << 0),
	SERVICE_ADAPTOR_PROTOCOL_SERVICE_TYPE_STORAGE		= (0x01 << 1),
	SERVICE_ADAPTOR_PROTOCOL_SERVICE_TYPE_CONTACT		= (0x01 << 2),
	SERVICE_ADAPTOR_PROTOCOL_SERVICE_TYPE_MESSAGE		= (0x01 << 3),
	SERVICE_ADAPTOR_PROTOCOL_SERVICE_TYPE_SHOP		= (0x01 << 4),
	SERVICE_ADAPTOR_PROTOCOL_SERVICE_TYPE_PUSH		= (0x01 << 5),
} service_adaptor_protocol_service_type_e;

///////////////////
///////////////////               private feature
///////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

#endif /* __DBUS_SERVER_H__ */
