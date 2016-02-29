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
#include <glib.h>
#include <gio/gio.h>
#include <errno.h>
#include <stdint.h>

#include "service-adaptor-log.h"
#include "service-adaptor.h"
#include "service-adaptor-type.h"
#include "dbus-service-adaptor.h"
#include "dbus-auth-adaptor.h"
#include "dbus-contact-adaptor.h"
#include "dbus-message-adaptor.h"
#include "dbus-shop-adaptor.h"
#include "dbus-storage-adaptor.h"
#include "dbus-push-adaptor.h"
#include "dbus-server.h"
#include "dbus-server-type.h"
#include "dbus-util.h"

static GThreadPool *thread_pool = NULL;

/**
 * Compiled introspection data describing Service Adaptor D-Bus interface.
 */
static GDBusNodeInfo *introspection_data = NULL;

/**
 * Service Adaptor D-Bus server owner id.
 */
static guint owner_id = 0;

/**
 * DBus connection used by server.
 */
static GDBusConnection *dbus_connection = NULL;

/**
 * Introspection data describing vService Channel D-Bus interface.
 */
static const gchar introspection_xml[] =
"<node>"
"  <interface name='" SERVICE_ADAPTOR_INTERFACE "'>"
/************************************************************************
 *
 *
 *                          private feature
 */
