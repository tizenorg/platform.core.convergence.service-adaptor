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

#ifndef __DBUS_STORAGE_ADAPTOR_H__
#define __DBUS_STORAGE_ADAPTOR_H__

void storage_adaptor_method_call(GDBusConnection *connection,
						const gchar *sender,
						const gchar *object_path,
						const gchar *interface_name,
						const gchar *method_name,
						GVariant *parameters,
						GDBusMethodInvocation *invocation,
						gpointer user_data);

/* private feature */
service_adaptor_internal_error_code_e private_dbus_storage_file_progress_callback(int32_t fd,
						uint64_t progress_size,
						uint64_t total_size,
						storage_adaptor_error_code_h error_code,
						void *server_data);

service_adaptor_internal_error_code_e private_dbus_storage_file_transfer_completion_callback(int32_t fd,
						char *publish_url,
						storage_adaptor_error_code_h error_code,
						void *server_data);

/* public feature */
service_adaptor_internal_error_code_e dbus_storage_file_progress_callback(long long int file_uid,
						unsigned long long progress_size,
						unsigned long long total_size);

service_adaptor_internal_error_code_e dbus_storage_file_transfer_state_changed_callback(long long int file_uid,
						storage_adaptor_transfer_state_e state,
						storage_adaptor_error_code_h _error_code);

#endif /* __DBUS_STORAGE_ADAPTOR_H__ */
