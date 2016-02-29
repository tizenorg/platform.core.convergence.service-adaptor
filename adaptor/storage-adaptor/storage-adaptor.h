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

#ifndef __STORAGE_ADAPTOR_H__
#define __STORAGE_ADAPTOR_H__

#ifndef EXPORT_API
#define EXPORT_API __attribute__((visibility("default")))
#endif

#include <glib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Storage adaptor error code
 */
typedef enum storage_error_code_e
{
	STORAGE_ADAPTOR_ERROR_NONE			= 0,
	STORAGE_ADAPTOR_ERROR_LAUNCH                    = 1,    /**< 1 ~ 99: internal error*/
	STORAGE_ADAPTOR_ERROR_INIT                      = 2,
	STORAGE_ADAPTOR_ERROR_DEINIT                    = 3,
	STORAGE_ADAPTOR_ERROR_CREATE                    = 4,
	STORAGE_ADAPTOR_ERROR_DESTROY                   = 5,
	STORAGE_ADAPTOR_ERROR_START                     = 6,
	STORAGE_ADAPTOR_ERROR_STOP                      = 7,
	STORAGE_ADAPTOR_ERROR_CONNECT                   = 8,
	STORAGE_ADAPTOR_ERROR_DISCONNECT                = 9,
	STORAGE_ADAPTOR_ERROR_NOT_FOUND                 = 10,
	STORAGE_ADAPTOR_ERROR_CORRUPTED                 = 11,
	STORAGE_ADAPTOR_ERROR_UNSUPPORTED               = 12,
	STORAGE_ADAPTOR_ERROR_INVALID_HANDLE            = 13,
	STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT          = 14,
	STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT_TYPE	= 15,
	STORAGE_ADAPTOR_ERROR_NOT_AUTHORIZED            = 16,
	STORAGE_ADAPTOR_ERROR_ADAPTOR_INTERNAL          = 17,
	STORAGE_ADAPTOR_ERROR_PLUGIN_INTERNAL           = 18,
	STORAGE_ADAPTOR_ERROR_SERVER_INTERNAL           = 19,
	STORAGE_ADAPTOR_ERROR_DBUS                      = 20,
	STORAGE_ADAPTOR_ERROR_CALLBACK_TIME_OUT         = 21,
	STORAGE_ADAPTOR_ERROR_MAX
} storage_error_code_t;

/**
 * Storage adaptor error code
 */
typedef enum _storage_plugin_internal_error_code_e
{
	STORAGE_PLUGIN_ERROR_HTTP_BAD_REQUEST	 	= 400,
	STORAGE_PLUGIN_ERROR_HTTP_UNAUTHORIZED	 	= 401,
	STORAGE_PLUGIN_ERROR_HTTP_FORBIDDEN		= 403,
	STORAGE_PLUGIN_ERROR_HTTP_NOT_FOUND		= 404,
	STORAGE_PLUGIN_ERROR_HTTP_METHOD_NOT_ALLOWED	= 405,
	STORAGE_PLUGIN_ERROR_HTTP_BAD_GATEWAY	 	= 502,
	STORAGE_PLUGIN_ERROR_HTTP_SERVICE_UNAVAILBLE	= 503,
	STORAGE_PLUGIN_ERROR_HTTP_INSUFFICIENT_STORAGE 	= 507,
	STORAGE_PLUGIN_ERROR_HTTP_ETC			= 598,
	STORAGE_PLUGIN_ERROR_HTTP_UNKNOWN		= 599,

	STORAGE_PLUGIN_ERROR_CURL_COULDNT_CONNECT	= 601,
	STORAGE_PLUGIN_ERROR_CURL_TIMEOUT		= 602,
	STORAGE_PLUGIN_ERROR_CURL_ETC			= 698,
	STORAGE_PLUGIN_ERROR_CURL_UNKNOWN		= 699,

	STORAGE_PLUGIN_ERROR_FILE_OPEN_FAILED		= 701,
	STORAGE_PLUGIN_ERROR_FILE_NOT_EXIST		= 702,
	STORAGE_PLUGIN_ERROR_FILE_TRANSFER_CANCELED	= 703,
	STORAGE_PLUGIN_ERROR_FILE_AREADY_EXIST		= 704, // EEXIST
	STORAGE_PLUGIN_ERROR_FILE_ACCESS_DENIED		= 705, // EACCES
	STORAGE_PLUGIN_ERROR_FILE_ETC			= 798,
	STORAGE_PLUGIN_ERROR_FILE_UNKNOWN		= 799,

	STORAGE_PLUGIN_ERROR_MEMORY_ALLOCATION_FAILED	= 801,
	STORAGE_PLUGIN_ERROR_MEMORY_ETC			= 898,
	STORAGE_PLUGIN_ERROR_MEMORY_UNKNOWN		= 899,

	STORAGE_PLUGIN_ERROR_THREAD_CREATE_FAILED	= 901,
	STORAGE_PLUGIN_ERROR_THREAD_STOPPED		= 902,
	STORAGE_PLUGIN_ERROR_THREAD_ETC			= 908,
	STORAGE_PLUGIN_ERROR_THREAD_UNNOWN		= 909,
	STORAGE_PLUGIN_ERROR_ETC			= 998,
	STORAGE_PLUGIN_ERROR_UNKNOWN			= 999,
} storage_plugin_internal_error_code_e;

/**
 * Storage adaptor status code
 */
typedef enum _storage_adaptor_transfer_state_e
{
	STORAGE_ADAPTOR_TRANSFER_STATE_IN_PROGRESS	= 1,
	STORAGE_ADAPTOR_TRANSFER_STATE_FINISHED		= 2,
	STORAGE_ADAPTOR_TRANSFER_STATE_CANCELED		= 3,	// canceled by request
	STORAGE_ADAPTOR_TRANSFER_STATE_FAILED		= 4,	// canceled by system

	STORAGE_ADAPTOR_TRANSFER_STATE_RESUME,	// not use this version yet (Next feature)
	STORAGE_ADAPTOR_TRANSFER_STATE_PAUSED,	// not use this version yet (Next feature)

	// Private feature
	STORAGE_ADAPTOR_TRANSFER_STATUS_PROGRESS	= 1,
	STORAGE_ADAPTOR_TRANSFER_STATUS_RESUME		= 1,
	STORAGE_ADAPTOR_TRANSFER_STATUS_PAUSE		= 2,
	STORAGE_ADAPTOR_TRANSFER_STATUS_CANCEL		= 3,	// canceled by request
	STORAGE_ADAPTOR_TRANSFER_STATUS_STOPPED		= 4,	// canceled by system
	STORAGE_ADAPTOR_TRANSFER_STATUS_FINISHED	= 5
} storage_adaptor_transfer_state_e;

typedef storage_adaptor_transfer_state_e storage_adaptor_transfer_status_e;

typedef enum
{
	STORAGE_ADAPTOR_FILE_ACCESS_READ	= O_RDONLY,
	STORAGE_ADAPTOR_FILE_ACCESS_WRITE	= O_WRONLY|O_CREAT|O_EXCL,
} storage_adaptor_file_access_mode_e;

/**
 * Storage adaptor content type
 */
typedef enum _storage_adaptor_content_type_e
{
	STORAGE_ADAPTOR_CONTENT_TYPE_DEFAULT		= -1,	// initalize value

	STORAGE_ADAPTOR_CONTENT_TYPE_IMGAE		= 160,
	STORAGE_ADAPTOR_CONTENT_TYPE_VIDEO		= 161,
	STORAGE_ADAPTOR_CONTENT_TYPE_SOUND		= 162,
	STORAGE_ADAPTOR_CONTENT_TYPE_MUSIC		= 163,
	STORAGE_ADAPTOR_CONTENT_TYPE_OTHER		= 164,
	STORAGE_ADAPTOR_CONTENT_TYPE_DOCUMENT		= 165,
	STORAGE_ADAPTOR_CONTENT_TYPE_THUMBNAIL		= 166,

	STORAGE_ADAPTOR_CONTENT_TYPE_CHUNK_MASK		= 320,
	STORAGE_ADAPTOR_CONTENT_TYPE_IMGAE_CHUNK	= 480,
	STORAGE_ADAPTOR_CONTENT_TYPE_VIDEO_CHUNK	= 481,
	STORAGE_ADAPTOR_CONTENT_TYPE_SOUND_CHUNK	= 482,
	STORAGE_ADAPTOR_CONTENT_TYPE_MUSIC_CHUNK	= 483,
	STORAGE_ADAPTOR_CONTENT_TYPE_OTHER_CHUNK	= 484,
	STORAGE_ADAPTOR_CONTENT_TYPE_DOCUMENT_CHUNK	= 485,
	STORAGE_ADAPTOR_CONTENT_TYPE_THUMBNAIL_CHUNK	= 486,

	STORAGE_ADAPTOR_CONTENT_TYPE_FOLDER		= 1024,
	STORAGE_ADAPTOR_CONTENT_TYPE_METADATA		= 2048,
} storage_adaptor_content_type_e;


