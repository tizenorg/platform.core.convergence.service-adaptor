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

#include "service-adaptor.h"
#include "service-adaptor-storage.h"
#include "service-adaptor-type.h"
#include "service-adaptor-log.h"
#include "dbus-ipc.h"
#include "dbus-server.h"
#include "dbus-service-adaptor.h"
#include "dbus-storage-adaptor.h"
#include "storage-adaptor.h"

/*#define STORAGE_PLUGIN_PATH	"/usr/lib/storage-adaptor/plugins"*/

/***********************************************************
 * Storage adaptor callback
 ***********************************************************/
/* private feature */

//LCOV_EXCL_START
void service_adaptor_storage_adaptor_download_file_async_cb(void *download_request_id,
						char *download_file_local_path,
						storage_adaptor_error_code_h error,
						void *response)
{
	service_adaptor_debug("Storage adaptor download finished");
	service_adaptor_info("Download path: %s", download_file_local_path);

	storage_adaptor_error_code_t _error;
	_error.code = 0LL;
	_error.msg = NULL;

	if (NULL == error) {
		error = &_error;
	}

	if (0 >= (int32_t) download_request_id) {
		service_adaptor_error("%s (%lld)", error->msg, error->code);
		return;
	}

	private_dbus_storage_file_transfer_completion_callback((int32_t) download_request_id, NULL, error, NULL);
}

void service_adaptor_storage_adaptor_upload_file_async_cb(void *upload_request_id,
						storage_adaptor_file_info_h file_info,
						storage_adaptor_error_code_h error,
						void *response)
{
	service_adaptor_debug("Storage adaptor upload finished");

	if ((NULL != file_info) && (NULL != file_info->storage_path)) {
		service_adaptor_info("Upload path: %s", file_info->storage_path);
	}

	storage_adaptor_error_code_t _error;
	_error.code = 0LL;
	_error.msg = NULL;

	if (NULL == error) {
		error = &_error;
	}

	if (0 >= (int32_t) upload_request_id) {
		service_adaptor_error("%s (%lld)", error->msg, error->code);
		return;
	}

	char *publish_url = NULL;
	char publish_url_str[1000] = {0,};

	if ((NULL != file_info) && (NULL != file_info->file_share_token)) {
		snprintf(publish_url_str, 1000, "%s?auth_code=%s", file_info->file_share_token->public_token, file_info->file_share_token->auth_code);
		publish_url = publish_url_str;
	}

	private_dbus_storage_file_transfer_completion_callback((int32_t) upload_request_id, publish_url, error, NULL);
}

void service_adaptor_storage_adaptor_file_transfer_progress_cb(void *transfer_request_id, unsigned long long progress_size, unsigned long long total_size, storage_adaptor_error_code_h error, void *response)
{
	service_adaptor_debug("Storage adaptor transfer progress");
	service_adaptor_debug("\t===transfer id   : %d", (int)transfer_request_id);
	service_adaptor_debug("\t===progress size : %llu", progress_size);
	service_adaptor_debug("\t===total size    : %llu", total_size);

	storage_adaptor_error_code_t _error;
	_error.code = 0LL;
	_error.msg = NULL;

	if (NULL == error) {
		error = &_error;
	}

	if (0 >= (int32_t) transfer_request_id) {
		service_adaptor_error("%s (%lld)", error->msg, error->code);
		return;
	}

	private_dbus_storage_file_progress_callback((int32_t) transfer_request_id, progress_size, total_size, error, NULL);
}

/* public feature */
void service_adaptor_storage_adaptor_download_state_changed_cb(long long int file_uid,
						storage_adaptor_transfer_state_e state,
						storage_adaptor_error_code_h error,
						void *user_data)
{
	service_adaptor_debug("Storage adaptor download finished");

	storage_adaptor_error_code_t _error;
	_error.code = 0LL;
	_error.msg = NULL;

	if (NULL == error) {
		error = &_error;
	}

	dbus_storage_file_transfer_state_changed_callback(file_uid, state, error);
}

void service_adaptor_storage_adaptor_upload_state_changed_cb(long long int file_uid,
						storage_adaptor_transfer_state_e state,
						storage_adaptor_file_info_h file_info,
						storage_adaptor_error_code_h error,
						void *user_data)
{
	service_adaptor_debug("Storage adaptor upload finished");

	storage_adaptor_error_code_t _error;
	_error.code = 0LL;
	_error.msg = NULL;

	if (NULL == error) {
		error = &_error;
	}

	dbus_storage_file_transfer_state_changed_callback(file_uid, state, error);
}

void service_adaptor_storage_adaptor_task_progress_cb(long long int file_uid,
						unsigned long long progress_size,
						unsigned long long total_size)
{
	service_adaptor_debug("Storage adaptor transfer progress");

	service_adaptor_debug("\t===transfer id   : %lld", file_uid);
	service_adaptor_debug("\t===progress size : %llu", progress_size);
	service_adaptor_debug("\t===total size    : %llu", total_size);

	dbus_storage_file_progress_callback(file_uid, progress_size, total_size);
}

storage_adaptor_h service_adaptor_get_storage_adaptor(service_adaptor_h service_adaptor)
{
	service_adaptor_debug("Get storage adaptor");

	if ((void *) NULL == service_adaptor) {
		service_adaptor_error("Invalid argument");
		return NULL;
	}

	return service_adaptor->storage_handle;
}

