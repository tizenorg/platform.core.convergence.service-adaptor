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
 * File: dbus-client.h
 * Desc: D-Bbus IPC client APIs
 * Created on: Oct, 2014
 * Auth: Yongjin Kim <youth.kim@samsung.com>
 *
 *****************************************************************************/
/**
 *	@file		dbus-client.h
 *	@brief		Defines interface of D-Bus IPC
 *	@version	0.1
 */

#ifndef __TIZEN_SOCIAL_SERVICE_ADAPTOR_DBUS_CLIENT_H__
#define __TIZEN_SOCIAL_SERVICE_ADAPTOR_DBUS_CLIENT_H__

#include <glib.h>
#include <gio/gio.h>
#include "service_adaptor_client_type.h"
#include "service_adaptor_client_private.h"

#define __safe_add_string(x)	(x==NULL)?"":x

#define SECURITY_SERVER_COOKIE_BUFFER_SIZE	21

/*==================================================================================================
                                         FUNCTION PROTOTYPES
==================================================================================================*/

GDBusProxy *_dbus_get_sac_interface_proxy();

/**
 * @brief Service Adaptor D-Bus client initialization.
 */
int _dbus_client_service_adaptor_init();

/**
 * @brief Service Adaptor D-Bus client deinitialization.
 */
void _dbus_client_service_adaptor_deinit();


/**
 * @brief Adds string into variant builder
 */
void __safe_g_variant_builder_add_string(GVariantBuilder *builder,
						const char *data);

/**
 * @brief Adds string into array variant builder
 */
void __safe_g_variant_builder_add_array_string(GVariantBuilder *builder,
						const char *data);

/**
 * @brief Returns NULL point if string is ""
 */
char *ipc_g_variant_dup_string(GVariant *string);

/**
 * @brief
 * @param[out]
 * @param[out]
 * @return
 * @pre This function requires opened DBus connection by service-adaptor-client.c
 */
int _dbus_connect_service_adaptor(service_adaptor_error_s *error);

int _dbus_disconnect_service_adaptor(service_adaptor_error_s *error);

int _dbus_get_plugin_list(plugin_entry_t ***plugin_list, unsigned int *plugins_len, service_adaptor_error_s *error);

int _dbus_is_login_required(service_plugin_h plugin, bool *required, service_adaptor_error_s *error);

int _dbus_request_login(service_plugin_h plugin, void *callback, void *user_data, service_adaptor_error_s *error);

int _dbus_start_service(service_plugin_h plugin, int service_flag, const char *security_cookie, service_adaptor_error_s *error);

int _dbus_external_request(const char *service_name,
						int service_flag,
						const char *api_uri,
						unsigned char *input_str,
						int input_len,
						unsigned char **output_str,
						int *output_len,
						service_adaptor_error_s *error);

#endif /* __TIZEN_SOCIAL_SERVICE_ADAPTOR_DBUS_CLIENT_H__ */
