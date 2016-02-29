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
 * File: dbus-client-auth.h
 * Desc: D-Bbus IPC client APIs for auth
 * Created on: Oct, 2014
 * Auth: Yongjin Kim <youth.kim@samsung.com>
 *
 *****************************************************************************/
/**
 *	@file		dbus-client-auth.h
 *	@brief		Defines interface of D-Bus IPC
 *	@version	0.1
 */

#ifndef __DBUS_CLIENT_AUTH_H__
#define __DBUS_CLIENT_AUTH_H__

#include <glib.h>
#include "service_adaptor_client_type.h"
#include "private/service-adaptor-client-auth.h"

/**
 * @brief
 * @param[out]
 * @param[out]
 * @return
 * @pre This function requires opened DBus connection by service-adaptor-client-auth.c
 */
int _dbus_get_auth_plugin_list(GList **plugin_list,
						const char *imsi,
						service_adaptor_error_s *error);

int _dbus_set_auth(const char *service_name,
						const char *imsi,
						const char *name,
						const char *app_id,
						const char *app_secret,
						unsigned int service_id,
						void *user_data,
						service_adaptor_error_s *error);

#endif /* __DBUS_CLIENT_AUTH_H__ */

