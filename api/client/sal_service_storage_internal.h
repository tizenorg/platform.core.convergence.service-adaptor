/*
 * Service Storage Internal
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

#ifndef __SERVICE_STORAGE_INTERNAL_H__
#define __SERVICE_STORAGE_INTERNAL_H__

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C"
{
#endif
/*
#include <glib.h>

#include "sal_service_adaptor.h"
#include "sal_service_storage.h"

typedef struct _service_storage_cloud_file_s
{
	service_plugin_h plugin;
	service_storage_cloud_file_cb callback;
	void *user_data;

	bool is_dir;
	char *dir_path;
	char *local_path;
	char *cloud_path;
	unsigned long long size;
	char *operation;

	GList *files;
} service_storage_cloud_file_s;

typedef struct _service_storage_task_s
{
        char *service_handle_name;
}service_storage_task_t;
*/
/*==================================================================================================
                                         FUNCTION PROTOTYPES
==================================================================================================*/
/*
int service_storage_cloud_start(service_storage_cloud_file_h file);
int service_storage_cloud_stop(service_storage_cloud_file_h file);
*/
/*==================================================================================================
                                       2.4 FUNCTION PROTOTYPES
==================================================================================================*/

/**
 * Storage adaptor content type
 */
/*
typedef enum _service_storage_file_content_type_e
{
	SERVICE_ADAPTOR_STORAGE_CONTENT_TYPE_DEFAULT               = -1,    // initalize value

	SERVICE_ADAPTOR_STORAGE_CONTENT_TYPE_IMGAE                 = 160,
	SERVICE_ADAPTOR_STORAGE_CONTENT_TYPE_VIDEO                 = 161,
	SERVICE_ADAPTOR_STORAGE_CONTENT_TYPE_SOUND                 = 162,
	SERVICE_ADAPTOR_STORAGE_CONTENT_TYPE_MUSIC                 = 163,
	SERVICE_ADAPTOR_STORAGE_CONTENT_TYPE_OTHER                 = 164,
	SERVICE_ADAPTOR_STORAGE_CONTENT_TYPE_DOCUMENT              = 165,
	SERVICE_ADAPTOR_STORAGE_CONTENT_TYPE_THUMBNAIL             = 166,

	SERVICE_ADAPTOR_STORAGE_CONTENT_TYPE_CHUNK_MASK            = 320,
	SERVICE_ADAPTOR_STORAGE_CONTENT_TYPE_IMGAE_CHUNK           = 480,
	SERVICE_ADAPTOR_STORAGE_CONTENT_TYPE_VIDEO_CHUNK           = 481,
	SERVICE_ADAPTOR_STORAGE_CONTENT_TYPE_SOUND_CHUNK           = 482,
	SERVICE_ADAPTOR_STORAGE_CONTENT_TYPE_MUSIC_CHUNK           = 483,
	SERVICE_ADAPTOR_STORAGE_CONTENT_TYPE_OTHER_CHUNK           = 484,
	SERVICE_ADAPTOR_STORAGE_CONTENT_TYPE_DOCUMENT_CHUNK        = 485,
	SERVICE_ADAPTOR_STORAGE_CONTENT_TYPE_THUMBNAIL_CHUNK       = 486,

	SERVICE_ADAPTOR_STORAGE_CONTENT_TYPE_FOLDER                = 1024,
	SERVICE_ADAPTOR_STORAGE_CONTENT_TYPE_METADATA              = 2048,
} service_storage_content_type_e;

typedef struct _service_storage_media_meta_s
{
	char *mime_type;
	char *title;
	char *album;
	char *artist;
	char *genere;
	char *recorded_date;
	int width;
	int height;
	int duration;
	char *copyright;
	char *track_num;
	char *description;
	char *composer;
	char *year;
	int bitrate;
	int samplerate;
	int channel;
	char *extra_media_meta;
} service_storage_media_meta_s;

typedef struct _service_storage_cloud_meta_s
{
	char *service_name;
	unsigned long long usage_byte;
	unsigned long long quota_byte;
	char *extra_cloud_meta;
} service_storage_cloud_meta_s;
*/

/**
* @brief Describes file information description
*/

/*
struct _service_storage_file_s
{
	char    *plugin_name;		// specifies plugin name generated file_info
	char    *object_id;		   // specifies file object id be used in storage /
	char    *storage_path;		// specifies file path in storage /
	unsigned long long file_size;	//< specifies file size (recomend byte)/
	unsigned long long created_time;	//< specifies timestamp /
	unsigned long long modified_time;	//< specifies timestamp /
	int     file_info_index;	//< specifies file info index (wide use; e.g : chunk upload, multi download)/
	service_storage_content_type_e content_type; //< specifies file content type (reference service_adaptor_file_content_type_e)  /

	service_storage_media_meta_s *media_meta;
	service_storage_cloud_meta_s *cloud_meta;
	char    *extra_file_info;		//< specifies content name in metadata/
};


typedef struct _service_storage_file_s service_storage_file_s;
*/
/**
* @brief The handle for File Description
*/
//typedef struct _service_storage_file_s *service_storage_file_h;

#ifdef __cplusplus
}
#endif

#endif /* __SERVICE_STORAGE_INTERNAL_H__ */
