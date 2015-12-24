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
#include <string.h>
#include <glib.h>
/*
#include <tizen.h>
#include <app.h>
#include <app_info.h>
#include <package_manager.h>

#include "service_adaptor_internal.h"

#include "sal.h"
#include "sal_observer.h"
#include "auth_adaptor.h"
*/
/******************************************************************************
 * Global variables and defines
 ******************************************************************************/
#define SAL_PLUGIN_METADATA_KEY_ADAPTOR	"service-adaptor"

/******************************************************************************
 * Private interface
 ******************************************************************************/

/******************************************************************************
 * Private interface definition
 ******************************************************************************/

/**
 * @brief callback of app info
 *
 * @return      void.
 */
/*
static bool _sal_app_meta_cb(const char *key, const char *value, void *user_data)
{
	SAL_FN_CALL;

	provider_user_data_h provider_user_data = (provider_user_data_h) user_data;

	if ((NULL != key) && (NULL != value)) {
		if (0 == strncmp(SAL_PLUGIN_METADATA_KEY_ADAPTOR, key, strlen(SAL_PLUGIN_METADATA_KEY_ADAPTOR))) {
			SAL_INFO("%s: %s", provider_user_data->uri, value);
			int ret = sal_provider_connect(provider_user_data->uri, value, provider_user_data->package);

			RETVM_IF(SERVICE_ADAPTOR_ERROR_NONE != ret, false, "sal_plugin_connect() Fail(%d)", ret);
		}
	}

	return true;
}
*/

/**
 * @brief callback of app info
 *
 * @return      void.
 */
/*
static bool _sal_app_info_cb(package_info_app_component_type_e comp_type, const char *app_id, void *user_data)
{
	SAL_FN_CALL;

	app_info_h app_info = NULL;
	int ret = PACKAGE_MANAGER_ERROR_NONE;
	char *package = (char *) user_data;

	provider_user_data_h provider_user_data = (provider_user_data_h) g_malloc0(sizeof(provider_user_data_s));
	provider_user_data->uri = strdup(app_id);
	provider_user_data->package = strdup(package);

	app_info_create(app_id, &app_info);
	ret = app_info_foreach_metadata(app_info, _sal_app_meta_cb, provider_user_data);
	RETVM_IF(PACKAGE_MANAGER_ERROR_NONE != ret, false, "app_info_foreach_metadata() Fail(%d)", ret);

	app_info_destroy(app_info);

	return true;
}
*/

/**
 * @brief callback of package manager
 *
 * @return      void.
 */
/*
static void _sal_package_event_cb(const char *type,
		const char *package,
		package_manager_event_type_e event_type,
		package_manager_event_state_e event_state,
		int progress,
		package_manager_error_e error,
		void *user_data)
{
	SAL_FN_CALL;

	if ((PACKAGE_MANAGER_EVENT_TYPE_INSTALL == event_type) && (PACKAGE_MANAGER_EVENT_STATE_COMPLETED == event_state)) {
		int ret = PACKAGE_MANAGER_ERROR_NONE;
		package_info_h p_info = NULL;

		ret = package_manager_get_package_info(package, &p_info);
		RETM_IF(PACKAGE_MANAGER_ERROR_NONE != ret, "package_manager_get_package_info() Fail(%d)", ret);

		ret = package_info_foreach_app_from_package(p_info, PACKAGE_INFO_ALLAPP, _sal_app_info_cb, (void *) package);
		RETM_IF(PACKAGE_MANAGER_ERROR_NONE != ret, "package_info_foreach_app_from_package() Fail(%d)", ret);

		ret = package_info_destroy(p_info);
		RETM_IF(PACKAGE_MANAGER_ERROR_NONE != ret, "package_info_destroy() Fail(%d)", ret);
	} else if ((PACKAGE_MANAGER_EVENT_TYPE_UNINSTALL == event_type) && (PACKAGE_MANAGER_EVENT_STATE_COMPLETED == event_state)) {
		sal_h sal = sal_get_handle();
		RETM_IF(NULL == sal, "sal_get_handle() Fail");

		char *uri = sal_provider_get_uri(package);

		auth_plugin_h auth_plugin = auth_adaptor_get_plugin(sal->auth, uri);
		auth_adaptor_remove_plugin(sal->auth, auth_plugin);
		auth_adaptor_unregister_plugin_service(auth_plugin);
		auth_adaptor_destroy_plugin(auth_plugin);

		storage_plugin_h storage_plugin = storage_adaptor_get_plugin(sal->storage, uri);
		storage_adaptor_remove_plugin(sal->storage, storage_plugin);
		storage_adaptor_unregister_plugin_service(storage_plugin);
		storage_adaptor_destroy_plugin(storage_plugin);

		// TODO: destroy plugin of other adaptor
	}
}
*/
/**
 * @brief callback of package_manager_foreach_package_info()
 *
 * @return      void.
 */
/*
static bool _sal_package_info_cb(package_info_h package_info, void *user_data)
{
	SAL_FN_CALL;

	int ret = PACKAGE_MANAGER_ERROR_NONE;

	char *package = NULL;
	ret = package_info_get_package(package_info, &package);
	RETVM_IF(PACKAGE_MANAGER_ERROR_NONE != ret, false, "package_info_get_package() Fail(%d)", ret);

	ret = package_info_foreach_app_from_package(package_info, PACKAGE_INFO_ALLAPP, _sal_app_info_cb, package);
	RETVM_IF(PACKAGE_MANAGER_ERROR_NONE != ret, false, "package_info_foreach_app_from_package() Fail(%d)", ret);

	return true;
}
*/

/******************************************************************************
 * Public interface definition
 ******************************************************************************/

/**
 * @brief start observer using package manager
 *
 * @return      void.
 */
/*
service_adaptor_error_e sal_observer_start()
{
	SAL_FN_CALL;

	int ret = 0;
	package_manager_h package = NULL;

	package_manager_create(&package);

	ret = package_manager_set_event_cb(package, _sal_package_event_cb, NULL);
	RETVM_IF(PACKAGE_MANAGER_ERROR_NONE != ret, SERVICE_ADAPTOR_ERROR_SYSTEM, "package_manager_set_event_cb() Fail(%d)", ret);

	return SERVICE_ADAPTOR_ERROR_NONE;
}
*/

/**
 * @brief register existing plugin using package manager
 *
 * @return      void.
 */
/*
service_adaptor_error_e sal_observer_register_existed_plugin()
{
	SAL_FN_CALL;

	int ret = PACKAGE_MANAGER_ERROR_NONE;

	ret = package_manager_foreach_package_info(_sal_package_info_cb, NULL);
	RETVM_IF(PACKAGE_MANAGER_ERROR_NONE != ret, SERVICE_ADAPTOR_ERROR_SYSTEM, "package_manager_foreach_package_info() Fail(%d)", ret);

	return SERVICE_ADAPTOR_ERROR_NONE;
}
*/
