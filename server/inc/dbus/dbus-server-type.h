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

#ifndef __DBUS_SERVER_TYPE_H__
#define __DBUS_SERVER_TYPE_H__

#include <glib.h>
#include <gio/gio.h>

/**
 * @brief Initialises D-Bus server.
 *
 * Initialises D-Bus server. Must by called on startup.
 * @return 0 on success, -1 on error.
 */
int dbus_server_init();

/**
 * @brief Deinitialises D-Bus server.
 *
 * Deinitialises D-Bus server. Must by called on shutdown.
 */
void dbus_server_deinit();

GDBusConnection *dbus_get_connection();

#endif /* __DBUS_SERVER_TYPE_H__ */
