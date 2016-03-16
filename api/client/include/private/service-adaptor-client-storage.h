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
 * File: service-adaptor-client-storage.h
 * Desc: Service Adaptor APIs
 * Created on: Oct, 2014
 * Auth: Yongjin Kim <youth.kim@samsung.com>
 *
 *****************************************************************************/
/**
 *	@file		service-adaptor-client-storage.h
 *	@brief		Defines interface of Service Adaptor's Storage
 *	@version	0.1
 */

#ifndef __PRIVATE_SERVICE_ADAPTOR_CLIENT_STORAGE_H__
#define __PRIVATE_SERVICE_ADAPTOR_CLIENT_STORAGE_H__

#include "service-adaptor-client.h"

typedef enum _service_adaptor_file_status_code_e
{
	SERVICE_ADAPTOR_TRANSFER_STATUS_PROGRESS        = 1,
	SERVICE_ADAPTOR_TRANSFER_STATUS_RESUME          = 1,
	SERVICE_ADAPTOR_TRANSFER_STATUS_PAUSE           = 2,
	SERVICE_ADAPTOR_TRANSFER_STATUS_CANCEL          = 3,    // canceled by request
	SERVICE_ADAPTOR_TRANSFER_STATUS_STOPPED         = 4,    // canceled by system
	SERVICE_ADAPTOR_TRANSFER_STATUS_FINISHED        = 5
} service_adaptor_file_status_code_e;

typedef enum _service_adaptor_thumbnail_size_e
{
	SERVICE_ADAPTOR_THUMBNAIL_XSMALL		= 96,
	SERVICE_ADAPTOR_THUMBNAIL_SMALL			= 200,
	SERVICE_ADAPTOR_THUMBNAIL_MEDIUM		= 500,
	SERVICE_ADAPTOR_THUMBNAIL_LARGE			= 760
} service_adaptor_thumbnail_size_e;

/**
* @brief Describes file description about download and upload
*/
typedef struct _service_adaptor_file_s
{
	int file_description;		/**< specifies status as none*/
} service_adaptor_file_s;

/**
* @brief Describes file infromation about download and upload
*/
typedef struct _service_adaptor_file_path_s
{
	char **file_paths;		/**< specifies status as none*/
	unsigned int file_paths_len;	/**< specifies status as none*/
} service_adaptor_file_path_s;

/**
* @brief Describes file status about transfer
*/
typedef struct _service_adaptor_file_status_s
{
	long long int total_size;			/**< specifies status as none*/
	long long int transferred_size;			/**< specifies status as none*/
	service_adaptor_file_status_code_e status;	/**< specifies status as none*/
} service_adaptor_file_status_s;

/**
* @brief The handle for File Description
*/
typedef service_adaptor_file_s *service_adaptor_file_h;

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
typedef void(* service_adaptor_file_progress_cb)(service_adaptor_h handle,
						service_adaptor_file_h file_handle,
						unsigned long long transferred_size,
						unsigned long long total_size,
						service_adaptor_error_s *error_code,
						void *user_data);

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
typedef void(* service_adaptor_file_transfer_completion_cb)(service_adaptor_h handle,
						service_adaptor_file_h file_handle,
						char *publish_url,
						service_adaptor_error_s *error_code,
						void *user_data);

/*==================================================================================================
                                         FUNCTION PROTOTYPES
==================================================================================================*/


#endif /* __PRIVATE_SERVICE_ADAPTOR_CLIENT_STORAGE_H__ */
