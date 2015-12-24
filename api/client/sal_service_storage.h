/*
 * Service Storage
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

#ifndef __SERVICE_STORAGE_H__
#define __SERVICE_STORAGE_H__

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#include "sal_service_adaptor.h"
#include "sal_service_task.h"

#define SERVICE_STORAGE_CLOUD_REMOVE_FILE_URI   "http://tizen.org/service-adaptor/storage/cloud/remove_file"
#define SERVICE_STORAGE_CLOUD_DOWNLOAD_FILE_URI "http://tizen.org/service-adaptor/storage/cloud/download_file"
#define SERVICE_STORAGE_CLOUD_UPLOAD_FILE_URI   "http://tizen.org/service-adaptor/storage/cloud/upload_file"
#define SERVICE_STORAGE_CLOUD_DOWNLOAD_FILE_THUMBNAIL_URI       "http://tizen.org/service-adaptor/storage/cloud/download_file_thumbnail"
#define SERVICE_STORAGE_CLOUD_GET_FILE_LIST_URI "http://tizen.org/service-adaptor/storage/cloud/get_file_list"

typedef struct _service_storage_cloud_file_s *service_storage_cloud_file_h;

typedef bool (*service_storage_cloud_file_cb)(int result, service_storage_cloud_file_h file, void *user_data);

/*==================================================================================================
                                         FUNCTION PROTOTYPES
==================================================================================================*/

int service_storage_cloud_file_create(service_plugin_h plugin, service_storage_cloud_file_h *file);
int service_storage_cloud_file_clone(service_storage_cloud_file_h src_file, service_storage_cloud_file_h *dst_file);

int service_storage_cloud_file_destroy(service_storage_cloud_file_h file);
int service_storage_cloud_file_set_callback(service_storage_cloud_file_h file, service_storage_cloud_file_cb callback, void *user_data);
int service_storage_cloud_file_unset_callback(service_storage_cloud_file_h file);
int service_storage_cloud_file_set_cloud_path(service_storage_cloud_file_h file, const char *cloud_path);
int service_storage_cloud_file_get_cloud_path(service_storage_cloud_file_h file, char **cloud_path);
int service_storage_cloud_file_set_local_path(service_storage_cloud_file_h file, const char *local_path);
int service_storage_cloud_file_get_local_path(service_storage_cloud_file_h file, char **local_path);
int service_storage_cloud_file_set_size(service_storage_cloud_file_h file, unsigned long long size);
int service_storage_cloud_file_get_size(service_storage_cloud_file_h file, unsigned long long *size);
int service_storage_cloud_file_set_operation(service_storage_cloud_file_h file, const char *operation);
int service_storage_cloud_file_get_operation(service_storage_cloud_file_h file, char **operation);
int service_storage_cloud_file_is_directory(service_storage_cloud_file_h file, bool *is_dir);
int service_storage_cloud_file_foreach_file(service_storage_cloud_file_h file, service_storage_cloud_file_cb callback, void *user_data);
int service_storage_cloud_file_create_task(service_storage_cloud_file_h file, service_task_h *task);
int service_storage_cloud_file_destroy_task(service_task_h task);

// 2.4 

/**
* @brief The handle of async task for storage service
* @see #service_storage_create_download_task()
* @see #service_storage_create_upload_task()
*/
typedef struct _service_storage_task_s *service_storage_task_h;

/**
* @brief The handle of file or directory in storage
* @see	#service_storage_file_clone()
* @see	#service_storage_file_destroy()
*/
typedef struct _service_storage_file_s *service_storage_file_h;

/**
* @brief The list handle of file or directory in storage
* @see	#service_storage_file_list_clone()
* @see	#service_storage_file_list_destroy()
*/
typedef struct _service_storage_file_list_s *service_storage_file_list_h;

/**
 * @brief Type of storage task
 */
typedef enum
{
	SERVICE_STORAGE_TASK_IN_PROGRESS	= 1,	/**< The task is progressing */
	SERVICE_STORAGE_TASK_COMPLETED		= 2,	/**< The task was completed */
	SERVICE_STORAGE_TASK_CANCELED		= 3,	/**< The task was canceled */
	SERVICE_STORAGE_TASK_FAILED		= 4,	/**< The task was failed */
} service_storage_task_state_e;

