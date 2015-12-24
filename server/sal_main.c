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

#include <stdio.h>
#include <glib-object.h>
#include <glib-unix.h>
/*
#include <app.h>

#include "service_adaptor_internal.h"

#include "sal.h"
#include "sal_auth.h"
#include "sal_contact.h"
#include "sal_storage.h"
#include "sal_resource.h"
#include "sal_observer.h"
#include "sal_ipc_server.h"
#include "sal_service_provider.h"

#include "service_discovery.h"
#include "service_federation.h"
*/

#include "sal_ipc_server.h"

#include "sal_log.h"
#include "sal_types.h"

#include "sal_engine.h"
#include "sal_base.h"

/******************************************************************************
 * Global variables and defines
 ******************************************************************************/
/*
#define FILE_PATH_LEN	256

static sal_h g_service_adaptor = NULL;
*/
/******************************************************************************
 * Private interface
 ******************************************************************************/

static int __thread_engine_init(void);

static int __thread_engine_deinit(void);

static int __ipc_server_init(void);

static int __ipc_server_deinit(void);

static void __glog_handler_cb(const gchar *log_domain,
		GLogLevelFlags log_level,
		const gchar *message,
		gpointer user_data);

/******************************************************************************
 * Private interface definition
 ******************************************************************************/

static void __glog_handler_cb(const gchar *log_domain,
		GLogLevelFlags log_level,
		const gchar *message,
		gpointer user_data)
{
	SAL_ERR("============================================================");
	SAL_ERR("============================================================");
	SAL_ERR("================== Critical GLib Error =====================");
	SAL_ERR("============================================================");
	SAL_ERR("============================================================");
	SAL_ERR("=== Log Domain : %s", log_domain);
	SAL_ERR("=== Level : %d", (int)log_level);
	SAL_ERR("=== Message : %s", message);
	SAL_ERR("============================================================");
	SAL_ERR("============================================================");
}

/**
 * @brief callback of app control
 *
 * @return      void.
 */
/*
static bool _app_control_extra_data_cb(app_control_h app_control, const char *key, void *user_data)
{
	char *value = NULL;

	app_control_get_extra_data(app_control, key, &value);
	SAL_INFO("PLUGIN Key (%s): %s", key, value);

	g_hash_table_insert((GHashTable *) user_data, (char *) key, value);

	return true;
}
*/
/**
 * @brief callback of plugin connection
 *
 * @return      void.
 */
/*
static void _provider_connect_cb(app_control_h request, app_control_h reply, app_control_result_e result, void *user_data)
{
	SAL_FN_CALL;

	RET_IF(APP_CONTROL_RESULT_FAILED == result);

	sal_h sal = sal_get_handle();
	RETM_IF(NULL == sal, "sal_get_handle() Fail");

	GHashTable *service = g_hash_table_new(g_str_hash, g_str_equal);
	app_control_foreach_extra_data(reply, _app_control_extra_data_cb, (void *) service);

	provider_user_data_h provider_user_data = (provider_user_data_h) user_data;

	GHashTableIter iter;
	gpointer iter_key, iter_value;

	g_hash_table_iter_init(&iter, service);
	while (g_hash_table_iter_next(&iter, &iter_key, &iter_value)) {
		if (0 == strcmp(iter_key, PLUGIN_KEY_AUTH)) {
			auth_plugin_h plugin = NULL;
			auth_adaptor_create_plugin(provider_user_data->uri, provider_user_data->name, provider_user_data->package, &plugin);
			auth_adaptor_register_plugin_service(plugin, service);
			auth_adaptor_add_plugin(sal->auth, plugin);
		} else if (0 == strcmp(iter_key, PLUGIN_KEY_STORAGE)) {
			storage_plugin_h plugin = NULL;
			storage_adaptor_create_plugin(provider_user_data->uri, provider_user_data->name, provider_user_data->package, &plugin);
			storage_adaptor_register_plugin_service(plugin, service);
			storage_adaptor_add_plugin(sal->storage, plugin);
		}
	}

	g_hash_table_destroy(service);

	SAL_FN_END;
}
*/
/**
 * @brief callback of plugin disconnection
 *
 * @return      void.
 */
