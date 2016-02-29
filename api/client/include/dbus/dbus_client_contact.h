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
 * File: dbus-client-contact.h
 * Desc: D-Bbus IPC client APIs for contact
 * Created on: Oct, 2014
 * Auth: Yongjin Kim <youth.kim@samsung.com>
 *
 *****************************************************************************/
/**
 *	@file		dbus-client-contact.h
 *	@brief		Defines interface of D-Bus IPC
 *	@version	0.1
 */

#ifndef __DBUS_CLIENT_CONTACT_H__
#define __DBUS_CLIENT_CONTACT_H__

#include <glib.h>
#include "service_adaptor_client_type.h"
#include "private/service-adaptor-client-contact.h"

#define CONTACT_PROFILE_IMAGE_PATH_LEN	2048

typedef struct _service_adaptor_profile_image_s
{
	service_adaptor_contact_request_type_e type;
	int index;
	char path[CONTACT_PROFILE_IMAGE_PATH_LEN];
} service_adaptor_profile_image_s;


/**
 * @brief
 * @param[out]
 * @param[out]
 * @return
 * @pre This function requires opened DBus connection by service-adaptor-client-contact.c
 */
int _dbus_set_new_contact_list(const char *service_name,
						service_adaptor_contact_req_s *contact_req,
						void *user_data,
						service_adaptor_contact_res_s **contact_res,
						void **server_data,
						service_adaptor_error_s *error);

int _dbus_set_contact_list(const char *service_name,
						service_adaptor_contact_req_s *contact_req,
						void *user_data,
						service_adaptor_contact_res_s **contact_res,
						void **server_data,
						service_adaptor_error_s *error);

int _dbus_get_contact_list(const char *service_name,
						service_adaptor_contact_res_s **contact_res,
						void **server_data,
						service_adaptor_error_s *error);

int _dbus_get_contact_infos_polling(const char *service_name,
						service_adaptor_contact_req_s *contact_req,
						void *user_data,
						service_adaptor_contact_res_s **contact_res,
						void **server_data,
						service_adaptor_error_s *error);

int _dbus_set_me_profile_with_push(const char *service_name,
						service_adaptor_profile_req_s *profile_req,
						void *user_data,
						service_adaptor_error_s *error);

int _dbus_get_profile(const char *service_name,
						service_adaptor_profile_req_s *profile_req,
						void *user_data,
						service_adaptor_profile_res_s **profile_res,
						void **server_data,
						service_adaptor_error_s *error);

int _dbus_set_profile_image_meta_with_push(const char *service_name,
						service_adaptor_profile_image_h *images,
						unsigned int images_len,
						void *user_data,
						service_adaptor_profile_image_h **failed_images,
						unsigned int *failed_images_len,
						void **server_data,
						service_adaptor_error_s *error);

int _dbus_delete_me_profile_image_meta_with_push(const char *service_name,
						void *user_data,
						service_adaptor_error_s *error);

int _dbus_set_me_profile_privacy(const char *service_name,
						service_adaptor_privacy_req_s *privacy_req,
						void *user_data,
						service_adaptor_error_s *error);

int _dbus_get_me_profile_privacy(const char *service_name,
						service_adaptor_privacy_res_s **privacy_res,
						void **server_data,
						service_adaptor_error_s *error);

int _dbus_set_me_presence_with_push(const char *service_name,
						service_adaptor_presence_req_s *presence_req,
						void *user_data,
						service_adaptor_error_s *error);

int _dbus_set_me_presence_on_off_with_push(const char *service_name,
						service_adaptor_presence_req_s *presence_req,
						void *user_data,
						service_adaptor_error_s *error);

int _dbus_set_me_profile_type(const char *service_name,
						int type,
						char **url,
						void *user_data,
						service_adaptor_error_s *error);

#endif /* __DBUS_CLIENT_CONTACT_H__ */