/**
* @brief Callback for changing state of storage task
*
* @param[in]	state		The state of storage task
* @param[in]	user_data	The user data passed from #service_storage_set_task_state_changed_cb()
* @return       void
* @see	#service_storage_unset_task_state_changed_cb()
* @pre	#service_storage_set_task_state_changed_cb() will invoke this callback.
*/
typedef void (*service_storage_task_state_cb)(service_storage_task_state_e state,
						void *user_data);

/**
* @brief Callback for progress of storage task
*
* @param[in]	progress	The progressed amount of storage task
* @param[in]	total		The total amount of storage task
* @param[in]	user_data	The user data passed from #service_storage_set_task_progress_cb()
* @return       void
* @see #service_storage_unset_task_progress_cb()
* @pre	#service_storage_set_task_progress_cb() will invoke this callback.
*/
typedef void (*service_storage_task_progress_cb)(unsigned long long progress,
						unsigned long long total,
						void *user_data);

/**
* @brief Callback for getting async storage operation result
*
* @param[in]	result		Result code for storage async operation (see #service_adaptor_error_e)
* @param[in]	user_data	Passed data from request function
* @remarks	If the @a result value is #SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED, error code and error message can be obtained using #service_adaptor_get_last_result() and #service_adaptor_get_last_error_message() method. Error codes and messages are described in Service Plugin.
* @remarks	The @a result values #SERVICE_ADAPTOR_ERROR_NONE Successful
* @remarks	The @a result values #SERVICE_ADAPTOR_ERROR_NO_DATA There is no files
* @remarks	The @a result values #SERVICE_ADAPTOR_ERROR_TIMED_OUT Timed out
* @remarks	The @a result values #SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED Failed in Plugin internal
* @remarks	The @a result values #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
* @see #service_adaptor_error_e
* @see service_storage_remove()
* @return       void
* @pre	#service_storage_remove() will invoke this callback.
*/
typedef void (*service_storage_result_cb)(int result,
						void *user_data);


/**
* @brief Callback for getting file list API
*
* @param[in]	result		Result code for #service_storage_get_file_list() (see #service_adaptor_error_e)
* @param[in]	list		The handle of file list
* @param[in]	user_data	Passed data from #service_storage_get_file_list()
* @remarks	If the @a result value is #SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED, error code and error message can be obtained using #service_adaptor_get_last_result() and #service_adaptor_get_last_error_message() method. Error codes and messages are described in Service Plugin.
* @remarks	The @a result values #SERVICE_ADAPTOR_ERROR_NONE Successful
* @remarks	The @a result values #SERVICE_ADAPTOR_ERROR_NO_DATA There is no files
* @remarks	The @a result values #SERVICE_ADAPTOR_ERROR_TIMED_OUT Timed out
* @remarks	The @a result values #SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED Failed in Plugin internal
* @remarks	The @a result values #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
* @see #service_adaptor_error_e
* @see #service_storage_file_list_h
* @return       void
* @pre	#service_storage_get_file_list() will invoke this callback.
*/
typedef void (*service_storage_file_list_cb)(int result,
						service_storage_file_list_h list,
						void *user_data);

/**
* @brief Callback for service_storage_file_list_foreach_file API
*
* @param[in]	file		The handle of file
* @param[in]	user_data	Passed data from #service_storage_file_list_foreach_file()
* @see #service_storage_file_h
* @return @c true to continue with the next iteration of the loop,
*         otherwise @c false to break out of the loop
* @pre	#service_storage_file_list_foreach_file() will invoke this callback.
*/
typedef bool (*service_storage_file_cb)(service_storage_file_h file,
						void *user_data);

/**
* @brief Gets file list from storage, asynchronously.
* @since_tizen 2.4
* @privlevel	public
* @privilege	%http://tizen.org/privilege/internet
*
* @param[in]	plugin		The handle for use Plugin APIs
* @param[in]	dir_path	The dir path (Physical path)
* @param[in]	callback	The callback for getting file list
* @param[in]	user_data	The user data to be passed to the callback function
* @remarks	For the @a dir_path, "/" means root path.
* @remarks	Reference details for <b>"Logical path"</b> and <b>"Physical path"</b> at @ref SERVICE_ADAPTOR_STORAGE_MODULE_OVERVIEW page
* @see		service_plugin_start()
* @see		service_storage_file_list_cb()
* @see		service_adaptor_get_last_result()
* @see		service_adaptor_get_last_error_message()
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_PERMISSION_DENIED Permission denied
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_STATE The handle's state is invalid
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
* @retval #SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE IPC failed with Service Adaptor Daemon
* @retval #SERVICE_ADAPTOR_ERROR_NOT_SUPPORTED Not supported API in this plugin
* @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
* @pre	API prerequires #service_plugin_start()
* @post	#service_storage_file_list_cb() will be invoked
*/
int service_storage_get_file_list(service_plugin_h plugin,
						const char *dir_path,
						service_storage_file_list_cb callback,
						void *user_data);

