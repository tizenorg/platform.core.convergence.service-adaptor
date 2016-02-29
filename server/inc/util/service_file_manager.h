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

#ifndef __SERVICE_FILE_MANAGER_H__
#define __SERVICE_FILE_MANAGER_H__

typedef enum
{
	SERVICE_FILE_DIRECTORY_AUTH	= (0x01 << 0),
	SERVICE_FILE_DIRECTORY_STORAGE	= (0x01 << 1),
	SERVICE_FILE_DIRECTORY_CONTACT	= (0x01 << 2),
	SERVICE_FILE_DIRECTORY_MESSAGE	= (0x01 << 3),
	SERVICE_FILE_DIRECTORY_SHOP	= (0x01 << 4),
	SERVICE_FILE_DIRECTORY_PUSH	= (0x01 << 5),

	SERVICE_FILE_DIRECTORY_SERVICES	= (0x01 << 10) & 0x01,
	SERVICE_FILE_DIRECTORY_PLUGINS	= (0x01 << 10) & 0x02,
} service_file_directory_e;

typedef enum
{
	SERVICE_FILE_SECTION_GENERAL	= 1,
	SERVICE_FILE_SECTION_PUSH	= 2,
	SERVICE_FILE_SECTION_BUS	= 3,
} service_file_section_e;

typedef struct _service_file_s *service_file_h;

int service_file_get_list(service_file_directory_e directory, char ***file_names, int *len);

int service_file_load(service_file_directory_e directory, const char *file_name, service_file_h *service_file);

int service_file_get_string(service_file_h service_file, service_file_section_e section, const char *key, char **value);

int service_file_unload(service_file_h service_file);

#endif /* __SERVICE_FILE_MANAGER_H__ */
