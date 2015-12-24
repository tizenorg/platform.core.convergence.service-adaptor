/*
 * Service Adaptor
 *
 * Copyright (c) 2014 - 2015 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <glib.h>

#include "sal_log.h"
#include "sal_types.h"
#include "sal_ipc.h"

#include "sal_base.h"

#include "sal_ipc_server_core.h"

/******************************************************************************
 * Global variables and defines
 ******************************************************************************/

/******************************************************************************
 * Private interface
 ******************************************************************************/

/******************************************************************************
 * Private interface definition
 ******************************************************************************/


/******************************************************************************
 * Public interface definition
 ******************************************************************************/

void client_connect_cb(ipc_server_session_h session, int pid, const char *client_uri)
{
	SAL_FN_CALL;
	SAL_FN_END;
}

void client_disconnect_cb(ipc_server_session_h session, int pid, const char *client_uri)
{
	SAL_FN_CALL;
	SAL_FN_END;
}

void client_plugin_start_cb(ipc_server_session_h session,
		int client_pid, const char *client_uri,
		const char *plugin_uri, int service_mask) /* TODO: add property param */
{
	SAL_FN_CALL;
	SAL_FN_END;
}

void client_plugin_stop_cb(ipc_server_session_h session,
		const char *plugin_handle)
{
	SAL_FN_CALL;
	SAL_FN_END;
}