/**
* @brief Removes file or directory in storage
* @since_tizen 2.4
* @privlevel	public
* @privilege	%http://tizen.org/privilege/internet
*
* @param[in]	plugin		The handle for use Plugin APIs
* @param[in]	remove_path	The target file or directory for remove (Physical path)
* @param[in]	callback	The callback for getting result this operation
* @param[in]	user_data	The user data to be passed to the callback function
* @remarks	If the function returns #SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED, error code and error message can be obtained using #service_adaptor_get_last_result() and #service_adaptor_get_last_error_message() method. Error codes and messages are described in Service Plugin.
* @remarks	Reference details for <b>"Logical path"</b> and <b>"Physical path"</b> at @ref SERVICE_ADAPTOR_STORAGE_MODULE_OVERVIEW page
* @see		service_plugin_start()
* @see		service_storage_result_cb()
* @see		service_adaptor_get_last_result()
* @see		service_adaptor_get_last_error_message()
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_PERMISSION_DENIED Permission denied
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_STATE The handle's state is invalid
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
* @retval #SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE IPC failed with Service Adaptor Daemon
* @retval #SERVICE_ADAPTOR_ERROR_NOT_SUPPORTED Not supported API in this plugin
* @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
* @pre	API prerequires #service_plugin_start()
* @post	#service_storage_result_cb() will be invoked
*/
int service_storage_remove(service_plugin_h plugin,
						const char *remove_path,
						service_storage_result_cb callback,
						void *user_data);

/**
* @brief Creates storage task for upload file to storage
* @since_tizen 2.4
* @privlevel	public
* @privilege	%http://tizen.org/privilege/internet
*
* @param[in]	plugin		The handle for use Plugin APIs
* @param[in]	file_path	The upload file path in local (Logical path)
* @param[in]	upload_path	The upload target path in storage (Physical path)
* @param[out]	task		The handle of download task
* @remarks	@a task must be released memory using service_storage_destroy_task() when the task no longer run
* @remarks	If the function returns #SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED, error code and error message can be obtained using #service_adaptor_get_last_result() and #service_adaptor_get_last_error_message() method. Error codes and messages are described in Service Plugin.
* @remarks	Reference details for <b>"Logical path"</b> and <b>"Physical path"</b> at @ref SERVICE_ADAPTOR_STORAGE_MODULE_OVERVIEW page
* @remarks	http://tizen.org/privilege/mediastorage is needed if @a file_path is relevant to media storage.
* @remarks	http://tizen.org/privilege/externalstorage is needed if @a file_path is relevant to external storage.
* @see		service_plugin_start()
* @see		service_storage_destroy_task()
* @see		service_adaptor_get_last_result()
* @see		service_adaptor_get_last_error_message()
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_PERMISSION_DENIED Permission denied
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_STATE The handle's state is invalid
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
* @retval #SERVICE_ADAPTOR_ERROR_NO_DATA There is no files
* @retval #SERVICE_ADAPTOR_ERROR_TIMED_OUT Timed out
* @retval #SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE IPC failed with Service Adaptor Daemon
* @retval #SERVICE_ADAPTOR_ERROR_NOT_SUPPORTED Not supported API in this plugin
* @retval #SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED Failed in Plugin internal
* @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
* @pre	API prerequires #service_plugin_start()
*/
int service_storage_create_upload_task(service_plugin_h plugin,
						const char *file_path,
						const char *upload_path,
						service_storage_task_h *task);

