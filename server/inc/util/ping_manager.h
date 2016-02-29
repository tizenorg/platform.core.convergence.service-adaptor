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

#ifndef __PING_MANAGER_H__
#define __PING_MANAGER_H__

#include <glib.h>

#define DETAILED_PEER

#ifndef DETAILED_PEER
typedef int peer_info_t;
#else
typedef struct _peer_info_s {
	int pid;
	char *name;
} peer_info_s;
typedef struct _peer_info_s *peer_info_t;
#endif

/*************************************************
 *               Public function prototype
 *************************************************/

void ping_manager_init(int interval, GMainLoop *loop);

void ping_manager_deinit(void);

int ping_manager_peer_connected(peer_info_t info);

int ping_manager_peer_disconnected(peer_info_t info);

int ping_manager_get_connected_count(void);



#endif /* __PING_MANAGER_H__ */