"    <method name='" PRIVATE_DBUS_CONNECT_SERVICE_ADAPTOR_METHOD "'>"
"      <arg type='" private_service_adaptor_essential_s_type "' name='req' direction='in'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_EXTERNAL_REQ_METHOD "'>"
"      <arg type='" private_service_adaptor_external_req_s_type "' name='ext_req' direction='in'/>"
"      <arg type='" service_adaptor_raw_data_s_type "' name='ext_res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_GET_AUTH_PLUGIN_LIST_METHOD "'>"
"      <arg type='s' name='imsi' direction='in'/>"
"      <arg type='" private_plugin_list_type "' name='plugin_list' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_SET_AUTH_METHOD "'>"
"      <arg type='" private_service_adaptor_set_auth_s_type "' name='set_auth' direction='in'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_SET_NEW_CONTACT_LIST_METHOD "'>"
"      <arg type='" private_service_adaptor_contact_req_s_type "' name='req' direction='in'/>"
"      <arg type='" private_service_adaptor_contact_res_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_SET_CONTACT_LIST_METHOD "'>"
"      <arg type='" private_service_adaptor_contact_req_s_type "' name='req' direction='in'/>"
"      <arg type='" private_service_adaptor_contact_res_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_GET_CONTACT_LIST_METHOD "'>"
"      <arg type='" private_service_adaptor_essential_s_type "' name='req' direction='in'/>"
"      <arg type='" private_service_adaptor_contact_res_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_GET_CONTACT_INFOS_POLLING_METHOD "'>"
"      <arg type='" private_service_adaptor_contact_req_s_type "' name='req' direction='in'/>"
"      <arg type='" private_service_adaptor_contact_res_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_SET_ME_PROFILE_WITH_PUSH_METHOD "'>"
"      <arg type='" private_service_adaptor_profile_req_s_type "' name='req' direction='in'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_GET_PROFILE_METHOD "'>"
"      <arg type='" private_service_adaptor_profile_req_s_type "' name='req' direction='in'/>"
"      <arg type='" private_service_adaptor_profile_res_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_SET_PROFILE_IMAGE_META_WITH_PUSH_METHOD "'>"
"      <arg type='" private_service_adaptor_contact_profile_image_req_s_type "' name='req' direction='in'/>"
"      <arg type='" private_service_adaptor_contact_profile_image_req_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_DEL_ME_PROFILE_IMAGE_META_WITH_PUSH_METHOD "'>"
"      <arg type='" private_service_adaptor_essential_s_type "' name='req' direction='in'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_SET_ME_PROFILE_PRIVACY_METHOD "'>"
"      <arg type='" private_service_adaptor_privacy_req_s_type "' name='req' direction='in'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_GET_PROFILE_PRIVACY_METHOD "'>"
"      <arg type='" private_service_adaptor_essential_s_type "' name='req' direction='in'/>"
"      <arg type='" private_service_adaptor_privacy_res_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_SET_ME_PRESENCE_WITH_PUSH_METHOD "'>"
"      <arg type='" private_service_adaptor_presence_req_s_type "' name='req' direction='in'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_SET_ME_PROFILE_TYPE_METHOD "'>"
"      <arg type='" private_service_adaptor_set_me_profile_type_req_s_type "' name='req' direction='in'/>"
"      <arg type='" private_service_adaptor_essential_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_SET_ME_PRESENCE_ON_OFF_WITH_PUSH_METHOD "'>"
"      <arg type='" private_service_adaptor_presence_req_s_type "' name='req' direction='in'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_REQUEST_CREATE_CHATROOM_METHOD "'>"
"      <arg type='" private_service_adaptor_create_chatroom_req_s_type "' name='req' direction='in'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_REQUEST_CHANGE_CHATROOM_META_METHOD "'>"
"      <arg type='" private_service_adaptor_change_chatroom_meta_req_s_type "' name='req' direction='in'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_REQUEST_CHAT_METHOD "'>"
"      <arg type='" private_service_adaptor_chat_req_s_type "' name='req' direction='in'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_REQUEST_ALLOW_CHAT_METHOD "'>"
"      <arg type='" private_service_adaptor_allow_chat_req_s_type "' name='req' direction='in'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_REQUEST_ALL_UNREAD_MESSAGE_METHOD "'>"
"      <arg type='" private_service_adaptor_all_unread_message_req_s_type "' name='req' direction='in'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_REPLY_FORWARD_ONLINE_MESSAGE_METHOD "'>"
"      <arg type='" private_service_adaptor_forward_online_message_res_s_type "' name='req' direction='in'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_REPLY_FORWARD_UNREAD_MESSAGE_METHOD "'>"
"      <arg type='" private_service_adaptor_forward_unread_message_res_s_type "' name='req' direction='in'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_REQUEST_READ_MESSAGE_METHOD "'>"
"      <arg type='" private_service_adaptor_read_message_req_s_type "' name='req' direction='in'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_REQUEST_INVITE_CHAT_METHOD "'>"
"      <arg type='" private_service_adaptor_invite_chat_req_s_type "' name='req' direction='in'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_REQUEST_END_CHAT_METHOD "'>"
"      <arg type='" private_service_adaptor_end_chat_req_s_type "' name='req' direction='in'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_REQUEST_UNSEAL_MESSAGE_METHOD "'>"
"      <arg type='" private_service_adaptor_unseal_message_req_s_type "' name='req' direction='in'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_REQUEST_SAVE_CALL_LOG_METHOD "'>"
"      <arg type='" private_service_adaptor_save_call_log_req_s_type "' name='req' direction='in'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_REQUEST_CURRENT_TIME_METHOD "'>"
"      <arg type='" private_service_adaptor_current_time_req_s_type "' name='req' direction='in'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_REQUEST_GET_CONNECTION_POLICY_METHOD "'>"
"      <arg type='" private_service_adaptor_get_connection_policy_req_s_type "' name='req' direction='in'/>"
"      <arg type='" private_service_adaptor_connection_policy_res_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_REQUEST_SET_CONNECTION_POLICY_METHOD "'>"
"      <arg type='" private_service_adaptor_set_connection_policy_req_s_type "' name='req' direction='in'/>"
"      <arg type='" private_service_adaptor_connection_policy_res_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_GET_CHAT_ID_LIST_METHOD "'>"
"      <arg type='" private_service_adaptor_chat_id_list_req_s_type "' name='req' direction='in'/>"
"      <arg type='" private_service_adaptor_chat_id_list_res_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_GET_MSISDN_LIST_METHOD "'>"
"      <arg type='" private_service_adaptor_msisdn_list_req_s_type "' name='req' direction='in'/>"
"      <arg type='" private_service_adaptor_msisdn_list_res_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_GET_ITEM_LIST_METHOD "'>"
"      <arg type='" private_service_adaptor_shop_req_s_type "' name='req' direction='in'/>"
"      <arg type='" private_service_adaptor_shop_res_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_DOWNLOAD_ITEM_PACKAGE_METHOD "'>"
"      <arg type='" private_service_adaptor_shop_req_s_type "' name='req' direction='in'/>"
"      <arg type='" private_service_adaptor_shop_item_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_DOWNLOAD_STICKER_METHOD "'>"
"      <arg type='" private_service_adaptor_shop_req_s_type "' name='req' direction='in'/>"
"      <arg type='" private_service_adaptor_shop_item_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_GET_PANEL_URL_METHOD "'>"
"      <arg type='" private_service_adaptor_shop_req_s_type "' name='req' direction='in'/>"
"      <arg type='" private_service_adaptor_shop_item_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_DOWNLOAD_FILE_METHOD "'>"
"      <arg type='" private_service_adaptor_file_transfer_req_s_type "' name='req' direction='in'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_DOWNLOAD_FILE_ASYNC_METHOD "'>"
"      <arg type='" private_service_adaptor_file_transfer_req_s_type "' name='req' direction='in'/>"
"      <arg type='" private_service_adaptor_file_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_UPLOAD_FILE_METHOD "'>"
"      <arg type='" private_service_adaptor_file_transfer_req_s_type "' name='req' direction='in'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_UPLOAD_FILE_ASYNC_METHOD "'>"
"      <arg type='" private_service_adaptor_file_transfer_req_s_type "' name='req' direction='in'/>"
"      <arg type='" private_service_adaptor_file_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_DOWNLOAD_FILE_PUBLISH_METHOD "'>"
"      <arg type='" private_service_adaptor_file_transfer_req_s_type "' name='req' direction='in'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_DOWNLOAD_FILE_PUBLISH_ASYNC_METHOD "'>"
"      <arg type='" private_service_adaptor_file_transfer_req_s_type "' name='req' direction='in'/>"
"      <arg type='" private_service_adaptor_file_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_UPLOAD_FILE_PUBLISH_METHOD "'>"
"      <arg type='" private_service_adaptor_file_transfer_req_s_type "' name='req' direction='in'/>"
"      <arg type='" private_service_adaptor_file_publish_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_UPLOAD_FILE_PUBLISH_ASYNC_METHOD "'>"
"      <arg type='" private_service_adaptor_file_transfer_req_s_type "' name='req' direction='in'/>"
"      <arg type='" private_service_adaptor_file_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_GET_FILE_STATUS_METHOD "'>"
"      <arg type='" private_service_adaptor_file_status_req_s_type "' name='req' direction='in'/>"
"      <arg type='" private_service_adaptor_file_status_res_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_CANCEL_FILE_TRANSFER_METHOD "'>"
"      <arg type='" private_service_adaptor_file_status_req_s_type "' name='req' direction='in'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_PAUSE_FILE_TRANSFER_METHOD "'>"
"      <arg type='" private_service_adaptor_file_status_req_s_type "' name='req' direction='in'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_RESUME_FILE_TRANSFER_METHOD "'>"
"      <arg type='" private_service_adaptor_file_status_req_s_type "' name='req' direction='in'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <signal name='" PRIVATE_DBUS_REPLY_CREATE_CHATROOM_SIGNAL "'>"
"      <arg type='" private_service_adaptor_create_chatroom_res_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </signal>"
"    <signal name='" PRIVATE_DBUS_REPLY_CHANGE_CHATROOM_META_SIGNAL "'>"
"      <arg type='" private_service_adaptor_change_chatroom_meta_res_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </signal>"
"    <signal name='" PRIVATE_DBUS_REPLY_CHAT_SIGNAL "'>"
"      <arg type='" private_service_adaptor_chat_res_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </signal>"
"    <signal name='" PRIVATE_DBUS_REPLY_ALLOW_CHAT_SIGNAL "'>"
"      <arg type='" private_service_adaptor_allow_chat_res_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </signal>"
"    <signal name='" PRIVATE_DBUS_REPLY_ALL_UNREAD_MESSAGE_SIGNAL "'>"
"      <arg type='" private_service_adaptor_all_unread_message_res_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </signal>"
"    <signal name='" PRIVATE_DBUS_REQUEST_FORWARD_ONLINE_MESSAGE_SIGNAL "'>"
"      <arg type='" private_service_adaptor_forward_online_message_req_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </signal>"
"    <signal name='" PRIVATE_DBUS_REQUEST_FORWARD_UNREAD_MESSAGE_SIGNAL "'>"
"      <arg type='" private_service_adaptor_forward_unread_message_req_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </signal>"
"    <signal name='" PRIVATE_DBUS_REPLY_READ_MESSAGE_SIGNAL "'>"
"      <arg type='" private_service_adaptor_read_message_res_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </signal>"
"    <signal name='" PRIVATE_DBUS_REPLY_INVITE_CHAT_SIGNAL "'>"
"      <arg type='" private_service_adaptor_invite_chat_res_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </signal>"
"    <signal name='" PRIVATE_DBUS_REPLY_END_CHAT_SIGNAL "'>"
"      <arg type='" private_service_adaptor_end_chat_res_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </signal>"
"    <signal name='" PRIVATE_DBUS_REPLY_UNSEAL_MESSAGE_SIGNAL "'>"
"      <arg type='" private_service_adaptor_unseal_message_res_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </signal>"
"    <signal name='" PRIVATE_DBUS_REPLY_SAVE_CALL_LOG_SIGNAL "'>"
"      <arg type='" private_service_adaptor_save_call_log_res_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </signal>"
"    <signal name='" PRIVATE_DBUS_REPLY_CURRENT_TIME_SIGNAL "'>"
"      <arg type='" private_service_adaptor_current_time_res_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </signal>"
"    <signal name='" PRIVATE_DBUS_REPLY_CHANNEL_DISCONNECTED_SIGNAL "'>"
"      <arg type='" private_service_adaptor_channel_disconnected_res_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </signal>"
"    <signal name='" PRIVATE_DBUS_STORAGE_FILE_PROGRESS_SIGNAL "'>"
"      <arg type='" private_service_adaptor_file_progress_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </signal>"
"    <signal name='" PRIVATE_DBUS_STORAGE_FILE_TRANSFER_COMPLETION_SIGNAL "'>"
"      <arg type='" private_service_adaptor_file_transfer_completion_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </signal>"
"    <signal name='" PRIVATE_DBUS_PUSH_DATA_SIGNAL "'>"
"      <arg type='" private_service_adaptor_push_data_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </signal>"
"    <method name='" PRIVATE_DBUS_PUSH_REGISTER_METHOD "'>"
"      <arg type='" private_service_adaptor_push_register_req_s_type "' name='req' direction='in'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" PRIVATE_DBUS_PUSH_DEREGISTER_METHOD "'>"
"      <arg type='" private_service_adaptor_push_deregister_req_s_type "' name='req' direction='in'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <signal name='" PRIVATE_DBUS_SERVICE_ADAPTOR_SIGNAL "'>"
"      <arg type='t' name='signal_code' direction='out'/>"
"      <arg type='s' name='signal_msg' direction='out'/>"
"    </signal>"
/*
 *                                private feature
 *
 *
 ***********************************************************************/