/**
* @brief Creates storage task for download file from storage
* @since_tizen 2.4
* @privlevel	public
* @privilege	%http://tizen.org/privilege/internet
*
* @param[in]	plugin		The handle for use Plugin APIs
* @param[in]	storage_path	The source file path in storage (Physical path)
* @param[in]	download_path	The download path in local (Logical path)
* @param[out]	task		The handle of download task
* @remarks	@a task must be released memory using service_storage_destroy_task() when the task no longer run
* @remarks	If the function returns #SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED, error code and error message can be obtained using #service_adaptor_get_last_result() and #service_adaptor_get_last_error_message() method. Error codes and messages are described in Service Plugin.
* @remarks	Reference details for <b>"Logical path"</b> and <b>"Physical path"</b> at @ref SERVICE_ADAPTOR_STORAGE_MODULE_OVERVIEW page
* @remarks	http://tizen.org/privilege/mediastorage is needed if @a download_path is relevant to media storage.
* @remarks	http://tizen.org/privilege/externalstorage is needed if @a download_path is relevant to external storage.
* @see		service_plugin_start()
* @see		service_storage_destroy_task()
* @see		service_adaptor_get_last_result()
* @see		service_adaptor_get_last_error_message()
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_PERMISSION_DENIED Permission denied
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_STATE The handle's state is invalid
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
* @retval #SERVICE_ADAPTOR_ERROR_NO_DATA There is no files
* @retval #SERVICE_ADAPTOR_ERROR_TIMED_OUT Timed out
* @retval #SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE IPC failed with Service Adaptor Daemon
* @retval #SERVICE_ADAPTOR_ERROR_NOT_SUPPORTED Not supported API in this plugin
* @retval #SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED Failed in Plugin internal
* @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
* @pre	API prerequires #service_plugin_start()
*/
int service_storage_create_download_task(service_plugin_h plugin,
						const char *storage_path,
						const char *download_path,
						service_storage_task_h *task);

/**
* @brief Creates storage task for download thumbnail from storage
* @since_tizen 2.4
* @privlevel	public
* @privilege	%http://tizen.org/privilege/internet
*
* @param[in]	plugin			The handle for use Plugin APIs
* @param[in]	storage_path		The source file path in storage (Physical path)
* @param[in]	download_path		The download path in local (Logical path)
* @param[in]	thumbnail_size		The size <b>level</b> of thumbnail, the level is defined service plugin SPEC
* @param[out]	task		The handle of download task
* @remarks	If @a thumbnail_size is <b>0</b>, gets default size thumbnail, the default size must be defined plugin SPEC
* @remarks	If @a thumbnail_size is <b>-1</b>, gets minimum size thumbnail be supported plugin
* @remarks	If @a thumbnail_size is <b>-2</b>, gets maximum size thumbnail be supported plugin
* @remarks	@a task must be released memory using service_storage_destroy_task() when the task no longer run
* @remarks	If the function returns #SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED, error code and error message can be obtained using #service_adaptor_get_last_result() and #service_adaptor_get_last_error_message() method. Error codes and messages are described in Service Plugin.
* @remarks	Reference details for <b>"Logical path"</b> and <b>"Physical path"</b> at @ref SERVICE_ADAPTOR_STORAGE_MODULE_OVERVIEW page
* @remarks	http://tizen.org/privilege/mediastorage is needed if @a download_path is relevant to media storage.
* @remarks	http://tizen.org/privilege/externalstorage is needed if @a download_path is relevant to external storage.
* @see		service_plugin_start()
* @see		service_adaptor_get_last_result()
* @see		service_adaptor_get_last_error_message()
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_PERMISSION_DENIED Permission denied
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_STATE The handle's state is invalid
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
* @retval #SERVICE_ADAPTOR_ERROR_NO_DATA There is no thumbnail data
* @retval #SERVICE_ADAPTOR_ERROR_TIMED_OUT Timed out
* @retval #SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE IPC failed with Service Adaptor Daemon
* @retval #SERVICE_ADAPTOR_ERROR_NOT_SUPPORTED Not supported API in this plugin
* @retval #SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED Failed in Plugin internal
* @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
* @pre	API prerequires #service_plugin_start()
*/
int service_storage_create_download_thumbnail_task (service_plugin_h plugin,
						const char *storage_path,
						const char *download_path,
						int thumbnail_size,
						service_storage_task_h *task);

/**
* @brief Destroys storage task
* @since_tizen 2.4
*
* @param[in]	task		The handle of storage task
* @remarks	If the function returns #SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED, error code and error message can be obtained using #service_adaptor_get_last_result() and #service_adaptor_get_last_error_message() method. Error codes and messages are described in Service Plugin.
* @see		service_storage_create_download_task()
* @see		service_storage_create_upload_task()
* @see		service_adaptor_get_last_result()
* @see		service_adaptor_get_last_error_message()
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
* @retval #SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE IPC failed with Service Adaptor Daemon
* @retval #SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED Failed in Plugin internal
* @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
*/
int service_storage_destroy_task(service_storage_task_h task);

