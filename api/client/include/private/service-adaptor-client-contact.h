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
 * File: service-adaptor-client-contact.h
 * Desc: Service Adaptor APIs
 * Created on: Oct, 2014
 * Auth: Yongjin Kim <youth.kim@samsung.com>
 *
 *****************************************************************************/
/**
 *	@file		service-adaptor-client-contact.h
 *	@brief		Defines interface of Service Adaptor's Contact
 *	@version	0.1
 */

#ifndef __SERVICE_ADAPTOR_CLIENT_CONTACT_H__
#define __SERVICE_ADAPTOR_CLIENT_CONTACT_H__

#include "service-adaptor-client.h"
#include "service_adaptor_client_type.h"
#include "service-adaptor-client-storage.h"

typedef struct _service_adaptor_profile_image_s *service_adaptor_profile_image_h;

/**
* @brief Describes contact infromation for request
*/
typedef struct _service_adaptor_contact_info_req_s {
	char *tp;		/**< specifies status as none*/
	char *id;		/**< specifies status as none*/
	char *pn;		/**< specifies status as none*/
	char *nm;		/**< specifies status as none*/
	char *cc;		/**< specifies status as none*/
} service_adaptor_contact_info_req_s;

/**
* @brief Describes request infromation about contact
*/
typedef struct _service_adaptor_contact_req_s {
	long long int tt;				/**< specifies status as none*/
	service_adaptor_contact_info_req_s **cts;	/**< specifies status as none*/
	unsigned int cts_len;				/**< specifies status as none*/
} service_adaptor_contact_req_s;

/**
* @brief Describes contact infromation for response
*/
typedef struct _service_adaptor_contact_info_res_s {
	char *duid;		/**< specifies status as none*/
	char *id;		/**< specifies status as none*/
	char *msisdn;		/**< specifies status as none*/
	char *ty;		/**< specifies status as none*/
	char *cc;		/**< specifies status as none*/
	char *pn;		/**< specifies status as none*/
	char *nm;		/**< specifies status as none*/
	char **evnt;		/**< specifies status as none*/
	unsigned int evnt_len;	/**< specifies status as none*/
	/* char *img; */	/**< specifies status as none*/
	service_adaptor_profile_image_h *images;
	unsigned int images_len;
	char **adrs;		/**< specifies status as none*/
	unsigned int adrs_len;	/**< specifies status as none*/
	char **mail;		/**< specifies status as none*/
	unsigned int mail_len;	/**< specifies status as none*/
	char *org;		/**< specifies status as none*/
	char *prsc;		/**< specifies status as none*/
	char *status;		/**< specifies status as none*/
	unsigned int sids;	/**< specifies status as none*/
	int profile_type;
	char *profile_url;
} service_adaptor_contact_info_res_s;

/**
* @brief Describes response infromation about contact
*/
typedef struct _service_adaptor_contact_res_s {
	long long int tt;				/**< specifies status as none*/
	service_adaptor_contact_info_res_s **cts;	/**< specifies status as none*/
	unsigned int cts_len;				/**< specifies status as none*/
} service_adaptor_contact_res_s;

/**
* @brief Describes request infromation about profile
*/
typedef struct _service_adaptor_profile_req_s {
	char *cc;		/**< specifies status as none*/
	char *pn;		/**< specifies status as none*/
	char *nm;		/**< specifies status as none*/
	char **evnt;		/**< specifies status as none*/
	unsigned int evnt_len;	/**< specifies status as none*/
	char *img;		/**< specifies status as none*/
	char **adrs;		/**< specifies status as none*/
	unsigned int adrs_len;	/**< specifies status as none*/
	char **mail;		/**< specifies status as none*/
	unsigned int mail_len;	/**< specifies status as none*/
	char *org;		/**< specifies status as none*/
	char *prsc;		/**< specifies status as none*/
	char *status;		/**< status message*/
} service_adaptor_profile_req_s;

/**
* @brief Describes response infromation about profile
*/
typedef struct _service_adaptor_profile_res_s {
	char *nm;		/**< specifies status as none*/
	char *img;		/**< specifies status as none*/
	char *prsc;		/**< specifies status as none*/
	char *status;		/**< specifies status as none*/
} service_adaptor_profile_res_s;

/**
* @brief Describes privacy infromation for request
*/
typedef struct _service_adaptor_privacy_info_req_s {
	char *cc;		/**< specifies status as none*/
	char *pn;		/**< specifies status as none*/
} service_adaptor_privacy_info_req_s;