/************************************************************************
 *
 *
 *                                public feature
 */
"    <method name='" DBUS_CONNECT_SERVICE_ADAPTOR_METHOD "'>"
"      <arg type='" "(s)" "' name='req' direction='in'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" DBUS_DISCONNECT_SERVICE_ADAPTOR_METHOD "'>"
"      <arg type='" "(s)" "' name='req' direction='in'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" DBUS_GET_AUTH_PLUGIN_LIST_METHOD "'>"
"      <arg type='s' name='temp' direction='in'/>"
"      <arg type='" plugin_list_type "' name='plugin_list' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" DBUS_IS_AUTH_METHOD "'>"
"      <arg type='" service_adaptor_is_auth_req_s_type "' name='is_auth' direction='in'/>"
"      <arg type='(b)' name='auth' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" DBUS_JOIN_METHOD "'>"
"      <arg type='" service_adaptor_join_req_s_type "' name='join' direction='in'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" DBUS_SET_AUTH_METHOD "'>"
"      <arg type='" service_adaptor_set_auth_s_type "' name='set_auth' direction='in'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" DBUS_DOWNLOAD_FILE_METHOD "'>"
"      <arg type='" service_adaptor_file_transfer_req_s_type "' name='req' direction='in'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" DBUS_DOWNLOAD_THUMBNAIL_METHOD "'>"
"      <arg type='" service_adaptor_download_thumbnail_req_s_type "' name='req' direction='in'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" DBUS_UPLOAD_FILE_METHOD "'>"
"      <arg type='" service_adaptor_file_transfer_req_s_type "' name='req' direction='in'/>"
"      <arg type='" service_adaptor_file_info_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" DBUS_GET_ROOT_FOLDER_PATH_METHOD "'>"
"      <arg type='" service_adaptor_get_root_folder_path_req_s_type "' name='req' direction='in'/>"
"      <arg type='" service_adaptor_get_root_folder_path_res_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" DBUS_MAKE_DIRECTORY_METHOD "'>"
"      <arg type='" service_adaptor_make_directory_req_s_type "' name='req' direction='in'/>"
"      <arg type='" service_adaptor_file_info_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" DBUS_REMOVE_FILE_METHOD "'>"
"      <arg type='" service_adaptor_remove_file_req_s_type "' name='req' direction='in'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" DBUS_REMOVE_DIRECTORY_METHOD "'>"
"      <arg type='" service_adaptor_remove_directory_req_s_type "' name='req' direction='in'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" DBUS_MOVE_FILE_METHOD "'>"
"      <arg type='" service_adaptor_move_file_req_s_type "' name='req' direction='in'/>"
"      <arg type='" service_adaptor_file_info_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" DBUS_MOVE_DIRECTORY_METHOD "'>"
"      <arg type='" service_adaptor_move_directory_req_s_type "' name='req' direction='in'/>"
"      <arg type='" service_adaptor_file_info_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" DBUS_GET_FILE_LIST_METHOD "'>"
"      <arg type='" service_adaptor_get_file_list_req_s_type "' name='req' direction='in'/>"
"      <arg type='" service_adaptor_get_file_list_res_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"

