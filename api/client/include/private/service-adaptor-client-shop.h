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
 * File: service-adaptor-client-shop.h
 * Desc: Service Adaptor APIs
 * Created on: Oct, 2014
 * Auth: Yongjin Kim <youth.kim@samsung.com>
 *
 *****************************************************************************/
/**
 *	@file		service-adaptor-client-shop.h
 *	@brief		Defines interface of Service Adaptor's Shop
 *	@version	0.1
 */

#ifndef __SERVICE_ADAPTOR_CLIENT_SHOP_H__
#define __SERVICE_ADAPTOR_CLIENT_SHOP_H__

#include "service-adaptor-client.h"
#include "service_adaptor_client_type.h"

/**
* @brief Describes infromation about shop
*/
typedef struct _service_adaptor_shop_info_s {
	int category_id;		/**< specifies category id of item*/
	long item_id;			/**< specifies item id*/
	long sticker_id;		/**< specifies sticker id*/
	char *lang_cd;			/**< specifies display language type*/
	char *cntry_cd;			/**< specifies country code*/
	int rwidth;			/**< specifies device resolution width*/
	int rheight;			/**< specifies device resolution height*/
	int start_idx;			/**< default value : 0*/
	int count;			/**< default value : 5*/
} service_adaptor_shop_info_s;

/**
* @brief Describes infromation about item
*/
typedef struct _service_adaptor_shop_item_s {
	long item_id;			/**< specifies category id of item*/
	int category_id;		/**< specifies category id of item*/
	long *sticker_ids;		/**< specifies category id of item*/
	unsigned int sticker_ids_len;	/**< specifies category id of item*/
	char *title;			/**< specifies category id of item*/
	char *character;		/**< specifies category id of item*/
	int version;			/**< specifies category id of item*/
	char *download_url;		/**< specifies category id of item*/
	char *panel_url;		/**< specifies category id of item*/
	char *sticker_url;		/**< specifies category id of item*/
	long file_size;			/**< specifies category id of item*/
	int count;			/**< specifies category id of item*/
	char *character_code;		/**< specifies category id of item*/
	long long int startdate;	/**< specifies category id of item*/
	long long int enddate;		/**< specifies category id of item*/
	long long int expired_date;	/**< specifies category id of item*/
	long long int valid_period;	/**< specifies category id of item*/
} service_adaptor_shop_item_s;

/*==================================================================================================
					FUNCTION PROTOTYPES
==================================================================================================*/

/**
* @brief Requests List of Item
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	info		specifies shop information
* @param[in]	user_data	specifies user_data (json) passed in API
* @param[in]	items		specifies list of items
* @param[in]	items_len	specifies length of items
* @param[out]	error_code	specifies error code
* @param[out]	server_data	specifies server_data (json) passed in API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_RESULT_SUCCEEDED if Successful
*/
int service_adaptor_get_item_list(service_adaptor_h handle,
						service_adaptor_shop_info_s *info,
						void *user_data,
						service_adaptor_shop_item_s ***items,
						unsigned int *items_len,
						service_adaptor_error_s **error_code,
						void **server_data);

/**
* @brief Requests Item Information for Download
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	info		specifies shop information
* @param[in]	user_data	specifies user_data (json) passed in API
* @param[in]	item		specifies item
* @param[out]	error_code	specifies error code
* @param[out]	server_data	specifies server_data (json) passed in API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_RESULT_SUCCEEDED if Successful
*/
int service_adaptor_download_item_package(service_adaptor_h handle,
						service_adaptor_shop_info_s *info,
						void *user_data,
						service_adaptor_shop_item_s **item,
						service_adaptor_error_s **error_code,
						void **server_data);

/**
* @brief Requests Download of Item
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	info		specifies shop information
* @param[in]	user_data	specifies user_data (json) passed in API
* @param[in]	item		specifies item
* @param[out]	error_code	specifies error code
* @param[out]	server_data	specifies server_data (json) passed in API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_RESULT_SUCCEEDED if Successful
*/
int service_adaptor_download_sticker(service_adaptor_h handle,
						service_adaptor_shop_info_s *info,
						void *user_data,
						service_adaptor_shop_item_s **item,
						service_adaptor_error_s **error_code,
						void **server_data);

/**
* @brief Requests Item Panel URL
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	info		specifies shop information
* @param[in]	user_data	specifies user_data (json) passed in API
* @param[in]	item		specifies item
* @param[out]	error_code	specifies error code
* @param[out]	server_data	specifies server_data (json) passed in API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_RESULT_SUCCEEDED if Successful
*/
int service_adaptor_get_panel_url(service_adaptor_h handle,
						service_adaptor_shop_info_s *info,
						void *user_data,
						service_adaptor_shop_item_s **item,
						service_adaptor_error_s **error_code,
						void **server_data);

#endif /* __SERVICE_ADAPTOR_CLIENT_SHOP_H__ */