/**
 * Storage adaptor plugin handle
 */
typedef struct storage_adaptor_plugin_s *storage_adaptor_plugin_h;

/**
 * Storage adaptor
 */
typedef struct storage_adaptor_s *storage_adaptor_h;

/**
 * Storage adaptor plugin context structure
 */
typedef struct storage_adaptor_plugin_context_s
{
	// Context variables
	int context_id;
	storage_adaptor_plugin_h plugin_handle;

	// User define (input by service-adaptor)
	char *app_id;
	char *app_secret;
	char *access_token;
	char *cid;
	char *uid;
	char *service_name;

	// Plugin define (input by plugin)
	char *plugin_uri;	// mandatory (package id)
	void *plugin_data;	// optional
} storage_adaptor_plugin_context_t;
typedef struct storage_adaptor_plugin_context_s *storage_adaptor_plugin_context_h;

/**
 * Structure for error code from server
 */
typedef struct storage_adaptor_error_code_s
{
	int64_t	code;
	char	*msg;
} storage_adaptor_error_code_t;
typedef struct storage_adaptor_error_code_s *storage_adaptor_error_code_h;

typedef struct _storage_adaptor_media_meta_s
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
} storage_adaptor_media_meta_s;

typedef struct _storage_adaptor_cloud_meta_s
{
	char *service_name;
	unsigned long long usage_byte;
	unsigned long long quota_byte;
	char *extra_cloud_meta;
} storage_adaptor_cloud_meta_s;

// private only!!
/**
 * Structure Storage File Share token Infomation
 */
typedef struct storage_adaptor_file_share_token_s
{
	char *public_token;
	char *auth_code;
} storage_adaptor_file_share_token_t;
typedef struct storage_adaptor_file_share_token_s *storage_adaptor_file_share_token_h;

/**
 * Structure Storage File Infomation
 */
typedef struct storage_adaptor_file_info_s
{
	// Common
	char    *plugin_uri;		/**< specifies plugin name generated file_info */
	char    *object_id;		/**< specifies file object id be used in storage */
	char    *storage_path;		/**< specifies file path in storage */
	unsigned long long file_size;	/**< specifies file size (recomend byte)*/
	int     file_info_index;	/**< specifies file info index (wide use; e.g : chunk upload, multi download)*/

	// private only!!
	int	revision;
	unsigned long long timestamp;
	char	*type;
	int	deleted;
	unsigned long long expired_time;
	unsigned int download_count;
	unsigned int max_download_count;
	char	*tag;
	storage_adaptor_file_share_token_h file_share_token;


	// public defined
	unsigned long long created_time;	/**< specifies timestamp */
	unsigned long long modified_time;	/**< specifies timestamp */
	storage_adaptor_content_type_e content_type;

	storage_adaptor_media_meta_s *media_meta;
	storage_adaptor_cloud_meta_s *cloud_meta;
	char    *extra_file_info;		/**< specifies content name in metadata */
} storage_adaptor_file_info_t;
typedef struct storage_adaptor_file_info_s *storage_adaptor_file_info_h;

/**
 * Storage adaptor plugin listener
 */
typedef struct storage_adaptor_plugin_listener_s *storage_adaptor_plugin_listener_h;

/**
 * Storage adaptor plugin handle
 */