"    <method name='" DBUS_OPEN_UPLOAD_FILE_METHOD "'>"
"      <arg type='" service_adaptor_open_upload_file_req_s_type "' name='req' direction='in'/>"
"      <arg type='" service_adaptor_file_descriptor_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" DBUS_OPEN_DOWNLOAD_FILE_METHOD "'>"
"      <arg type='" service_adaptor_open_download_file_req_s_type "' name='req' direction='in'/>"
"      <arg type='" service_adaptor_file_descriptor_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" DBUS_OPEN_DOWNLOAD_THUMBNAIL_METHOD "'>"
"      <arg type='" service_adaptor_open_download_thumbnail_req_s_type "' name='req' direction='in'/>"
"      <arg type='" service_adaptor_file_descriptor_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" DBUS_CLOSE_FILE_METHOD "'>"
"      <arg type='" service_adaptor_close_file_req_s_type "' name='req' direction='in'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" DBUS_START_UPLOAD_FILE_METHOD "'>"
"      <arg type='" service_adaptor_start_upload_file_req_s_type "' name='req' direction='in'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" DBUS_START_DOWNLOAD_FILE_METHOD "'>"
"      <arg type='" service_adaptor_start_download_file_req_s_type "' name='req' direction='in'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" DBUS_START_DOWNLOAD_THUMBNAIL_METHOD "'>"
"      <arg type='" service_adaptor_start_download_thumbnail_req_s_type "' name='req' direction='in'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" DBUS_CANCEL_UPLOAD_FILE_METHOD "'>"
"      <arg type='" service_adaptor_cancel_file_task_req_s_type "' name='req' direction='in'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" DBUS_CANCEL_DOWNLOAD_FILE_METHOD "'>"
"      <arg type='" service_adaptor_cancel_file_task_req_s_type "' name='req' direction='in'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <method name='" DBUS_CANCEL_DOWNLOAD_THUMBNAIL_METHOD "'>"
"      <arg type='" service_adaptor_cancel_file_task_req_s_type "' name='req' direction='in'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </method>"
"    <signal name='" DBUS_STORAGE_FILE_PROGRESS_SIGNAL "'>"
"      <arg type='" service_adaptor_file_progress_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </signal>"
"    <signal name='" DBUS_STORAGE_FILE_TRANSFER_STATE_CHANGED_SIGNAL "'>"
"      <arg type='" service_adaptor_file_transfer_state_changed_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </signal>"
"    <signal name='" DBUS_PUSH_DATA_SIGNAL "'>"
"      <arg type='" service_adaptor_push_data_s_type "' name='res' direction='out'/>"
"      <arg type='t' name='error_code' direction='out'/>"
"      <arg type='s' name='error_msg' direction='out'/>"
"    </signal>"
"    <signal name='" DBUS_SERVICE_ADAPTOR_SIGNAL "'>"
"      <arg type='t' name='signal_code' direction='out'/>"
"      <arg type='s' name='signal_msg' direction='out'/>"
"    </signal>"