/*
static void _provider_disconnect_cb(app_control_h request, app_control_h reply, app_control_result_e result, void *user_data)
{
	SAL_FN_CALL;

	RET_IF(APP_CONTROL_RESULT_FAILED == result);

	sal_h sal = sal_get_handle();
	RETM_IF(NULL == sal, "sal_get_handle() Fail");

	char *uri = (char *) user_data;

	auth_plugin_h plugin = auth_adaptor_get_plugin(sal->auth, uri);
	auth_adaptor_remove_plugin(sal->auth, plugin);
	auth_adaptor_unregister_plugin_service(plugin);
	auth_adaptor_destroy_plugin(plugin);

	// TODO: destroy plugin of other adaptor

	SAL_FN_END;
}
*/
/**
 * @brief create spec file
 *
 * @return      void.
 */
/*
static service_adaptor_error_e _sal_create_spec_file(sal_h sal)
{
	SAL_FN_CALL;

	int ret = 0;
	RETV_IF(NULL == sal, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	return ret;
}
*/
/**
 * @brief destroy spec file
 *
 * @return      void.
 */
/*
static service_adaptor_error_e _sal_destroy_spec_file(sal_h sal)
{
	SAL_FN_CALL;

	int ret = 0;
	RETV_IF(NULL == sal, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	return ret;
}
*/
/**
 * @brief start service adaptor
 *
 * @return      void.
 */
/*
static service_adaptor_error_e _sal_start(sal_h sal)
{
	SAL_FN_CALL;

	int ret = 0;
	RETV_IF(NULL == sal, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	ret = auth_adaptor_start(sal->auth);
	RETVM_IF(SERVICE_ADAPTOR_ERROR_NONE != ret, SERVICE_ADAPTOR_ERROR_INTERNAL, "auth_adaptor_start() Fail(%d)", ret);

	ret = contact_adaptor_start(sal->contact);
	RETVM_IF(SERVICE_ADAPTOR_ERROR_NONE != ret, SERVICE_ADAPTOR_ERROR_INTERNAL, "contact_adaptor_start() Fail(%d)", ret);

	ret = storage_adaptor_start(sal->storage);
	RETVM_IF(SERVICE_ADAPTOR_ERROR_NONE != ret, SERVICE_ADAPTOR_ERROR_INTERNAL, "storage_adaptor_start() Fail(%d)", ret);

	ret = resource_adaptor_start(sal->resource);
	RETVM_IF(SERVICE_ADAPTOR_ERROR_NONE != ret, SERVICE_ADAPTOR_ERROR_INTERNAL, "resource_adaptor_start() Fail(%d)", ret);

	ret = sal_observer_register_existed_plugin();
	RETVM_IF(SERVICE_ADAPTOR_ERROR_NONE != ret, ret, "sal_observer_register_existed_plugin() Fail(%d)", ret);

	g_mutex_lock(&sal->mutex);
	sal->start = sal->start + 1;
	g_cond_signal(&sal->cond);
	g_mutex_unlock(&sal->mutex);

	return ret;
}
*/
/**
 * @brief stop service adaptor
 *
 * @return      void.
 */
/*
static service_adaptor_error_e _sal_stop(sal_h sal)
{
	SAL_FN_CALL;

	int ret = 0;
	RETV_IF(NULL == sal, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETVM_IF(0 == sal->start, SERVICE_ADAPTOR_ERROR_INTERNAL, "could not start service adaptor");

	ret += auth_adaptor_stop(sal->auth);
	ret += contact_adaptor_stop(sal->contact);
	ret += storage_adaptor_stop(sal->storage);
	ret += resource_adaptor_stop(sal->resource);
	RETVM_IF(SERVICE_ADAPTOR_ERROR_NONE != ret, SERVICE_ADAPTOR_ERROR_INTERNAL, "could not stop service adaptor(%d)", ret);

	sal->start = 0;

	return ret;
}
*/
/**
 * @brief create service adaptor
 *
 * @return      void.
 */
