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
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <glib-object.h>
#include <glib-unix.h>

#include <app_info.h>
#include <package_manager.h>
#include <auth-adaptor.h>
#include <storage-adaptor.h>

#include "service-adaptor.h"
#include "service-adaptor-plugin.h"
#include "service-adaptor-auth.h"
#include "service-adaptor-storage.h"
#include "service-adaptor-type.h"
#include "service-adaptor-log.h"

void blank(char *s, ...) {}
void _blank() {}

#ifdef DEBUG_PLUGIN_OBSERVER_FLAG
#define _service_adaptor_debug		service_adaptor_debug
#define _service_adaptor_debug_func	service_adaptor_debug_func
#define _service_adaptor_info		service_adaptor_info
#define _FUNC_START			FUNC_START
#define _FUNC_END			FUNC_END
#else
#define _service_adaptor_debug		blank
#define _service_adaptor_debug_func	blank
#define _service_adaptor_info		blank
#define _FUNC_START			_blank
#define _FUNC_END			_blank
#endif

static package_manager_h g_package_manager = NULL;

struct package_checker_context_s {
	char *package_id;
	char *package_root;
	char *auth_plugin_path;
	char *storage_plugin_path;

	bool version_defined;
	bool auth_defined;
	bool storage_defined;
};

bool __service_adaptor_app_meta_iterator_cb(const char *_key,
						const char *value,
						void *user_data)
{
	_FUNC_START();
	char *key = strdup(_key);
	struct package_checker_context_s *ctx = (struct package_checker_context_s *) user_data;
	_service_adaptor_debug_func("app meta key(%s) value(%s)", key, value);
	if (NULL != key) {
		if (0 == strncmp(SERVICE_ADAPTOR_3RD_PARTY_METADATA_KEY_VERSION, key,
				strlen(SERVICE_ADAPTOR_3RD_PARTY_METADATA_KEY_VERSION))) {
			service_adaptor_info("Discovered 3rd party Plugin with Service adaptor");
			service_adaptor_info("Package id : %s", ctx->package_id);
			service_adaptor_info("Compatible Version : over %s", value);
			ctx->version_defined = true;
		} else if (0 == strncmp(SERVICE_ADAPTOR_3RD_PARTY_METADATA_KEY_AUTH, key, strlen(SERVICE_ADAPTOR_3RD_PARTY_METADATA_KEY_AUTH))) {
			if (NULL != value) {
				/*free(ctx->auth_plugin_path); */
				ctx->auth_plugin_path = strdup(value);
				ctx->auth_defined = true;
			}
			_service_adaptor_debug_func("auth plugin path : %s", value);
		} else if (0 == strncmp(SERVICE_ADAPTOR_3RD_PARTY_METADATA_KEY_STORAGE, key, strlen(SERVICE_ADAPTOR_3RD_PARTY_METADATA_KEY_STORAGE))) {
			if (NULL != value) {
				/*free(ctx->storage_plugin_path); */
				ctx->storage_plugin_path = strdup(value);
				ctx->storage_defined = true;
			}
			_service_adaptor_debug_func("storage plugin path : %s", value);
		}
	}
	free(key);
	_FUNC_END();
	if ((ctx->version_defined) && (ctx->auth_defined) && (ctx->storage_defined)) {
		_service_adaptor_debug("Stop iter (All fields are filled)");
		return false;
	}
	return true;

}

bool __service_adaptor_app_info_iterator_cb(package_info_app_component_type_e comp_type,
						const char *app_id,
						void *user_data)
{
	_FUNC_START();
	int ret = 0;

	_service_adaptor_debug("package app_info callback : type(%d), app_id(%s)\n",
			comp_type, app_id);
	app_info_h app_info;
	ret = app_info_create(app_id, &app_info);
	if (!ret) {
		ret = app_info_foreach_metadata(app_info,
				__service_adaptor_app_meta_iterator_cb, user_data);
		_service_adaptor_debug("app_meta foreach res : %d\n", ret);
		app_info_destroy(app_info);
	} else {
		_service_adaptor_debug("app_info creation failed (%d)", ret);
	}

	_FUNC_END();
	return true;
}

