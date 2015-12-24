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

#include "sal_engine.h"

/******************************************************************************
 * Global variables and defines
 ******************************************************************************/

static sal_engine_mode_e g_mode;

/*
static GMainLoop *ipc_engines[SAL_ENGINE_SERVICE] = NULL;

static GMainLoop *ipc_adaptor_engine = NULL;

static GMainLoop *main_engine = NULL;

static GMainLoop *engines[SAL_ENGINE_SERVICE] = NULL;

static GMainLoop *file_engine = NULL;
*/
static GMainLoop *engine_loops[5] = {NULL, };

static GThread *engine_threads[5] = {NULL, };

static char *engine_names[5] = {"main", "service", "file", "ipc server", "ipc adaptor"};

USER_DATA_TYPEDEF(engine_task_data_t, 2);

/******************************************************************************
 * Private interface
 ******************************************************************************/

static void *__engine_runnable(void *user_data);

static gboolean *__engine_task_runnable(void *data);

/******************************************************************************
 * Private interface definition
 ******************************************************************************/

static void *__engine_runnable(void *user_data)
{
	SAL_FN_CALL;
	GMainLoop *loop = (GMainLoop *)user_data;
	int n = 0;
	for (n = SAL_ENGINE_MAIN; n < SAL_ENGINE_MAX; n++) {
		if (loop == engine_loops[n])
			break;
	}
	if (SAL_ENGINE_MAX < n) {
		SAL_ERR("Thread critical error");
		return NULL;
	}

	GMainContext *ctx = g_main_loop_get_context(engine_loops[n]);
	g_main_context_push_thread_default(ctx);

	SAL_INFO("==: Engine thread running success <%s>, ", engine_names[n]);
	g_main_loop_run(engine_loops[n]);
	SAL_INFO("==: Engine thread loop finished <%s>, ", engine_names[n]);

	g_main_context_pop_thread_default(ctx);

	engine_threads[n] = NULL;
	SAL_FN_END;
	return NULL;
}

static gboolean *__engine_task_runnable(void *data)
{
	SAL_FN_CALL;
	RETV_IF(NULL == data, FALSE);

	USER_DATA_DEFINE(engine_task_data_t, task_data) = (USER_DATA_TYPE(engine_task_data_t) *)data;
	sal_engine_task_logic callback = (sal_engine_task_logic)USER_DATA_ELEMENT(task_data, 0);
	user_data_t user_data = (user_data_t)USER_DATA_ELEMENT(task_data, 1);

	if (callback) {
		SAL_DBG("----- Task handoff start -----");
		callback(user_data);
		SAL_DBG("----- Task handoff end -----");
	}

	SAL_FN_END;
	return FALSE;
}

/******************************************************************************
 * Public interface definition
 ******************************************************************************/