typedef struct storage_adaptor_plugin_handle_s
{
	// Mandatory functions to handle plugin in adaptor
	storage_error_code_t (*create_context)(storage_adaptor_plugin_context_h *context,
							const char *app_id,
							const char *app_secret,
							const char *access_token,
							const char *cid,
							const char *uid);
	storage_error_code_t (*destroy_context)(storage_adaptor_plugin_context_h context);
	storage_error_code_t (*destroy_handle)(struct storage_adaptor_plugin_handle_s *handle);
	storage_error_code_t (*set_listener)(storage_adaptor_plugin_listener_h listener);
	storage_error_code_t (*unset_listener)(void);
	// Mandatory end

	// Optional

	storage_error_code_t (*open_file) (storage_adaptor_plugin_context_h context,		// Do Not define from plugin (TBD)
							const char *file_path,
							storage_adaptor_file_access_mode_e mode,
							int *file_descriptor,
							storage_adaptor_error_code_h *error);

	storage_error_code_t (*close_file) (storage_adaptor_plugin_context_h context,		// Do Not define from plugin (TBD)
							int file_descriptor,
							storage_adaptor_error_code_h *error);

	storage_error_code_t (*start_upload_task)(storage_adaptor_plugin_context_h context,
							int src_file_descriptor,		// read only opened
							const char *upload_dir_path,
							const char *file_name,
							bool need_progress,
							storage_adaptor_error_code_h *error,
							void *user_data);

	storage_error_code_t (*start_download_task)(storage_adaptor_plugin_context_h context,
							const char *storage_dir_path,
							const char *file_name,
							int dst_file_descriptor,		// write only opened
							bool need_progress,
							storage_adaptor_error_code_h *error,
							void *user_data);

	storage_error_code_t (*start_download_thumb_task)(storage_adaptor_plugin_context_h context,
							const char *storage_dir_path,
							const char *file_name,
							int dst_file_descriptor,		// write only opened
							int thumbnail_size,			// level (defined plugin SPEC)
							bool need_progress,
							storage_adaptor_error_code_h *error,
							void *user_data);

	storage_error_code_t (*cancel_upload_task)(storage_adaptor_plugin_context_h context,
							int file_descriptor,
							storage_adaptor_error_code_h *error);

	storage_error_code_t (*cancel_download_task)(storage_adaptor_plugin_context_h context,
							int file_descriptor,
							storage_adaptor_error_code_h *error);

	storage_error_code_t (*cancel_download_thumb_task)(storage_adaptor_plugin_context_h context,
							int file_descriptor,
							storage_adaptor_error_code_h *error);

	// common (yet)
	storage_error_code_t (*set_server_info)(storage_adaptor_plugin_context_h context,
							GHashTable *server_info,
							void *request,
							storage_adaptor_error_code_h *error,
							void *response);
	// Optional end
	// common
	storage_error_code_t (*get_root_folder_path)(storage_adaptor_plugin_context_h context,
							void *request,
							char **root_folder_path,
							storage_adaptor_error_code_h *error,
							void *response);

	// common
	storage_error_code_t (*list)(storage_adaptor_plugin_context_h context,
							const char *parent_folder_storage_path,
							const char *folder_name,
							void *request,
							storage_adaptor_file_info_h **file_info_list,
							int *file_info_list_len,
							storage_adaptor_error_code_h *error,
							void *response);

	// common
	storage_error_code_t (*make_directory)(storage_adaptor_plugin_context_h context,
							const char *parent_folder_storage_path,
							const char *folder_name,
							void *request,
							storage_adaptor_file_info_h *file_info,
							storage_adaptor_error_code_h *error,
							void *response);

	// common
	storage_error_code_t (*upload_file_sync)(storage_adaptor_plugin_context_h context,
							const char *parent_folder_storage_path,
							const char *file_name,
							const char *upload_file_local_path,
							const int publish,
							void *request,
							storage_adaptor_file_info_h *file_info,
							storage_adaptor_error_code_h *error,
							void *response);

	// common
	storage_error_code_t (*download_file_sync)(storage_adaptor_plugin_context_h context,
							const char *parent_folder_storage_path,
							const char *file_name,
							const char *download_file_local_path,
							void *request,
							storage_adaptor_error_code_h *error,
							void *response);

	// common
	storage_error_code_t (*download_thumbnail)(storage_adaptor_plugin_context_h context,
							const char *folder_path,
							const char *file_name,
							const char *download_path,
							int thumbnail_size,
							void *request,
							storage_adaptor_error_code_h *error,
							void *response);

	// common
	storage_error_code_t (*delete_file)(storage_adaptor_plugin_context_h context,
							const char *parent_folder_storage_path,
							const char *file_name,
							void *request,
							storage_adaptor_file_info_h *file_info,
							storage_adaptor_error_code_h *error,
							void *response);

	// common
	storage_error_code_t (*remove_directory)(storage_adaptor_plugin_context_h context,
							const char *parent_folder_storage_path,
							const char *folder_name,
							void *request,
							storage_adaptor_file_info_h *file_info,
							storage_adaptor_error_code_h *error,
							void *response);

	// common
	storage_error_code_t (*move_file)(storage_adaptor_plugin_context_h context,
							const char *parent_folder_storage_path,
							const char *file_name,
							const char *dest_parent_folder_storage_path,
							const char *new_file_name,
							void *request,
							storage_adaptor_file_info_h *file_info,
							storage_adaptor_error_code_h *error,
							void *response);

	// common
	storage_error_code_t (*move_directory)(storage_adaptor_plugin_context_h context,
							const char *parent_folder_storage_path,
							const char *folder_name,
							const char *dest_parent_folder_storage_path,
							const char *new_folder_name,
							void *request,
							storage_adaptor_file_info_h *file_info,
							storage_adaptor_error_code_h *error,
							void *response);

	// common
	storage_error_code_t (*set_transfer_state)(storage_adaptor_plugin_context_h context,
							void *transfer_request_id,
							storage_adaptor_transfer_state_e state,
							void *request,
							storage_adaptor_error_code_h *error,
							void *response);

	// common
	storage_error_code_t (*get_transfer_state)(storage_adaptor_plugin_context_h context,
							void *transfer_request_id,
							void *request,
							storage_adaptor_transfer_state_e *state,
							storage_adaptor_error_code_h *error,
							void *response);


	storage_error_code_t (*upload_file_async)(storage_adaptor_plugin_context_h context,
							const char *parent_folder_storage_path,
							const char *file_name,
							const char *upload_file_local_path,
							const int publish,
							void *request,
							void *transfer_request_id);

	storage_error_code_t (*download_file_async)(storage_adaptor_plugin_context_h context,
							const char *parent_folder_storage_path,
							const char *file_name,
							const char *download_file_local_path,
							void *request,
							void *transfer_request_id);

	storage_error_code_t (*download_file_sync_by_public_token)(storage_adaptor_plugin_context_h context,
							const char *public_token,
							const char *auth_code,
							const char *download_file_local_path,
							void *request,
							storage_adaptor_error_code_h *error,
							void *response);

	storage_error_code_t (*download_file_async_by_public_token)(storage_adaptor_plugin_context_h context,
							const char *public_token,
							const char *auth_code,
							const char *download_file_local_path,
							void *request,
							void *transfer_request_id);

	storage_error_code_t (*get_transfer_progress)(storage_adaptor_plugin_context_h context,
							void *transfer_request_id,
							void *request,
							unsigned long long *progress_size_byte,
							unsigned long long *total_size_byte,
							storage_adaptor_error_code_h *error,
							void *response);

	storage_error_code_t (*set_meta)(storage_adaptor_plugin_context_h context,
							const char *parent_folder_storage_path,
							const char *file_name,
							const void *meta_data,
							void *request,
							storage_adaptor_file_info_h *file_info,
							storage_adaptor_error_code_h *error,
							void *response);

	storage_error_code_t (*get_meta)(storage_adaptor_plugin_context_h context,
							const char *parent_folder_storage_path,
							const char *file_name,
							void *request,
							storage_adaptor_file_info_h *file_info,
							void **meta_data,
							storage_adaptor_error_code_h *error,
							void *response);

	storage_error_code_t (*start_mupload)(storage_adaptor_plugin_context_h context,
							const char *parent_folder_storage_path,
							const char *file_name,
							const char *upload_file_local_path,
							const unsigned long long chunk_size_byte,
							void *request,
							char **mupload_key,
							int *chunk_count,
							storage_adaptor_file_info_h *file_info,
							storage_adaptor_error_code_h *error,
							void *response);

	storage_error_code_t (*upload_mupload)(storage_adaptor_plugin_context_h context,
							const char *mupload_key,
							const int chunk_number,
							void *request,
							storage_adaptor_file_info_h *file_info,
							storage_adaptor_error_code_h *error,
							void *response);

	storage_error_code_t (*end_mupload)(storage_adaptor_plugin_context_h context,
							const char *mupload_key,
							const int publish,
							void *request,
							storage_adaptor_file_info_h *file_info,
							storage_adaptor_error_code_h *error,
							void *response);

	storage_error_code_t (*list_mupload)(storage_adaptor_plugin_context_h context,
							const char *mupload_key,
							void *request,
							storage_adaptor_file_info_h **file_info_list,
							int *file_info_list_len,
							storage_adaptor_error_code_h *error,
							void *response);

	storage_error_code_t (*cancel_mupload)(storage_adaptor_plugin_context_h context,
							const char *mupload_key,
							void *request,
							storage_adaptor_file_info_h *file_info,
							storage_adaptor_error_code_h *error,
							void *response);

	storage_error_code_t (*start_transaction)(storage_adaptor_plugin_context_h context,
							void *request,
							char **tx_key,
							storage_adaptor_error_code_h *error,
							void *response);

	storage_error_code_t (*upload_file_transaction)(storage_adaptor_plugin_context_h context,
							const char *tx_key,
							const int tx_seq,
							const char *parent_folder_storage_path,
							const char *file_name,
							const char *upload_file_local_path,
							void *request,
							storage_adaptor_file_info_h *file_info,
							storage_adaptor_error_code_h *error,
							void *response);

	storage_error_code_t (*set_dir_transaction)(storage_adaptor_plugin_context_h context,
							const char *tx_key,
							const int tx_seq,
							const char *parent_folder_storage_path,
							const char *folder_name,
							void *request,
							storage_adaptor_file_info_h *file_info,
							storage_adaptor_error_code_h *error,
							void *response);

	storage_error_code_t (*delete_file_transaction)(storage_adaptor_plugin_context_h context,
							const char *tx_key,
							const int tx_seq,
							const char *parent_folder_storage_path,
							const char *file_name,
							void *request,
							storage_adaptor_file_info_h *file_info,
							storage_adaptor_error_code_h *error,
							void *response);

	storage_error_code_t (*delete_dir_transaction)(storage_adaptor_plugin_context_h context,
							const char *tx_key,
							const int tx_seq,
							const char *parent_folder_storage_path,
							const char *folder_name,
							void *request,
							storage_adaptor_file_info_h *file_info,
							storage_adaptor_error_code_h *error,
							void *response);

	storage_error_code_t (*end_transaction)(storage_adaptor_plugin_context_h context,
							const char *tx_key,
							const int tx_count,
							void *request,
							storage_adaptor_file_info_h **file_info_list,
							int *file_info_list_len,
							storage_adaptor_error_code_h *error,
							void *response);

	storage_error_code_t (*list_transaction)(storage_adaptor_plugin_context_h context,
							const char *tx_key,
							void *request,
							storage_adaptor_file_info_h **file_info_list,
							int *file_info_list_len,
							storage_adaptor_error_code_h *error,
							void *response);

	storage_error_code_t (*cancel_transaction)(storage_adaptor_plugin_context_h context,
							const char *tx_key,
							void *request,
							storage_adaptor_file_info_h **file_info_list,
							int *file_info_list_len,
							storage_adaptor_error_code_h *error,
							void *response);

	storage_error_code_t (*multi_file_upload)(storage_adaptor_plugin_context_h context,
							const char *parent_folder_storage_path,
							const char **upload_file_local_path_list,
							const int upload_list_len,
							void *request,
							storage_adaptor_error_code_h *error,
							void *response);

	storage_error_code_t (*multi_file_download)(storage_adaptor_plugin_context_h context,
							const char *parent_folder_storage_path,
							const char *file_name_list,
							const int file_name_list_len,
							const char *download_folder_local_path,
							void *request,
							storage_adaptor_file_info_h **file_info_list,
							int *file_info_list_len,
							storage_adaptor_error_code_h *error,
							void *response);

	storage_error_code_t (*get_timestamp)(storage_adaptor_plugin_context_h context,
							void *request,
							unsigned long long *timestamp,
							storage_adaptor_error_code_h *error,
							void *response);

	storage_error_code_t (*get_file_info_by_public_token)(storage_adaptor_plugin_context_h context,
							const char *public_token,
							const char *auth_code,
							void *request,
							storage_adaptor_file_info_h *file_info,
							storage_adaptor_error_code_h *error,
							void *response);

	storage_error_code_t (*auth_public_authcode_by_public_token)(storage_adaptor_plugin_context_h context,
							const char *public_token,
							const char *auth_code,
							void *request,
							storage_adaptor_error_code_h *error,
							void *response);

	storage_error_code_t (*redirect_url_by_public_token)(storage_adaptor_plugin_context_h context,
							const char *public_token,
							void *request,
							storage_adaptor_error_code_h *error,
							void *response);

	storage_error_code_t (*create_resuming_upload_url)(storage_adaptor_plugin_context_h context,
							const char *parent_folder_storage_path,
							const char *file_name,
							const unsigned long long x_upload_content_length,
							void *request,
							storage_adaptor_error_code_h *error,
							void *response);

	storage_error_code_t (*create_resuming_chunk_upload_url)(storage_adaptor_plugin_context_h context,
							const char *mupload_key,
							const int chunk_number,
							const unsigned long long x_upload_content_length,
							void *request,
							char **rupload_key,
							storage_adaptor_error_code_h *error,
							void *response);

	storage_error_code_t (*resuming_upload)(storage_adaptor_plugin_context_h context,
							const char *rupload_key,
							const char *content_range,
							const unsigned long long content_length,
							void *request,
							storage_adaptor_error_code_h *error,
							void *response);

	storage_error_code_t (*delete_multi_file_in_folder)(storage_adaptor_plugin_context_h context,
							const char *parent_folder_storage_path,
							const char **file_name_list,
							const int file_name_list_len,
							void *request,
							storage_adaptor_file_info_h **file_info_list,
							int *file_info_list_len,
							storage_adaptor_error_code_h *error,
							void *response);

	storage_error_code_t (*get_policy)(storage_adaptor_plugin_context_h context,
							void *request,
							char ***allowed_extension,
							int *allowed_extension_len,
							storage_adaptor_error_code_h *error,
							void *response);

	storage_error_code_t (*get_quota)(storage_adaptor_plugin_context_h context,
							void *request,
							unsigned long long *total_usage,
							unsigned long long *total_quota,
							storage_adaptor_error_code_h *error,
							void *response);

	// Optional end

	// Mandatory
	char *plugin_uri;	// get from config file
	// Mandatory end

} storage_adaptor_plugin_handle_t;
typedef struct storage_adaptor_plugin_handle_s *storage_adaptor_plugin_handle_h;

