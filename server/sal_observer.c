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
#include <glib.h>

#include <tizen.h>
#include <app_info.h>
#include <package_manager.h>

#include "sal.h"
#include "sal_observer.h"

//******************************************************************************
//* Global variables and defines
//******************************************************************************
#define SAL_PLUGIN_METADATA_KEY_ADAPTOR	"service-adaptor"

//******************************************************************************
//* Private interface
//******************************************************************************

//******************************************************************************
//* Private interface definition
//******************************************************************************

/**
 * @brief callback of app info
 *
 * @return      void.
 */
static bool _sal_app_meta_cb(const char *key, const char *value, void *user_data)
{
	SAL_FN_CALL;

	char **app_meta = (char **) user_data;

	if ((NULL != key) && (NULL != value))
	{
		if (0 == strncmp(SAL_PLUGIN_METADATA_KEY_ADAPTOR, key, strlen(SAL_PLUGIN_METADATA_KEY_ADAPTOR)))
		{
			app_meta[0] = strdup(value);
			SAL_INFO("SAL_PLUGIN_METADATA_KEY_ADAPTOR: %s", value);
		}
	}

	return true;
}

/**
 * @brief callback of app info
 *
 * @return      void.
 */
static bool _sal_app_info_cb(package_info_app_component_type_e comp_type, const char *app_id, void *user_data)
{
	SAL_FN_CALL;

	app_info_h app_info = NULL;
	int ret = PACKAGE_MANAGER_ERROR_NONE;
	char *pkg_path = (char *) user_data;
	char **app_meta = (char **) g_malloc0(sizeof(char *) * 1);

	(void) pkg_path;

	app_info_create(app_id, &app_info);
	ret = app_info_foreach_metadata(app_info, _sal_app_meta_cb, (void *) app_meta);
	RETVM_IF(PACKAGE_MANAGER_ERROR_NONE != ret, false, "app_info_foreach_metadata() Fail(%d)", ret);

	app_info_destroy(app_info);

	// TODO: using app_meta

	SAL_FREE(app_meta);

	return true;
}

/**
 * @brief callback of package manager
 *
 * @return      void.
 */
static void _sal_package_event_cb(const char *type,
                const char *package,
                package_manager_event_type_e event_type,
                package_manager_event_state_e event_state,
                int progress,
                package_manager_error_e error,
                void *user_data)
{
        SAL_FN_CALL;

        if ((PACKAGE_MANAGER_EVENT_TYPE_INSTALL == event_type)
                        && (PACKAGE_MANAGER_EVENT_STATE_COMPLETED == event_state))
        {
                int ret = PACKAGE_MANAGER_ERROR_NONE;
                package_info_h p_info = NULL;

                ret = package_manager_get_package_info(package, &p_info);
                RETM_IF(PACKAGE_MANAGER_ERROR_NONE != ret, "package_manager_get_package_info() Fail(%d)", ret);

                char *path = NULL;
                ret = package_info_get_root_path(p_info, &path);
                RETM_IF(PACKAGE_MANAGER_ERROR_NONE != ret, "package_info_get_root_path() Fail(%d)", ret);

                ret = package_info_foreach_app_from_package(p_info, PACKAGE_INFO_ALLAPP, _sal_app_info_cb, path);
                RETM_IF(PACKAGE_MANAGER_ERROR_NONE != ret, "package_info_foreach_app_from_package() Fail(%d)", ret);

                ret = package_info_destroy(p_info);
                RETM_IF(PACKAGE_MANAGER_ERROR_NONE != ret, "package_info_destroy() Fail(%d)", ret);

                SAL_FREE(path);
        }

        SAL_FN_END;
}

//******************************************************************************
//* Public interface definition
//******************************************************************************

/**
 * @brief start observer using package manager
 *
 * @return      void.
 */
service_adaptor_error_e sal_observer_start()
{
	int ret = 0;
	package_manager_h package = NULL;

	package_manager_create(&package);

	ret = package_manager_set_event_cb(package, _sal_package_event_cb, NULL);
	RETVM_IF(PACKAGE_MANAGER_ERROR_NONE != ret, SERVICE_ADAPTOR_ERROR_SYSTEM, "package_manager_set_event_cb() Fail(%d)", ret);

	return SERVICE_ADAPTOR_ERROR_NONE;
}
