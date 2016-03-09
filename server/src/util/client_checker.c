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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdarg.h>

#include <glib.h>
/*
#include <sys/smack.h>
#include <security-server.h>
*/
#include "util/client_checker.h"
#include "service-adaptor-log.h"

/*************************************************
 *               Type definition
 *************************************************/

#define CLIENT_TYPE_APP		1
#define CLIENT_TYPE_ETC		0

#define ERROR_MSG_MAX_LEN 200

typedef struct _client_data_s {
	int type;
	int pid;
	char *service_handle_name;
	char cookie[21];
} client_data_s;


/*************************************************
 *               Global valuable
 *************************************************/

static GHashTable *g_clients = NULL;

static __thread char last_error_message[ERROR_MSG_MAX_LEN] = {0, };

/*************************************************
 *               Internal function prototype
 *************************************************/

/* static client_data_s *__client_data_create(void); */

static void __client_data_free(void *data);

static client_data_s *__client_data_find(const char *_key);

static int __get_dir_path(const char *path, char **dir_path);

static void __set_last_error(const char *format, ...);


/*************************************************
 *               Public function prototype
 *************************************************/

int client_checker_init(void);

void client_checker_deinit(void);

int client_checker_add_client(const char *service_handle_name, const char *cookie);

int client_checker_del_client(const char *service_handle_name);

int client_checker_check_privilege(const char *service_handle_name, const char *privilege);

int client_checker_check_access_rights(const char *service_handle_name, const char *object, const char *access_rights);

int client_checker_check_access_right_read(const char *service_handle_name, const char *path);

int client_checker_check_access_right_write(const char *service_handle_name, const char *path);

int client_checker_check_access_right_create(const char *service_handle_name, const char *path);

const char *clieht_checker_get_last_error(void);

/*************************************************
 *               Internal function definition
 *************************************************/

/*
static client_data_s *__client_data_create(void)
{
	client_data_s *data = (client_data_s *) calloc(1, sizeof(client_data_s));
	if (NULL != data) {
		data->service_handle_name = NULL;
		memset(data->cookie, 0, 21);
	}
	return data;
}
*/

static void __client_data_free(void *data)
{
	if (NULL != data) {
		free(((client_data_s *) data)->service_handle_name);
		memset(((client_data_s *) data)->cookie, 0, 20);
		free((client_data_s *) data);
	}
}

static client_data_s *__client_data_find(const char *_key)
{
	client_data_s *data = (client_data_s *) g_hash_table_lookup(g_clients, _key);

	service_adaptor_debug_func("+++ find data (%p)", data);
	return data;
}

static int __get_dir_path(const char *path, char **dir_path)
{
	if ((NULL == path) || ('/' != path[0]) || (1 >= strlen(path))) {
		/* path must be absolute path (starts with '/') */
		__set_last_error("[Permission checker] Path must be absolute path (starts with '/')");
		return -1;
	}

	char *full_path = strdup(path);
	if (NULL == full_path) {
		/* dup failed */
		__set_last_error("[Permission checker] Internal error");
		return -1;
	} else if (full_path[strlen(full_path) - 1] == '/') {
		/* if full_path is "/abc/d/" -> "abc/d" */
		full_path[strlen(full_path) - 1] = '\0';
	}

	char *base = strrchr(full_path, '/');
	if (base == full_path) {
		/* if full_path is "/abc" */
		*dir_path = strdup("/");
	} else if (NULL != base) {
		/* expected case */
		*dir_path = strndup(full_path, (base - full_path));
	} else {
		__set_last_error("[Permission checker] Invalid path (Can not parse string)");
		free(full_path);
		return -1;
	}
	free(full_path);

	return 0;
}


/*************************************************
 *               Public function definition
 *************************************************/

int client_checker_init(void)
{
	g_clients = g_hash_table_new_full(g_str_hash, g_str_equal, free, __client_data_free);

	if (NULL == g_clients) {
		return -101;
	}
	return 0;
}

void client_checker_deinit(void)
{
	if (NULL != g_clients) {
		g_hash_table_destroy(g_clients);
		g_clients = NULL;
	}
}