/**
 * Callback function variable for service adaptor
 */

// private feature
typedef void (*storage_adaptor_service_download_file_async_reply_cb)(void *transfer_request_id,
						char *download_file_local_path,
						storage_adaptor_error_code_h error,
						void *response);

typedef void (*storage_adaptor_service_upload_file_async_reply_cb)(void *transfer_request_id,
						storage_adaptor_file_info_h file_info,
						storage_adaptor_error_code_h error,
						void *response);

typedef void (*storage_adaptor_service_file_transfer_progress_reply_cb)(void *transfer_request_id,
						unsigned long long progress_size_byte,
						unsigned long long total_size_byte,
						storage_adaptor_error_code_h error,
						void *response);

// public feature
typedef void (*storage_adaptor_service_download_state_changed_reply_cb)(long long int file_descriptor,
						storage_adaptor_transfer_state_e state,
						storage_adaptor_error_code_h error,
						void *user_data);

typedef void (*storage_adaptor_service_upload_state_changed_reply_cb)(long long int file_descriptor,
						storage_adaptor_transfer_state_e state,
						storage_adaptor_file_info_h file_info,
						storage_adaptor_error_code_h error,
						void *user_data);

typedef void (*storage_adaptor_service_task_progress_reply_cb)(long long int file_descriptor,
						unsigned long long progress_size_byte,
						unsigned long long total_size_byte);



/**
 * Storage adaptor listener for service adaptor
 * Listener is used by service adaptor
 */
typedef struct storage_adaptor_listener_s
{
// private feature
	void (*download_file_async_reply)(void *transfer_request_id,
							char *download_file_local_path,
							storage_adaptor_error_code_h error,
							void *response);

	void (*upload_file_async_reply)(void *transfer_request_id,
							storage_adaptor_file_info_h file_info,
							storage_adaptor_error_code_h error,
							void *response);

	void (*file_transfer_progress_reply)(void *transfer_request_id,
							unsigned long long progress_size_byte,
							unsigned long long total_size_byte,
							storage_adaptor_error_code_h error,
							void *response);

// public feature
	void (*download_state_changed_reply)(long long int file_descriptor,
						storage_adaptor_transfer_state_e state,
						storage_adaptor_error_code_h error,
						void *user_data);

	void (*upload_state_changed_reply)(long long int file_descriptor,
						storage_adaptor_transfer_state_e state,
						storage_adaptor_file_info_h file_info,
						storage_adaptor_error_code_h error,
						void *user_data);

	void (*task_progress_reply)(long long int file_descriptor,
						unsigned long long progress_size_byte,
						unsigned long long total_size_byte);

} storage_adaptor_listener_t;
typedef struct storage_adaptor_listener_s *storage_adaptor_listener_h;

/**
 * Callback function variables for plugins
 * These callbacks are expected to be support by plugins
 */
// private feature
typedef void (*storage_adaptor_plugin_download_file_async_reply_cb)(void *transfer_request_id,
						char *download_file_local_path,
						storage_adaptor_error_code_h error,
						void *response);

typedef void (*storage_adaptor_plugin_upload_file_async_reply_cb)(void *transfer_request_id,
						storage_adaptor_file_info_h file_info,
						storage_adaptor_error_code_h error,
						void *response);

typedef void (*storage_adaptor_plugin_file_transfer_progress_reply_cb)(void *transfer_request_id,
						unsigned long long progress_size_byte,
						unsigned long long total_size_byte,
						storage_adaptor_error_code_h error,
						void *response);

// public feature
typedef void (*storage_adaptor_plugin_download_state_changed_reply_cb)(int file_descriptor,
						storage_adaptor_transfer_state_e state,
						storage_adaptor_error_code_h error,
						void *user_data);

typedef void (*storage_adaptor_plugin_upload_state_changed_reply_cb)(int file_descriptor,
						storage_adaptor_transfer_state_e state,
						storage_adaptor_file_info_h file_info,
						storage_adaptor_error_code_h error,
						void *user_data);

typedef void (*storage_adaptor_plugin_task_progress_reply_cb)(int file_descriptor,
						unsigned long long progress_size_byte,
						unsigned long long total_size_byte,
						storage_adaptor_error_code_h error,
						void *user_data);

/**
 * Storage adaptor listener for plugins
 * Listener is used by plugins
 */
typedef struct storage_adaptor_plugin_listener_s
{
// private feature
	storage_adaptor_plugin_download_file_async_reply_cb	storage_adaptor_download_file_async_reply;
	storage_adaptor_plugin_upload_file_async_reply_cb 	storage_adaptor_upload_file_async_reply;
	storage_adaptor_plugin_file_transfer_progress_reply_cb 	storage_adaptor_file_transfer_progress_reply;

// public feature
	storage_adaptor_plugin_download_state_changed_reply_cb	storage_adaptor_download_state_changed_reply;
	storage_adaptor_plugin_upload_state_changed_reply_cb 	storage_adaptor_upload_state_changed_reply;
	storage_adaptor_plugin_task_progress_reply_cb 	storage_adaptor_task_progress_reply;
} storage_adaptor_plugin_listener_t;

/**
 * Loads plugin from selected path
 */
EXPORT_API
int storage_adaptor_load_plugin(storage_adaptor_h,
						const char *plugin_path);

// For 3rd party plugin packages
EXPORT_API
int storage_adaptor_load_plugin_from_package(storage_adaptor_h adaptor,
						const char *package_id,
						const char *plugin_path);

/**
 * Unloads selected plugin
 */
EXPORT_API
int storage_adaptor_unload_plugin(storage_adaptor_h,
						storage_adaptor_plugin_h);

/**
 * Gets plugin name
 */
EXPORT_API
void storage_adaptor_get_plugin_uri(storage_adaptor_plugin_h plugin,
						char **plugin_uri);

/**
 * Refresh access token
 */
EXPORT_API
storage_error_code_t storage_adaptor_refresh_access_token(storage_adaptor_plugin_context_h context,
						const char *new_access_token);

/**
 * Refresh access token
 */
EXPORT_API
storage_error_code_t storage_adaptor_refresh_uid(storage_adaptor_plugin_context_h context,
						const char *new_uid);

/**
 * Create error code
 */
EXPORT_API
storage_adaptor_error_code_h storage_adaptor_create_error_code(const int64_t code,
						const char *msg);

/**
 * Destroy error code
 */
EXPORT_API
void storage_adaptor_destroy_error_code(storage_adaptor_error_code_h *error_code);

/**
 * Creates storage adaptor
 */
EXPORT_API
storage_adaptor_h storage_adaptor_create(const char *plugins_dir);

/**
 * Destroys storage adaptor
 * Destroys storage adaptor. If storage adaptor was started it is stopped first.
 */
EXPORT_API
void storage_adaptor_destroy(storage_adaptor_h adaptor);

/**
 * Starts storage adaptor
 * Starts storage adaptor and loads plugins that were found in plugins search dir
 * specified in storage_adaptor_create
 */
EXPORT_API
int storage_adaptor_start(storage_adaptor_h adaptor);

/**
 * Stops storage adaptor.
 */
EXPORT_API
int storage_adaptor_stop(storage_adaptor_h adaptor);

/**
 * Registers plugin state listener
 */
EXPORT_API
int storage_adaptor_register_listener(storage_adaptor_h,
						storage_adaptor_listener_h);

/**
 * Unregisters plugin state listener
 */
EXPORT_API
int storage_adaptor_unregister_listener(storage_adaptor_h,
						storage_adaptor_listener_h);

/**
 * Creates plugin context.
 */
EXPORT_API
storage_adaptor_plugin_context_h storage_adaptor_create_plugin_context(storage_adaptor_plugin_h plugin,
						const char *app_id,
						const char *app_secret,
						const char *access_token,
						const char *cid,
						const char *uid,
						const char *service_name);

/**
 * Destroys plugin context.
 */
EXPORT_API
void storage_adaptor_destroy_plugin_context(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h);

