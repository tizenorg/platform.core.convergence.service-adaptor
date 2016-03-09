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
#include <unistd.h>
#include <sys/types.h>
#include <stdarg.h>
#include <stdint.h>

#include <glib.h>

#include "service-adaptor-log.h"
#include "util/ping_manager.h"
#include "service-adaptor-push.h"

/*************************************************
 *               Type definition
 *************************************************/

#define POINTER_TO_PINFO(pt)	((peer_info_t)(intptr_t) (pt))
#ifndef	DETAILED_PEER
	#define PINFO_TO_POINTER(pi)	((void *)(intptr_t) (pi))
#else
	#define PINFO_TO_POINTER(pi)	((void *) (pi))
#endif
#define COMPARE_PINFO(a, b)		((a) - (b))

#define	DAEMON_CONTINUE		FALSE
#define	DAEMON_WILL_STOP	TRUE

#define NO_ANY_ACCESS		TRUE
#define ACCESS_EXISTED		FALSE

#define PEERS_INFO_DEBUG

/*************************************************
 *               Global valuable
 *************************************************/

/*	static GAsyncQueue *peers_list = NULL; */
static GList *peers_list = NULL;
G_LOCK_DEFINE(locker);
static GMainContext *main_ctx = NULL;
static gboolean g_ping_watcher_stop = TRUE;

#ifdef PEERS_INFO_DEBUG
static int repet_print_count = 0;
static int repet_print_count_max = 5;
#endif

gboolean g_suspend_flag	= DAEMON_CONTINUE;
gboolean g_suspend_flag_integrity = ACCESS_EXISTED;

/*************************************************
 *               Internal function prototype
 *************************************************/

static void __remove_peer_info(peer_info_t pinfo);

static void __add_peer_info(peer_info_t pinfo);

static int __get_peers_length(void);

static gboolean __ping_watcher_cb(gpointer user_data);

static void __peer_info_destroy(void *info);

/*************************************************
 *               Internal function definition
 *************************************************/

static void __remove_peer_info(peer_info_t pinfo)
{
	G_LOCK(locker);
	#ifndef	DETAILED_PEER
	peers_list = g_list_remove(peers_list, PINFO_TO_POINTER(pinfo));
	#else
	int len = (int)g_list_length(peers_list);
	peer_info_t find = NULL, current = NULL;
	for (int i = 0; i < len; i++) {
		current = (peer_info_t)g_list_nth_data(peers_list, i);
		if (current) {
			if (current->pid == pinfo->pid) {
				find = current;
				break;
			}
		}
	}
	peers_list = g_list_remove(peers_list, PINFO_TO_POINTER(find));
	__peer_info_destroy(PINFO_TO_POINTER(find));
	#endif

	#ifdef PEERS_INFO_DEBUG
	repet_print_count = 0;
	#endif

	G_UNLOCK(locker);
}

static void __add_peer_info(peer_info_t pinfo)
{
	G_LOCK(locker);
	#ifndef	DETAILED_PEER
	peers_list = g_list_append(peers_list, PINFO_TO_POINTER(pinfo));
	#else
	peer_info_t pi = (peer_info_t) calloc(1, sizeof(peer_info_s));
	if (pi) {
		pi->pid = pinfo->pid;
		pi->name = strdup((pinfo->name) ? (pinfo->name) : "");
		peers_list = g_list_append(peers_list, PINFO_TO_POINTER(pi));
	}
	#endif

	g_suspend_flag_integrity = ACCESS_EXISTED;

	#ifdef PEERS_INFO_DEBUG
	repet_print_count = 0;
	#endif

	G_UNLOCK(locker);
}

#ifdef PEERS_INFO_DEBUG
void _print_info_cb(gpointer data, gpointer user_data)
{
	#ifndef	DETAILED_PEER
	service_adaptor_info("--- > iterator : %d", (int) POINTER_TO_PINFO(data));
	#else
	service_adaptor_info("--- > iterator : <%d> %s", (POINTER_TO_PINFO(data))->pid, (POINTER_TO_PINFO(data))->name);
	#endif
}
#endif