/*
static sal_h _sal_create()
{
	SAL_FN_CALL;

	sal_h sal = NULL;

	// 1) create auth adaptor handle
	auth_adaptor_h auth = auth_adaptor_create();
	TRYM_IF(NULL == auth, "sal_auth_create_handle() Fail");

	// 2) create contact adaptor handle
	contact_adaptor_h contact = contact_adaptor_create();
	TRYM_IF(NULL == contact, "sal_contact_create_handle() Fail");

	// 3) create storage adaptor handle
	storage_adaptor_h storage = storage_adaptor_create();
	TRYM_IF(NULL == storage, "sal_storage_create_handle() Fail");

	// 4) create resource adaptor handle
	resource_adaptor_h resource = resource_adaptor_create();
	TRYM_IF(NULL == resource, "sal_resource_create_handle() Fail");

	// 5) register auth adaptor listener
	auth_adaptor_listener_h auth_listener = sal_auth_register_listener(auth);
	TRYM_IF(NULL == auth_listener, "sal_auth_register_listener() Fail");

	// 6) register contact adaptor listener
	contact_adaptor_listener_h contact_listener = sal_contact_register_listener(contact);
	TRYM_IF(NULL == contact_listener, "sal_contact_register_listener() Fail");

	// 7) register storage adaptor listener
	storage_adaptor_listener_h storage_listener = sal_storage_register_listener(storage);
	TRYM_IF(NULL == storage_listener, "sal_storage_register_listener() Fail");

	// 8) register resource adaptor listener
	resource_adaptor_listener_h resource_listener = sal_resource_register_listener(resource);
	TRYM_IF(NULL == resource_listener, "sal_resource_register_listener() Fail");

	// 9) create service adaptor
	sal = (sal_h) g_malloc0(sizeof(sal_s));
	TRYM_IF(NULL == sal, "could not create service adaptor");

	sal->auth = auth;
	sal->contact = contact;
	sal->storage = storage;
	sal->resource = resource;

	g_mutex_init(&sal->mutex);
	g_cond_init(&sal->cond);

catch:
	// TODO: free

	return sal;
}
*/

/**
 * @brief destroy service adaptor
 *
 * @return      void.
 */
/*
static void _sal_destroy(sal_h sal)
{
	SAL_FN_CALL;

	RET_IF(NULL == sal);

	// 1) destroy service list
	if (NULL != sal->svc_list) {
		g_list_free(sal->svc_list);
		sal->svc_list = NULL;
	}

	// 2) free service adaptor handle
	SAL_FREE(sal);

	SAL_FN_END;
}
*/

static int __thread_engine_init()
{
	SAL_FN_CALL;
	int ret = 0;
	ret = sal_engine_init(SAL_ENGINE_MODE_DEFAULT);
	RETVM_IF(SAL_ERROR_NONE != ret, ret, "Working thread init failed : %d", ret);

	for (sal_engine_e en = (SAL_ENGINE_MAIN + 1); en < SAL_ENGINE_MAX; en++) {
		ret = sal_engine_run(en);
		TRYM_IF(ret, "Working therad(%d) running failed : %d", (int)en, ret);
	}

	SAL_FN_END;
	return ret;

catch:
	for (sal_engine_e en = (SAL_ENGINE_MAIN + 1); en < SAL_ENGINE_MAX; en++) {
		ret = sal_engine_quit(en);
	}
	sal_engine_deinit();

	SAL_FN_END;
	return SAL_ERROR_INTERNAL;
}

static int __thread_engine_deinit()
{
	SAL_FN_CALL;
	int ret = 0;

	for (sal_engine_e en = (SAL_ENGINE_MAIN + 1); en < SAL_ENGINE_MAX; en++) {
		ret = sal_engine_quit(en);
	}
	sal_engine_deinit();

	SAL_FN_END;
	return SAL_ERROR_INTERNAL;
}

static void __temp_chunk_cb(ipc_server_session_h session)
{}

static void __ipc_server_open(user_data_t data)
{
	SAL_FN_CALL;
	GMainLoop *loop = sal_get_engine_loop(SAL_ENGINE_SERVICE);
	if (!loop)
		return;

	GMainContext *service_engine_context = g_main_loop_get_context(loop);

	static ipc_server_base_req_s base_request = {client_connect_cb,
			client_disconnect_cb,
			client_plugin_start_cb,
			client_plugin_stop_cb};

	static ipc_server_auth_req_s auth_request = {__temp_chunk_cb};

	static ipc_server_storage_req_s storage_request = {__temp_chunk_cb};

	int ret = sal_ipc_server_init(service_engine_context, &base_request, &auth_request, &storage_request);
	if (ret) {
		SAL_ERR("ipc server init error : %d", ret);
	}

	SAL_FN_END;
}

static void __ipc_server_close(user_data_t data)
{
	SAL_FN_CALL;

	int ret = sal_ipc_server_deinit();
	if (ret) {
		SAL_ERR("ipc server deinit error : %d", ret);
	}

	SAL_FN_END;
}