int sal_engine_init(sal_engine_mode_e mode)
{
	SAL_FN_CALL;
	int ret = SAL_ERROR_NONE;
	g_mode = mode;

	GMainContext *main_context = NULL;
	GMainContext *server_context = NULL;
	GMainContext *file_context = NULL;
	GMainContext *ipc_server_context = NULL;
	GMainContext *ipc_adaptor_context = NULL;

	main_context = g_main_context_new();
	TRY_IF(NULL == main_context);
	engine_loops[SAL_ENGINE_MAIN] = g_main_loop_new(main_context, FALSE);
	TRY_IF(NULL == engine_loops[SAL_ENGINE_MAIN]);

	server_context = g_main_context_new();
	TRY_IF(NULL == server_context);
	engine_loops[SAL_ENGINE_SERVICE] = g_main_loop_new(server_context, FALSE);
	TRY_IF(NULL == server_context);

	file_context = g_main_context_new();
	TRY_IF(NULL == file_context);
	engine_loops[SAL_ENGINE_FILE] = g_main_loop_new(file_context, FALSE);
	TRY_IF(NULL == engine_loops[SAL_ENGINE_FILE]);

	if (SAL_ENGINE_MODE_DEFAULT == mode) {
		ipc_server_context = g_main_context_new();
		TRY_IF(NULL == ipc_server_context);
		engine_loops[SAL_ENGINE_IPC_SERVER] = g_main_loop_new(ipc_server_context, FALSE);
		TRY_IF(NULL == engine_loops[SAL_ENGINE_IPC_SERVER]);

		ipc_adaptor_context = g_main_context_new();
		TRY_IF(NULL == ipc_adaptor_context);
		engine_loops[SAL_ENGINE_IPC_ADAPTOR] = g_main_loop_new(ipc_adaptor_context, FALSE);
		TRY_IF(NULL == engine_loops[SAL_ENGINE_IPC_ADAPTOR]);
	} else {
		ipc_server_context = main_context;
		engine_loops[SAL_ENGINE_IPC_SERVER] = engine_loops[SAL_ENGINE_MAIN];

		ipc_adaptor_context = server_context;
		engine_loops[SAL_ENGINE_IPC_ADAPTOR] = engine_loops[SAL_ENGINE_SERVICE];
	}

	SAL_FN_END;
	return ret;

catch:
	if (engine_loops[SAL_ENGINE_MAIN])
		g_main_loop_unref(engine_loops[SAL_ENGINE_MAIN]);
	if (engine_loops[SAL_ENGINE_SERVICE])
		g_main_loop_unref(engine_loops[SAL_ENGINE_SERVICE]);
	if (engine_loops[SAL_ENGINE_FILE])
		g_main_loop_unref(engine_loops[SAL_ENGINE_FILE]);

	if (SAL_ENGINE_MODE_DEFAULT == mode) {
		if (engine_loops[SAL_ENGINE_IPC_SERVER])
			g_main_loop_unref(engine_loops[SAL_ENGINE_IPC_SERVER]);
		if (engine_loops[SAL_ENGINE_IPC_ADAPTOR])
			g_main_loop_unref(engine_loops[SAL_ENGINE_IPC_ADAPTOR]);
	}

	if (main_context)
		g_main_context_unref(main_context);
	if (server_context)
		g_main_context_unref(server_context);
	if (file_context)
		g_main_context_unref(file_context);

	if (SAL_ENGINE_MODE_DEFAULT == mode) {
		if (ipc_server_context)
			g_main_context_unref(ipc_server_context);
		if (ipc_adaptor_context)
			g_main_context_unref(ipc_adaptor_context);
	}

	engine_loops[SAL_ENGINE_MAIN] = NULL;
	engine_loops[SAL_ENGINE_SERVICE] = NULL;
	engine_loops[SAL_ENGINE_FILE] = NULL;
	engine_loops[SAL_ENGINE_IPC_SERVER] = NULL;
	engine_loops[SAL_ENGINE_IPC_ADAPTOR] = NULL;

	SAL_FN_END;
//	return SAL_ERROR_OUT_OF_MEMORY;
	return SAL_ERROR_SYSTEM;
}

int sal_engine_deinit()
{
	SAL_FN_CALL;
	int ret = SAL_ERROR_NONE;
	sal_engine_mode_e mode = g_mode;

	if (engine_loops[SAL_ENGINE_MAIN]) {
		g_main_context_unref(g_main_loop_get_context(engine_loops[SAL_ENGINE_MAIN]));
		g_main_loop_unref(engine_loops[SAL_ENGINE_MAIN]);
	}
	if (engine_loops[SAL_ENGINE_SERVICE]) {
		g_main_context_unref(g_main_loop_get_context(engine_loops[SAL_ENGINE_SERVICE]));
		g_main_loop_unref(engine_loops[SAL_ENGINE_SERVICE]);
	}
	if (engine_loops[SAL_ENGINE_FILE])
		g_main_context_unref(g_main_loop_get_context(engine_loops[SAL_ENGINE_FILE]));
		g_main_loop_unref(engine_loops[SAL_ENGINE_FILE]);

	if (SAL_ENGINE_MODE_DEFAULT == mode) {
		if (engine_loops[SAL_ENGINE_IPC_SERVER]) {
			g_main_context_unref(g_main_loop_get_context(engine_loops[SAL_ENGINE_IPC_SERVER]));
			g_main_loop_unref(engine_loops[SAL_ENGINE_IPC_SERVER]);
		}
		if (engine_loops[SAL_ENGINE_IPC_ADAPTOR]) {
			g_main_context_unref(g_main_loop_get_context(engine_loops[SAL_ENGINE_IPC_ADAPTOR]));
			g_main_loop_unref(engine_loops[SAL_ENGINE_IPC_ADAPTOR]);
		}
	}

	engine_loops[SAL_ENGINE_MAIN] = NULL;
	engine_loops[SAL_ENGINE_SERVICE] = NULL;
	engine_loops[SAL_ENGINE_FILE] = NULL;
	engine_loops[SAL_ENGINE_IPC_SERVER] = NULL;
	engine_loops[SAL_ENGINE_IPC_ADAPTOR] = NULL;

	SAL_FN_END;
	return ret;
}