static int __get_peers_length()
{
	G_LOCK(locker);
	#ifdef PEERS_INFO_DEBUG
	g_list_foreach(peers_list, _print_info_cb, NULL);
	#endif
	int ret = (int) g_list_length(peers_list);
	G_UNLOCK(locker);

	push_activate_h *push_services = NULL;
	int push_len = 0;
	service_adaptor_ref_enabled_push_services(&push_services, &push_len);

	ret += push_len;
	#ifdef PEERS_INFO_DEBUG
		#ifndef	DETAILED_PEER
		service_adaptor_info("--- > registered push : %d files", push_len);
		#else
		for (int k = 0; k < push_len; k++) {
			service_adaptor_info("--- > registered push : %s", push_services[k]->file_name);
		}
		#endif
	#endif
	free(push_services);
	push_services = NULL;

	return ret;
}

static gboolean __ping_watcher_cb(gpointer user_data)
{
	#ifdef PEERS_INFO_DEBUG
	if (repet_print_count_max >= (++repet_print_count)) {
	#endif

	service_adaptor_info("=== === === ping manager loop === === ===");

	int len = __get_peers_length();
	service_adaptor_info("--- peers len : %d", len);
	if (len == 0) {
		if (g_suspend_flag == DAEMON_WILL_STOP) {
			if (g_suspend_flag_integrity == ACCESS_EXISTED) {
				service_adaptor_info("--- --- violated integrity");
				service_adaptor_info("--- --- suspend dleay to next term");
				g_suspend_flag = DAEMON_WILL_STOP;
				g_suspend_flag_integrity = NO_ANY_ACCESS;
			} else {
				/* daemon stop */
				service_adaptor_info("--- --- start daemon termination");
				g_ping_watcher_stop = FALSE;

				if (user_data) {
					service_adaptor_info("--- --- quit main loop");
					g_main_loop_quit((GMainLoop *)user_data);
				}
			}
		} else { /* == DAEMON_CONTINUE */
			g_suspend_flag = DAEMON_WILL_STOP;
			g_suspend_flag_integrity = NO_ANY_ACCESS;

		}
	} else {
		service_adaptor_info("--- --- clears suspend flag");
		g_suspend_flag	= DAEMON_CONTINUE;
		g_suspend_flag_integrity = ACCESS_EXISTED;
	}

	service_adaptor_info("=== === === return %s === === ===",
			(g_ping_watcher_stop == TRUE) ? "TRUE" : "FALSE");

	#ifdef PEERS_INFO_DEBUG
	}
	#endif
	return g_ping_watcher_stop;
}

static void __peer_info_destroy(void *info)
{
	#ifdef	DETAILED_PEER
	peer_info_t find = (peer_info_t)info;
	if (find) {
		free(find->name);
		find->name = NULL;
		free(find);
	}
	#endif
}

static void __ping_manager_clear_cb(gpointer data)
{
	service_adaptor_info("Func start <%s>", __FUNCTION__);
	ping_manager_deinit();
}

/*************************************************
 *               Public function prototype
 *************************************************/

void ping_manager_init(int interval, GMainLoop *loop)
{
	service_adaptor_info("Func start <%s>", __FUNCTION__);
	peers_list = NULL;

	#ifdef PEERS_INFO_DEBUG
	repet_print_count = 0;
	#endif

	main_ctx = g_main_context_default();
	GSource *src = g_timeout_source_new_seconds(interval);
	g_source_set_callback(src, __ping_watcher_cb, (void *)loop, __ping_manager_clear_cb);
	g_source_attach(src, main_ctx);
}

void ping_manager_deinit(void)
{
	service_adaptor_info("Func start <%s>", __FUNCTION__);
	G_LOCK(locker);
	g_list_free_full(peers_list, __peer_info_destroy);
	peers_list = NULL;
	G_UNLOCK(locker);
}

int ping_manager_peer_connected(peer_info_t info)
{
	service_adaptor_info("Func start <%s> [%d]", __FUNCTION__, (int)info);
	__remove_peer_info(info);
	__add_peer_info(info);
	int ret = __get_peers_length();
	service_adaptor_info("peers length : %d", ret);
	return ret;
}

int ping_manager_peer_disconnected(peer_info_t info)
{
	service_adaptor_info("Func start <%s> [%d]", __FUNCTION__, (int)info);
	__remove_peer_info(info);
	int ret = __get_peers_length();
	service_adaptor_info("peers length : %d", ret);
	return ret;
}

int ping_manager_get_connected_count(void)
{
	service_adaptor_info("Func start <%s>", __FUNCTION__);
	int ret = __get_peers_length();
	service_adaptor_info("peers length : %d", ret);
	return ret;
}

