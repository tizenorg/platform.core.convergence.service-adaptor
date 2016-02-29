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

#ifndef __CLIENT_CHECKER_H__
#define __CLIENT_CHECKER_H__

int client_checker_init(void);

void client_checker_deinit(void);

int client_checker_add_client(const char *service_handle_name, const char *cookie);

int client_checker_del_client(const char *service_handle_name);

int client_checker_check_privilege(const char *service_handle_name, const char *privilege);

/**
* @brief Checks raw level Smack access rights for client using security-server
* @since_tizen 2.4
*
* @param[in]	service_handle_name	The client's unique ID. It related with security cookie
* @param[in]	object			The smack label of target object
* @param[in]	access_rights		The smack's rights string (e.g. "rwxat")
* @retval	0			Success, Smack condition is satisfied
* @retval	others			Failed, Client needs additional rule
*/
int client_checker_check_access_rights(const char *service_handle_name, const char *object, const char *access_rights);

/**
* @brief Checks Smack read permission for client using security-server
* @since_tizen 2.4
*
* @param[in]	service_handle_name	The client's unique ID. It related with security cookie
* @param[in]	path			The file path that will be access
* @retval	0			Success, Smack condition is satisfied
* @retval	others			Failed, Client needs additional rule
*/
int client_checker_check_access_right_read(const char *service_handle_name, const char *path);

/**
* @brief Checks Smack write permission for client using security-server
* @since_tizen 2.4
*
* @param[in]	service_handle_name	The client's unique ID. It related with security cookie
* @param[in]	path			The file path that will be access
* @retval	0			Success, Smack condition is satisfied
* @retval	others			Failed, Client needs additional rule
*/
int client_checker_check_access_right_write(const char *service_handle_name, const char *path);

/**
* @brief Checks Smack file create permission for client using security-server
* @since_tizen 2.4
*
* @param[in]	service_handle_name	The client's unique ID. It related with security cookie
* @param[in]	path			The file path that will be access
* @retval	0			Success, Smack condition is satisfied
* @retval	others			Failed, Client needs additional rule
*/
int client_checker_check_access_right_create(const char *service_handle_name, const char *path);

const char *clieht_checker_get_last_error(void);

#endif /* __CLIENT_CHECKER_H__ */
