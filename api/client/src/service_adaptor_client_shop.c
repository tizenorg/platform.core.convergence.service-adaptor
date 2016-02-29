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
 * File: service-adaptor-client-shop.c
 * Desc:
 * Created on: Oct, 2014
 * Auth: Yongjin Kim <youth.kim@samsung.com>
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "private/service-adaptor-client-shop.h"
#include "service_adaptor_client_type.h"
#include "service_adaptor_client_log.h"
#include "dbus_client.h"
#include "dbus_client_shop.h"

#include "util/service_adaptor_client_util.h"
/**	@brief	Requests List of Item
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_get_item_list(service_adaptor_h handle,
						service_adaptor_shop_info_s *info,
						void *user_data,
						service_adaptor_shop_item_s ***items,
						unsigned int *items_len,
						service_adaptor_error_s **error_code,
						void **server_data)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;

	if ((NULL == handle) || (NULL == handle->plugin) || (NULL == info) || (NULL == items)) {
		sac_error("Invalid Parameter");
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER, "Invalid Parameter");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	if (NULL == handle->service_name) {
		sac_error("The service_name is NULL");
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_STATE, "Invalid handle state");
		return SERVICE_ADAPTOR_ERROR_INVALID_STATE;
	}

	ret = _dbus_get_item_list(handle->service_name, info, user_data, items, items_len, server_data, &error);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		_assign_error_code(&error, error_code);
	}

	sac_api_end(ret);
	return ret;
}

/**	@brief	Requests Item Information for Download
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_download_item_package(service_adaptor_h handle,
						service_adaptor_shop_info_s *info,
						void *user_data,
						service_adaptor_shop_item_s **item,
						service_adaptor_error_s **error_code,
						void **server_data)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;

	if ((NULL == handle) || (NULL == handle->plugin) || (NULL == info) || (NULL == item)) {
		sac_error("Invalid Parameter");
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER, "Invalid Parameter");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	if (NULL == handle->service_name) {
		sac_error("The service_name is NULL");
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_STATE, "Invalid handle state");
		return SERVICE_ADAPTOR_ERROR_INVALID_STATE;
	}

	ret = _dbus_download_item_package(handle->service_name, info, user_data, item, server_data, &error);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		_assign_error_code(&error, error_code);
	}

	sac_api_end(ret);
	return ret;
}

/**	@brief	Requests Download of Item
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_download_sticker(service_adaptor_h handle,
						service_adaptor_shop_info_s *info,
						void *user_data,
						service_adaptor_shop_item_s **item,
						service_adaptor_error_s **error_code,
						void **server_data)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;

	if ((NULL == handle) || (NULL == handle->plugin) || (NULL == info) || (NULL == item)) {
		sac_error("Invalid Parameter");
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER, "Invalid Parameter");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	if (NULL == handle->service_name) {
		sac_error("The service_name is NULL");
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_STATE, "Invalid handle state");
		return SERVICE_ADAPTOR_ERROR_INVALID_STATE;
	}

	ret = _dbus_download_sticker(handle->service_name, info, user_data, item, server_data, &error);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		_assign_error_code(&error, error_code);
	}

	sac_api_end(ret);
	return ret;
}

/**	@brief	Requests Item Panel URL
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_get_panel_url(service_adaptor_h handle,
						service_adaptor_shop_info_s *info,
						void *user_data,
						service_adaptor_shop_item_s **item,
						service_adaptor_error_s **error_code,
						void **server_data)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;

	if ((NULL == handle) || (NULL == handle->plugin) || (NULL == info) || (NULL == item)) {
		sac_error("Invalid Parameter");
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER, "Invalid Parameter");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	if (NULL == handle->service_name) {
		sac_error("The service_name is NULL");
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_STATE, "Invalid handle state");
		return SERVICE_ADAPTOR_ERROR_INVALID_STATE;
	}

	ret = _dbus_get_panel_url(handle->service_name, info, user_data, item, server_data, &error);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		_assign_error_code(&error, error_code);
	}

	sac_api_end(ret);
	return ret;
}

