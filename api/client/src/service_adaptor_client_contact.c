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
 * File: service-adaptor-client-contact.c
 * Desc:
 * Created on: Oct, 2014
 * Auth: Yongjin Kim <youth.kim@samsung.com>
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "service_adaptor_client_type.h"
#include "service_adaptor_client_log.h"
#include "private/service-adaptor-client-contact.h"
#include "dbus_client.h"
#include "dbus_client_contact.h"

#include "util/service_adaptor_client_util.h"
/**	@brief	Resets contact information in Contact Server and
 *		uploads native contact information of device to the server
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_set_new_contact_list(service_adaptor_h handle,
						service_adaptor_contact_req_s *contact_req,
						void *user_data,
						service_adaptor_contact_res_s **contact_res,
						service_adaptor_error_s **error_code,
						void **server_data)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;

	if ((NULL == handle) || (NULL == handle->plugin) || (NULL == contact_req)) {
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER, "Invalid Argument");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	if (NULL == handle->service_name) {
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_STATE, "Invalid handle state");
		return SERVICE_ADAPTOR_ERROR_INVALID_STATE;
	}

	ret = _dbus_set_new_contact_list(handle->service_name, contact_req, user_data, contact_res, server_data, &error);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		_assign_error_code(&error, error_code);
	}

	sac_api_end(ret);
	return ret;
}

/**	@brief	Synchronizes native contact information of device with Contact Server
 *		according to [type] field of each contact
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_set_contact_list(service_adaptor_h handle,
						service_adaptor_contact_req_s *contact_req,
						void *user_data,
						service_adaptor_contact_res_s **contact_res,
						service_adaptor_error_s **error_code,
						void **server_data)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;

	if ((NULL == handle) || (NULL == handle->plugin) || (NULL == contact_req)) {
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER, "Invalid Argument");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	if (NULL == handle->service_name) {
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_STATE, "Invalid handle state");
		return SERVICE_ADAPTOR_ERROR_INVALID_STATE;
	}

	ret = _dbus_set_contact_list(handle->service_name, contact_req, user_data, contact_res, server_data, &error);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		_assign_error_code(&error, error_code);
	}

	sac_api_end(ret);
	return ret;
}

/**	@brief	Gets profile and service registration information of each contact
 *		(only contacts agreed to share with me are returned)
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_get_contact_infos_latest(service_adaptor_h handle,
						service_adaptor_contact_res_s **contact_res,
						service_adaptor_error_s **error_code,
						void **server_data)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;

	if ((NULL == handle) || (NULL == handle->plugin) || (NULL == contact_res)) {
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER, "Invalid Argument");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	if (NULL == handle->service_name) {
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_STATE, "Invalid handle state");
		return SERVICE_ADAPTOR_ERROR_INVALID_STATE;
	}

	ret = _dbus_get_contact_list(handle->service_name, contact_res, server_data, &error);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		_assign_error_code(&error, error_code);
	}

	sac_api_end(ret);
	return ret;
}

/**	@brief	Gets profiles and service registration information of contacts
 *		that have been updated since last update
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_get_contact_infos_polling(service_adaptor_h handle,
						service_adaptor_contact_req_s *contact_req,
						void *user_data,
						service_adaptor_contact_res_s **contact_res,
						service_adaptor_error_s **error_code,
						void **server_data)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;

	if ((NULL == handle) || (NULL == handle->plugin) || (NULL == contact_req)) {
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER, "Invalid Argument");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	if (NULL == handle->service_name) {
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_STATE, "Invalid handle state");
		return SERVICE_ADAPTOR_ERROR_INVALID_STATE;
	}

	ret = _dbus_get_contact_infos_polling(handle->service_name, contact_req, user_data, contact_res, server_data, &error);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		_assign_error_code(&error, error_code);
	}

	sac_api_end(ret);
	return ret;
}

/**	@brief	Sets or updates device’s profile to server
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_set_me_profile_with_push(service_adaptor_h handle,
						service_adaptor_profile_req_s *profile_req,
						service_adaptor_error_s **error_code,
						void *user_data)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;

	if ((NULL == handle) || (NULL == handle->plugin) || (NULL == profile_req)) {
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER, "Invalid Argument");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	if (NULL == handle->service_name) {
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_STATE, "Invalid handle state");
		return SERVICE_ADAPTOR_ERROR_INVALID_STATE;
	}

	ret = _dbus_set_me_profile_with_push(handle->service_name, profile_req, user_data, &error);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		_assign_error_code(&error, error_code);
	}

	sac_api_end(ret);
	return ret;
}

/**	@brief	Gets the profile information of a contact which is correspondent with
 *		country code (optional) and phone number (mandatory)
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_get_profile(service_adaptor_h handle,
						service_adaptor_profile_req_s *profile_req,
						void *user_data,
						service_adaptor_profile_res_s **profile_res,
						service_adaptor_error_s **error_code,
						void **server_data)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;

	if ((NULL == handle) || (NULL == handle->plugin) || (NULL == profile_req) || (NULL == profile_res)) {
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER, "Invalid Argument");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	if (NULL == handle->service_name) {
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_STATE, "Invalid handle state");
		return SERVICE_ADAPTOR_ERROR_INVALID_STATE;
	}

	ret = _dbus_get_profile(handle->service_name, profile_req, user_data,
			profile_res, server_data, &error);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		_assign_error_code(&error, error_code);
	}

	sac_api_end(ret);
	return ret;
}

/**	@brief	Uploads profile image meta to File Server and sets my profile image to Profile Server
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_set_profile_image_meta_with_push(service_adaptor_h handle,
						service_adaptor_profile_image_h *images,
						unsigned int images_len,
						void *user_data,
						service_adaptor_profile_image_h **failed_images,
						unsigned int *failed_images_len,
						service_adaptor_error_s **error_code,
						void **server_data)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;

	if ((NULL == handle) || (NULL == handle->plugin) || (NULL == images)) {
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER, "Invalid Argument");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	if (NULL == handle->service_name) {
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_STATE, "Invalid handle state");
		return SERVICE_ADAPTOR_ERROR_INVALID_STATE;
	}

	ret = _dbus_set_profile_image_meta_with_push(handle->service_name,
			images, images_len, user_data, failed_images, failed_images_len, server_data, &error);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		_assign_error_code(&error, error_code);
	}

	sac_api_end(ret);
	return ret;
}

/**	@brief	Deletes profile image meta from Profile Server
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_delete_me_profile_image_meta_with_push(service_adaptor_h handle,
						service_adaptor_error_s **error_code,
						void *user_data)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;

	if ((NULL == handle) || (NULL == handle->plugin)) {
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER, "Invalid Argument");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	if (NULL == handle->service_name) {
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_STATE, "Invalid handle state");
		return SERVICE_ADAPTOR_ERROR_INVALID_STATE;
	}

	ret = _dbus_delete_me_profile_image_meta_with_push(handle->service_name, user_data, &error);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		_assign_error_code(&error, error_code);
	}

	sac_api_end(ret);
	return ret;
}

/**	@brief	Sets the level of privacy, the scope of people to be opened
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_set_me_profile_privacy(service_adaptor_h handle,
						service_adaptor_privacy_req_s *privacy_req,
						service_adaptor_error_s **error_code,
						void *user_data)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;

	if ((NULL == handle) || (NULL == handle->plugin) || (NULL == privacy_req)) {
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER, "Invalid Argument");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	if (NULL == handle->service_name) {
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_STATE, "Invalid handle state");
		return SERVICE_ADAPTOR_ERROR_INVALID_STATE;
	}

	ret = _dbus_set_me_profile_privacy(handle->service_name, privacy_req, user_data, &error);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		_assign_error_code(&error, error_code);
	}

	sac_api_end(ret);
	return ret;
}

/**	@brief	Gets my profile’s privacy level
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_get_me_profile_privacy(service_adaptor_h handle,
						service_adaptor_privacy_res_s **privacy_res,
						service_adaptor_error_s **error_code,
						void **server_data)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;

	if ((NULL == handle) || (NULL == handle->plugin) || (NULL == privacy_res)) {
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER, "Invalid Argument");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	if (NULL == handle->service_name) {
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_STATE, "Invalid handle state");
		return SERVICE_ADAPTOR_ERROR_INVALID_STATE;
	}

	ret = _dbus_get_me_profile_privacy(handle->service_name, privacy_res, server_data, &error);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		_assign_error_code(&error, error_code);
	}

	sac_api_end(ret);
	return ret;
}

/**	@brief	Sets my presence information to Presence Server
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_set_me_presence_with_push(service_adaptor_h handle,
						service_adaptor_presence_req_s *presence_req,
						service_adaptor_error_s **error_code,
						void *user_data)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;

	if ((NULL == handle) || (NULL == handle->plugin) || (NULL == presence_req)) {
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER, "Invalid Argument");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	if (NULL == handle->service_name) {
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_STATE, "Invalid handle state");
		return SERVICE_ADAPTOR_ERROR_INVALID_STATE;
	}

	ret = _dbus_set_me_presence_with_push(handle->service_name, presence_req, user_data, &error);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		_assign_error_code(&error, error_code);
	}

	sac_api_end(ret);
	return ret;
}

/**	@brief	Sets my presence ON/OFF information to Presence Server
 *	@return	int
 *	@remarks :
 */