/*
 *                                public feature
 *
 *
 ***********************************************************************/
"  </interface>"
"</node>";

typedef struct _method_call_s {
	GDBusConnection *connection;
	gchar *sender;
	gchar *object_path;
	gchar *interface_name;
	gchar *method_name;
	GVariant *parameters;
	GDBusMethodInvocation *invocation;
	gpointer user_data;
} method_call_s;

GDBusConnection *dbus_get_connection()
{
	return dbus_connection;
}

static void method_call_async_func(gpointer data, gpointer user_data)
{
	method_call_s *handle = data;

	service_adaptor_debug("Received %s() call", handle->method_name);
	if (0 == strncmp(handle->method_name, DBUS_SERVICE_ADAPTOR, DBUS_NAME_LENGTH)) {
		service_adaptor_method_call(handle->connection,
				handle->sender,
				handle->object_path,
				handle->interface_name,
				handle->method_name,
				handle->parameters,
				handle->invocation,
				handle->user_data);
	} else if (0 == strncmp(handle->method_name, DBUS_AUTH_ADAPTOR, DBUS_NAME_LENGTH)) {
		auth_adaptor_method_call(handle->connection,
				handle->sender,
				handle->object_path,
				handle->interface_name,
				handle->method_name,
				handle->parameters,
				handle->invocation,
				handle->user_data);
	} else if (0 == strncmp(handle->method_name, DBUS_CONTACT_ADAPTOR, DBUS_NAME_LENGTH)) {
		contact_adaptor_method_call(handle->connection,
				handle->sender,
				handle->object_path,
				handle->interface_name,
				handle->method_name,
				handle->parameters,
				handle->invocation,
				handle->user_data);
	} else if (0 == strncmp(handle->method_name, DBUS_MESSAGE_ADAPTOR, DBUS_NAME_LENGTH)) {
		message_adaptor_method_call(handle->connection,
				handle->sender,
				handle->object_path,
				handle->interface_name,
				handle->method_name,
				handle->parameters,
				handle->invocation,
				handle->user_data);
	} else if (0 == strncmp(handle->method_name, DBUS_SHOP_ADAPTOR, DBUS_NAME_LENGTH)) {
		shop_adaptor_method_call(handle->connection,
				handle->sender,
				handle->object_path,
				handle->interface_name,
				handle->method_name,
				handle->parameters,
				handle->invocation,
				handle->user_data);
	} else if (0 == strncmp(handle->method_name, DBUS_STORAGE_ADAPTOR, DBUS_NAME_LENGTH)) {
		storage_adaptor_method_call(handle->connection,
				handle->sender,
				handle->object_path,
				handle->interface_name,
				handle->method_name,
				handle->parameters,
				handle->invocation,
				handle->user_data);
	} else if (0 == strncmp(handle->method_name, DBUS_PUSH_ADAPTOR, DBUS_NAME_LENGTH)) {
		push_adaptor_method_call(handle->connection,
				handle->sender,
				handle->object_path,
				handle->interface_name,
				handle->method_name,
				handle->parameters,
				handle->invocation,
				handle->user_data);
	}

}