int client_checker_add_client(const char *service_handle_name, const char *cookie)
{
	if (NULL == g_clients) {
		return -201;
	}

	if ((NULL == cookie) || ('\0' == cookie[0]) || (NULL == service_handle_name)) {
		return -202;
	}
/*
	client_data_s *data = __client_data_create();
	if (NULL == data) {
		return -203;
	}

	char *key = strdup(service_handle_name);
	if (NULL == key) {
		__client_data_free(data);
		return -204;
	}
	strncpy(data->cookie, cookie, 20);
*/
	/* pid get */
/*
	int pid = security_server_get_cookie_pid(cookie);
	service_adaptor_debug("pid : %d\n", pid);
	data->pid = pid;

	data->service_handle_name = strdup(service_handle_name);

	g_hash_table_insert(g_clients, key, data);
*/
	return 0;
}

int client_checker_del_client(const char *service_handle_name)
{
	if (NULL == g_clients) {
		return -301;
	}

	if (NULL == service_handle_name) {
		return -302;
	}

	g_hash_table_remove(g_clients, service_handle_name);

	return 0;
}

int client_checker_check_privilege(const char *service_handle_name, const char *privilege)
{
	if (NULL == g_clients) {
		return -401;
	}

	if ((NULL == service_handle_name) || (NULL == privilege)) {
		return -402;
	}

	client_data_s *data = __client_data_find(service_handle_name);
	if (NULL == data) {
		return -403;
	}
/*
	int ret = security_server_check_privilege_by_cookie(data->cookie, privilege, "rw");

	service_adaptor_debug_func("+++ [Service Adaptor Client privilege check]");
	service_adaptor_debug_func("+++ handle name : %s", service_handle_name);
	service_adaptor_debug_func("+++ pid : %d", data->pid);
	service_adaptor_debug_func("+++ check privilege : %s", privilege);
	service_adaptor_debug_func("+++ privilege check ret : %d", ret);

	if (ret) {
		fprintf(stderr, "[service-adaptor] User space smack denied : subject pid (%d), check privilege (%s)\n", data->pid, privilege);
		service_adaptor_error("[service-adaptor] User space smack denied : subject pid (%d), check privilege (%s)\n", data->pid, privilege);
		ret = -404;
	} else {
		service_adaptor_debug_func("===== privilege check passed =====");
	}

	return ret;
*/
	return 0;
}

int client_checker_check_access_rights(const char *service_handle_name, const char *object, const char *access_rights)
{
	if (NULL == g_clients) {
		return -501;
	}

	if ((NULL == service_handle_name) || (NULL == object) || (NULL == access_rights)) {
		return -502;
	}

	client_data_s *data = __client_data_find(service_handle_name);
	if (NULL == data) {
		return -503;
	}
/*
	int ret = security_server_check_privilege_by_cookie(data->cookie, object, access_rights);

	service_adaptor_debug_func("+++ [Service Adaptor Client access right check]");
	service_adaptor_debug_func("+++ handle name : %s", service_handle_name);
	service_adaptor_debug_func("+++ pid : %d", data->pid);
	service_adaptor_debug_func("+++ check object : %s, access_rights : %s", object, access_rights);
	service_adaptor_debug_func("+++ privilege check ret : %d", ret);

	if (ret) {
		fprintf(stderr, "[service-adaptor] User space smack denied : subject pid (%d), check object (%s), access rights (%s)\n",
				data->pid, object, access_rights);
		service_adaptor_error("[service-adaptor] User space smack denied : subject pid (%d), check object (%s), access rights (%s)\n",
				data->pid, object, access_rights);
		ret = -504;
	} else {
		service_adaptor_debug_func("===== access rights check passed =====");
	}

	return ret;
*/
	return 0;
}