/**
* @brief Starts storage task, asynchronously.
* @since_tizen 2.4
*
* @param[in]	task	The handle of storage task
* @remarks	If the function returns #SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED, error code and error message can be obtained using #service_adaptor_get_last_result() and #service_adaptor_get_last_error_message() method. Error codes and messages are described in Service Plugin.
* @see		service_storage_create_upload_task()
* @see		service_storage_create_download_task()
* @see		service_storage_create_download_thumbnail_task()
* @see		service_adaptor_get_last_result()
* @see		service_adaptor_get_last_error_message()
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
* @retval #SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE IPC failed with Service Adaptor Daemon
* @retval #SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED Failed in Plugin internal
* @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
*/
int service_storage_start_task(service_storage_task_h task);

/**
* @brief Cancels storage task, asynchronously.
* @since_tizen 2.4
*
* @param[in]	task	The handle of storage task
* @remarks	@a task must be released memory using service_storage_destroy_task() when the task no longer run
* @remarks	If the function returns #SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED, error code and error message can be obtained using #service_adaptor_get_last_result() and #service_adaptor_get_last_error_message() method. Error codes and messages are described in Service Plugin.
* @see		service_storage_start_task()
* @see		service_adaptor_get_last_result()
* @see		service_adaptor_get_last_error_message()
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
* @retval #SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE IPC failed with Service Adaptor Daemon
* @retval #SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED Failed in Plugin internal
* @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
*/
int service_storage_cancel_task(service_storage_task_h task);

/**
* @brief Sets a callback function to be invoked when progress of the task running.
* @since_tizen 2.4
*
* @param[in]	task		The handle of storage task
* @param[in]	callback	The callback function to register
* @param[in]	user_data	The user data to be passed to the callback function
* @remarks	This function must be called before starting task (see #service_storage_start_task())
* @remarks	If the function returns #SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED, error code and error message can be obtained using #service_adaptor_get_last_result() and #service_adaptor_get_last_error_message() method. Error codes and messages are described in Service Plugin.
* @see		service_storage_start_task()
* @see		service_adaptor_get_last_result()
* @see		service_adaptor_get_last_error_message()
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
* @retval #SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE IPC failed with Service Adaptor Daemon
* @retval #SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED Failed in Plugin internal
* @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
* @post	#service_storage_task_progress_cb() will be invoked
*/
int service_storage_set_task_progress_cb(service_storage_task_h task,
						service_storage_task_progress_cb callback,
						void *user_data);

/**
* @brief Unsets the progress callback function.
* @since_tizen 2.4
*
* @param[in]	task		The handle of storage task
* @remarks	This function must be called before starting task (see #service_storage_start_task())
* @see		service_storage_start_task()
* @see		service_storage_set_task_progress_cb()
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
*/
int service_storage_unset_task_progress_cb(service_storage_task_h task);


/**
* @brief Sets a callback function to be invoked when change of the task running state.
* @since_tizen 2.4
*
* @param[in]	task		The handle of storage task
* @param[in]	callback	The callback function to register
* @param[in]	user_data	The user data to be passed to the callback function
* @remarks	This function must be called before starting task (see #service_storage_start_task())
* @remarks	If the function returns #SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED, error code and error message can be obtained using #service_adaptor_get_last_result() and #service_adaptor_get_last_error_message() method. Error codes and messages are described in Service Plugin.
* @see		service_storage_start_task()
* @see		service_adaptor_get_last_result()
* @see		service_adaptor_get_last_error_message()
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
* @retval #SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE IPC failed with Service Adaptor Daemon
* @retval #SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED Failed in Plugin internal
* @retval #SERVICE_ADAPTOR_ERROR_UNKNOWN Unknown error
* @post	#service_storage_task_state_cb() will be invoked
*/
int service_storage_set_task_state_changed_cb(service_storage_task_h task,
						service_storage_task_state_cb callback,
						void *user_data);

/**
* @brief Unsets the state changed callback function.
* @since_tizen 2.4
*
* @param[in]	task		The handle of storage task
* @remarks	This function must be called before starting task (see #service_storage_start_task())
* @see		service_storage_start_task()
* @see		service_storage_set_task_progress_cb()
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
*/
int service_storage_unset_task_state_changed_cb(service_storage_task_h task);

