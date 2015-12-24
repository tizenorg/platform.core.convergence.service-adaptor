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

#ifndef __TIZEN_SOCIAL_SERVICE_PROVIDER_TYPE_H__
#define __TIZEN_SOCIAL_SERVICE_PROVIDER_TYPE_H__

#include <fcntl.h>
#include <unistd.h>
#include <service_adaptor_type.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup	SERVICE_PROVIDER_MODULE

 * @{
 */

/**
 * @brief Definition of app_control operation for providing control to Service Adaptor
 * @details The Service Provider must be able to receive app-control with this operation
 * @since_tizen 3.0
 *
 * @see service_provider_open_channel()
 */
#define APP_CONTROL_OPERATION_SERVICE_PROVIDER_CHANNEL	"http://tizen.org/appcontrol/operation/service-provider/channel"

/**
 * @brief	The object of user session
 * @details	The session is logical object that is bound to client who use plugin\n
 * There are multiple session can exist for a one client, because client can use different multiple property for a plugin.\n
 * Session is created by Service Adaptor and you can obtain this handle in a #storage_provider_s 's callback functions.
 * @since_tizen 3.0
 *
 * @remarks	If you need to divide callback's requester, you can use session handle in callback functions.\n
 */
typedef struct _service_provider_session_s *service_provider_session_h;

/**
 * @}
 */


/**
 * @addtogroup	STORAGE_PROVIDER_MODULE
 * @{
 */

/**
 * @brief The status information of file or directory
 * @details It includes file's timestamp, size, path and flag for checking directory\n
 * It designed similar to <b>struct stat</b> in <b><sys/stat.h></b>
 * @since_tizen 3.0
*/
typedef struct _storage_provider_stat_s *storage_provider_stat_h;

/**
 * @}
 */


/**
 * @addtogroup	SERVICE_PROVIDER_MODULE
 * @{
 */

/**
 * @brief Enumerations of choice about application state
 * @details	The application(Service Provider)'s life cycle can be changed by this values
 * @since_tizen 3.0
 *
 * @see	service_provider_channel_cb()
 */
typedef enum
{
	SERVICE_PROVIDER_RECOMMENDED_DEFAULT	= 0,	/**< The next life cycle will be decided to recommended state  */
	SERVICE_PROVIDER_APPLICATION_CONTINUE	= 1,	/**< The application will be alive */
	SERVICE_PROVIDER_APPLICATION_SHUTDOWN	= 2,	/**< The application will be terminated */
} service_provider_life_cycle_e;

/**
 * @brief Enumerations of state type for channel between Service Adaptor and Service Provider
 * @since_tizen 3.0
 *
 * @see	service_provider_channel_cb()
 */
typedef enum
{
	SERVICE_PROVIDER_CHANNEL_OPENED	= 1,	/**< The channel was opened */
	SERVICE_PROVIDER_CHANNEL_CLOSED	= 2,	/**< The channel was closed */
} service_provider_channel_state_e;

/**
 * @brief Enumerations of event type for client session
 * @since_tizen 3.0
 *
 * @see	service_provider_session_event_cb()
 */
typedef enum
{
    SERVICE_PROVIDER_SESSION_STARTED    = 1,	/**< The new session was started */
    SERVICE_PROVIDER_SESSION_STOPPED    = 2,	/**< The session was stopped */
} service_provider_session_event_e;

/**
 * @}
 */


/**
 * @addtogroup	STORAGE_PROVIDER_MODULE
 * @{
 */

/**
 * @brief Enumerations of flag for file open
 * @details	It will be set to #storage_provider_open_cb() 's \a flag parameter.\n
 * These flags follow \b POSIX. So, almost flags similar to \b POSIX.
 * Please refer to \b <sys/stat.h> and \b <fcntl.h>
 *
 * @since_tizen 3.0
 *
 * @remarks	STORAGE_PROVIDER_FLAG_O_THUMBNAIL is specific flag for Service Adaptor.
 *
 * @see	storage_provider_open_cb()
 */
typedef enum
{
	STORAGE_PROVIDER_FLAG_O_RDONLY		= 0x0000,	/**< Open for reading only */
	STORAGE_PROVIDER_FLAG_O_WRONLY		= 0x0001,	/**< Open for writing only */

	STORAGE_PROVIDER_FLAG_O_CREAT		= 0x0100,	/**< If the file not exists, the file shall be created */
	STORAGE_PROVIDER_FLAG_O_EXCL		= 0x0200,	/**< If O_CREAT and O_EXCL are set, open() shall fail if the file exists. */
	STORAGE_PROVIDER_FLAG_O_TRUNC		= 0x0800,	/**< If the file exists, its length shall be truncated to 0 */
	STORAGE_PROVIDER_FLAG_O_APPEND		= 0x1000,	/**< If set, the file offset shall be set to the end of the file prior to each write */
	STORAGE_PROVIDER_FLAG_O_THUMBNAIL	= 0xf000,	/**< If set, open thumbnail of file */
} storage_provider_flag_e;

/**
 * @}
 */


/**
 * @addtogroup	SERVICE_PROVIDER_MODULE

 * @{
 */

/**
 * @brief Callback for getting channel state
 * @details	Channel is important object for Service Provider.\n
 * All of the communication is consisted of Channel.\n
 * \n
 * <b>Channel opened</b> : \a state is #SERVICE_PROVIDER_CHANNEL_OPENED.\n
 * "Channel opened" means starting assignment of the Service Provider.\n
 * \li If function \b returns #SERVICE_PROVIDER_APPLICATION_CONTINUE, working flow will going successfully. (default)\n
 * \li But, function \b returns #SERVICE_PROVIDER_APPLICATION_SHUTDOWN, the channel will be closed and application will be \b terminated.\n
 * \li #SERVICE_PROVIDER_RECOMMENDED_DEFAULT has same logic with #SERVICE_PROVIDER_APPLICATION_CONTINUE \n
 *
 * <b>Channel closed</b> : \a state is #SERVICE_PROVIDER_CHANNEL_CLOSED.\n
 * Otherwise, "Channel closed" means that Service Adaptor doesn't need Service Provider now.\n
 * \li If function \b returns #SERVICE_PROVIDER_APPLICATION_SHUTDOWN, application will be \b terminated. (default)\n
 * \li But, function \b returns #SERVICE_PROVIDER_APPLICATION_CONTINUE, application keep on launch state. (Not recomended)\n
 * \li #SERVICE_PROVIDER_RECOMMENDED_DEFAULT has same logic with #SERVICE_PROVIDER_APPLICATION_SHUTDOWN \n
 *
 * @since_tizen 3.0
 *
 * @param[in]	state		The state type of channel
 * @param[in]	user_data   The user data passed from #service_provider_open_channel()
 *
 * @remarks	For resource efficiency, recommends to terminate provider application.
 * @remarks	Application can be terminated after this callback.
 * @remarks	Recommend return \c true, for efficiency of managing.
 *
 * @return	#service_provider_life_cycle_e must be returned.
 * @retval	#SERVICE_PROVIDER_RECOMMENDED_DEFAULT	The next life cycle will be decided to recommended state
 * @retval	#SERVICE_PROVIDER_APPLICATION_CONTINUE	The application will be alive
 * @retval	#SERVICE_PROVIDER_APPLICATION_SHUTDOWN	The application will be terminated
 *
 * @see #service_provider_channel_state_e
 * @pre #service_provider_open_channel() will invoke this callback.
 *
 * @code
 * int __channel_state_cb(service_provider_channel_state_e state, void *user_data)
 * {
 *     if (state == SERVICE_PROVIDER_CHANNEL_OPENED) {
 *
 *         // if provider state is fine
 *         if (__check_internal_state_fine()) {
 *
 *             // keep on launch state and start provider (recommended)
 *             // same effect with return SERVICE_PROVIDER_APPLICATION_CONTINUE
 *             return SERVICE_PROVIDER_RECOMMENDED_DEFAULT;
 *         } else {  // if provider state is bad
 *
 *             // application will be terminated
 *             return SERVICE_PROVIDER_APPLICATION_SHUTDOWN;
 *         }
 *
 *     } else if (state == SERVICE_PROVIDER_CHANNEL_CLOSED) {
 *
 *         // if provider need to remain task
 *         if (__check_remain_task_exist()) {
 *
 *             // keep on launch state and do remain work
 *             return SERVICE_PROVIDER_APPLICATION_CONTINUE;
 *         } else {  // if provider can be terminated
 *
 *             // application will be terminated (recommended)
 *             // same effect with return SERVICE_PROVIDER_APPLICATION_SHUTDOWN
 *             return SERVICE_PROVIDER_RECOMMENDED_DEFAULT;
 *         }
 *
 *     }
 *     // return Recommend default
 *     return SERVICE_PROVIDER_RECOMMENDED_DEFAULT;
 * }
 *
 * @endcode
 */
typedef int (*service_provider_channel_cb)(service_provider_channel_state_e state, void *user_data);

/**
 * @brief Callback for getting session event
 * @details	If Provider needs to check property's validation, recommends in this callback.\n
 * \n
 * If session \a event flag is "started", maybe someone called #service_plugin_start().\n
 * Otherwise, session \a event flag is "stopped", maybe someone called #service_plugin_stop().\n
 * So in this callback, Service Provider response or prepare to client request.\n
 * \n
 * But sometimes, session is opened/closed by management policy of Service Adaptor (without client request).
 * @since_tizen 3.0
 *
 * @param[in]	session		The handle of session
 * @param[in]	event		The event type of session
 * @param[in]	user_data   The user data passed from #service_provider_set_session_event_cb()
 *
 * @see #service_provider_session_event_e
 * @see #service_provider_set_session_event_cb()
*/
typedef void (*service_provider_session_event_cb)(service_provider_session_h session, service_provider_session_event_e event, void *user_data);

/**
 * @brief Callback for interating property
 * @since_tizen 3.0
 *
 * @param[in]	session		The handle of session information
 * @param[in]	key			The key of property
 * @param[in]	value		The value of property
 * @param[in]	user_data	Passed data from request function
 *
 * @return @c true to continue with the next iteration of the loop,
 *         otherwise @c false to break out of the loop
 * @pre  service_provider_foreach_session_property() will invoke this callback.
 */
typedef bool (*service_provider_session_property_cb)(service_provider_session_h session, const char *key, const char *value, void *user_data);

/**
 * @}
 */


/**
 * @addtogroup	STORAGE_PROVIDER_MODULE
 * @{
 */

/**
 * @brief	Initialize to perform operation likes read or write
 * @details	The provider can prepare to storage operation.\n
 * As a case, provider can creating libcurl handle \b or creating socket \b or checking device network status \b or allocating memory space \b or etc... various of initialization logic can be performed at this callback.\n
 * It is fair with #storage_provider_close_cb(). Close callback will be invoked after completing task.\n
 * \n
 * This API design is based POSIX API. Please refer to 'open()'.\n
 * But, the target of open must be a file on storage.
 * @since_tizen 3.0
 *
 * @param[in]	session		Handle of user session
 * @param[in]	fd			File descriptor for a file
 * @param[in]	path		File path on storage
 * @param[in]	flags		Flags are masked multiply
 * @param[in]	options		Optional flags be depended on \a flags \n
 * This parameter is ignored unless the \a flags is not masked #STORAGE_PROVIDER_FLAG_O_WRONLY or #STORAGE_PROVIDER_FLAG_O_THUMBNAIL.
 * @param[in]	timeout		Operation time out (millisecond)
 * @param[in]	user_data	Passed data from #service_provider_set_storage_provider()
 *
 * @remarks	Callback must be finished in timeout.
 * @remarks	Callback will be invoked on <b>additional thread</b>(Not main thread).
 * @remarks	\a fd is unique identifier for a task, and it is kept until finishing task. In same task logic, fd of open_cb, read_cb(write_cb), close_cb  are same.\n
 * \n
 * @remarks	\a flags can be masked multiple flag enums. And the \b enum is defined to #storage_provider_flag_e \n
 * \n
 * @remarks	If #STORAGE_PROVIDER_FLAG_O_WRONLY is set to \a flags (without #STORAGE_PROVIDER_FLAG_O_THUMBNAIL), \a options is <b>access mode</b> \n
 * Supporting bellow <b>access mode</b> and it can be masked multiple mode using 'bit and' for a parameter \b \a options.
 * \li \c \b S_IRWXU 00700 user (file owner) has read, write and execute permission
 * \li \c \b S_IRUSR 00400 user has read permission
 * \li \c \b S_IWUSR 00200 user has write permission
 * \li \c \b S_IXUSR 00100 user has execute permission
 * \li \c \b S_IRWXG 00070 group has read, write and execute permission
 * \li \c \b S_IRGRP 00040 group has read permission
 * \li \c \b S_IWGRP 00020 group has write permission
 * \li \c \b S_IXGRP 00010 group has execute permission
 * \li \c \b S_IRWXO 00007 others have read, write and execute permission
 * \li \c \b S_IROTH 00004 others have read permission
 * \li \c \b S_IWOTH 00002 others have write permission
 * \li \c \b S_IXOTH 00001 others have execute permission
 * \li Upper constant is defined in \b <fcntl.h>
 * \li Sometimes the access right for \b group and \b others means \b share
 * \n
 * @remarks	If #STORAGE_PROVIDER_FLAG_O_THUMBNAIL is set to \a flags, \a options is <b>thumbnail size</b> \n
 * Supporting bellow <b>thumbnail size</b> and it releates with service_storage_create_download_thumbnail_task()
 * \li \c \b 0	default size
 * \li \c \b -1	minimum size
 * \li \c \b -2	maximum size
 * \n
 * @remarks	If not be set #STORAGE_PROVIDER_FLAG_O_WRONLY or #STORAGE_PROVIDER_FLAG_O_THUMBNAIL, \a options is 0 (zero, not valid).\n
 * \n
 * @remarks	Sometimes authentication is vitiated by user request or expiring or some policy, then return #SERVICE_ADAPTOR_ERROR_NOT_AUTHORIZED
 * @remarks	Before returnning #SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED, you should call #service_provider_set_last_error().
 *
 * @return 0 on success, otherwise a negative error value
 * @retval	#SERVICE_ADAPTOR_ERROR_NONE					Successful
 * @retval	#SERVICE_ADAPTOR_ERROR_NOT_SUPPORTED		Not supported operation
 * @retval	#SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval	#SERVICE_ADAPTOR_ERROR_PERMISSION_DENIED	Permission denied
 * @retval	#SERVICE_ADAPTOR_ERROR_OUT_OF_MEMORY		Out of memory
 * @retval	#SERVICE_ADAPTOR_ERROR_NETWORK_UNREACHABLE	Network is unreachable
 * @retval	#SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED		Failed by internal plugin issue
 * @retval	#SERVICE_ADAPTOR_ERROR_NOT_AUTHORIZED		Need authorized
 * @retval	#SERVICE_ADAPTOR_ERROR_UNKNOWN				Unknown error
 *
 * @see	storage_provider_flag_e
 * @see	storage_provider_read_cb()
 * @see	storage_provider_write_cb()
 * @see	storage_provider_close_cb()
 * @see service_provider_set_last_error()
 *
 * @pre	#service_provider_set_storage_provider() must be called in app_create_cb()
 * @pre	#service_provider_open_channel() must be called in app_control_cb()
 */
typedef int (*storage_provider_open_cb)(service_provider_session_h session,
						int fd,
						const char *path,
						int flags,
						int options,
						int timeout,
						void *user_data);

/**
 * @brief Read data from descriptor of storage file on storage
 * @details	Provider should try to read data as much as buf_size, but sometimes it can not read enough.
 * Then, just assign byte to "result" as amount of read.
 * \n
 * This API design is based POSIX API. Please refer to 'read()'.\n
 * @since_tizen 3.0
 *
 * @param[in]	session		Handle of user session
 * @param[in]	fd			File descriptor for a file
 * @param[in,out] buffer	Buffer for getting data
 * @param[in]	buf_size	Size of buffer
 * @param[in]	timeout		Operation time out (millisecond)
 * @param[out]	result		On success, the number of bytes read is returned (zero indicates end of file)
 * @param[in]	user_data	Passed data from #service_provider_set_storage_provider()
 *
 * @remarks	Callback must be finished in timeout.
 * @remarks	Callback will be invoked on <b>additional thread</b>(Not main thread).
 * @remarks	\a buffer is already allocated memory space as \a buf_size.\n
 * You can just write data to \a buffer.\n
 * \n
 * @remarks	\a result must be input likes bellow
 * \li \c \b 0	zero indicates end of file.
 * \li \c <b>positive value</b>	the number of bytes read.
 * \li \c <b>negative value</b>	can be assigned I/O error (used by POSIX 'read()').
 * \n
 * @remarks	Sometimes authentication is vitiated by user request or expiring or some policy, then return #SERVICE_ADAPTOR_ERROR_NOT_AUTHORIZED
 * @remarks	Before returnning #SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED, you should call #service_provider_set_last_error().
 *
 * @return 0 on success, otherwise a negative error value
 * @retval	#SERVICE_ADAPTOR_ERROR_NONE					Successful
 * @retval	#SERVICE_ADAPTOR_ERROR_PERMISSION_DENIED	Permission denied
 * @retval	#SERVICE_ADAPTOR_ERROR_OUT_OF_MEMORY		Out of memory
 * @retval	#SERVICE_ADAPTOR_ERROR_NETWORK_UNREACHABLE	Network is unreachable
 * @retval	#SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED		Failed by internal plugin issue
 * @retval	#SERVICE_ADAPTOR_ERROR_NOT_AUTHORIZED		Need authorized
 * @retval	#SERVICE_ADAPTOR_ERROR_UNKNOWN				Unknown error
 *
 * @see	storage_provider_open_cb()
 * @see	storage_provider_close_cb()
 * @see service_provider_set_last_error()
 *
 * @pre	#service_provider_set_storage_provider() must be called in app_create_cb()
 * @pre	#service_provider_open_channel() must be called in app_control_cb()
 */
typedef int (*storage_provider_read_cb)(service_provider_session_h session,
						int fd,
						char *buffer,
						int buf_size,
						int timeout,
						int *result,
						void *user_data);

/**
 * @brief Write data to descriptor of storage file on storage
 * @details	Provider should try to write data as much as buf_size, but sometimes it can not write enough.
 * Then, just assign byte to "result" as amount of write.\n
 * \n
 * This API design is based POSIX API. Please refer to 'write()'.\n
 * @since_tizen 3.0
 *
 * @param[in]	session		Handle of user session
 * @param[in]	fd			File descriptor for a file
 * @param[in]	buffer		Buffer for writting data
 * @param[in]	buf_size	Size of buffer
 * @param[in]	timeout		Operation time out (millisecond)
 * @param[out]	result		On success, the number of bytes written is returned (zero indicates nothing was written)
 * @param[in]	user_data	Passed data from #service_provider_set_storage_provider()
 *
 * @remarks	Callback must be finished in timeout.
 * @remarks	Callback will be invoked on <b>additional thread</b>(Not main thread).
 * @remarks	\a result must be input likes bellow
 * \li \c \b 0	zero indicates nothing was written.
 * \li \c <b>positive value</b>	the number of bytes written.
 * \li \c <b>negative value</b>	can be assigned I/O error (used by POSIX 'write()').
 * \n
 * @remarks	Sometimes authentication is vitiated by user request or expiring or some policy, then return #SERVICE_ADAPTOR_ERROR_NOT_AUTHORIZED
 * @remarks	Before returnning #SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED, you should call #service_provider_set_last_error().
 *
 * @return 0 on success, otherwise a negative error value
 * @retval	#SERVICE_ADAPTOR_ERROR_NONE					Successful
 * @retval	#SERVICE_ADAPTOR_ERROR_PERMISSION_DENIED	Permission denied
 * @retval	#SERVICE_ADAPTOR_ERROR_OUT_OF_MEMORY		Out of memory
 * @retval	#SERVICE_ADAPTOR_ERROR_NETWORK_UNREACHABLE	Network is unreachable
 * @retval	#SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED		Failed by internal plugin issue
 * @retval	#SERVICE_ADAPTOR_ERROR_NOT_AUTHORIZED		Need authorized
 * @retval	#SERVICE_ADAPTOR_ERROR_UNKNOWN				Unknown error
 *
 * @see	storage_provider_open_cb()
 * @see	storage_provider_close_cb()
 * @see service_provider_set_last_error()
 *
 * @pre	#service_provider_set_storage_provider() must be called in app_create_cb()
 * @pre	#service_provider_open_channel() must be called in app_control_cb()
 */
typedef int (*storage_provider_write_cb)(service_provider_session_h session,
						int fd,
						char *buffer,
						int buf_size,
						int timeout,
						int *result,
						void *user_data);

/**
 * @brief Synchronize a file's in-core state with local device from fd
 * @details	Provider transfers ("flushes") all modified in-core data of (i.e., modified buffer cache pages for) the file referred to by the file descriptor fd to the disk device (or other permanent storage device).\n
 * \n
 * This API design is based POSIX API. Please refer to 'fsync()'.\n
 * @since_tizen 3.0
 *
 * @param[in]	session		Handle of user session
 * @param[in]	fd			File descriptor for a file
 * @param[in]	timeout		Operation time out (millisecond)
 * @param[in]	user_data	Passed data from #service_provider_set_storage_provider()
 *
 * @remarks	Callback must be finished in timeout.
 * @remarks	Callback will be invoked on <b>additional thread</b>(Not main thread).
 * @remarks	Sometimes authentication is vitiated by user request or expiring or some policy, then return #SERVICE_ADAPTOR_ERROR_NOT_AUTHORIZED
 * @remarks	Before returnning #SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED, you should call #service_provider_set_last_error().
 *
 * @return 0 on success, otherwise a negative error value
 * @retval	#SERVICE_ADAPTOR_ERROR_NONE					Successful
 * @retval	#SERVICE_ADAPTOR_ERROR_PERMISSION_DENIED	Permission denied
 * @retval	#SERVICE_ADAPTOR_ERROR_OUT_OF_MEMORY		Out of memory
 * @retval	#SERVICE_ADAPTOR_ERROR_NETWORK_UNREACHABLE	Network is unreachable
 * @retval	#SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED		Failed by internal plugin issue
 * @retval	#SERVICE_ADAPTOR_ERROR_NOT_AUTHORIZED		Need authorized
 * @retval	#SERVICE_ADAPTOR_ERROR_UNKNOWN				Unknown error
 *
 * @see	storage_provider_open_cb()
 * @see	storage_provider_read_cb()
 * @see	storage_provider_write_cb()
 * @see	storage_provider_close_cb()
 * @see service_provider_set_last_error()
 *
 * @pre	#service_provider_set_storage_provider() must be called in app_create_cb()
 * @pre	#service_provider_open_channel() must be called in app_control_cb()
 */
typedef int (*storage_provider_fsync_cb)(service_provider_session_h session,
						int fd,
						int timeout,
						void *user_data);

/**
 * @brief	Finalize to perform operation likes read or write
 * @details	The provider can finalize to storage operation.\n
 * As a case, provider can releasing libcurl handle \b or closing socket \b or or releasing memory space \b or etc... various of finalization logic can be performed at this callback.\n
 * It is fair with #storage_provider_open_cb(). Open callback must be invoked before this.\n
 * \n
 * This API design is based POSIX API. Please refer to 'close()'.\n
 * But, the target of close must be a file on storage.
 * @since_tizen 3.0
 *
 * @param[in]	session		Handle of user session
 * @param[in]	fd			File descriptor for a file
 * @param[in]	timeout		Operation time out (millisecond)
 * @param[in]	user_data	Passed data from #service_provider_set_storage_provider()
 *
 * @remarks	Callback must be finished in timeout.
 * @remarks	Callback will be invoked on <b>additional thread</b>(Not main thread).
 * @remarks	Sometimes authentication is vitiated by user request or expiring or some policy, then return #SERVICE_ADAPTOR_ERROR_NOT_AUTHORIZED
 * @remarks	Before returnning #SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED, you should call #service_provider_set_last_error().
 *
 * @return 0 on success, otherwise a negative error value
 * @retval	#SERVICE_ADAPTOR_ERROR_NONE					Successful
 * @retval	#SERVICE_ADAPTOR_ERROR_PERMISSION_DENIED	Permission denied
 * @retval	#SERVICE_ADAPTOR_ERROR_OUT_OF_MEMORY		Out of memory
 * @retval	#SERVICE_ADAPTOR_ERROR_NETWORK_UNREACHABLE	Network is unreachable
 * @retval	#SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED		Failed by internal plugin issue
 * @retval	#SERVICE_ADAPTOR_ERROR_NOT_AUTHORIZED		Need authorized
 * @retval	#SERVICE_ADAPTOR_ERROR_UNKNOWN				Unknown error
 *
 * @see	storage_provider_open_cb()
 * @see	storage_provider_read_cb()
 * @see	storage_provider_write_cb()
 * @see service_provider_set_last_error()
 *
 * @pre	#service_provider_set_storage_provider() must be called in app_create_cb()
 * @pre	#service_provider_open_channel() must be called in app_control_cb()
 */
typedef int (*storage_provider_close_cb)(service_provider_session_h session,
						int fd,
						int timeout,
						void *user_data);

/**
 * @brief Removes file or (empty) directory on storage
 * @details	This API design is based POSIX API. Please refer to 'remove()'.\n
 * But, the target of remove must be a file or directory on storage.
 * @since_tizen 3.0
 *
 * @param[in]	session		Handle of user session
 * @param[in]	path		File path on storage
 * @param[in]	timeout		Operation time out (millisecond)
 * @param[in]	user_data	Passed data from #service_provider_set_storage_provider()
 *
 * @remarks	Callback must be finished in timeout.
 * @remarks	Callback will be invoked on <b>additional thread</b>(Not main thread).
 * @remarks	Sometimes authentication is vitiated by user request or expiring or some policy, then return #SERVICE_ADAPTOR_ERROR_NOT_AUTHORIZED
 * @remarks	Before returnning #SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED, you should call #service_provider_set_last_error().
 *
 * @return 0 on success, otherwise a negative error value
 * @retval	#SERVICE_ADAPTOR_ERROR_NONE					Successful
 * @retval	#SERVICE_ADAPTOR_ERROR_PERMISSION_DENIED	Permission denied
 * @retval	#SERVICE_ADAPTOR_ERROR_OUT_OF_MEMORY		Out of memory
 * @retval	#SERVICE_ADAPTOR_ERROR_NETWORK_UNREACHABLE	Network is unreachable
 * @retval	#SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED		Failed by internal plugin issue
 * @retval	#SERVICE_ADAPTOR_ERROR_NOT_AUTHORIZED		Need authorized
 * @retval	#SERVICE_ADAPTOR_ERROR_UNKNOWN				Unknown error
 *
 * @see service_provider_set_last_error()
 *
 * @pre	#service_provider_set_storage_provider() must be called in app_create_cb()
 * @pre	#service_provider_open_channel() must be called in app_control_cb()
 */
typedef int (*storage_provider_remove_cb)(service_provider_session_h session,
						const char *path,
						int timeout,
						void *user_data);

/**
 * @brief Renames file or direvtory on storage
 * @details	This API design is based POSIX API. Please refer to 'rename()'.\n
 * But, the target of rename must be a file or directory on storage.
 * @since_tizen 3.0
 *
 * @param[in]	session		Handle of user session
 * @param[in]	src_path	Source file path on storage
 * @param[in]	dst_path	Destination file path on storage
 * @param[in]	timeout		Operation time out (millisecond)
 * @param[in]	user_data	Passed data from #service_provider_set_storage_provider()
 *
 * @remarks	Callback must be finished in timeout.
 * @remarks	Callback will be invoked on <b>additional thread</b>(Not main thread).
 * @remarks	Sometimes authentication is vitiated by user request or expiring or some policy, then return #SERVICE_ADAPTOR_ERROR_NOT_AUTHORIZED
 * @remarks	Before returnning #SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED, you should call #service_provider_set_last_error().
 *
 * @return 0 on success, otherwise a negative error value
 * @retval	#SERVICE_ADAPTOR_ERROR_NONE					Successful
 * @retval	#SERVICE_ADAPTOR_ERROR_PERMISSION_DENIED	Permission denied
 * @retval	#SERVICE_ADAPTOR_ERROR_OUT_OF_MEMORY		Out of memory
 * @retval	#SERVICE_ADAPTOR_ERROR_NETWORK_UNREACHABLE	Network is unreachable
 * @retval	#SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED		Failed by internal plugin issue
 * @retval	#SERVICE_ADAPTOR_ERROR_NOT_AUTHORIZED		Need authorized
 * @retval	#SERVICE_ADAPTOR_ERROR_UNKNOWN				Unknown error
 *
 * @see service_provider_set_last_error()
 *
 * @pre	#service_provider_set_storage_provider() must be called in app_create_cb()
 * @pre	#service_provider_open_channel() must be called in app_control_cb()
 */
typedef int (*storage_provider_rename_cb)(service_provider_session_h session,
						const char *src_path,
						const char *dst_path,
						int timeout,
						void *user_data);

/**
 * @brief Makes directory on storage
 * @details	This API design is based POSIX API. Please refer to 'mkdir()'.\n
 * But, the target of mkdir must be a directory on storage.
 * @since_tizen 3.0
 *
 * @param[in]	session		Handle of user session
 * @param[in]	path		Directory path on storage
 * @param[in]	timeout		Operation time out (millisecond)
 * @param[in]	user_data	Passed data from #service_provider_set_storage_provider()
 *
 * @remarks	Callback must be finished in timeout.
 * @remarks	Callback will be invoked on <b>additional thread</b>(Not main thread).
 * @remarks	Sometimes authentication is vitiated by user request or expiring or some policy, then return #SERVICE_ADAPTOR_ERROR_NOT_AUTHORIZED
 * @remarks	Before returnning #SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED, you should call #service_provider_set_last_error().
 *
 * @return 0 on success, otherwise a negative error value
 * @retval	#SERVICE_ADAPTOR_ERROR_NONE					Successful
 * @retval	#SERVICE_ADAPTOR_ERROR_PERMISSION_DENIED	Permission denied
 * @retval	#SERVICE_ADAPTOR_ERROR_OUT_OF_MEMORY		Out of memory
 * @retval	#SERVICE_ADAPTOR_ERROR_NETWORK_UNREACHABLE	Network is unreachable
 * @retval	#SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED		Failed by internal plugin issue
 * @retval	#SERVICE_ADAPTOR_ERROR_NOT_AUTHORIZED		Need authorized
 * @retval	#SERVICE_ADAPTOR_ERROR_UNKNOWN				Unknown error
 *
 * @see service_provider_set_last_error()
 *
 * @pre	#service_provider_set_storage_provider() must be called in app_create_cb()
 * @pre	#service_provider_open_channel() must be called in app_control_cb()
 */
typedef int (*storage_provider_mkdir_cb)(service_provider_session_h session,
						const char *path,
						int timeout,
						void *user_data);

/**
 * @brief Changes to access mode of file or directory on storage
 * @details	If target path is exist on cloud, it can modify access right for other user.\n
 * This API design is based POSIX API. Please refer to 'chmod()'.\n
 * But, the target of chmod must be a file or directory on storage.
 * @since_tizen 3.0
 *
 * @param[in]	session		Handle of user session
 * @param[in]	path		Directory path on storage
 * @param[in]	mode		Access mode
 * @param[in]	timeout		Operation time out (millisecond)
 * @param[in]	user_data	Passed data from #service_provider_set_storage_provider()
 *
 * @remarks	Callback must be finished in timeout.
 * @remarks	Callback will be invoked on <b>additional thread</b>(Not main thread).
 * @remarks	Supporting bellow <b>access mode</b> and it can be masked multiple mode using 'bit and' for a parameter \b \a mode.
 * \li \c \b S_IRWXU 00700 user (file owner) has read, write and execute permission
 * \li \c \b S_IRUSR 00400 user has read permission
 * \li \c \b S_IWUSR 00200 user has write permission
 * \li \c \b S_IXUSR 00100 user has execute permission
 * \li \c \b S_IRWXG 00070 group has read, write and execute permission
 * \li \c \b S_IRGRP 00040 group has read permission
 * \li \c \b S_IWGRP 00020 group has write permission
 * \li \c \b S_IXGRP 00010 group has execute permission
 * \li \c \b S_IRWXO 00007 others have read, write and execute permission
 * \li \c \b S_IROTH 00004 others have read permission
 * \li \c \b S_IWOTH 00002 others have write permission
 * \li \c \b S_IXOTH 00001 others have execute permission
 * \li Upper constant is defined in \b <fcntl.h>
 * \li Sometimes the access right for \b group and \b others means \b share
 * \n
 * @remarks	Sometimes authentication is vitiated by user request or expiring or some policy, then return #SERVICE_ADAPTOR_ERROR_NOT_AUTHORIZED
 * @remarks	Before returnning #SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED, you should call #service_provider_set_last_error().
 *
 * @return 0 on success, otherwise a negative error value
 * @retval	#SERVICE_ADAPTOR_ERROR_NONE					Successful
 * @retval	#SERVICE_ADAPTOR_ERROR_PERMISSION_DENIED	Permission denied
 * @retval	#SERVICE_ADAPTOR_ERROR_OUT_OF_MEMORY		Out of memory
 * @retval	#SERVICE_ADAPTOR_ERROR_NETWORK_UNREACHABLE	Network is unreachable
 * @retval	#SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED		Failed by internal plugin issue
 * @retval	#SERVICE_ADAPTOR_ERROR_NOT_AUTHORIZED		Need authorized
 * @retval	#SERVICE_ADAPTOR_ERROR_UNKNOWN				Unknown error
 *
 * @see service_provider_set_last_error()
 *
 * @pre	#service_provider_set_storage_provider() must be called in app_create_cb()
 * @pre	#service_provider_open_channel() must be called in app_control_cb()
 */
typedef int (*storage_provider_chmod_cb)(service_provider_session_h session,
						const char *path,
						int mode,
						int timeout,
						void *user_data);

/**
 * @brief Check user's permissions for file or directory
 * @details	This API design is based POSIX API. Please refer to 'access()'.\n
 * But, the target of access must be a file or directory on storage.
 * @since_tizen 3.0
 *
 * @param[in]	session			Handle of user session
 * @param[in]	path			Directory path on storage
 * @param[in]	accessibility	The flags for checking accessibility
 * @param[in]	timeout			Operation time out (millisecond)
 * @param[in]	user_data		Passed data from #service_provider_set_storage_provider()
 *
 * @remarks	Callback must be finished in timeout.
 * @remarks	Callback will be invoked on <b>additional thread</b>(Not main thread).
 * @remarks	Supporting bellow <b>accessibility flags</b> and it can be masked multiple flags using 'bit and' for a parameter \b \a accessibility.
 * \li \c \b R_OK	4	Test for read permission.
 * \li \c \b W_OK	2	Test for write permission.
 * \li \c \b X_OK	1	Test for execute permission.
 * \li \c \b F_OK	0	Test for existence.
 * \li Upper constant is defined in \b <unistd.h>
 * \n
 * @remarks	If access right is not exist, it must return #SERVICE_ADAPTOR_ERROR_PERMISSION_DENIED
 * @remarks	Sometimes authentication is vitiated by user request or expiring or some policy, then return #SERVICE_ADAPTOR_ERROR_NOT_AUTHORIZED
 * @remarks	Before returnning #SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED, you should call #service_provider_set_last_error().
 *
 * @return 0 on success, otherwise a negative error value
 * @retval	#SERVICE_ADAPTOR_ERROR_NONE					Successful
 * @retval	#SERVICE_ADAPTOR_ERROR_PERMISSION_DENIED	Permission denied
 * @retval	#SERVICE_ADAPTOR_ERROR_OUT_OF_MEMORY		Out of memory
 * @retval	#SERVICE_ADAPTOR_ERROR_NETWORK_UNREACHABLE	Network is unreachable
 * @retval	#SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED		Failed by internal plugin issue
 * @retval	#SERVICE_ADAPTOR_ERROR_NOT_AUTHORIZED		Need authorized
 * @retval	#SERVICE_ADAPTOR_ERROR_UNKNOWN				Unknown error
 *
 * @see service_provider_set_last_error()
 *
 * @pre	#service_provider_set_storage_provider() must be called in app_create_cb()
 * @pre	#service_provider_open_channel() must be called in app_control_cb()
 */
typedef int (*storage_provider_access_cb)(service_provider_session_h session,
						const char *path,
						int accessibility,
						int timeout,
						void *user_data);

/**
 * @brief Gets status information of file or direstory on storage
 * @details The status includes file's timestamp, size, path and flag for checking directory.\n
 * \n
 * This API design is based POSIX API. Please refer to 'stat()'.\n
 * But, the target of stat must be a file or directory on storage.
 * @since_tizen 3.0
 *
 * @param[in]	session		Handle of user session
 * @param[in]	path		Directory path on storage
 * @param[in,out] stat		Status of file
 * @param[in]	timeout		Operation time out (millisecond)
 * @param[in]	user_data	Passed data from #service_provider_set_storage_provider()
 *
 * @remarks	Callback must be finished in timeout.
 * @remarks	Callback will be invoked on <b>additional thread</b>(Not main thread).
 * @remarks	For returning success, provider must fill status to \a stat handle.
 * @remarks	\a stat is already allocated memory space.
 * @remarks	Sometimes authentication is vitiated by user request or expiring or some policy, then return #SERVICE_ADAPTOR_ERROR_NOT_AUTHORIZED
 * @remarks	Before returnning #SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED, you should call #service_provider_set_last_error().
 *
 * @return 0 on success, otherwise a negative error value
 * @retval	#SERVICE_ADAPTOR_ERROR_NONE					Successful
 * @retval	#SERVICE_ADAPTOR_ERROR_PERMISSION_DENIED	Permission denied
 * @retval	#SERVICE_ADAPTOR_ERROR_OUT_OF_MEMORY		Out of memory
 * @retval	#SERVICE_ADAPTOR_ERROR_NETWORK_UNREACHABLE	Network is unreachable
 * @retval	#SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED		Failed by internal plugin issue
 * @retval	#SERVICE_ADAPTOR_ERROR_NOT_AUTHORIZED		Need authorized
 * @retval	#SERVICE_ADAPTOR_ERROR_UNKNOWN				Unknown error
 *
 * @see	#storage_provider_stat_h
 * @see storage_provider_stat_set_path()
 * @see	storage_provider_stat_set_size()
 * @see	storage_provider_stat_set_dir()
 * @see	storage_provider_stat_set_mode()
 * @see	storage_provider_stat_set_atime()
 * @see	storage_provider_stat_set_mtime()
 * @see	storage_provider_stat_set_ctime()
 * @see service_provider_set_last_error()
 *
 * @pre	#service_provider_set_storage_provider() must be called in app_create_cb()
 * @pre	#service_provider_open_channel() must be called in app_control_cb()
 */
typedef int (*storage_provider_stat_cb)(service_provider_session_h session,
						const char *path,
						storage_provider_stat_h stat,
						int timeout,
						void *user_data);

/**
 * @brief Open and gets status information of directory entries on storage.
 * @details	It should get all of the status information of directory of storage.\n
 * After this callback, #storage_provider_readdir_cb() will be invoked and requires \a stat handle of entry element.\n
 * \n
 * This API design is based POSIX API. Please refer to 'opendir()'.\n
 * But, the target of opendir must be a directory on storage.
 * @since_tizen 3.0
 *
 * @param[in]	session		Handle of user session
 * @param[in]	fd			Descriptor of a directory
 * @param[in]	path		Directory path on storage
 * @param[in]	timeout		Operation time out (millisecond)
 * @param[in]	user_data	Passed data from #service_provider_set_storage_provider()
 *
 * @remarks	Callback must be finished in timeout.
 * @remarks	Callback will be invoked on <b>additional thread</b>(Not main thread).
 * @remarks	\a fd is unique identifier for a task, and it is kept until finishing task. In same task logic, fd of opendir_cb, readdir_cb, closedir_cb  are same.\n
 * @remarks	Sometimes authentication is vitiated by user request or expiring or some policy, then return #SERVICE_ADAPTOR_ERROR_NOT_AUTHORIZED
 * @remarks	Before returnning #SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED, you should call #service_provider_set_last_error().
 *
 * @return 0 on success, otherwise a negative error value
 * @retval	#SERVICE_ADAPTOR_ERROR_NONE					Successful
 * @retval	#SERVICE_ADAPTOR_ERROR_PERMISSION_DENIED	Permission denied
 * @retval	#SERVICE_ADAPTOR_ERROR_OUT_OF_MEMORY		Out of memory
 * @retval	#SERVICE_ADAPTOR_ERROR_NETWORK_UNREACHABLE	Network is unreachable
 * @retval	#SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED		Failed by internal plugin issue
 * @retval	#SERVICE_ADAPTOR_ERROR_NOT_AUTHORIZED		Need authorized
 * @retval	#SERVICE_ADAPTOR_ERROR_UNKNOWN				Unknown error
 *
 * @see	storage_provider_readdir_cb()
 * @see	storage_provider_closedir_cb()
 * @see service_provider_set_last_error()
 *
 * @pre	#service_provider_set_storage_provider() must be called in app_create_cb()
 * @pre	#service_provider_open_channel() must be called in app_control_cb()
 */
typedef int (*storage_provider_opendir_cb)(service_provider_session_h session,
						int fd,
						const char *path,
						int timeout,
						void *user_data);

/**
 * @brief Reads file status information of directory element.
 * @details	Iterates of information from fd.\n
 * It can obtains status information in due order.\n
 * It will be invoked until callback returns #SERVICE_ADAPTOR_ERROR_NO_DATA.\n
 * \n
 * This API design is based POSIX API. Please refer to 'readdir()'.\n
 * But, the target of readdir must be a directory on storage.
 * @since_tizen 3.0
 *
 * @param[in]	session		Handle of user session
 * @param[in]	fd			Descriptor of a directory
 * @param[in,out] stat		Status of file
 * @param[in]	timeout		Operation time out (millisecond)
 * @param[in]	user_data	Passed data from #service_provider_set_storage_provider()
 *
 * @remarks	Callback must be finished in timeout.
 * @remarks	Callback will be invoked on <b>additional thread</b>(Not main thread).
 * @remarks	If end of diretory entry, callback must returns #SERVICE_ADAPTOR_ERROR_NO_DATA and do not set any data to \a stat handle.
 * @remarks	For returning success, provider must fill status to \a stat handle.
 * @remarks	\a stat is already allocated memory space.
 * @remarks	Sometimes authentication is vitiated by user request or expiring or some policy, then return #SERVICE_ADAPTOR_ERROR_NOT_AUTHORIZED
 * @remarks	Before returnning #SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED, you should call #service_provider_set_last_error().
 *
 * @return 0 on success, otherwise a negative error value
 * @retval	#SERVICE_ADAPTOR_ERROR_NONE					Successful
 * @retval	#SERVICE_ADAPTOR_ERROR_NO_DATA				There is no data available
 * @retval	#SERVICE_ADAPTOR_ERROR_PERMISSION_DENIED	Permission denied
 * @retval	#SERVICE_ADAPTOR_ERROR_OUT_OF_MEMORY		Out of memory
 * @retval	#SERVICE_ADAPTOR_ERROR_NETWORK_UNREACHABLE	Network is unreachable
 * @retval	#SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED		Failed by internal plugin issue
 * @retval	#SERVICE_ADAPTOR_ERROR_NOT_AUTHORIZED		Need authorized
 * @retval	#SERVICE_ADAPTOR_ERROR_UNKNOWN				Unknown error
 *
 * @see	#storage_provider_stat_h
 * @see storage_provider_stat_set_path()
 * @see	storage_provider_stat_set_size()
 * @see	storage_provider_stat_set_dir()
 * @see	storage_provider_stat_set_mode()
 * @see	storage_provider_stat_set_atime()
 * @see	storage_provider_stat_set_mtime()
 * @see	storage_provider_stat_set_ctime()
 * @see	storage_provider_opendir_cb()
 * @see	storage_provider_closedir_cb()
 * @see service_provider_set_last_error()
 *
 * @pre	#service_provider_set_storage_provider() must be called in app_create_cb()
 * @pre	#service_provider_open_channel() must be called in app_control_cb()
 */
typedef int (*storage_provider_readdir_cb)(service_provider_session_h session,
						int fd,
						storage_provider_stat_h stat,
						int timeout,
						void *user_data);

/**
 * @brief Finalize to perform operation likes readdir.
 * @details	The provider can finalize to storage operation.\n
 * \n
 * This API design is based POSIX API. Please refer to 'closedir()'.\n
 * But, the target of closedir must be a directory on storage.
 * @since_tizen 3.0
 *
 * @param[in]	session		Handle of user session
 * @param[in]	fd			Descriptor of a directory
 * @param[in]	timeout		Operation time out (millisecond)
 * @param[in]	user_data	Passed data from #service_provider_set_storage_provider()
 *
 * @remarks	Callback must be finished in timeout.
 * @remarks	Callback will be invoked on <b>additional thread</b>(Not main thread).
 * @remarks	Sometimes authentication is vitiated by user request or expiring or some policy, then return #SERVICE_ADAPTOR_ERROR_NOT_AUTHORIZED
 * @remarks	Before returnning #SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED, you should call #service_provider_set_last_error().
 *
 * @return 0 on success, otherwise a negative error value
 * @retval	#SERVICE_ADAPTOR_ERROR_NONE					Successful
 * @retval	#SERVICE_ADAPTOR_ERROR_OUT_OF_MEMORY		Out of memory
 * @retval	#SERVICE_ADAPTOR_ERROR_NETWORK_UNREACHABLE	Network is unreachable
 * @retval	#SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED		Failed by internal plugin issue
 * @retval	#SERVICE_ADAPTOR_ERROR_NOT_AUTHORIZED		Need authorized
 * @retval	#SERVICE_ADAPTOR_ERROR_UNKNOWN				Unknown error
 *
 * @see	storage_provider_opendir_cb()
 * @see	storage_provider_readdir_cb()
 * @see service_provider_set_last_error()
 *
 * @pre	#service_provider_set_storage_provider() must be called in app_create_cb()
 * @pre	#service_provider_open_channel() must be called in app_control_cb()
 */
typedef int (*storage_provider_closedir_cb)(service_provider_session_h session,
						int fd,
						int timeout,
						void *user_data);

/**
 * @brief	The structure type containing the set of callback functions for providing storage service.
 * @details	For providing storage feature, all of the structure elements(callbacks) are assigned to provider's functions.
 * @since_tizen 3.0
 *
 * @see	storage_provider_open_cb()
 * @see	storage_provider_read_cb()
 * @see	storage_provider_write_cb()
 * @see	storage_provider_fsync_cb()
 * @see	storage_provider_close_cb()
 * @see	storage_provider_remove_cb()
 * @see	storage_provider_rename_cb()
 * @see	storage_provider_mkdir_cb()
 * @see	storage_provider_chmod_cb()
 * @see	storage_provider_access_cb()
 * @see	storage_provider_stat_cb()
 * @see	storage_provider_opendir_cb()
 * @see	storage_provider_readdir_cb()
 * @see	storage_provider_closedir_cb()
*/
typedef struct
{
	storage_provider_open_cb		open;		/**< This callback function must be able to initialize of operation for file on storage (likes POSIX 'open')*/
	storage_provider_read_cb		read;		/**< This callback function must be able to read from descriptor of storage file on storage (likes POSIX 'read')*/
	storage_provider_write_cb		write;		/**< This callback function must be able to write to descriptor of storage file on storage (likes POSIX 'write')*/
	storage_provider_fsync_cb		fsync;		/**< This callback function must be able to synchronize changes to file or directory on storage (likes POSIX 'fsync')*/
	storage_provider_close_cb		close;		/**< This callback function must be able to deinitialize of operation for file on storage (likes POSIX 'close')*/

	storage_provider_remove_cb		remove;		/**< This callback function must be able to remove to file or directory on storage (likes POSIX 'remove')*/
	storage_provider_rename_cb		rename;		/**< This callback function must be able to rename to file or directory on storage (likes POSIX 'rename')*/
	storage_provider_mkdir_cb		mkdir;		/**< This callback function must be able to make directory on storage (likes POSIX 'mkdir')*/
	storage_provider_chmod_cb		chmod;		/**< This callback function must be able to change mode of file or directory on storage (likes POSIX 'chmod')*/
	storage_provider_access_cb		access;		/**< This callback function must be able to check user's permissions for file or directory on storage (likes POSIX 'access')*/
	storage_provider_stat_cb		stat;		/**< This callback function must be able to get status for file or directory on storage (likes POSIX 'stat')*/

	storage_provider_opendir_cb		opendir;	/**< This callback function must be able to get descriptor of storage directory  (likes POSIX 'opendir')*/
	storage_provider_readdir_cb		readdir;	/**< This callback function must be able to get attribute of directory entry from descriptor of storage directory  (likes POSIX 'readdir')*/
	storage_provider_closedir_cb	closedir;	/**< This callback function must be able to close descriptor of storage directory  (likes POSIX 'closedir')*/
} storage_provider_s;

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif /* __TIZEN_SOCIAL_SERVICE_PROVIDER_TYPE_H__ */