/**
 * @brief Service Adaptor D-Bus interface method call handler.
 *
 * Service Adaptor D-Bus interface method call handler. Called when client calls some function defined in
 * Service Adaptor D-Bus interface.
 * @param connection A GDBusConnection.
 * @param sender The unique bus name of the remote caller.
 * @param object_path The object path that the method was invoked on.
 * @param interface_name The D-Bus interface name the method was invoked on.
 * @param method_name The name of the method that was invoked.
 * @param parameters A GVariant tuple with parameters.
 * @param invocation A GDBusMethodInvocation object that can be used to return a value or error.
 * @param user_data The user_data gpointer passed to g_dbus_connection_register_object().
 */
static void handle_method_call(GDBusConnection *connection,
						const gchar *sender,
						const gchar *object_path,
						const gchar *interface_name,
						const gchar *method_name,
						GVariant *parameters,
						GDBusMethodInvocation *invocation,
						gpointer user_data)
{
	method_call_s *handle = (method_call_s *) g_malloc0(sizeof(method_call_s));
	if (NULL == handle) {
		return;
	}
	handle->connection = connection;
	handle->sender = (gchar *) sender;
	handle->object_path = (gchar *) object_path;
	handle->interface_name = (gchar *) interface_name;
	handle->method_name = (gchar *) method_name;
	handle->parameters = parameters;
	handle->invocation = invocation;
	handle->user_data = user_data;

	g_thread_pool_push(thread_pool, (gpointer) handle, NULL);
}

