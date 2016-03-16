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

/******************************************************************************
 * File: service_adaptor_client_storage_private.h
 * Desc: Service Adaptor APIs
 * Created on: Feb, 2015
 * Auth: Jiwon Kim <jiwon177.kim@samsung.com>
 *
 *****************************************************************************/

/**
 *	@file		service_adaptor_client_storage_internal.h
 *	@brief		Defines interface of Service Adaptor's Storage
 *	@version	0.1
 */

#ifndef __TIZEN_SOCIAL_SERVICE_ADAPTOR_CLIENT_STORAGE_INTERNAL_H__
#define __TIZEN_SOCIAL_SERVICE_ADAPTOR_CLIENT_STORAGE_INTERNAL_H__

#include "service_adaptor_client.h"
#include "service_adaptor_client_storage.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Storage adaptor content type
 */
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

/**
* @brief Describes file information description
*/
struct _service_storage_file_s
{
	char    *plugin_name;		/**< specifies plugin name generated file_info */
	char    *object_id;		/**< specifies file object id be used in storage */
	char    *storage_path;		/**< specifies file path in storage */
	unsigned long long file_size;	/**< specifies file size (recomend byte)*/
	unsigned long long created_time;	/**< specifies timestamp */
	unsigned long long modified_time;	/**< specifies timestamp */
	int     file_info_index;	/**< specifies file info index (wide use; e.g : chunk upload, multi download)*/
	service_storage_content_type_e content_type; /**< specifies file content type (reference service_adaptor_file_content_type_e)  */

	service_storage_media_meta_s *media_meta;
	service_storage_cloud_meta_s *cloud_meta;
	char    *extra_file_info;		/**< specifies content name in metadata */
};


typedef struct _service_storage_file_s service_storage_file_s;

/**
* @brief The handle for File Description
*/
typedef struct _service_storage_file_s *service_storage_file_h;



/**
* @brief Callback for file upload and download API
*
* @param[in]	handle		specifies Service Adaptor handle. use this handle to get internal data
* @param[in]	file_handle	specifies file description
* @param[in]	transferred_size	specifies transferred size
* @param[in]	total_size	specifies total size
* @param[in]	error_code	specifies error code
* @param[in]	user_data	specifies user_data passed in API
* @return	void
* @pre	service_adaptor_request_channel_auth will invoke this callback.
* @see
*/
/*
typedef void(* service_adaptor_file_progress_cb)(service_adaptor_h handle,
						service_adaptor_file_h file_handle,
						unsigned long long transferred_size,
						unsigned long long total_size,
						service_adaptor_error_s *error_code,
						void *user_data);
*/
/**
* @brief Callback for file transfer completion
*
* @param[in]	handle		specifies Service Adaptor handle. use this handle to get internal data
* @param[in]	file_handle	specifies file description
* @param[in]	publish_url	specifies publish url for another user
* @param[in]	error_code	specifies error code
* @param[in]	user_data	specifies user_data passed in API
* @return	void
* @pre	service_adaptor_request_channel_auth will invoke this callback.
* @see
*/
/*
typedef void(* service_adaptor_file_transfer_completion_cb)(service_adaptor_h handle,
						service_adaptor_file_h file_handle,
						char *publish_url,
						service_adaptor_error_s *error_code,
						void *user_data);

*/

/**
* @brief Create memory for service_storage_file_h
* @since_tizen 2.4
*
* @param[in]	void
* @return service_storage_file_h
* @retval Allocated and filled default value file_info's pointer
*/
service_storage_file_h service_storage_create_file_info(void);

/**
* @brief Release memory for service_storage_file_h
* @since_tizen 2.4
*
* @param[in]	file_info	specifies Service Adaptor file information handle
* @return service_storage_file_h
* @retval released file_info's pointer
*/
int service_storage_unref_file_info(service_storage_file_h *file_info);