/**
 * Gets plugin with specified unique name
 */
EXPORT_API
storage_adaptor_plugin_h storage_adaptor_get_plugin_by_name(storage_adaptor_h adaptor,
						const char *plugin_uri);

/**
 * Gets plugins
 */
EXPORT_API
GList *storage_adaptor_get_plugins(storage_adaptor_h adaptor);

////////////////////////////////////////////////////////////
// Adaptor Util Functions
////////////////////////////////////////////////////////////
EXPORT_API
storage_adaptor_file_info_h storage_adaptor_create_file_info(void);

EXPORT_API
int storage_adaptor_destroy_file_info(storage_adaptor_file_info_h *file_info);


////////////////////////////////////////////////////////////
// Adaptor Plugin call Functions
////////////////////////////////////////////////////////////


EXPORT_API
storage_error_code_t storage_adaptor_open_file(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *file_path,
						storage_adaptor_file_access_mode_e mode,
						long long int *file_uid,
						storage_adaptor_error_code_h *error);

EXPORT_API
storage_error_code_t storage_adaptor_close_file(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						long long int file_uid,
						storage_adaptor_error_code_h *error);

EXPORT_API
storage_error_code_t storage_adaptor_start_upload_task(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						long long int src_file_descriptor,		// read only opened
						const char *upload_dir_path,
						const char *file_name,
						bool need_progress,
						storage_adaptor_error_code_h *error,
						void *user_data);

EXPORT_API
storage_error_code_t storage_adaptor_start_download_task(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *storage_dir_path,
						const char *file_name,
						long long int dst_file_descriptor,		// write only opened
						bool need_progress,
						storage_adaptor_error_code_h *error,
						void *user_data);

EXPORT_API
storage_error_code_t storage_adaptor_start_download_thumb_task(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *storage_dir_path,
						const char *file_name,
						long long int dst_file_descriptor,		// write only opened
						int thumbnail_size,			// level (defined plugin SPEC)
						bool need_progress,
						storage_adaptor_error_code_h *error,
						void *user_data);

EXPORT_API
storage_error_code_t storage_adaptor_cancel_upload_task(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						long long int file_uid,
						storage_adaptor_error_code_h *error);

EXPORT_API
storage_error_code_t storage_adaptor_cancel_download_task(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						long long int file_uid,
						storage_adaptor_error_code_h *error);

EXPORT_API
storage_error_code_t storage_adaptor_cancel_download_thumb_task(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						long long int file_uid,
						storage_adaptor_error_code_h *error);

