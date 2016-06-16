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

#include <string.h>
#include <stdlib.h>
#include <dbus-util.h>
#include <service-adaptor.h>
#include <service-adaptor-log.h>

/* Tizen 3.0 Privilege check with Cynara [--*/
#include <cynara-client.h>
#include <cynara-session.h>
#include <cynara-creds-gdbus.h>
static cynara *_cynara;
/* --] Tizen 3.0 Privilege check with Cynara */

//LCOV_EXCL_START
/**
 * Free string memory
 * @param data Data to be fried
 */
void free_string(gpointer data)
{
	g_free((gchar *) data);
}

/**
 * Adds string into variant builder
 * @param builder Builder
 * @param data String to be added
 */
void safe_g_variant_builder_add_string(GVariantBuilder *builder, const char *data)
{
	if (NULL == data) {
		g_variant_builder_add(builder, "s", "");
	} else {
		g_variant_builder_add(builder, "s", data);
	}
}

void safe_g_variant_builder_add_array_string(GVariantBuilder *builder, const char *data)
{
	if (NULL == data) {
		g_variant_builder_add(builder, "(s)", "");
	} else {
		g_variant_builder_add(builder, "(s)", data);
	}
}

char *ipc_g_variant_dup_string(GVariant *string)
{
	char *ret = g_variant_dup_string(string, NULL);

	if (0 == strcmp(ret, "")) {
		free(ret);
		ret = NULL;
	}

	return ret;
}
//LCOV_EXCL_STOP

int sa_cynara_init()
{
	int ret;
	cynara_configuration *p_conf;
	size_t cache_size = 100;

	if (CYNARA_API_SUCCESS != cynara_configuration_create(&p_conf)) {
		service_adaptor_error("cynara_configuration_create() failed"); //LCOV_EXCL_LINE
		return SERVICE_ADAPTOR_INTERNAL_ERROR_ADAPTOR_INTERNAL; //LCOV_EXCL_LINE
	}
	if (CYNARA_API_SUCCESS != cynara_configuration_set_cache_size(p_conf, cache_size)) {
		service_adaptor_error("cynara_configuration_set_cache_size() failed"); //LCOV_EXCL_LINE
		cynara_configuration_destroy(p_conf); //LCOV_EXCL_LINE
		return SERVICE_ADAPTOR_INTERNAL_ERROR_ADAPTOR_INTERNAL; //LCOV_EXCL_LINE
	}

	ret = cynara_initialize(&_cynara, NULL);

	if (CYNARA_API_SUCCESS != ret) {
		service_adaptor_error("cynara_initialize() Fail(%d)", ret); //LCOV_EXCL_LINE
		cynara_configuration_destroy(p_conf); //LCOV_EXCL_LINE
		return SERVICE_ADAPTOR_INTERNAL_ERROR_ADAPTOR_INTERNAL; //LCOV_EXCL_LINE
	}
	return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
}

void sa_cynara_deinit()
{
	if (_cynara)
		cynara_finish(_cynara);

	_cynara = NULL;
}


int sa_cynara_check(GDBusMethodInvocation *invocation, const char *privilege)
{
	int ret;
	pid_t pid;
	char *user = NULL;
	char *client = NULL;
	char *session = NULL;
	const char *sender = NULL;
	GDBusConnection *conn = NULL;

	conn = g_dbus_method_invocation_get_connection(invocation);
	if (NULL == conn) {
		service_adaptor_error("g_dbus_method_invocation_get_connection() return NULL"); //LCOV_EXCL_LINE
		return SERVICE_ADAPTOR_INTERNAL_ERROR_ADAPTOR_INTERNAL; //LCOV_EXCL_LINE
	}

	sender = g_dbus_method_invocation_get_sender(invocation);
	if (NULL == sender) {
		service_adaptor_error("g_dbus_method_invocation_get_sender() return NULL"); //LCOV_EXCL_LINE
		return SERVICE_ADAPTOR_INTERNAL_ERROR_ADAPTOR_INTERNAL; //LCOV_EXCL_LINE
	}

	ret = cynara_creds_gdbus_get_client(conn, sender, CLIENT_METHOD_SMACK, &client);
	if (CYNARA_API_SUCCESS != ret) {
		service_adaptor_error("cynara_creds_dbus_get_client() Fail(%d)", ret); //LCOV_EXCL_LINE
		return SERVICE_ADAPTOR_INTERNAL_ERROR_ADAPTOR_INTERNAL; //LCOV_EXCL_LINE
	}

	ret = cynara_creds_gdbus_get_user(conn, sender, USER_METHOD_UID, &user);
	if (CYNARA_API_SUCCESS != ret) {
		service_adaptor_error("cynara_creds_dbus_get_user() Fail(%d)", ret); //LCOV_EXCL_LINE
		free(client); //LCOV_EXCL_LINE
		return SERVICE_ADAPTOR_INTERNAL_ERROR_ADAPTOR_INTERNAL; //LCOV_EXCL_LINE
	}

	ret = cynara_creds_gdbus_get_pid(conn, sender, &pid);
	if (CYNARA_API_SUCCESS != ret) {
		service_adaptor_error("cynara_creds_gdbus_get_pid() Fail(%d)", ret); //LCOV_EXCL_LINE
		free(user); //LCOV_EXCL_LINE
		free(client); //LCOV_EXCL_LINE
		return SERVICE_ADAPTOR_INTERNAL_ERROR_ADAPTOR_INTERNAL; //LCOV_EXCL_LINE
	}

	session = cynara_session_from_pid(pid);
	if (NULL == session) {
		service_adaptor_error("cynara_session_from_pid() return NULL"); //LCOV_EXCL_LINE
		free(user); //LCOV_EXCL_LINE
		free(client); //LCOV_EXCL_LINE
		return SERVICE_ADAPTOR_INTERNAL_ERROR_ADAPTOR_INTERNAL; //LCOV_EXCL_LINE
	}

	service_adaptor_debug("privilege: %s, user: %s, client: %s", privilege, user, client);
	ret = cynara_check(_cynara, client, session, user, privilege);
	if (CYNARA_API_ACCESS_DENIED == ret) {
		service_adaptor_error("Denied (%s)", privilege); //LCOV_EXCL_LINE
		free(session); //LCOV_EXCL_LINE
		free(user); //LCOV_EXCL_LINE
		free(client); //LCOV_EXCL_LINE
		return SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED; //LCOV_EXCL_LINE
	} else if (CYNARA_API_ACCESS_ALLOWED != ret) {
		service_adaptor_error("cynara_check(%s) Fail(%d)", privilege, ret); //LCOV_EXCL_LINE
		free(session); //LCOV_EXCL_LINE
		free(user); //LCOV_EXCL_LINE
		free(client); //LCOV_EXCL_LINE
		return SERVICE_ADAPTOR_INTERNAL_ERROR_ADAPTOR_INTERNAL; //LCOV_EXCL_LINE
	}

	free(session);
	free(user);
	free(client);

	return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
}

