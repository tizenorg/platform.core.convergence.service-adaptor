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
#include <glib.h>

#include <pluginConfig.h>
#include <tzplatform_config.h>

#include "util/service_file_manager.h"
#include "service-adaptor-log.h"

/*************************************************
 *               Type definition
 *************************************************/

typedef struct _service_file_s {
	ConfigHandle handle;
} service_file_t;

#define SERVICE_FILE_PUSH_PATH		tzplatform_mkpath(TZ_SYS_SHARE, "/service-adaptor/.push/")

#define SERVICE_FILE_SECTION_STRING_GENERAL	"general"
#define SERVICE_FILE_SECTION_STRING_BUS		"bus"
#define SERVICE_FILE_SECTION_STRING_PUSH	"push"


/*************************************************
 *               Global valuable
 *************************************************/


/*************************************************
 *               Internal function prototype
 *************************************************/



/*************************************************
 *               Public function prototype
 *************************************************/

int service_file_get_list(service_file_directory_e directory, char ***file_names, int *len);

int service_file_load(service_file_directory_e directory, const char *file_name, service_file_h *service_file);

int service_file_get_string(service_file_h service_file, service_file_section_e section, const char *key, char **value);

int service_file_unload(service_file_h service_file);


/*************************************************
 *               Internal function definition
 *************************************************/


/*************************************************
 *               Public function definition
 *************************************************/

int service_file_get_list(service_file_directory_e directory, char ***file_names, int *files_len)
{
	service_adaptor_debug("<Start> %s", __FUNCTION__);
	if ((NULL == file_names) || (NULL == files_len)) {
		return -101;
	}

	if (SERVICE_FILE_DIRECTORY_PUSH != directory) {
		service_adaptor_debug("Not supported yet");
		return -102;
	}

	DIR *dirp = NULL;
	struct dirent dent, *result = NULL;

	dirp = opendir(SERVICE_FILE_PUSH_PATH);
	if (NULL == dirp) {
		service_adaptor_error("dir open error");
		return -103;
	}

	GList *_file_list = NULL;
	while (0 == readdir_r(dirp, &dent, &result)) {
		if (NULL == result) {
			break;
		}
		service_adaptor_debug_func("===== entry name [%s]", dent.d_name);
		if ((0 == strcmp(".", dent.d_name)) || (0 == strcmp("..", dent.d_name))) {
			continue;
		}
		char *file = strdup(dent.d_name);
		if (NULL != file) {
			_file_list = g_list_append(_file_list, (void *)file);
		}
	}

	closedir(dirp);

	int len = g_list_length(_file_list);
	char **list = NULL;
	if (0 < len) {
		list = (char **) calloc(len, sizeof(char *));
		if (NULL != list) {
			for (int i = 0; i < len; i++) {
				list[i] = (char *) g_list_nth_data(_file_list, i);
			}
		} else {
			g_list_free_full(_file_list, free);
			return -104;
		}
	}

	*file_names = list;
	*files_len = len;
	g_list_free(_file_list);

	service_adaptor_debug("<End> %s", __FUNCTION__);
	return 0;
}

int service_file_load(service_file_directory_e directory, const char *file_name, service_file_h *service_file)
{
	service_adaptor_debug("<Start> %s", __FUNCTION__);
	if ((NULL == file_name) || (NULL == service_file)) {
		return -201;
	}

	if (SERVICE_FILE_DIRECTORY_PUSH != directory) {
		service_adaptor_debug("Not supported yet");
		return -202;
	}

	service_adaptor_debug("Aollcates handle's memory");
	char *path = g_strconcat(SERVICE_FILE_PUSH_PATH, file_name, NULL);
	service_file_h _service = (service_file_h) calloc(1, sizeof(service_file_t));
	ConfigHandle handle = plugin_config_create();

	if ((NULL == path) || (NULL == _service) || (NULL == handle)) {
		service_adaptor_error("Memory allocation failed");
		g_free(path);
		free(_service);
		plugin_config_delete(handle);
		return -203;
	}

	plugin_config_load(handle, path, CCT_INI);

	_service->handle = handle;
	*service_file = _service;

	g_free(path);

	service_adaptor_debug("<End> %s", __FUNCTION__);
	return 0;
}

int service_file_get_string(service_file_h service_file, service_file_section_e section, const char *key, char **value)
{
	service_adaptor_debug("<Start> %s", __FUNCTION__);
	if ((NULL == service_file) || (NULL == key) || (NULL == value)) {
		return -301;
	}

	char section_name[10] = {0, };
	switch (section) {
	case SERVICE_FILE_SECTION_GENERAL:
		snprintf(section_name, 10, "%s", SERVICE_FILE_SECTION_STRING_GENERAL);
		break;
	case SERVICE_FILE_SECTION_PUSH:
		snprintf(section_name, 10, "%s", SERVICE_FILE_SECTION_STRING_PUSH);
		break;
	case SERVICE_FILE_SECTION_BUS:
		snprintf(section_name, 10, "%s", SERVICE_FILE_SECTION_STRING_BUS);
		break;
	default:
		service_adaptor_debug("Invalid section (%d)", section);
		return -302;
	}

	char *data = (char *) plugin_config_get_string(service_file->handle, section_name, key);
	service_adaptor_debug_func("Config result : section(%s) key(%s) value(%s)",
			section_name, key, data);
	char *_data = NULL;
	if ((NULL == data) || (NULL == (_data = strdup(data)))) {
		return -303;
	}
	*value = _data;

	service_adaptor_debug("<End> %s", __FUNCTION__);
	return 0;
}

int service_file_unload(service_file_h service_file)
{
	service_adaptor_debug("<Start> %s", __FUNCTION__);
	if (NULL == service_file) {
		return -401;
	}

	plugin_config_delete(service_file->handle);
	free(service_file);

	service_adaptor_debug("<End> %s", __FUNCTION__);
	return 0;
}


