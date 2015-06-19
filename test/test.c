#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib-object.h>
#include <glib-unix.h>

#include "service_adaptor.h"

service_plugin_h service_plugin = NULL;

void _service_auth_oauth1_cb(int result, service_auth_oauth1_h oauth1, void *user_data)
{
	char *access_token = NULL;
	service_auth_oauth1_get_access_token(oauth1, &access_token);

	printf("access_token = %s (%s)\n", access_token, (char *) user_data);
}

bool _service_storage_cloud_file_cb(int result, service_storage_cloud_file_h file, void *user_data)
{
	char *cloud_path = NULL;
	service_storage_cloud_file_get_cloud_path(file, &cloud_path);

	printf("cloud_path = %s (%s)\n", cloud_path, (char *) user_data);

	return true;
}

void _service_plugin_login_callback(int result, void *user_data)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	ret = service_plugin_start(service_plugin);

	if (SERVICE_ADAPTOR_ERROR_NONE != ret)
	{
		printf("service_plugin_start Failed(%d)\n", ret);
		return;
	}

	/* operation */
	service_auth_oauth1_h oauth1 = NULL;
	ret = service_auth_oauth1_create(service_plugin, &oauth1);
	ret = service_auth_oauth1_set_callback(oauth1, _service_auth_oauth1_cb, NULL);
	ret = service_auth_oauth1_set_operation(oauth1, SERVICE_AUTH_OAUTH1_0_GET_ACCESS_TOKEN_URI);

	service_task_h auth_task = NULL;
	ret = service_auth_oauth1_create_task(oauth1, &auth_task);
	ret = service_task_start(auth_task);
	if (SERVICE_ADAPTOR_ERROR_NONE == ret)
	{
		printf("service_auth_oauth1_get_access_token() Request Successed\n");
	}

	char *cloud_path = "/root/cloud";
	service_storage_cloud_file_h file = NULL;
	ret = service_storage_cloud_file_create(service_plugin, &file);
	ret = service_storage_cloud_file_set_callback(file, _service_storage_cloud_file_cb, NULL);
	ret = service_storage_cloud_file_set_cloud_path(file, cloud_path);
	ret = service_storage_cloud_file_set_operation(file, SERVICE_STORAGE_CLOUD_REMOVE_FILE_URI);

	service_task_h task = NULL;
	ret = service_storage_cloud_file_create_task(file, &task);
	ret = service_task_start(task);
	if (SERVICE_ADAPTOR_ERROR_NONE == ret)
	{
		printf("service_storage_cloud_remove_file() Request Successed: %s\n", cloud_path);
	}
}

bool _service_adaptor_plugin_callback(const char *uri, int service_mask, void *user_data)
{
	if (0 != strcmp(uri, "org.tizen.service-plugin-sample"))
	{
		return true;
	}

	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	ret = service_plugin_create(uri, &service_plugin);

	if (SERVICE_ADAPTOR_ERROR_NONE != ret)
	{
		printf("service_plugin_create(%s) Failed(%d)\n", uri, ret);
		return true;
	}

	ret = service_plugin_login(service_plugin, _service_plugin_login_callback, NULL);

	if (SERVICE_ADAPTOR_ERROR_NONE != ret)
	{
		printf("service_plugin_login(%s) Failed(%d)\n", uri, ret);
		return true;
	}

	return true;
}

static gint _sigterm_callback(void *data)
{
	g_main_loop_quit((GMainLoop*)data);

	return 0;
}

int main()
{
	GMainLoop *loop = NULL;

#if !GLIB_CHECK_VERSION(2,32,0)
	g_thread_init(NULL);
#endif
#if !GLIB_CHECK_VERSION(2,35,0)
	g_type_init();
#endif

	/* init */
	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	ret = service_adaptor_connect();
	ret = service_adaptor_foreach_plugin(_service_adaptor_plugin_callback, NULL);

	loop = g_main_loop_new(NULL, FALSE);

	g_unix_signal_add_full(G_PRIORITY_HIGH, SIGINT,
			_sigterm_callback, loop, NULL );
	g_unix_signal_add_full(G_PRIORITY_HIGH, SIGTERM,
			_sigterm_callback, loop, NULL );

	g_main_loop_run(loop);

	g_main_loop_unref(loop);

	/* deinit */
	ret = service_plugin_stop(service_plugin);
	ret = service_plugin_destroy(service_plugin);
	ret = service_adaptor_disconnect();

	return ret;
}