/**
* @brief Describes request infromation about privacy
*/
typedef struct _service_adaptor_privacy_req_s {
	unsigned int lvl;				/**< specifies status as none*/
	service_adaptor_privacy_info_req_s **cts;	/**< specifies status as none*/
	unsigned int cts_len;				/**< specifies status as none*/
} service_adaptor_privacy_req_s;

/**
* @brief Describes response infromation about privacy
*/
typedef struct _service_adaptor_privacy_res_s {
	unsigned int lvl;	/**< specifies status as none*/
	unsigned int prscon;	/**< specifies status as none*/
} service_adaptor_privacy_res_s;

/**
* @brief Describes presence infromation for request
*/
typedef struct _service_adaptor_presence_req_s {
	char *prsc;		/**< specifies status as none*/
	char *status;		/**< specifies status as none*/
	unsigned int prscon;	/**< specifies status as none*/
} service_adaptor_presence_req_s;

/*==================================================================================================
					FUNCTION PROTOTYPES
==================================================================================================*/

/**
* @brief Resets contact information in Contact Server and
*        uploads native contact information of device to the server
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	contact_req	request information about contact
* @param[in]	user_data	specifies user_data (json) passed in API
* @param[out]	contact_res	response information about contact
* @param[out]	error_code	specifies error code
* @param[out]	server_data	specifies server_data (json) passed in Server API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_RESULT_SUCCEEDED if Successful
*/
int service_adaptor_set_new_contact_list(service_adaptor_h handle,
						service_adaptor_contact_req_s *contact_req,
						void *user_data,
						service_adaptor_contact_res_s **contact_res,
						service_adaptor_error_s **error_code,
						void **server_data);

/**
* @brief Synchronizes native contact information of device with Contact Server
*        according to [type] field of each contact
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	contact_req	request information about contact
* @param[in]	user_data	specifies user_data (json) passed in API
* @param[out]	contact_res	response information about contact
* @param[out]	error_code	specifies error code
* @param[out]	server_data	specifies server_data (json) passed in Server API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_RESULT_SUCCEEDED if Successful
*/
int service_adaptor_set_contact_list(service_adaptor_h handle,
						service_adaptor_contact_req_s *contact_req,
						void *user_data,
						service_adaptor_contact_res_s **contact_res,
						service_adaptor_error_s **error_code,
						void **server_data);

/**
* @brief Gets profile and service registration information of each contact
*        (only contacts agreed to share with me are returned)
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[out]	contact_res	response information about contact
* @param[out]	error_code	specifies error code
* @param[out]	server_data	specifies server_data (json) passed in Server API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_RESULT_SUCCEEDED if Successful
*/
int service_adaptor_get_contact_infos_latest(service_adaptor_h handle,
						service_adaptor_contact_res_s **contact_res,
						service_adaptor_error_s **error_code,
						void **server_data);

/**
* @brief Gets profiles and service registration information of contacts
*        that have been updated since last update
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	contact_req	request information about contact
* @param[in]	user_data	specifies user_data (json) passed in API
* @param[out]	contact_res	response information about contact
* @param[out]	error_code	specifies error code
* @param[out]	server_data	specifies server_data (json) passed in Server API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_RESULT_SUCCEEDED if Successful
*/
int service_adaptor_get_contact_infos_polling(service_adaptor_h handle,
						service_adaptor_contact_req_s *contact_req,
						void *user_data,
						service_adaptor_contact_res_s **contact_res,
						service_adaptor_error_s **error_code,
						void **server_data);

/**
* @brief Sets or updates device’s profile to server
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	profile_req	request information about profile
* @param[out]	error_code	specifies error code
* @param[in]	user_data	specifies user_data (json) passed in API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_RESULT_SUCCEEDED if Successful
*/
int service_adaptor_set_me_profile_with_push(service_adaptor_h handle,
						service_adaptor_profile_req_s *profile_req,
						service_adaptor_error_s **error_code,
						void *user_data);

/**
* @brief Gets the profile information of a contact which is correspondent with
*        country code (optional) and phone number (mandatory)
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	profile_req	request information about profile
* @param[in]	user_data	specifies user_data (json) passed in API
* @param[out]	profile_res	response information about profile
* @param[out]	error_code	specifies error code
* @param[out]	server_data	specifies server_data (json) passed in Server API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_RESULT_SUCCEEDED if Successful
*/
int service_adaptor_get_profile(service_adaptor_h handle,
						service_adaptor_profile_req_s *profile_req,
						void *user_data,
						service_adaptor_profile_res_s **profile_res,
						service_adaptor_error_s **error_code,
						void **server_data);