/**
* @brief Set server information for Storage Plugin
*
* @param[in]	plugin				specifies Storage Adaptor Plugin handle
* @param[in]	context				specifies Storage Adaptor Plugin Context handle
* @param[in]	server_info			specifies server information for Storage Plugin
* @param[in]	request				specifies optional parameter
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
storage_error_code_t storage_adaptor_set_server_info(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						GHashTable *server_info,
						void *request,
						storage_adaptor_error_code_h *error,
						void *response);

/**
* @brief Makes a directory at cloud
*
* @param[in]	plugin				specifies Storage Adaptor Plugin handle
* @param[in]	context				specifies Storage Adaptor Plugin Context handle
* @param[in]	parent_folder_storage_path	specifies path to locate the folder you want to create
* @param[in]	folder_name			specifies folder name to be created at cloud
* @param[in]	request				specifies optional parameter
* @param[out]	file_info			specifies Storage Adaptor File Info handle
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
storage_error_code_t storage_adaptor_make_directory(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *parent_folder_storage_path,
						const char *folder_name,
						void *request,
						storage_adaptor_file_info_h *file_info,
						storage_adaptor_error_code_h *error,
						void *response);

/**
* @brief Removes a directory at cloud
*
* @param[in]	plugin				specifies Storage Adaptor Plugin handle
* @param[in]	context				specifies Storage Adaptor Plugin Context handle
* @param[in]	parent_folder_storage_path	specifies parent folder path of folder you want to delete
* @param[in]	folder_name			specifies folder name to be deleted from cloud
* @param[in]	request				specifies optional parameter
* @param[out]	file_info			specifies Storage Adaptor File Info handle
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
storage_error_code_t storage_adaptor_remove_directory(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *parent_folder_storage_path,
						const char *folder_name,
						void *request,
						storage_adaptor_file_info_h *file_info,
						storage_adaptor_error_code_h *error,
						void *response);

/**
* @brief Requests folder and file list in a folder
*
* @param[in]	plugin				specifies Storage Adaptor Plugin handle
* @param[in]	context				specifies Storage Adaptor Plugin Context handle
* @param[in]	parent_folder_storage_path	specifies parent folder path of folder you want to get list
* @param[in]	folder_name			specifies folder name you want to get list
* @param[in]	request				specifies optional parameter
* @param[out]	file_info_list			specifies Storage Adaptor File Info handle
* @param[out]	file_info_list_len		specifies length of the file_info_list
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
storage_error_code_t storage_adaptor_list(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *parent_folder_storage_path,
						const char *folder_name,
						void *request,
						storage_adaptor_file_info_h **file_info_list,
						int *file_info_list_len,
						storage_adaptor_error_code_h *error,
						void *response);

/**
* @brief Uploads a file to cloud (Sync)
*
* @param[in]	plugin				specifies Storage Adaptor Plugin handle
* @param[in]	context				specifies Storage Adaptor Plugin Context handle
* @param[in]	parent_folder_storage_path	specifies folder path of file you want to upload
* @param[in]	file_name			specifies file name to be uploaded to cloud
* @param[in]	upload_file_local_path		specifies local path of the file to be uploaded
* @param[in]	publish				specifies Allow to share file with no authentication
* @param[in]	request				specifies optional parameter
* @param[out]	file_info			specifies Storage Adaptor File Info handle
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
storage_error_code_t storage_adaptor_upload_file_sync(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *parent_folder_storage_path,
						const char *file_name,
						const char *upload_file_local_path,
						const int publish,
						void *request,
						storage_adaptor_file_info_h *file_info,
						storage_adaptor_error_code_h *error,
						void *response);

/**
* @brief Uploads a file to cloud (Async)
*
* @param[in]	plugin				specifies Storage Adaptor Plugin handle
* @param[in]	context				specifies Storage Adaptor Plugin Context handle
* @param[in]	parent_folder_storage_path	specifies folder path of file you want to upload
* @param[in]	file_name			specifies file name to be uploaded to cloud
* @param[in]	upload_file_local_path		specifies local path of the file to be uploaded
* @param[in]	publish				specifies Allow to share file with no authentication
* @param[in]	request				specifies optional parameter
* @param[out]	transfer_request_id		specifies
* @param[out]	error				specifies error code
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
storage_error_code_t storage_adaptor_upload_file_async(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *parent_folder_storage_path,
						const char *file_name,
						const char *upload_file_local_path,
						const int publish,
						void *request,
						void *transfer_request_id,
						storage_adaptor_error_code_h *error);

/**
* @brief Downloads a file to local (Sync)
*
* @param[in]	plugin				specifies Storage Adaptor Plugin handle
* @param[in]	context				specifies Storage Adaptor Plugin Context handle
* @param[in]	parent_folder_storage_path	specifies folder path of file you want to download
* @param[in]	file_name			specifies file name to be downloaded to local
* @param[in]	download_file_local_path	specifies local path to download
* @param[in]	request				specifies optional parameter
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
storage_error_code_t storage_adaptor_download_file_sync(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *parent_folder_storage_path,
						const char *file_name,
						const char *download_file_local_path,
						void *request,
						storage_adaptor_error_code_h *error,
						void *response);

/**
* @brief Downloads a file to local (Async)
*
* @param[in]	plugin				specifies Storage Adaptor Plugin handle
* @param[in]	context				specifies Storage Adaptor Plugin Context handle
* @param[in]	parent_folder_storage_path	specifies folder path of file you want to download
* @param[in]	file_name			specifies file name to be downloaded to local
* @param[in]	download_file_local_path	specifies local path to download
* @param[in]	request				specifies optional parameter
* @param[out]	transfer_request_id		specifies
* @param[out]	error				specifies error code
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
storage_error_code_t storage_adaptor_download_file_async(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *parent_folder_storage_path,
						const char *file_name,
						const char *download_file_local_path,
						void *request,
						void *transfer_request_id,
						storage_adaptor_error_code_h *error);

/**
* @brief Downloads a thumbnail to local (Sync)
*
* @param[in]	plugin				specifies Storage Adaptor Plugin handle
* @param[in]	context				specifies Storage Adaptor Plugin Context handle
* @param[in]	parent_folder_storage_path	specifies folder path of file you want to download
* @param[in]	file_name			specifies file name to be downloaded to local
* @param[in]	download_file_local_path	specifies local path to download
* @param[in]	thumbnail_size			specifies thumbnail_size
* @param[in]	request				specifies optional parameter
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
storage_error_code_t storage_adaptor_download_thumbnail(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *folder_path,
						const char *file_name,
						const char *download_path,
						int thumbnail_size,
						void *request,
						storage_adaptor_error_code_h *error,
						void *response);

/**
* @brief Removes a file at cloud
*
* @param[in]	plugin				specifies Storage Adaptor Plugin handle
* @param[in]	context				specifies Storage Adaptor Plugin Context handle
* @param[in]	parent_folder_storage_path	specifies folder path of file you want to delete
* @param[in]	file_name			specifies file name to be deleted from cloud
* @param[in]	request				specifies optional parameter
* @param[out]	file_info			specifies Storage Adaptor File Info handle
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
storage_error_code_t storage_adaptor_delete_file(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *parent_folder_storage_path,
						const char *file_name,
						void *request,
						storage_adaptor_file_info_h *file_info,
						storage_adaptor_error_code_h *error,
						void *response);

/**
* @brief Sets metadata of file at cloud
*
* @param[in]	plugin				specifies Storage Adaptor Plugin handle
* @param[in]	context				specifies Storage Adaptor Plugin Context handle
* @param[in]	parent_folder_storage_path	specifies folder path of file you want to set meta data
* @param[in]	file_name			specifies file name to be updated meta data
* @param[in]	meta_data			specifies meta data (A pair of Key, Value)
* @param[in]	request				specifies optional parameter
* @param[out]	file_info			specifies Storage Adaptor File Info handle
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
storage_error_code_t storage_adaptor_set_meta(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *parent_folder_storage_path,
						const char *file_name,
						const void *meta_data,
						void *request,
						storage_adaptor_file_info_h *file_info,
						storage_adaptor_error_code_h *error,
						void *response);

/**
* @brief Gets metatdata of file at cloud
*
* @param[in]	plugin				specifies Storage Adaptor Plugin handle
* @param[in]	context				specifies Storage Adaptor Plugin Context handle
* @param[in]	parent_folder_storage_path	specifies folder path of file you want to get meta data
* @param[in]	file_name			specifies file name
* @param[in]	request				specifies optional parameter
* @param[out]	file_info			specifies Storage Adaptor File Info handle
* @param[out]	meta_data			specifies meta data (A pair of Key, Value)
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
storage_error_code_t storage_adaptor_get_meta(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *parent_folder_storage_path,
						const char *file_name,
						void *request,
						storage_adaptor_file_info_h *file_info,
						void **meta_data,
						storage_adaptor_error_code_h *error,
						void *response);

/**
* @brief Set up Multi Channel Upload
*
* @param[in]	plugin				specifies Storage Adaptor Plugin handle
* @param[in]	context				specifies Storage Adaptor Plugin Context handle
* @param[in]	parent_folder_storage_path	specifies folder path of file you want to upload
* @param[in]	file_name			specifies file name to be uploaded to cloud
* @param[in]	upload_file_local_path		specifies local path of the file to be uploaded
* @param[in]	chunk_size_byte			specifies size of chunk
* @param[in]	request				specifies optional parameter
* @param[out]	mupload_key			specifies Multi Channel Upload key
* @param[out]	chunk_count			specifies total number of chunks
* @param[out]	file_info			specifies Storage Adaptor File Info handle
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
storage_error_code_t storage_adaptor_start_mupload(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *parent_folder_storage_path,
						const char *file_name,
						const char *upload_file_local_path,
						const unsigned long long chunk_size_byte,
						void *request,
						char **mupload_key,
						int *chunk_count,
						storage_adaptor_file_info_h *file_info,
						storage_adaptor_error_code_h *error,
						void *response);

/**
* @brief Uploads a chunk to cloud
*
* @param[in]	plugin				specifies Storage Adaptor Plugin handle
* @param[in]	context				specifies Storage Adaptor Plugin Context handle
* @param[in]	mupload_key			specifies Multi Channel Upload key
* @param[in]	chunk_number			specifies number of chunk (Starting at 1)
* @param[in]	request				specifies optional parameter
* @param[out]	file_info			specifies Storage Adaptor File Info handle
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
storage_error_code_t storage_adaptor_upload_mupload(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *mupload_key,
						const int chunk_number,
						void *request,
						storage_adaptor_file_info_h *file_info,
						storage_adaptor_error_code_h *error,
						void *response);

/**
* @brief Ends Multi channel Upload
*
* @param[in]	plugin				specifies Storage Adaptor Plugin handle
* @param[in]	context				specifies Storage Adaptor Plugin Context handle
* @param[in]	mupload_key			specifies Multi Channel Upload key
* @param[in]	publish				specifies Allow to share file with no authentication
* @param[in]	request				specifies optional parameter
* @param[out]	file_info			specifies Storage Adaptor File Info handle
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
storage_error_code_t storage_adaptor_end_mupload(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *mupload_key,
						const int publish,
						void *request,
						storage_adaptor_file_info_h *file_info,
						storage_adaptor_error_code_h *error,
						void *response);

/**
* @brief Requests list of chunks uploaded
*
* @param[in]	plugin				specifies Storage Adaptor Plugin handle
* @param[in]	context				specifies Storage Adaptor Plugin Context handle
* @param[in]	mupload_key			specifies Multi Channel Upload key
* @param[in]	request				specifies optional parameter
* @param[out]	file_info_list			specifies Storage Adaptor File Info handle
* @param[out]	file_info_list_len		specifies length of the file_info_list
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
storage_error_code_t storage_adaptor_list_mupload(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *mupload_key,
						void *request,
						storage_adaptor_file_info_h **file_info_list,
						int *file_info_list_len,
						storage_adaptor_error_code_h *error,
						void *response);

/**
* @brief Cancels all operations
*
* @param[in]	plugin				specifies Storage Adaptor Plugin handle
* @param[in]	context				specifies Storage Adaptor Plugin Context handle
* @param[in]	mupload_key			specifies Multi Channel Upload key
* @param[in]	request				specifies optional parameter
* @param[out]	file_info			specifies Storage Adaptor File Info handle
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
storage_error_code_t storage_adaptor_cancel_mupload(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *mupload_key,
						void *request,
						storage_adaptor_file_info_h *file_info,
						storage_adaptor_error_code_h *error,
						void *response);

/**
* @brief Starts Transaction
*
* @param[in]	plugin				specifies Storage Adaptor Plugin handle
* @param[in]	context				specifies Storage Adaptor Plugin Context handle
* @param[in]	request				specifies optional parameter
* @param[out]	tx_key				specifies Transaction key
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
storage_error_code_t storage_adaptor_start_transaction(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						void *request,
						char **tx_key,
						storage_adaptor_error_code_h *error,
						void *response);

/**
* @brief Uploads a file
*
* @param[in]	plugin				specifies Storage Adaptor Plugin handle
* @param[in]	context				specifies Storage Adaptor Plugin Context handle
* @param[in]	tx_key				specifies Transaction key
* @param[in]	tx_seq				specifies Transaction sequesnce (Starting at 1)
* @param[in]	parent_folder_storage_path	specifies folder path of file you want to upload
* @param[in]	file_name			specifies file name to be uploaded to cloud
* @param[in]    upload_file_local_path          specifies local path of the file to be uploaded
* @param[in]	request				specifies optional parameter
* @param[out]	file_info			specifies Storage Adaptor File Info handle
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
storage_error_code_t storage_adaptor_upload_file_transaction(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *tx_key,
						const int tx_seq,
						const char *parent_folder_storage_path,
						const char *file_name,
						const char *upload_file_local_path,
						void *request,
						storage_adaptor_file_info_h *file_info,
						storage_adaptor_error_code_h *error,
						void *response);

/**
* @brief Updates a folder
*
* @param[in]	plugin				specifies Storage Adaptor Plugin handle
* @param[in]	context				specifies Storage Adaptor Plugin Context handle
* @param[in]	tx_key				specifies Transaction key
* @param[in]	tx_seq				specifies Transaction sequesnce (Starting at 1)
* @param[in]	parent_folder_storage_path	specifies folder path of folder you want to update
* @param[in]	folder_name			specifies folder name to be updated
* @param[in]	request				specifies optional parameter
* @param[out]	file_info			specifies Storage Adaptor File Info handle
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
storage_error_code_t storage_adaptor_set_dir_transaction(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *tx_key,
						const int tx_seq,
						const char *parent_folder_storage_path,
						const char *folder_name,
						void *request,
						storage_adaptor_file_info_h *file_info,
						storage_adaptor_error_code_h *error,
						void *response);

/**
* @brief Removes a file
*
* @param[in]	plugin				specifies Storage Adaptor Plugin handle
* @param[in]	context				specifies Storage Adaptor Plugin Context handle
* @param[in]	tx_key				specifies Transaction key
* @param[in]	tx_seq				specifies Transaction sequesnce (Starting at 1)
* @param[in]	parent_folder_storage_path	specifies folder path of file you want to delete
* @param[in]	file_name			specifies file name to be deleted from cloud
* @param[in]	request				specifies optional parameter
* @param[in]    upload_file_local_path          specifies local path of the file to be uploaded
* @param[out]	file_info			specifies Storage Adaptor File Info handle
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
storage_error_code_t storage_adaptor_delete_file_transaction(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *tx_key,
						const int tx_seq,
						const char *parent_folder_storage_path,
						const char *file_name,
						void *request,
						storage_adaptor_file_info_h *file_info,
						storage_adaptor_error_code_h *error,
						void *response);

/**
* @brief Removes a folder
*
* @param[in]	plugin				specifies Storage Adaptor Plugin handle
* @param[in]	context				specifies Storage Adaptor Plugin Context handle
* @param[in]	tx_key				specifies Transaction key
* @param[in]	tx_seq				specifies Transaction sequesnce (Starting at 1)
* @param[in]	parent_folder_storage_path	specifies folder path of folder you want to delete
* @param[in]	folder_name			specifies folder name to be deleted
* @param[in]	request				specifies optional parameter
* @param[out]	file_info			specifies Storage Adaptor File Info handle
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
storage_error_code_t storage_adaptor_delete_dir_transaction(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *tx_key,
						const int tx_seq,
						const char *parent_folder_storage_path,
						const char *folder_name,
						void *request,
						storage_adaptor_file_info_h *file_info,
						storage_adaptor_error_code_h *error,
						void *response);

/**
* @brief Ends Transaction
*
* @param[in]	plugin				specifies Storage Adaptor Plugin handle
* @param[in]	context				specifies Storage Adaptor Plugin Context handle
* @param[in]	tx_key				specifies Transaction key
* @param[in]	tx_count			specifies Transaction order count
* @param[in]	request				specifies optional parameter
* @param[out]	file_info_list			specifies Storage Adaptor File Info handle
* @param[out]	file_info_list_len		specifies length of the file_info_list
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
storage_error_code_t storage_adaptor_end_transaction(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *tx_key,
						const int tx_count,
						void *request,
						storage_adaptor_file_info_h **file_info_list,
						int *file_info_list_len,
						storage_adaptor_error_code_h *error,
						void *response);

/**
* @brief Requests Transaction list
*
* @param[in]	plugin				specifies Storage Adaptor Plugin handle
* @param[in]	context				specifies Storage Adaptor Plugin Context handle
* @param[in]	tx_key				specifies Transaction key
* @param[in]	request				specifies optional parameter
* @param[out]	file_info_list			specifies Storage Adaptor File Info handle
* @param[out]	file_info_list_len		specifies length of the file_info_list
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
storage_error_code_t storage_adaptor_list_transaction(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *tx_key,
						void *request,
						storage_adaptor_file_info_h **file_info_list,
						int *file_info_list_len,
						storage_adaptor_error_code_h *error,
						void *response);

/**
* @brief Cancels all transactions
*
* @param[in]	plugin				specifies Storage Adaptor Plugin handle
* @param[in]	context				specifies Storage Adaptor Plugin Context handle
* @param[in]	tx_key				specifies Transaction key
* @param[in]	request				specifies optional parameter
* @param[out]	file_info_list			specifies Storage Adaptor File Info handle
* @param[out]	file_info_list_len		specifies length of the file_info_list
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
storage_error_code_t storage_adaptor_cancel_transaction(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *tx_key,
						void *request,
						storage_adaptor_file_info_h **file_info_list,
						int *file_info_list_len,
						storage_adaptor_error_code_h *error,
						void *response);

/**
* @brief Uploads multiple files to cloud
*
* @param[in]	plugin				specifies Storage Adaptor Plugin handle
* @param[in]	context				specifies Storage Adaptor Plugin Context handle
* @param[in]	parent_folder_storage_path	specifies folder path of files you want to upload
* @param[in]	upload_file_local_path_list	specifies local path list of the files to be uploaded
* @param[in]	upload_list_len			specifies total number of files to be uploaded
* @param[in]	request				specifies optional parameter
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
storage_error_code_t storage_adaptor_multi_file_upload(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *parent_folder_storage_path,
						const char **upload_file_local_path_list,
						const int upload_list_len,
						void *request,
						storage_adaptor_error_code_h *error,
						void *response);

/**
* @brief Downloads multiple files in a folder
*
* @param[in]	plugin				specifies Storage Adaptor Plugin handle
* @param[in]	context				specifies Storage Adaptor Plugin Context handle
* @param[in]	parent_folder_storage_path	specifies folder path of files you want to download
* @param[in]	file_name_list			specifies file name list to be downloaded
* @param[in]	file_name_list_len		specifies total number of files to be downloaded
* @param[in]	download_folder_local_path	specifies local folder path to download files
* @param[in]	request				specifies optional parameter
* @param[out]	file_info_list			specifies Storage Adaptor File Info handle
* @param[out]	file_info_list_len		specifies length of the file_info_list
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
storage_error_code_t storage_adaptor_multi_file_download(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *parent_folder_storage_path,
						const char *file_name_list,
						const int file_name_list_len,
						const char *download_folder_local_path,
						void *request,
						storage_adaptor_file_info_h **file_info_list,
						int *file_info_list_len,
						storage_adaptor_error_code_h *error,
						void *response);

/**
* @brief Requests current server timestamp
*
* @param[in]	plugin				specifies Storage Adaptor Plugin handle
* @param[in]	context				specifies Storage Adaptor Plugin Context handle
* @param[in]	request				specifies optional parameter
* @param[out]	timestamp			specifies server timestamp
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
storage_error_code_t storage_adaptor_get_timestamp(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						void *request,
						unsigned long long *timestamp,
						storage_adaptor_error_code_h *error,
						void *response);

/**
* @brief Requests a file info by public token
*
* @param[in]	plugin				specifies Storage Adaptor Plugin handle
* @param[in]	context				specifies Storage Adaptor Plugin Context handle
* @param[in]	public_token			specifies token for Download, Get API
						(when terminal upload file and add publish=true parameter, ors return public_token)
* @param[in]	auth_code			specifies Authentication code for public APIs
* @param[in]	request				specifies optional parameter
* @param[out]	file_info			specifies Storage Adaptor File Info handle
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
storage_error_code_t storage_adaptor_get_file_info_by_public_token(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *public_token,
						const char *auth_code,
						void *request,
						storage_adaptor_file_info_h *file_info,
						storage_adaptor_error_code_h *error,
						void *response);

/**
* @brief Downloads a file by public token (Sync)
*
* @param[in]	plugin				specifies Storage Adaptor Plugin handle
* @param[in]	context				specifies Storage Adaptor Plugin Context handle
* @param[in]	public_token			specifies token for Download, Get API
						(when terminal upload file and add publish=true parameter, ors return public_token)
* @param[in]	auth_code			specifies Authentication code for public APIs
* @param[in]	download_file_local_path	specifies local path to download
* @param[in]	request				specifies optional parameter
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
storage_error_code_t storage_adaptor_download_file_sync_by_public_token(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *public_token,
						const char *auth_code,
						const char *download_file_local_path,
						void *request,
						storage_adaptor_error_code_h *error,
						void *response);

/**
* @brief Downloads a file by public token (Async)
*
* @param[in]	plugin				specifies Storage Adaptor Plugin handle
* @param[in]	context				specifies Storage Adaptor Plugin Context handle
* @param[in]	public_token			specifies token for Download, Get API
						(when terminal upload file and add publish=true parameter, ors return public_token)
* @param[in]	auth_code			specifies Authentication code for public APIs
* @param[in]	download_file_local_path	specifies local path to download
* @param[in]	request				specifies optional parameter
* @param[out]	transfer_request_id		specifies
* @param[out]	error				specifies error code
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
storage_error_code_t storage_adaptor_download_file_async_by_public_token(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *public_token,
						const char *auth_code,
						const char *download_file_local_path,
						void *request,
						void *transfer_request_id,
						storage_adaptor_error_code_h *error);

/**
* @brief Authenticates public auth code
*
* @param[in]	plugin				specifies Storage Adaptor Plugin handle
* @param[in]	context				specifies Storage Adaptor Plugin Context handle
* @param[in]	public_token			specifies token for Download, Get API
						(when terminal upload file and add publish=true parameter, ors return public_token)
* @param[in]	auth_code			specifies Authentication code for public APIs
* @param[in]	request				specifies optional parameter
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
storage_error_code_t storage_adaptor_auth_public_authcode_by_public_token(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *public_token,
						const char *auth_code,
						void *request,
						storage_adaptor_error_code_h *error,
						void *response);

/**
* @brief Removes multiple files in a folder
*
* @param[in]	plugin				specifies Storage Adaptor Plugin handle
* @param[in]	context				specifies Storage Adaptor Plugin Context handle
* @param[in]	parent_folder_storage_path	specifies folder path of files you want to delete
* @param[in]	file_name_list			specifies file name list to be deleted
* @param[in]	file_name_list_len		specifies total number of files to be deleted
* @param[in]	request				specifies optional parameter
* @param[out]	file_info_list			specifies Storage Adaptor File Info handle
* @param[out]	file_info_list_len		specifies length of the file_info_list
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
storage_error_code_t storage_adaptor_delete_multi_file_in_folder(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *parent_folder_storage_path,
						const char **file_name_list,
						const int file_name_list_len,
						void *request,
						storage_adaptor_file_info_h **file_info_list,
						int *file_info_list_len,
						storage_adaptor_error_code_h *error,
						void *response);

/**
* @brief Requests policy for upload
*
* @param[in]	plugin				specifies Storage Adaptor Plugin handle
* @param[in]	context				specifies Storage Adaptor Plugin Context handle
* @param[in]	request				specifies optional parameter
* @param[out]	allowed_extension		specifies
* @param[out]	allowed_extension_len		specifies length of allowed_extension
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
storage_error_code_t storage_adaptor_get_policy(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						void *request,
						char ***allowed_extension,
						int *allowed_extension_len,
						storage_adaptor_error_code_h *error,
						void *response);

/**
* @brief Requests quota of user
*
* @param[in]	plugin				specifies Storage Adaptor Plugin handle
* @param[in]	context				specifies Storage Adaptor Plugin Context handle
* @param[in]	request				specifies optional parameter
* @param[out]	total_usage			specifies total usage of user
* @param[out]	total_quota			specifies total quota of user
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
storage_error_code_t storage_adaptor_get_quota(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						void *request,
						unsigned long long *total_usage,
						unsigned long long *total_quota,
						storage_adaptor_error_code_h *error,
						void *response);

/**
* @brief Requests Redirect URL mapped with public token (Not yet supported)
*
* @param[in]	plugin				specifies Storage Adaptor Plugin handle
* @param[in]	context				specifies Storage Adaptor Plugin Context handle
* @param[in]	public_token			specifies token for Download, Get API
						(when terminal upload file and add publish=true parameter, ors return public_token)
* @param[in]	request				specifies optional parameter
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
storage_error_code_t storage_adaptor_redirect_url_by_public_token(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *public_token,
						void *request,
						storage_adaptor_error_code_h *error,
						void *response);

/**
* @brief Creates Upload URL (Not yet supported)
*
* @param[in]	plugin				specifies Storage Adaptor Plugin handle
* @param[in]	context				specifies Storage Adaptor Plugin Context handle
* @param[in]	parent_folder_storage_path	specifies folder path of files you want to upload
* @param[in]	file_name			specifies file name to be uploaded
* @param[in]	x_upload_content_length		specifies length of content
* @param[in]	request				specifies optional parameter
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
storage_error_code_t storage_adaptor_create_resuming_upload_url(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *parent_folder_storage_path,
						const char *file_name,
						const unsigned long long x_upload_content_length,
						void *request,
						storage_adaptor_error_code_h *error,
						void *response);

/**
* @brief Creates chunk Upload URL (Not yet supported)
*
* @param[in]	plugin				specifies Storage Adaptor Plugin handle
* @param[in]	context				specifies Storage Adaptor Plugin Context handle
* @param[in]	mupload_key			specifies Multi Channel Upload key
* @param[in]	chunk_number			specifies number of chunk (Starting at 1)
* @param[in]	x_upload_content_length		specifies length of content
* @param[in]	request				specifies optional parameter
* @param[out]	rupload_key			specifies Resuming Upload key
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
storage_error_code_t storage_adaptor_create_resuming_chunk_upload_url(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *mupload_key,
						const int chunk_number,
						const unsigned long long x_upload_content_length,
						void *request,
						char **rupload_key,
						storage_adaptor_error_code_h *error,
						void *response);

/**
* @brief Resumes Upload (Not yet supported)
*
* @param[in]	plugin				specifies Storage Adaptor Plugin handle
* @param[in]	context				specifies Storage Adaptor Plugin Context handle
* @param[in]	rupload_key			specifies Resuming Upload key
* @param[in]	content_range			specifies range of content
* @param[in]	content_length			specifies length of content
* @param[in]	request				specifies optional parameter
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
storage_error_code_t storage_adaptor_resuming_upload(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *rupload_key,
						const char *content_range,
						const unsigned long long content_length,
						void *request,
						storage_adaptor_error_code_h *error,
						void *response);

/**
* @brief Move a folder into destination folder
*
* @param[in]	plugin				specifies Storage Adaptor Plugin handle
* @param[in]	context				specifies Storage Adaptor Plugin Context handle
* @param[in]	parent_folder_storage_path	specifies parent folder path of folder you want to move
* @param[in]	folder_name			specifies folder name to be moved
* @param[in]	dest_parent_folder_storage_path	specifies new parent folder path
* @param[in]	new_folder_name			specifies new folder name
* @param[in]	request				specifies optional parameter
* @param[out]	file_info			specifies Storage Adaptor File Info handle
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
storage_error_code_t storage_adaptor_move_directory(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *parent_folder_storage_path,
						const char *folder_name,
						const char *dest_parent_folder_storage_path,
						const char *new_folder_name,
						void *request,
						storage_adaptor_file_info_h *file_info,
						storage_adaptor_error_code_h *error,
						void *response);

/**
* @brief Move a file into destination folder
*
* @param[in]	plugin				specifies Storage Adaptor Plugin handle
* @param[in]	context				specifies Storage Adaptor Plugin Context handle
* @param[in]	parent_folder_storage_path	specifies folder path of file you want to move
* @param[in]	file_name			specifies file name to be moved
* @param[in]	dest_parent_folder_storage_path	specifies new folder path
* @param[in]	new_file_name			specifies new file name
* @param[in]	request				specifies optional parameter
* @param[out]	file_info			specifies Storage Adaptor File Info handle
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
storage_error_code_t storage_adaptor_move_file(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						const char *parent_folder_storage_path,
						const char *file_name,
						const char *dest_parent_folder_storage_path,
						const char *new_file_name,
						void *request,
						storage_adaptor_file_info_h *file_info,
						storage_adaptor_error_code_h *error,
						void *response);

/**
* @brief Get progress of file transfer request
*
* @param[in]    plugin                          specifies Storage Adaptor Plugin handle
* @param[in]    context                         specifies Storage Adaptor Plugin Context handle
* @param[in]    transfer_request_id             specifies unique id for file transfer request
* @param[in]    request                         specifies optional parameter
* @param[out]   progress_size                   specifies current progress size
* @param[out]   total_size                      specifies total size to transfer
* @param[out]   error                           specifies error code
* @param[out]   response                        specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
storage_error_code_t storage_adaptor_get_transfer_progress(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						void *transfer_request_id,
						void *request,
						unsigned long long *progress_size_byte,
						unsigned long long *total_size_byte,
						storage_adaptor_error_code_h *error,
						void *response);

/**
* @brief Get state of file transfer request
*
* @param[in]    plugin                          specifies Storage Adaptor Plugin handle
* @param[in]    context                         specifies Storage Adaptor Plugin Context handle
* @param[in]    transfer_request_id             specifies unique id for file transfer request
* @param[in]    request                         specifies optional parameter
* @param[out]   state                          specifies current state of transfer request
* @param[out]   error                           specifies error code
* @param[out]   response                        specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
storage_error_code_t storage_adaptor_get_transfer_state(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						void *transfer_request_id,
						void *request,
						storage_adaptor_transfer_state_e *state,
						storage_adaptor_error_code_h *error,
						void *response);

/**
* @brief Set state of file transfer request
*
* @param[in]    plugin                          specifies Storage Adaptor Plugin handle
* @param[in]    context                         specifies Storage Adaptor Plugin Context handle
* @param[in]    transfer_request_id             specifies unique id for file transfer request
* @param[in]    state                          specifies state to set
* @param[in]    request                         specifies optional parameter
* @param[out]   error                           specifies error code
* @param[out]   response                        specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
storage_error_code_t storage_adaptor_set_transfer_state(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						void *transfer_request_id,
						storage_adaptor_transfer_state_e state,
						void *request,
						storage_adaptor_error_code_h *error,
						void *response);

/**
* @brief Requests root folder path
*
* @param[in]	plugin				specifies Storage Adaptor Plugin handle
* @param[in]	context				specifies Storage Adaptor Plugin Context handle
* @param[in]	request				specifies optional parameter
* @param[out]	root_folder_path	specifies root folder path
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return 0 on success, otherwise a positive error value
* @retval error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
EXPORT_API
storage_error_code_t storage_adaptor_get_root_folder_path(storage_adaptor_plugin_h plugin,
						storage_adaptor_plugin_context_h context,
						void *request,
						char **root_folder_path,
						storage_adaptor_error_code_h *error,
						void *response);

#ifdef __cplusplus
}
#endif

#endif /* __STORAGE_ADAPTOR_H__ */