service_adaptor_internal_error_code_e service_adaptor_connect_storage_plugin(service_adaptor_h service_adaptor,
						service_adaptor_service_context_h service,
						const char *app_secret,
						char *ret_msg)
{
	service_adaptor_debug("Connect to storage plugin");

	if ((NULL == service_adaptor) || (NULL == service)) {
		service_adaptor_error("Invalid parameter");
		snprintf(ret_msg, 2048, "storage plugin connect failed [%d]", (int)__LINE__);
		return SERVICE_ADAPTOR_INTERNAL_ERROR_INVALID_ARGUMENT;
	}

	storage_adaptor_h adaptor = service_adaptor_get_storage_adaptor(service_adaptor);
	storage_adaptor_plugin_h plugin = storage_adaptor_get_plugin_by_name(adaptor, service->plugin_uri);

	if ((NULL == service->context_info) || (NULL == service->service_name)) {
		service_adaptor_error("Invalid service_name or plugin_uri: %s, %s",
				service->service_name, service->plugin_uri);
		snprintf(ret_msg, 2048, "storage plugin connect failed [%d]", (int)__LINE__);
		return SERVICE_ADAPTOR_INTERNAL_ERROR_INVALID_ARGUMENT;
	}

	storage_adaptor_plugin_context_h storage_context = storage_adaptor_create_plugin_context(
			plugin, service->context_info->app_id, app_secret,
			service->context_info->access_token, service->context_info->user_id,
			service->context_info->duid, service->service_name);

	if (NULL == storage_context) {
		service_adaptor_debug_func("Could not get storage plugin context: %s", service->service_name);
		snprintf(ret_msg, 2048, "storage plugin connect failed [%d]", (int)__LINE__);
		return SERVICE_ADAPTOR_INTERNAL_ERROR_CORRUPTED;
	}

	/* Set server info */
	int ret = 0;
	storage_adaptor_error_code_h error = NULL;
	SERVICE_ADAPTOR_API_TIME_CHECK_PAUSE();
	SERVICE_ADAPTOR_PLUGIN_API_TIME_CHECK_START(SA_TIME_CHECK_FLAG_STORAGE);
	ret = storage_adaptor_set_server_info(plugin, storage_context, service->server_info, NULL, &error, NULL);
	SERVICE_ADAPTOR_PLUGIN_API_TIME_CHECK_PAUSE(SA_TIME_CHECK_FLAG_STORAGE);
	SERVICE_ADAPTOR_API_TIME_CHECK_START();
	if (STORAGE_ADAPTOR_ERROR_NONE != ret) {
		service_adaptor_warning("Could not set storage plugin server information: %d", ret);
		if (NULL != error) {
			service_adaptor_warning("[%lld] %s", error->code, error->msg);
		}
		storage_adaptor_destroy_error_code(&error);
	}

	service->storage_context = storage_context;
	service->connected |= 0x0100000;

	service_adaptor_debug("Connected to storage plugin");

	return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
}

service_adaptor_internal_error_code_e service_adaptor_disconnect_storage_plugin(service_adaptor_h service_adaptor,
						service_adaptor_service_context_h service)
{
	service_adaptor_debug("Disconnect from storage plugin");

	service_adaptor_debug("get storage adaptor");
	storage_adaptor_h storage_adaptor = service_adaptor_get_storage_adaptor(service_adaptor);
	if ((NULL != service->storage_context) && (NULL != storage_adaptor)) {
		service_adaptor_debug("disconnects storage");
		FUNC_STEP();
		storage_adaptor_plugin_h storage_plugin = storage_adaptor_get_plugin_by_name(storage_adaptor, service->storage_context->plugin_uri);

		if (NULL == storage_plugin) {
			service_adaptor_error("Cannot find plugin");
		} else {
			service_adaptor_debug("dsetroys storage context");
			storage_adaptor_destroy_plugin_context(storage_plugin, service->storage_context);
			service->storage_context = NULL;
		}
	}

	service_adaptor_debug("Disconnected from storage plugin");

	return SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;
}
//LCOV_EXCL_STOP

storage_adaptor_h service_adaptor_create_storage()
{
	storage_adaptor_h storage_adaptor = storage_adaptor_create(STORAGE_PLUGIN_PATH);

	if (NULL == storage_adaptor) {
		service_adaptor_error("Could not create storage adaptor"); //LCOV_EXCL_LINE
		return NULL; //LCOV_EXCL_LINE
	}

	service_adaptor_debug("Storage adaptor created");

	return storage_adaptor;
}

storage_adaptor_listener_h service_adaptor_register_storage_listener(storage_adaptor_h storage_adaptor)
{
	if (NULL == storage_adaptor) {
		service_adaptor_error("Could not create storage adaptor"); //LCOV_EXCL_LINE
		return NULL; //LCOV_EXCL_LINE
	}

	storage_adaptor_listener_h storage_listener =
		(storage_adaptor_listener_h) malloc(sizeof(storage_adaptor_listener_t));

	if ((void *) NULL == storage_listener) {
		service_adaptor_error("Could not create storage listener"); //LCOV_EXCL_LINE
		return NULL; //LCOV_EXCL_LINE
	}

	/* private feature */
	storage_listener->download_file_async_reply = service_adaptor_storage_adaptor_download_file_async_cb;
	storage_listener->upload_file_async_reply   = service_adaptor_storage_adaptor_upload_file_async_cb;
	storage_listener->file_transfer_progress_reply = service_adaptor_storage_adaptor_file_transfer_progress_cb;

	/* public feature */
	storage_listener->download_state_changed_reply = service_adaptor_storage_adaptor_download_state_changed_cb;
	storage_listener->upload_state_changed_reply   = service_adaptor_storage_adaptor_upload_state_changed_cb;
	storage_listener->task_progress_reply = service_adaptor_storage_adaptor_task_progress_cb;

	storage_adaptor_register_listener(storage_adaptor, storage_listener);
	service_adaptor_debug("Storage adaptor listener created");

	return storage_listener;
}