/**
* @brief Uploads profile image meta to File Server and sets my profile image to Profile Server
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	file_paths	file information for request
* @param[in]	user_data	specifies user_data (json) passed in API
* @param[out]	server_paths	file information for response
* @param[out]	error_code	specifies error code
* @param[out]	server_data	specifies server_data (json) passed in Server API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_RESULT_SUCCEEDED if Successful
*/
int service_adaptor_set_profile_image_meta_with_push(service_adaptor_h handle,
						service_adaptor_profile_image_h *images,
						unsigned int images_len,
						void *user_data,
						service_adaptor_profile_image_h **failed_images,
						unsigned int *failed_images_len,
						service_adaptor_error_s **error_code,
						void **server_data);

/**
* @brief Deletes profile image meta from Profile Server
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	server_paths	file information responsed by xxx_set_profile_image_meta_with_push()
* @param[out]	error_code	specifies error code
* @param[in]	user_data	specifies user_data (json) passed in API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_RESULT_SUCCEEDED if Successful
*/
int service_adaptor_delete_me_profile_image_meta_with_push(service_adaptor_h handle,
						service_adaptor_error_s **error_code,
						void *user_data);

/**
* @brief Sets the level of privacy, the scope of people to be opened
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	privacy_req	request information about privacy
* @param[out]	error_code	specifies error code
* @param[in]	user_data	specifies user_data (json) passed in API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_RESULT_SUCCEEDED if Successful
*/
int service_adaptor_set_me_profile_privacy(service_adaptor_h handle,
						service_adaptor_privacy_req_s *privacy_req,
						service_adaptor_error_s **error_code,
						void *user_data);

/**
* @brief Gets my profile’s privacy level
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[out]	privacy_res	response information about privacy
* @param[out]	error_code	specifies error code
* @param[out]	server_data	specifies server_data (json) passed in Server API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_RESULT_SUCCEEDED if Successful
*/
int service_adaptor_get_me_profile_privacy(service_adaptor_h handle,
						service_adaptor_privacy_res_s **privacy_res,
						service_adaptor_error_s **error_code,
						void **server_data);

/**
* @brief Sets my presence information to Presence Server
*
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	presence_req	request information about presence
* @param[out]	error_code	specifies error code
* @param[in]	user_data	specifies user_data (json) passed in API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_RESULT_SUCCEEDED if Successful
*/
int service_adaptor_set_me_presence_with_push(service_adaptor_h handle,
						service_adaptor_presence_req_s *presence_req,
						service_adaptor_error_s **error_code,
						void *user_data);

/**
* @brief Sets my presence ON/OFF information to Presence Server
*
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	presence_req	request information about presence
* @param[out]	error_code	specifies error code
* @param[in]	user_data	specifies user_data (json) passed in API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_RESULT_SUCCEEDED if Successful
*/
int service_adaptor_set_me_presence_on_off_with_push(service_adaptor_h handle,
						service_adaptor_presence_req_s *presence_req,
						service_adaptor_error_s **error_code,
						void *user_data);

/**
* @brief Sets my profile type to Presence Server
*
*
* @param[in]	handle		specifies Service Adaptor handle
* @param[in]	profile_type	the profile types, this value is following a server spec
* @param[out]	profile_url	the profile web url, this value is optional output value
* @param[out]	error_code	specifies error code
* @param[in]	user_data	specifies user_data (json) passed in API
* @return 0 on success, otherwise a negative error value
* @retval error code defined in int - SERVICE_ADAPTOR_RESULT_SUCCEEDED if Successful
* @remarks      @a profile_url must be released using free()
*/
int service_adaptor_set_me_profile_type(service_adaptor_h handle,
						int profile_type,
						char **profile_url,
						service_adaptor_error_s **error_code,
						void *user_data);

typedef enum {
	SERVICE_ADAPTOR_CONTACT_SET	= 1,
	SERVICE_ADAPTOR_CONTACT_DELETE	= 2,
} service_adaptor_contact_request_type_e;

int service_adaptor_profile_image_create(service_adaptor_profile_image_h *image);

int service_adaptor_profile_image_set_element(service_adaptor_profile_image_h image,
						service_adaptor_contact_request_type_e req_type,
						int index,
						const char *path);

int service_adaptor_profile_image_get_index(service_adaptor_profile_image_h image, int *index);

int service_adaptor_profile_image_get_url(service_adaptor_profile_image_h image, char **url);

int service_adaptor_profile_image_get_req_type(service_adaptor_profile_image_h image, int *req_type);

void service_adaptor_profile_image_destroy(service_adaptor_profile_image_h image);



#endif /* __SERVICE_ADAPTOR_CLIENT_CONTACT_H__ */