/**
 * @brief Service Adaptor D-Bus interface get property call handler.
 *
 * Service Adaptor D-Bus interface get property call handler.
 * @param connection A GDBusConnection.
 * @param sender The unique bus name of the remote caller.
 * @param object_path The object path that the method was invoked on.
 * @param interface_name The D-Bus interface name for the property.
 * @param property_name The name of the property to get the value of.
 * @param error Return location for error.
 * @param user_data The user_data gpointer passed to g_dbus_connection_register_object().
 * @return A GVariant with the value for property_name or NULL if error is set. If the returned GVariant
 *         is floating, it is consumed - otherwise its reference count is decreased by one.
 */
static GVariant *handle_get_property(GDBusConnection *connection,
						const gchar *sender,
						const gchar *object_path,
						const gchar *interface_name,
						const gchar *property_name,
						GError **error,
						gpointer user_data)
{
	GVariant *ret = NULL;

	return ret;
}

/**
 * @brief Service Adaptor D-Bus interface set property call handler.
 *
 * Service Adaptor D-Bus interface set property call handler.
 * @param connection A GDBusConnection.
 * @param sender The unique bus name of the remote caller.
 * @param object_path The object path that the method was invoked on.
 * @param interface_name The D-Bus interface name for the property.
 * @param property_name The name of the property to get the value of.
 * @param value The value to set the property to.
 * @param error Return location for error.
 * @param user_data     The user_data gpointer passed to g_dbus_connection_register_object().
 * @return TRUE if the property was set to value, FALSE if error is set.
 */
static gboolean handle_set_property(GDBusConnection *connection,
						const gchar *sender,
						const gchar *object_path,
						const gchar *interface_name,
						const gchar *property_name,
						GVariant *value,
						GError **error,
						gpointer user_data)
{
	return *error == NULL;
}

/**
 * D-Bus handlers vtable.
 */
static const GDBusInterfaceVTable interface_vtable = {
	handle_method_call,
	handle_get_property,
	handle_set_property
};