/**
* @brief Registers File Progress Listener
* @since_tizen 2.4
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	callback	the callback function to invoke
* @param[in]	user_data	specifies user_data passed in API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_ERROR_NONE if Successful
*/
/*
int service_adaptor_register_file_progress_listener(service_adaptor_h handle,
						service_adaptor_file_progress_cb callback,
						void *user_data);
*/
/**
* @brief Unregisters File Progress Listener
* @since_tizen 2.4
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	user_data	specifies user_data passed in API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_ERROR_NONE if Successful
*/
/*
int service_adaptor_unregister_file_progress_listener(service_adaptor_h handle,
						void *user_data);
*/
/**
* @brief Registers File Transfer Completion Listener
* @since_tizen 2.4
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	callback	the callback function to invoke
* @param[in]	user_data	specifies user_data passed in API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_ERROR_NONE if Successful
*/
/*
int service_adaptor_register_file_transfer_completion_listener(service_adaptor_h handle,
						service_adaptor_file_transfer_completion_cb callback,
						void *user_data);
*/
/**
* @brief Unregisters File Transfer Completion Listener
* @since_tizen 2.4
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	user_data	specifies user_data passed in API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_ERROR_NONE if Successful
*/
/*
int service_adaptor_unregister_file_transfer_completion_listener(service_adaptor_h handle,
						void *user_data);

*/
/**
* @brief Downloads a server file and writes it to local file (Async)
* @since_tizen 2.4
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	server_path	specifies server url of file to download
* @param[in]	download_path	specifies local path where we expect downloaded file to be written
* @param[out]	file_handle	specifies file description
* @param[out]	error_code	specifies error code
* @param[in]	user_data	specifies user_data (json) passed in Server API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_ERROR_NONE if Successful
*/
/*
int service_storage_download_file_async(service_adaptor_h handle,
						const char *storage_path,
						const char *local_path,
						service_adaptor_file_h *file_handle,
						service_adaptor_error_s **error_code,
						void *user_data);
*/


/**
* @brief Uploads a local file to server path (Async)
* @since_tizen 2.4
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	upload_path	specifies local path for upload
* @param[in]	server_path	specifies server url of file to download
* @param[out]	file_handle	specifies file description
* @param[out]	error_code	specifies error code
* @param[in]	user_data	specifies user_data (json) passed in Server API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_ERROR_NONE if Successful
*/
/*
int service_storage_upload_file_async(service_adaptor_h handle,
						const char *local_path,
						const char *storage_path,
						service_adaptor_file_h *file_handle,
						service_adaptor_error_s **error_code,
						void *user_data);
*/



/**
* @brief Downloads a server file and writes it to local file
* @since_tizen 2.4
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	publish_url	specifies publish url of file to download
* @param[in]	download_path	specifies local path where we expect downloaded file to be written
* @param[out]	error_code	specifies error code
* @param[in]	user_data	specifies user_data (json) passed in Server API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_ERROR_NONE if Successful
*/
/*
int service_storage_download_file_publish(service_adaptor_h handle,
						const char *publish_url,
						const char *local_path,
						service_adaptor_error_s **error_code,
						void *user_data);
*/
/**
* @brief Downloads a server file and writes it to local file (Async)
* @since_tizen 2.4
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	publish_url	specifies publish url of file to download
* @param[in]	download_path	specifies local path where we expect downloaded file to be written
* @param[out]	file_handle	specifies file description
* @param[out]	error_code	specifies error code
* @param[in]	user_data	specifies user_data (json) passed in Server API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_ERROR_NONE if Successful
*/
/*
int service_storage_download_file_publish_async(service_adaptor_h handle,
						const char *publish_url,
						const char *local_path,
						service_adaptor_file_h *file_handle,
						service_adaptor_error_s **error_code,
						void *user_data);
*/
/**
* @brief Uploads a local file to server path
* @since_tizen 2.4
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	upload_path	specifies local path for upload
* @param[in]	server_path	specifies server url of file to download
* @param[out]	publish_url	specifies publish url
* @param[out]	error_code	specifies error code
* @param[in]	user_data	specifies user_data (json) passed in Server API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_ERROR_NONE if Successful
*/