SERVICE_ADAPTOR_CLIENT_PUBLIC_API
int service_adaptor_set_me_presence_on_off_with_push(service_adaptor_h handle,
						service_adaptor_presence_req_s *presence_req,
						service_adaptor_error_s **error_code,
						void *user_data)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;

	if ((NULL == handle) || (NULL == handle->plugin) || (NULL == presence_req)) {
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER, "Invalid Argument");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	if (NULL == handle->service_name) {
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_STATE, "Invalid handle state");
		return SERVICE_ADAPTOR_ERROR_INVALID_STATE;
	}

	ret = _dbus_set_me_presence_on_off_with_push(handle->service_name,
			presence_req, user_data, &error);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		_assign_error_code(&error, error_code);
	}

	sac_api_end(ret);
	return ret;
}

int service_adaptor_set_me_profile_type(service_adaptor_h handle,
						int profile_type,
						char **profile_url,
						service_adaptor_error_s **error_code,
						void *user_data)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	service_adaptor_error_s error;
	error.msg = NULL;

	if ((NULL == handle) || (NULL == handle->plugin) || (NULL == profile_url)) {
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER, "Invalid Argument");
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	if (NULL == handle->service_name) {
		_set_error_code(error_code, SERVICE_ADAPTOR_ERROR_INVALID_STATE, "Invalid handle state");
		return SERVICE_ADAPTOR_ERROR_INVALID_STATE;
	}

	ret = _dbus_set_me_profile_type(handle->service_name,
			profile_type, profile_url, user_data, &error);

	if (ret != SERVICE_ADAPTOR_ERROR_NONE) {
		_assign_error_code(&error, error_code);
	}

	sac_api_end(ret);
	return ret;
}



