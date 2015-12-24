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

/******************************************************************************
 * Global variables and defines
 ******************************************************************************/
#define FILE_PATH_LEN	256

static sal_h g_service_adaptor = NULL;

typedef enum
{
	SAL_ENGINE_MODE_DEFAULT	= 0,
	SAL_ENGINE_MODE_SLIM	= 1,
} sal_engine_mode_e;

static GMainLoop *ipc_server_engine = NULL;

static GMainLoop *ipc_adaptor_engine = NULL;

static GMainLoop *main_engine = NULL;

static GMainLoop *server_engine = NULL;

static GMainLoop *file_engine = NULL;

/******************************************************************************
 * Private interface
 ******************************************************************************/

/******************************************************************************
 * Private interface definition
 ******************************************************************************/

static int __sal_engine_run(sal_engine_mode_e mode)
{
	SAL_FN_CALL;
	int ret = SAL_ERROR_NONE;

	GMainContext *ipc_server_context = NULL;
	GMainContext *ipc_adaptor_context = NULL;
	GMainContext *main_context = NULL;
	GMainContext *server_context = NULL;
	GMainContext *file_context = NULL;

	ipc_server_context	= g_main_context_new();
	TRY_IF(NULL == ipc_server_context);
	ipc_server_engine = g_main_loop_new(ipc_server_context, FALSE);
	TRY_IF(NULL == ipc_server_engine);

	ipc_adaptor_context	= g_main_context_new();
	TRY_IF(NULL == ipc_adaptor_context);
	ipc_adaptor_engine = g_main_loop_new(ipc_adaptor_context, FALSE);
	TRY_IF(NULL == ipc_adaptor_engine);

	main_context		= g_main_context_new();
	TRY_IF(NULL == main_context);
	main_engine = g_main_loop_new(main_context, FALSE);
	TRY_IF(NULL == main_engine);

	server_context		= g_main_context_new();
	TRY_IF(NULL == server_context);
	g_main_loop_new(server_context, FALSE);
	TRY_IF(NULL == server_context);

	file_context		= g_main_context_new();
	TRY_IF(NULL == file_context);
	g_main_loop_new(file_context, FALSE);
	TRY_IF(NULL == file_engine);

	ipc_server_engine = NULL;
	ipc_adaptor_engine = NULL;
	main_engine = NULL;
	server_engine = NULL;
	file_engine = NULL;

	SAL_FN_END;
	return ret;

catch:

	if (SAL_ENGINE_MODE_DEFAULT == mode) {
		if (ipc_server_engine)
			g_main_loop_unref(ipc_server_engine);
		if (ipc_adaptor_engine)
			g_main_loop_unref(ipc_adaptor_engine);
	}

	if (main_engine)
		g_main_loop_unref(main_engine);
	if (server_engine)
		g_main_loop_unref(server_engine);
	if (file_engine)
		g_main_loop_unref(file_engine);

	if (SAL_ENGINE_MODE_DEFAULT == mode) {
		if (ipc_server_context)
			g_main_context_unref(ipc_server_context);
		if (ipc_adaptor_context)
			g_main_context_unref(ipc_adaptor_context);
	}

	if (main_context)
		g_main_context_unref(main_context);
	if (server_context)
		g_main_context_unref(server_context);
	if (file_context)
		g_main_context_unref(file_context);

	ipc_server_engine = NULL;
	ipc_adaptor_engine = NULL;
	main_engine = NULL;
	server_engine = NULL;
	file_engine = NULL;

	return ret;
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
static sal_h _sal_create()
{
	SAL_FN_CALL;

	sal_h sal = NULL;

	/* 1) create auth adaptor handle */
	auth_adaptor_h auth = auth_adaptor_create();
	TRYM_IF(NULL == auth, "sal_auth_create_handle() Fail");

	/* 2) create contact adaptor handle */
	contact_adaptor_h contact = contact_adaptor_create();
	TRYM_IF(NULL == contact, "sal_contact_create_handle() Fail");

	/* 3) create storage adaptor handle */
	storage_adaptor_h storage = storage_adaptor_create();
	TRYM_IF(NULL == storage, "sal_storage_create_handle() Fail");

	/* 4) create resource adaptor handle */
	resource_adaptor_h resource = resource_adaptor_create();
	TRYM_IF(NULL == resource, "sal_resource_create_handle() Fail");

	/* 5) register auth adaptor listener */
	auth_adaptor_listener_h auth_listener = sal_auth_register_listener(auth);
	TRYM_IF(NULL == auth_listener, "sal_auth_register_listener() Fail");

	/* 6) register contact adaptor listener */
	contact_adaptor_listener_h contact_listener = sal_contact_register_listener(contact);
	TRYM_IF(NULL == contact_listener, "sal_contact_register_listener() Fail");

	/* 7) register storage adaptor listener */
	storage_adaptor_listener_h storage_listener = sal_storage_register_listener(storage);
	TRYM_IF(NULL == storage_listener, "sal_storage_register_listener() Fail");

	/* 8) register resource adaptor listener */
	resource_adaptor_listener_h resource_listener = sal_resource_register_listener(resource);
	TRYM_IF(NULL == resource_listener, "sal_resource_register_listener() Fail");

	/* 9) create service adaptor */
	sal = (sal_h) g_malloc0(sizeof(sal_s));
	TRYM_IF(NULL == sal, "could not create service adaptor");

	sal->auth = auth;
	sal->contact = contact;
	sal->storage = storage;
	sal->resource = resource;

	g_mutex_init(&sal->mutex);
	g_cond_init(&sal->cond);

catch:
	/* TODO: free */

	return sal;
}

/**
 * @brief destroy service adaptor
 *
 * @return      void.
 */
static void _sal_destroy(sal_h sal)
{
	SAL_FN_CALL;

	RET_IF(NULL == sal);

	/* 1) destroy service list */
	if (NULL != sal->svc_list) {
		g_list_free(sal->svc_list);
		sal->svc_list = NULL;
	}

	/* 2) free service adaptor handle */
	SAL_FREE(sal);

	SAL_FN_END;
}

/**
 * @brief init service adaptor
 *
 * @return      void.
 */
static service_adaptor_error_e _sal_init()
{
	SAL_FN_CALL;
	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	/* 1) create engine (based g_main_loop) */
	sal_engine_run();

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
static void _sal_deinit(sal_h sal)
{
	SAL_FN_CALL;

	RET_IF(NULL == sal);

	sal_ipc_server_deinit();

	_sal_destroy_spec_file(sal);

	if (0 < sal->start) {
		_sal_stop(sal);
	}

	_sal_destroy(sal);

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
	g_main_loop_quit((GMainLoop*)data);

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
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GMainLoop *loop = NULL;

#if !GLIB_CHECK_VERSION(2, 32, 0)
	g_thread_init(NULL);
#endif
#if !GLIB_CHECK_VERSION(2, 35, 0)
	g_type_init();
#endif

	ret = _sal_init();
	RETVM_IF(SERVICE_ADAPTOR_ERROR_NONE != ret, 0, "_sal_init() Fail(%d)", ret);

	/* mainloop of main thread */
	loop = g_main_loop_new(NULL, FALSE);

	/* installing signal handlers */
	g_unix_signal_add_full(G_PRIORITY_HIGH, SIGINT,
			_sigterm_callback, loop, NULL);
	g_unix_signal_add_full(G_PRIORITY_HIGH, SIGTERM,
			_sigterm_callback, loop, NULL);

	/* start application's main loop */
	g_main_loop_run(loop);

	/* cleanup after mainloop */
	g_main_loop_unref(loop);

	sal_h sal = sal_get_handle();
	_sal_deinit(sal);

	return 0;
}

/******************************************************************************
 * Public interface definition
 ******************************************************************************/

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

	/* TODO: tmp -> need to use get_path APIs of TNFS */
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
