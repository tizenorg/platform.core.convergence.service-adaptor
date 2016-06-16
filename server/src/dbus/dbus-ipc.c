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

#include <glib.h>

#include "service-adaptor-log.h"
#include "dbus/dbus-ipc.h"
#include "dbus/dbus-server.h"
#include "dbus/dbus-server-type.h"

/**
 * D-Bus server thread.
 */
static GThread* dbusServerThread = NULL;

/**
 * D-Bus server thread main loop context.
 */
static GMainContext* dbusServerMainContext = NULL;

/**
 * D-Bus server thread main loop.
 */
static GMainLoop* dbusServerMainLoop = NULL;

/**
 * @brief D-Bus server thread function.
 *
 * D-Bus server thread function. It initialises structures and callbacks needed to export D-Bus interfaces.
 * @param data Data passed to thread.
 * @return Result data from thread (always NULL in this implementation).
 */
static gpointer dbus_server_thread_func(gpointer data)
{
	int ret = 0;

	service_adaptor_debug("[Start] %s", __FUNCTION__);
	dbusServerMainContext = g_main_context_new();
	dbusServerMainLoop = g_main_loop_new(dbusServerMainContext, FALSE);
	g_main_context_push_thread_default(dbusServerMainContext);

	ret = dbus_server_init();

	if (ret == 0) {
		g_main_loop_run(dbusServerMainLoop);
	}

	dbus_server_deinit();

	g_main_context_pop_thread_default(dbusServerMainContext);
	g_main_loop_unref(dbusServerMainLoop);
	dbusServerMainLoop = NULL;
	g_main_context_unref(dbusServerMainContext);
	dbusServerMainContext = NULL;

	return NULL;
}

int dbus_ipc_server_layer_init()
{
	if (dbusServerThread != NULL) {
		/* D-Bus server thread is already running */
		return 1;
	}
	dbusServerThread = g_thread_new("D-Bus Server Thread", dbus_server_thread_func, NULL);

	return 0;
}

void dbus_ipc_server_layer_deinit()
{
	if (dbusServerMainLoop != NULL) {
		if (g_main_loop_is_running(dbusServerMainLoop)) {
			g_main_loop_quit(dbusServerMainLoop);
		}
	}
	if (dbusServerThread != NULL) {
		g_thread_join(dbusServerThread);
		dbusServerThread = NULL;
	}
	if (dbusServerMainLoop != NULL) {
		g_main_loop_unref(dbusServerMainLoop); //LCOV_EXCL_LINE
		dbusServerMainLoop = NULL; //LCOV_EXCL_LINE
	}
	if (dbusServerMainContext != NULL) {
		g_main_context_pop_thread_default(dbusServerMainContext); //LCOV_EXCL_LINE
		g_main_context_unref(dbusServerMainContext); //LCOV_EXCL_LINE
		dbusServerMainContext = NULL; //LCOV_EXCL_LINE
	}
}

/* EOF */