int service_adaptor_profile_image_create(service_adaptor_profile_image_h *image)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	if (NULL == image) {
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	*image = (service_adaptor_profile_image_h)calloc(1, sizeof(struct _service_adaptor_profile_image_s));

	if (NULL == *image) {
		return SERVICE_ADAPTOR_ERROR_UNKNOWN;
	}

	memset((*image)->path, 0, 2048);

	sac_api_end(ret);
	return ret;
}

int service_adaptor_profile_image_set_element(service_adaptor_profile_image_h image,
						service_adaptor_contact_request_type_e req_type,
						int index,
						const char *path)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	if ((NULL == image) || (NULL == path)) {
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	image->type = req_type;
	image->index = index;
	strncpy(image->path, path, (CONTACT_PROFILE_IMAGE_PATH_LEN - 1));

	sac_api_end(ret);
	return ret;
}

int service_adaptor_profile_image_get_index(service_adaptor_profile_image_h image, int *index)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	if ((NULL == image) || (NULL == index)) {
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	*index = image->index;

	sac_api_end(ret);
	return ret;
}

int service_adaptor_profile_image_get_url(service_adaptor_profile_image_h image, char **url)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	if ((NULL == image) || (NULL == url)) {
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	if ('\0' == image->path[0]) {
		*url = NULL;
		ret = SERVICE_ADAPTOR_ERROR_NO_DATA;
	} else {
		*url = strdup(image->path);
		if (*url == NULL) {
			ret = SERVICE_ADAPTOR_ERROR_UNKNOWN;
		}
	}

	sac_api_end(ret);
	return ret;
}

int service_adaptor_profile_image_get_req_type(service_adaptor_profile_image_h image, int *req_type)
{
	sac_api_start();
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	if ((NULL == image) || (NULL == req_type)) {
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;
	}

	*req_type = image->type;

	sac_api_end(ret);
	return ret;
}

void service_adaptor_profile_image_destroy(service_adaptor_profile_image_h image)
{
	free(image);
}

