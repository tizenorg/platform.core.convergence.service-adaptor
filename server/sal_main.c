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

#include "service_adaptor.h"
#include "service_discovery.h"
#include "service_federation.h"
#include "sal.h"
#include "sal_ipc.h"
#include "sal_auth.h"
#include "sal_contact.h"
#include "sal_storage.h"
#include "sal_observer.h"

//******************************************************************************
//* Global variables and defines
//******************************************************************************
#define SAL_PLUGIN_METADATA_KEY_ADAPTOR	"service-adaptor"

static service_adaptor_h g_service_adaptor = NULL;

//******************************************************************************
//* Private interface
//******************************************************************************

//******************************************************************************
//* Private interface definition
//******************************************************************************

/**
 * @brief create spec file
 *
 * @return      void.
 */
static service_adaptor_error_e _sal_create_spec_file(service_adaptor_h sal)
{
	SAL_FN_CALL;

	int ret = 0;
	RETV_IF(NULL == sal, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	return ret;
}

/**
 * @brief destroy spec file
 *
 * @return      void.
 */
static service_adaptor_error_e _sal_destroy_spec_file(service_adaptor_h sal)
{
	SAL_FN_CALL;

	int ret = 0;
	RETV_IF(NULL == sal, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

	return ret;
}

/**
 * @brief start service adaptor
 *
 * @return      void.
 */
static service_adaptor_error_e _sal_start(service_adaptor_h sal)
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

	g_mutex_lock(&sal->mutex);
	sal->start = sal->start + 1;
	g_cond_signal(&sal->cond);
	g_mutex_unlock(&sal->mutex);

	return ret;
}

/**
 * @brief stop service adaptor
 *
 * @return      void.
 */
static service_adaptor_error_e _sal_stop(service_adaptor_h sal)
{
	SAL_FN_CALL;

	int ret = 0;
	RETV_IF(NULL == sal, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);
	RETVM_IF(0 == sal->start, SERVICE_ADAPTOR_ERROR_INTERNAL, "could not start service adaptor");

	ret += auth_adaptor_stop(sal->auth);
	ret += contact_adaptor_stop(sal->contact);
	ret += storage_adaptor_stop(sal->storage);
	RETVM_IF(SERVICE_ADAPTOR_ERROR_NONE != ret, SERVICE_ADAPTOR_ERROR_INTERNAL, "could not stop service adaptor(%d)", ret);

	sal->start = 0;

	return ret;
}

/**
 * @brief create service adaptor
 *
 * @return      void.
 */
static service_adaptor_h _sal_create()
{
	SAL_FN_CALL;

	service_adaptor_h sal = NULL;

	// 1) create auth adaptor handle
	auth_adaptor_h auth = sal_auth_create_handle();
	TRYM_IF(NULL == auth, "sal_auth_create_handle() Fail");

	// 2) create contact adaptor handle
	contact_adaptor_h contact = sal_contact_create_handle();
	TRYM_IF(NULL == contact, "sal_contact_create_handle() Fail");

	// 3) create storage adaptor handle
	storage_adaptor_h storage = sal_storage_create_handle();
	TRYM_IF(NULL == storage, "sal_storage_create_handle() Fail");

	// 4) register auth adaptor listener
	auth_adaptor_listener_h auth_listener = sal_auth_register_listener(auth);
	TRYM_IF(NULL == auth_listener, "sal_auth_register_listener() Fail");

	// 5) register contact adaptor listener
	contact_adaptor_listener_h contact_listener = sal_contact_register_listener(contact);
	TRYM_IF(NULL == contact_listener, "sal_contact_register_listener() Fail");

	// 6) register storage adaptor listener
	storage_adaptor_listener_h storage_listener = sal_storage_register_listener(storage);
	TRYM_IF(NULL == storage_listener, "sal_storage_register_listener() Fail");

	// 7) create service adaptor
	sal = (service_adaptor_h) g_malloc0(sizeof(service_adaptor_s));
	TRYM_IF(NULL == sal, "could not create service adaptor");

	sal->auth = auth;
	sal->contact = contact;
	sal->storage = storage;

	g_mutex_init(&sal->mutex);
	g_cond_init(&sal->cond);

catch:
	// free

	return sal;
}

/**
 * @brief destroy service adaptor
 *
 * @return      void.
 */
static void _sal_destroy(service_adaptor_h sal)
{
	SAL_FN_CALL;

	RET_IF(NULL == sal);

        // 1) destroy service list
        if (NULL != sal->svc_list)
        {
                g_list_free(sal->svc_list);
                sal->svc_list = NULL;
        }

	// 2) free service adaptor handle
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

	// 1) create adaptor (memory allocation)
	service_adaptor_h sal = _sal_create();
	RETVM_IF(NULL == sal, SERVICE_ADAPTOR_ERROR_INTERNAL, "_sal_create() Fail");

	// 2) start adaptor (plugin loading)
	ret = _sal_start(sal);
	RETVM_IF(SERVICE_ADAPTOR_ERROR_NONE != ret, SERVICE_ADAPTOR_ERROR_INTERNAL, "_sal_start() Fail(%d)", ret);

	// 3) start adaptor (spec file creation)
	ret = _sal_create_spec_file(sal);
	RETVM_IF(SERVICE_ADAPTOR_ERROR_NONE != ret, SERVICE_ADAPTOR_ERROR_INTERNAL, "_sal_create_spec_file() Fail(%d)", ret);

	// 4) init dbus
	ret = sal_ipc_init_server();
	RETVM_IF(SERVICE_ADAPTOR_ERROR_NONE != ret, SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE, "sal_ipc_init_server() Fail(%d)", ret);

	// 5) assign to global service adaptor handle
	g_service_adaptor = sal;

	// 6) register callback for package event
	ret = sal_observer_start();
	RETVM_IF(SERVICE_ADAPTOR_ERROR_NONE != ret, ret, "sal_observer_start() Fail(%d)", ret);

	// 7) create service discovery
	ret = service_discovery_create();
	RETVM_IF(SERVICE_ADAPTOR_ERROR_NONE != ret, ret, "service_discovery_create() Fail(%d)", ret);

	// 8) create service federation
	ret = service_federation_create();
	RETVM_IF(SERVICE_ADAPTOR_ERROR_NONE != ret, ret, "service_federation_create() Fail(%d)", ret);

	return ret;
}

/**
 * @brief deinit service adaptor
 *
 * @param[in]   service_adaptor         specifies handle of service adaptor
 * @return      void.
 */
static void _sal_deinit(service_adaptor_h sal)
{
	SAL_FN_CALL;

	RET_IF(NULL == sal);

	sal_ipc_deinit_server();

	_sal_destroy_spec_file(sal);

        if (0 < sal->start)
        {
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

#if !GLIB_CHECK_VERSION(2,32,0)
	g_thread_init(NULL);
#endif
#if !GLIB_CHECK_VERSION(2,35,0)
	g_type_init();
#endif

	ret = _sal_init();
	RETVM_IF(SERVICE_ADAPTOR_ERROR_NONE != ret, 0, "_sal_init() Fail(%d)", ret);

	// mainloop of main thread
	loop = g_main_loop_new(NULL, FALSE);

	// installing signal handlers
	g_unix_signal_add_full(G_PRIORITY_HIGH, SIGINT,
			_sigterm_callback, loop, NULL );
	g_unix_signal_add_full(G_PRIORITY_HIGH, SIGTERM,
			_sigterm_callback, loop, NULL );

	// start application's main loop
	g_main_loop_run(loop);

	// cleanup after mainloop
	g_main_loop_unref(loop);

	service_adaptor_h sal = sal_get_handle();
	_sal_deinit(sal);

	return 0;
}

//******************************************************************************
//* Public interface definition
//******************************************************************************

service_adaptor_h sal_get_handle()
{
	SAL_FN_CALL;

        return g_service_adaptor;
}

char *sal_get_root_path()
{
	SAL_FN_CALL;

        char *root_path = NULL;
        char tnfs_root_path[FILE_PATH_LEN] = {0,};

        // TODO: tmp -> need to use get_path APIs of TNFS
        snprintf(tnfs_root_path, FILE_PATH_LEN, "%s", "/opt/storage/tnfs");
        root_path = strdup(tnfs_root_path);

        return root_path;
}

service_adaptor_error_e sal_connect(service_adaptor_h sal)
{
	SAL_FN_CALL;

	RETV_IF(NULL == sal, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

        return SERVICE_ADAPTOR_ERROR_NONE;
}

service_adaptor_error_e sal_disconnect(service_adaptor_h sal)
{
	SAL_FN_CALL;

	RETV_IF(NULL == sal, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);

        return SERVICE_ADAPTOR_ERROR_NONE;
}