static int __ipc_server_init(void)
{
	SAL_FN_CALL;
	int ret = SAL_ERROR_NONE;

	ret = sal_engine_task_handoff(SAL_ENGINE_IPC_SERVER, __ipc_server_open, NULL);

	SAL_FN_END;
	return ret;
}

static int __ipc_server_deinit(void)
{
	SAL_FN_CALL;
	int ret = SAL_ERROR_NONE;

	ret = sal_engine_task_handoff(SAL_ENGINE_IPC_SERVER, __ipc_server_close, NULL);

	SAL_FN_END;
	return ret;
}

/**
 * @brief oiit service adaptor
 *
 * @return      void.
 */
static sal_error_e __sal_init()
{
	SAL_FN_CALL;
	int ret = SAL_ERROR_NONE;

	g_log_set_handler("GLib", G_LOG_LEVEL_CRITICAL, __glog_handler_cb, NULL);

	/* 1) create engine (based g_main_loop) */
	ret = __thread_engine_init();
	SAL_INFO("Initialize 1. Thread engine : %d", ret);
	if (ret) {
		return SAL_ERROR_INTERNAL;
	}
	sleep(1); // TODO fix sync with thread init

	ret = __ipc_server_init();
	SAL_INFO("Initialize 2. IPC Server : %d", ret);
	if (ret) {
		ret = __thread_engine_deinit();
		return SAL_ERROR_INTERNAL;
	}
	/* 1) create adaptor (memory allocation) */
/*
	sal_h sal = _sal_create();
	RETVM_IF(NULL == sal, SERVICE_ADAPTOR_ERROR_INTERNAL, "_sal_create() Fail");
*/
	/* 2) start adaptor (plugin loading) */
/*
	ret = _sal_start(sal);
	RETVM_IF(SERVICE_ADAPTOR_ERROR_NONE != ret, SERVICE_ADAPTOR_ERROR_INTERNAL, "_sal_start() Fail(%d)", ret);
*/
	/* 3) start adaptor (spec file creation) */
/*
	ret = _sal_create_spec_file(sal);
	RETVM_IF(SERVICE_ADAPTOR_ERROR_NONE != ret, SERVICE_ADAPTOR_ERROR_INTERNAL, "_sal_create_spec_file() Fail(%d)", ret);
*/
	/* 4) init dbus server */
/*
	ret = sal_ipc_server_init();
	RETVM_IF(SERVICE_ADAPTOR_ERROR_NONE != ret, SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE, "sal_ipc_server_init() Fail(%d)", ret);
*/
	/* 5) assign to global service adaptor handle */
/*
	g_service_adaptor = sal;
*/
	/* 6) register callback for package event */
/*
	ret = sal_observer_start();
	RETVM_IF(SERVICE_ADAPTOR_ERROR_NONE != ret, ret, "sal_observer_start() Fail(%d)", ret);
*/
	/* 7) create service discovery */
/*
	ret = service_discovery_create();
	RETVM_IF(SERVICE_ADAPTOR_ERROR_NONE != ret, ret, "service_discovery_create() Fail(%d)", ret);
*/
	/* 8) create service federation */
/*
	ret = service_federation_create();
	RETVM_IF(SERVICE_ADAPTOR_ERROR_NONE != ret, ret, "service_federation_create() Fail(%d)", ret);
*/
	return ret;
}

/**
 * @brief deinit service adaptor
 *
 * @param[in]   service_adaptor         specifies handle of service adaptor
 * @return      void.
 */
static void __sal_deinit()
{
	SAL_FN_CALL;

	//RET_IF(NULL == sal);

	int ret = __thread_engine_deinit();

/*
	sal_ipc_server_deinit();

	_sal_destroy_spec_file(sal);

	if (0 < sal->start) {
		_sal_stop(sal);
	}

	_sal_destroy(sal);
*/
	SAL_FN_END;
}

/**
 * @brief main signal function
 *
 * @param[in]   data            specifies user data passed by main function
 * @return      void.
 */
static gint _sigterm_callback(void *data)
{
	sal_engine_main_quit();

	return 0;
}

/**
 * @brief main function
 *
 * @param[in]   argc            specifies count of arguments
 * @param[in]   argv            specifies value list of arguments
 * @return      void.
 */
