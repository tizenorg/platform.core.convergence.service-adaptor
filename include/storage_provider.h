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

#ifndef __TIZEN_SOCIAL_SERVICE_PROVIDER_STORAGE_H__
#define __TIZEN_SOCIAL_SERVICE_PROVIDER_STORAGE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <service_provider_type.h>

/**
 * @addtogroup	STORAGE_PROVIDER_MODULE
 * @{
 */

/**
 * @brief Sets file path to handle of stat
 * @since_tizen 3.0
 *
 * @param[in]	stat		Status handle of file or directory
 * @param[in]	path		Path of file on storage
 *
 * @remarks	For the description style of the \a \b path , follow the linux root file system.
 *
 * @return 0 on success, otherwise a negative error value
 * @retval	#SERVICE_ADAPTOR_ERROR_NONE					Successful
 * @retval	#SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval	#SERVICE_ADAPTOR_ERROR_OUT_OF_MEMORY		Out of memory
 * @retval	#SERVICE_ADAPTOR_ERROR_UNKNOWN				Unknown error
 */
int storage_provider_stat_set_path(storage_provider_stat_h stat, const char *path);

/**
 * @brief Sets file size to handle of stat
 * @since_tizen 3.0
 *
 * @param[in]	stat		Status handle of file or directory
 * @param[in]	size		Size of file
 *
 * @remarks	For the unit of the \a \b size , use "byte".
 *
 * @return 0 on success, otherwise a negative error value
 * @retval	#SERVICE_ADAPTOR_ERROR_NONE					Successful
 * @retval	#SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval	#SERVICE_ADAPTOR_ERROR_OUT_OF_MEMORY		Out of memory
 * @retval	#SERVICE_ADAPTOR_ERROR_UNKNOWN				Unknown error
 */
int storage_provider_stat_set_size(storage_provider_stat_h stat, unsigned long long size);

/**
 * @brief Sets diretory type to handle of stat
 * @since_tizen 3.0
 *
 * @param[in]	stat		Status handle of file or directory
 * @param[in]	is_dir		On true, it is directory
 *
 * @remarks	If not be called this API, the defalut type is \b false (not a directory).
 *
 * @return 0 on success, otherwise a negative error value
 * @retval	#SERVICE_ADAPTOR_ERROR_NONE					Successful
 * @retval	#SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval	#SERVICE_ADAPTOR_ERROR_OUT_OF_MEMORY		Out of memory
 * @retval	#SERVICE_ADAPTOR_ERROR_UNKNOWN				Unknown error
 */
int storage_provider_stat_set_dir(storage_provider_stat_h stat, bool is_dir);

/**
 * @brief Sets access mode to handle of stat
 * @details	The stat's perpose is getting files information.\n
 * If provider sets illegal access mode, it can be a cause of malfunction.
 * @since_tizen 3.0
 *
 * @param[in]	stat		Status handle of file or directory
 * @param[in]	mode		Access mode
 *
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
 * @remarks	If not be called this API, the defalut mode is \b S_IRWXU.
 *
 * @return 0 on success, otherwise a negative error value
 * @retval	#SERVICE_ADAPTOR_ERROR_NONE					Successful
 * @retval	#SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval	#SERVICE_ADAPTOR_ERROR_OUT_OF_MEMORY		Out of memory
 * @retval	#SERVICE_ADAPTOR_ERROR_UNKNOWN				Unknown error
 */
int storage_provider_stat_set_mode(storage_provider_stat_h stat, int mode);

/**
 * @brief Sets time of last access to handle of stat
 * @since_tizen 3.0
 *
 * @param[in]	stat		Status handle of file or directory
 * @param[in]	timestamp	Time of last access
 *
 * @remarks	For the description style of the \a \b timestamp , follow the Unix time (also known as POSIX time or Epoch time).
 *
 * @return 0 on success, otherwise a negative error value
 * @retval	#SERVICE_ADAPTOR_ERROR_NONE					Successful
 * @retval	#SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval	#SERVICE_ADAPTOR_ERROR_OUT_OF_MEMORY		Out of memory
 * @retval	#SERVICE_ADAPTOR_ERROR_UNKNOWN				Unknown error
 */
int storage_provider_stat_set_atime(storage_provider_stat_h stat, unsigned long long timestamp);

/**
 * @brief Sets time of last modification to handle of stat
 * @since_tizen 3.0
 *
 * @param[in]	stat		Status handle of file or directory
 * @param[in]	timestamp	Time of last modification
 *
 * @remarks	For the description style of the \a \b timestamp , follow the Unix time (also known as POSIX time or Epoch time).
 *
 * @return 0 on success, otherwise a negative error value
 * @retval	#SERVICE_ADAPTOR_ERROR_NONE					Successful
 * @retval	#SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval	#SERVICE_ADAPTOR_ERROR_OUT_OF_MEMORY		Out of memory
 * @retval	#SERVICE_ADAPTOR_ERROR_UNKNOWN				Unknown error
 */
int storage_provider_stat_set_mtime(storage_provider_stat_h stat, unsigned long long timestamp);

/**
 * @brief Sets time of last status change to handle of stat
 * @since_tizen 3.0
 *
 * @param[in]	stat		Status handle of file or directory
 * @param[in]	timestamp	Time of last status change
 *
 * @remarks	For the description style of the \a \b timestamp , follow the Unix time (also known as POSIX time or Epoch time).
 *
 * @return 0 on success, otherwise a negative error value
 * @retval	#SERVICE_ADAPTOR_ERROR_NONE					Successful
 * @retval	#SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER	Invalid parameter
 * @retval	#SERVICE_ADAPTOR_ERROR_OUT_OF_MEMORY		Out of memory
 * @retval	#SERVICE_ADAPTOR_ERROR_UNKNOWN				Unknown error
 */
int storage_provider_stat_set_ctime(storage_provider_stat_h stat, unsigned long long timestamp);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif /* __TIZEN_SOCIAL_SERVICE_PROVIDER_STORAGE_H__ */