service_adaptor_internal_error_code_e
_service_adaptor_check_package(service_adaptor_h service_adaptor,
						const char *_package_id)
{
	_FUNC_START();
	int ret;
	package_info_h p_info;
	ret = package_manager_get_package_info(_package_id, &p_info);
	_service_adaptor_debug("package info get(%d)\n", ret);

	if (ret == 0) {
		char *path = NULL;
		ret = package_info_get_root_path(p_info, &path);
		_service_adaptor_debug("package root path(%d) : %s\n", ret, path);

		/* Init context */
		struct package_checker_context_s ctx;
		ctx.package_id = strdup(_package_id);
		ctx.package_root = path;
		ctx.auth_plugin_path = NULL;
		ctx.storage_plugin_path = NULL;

		ctx.version_defined = false;
		ctx.auth_defined = false;
		ctx.storage_defined = false;

		ret = package_info_foreach_app_from_package(p_info, PACKAGE_INFO_ALLAPP,
				__service_adaptor_app_info_iterator_cb, (void *)&ctx);
		_service_adaptor_debug("package app foreach (%d)\n", ret);
		ret = package_info_destroy(p_info);

		if (ctx.version_defined) {
			_service_adaptor_debug("Version is defined");
			if ((ctx.auth_defined) && (NULL != ctx.auth_plugin_path)) {
				auth_adaptor_h auth_adaptor = service_adaptor_get_auth_adaptor(service_adaptor);
				ret = auth_adaptor_load_plugin_from_package(auth_adaptor, _package_id, ctx.auth_plugin_path);
				_service_adaptor_info("auth plugin load ret (%d)", ret);
			}

			if ((ctx.storage_defined) && (NULL != ctx.storage_plugin_path)) {
				storage_adaptor_h storage_adaptor = service_adaptor_get_storage_adaptor(service_adaptor);
				ret = storage_adaptor_load_plugin_from_package(storage_adaptor, _package_id, ctx.storage_plugin_path);
				_service_adaptor_info("storage plugin load ret (%d)", ret);
			}
		}

		free(ctx.auth_plugin_path);
		free(ctx.storage_plugin_path);
	}
	_FUNC_END();
	return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
}


void service_adaptor_package_event_callback(const char *type,
						const char *package_id,
						package_manager_event_type_e event_type,
						package_manager_event_state_e event_state,
						int progress,
						package_manager_error_e error,
						void *user_data)
{
	_FUNC_START();
	_service_adaptor_debug("package callback\n");

	int ret = 0;
	/* TODO add handling when the package is updated */
	if ((PACKAGE_MANAGER_EVENT_TYPE_INSTALL == event_type) && (PACKAGE_MANAGER_EVENT_STATE_COMPLETED  == event_state)) {
		_service_adaptor_debug("New package installed <package type(%s) / name(%s)>",
				type, package_id);
		ret = _service_adaptor_check_package((service_adaptor_h)user_data, package_id);
		_service_adaptor_debug("Package check ret (%d)", ret);
	}
	_FUNC_END();
}

service_adaptor_internal_error_code_e
service_adaptor_set_package_installed_callback(service_adaptor_h _service_adaptor)
{
	_FUNC_START();
	int ret = 0;
	package_manager_h p_manager;
	package_manager_create(&p_manager);

	ret = package_manager_set_event_cb(p_manager,
			service_adaptor_package_event_callback, (void *)_service_adaptor);
	_service_adaptor_debug("package manager set event (%d)\n", ret);
	g_package_manager = p_manager;

	_FUNC_END();
	return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
}

service_adaptor_internal_error_code_e
service_adaptor_unset_package_installed_callback(service_adaptor_h _service_adaptor)
{
	_FUNC_START();
	package_manager_h p_manager = g_package_manager;
	if (NULL != p_manager) {
		package_manager_unset_event_cb(p_manager);
		package_manager_destroy(p_manager);

		g_package_manager = NULL;
		_service_adaptor_debug("package manager unset event");
	}

	_FUNC_END();
	return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
}


bool _service_adaptor_package_iterator_cb(package_info_h package_info,
						void *user_data)
{
	int ret = 0;
	char *package = NULL;
	ret = package_info_get_package(package_info, &package);
	_service_adaptor_debug("package info get package(%d) : name(%s)", ret, package);
	if ((0 == ret) && (NULL != package)) {
		ret = _service_adaptor_check_package((service_adaptor_h)user_data, package);
	}
	free(package);
	return true;
}

service_adaptor_internal_error_code_e
service_adaptor_scan_all_packages(service_adaptor_h _service_adaptor)
{
	int ret = 0;
	ret = package_manager_foreach_package_info(_service_adaptor_package_iterator_cb, (void *)_service_adaptor);
	_service_adaptor_debug("Scan all packages ret(%d)", ret);
	return 0;	/*TODO */
}


void *_scanner_runnable(void *data)
{
	int ret = package_manager_foreach_package_info(_service_adaptor_package_iterator_cb, data);
	_service_adaptor_debug("Scan all packages ret(%d)", ret);

	return NULL;
}

service_adaptor_internal_error_code_e
service_adaptor_scan_all_packages_async(service_adaptor_h _service_adaptor)
{
	pthread_t runnable;
	pthread_create(&runnable, NULL, _scanner_runnable, (void *)_service_adaptor);
	return 0;	/*TODO */
}

