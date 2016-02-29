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

#ifndef __SERVICE_ADAPTOR_TYPE_H__
#define __SERVICE_ADAPTOR_TYPE_H__

#include <stdio.h>
#include "service-adaptor.h"
#include "auth-adaptor.h"
#include "contact-adaptor.h"
#include "message-adaptor.h"
#include "shop-adaptor.h"
#include "storage-adaptor.h"
#include "push-adaptor.h"

#define service_adaptor_safe_free(p) safe_free((void**)&(p))

#define SERVICE_ADAPTOR_OPERATION_PUSH_NOTI_TO_APPCONTROL "http://tizen.org/serviceadaptor/operation/v1/push"

typedef struct _service_adaptor_context_info_s
{
	char *user_id;
	char *app_id;
	unsigned int service_id;
	char *imsi;
	char *duid;
	char *msisdn;
	char *access_token;
	char *refresh_token;
	void *property;
} service_adaptor_context_info_s;

typedef struct _service_adaptor_service_context_s
{
	char *service_name;		// com.serviceadaptor.service1
	char *plugin_uri;

	int authenticated;
	int connected;

	service_adaptor_context_info_s *context_info;

	GMutex service_context_mutex;
	GCond service_context_cond;
	GHashTable *server_info;

	auth_adaptor_plugin_context_h		auth_context;
	contact_adaptor_plugin_context_h	contact_context;
	message_adaptor_plugin_context_h	message_context;
	shop_adaptor_plugin_context_h		shop_context;
	storage_adaptor_plugin_context_h	storage_context;
	push_adaptor_plugin_context_h		push_context;
} service_adaptor_service_context_s;
typedef struct _service_adaptor_service_context_s *service_adaptor_service_context_h;

typedef struct _service_adaptor_s
{
	int started;

	GMutex service_adaptor_mutex;
	GCond service_adaptor_cond;

	auth_adaptor_h			auth_handle;
	contact_adaptor_h		contact_handle;
	message_adaptor_h		message_handle;
	shop_adaptor_h			shop_handle;
	storage_adaptor_h		storage_handle;
	push_adaptor_h			push_handle;

	auth_adaptor_listener_h		auth_listener;
	contact_adaptor_listener_h	contact_listener;
	message_adaptor_listener_h	message_listener;
	shop_adaptor_listener_h		shop_listener;
	storage_adaptor_listener_h	storage_listener;
	push_adaptor_listener_h		push_listener;

	GList *service_list;		// service_adaptor_service_context_h
} service_adaptor_s;
typedef struct _service_adaptor_s *service_adaptor_h;

// Gets service adaptor handle
service_adaptor_h service_adaptor_get_handle();

service_adaptor_internal_error_code_e service_adaptor_init();
void service_adaptor_deinit();

// Gets adaptor context
service_adaptor_service_context_h service_adaptor_get_service_context(service_adaptor_h service_adaptor,
						const char *service_name);

GList *service_adaptor_get_services_by_plugin_uri(service_adaptor_h service_adaptor,
						const char *plugin_uri);

service_adaptor_internal_error_code_e service_adaptor_bind_context(service_adaptor_h service_adaptor,
						service_adaptor_service_context_h service_src,
						service_adaptor_service_context_h service_dst);

service_adaptor_internal_error_code_e service_adaptor_bind_storage_context(service_adaptor_h service_adaptor,
						service_adaptor_service_context_h service_src,
						service_adaptor_service_context_h service_dst);

service_adaptor_internal_error_code_e service_adaptor_bind_push_context(service_adaptor_h service_adaptor,
						service_adaptor_service_context_h service_src,
						service_adaptor_service_context_h service_dst);

// Create / Destroy adaptors (Internal function)
service_adaptor_h service_adaptor_create();
void service_adaptor_destroy(service_adaptor_h service_adaptor);

// Connects / Disconnects adaptors
service_adaptor_internal_error_code_e service_adaptor_connect(service_adaptor_h service_adaptor,
						service_adaptor_context_info_s *context_info,
						const char *service_name,
						const char *plugin_uri,
						const char *user_password,
						const char *app_secret,
						bool auth_enable,
						bool storage_enable,
						bool contact_enable,
						bool message_enable,
						bool push_enable,
						bool shop_enable,
						char *ret_msg);

service_adaptor_internal_error_code_e service_adaptor_disconnect(service_adaptor_h service_adaptor,
						const char *service_name);

service_adaptor_internal_error_code_e service_adaptor_auth_refresh(service_adaptor_h service_adaptor,
						const char *service_name,
						const char *plugin_uri);

service_adaptor_internal_error_code_e service_adaptor_auth_refresh_with_service_context(service_adaptor_h service_adaptor,
						service_adaptor_service_context_h service_context,
						const char *plugin_uri);

int service_adaptor_is_service_binded(service_adaptor_h service_adaptor,
						const char *service_package_id);

#ifdef SERVICE_ADAPTOR_DEBUG_CONTEXT
void debug_service_context(GList *service_list);
#endif

typedef enum
{
	SA_TIME_CHECK_FLAG_AUTH,
	SA_TIME_CHECK_FLAG_STORAGE,
	SA_TIME_CHECK_FLAG_CONTACT,
	SA_TIME_CHECK_FLAG_MESSAGE,
	SA_TIME_CHECK_FLAG_PUSH,
	SA_TIME_CHECK_FLAG_SHOP,
}sa_time_check_flag_e;

void SERVICE_ADAPTOR_API_TIME_CHECK_START();
void SERVICE_ADAPTOR_API_TIME_CHECK_PAUSE();

void SERVICE_ADAPTOR_PLUGIN_API_TIME_CHECK_START(sa_time_check_flag_e flag);
void SERVICE_ADAPTOR_PLUGIN_API_TIME_CHECK_PAUSE(sa_time_check_flag_e flag);

void SERVICE_ADAPTOR_API_TIME_CHECK_TOTAL_REPORT(const char *service_name);

#endif /* __SERVICE_ADAPTOR_TYPE_H__ */