int client_checker_check_access_right_read(const char *service_handle_name, const char *path)
{
	int ret;
	char *target_label = NULL;
	/* char *check_permission = NULL; */
	service_adaptor_debug("service_handle(%s), path(%s)", service_handle_name, path);
	if (NULL == path) {
		__set_last_error("[Permission checker] Invalid path");
		return -601;
	}

	/* Gets dir path */
	char *dir_path = NULL;
	ret = __get_dir_path(path, &dir_path);
	if (ret) {
		service_adaptor_error("path error (%s)", path);
		return -602;
	}

	/* Check to 'x' permission to dir_path */
/*
	check_permission = "x";
	service_adaptor_info("[serice-adaptor] check dir execute permission : <path : %s> <right : %s>", dir_path, check_permission);
	target_label = NULL;
	ret = smack_getlabel(dir_path, &target_label, SMACK_LABEL_ACCESS);
	if (ret || (NULL == target_label)) {
		__set_last_error("[Permission checker] Security server internal (%d)", ret);
		service_adaptor_error("file(%s) label get error (%d)", dir_path, ret);
		free(dir_path);
		free(target_label);
		return -604;
	}

	service_adaptor_info("target path (%s) label(%s)", dir_path, target_label);
	ret = client_checker_check_access_rights(service_handle_name, target_label, check_permission);

	if (ret) {
		__set_last_error("[Permission checker] Folder Access denied rights<%s> label<%s>", target_label, check_permission);
		fprintf(stderr, "[service-adaptor] smack permission denied : path (%s), right (%s)\n",
				dir_path, check_permission);
		service_adaptor_error("[service-adaptor] smack permission denied : path (%s), right (%s)\n",
				dir_path, check_permission);
		free(dir_path);
		free(target_label);
		return -605;
	} else {
		service_adaptor_info("===== dir path check passed =====");
	}
*/
	free(dir_path);

	/* Check to 'r' permission to path */
/*
	check_permission = "r";
	service_adaptor_info("[serice-adaptor] check path execute permission : <path : %s> <right : %s>", path, check_permission);
	free(target_label);
	target_label = NULL;
	ret = smack_getlabel(path, &target_label, SMACK_LABEL_ACCESS);
	if (ret || (NULL == target_label)) {
		__set_last_error("[Permission checker] Security server internal (%d)", ret);
		service_adaptor_error("file(%s) label get error (%d)", path, ret);
		free(target_label);
		return -606;
	}

	service_adaptor_info("target path (%s) label(%s)", path, target_label);
	ret = client_checker_check_access_rights(service_handle_name, target_label, "r");

	if (ret) {
		__set_last_error("[Permission checker] Access denied rights<%s> label<%s>", target_label, check_permission);
		fprintf(stderr, "[service-adaptor] smack permission denied : path (%s), right (%s)\n",
				path, check_permission);
		service_adaptor_error("[service-adaptor] smack permission denied : path (%s), right (%s)\n",
				path, check_permission);
		free(target_label);
		return -607;
	} else {
		service_adaptor_info("===== path check passed =====");
	}
*/
	free(target_label);
	return ret;
}

int client_checker_check_access_right_write(const char *service_handle_name, const char *path)
{
	/* TODO */
	return 0;
}

int client_checker_check_access_right_create(const char *service_handle_name, const char *path)
{
	int ret;
	/* char *target_label = NULL; */
	/* char *check_permission = NULL; */
	service_adaptor_debug("service_handle(%s), path(%s)", service_handle_name, path);
	if (NULL == path) {
		return -801;
	}

	/* Gets dir path */
	char *dir_path = NULL;
	ret = __get_dir_path(path, &dir_path);
	if (ret) {
		service_adaptor_error("path error (%s)", path);
		return -802;
	}

	/* Check to 'rwx' permission to dir_path */
/*
	check_permission = "rwx";
	service_adaptor_info("[serice-adaptor] check dir execute permission : <path : %s> <right : %s>", dir_path, check_permission);
	target_label = NULL;
	ret = smack_getlabel(dir_path, &target_label, SMACK_LABEL_ACCESS);
	if (ret || (NULL == target_label)) {
		service_adaptor_error("file(%s) label get error (%d)", dir_path, ret);
		free(dir_path);
		free(target_label);
		return -804;
	}

	service_adaptor_info("target path (%s) label(%s)", dir_path, target_label);
	ret = client_checker_check_access_rights(service_handle_name, target_label, check_permission);

	if (ret) {
		fprintf(stderr, "[service-adaptor] smack permission denied : path (%s), right (%s)\n",
				dir_path, check_permission);
		service_adaptor_error("[service-adaptor] smack permission denied : path (%s), right (%s)\n",
				dir_path, check_permission);
		free(dir_path);
		free(target_label);
		return -805;
	} else {
		service_adaptor_info("===== dir path check passed =====");
	}

	free(target_label);
*/
	free(dir_path);

	return ret;
}


static void __set_last_error(const char *format, ...)
{
	va_list args;
	va_start(args, format);

	vsnprintf(last_error_message, ERROR_MSG_MAX_LEN, format, args);

	va_end(args);
}

const char *clieht_checker_get_last_error(void)
{
	return last_error_message;
}