int main(int argc, char *argv[])
{
	int ret = SAL_ERROR_NONE;
	GMainLoop *loop = NULL;

#if !GLIB_CHECK_VERSION(2, 32, 0)
	g_thread_init(NULL);
#endif
#if !GLIB_CHECK_VERSION(2, 35, 0)
	g_type_init();
#endif

	ret = __sal_init();
//	RETVM_IF(SERVICE_ADAPTOR_ERROR_NONE != ret, 0, "_sal_init() Fail(%d)", ret);

	/* mainloop of main thread */
//	loop = g_main_loop_new(NULL, FALSE);

	/* installing signal handlers */
	g_unix_signal_add_full(G_PRIORITY_HIGH, SIGINT,
			_sigterm_callback, loop, NULL);
	g_unix_signal_add_full(G_PRIORITY_HIGH, SIGTERM,
			_sigterm_callback, loop, NULL);

	/* start application's main loop */
	sal_engine_main_run();

//	sal_h sal = sal_get_handle();
	__sal_deinit();

	return 0;
}

/******************************************************************************
 * Public interface definition
 ******************************************************************************/
/*
API sal_h sal_get_handle()
{
	SAL_FN_CALL;

	return g_service_adaptor;
}

API char *sal_get_root_path()
{
	SAL_FN_CALL;

	char *root_path = NULL;
	char tnfs_root_path[FILE_PATH_LEN] = {0,};

	// TODO: tmp -> need to use get_path APIs of TNFS
	snprintf(tnfs_root_path, FILE_PATH_LEN, "%s", "/opt/storage/tnfs");
	root_path = strdup(tnfs_root_path);

	return root_path;
}

API service_adaptor_error_e sal_adaptor_connect(const char *uri)
{
	SAL_FN_CALL;

	RETV_IF(NULL == uri, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	SAL_INFO("uri: %s", uri);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API service_adaptor_error_e sal_adaptor_disconnect(const char *uri)
{
	SAL_FN_CALL;

	RETV_IF(NULL == uri, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	SAL_INFO("uri: %s", uri);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API service_adaptor_error_e sal_adaptor_get_plugins(char ***plugins, int *plugins_size)
{
	SAL_FN_CALL;

	RETV_IF(NULL == g_service_adaptor, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == g_service_adaptor->auth, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	int size = g_list_length(g_service_adaptor->auth->plugins);
	RETV_IF(0 == size, SERVICE_ADAPTOR_ERROR_NO_DATA);

	int i = 0;
	char **uri = (char **) g_malloc0(sizeof(char *) * size);

	for (GList *list = g_list_first(g_service_adaptor->auth->plugins); list != NULL; list = list->next) {
		auth_plugin_h this = (auth_plugin_h) list->data;

		uri[i] = strdup(this->uri);
	}

	*plugins = uri;
	*plugins_size = size;

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API service_adaptor_error_e sal_provider_connect(const char *uri, const char *name, const char *package)
{
	SAL_FN_CALL;

	RETV_IF(NULL == uri, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	app_control_h request;
	app_control_create(&request);

	app_control_set_app_id(request, uri);
	app_control_set_operation(request, PLUGIN_CONNECT_URI);

	provider_user_data_h provider_user_data = (provider_user_data_h) g_malloc0(sizeof(provider_user_data_s));
	provider_user_data->uri = strdup(uri);
	provider_user_data->name = strdup(name);
	provider_user_data->package = strdup(package);

	int ret = app_control_send_launch_request(request, _provider_connect_cb, provider_user_data);

	RETVM_IF(APP_CONTROL_ERROR_NONE != ret, SERVICE_ADAPTOR_ERROR_SYSTEM, "app_control_send_launch_request() Fail(%d)", ret);

	app_control_destroy(request);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API service_adaptor_error_e sal_provider_disconnect(const char *uri)
{
	SAL_FN_CALL;

	RETV_IF(NULL == uri, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	app_control_h request;
	app_control_create(&request);

	app_control_set_app_id(request, uri);
	app_control_set_operation(request, PLUGIN_DISCONNECT_URI);

	int ret = app_control_send_launch_request(request, _provider_disconnect_cb, (void *) uri);

	RETVM_IF(APP_CONTROL_ERROR_NONE != ret, SERVICE_ADAPTOR_ERROR_SYSTEM, "app_control_send_launch_request() Fail(%d)", ret);

	app_control_destroy(request);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API char *sal_provider_get_uri(const char *package)
{
	SAL_FN_CALL;

	RETV_IF(NULL == package, NULL);

	sal_h sal = sal_get_handle();
	RETVM_IF(NULL == sal, NULL, "sal_get_handle() Fail");

	char *uri = auth_adaptor_get_uri(sal->auth, package);

	return uri;
}
*/