/**
* @brief Clones the file list handle
* @since_tizen 2.4
*
* @param[in]	src_list	The source handle
* @param[out]	dst_list	The destination handle
* @remarks	@a file must be released memory using service_storage_file_list_destroy() when you no longer needs this handle
* @see	service_storage_file_list_h
* @see	service_storage_file_list_destroy()
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
*/
int service_storage_file_list_clone(service_storage_file_list_h src_list,
						service_storage_file_list_h *dst_list);

/**
* @brief Destroys the file list handle
* @since_tizen 2.4
*
* @param[in]	list		The file list handle
* @remarks	It must be used for cloned file list handle
* @see	service_storage_file_list_h
* @see	service_storage_file_list_clone()
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
*/
int service_storage_file_list_destroy(service_storage_file_list_h list);

/**
* @brief Gets length of the file list handle
* @since_tizen 2.4
*
* @param[in]	list		The file list handle
* @param[out]	length		The length of the file list handle
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
*/
int service_storage_file_list_get_length(service_storage_file_list_h list,
						int *length);

/**
* @brief Foreach All of the file from file list
* @since_tizen 2.4
*
* @param[in]	list		The file list handle
* @param[in]	callback	The callback for foreach file
* @param[in]	user_data	Passed data to callback
* @see		#service_storage_file_cb
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
* @retval #SERVICE_ADAPTOR_ERROR_NO_DATA There is no files
*/
int service_storage_file_list_foreach_file(service_storage_file_list_h list,
						service_storage_file_cb callback,
						void *user_data);

/**
* @brief Clones the file handle
* @since_tizen 2.4
*
* @param[in]	src_file	The source handle
* @param[out]	dst_file	The destination handle
* @remarks	@a file must be released memory using service_storage_file_destroy() when you no longer needs this handle
* @see	#service_storage_file_h
* @see	#service_storage_file_destroy()
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
*/
int service_storage_file_clone(service_storage_file_h src_file,
						service_storage_file_h *dst_file);

/**
* @brief Destroys the file handle
* @details This function must be used for cloned file handle.
* @since_tizen 2.4
*
* @param[in]	file		The handle of file or directory in storage
* @see	service_storage_file_h
* @see	service_storage_file_clone()
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
*/
int service_storage_file_destroy(service_storage_file_h file);

/**
* @brief Gets whether directory or file for file handle
* @since_tizen 2.4
*
* @param[in]	file		The handle of file or directory in storage
* @param[out]	is_dir		true on directory, false on file
* @see	service_storage_file_h
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
*/
int service_storage_file_is_dir(service_storage_file_h file,
						bool *is_dir);

/**
* @brief Gets size of handle
* @since_tizen 2.4
*
* @param[in]	file		The handle of file or directory in storage
* @param[out]	size		The size of file (byte)
* @see	service_storage_file_h
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
*/
int service_storage_file_get_size(service_storage_file_h file,
						unsigned long long *size);

/**
* @brief Gets logical path from file handle
* @since_tizen 2.4
*
* @param[in]	file		The handle of file or directory in storage
* @param[out]	path		The logical path of file
* @remarks	@a path must be released using free()
* @remarks	Reference details for <b>"Logical path"</b> and <b>"Physical path"</b> at @ref SERVICE_ADAPTOR_STORAGE_MODULE_OVERVIEW page
* @see	#service_storage_file_h
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
* @retval #SERVICE_ADAPTOR_ERROR_NO_DATA There is no data
*/
int service_storage_file_get_logical_path(service_storage_file_h file,
						char **path);

/**
* @brief Gets physical path from file handle
* @since_tizen 2.4
*
* @param[in]	file		The handle of file or directory in storage
* @param[out]	path		The physical path of file
* @remarks	@a path must be released using free()
* @remarks	Reference details for <b>"Logical path"</b> and <b>"Physical path"</b> at @ref SERVICE_ADAPTOR_STORAGE_MODULE_OVERVIEW page
* @see	#service_storage_file_h
* @return 0 on success, otherwise a negative error value
* @retval #SERVICE_ADAPTOR_ERROR_NONE Successful
* @retval #SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER Invalid parameter
* @retval #SERVICE_ADAPTOR_ERROR_NO_DATA There is no data
*/
int service_storage_file_get_physical_path(service_storage_file_h file,
						char **path);

#ifdef __cplusplus
}
#endif

#endif /* __SERVICE_STORAGE_H__ */