GMainLoop *sal_get_engine_loop(sal_engine_e engine)
{
	if ((0 < engine) && (SAL_ENGINE_MAX > engine))
		return engine_loops[engine];

	return NULL;
}

int sal_engine_run(sal_engine_e engine)
{
	if ((SAL_ENGINE_MAIN > engine) || (SAL_ENGINE_MAX <= engine)) {
		return SAL_ERROR_INVALID_PARAMETER;
	}

	if (g_main_loop_is_running(engine_loops[engine])) {
		SAL_INFO("engine (%d) already running");
		return SAL_ERROR_NONE;
	}

	engine_threads[engine] = g_thread_new(engine_names[engine], __engine_runnable, (void *)engine_loops[engine]);
	RETVM_IF(NULL == engine_threads[engine], SAL_ERROR_INTERNAL, "Thread creation failed");

	return SAL_ERROR_NONE;
}

int sal_engine_quit(sal_engine_e engine)
{
	if ((SAL_ENGINE_MAIN > engine) || (SAL_ENGINE_MAX <= engine)) {
		return SAL_ERROR_INVALID_PARAMETER;
	}

	RETV_IF(NULL == engine_loops[engine], SAL_ERROR_NONE);

	if (FALSE == g_main_loop_is_running(engine_loops[engine])) {
		SAL_INFO("engine (%d) does not running");
		return SAL_ERROR_INTERNAL;
	}

	g_main_loop_quit(engine_loops[engine]);

	return SAL_ERROR_NONE;
}

int sal_engine_main_run()
{
	RETV_IF(NULL == engine_loops[SAL_ENGINE_MAIN], SAL_ERROR_INVALID_PARAMETER);

	g_main_loop_run(engine_loops[SAL_ENGINE_MAIN]);

	return SAL_ERROR_NONE;
}

int sal_engine_main_quit()
{
	RETV_IF(NULL == engine_loops[SAL_ENGINE_MAIN], SAL_ERROR_INVALID_PARAMETER);

	if (FALSE == g_main_loop_is_running(engine_loops[SAL_ENGINE_MAIN])) {
		SAL_INFO("engine (%d) does not running");
		return SAL_ERROR_INTERNAL;
	}

	g_main_loop_quit(engine_loops[SAL_ENGINE_MAIN]);
	return SAL_ERROR_NONE;
}

int sal_engine_task_handoff(sal_engine_e target_engine, sal_engine_task_logic logic, user_data_t data)
{
	SAL_FN_CALL;
	RETV_IF(NULL == engine_threads[target_engine], SAL_ERROR_INTERNAL);
	RETV_IF(FALSE == g_main_loop_is_running(engine_loops[target_engine]), SAL_ERROR_INTERNAL);

	USER_DATA_DEFINE(engine_task_data_t, task_data) = USER_DATA_CREATE(engine_task_data_t);
	RETV_IF(NULL == USER_DATA_VAL(task_data), SAL_ERROR_SYSTEM);

	USER_DATA_ELEMENT(task_data, 0) = (user_data_t)logic;
	USER_DATA_ELEMENT(task_data, 1) = (user_data_t)data;

	g_main_context_invoke_full(g_main_loop_get_context(engine_loops[target_engine]),
			G_PRIORITY_DEFAULT, __engine_task_runnable,
			USER_DATA_VAL(task_data), USER_DATA_DESTROY_FUNC);

	SAL_FN_END;
	return SAL_ERROR_NONE;
}