/*
int service_storage_upload_file_publish(service_adaptor_h handle,
						const char *local_path,
						const char *storage_path,
						char **publish_url,
						service_adaptor_error_s **error_code,
						void *user_data);
*/
/**
* @brief Uploads a local file to server path (Async)
* @since_tizen 2.4
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	upload_path	specifies local path for upload
* @param[in]	server_path	specifies server url of file to download
* @param[out]	file_handle	specifies file description
* @param[out]	error_code	specifies error code
* @param[in]	user_data	specifies user_data (json) passed in Server API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_ERROR_NONE if Successful
*/
/*
int service_storage_upload_file_publish_async(service_adaptor_h handle,
						const char *local_path,
						const char *storage_path,
						service_adaptor_file_h *file_handle,
						service_adaptor_error_s **error_code,
						void *user_data);
*/
/**
* @brief Downloads a thumbnail file and writes it to local file
* @since_tizen 2.4
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	publish_url	specifies publish url of file to download
* @param[in]	download_path	specifies local path where we expect downloaded file to be written
* @param[in]	size		specifies size of thumbnail
* @param[out]	error_code	specifies error code
* @param[in]	user_data	specifies user_data (json) passed in Server API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_ERROR_NONE if Successful
*/
/*
int service_storage_download_thumbnail_publish(service_adaptor_h handle,
						const char *publish_url,
						const char *local_path,
						storage_adaptor_thumbnail_size_e size,
						service_adaptor_error_s **error_code,
						void *user_data);
*/
/**
* @brief Requests File Status
* @since_tizen 2.4
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	file_handle	specifies file description
* @param[out]	status		specifies status of file transfer
* @param[out]	error_code	specifies error code
* @param[out]	server_data	specifies server_data (json) received from Server API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_ERROR_NONE if Successful
*/
/*
int service_storage_get_file_status(service_adaptor_h handle,
						service_adaptor_file_h file_handle,
						service_adaptor_file_status_s **status,
						service_adaptor_error_s **error_code,
						void **server_data);
*/

/**
* @brief Cancels File Transfer
* @since_tizen 2.4
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	file_handle	specifies file description
* @param[out]	error_code	specifies error code
* @param[out]	server_data	specifies server_data (json) received from Server API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_ERROR_NONE if Successful
*/
/*
int service_storage_cancel_file_transfer(service_adaptor_h handle,
						service_adaptor_file_h file_handle,
						service_adaptor_error_s **error_code,
						void **server_data);
*/
/**
* @brief Pause File Transfer
* @since_tizen 2.4
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	file_handle	specifies file description
* @param[out]	error_code	specifies error code
* @param[out]	server_data	specifies server_data (json) received from Server API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_ERROR_NONE if Successful
*/
/*
int service_storage_pause_file_transfer(service_adaptor_h handle,
						service_adaptor_file_h file_handle,
						service_adaptor_error_s **error_code,
						void **server_data);
*/
/**
* @brief Resume File Transfer
* @since_tizen 2.4
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	file_handle	specifies file description
* @param[out]	error_code	specifies error code
* @param[out]	server_data	specifies server_data (json) received from Server API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_ERROR_NONE if Successful
*/
/*
int service_storage_resume_file_transfer(service_adaptor_h handle,
						service_adaptor_file_h file_handle,
						service_adaptor_error_s **error_code,
						void **server_data);
*/




/**
* @brief Gets result after finishing storage task
* @since_tizen 2.4
*
* @param[in]	task		The handle of storage task
* @param[out]	result		The result information
* @remarks	This function must be called after complete to specfic task
* @remarks	When the <b>upload task</b> was finished, @a result is filled to #service_storage_file_h
* @remarks	If the function returns #SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED, error code and error message can be obtained using #service_adaptor_get_last_result() and #service_adaptor_get_last_error_message() method. Error codes and messages are described in Service Plugin.
* @see		service_storage_start_task()
* @see		service_adaptor_get_last_result()
* @see		service_adaptor_get_last_error_message()
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_PERMISSION_DENIED Permission denied
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
* @retval #SERVICE_ADAPTOR_ERROR_NO_DATA There is no files
* @retval #SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE IPC failed with Service Adaptor Daemon
* @retval #SERVICE_ADAPTOR_ERROR_NOT_SUPPORTED Not supported API in this plugin
* @retval #SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED Failed in Plugin internal
* @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
* @pre	API prerequires #service_plugin_start()
*/
int service_storage_get_task_result(service_storage_task_h task,
						void **result);

#ifdef __cplusplus
}
#endif /* __cpluscplus */
#endif /*__TIZEN_SOCIAL_SERVICE_ADAPTOR_CLIENT_STORAGE_INTERNAL_H__*/