/**
 * @brief Callback function called when D-Bus bus name for vService Channel D-Bus server is acquired.
 *
 * Callback function called when D-Bus bus name for vService Channel D-Bus server is acquired.
 * @param connection The GDBusConnection to a message bus.
 * @param name The name that is requested to be owned.
 * @param user_data User data passed to g_bus_own_name().
 */
static void on_bus_acquired(GDBusConnection *connection,
						const gchar *name,
						gpointer user_data)
{
	service_adaptor_debug("D-bus bus acquired");
	guint registration_id;

	registration_id = g_dbus_connection_register_object(connection,
			SERVICE_ADAPTOR_OBJECT_PATH,
			introspection_data->interfaces[0],
			&interface_vtable,
			NULL, /* user_data */
			NULL, /* user_data_free_func */
			NULL); /* GError** */
	g_assert(registration_id > 0);
}

/**
 * @brief Callback function called when D-Bus name for vService Channel D-Bus server is acquired.
 *
 * Callback function called when D-Bus name for vService Channel D-Bus server is acquired.
 * @param connection The GDBusConnection on which to acquired the name.
 * @param name The name being owned.
 * @param user_data User data passed to g_bus_own_name() or g_bus_own_name_on_connection().
 */
static void on_name_acquired(GDBusConnection *connection,
						const gchar *name,
						gpointer user_data)
{
	service_adaptor_debug("D-bus bus name acquired");
	dbus_connection = connection;
	g_object_ref(dbus_connection);
}

/**
 * @brief Callback function called when the vService Channel D-Bus name is lost or connection has been closed.
 *
 * Callback function called when the vService Channel D-Bus name is lost or connection has been closed.
 * @param connection The GDBusConnection on which to acquire the name or NULL if the connection was disconnected.
 * @param name The name being owned.
 * @param user_data User data passed to g_bus_own_name() or g_bus_own_name_on_connection().
 */
static void on_name_lost(GDBusConnection *connection,
						const gchar *name,
						gpointer user_data)
{
	if (NULL != dbus_connection) {
		g_object_unref(dbus_connection);
		dbus_connection = NULL;
	}

	service_adaptor_warning("Unexpected D-bus bus name lost.");
	service_adaptor_info("Service-adaptor Safe Shutdown");

	exit(1);
	/* Send SIGINT to main thread to stop File Manager process and cleanly close vService Channel */
/*	kill(getpid(), SIGINT); */
}

int dbus_server_init()
{
	service_adaptor_debug("[Start] %s", __FUNCTION__);
	if ((NULL != introspection_data) || (0 != owner_id)) {
		/* Server is already running */
		return -1;
	}

	service_adaptor_debug("[Step]");
	introspection_data = g_dbus_node_info_new_for_xml(introspection_xml, NULL);
	if (NULL == introspection_data) {
		return -1;
	}

	service_adaptor_debug("[Step]");
	thread_pool = g_thread_pool_new(method_call_async_func, NULL, -1, FALSE, NULL);

	if (NULL == thread_pool) {
		return -1;
	}

	service_adaptor_debug("[Step] thread pool (%p)", thread_pool);
	owner_id = g_bus_own_name(G_BUS_TYPE_SYSTEM,
			SERVICE_ADAPTOR_BUS_NAME,
			G_BUS_NAME_OWNER_FLAGS_NONE,
			on_bus_acquired,
			on_name_acquired,
			on_name_lost,
			NULL,
			NULL);

	service_adaptor_debug("[Step] owner_id %u", owner_id);
	if (0 == owner_id) {
		g_dbus_node_info_unref(introspection_data);
		introspection_data = NULL;
		return -1;
	}

	service_adaptor_debug("[End] %s", __FUNCTION__);
	return 0;
}

void dbus_server_deinit()
{
	if (NULL != thread_pool) {
		g_thread_pool_free(thread_pool, TRUE, TRUE);
	}

	if (0 != owner_id) {
		g_bus_unown_name(owner_id);
		owner_id = 0;
	}

	if (NULL != introspection_data) {
		g_dbus_node_info_unref(introspection_data);
		introspection_data = NULL;
	}
}
/* EOF */
