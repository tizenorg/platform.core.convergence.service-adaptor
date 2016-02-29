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

#ifndef __TIZEN_SOCIAL_SERVICE_ADAPTOR_UTIL_H__
#define __TIZEN_SOCIAL_SERVICE_ADAPTOR_UTIL_H__

#include <glib.h>
#include <glib-object.h>
#include "service_adaptor_client_type.h"

#ifdef __cplusplus
extern "C" {
#endif

char *_safe_strdup(const char *str);

void __set_error_code(service_adaptor_error_s **error_code, int code, const char *msg);

void __assign_error_code(service_adaptor_error_s *source_error, service_adaptor_error_s **target_error);

int _get_result_code(long long int error_code);

void _create_raw_data_from_plugin_property(void *property, GVariantBuilder **builder);

static const int dbus_default_timeout_msec = 5000;

#define sac_safe_add_string(str)	((str) ? (str) : "")

#define	ipc_check_proxy(proxy)	do { \
		if (NULL == (proxy)) { \
			error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE; \
			error->msg = strdup("D-Bus interface proxy has NOT been initialized"); \
			return SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE; \
		} \
	} while (0)

int _ipc_get_simple_result(GVariant *call_result, GError *g_error, service_adaptor_error_s *error);

#define _ipc_get_complex_result(expected_type, __DO_WORK)	do { \
	if (NULL == call_result) { \
		error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE; \
		ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE; \
\
		if (NULL != g_error) { \
			sac_error("G_IO_ERROR DEBUG (%d)", (int)(g_error->code)); \
			if (g_error->code == G_IO_ERROR_TIMED_OUT) { \
				ret = SERVICE_ADAPTOR_ERROR_TIMED_OUT; \
			} \
			error->msg = __SAFE_STRDUP(g_error->message); \
			g_error_free(g_error); \
		} \
	} else { \
		if (FALSE == g_variant_is_of_type(call_result, G_VARIANT_TYPE((expected_type)))) { \
			error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE; \
			error->msg = strdup("D-Bus return type error"); \
			ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE; \
		} else { \
			GVariant *call_result_struct[3]; \
			call_result_struct[0] = g_variant_get_child_value(call_result, 0); \
			call_result_struct[1] = g_variant_get_child_value(call_result, 1); \
			call_result_struct[2] = g_variant_get_child_value(call_result, 2); \
\
			uint64_t remote_call_result = g_variant_get_uint64(call_result_struct[1]); \
\
			if (SERVICE_ADAPTOR_ERROR_NONE != remote_call_result) { \
				error->code = remote_call_result; \
				error->msg = ipc_g_variant_dup_string(call_result_struct[2]); \
				ret = _get_result_code(remote_call_result); \
			} else { \
				__DO_WORK \
			} \
\
			g_variant_unref(call_result_struct[0]); \
			g_variant_unref(call_result_struct[1]); \
			g_variant_unref(call_result_struct[2]); \
		} \
\
		g_variant_unref(call_result); \
	} \
} while (0)

#define sac_check_param_null(val, val_name) {\
	if (NULL == (val)) {\
		sac_error ("\"%s\" is NULL, return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER(%d)", \
				(const char *)val_name, (int)SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER);\
		return SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER;\
	}\
}

#ifdef __cplusplus
}
#endif
#endif /* __TIZEN_SOCIAL_SERVICE_ADAPTOR_UTIL_H__ */
