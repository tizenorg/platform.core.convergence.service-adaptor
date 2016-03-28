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

#ifndef __SERVICE_ADAPTOR_H__
#define __SERVICE_ADAPTOR_H__

#include <stdbool.h>
#include <glib.h>

typedef enum {
	SERVICE_ADAPTOR_INTERNAL_ERROR_NONE			= 0,
	SERVICE_ADAPTOR_INTERNAL_ERROR_LAUNCH                    = 1,    /**< 1 ~ 99: internal error*/
	SERVICE_ADAPTOR_INTERNAL_ERROR_INIT                      = 2,
	SERVICE_ADAPTOR_INTERNAL_ERROR_DEINIT                    = 3,
	SERVICE_ADAPTOR_INTERNAL_ERROR_CREATE                    = 4,
	SERVICE_ADAPTOR_INTERNAL_ERROR_DESTROY                   = 5,
	SERVICE_ADAPTOR_INTERNAL_ERROR_START                     = 6,
	SERVICE_ADAPTOR_INTERNAL_ERROR_STOP                      = 7,
	SERVICE_ADAPTOR_INTERNAL_ERROR_CONNECT                   = 8,
	SERVICE_ADAPTOR_INTERNAL_ERROR_DISCONNECT                = 9,
	SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_FOUND                 = 10,
	SERVICE_ADAPTOR_INTERNAL_ERROR_CORRUPTED                 = 11,
	SERVICE_ADAPTOR_INTERNAL_ERROR_UNSUPPORTED               = 12,
	SERVICE_ADAPTOR_INTERNAL_ERROR_INVALID_HANDLE            = 13,
	SERVICE_ADAPTOR_INTERNAL_ERROR_INVALID_ARGUMENT          = 14,
	SERVICE_ADAPTOR_INTERNAL_ERROR_INVALID_ARGUMENT_TYPE     = 15,
	SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED            = 16,
	SERVICE_ADAPTOR_INTERNAL_ERROR_ADAPTOR_INTERNAL          = 17,
	SERVICE_ADAPTOR_INTERNAL_ERROR_PLUGIN_INTERNAL           = 18,
	SERVICE_ADAPTOR_INTERNAL_ERROR_SERVER_INTERNAL           = 19,
	SERVICE_ADAPTOR_INTERNAL_ERROR_DBUS                      = 20,
	SERVICE_ADAPTOR_INTERNAL_ERROR_CALLBACK_TIME_OUT         = 21,
	SERVICE_ADAPTOR_INTERNAL_ERROR_INTERNAL_MAX              = 99,
	SERVICE_ADAPTOR_INTERNAL_ERROR_NO_DATA,
	SERVICE_ADAPTOR_INTERNAL_ERROR_MAX
} service_adaptor_internal_error_code_e;

typedef enum {
	SERVICE_ADAPTOR_INTERNAL_RESULT_SUCCEEDED		= 0,
	SERVICE_ADAPTOR_INTERNAL_RESULT_FAILED			= -1,
	SERVICE_ADAPTOR_INTERNAL_RESULT_CANCELED		= -2,
	SERVICE_ADAPTOR_INTERNAL_RESULT_MIN,

} service_adaptor_internal_result_e;

typedef enum {
	SERVICE_ADAPTOR_INTERNAL_SIGNAL_NONE                     = 0,    /**< specifies status as none*/
	SERVICE_ADAPTOR_INTERNAL_SIGNAL_INITIALIZED              = 1,    /**< specifies status as none*/
	SERVICE_ADAPTOR_INTERNAL_SIGNAL_NEW_PLUGIN               = 2,    /**< specifies status as none*/
	SERVICE_ADAPTOR_INTERNAL_SIGNAL_ACTIVATE_PLUGIN          = 3,    /**< specifies status as none*/
	SERVICE_ADAPTOR_INTERNAL_SIGNAL_SHUTDOWN                 = 4,    /**< specifies status as none*/
	SERVICE_ADAPTOR_INTERNAL_SIGNAL_OTHER                    = 99,   /**< specifies status as none*/
	SERVICE_ADAPTOR_INTERNAL_SIGNAL_UNKNOWN                  = 999,  /**< specifies status as none*/
	SERVICE_ADAPTOR_INTERNAL_SIGNAL_MAX,

} service_adaptor_internal_signal_code_e;

typedef struct service_adaptor_internal_error_s {
	long long int code;
	char *msg;

} service_adaptor_internal_error_t;
typedef struct service_adaptor_internal_error_s *service_adaptor_internal_error_h;

typedef struct service_adaptor_internal_plugin_s {
	char *name;
	bool login;
} service_adaptor_internal_plugin_t;
typedef struct service_adaptor_internal_plugin_s *service_adaptor_internal_plugin_h;

typedef int(*service_daptor_internal_signal_cb)(service_adaptor_internal_signal_code_e signal, char *msg);

#endif /* __SERVICE_ADAPTOR_H__ */
