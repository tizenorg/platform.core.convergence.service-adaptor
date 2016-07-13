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

#include <gio/gio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <glib-object.h>

#include <dbus-server.h>

#include "service_adaptor_client_log.h"
#include "dbus_client.h"
#include "util/service_adaptor_client_util.h"

#include <bundle.h>

/* LCOV_EXCL_START */
/**	@brief	NULL-pointer safe string duplication function
 *		This function will not crash if source string pointer is NULL. It is user's
 *		responsibility to free the result pointer.
 *	@param str pointer to string which should be duplicated
 *	@return	pointer to the duplicated string
 *	@remarks :
 */
char *_safe_strdup(const char *str)
{
	if (NULL == str) {
		return NULL;
	} else {
		return strdup(str);
	}
}

void __set_error_code(service_adaptor_error_s **error_code, int code, const char *msg)
{
	if (NULL != error_code) {
		service_adaptor_error_s *_error = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));
		if (NULL != _error) {
			_error->code = code;
			_error->msg = strdup(msg);
		}
		*error_code = _error;
	}
}

void __assign_error_code(service_adaptor_error_s *source_error, service_adaptor_error_s **target_error)
{
	service_adaptor_error_s *_error = (service_adaptor_error_s *) calloc(1, sizeof(service_adaptor_error_s));
	if ((NULL != _error) && (NULL != target_error)) {
		_error->code = source_error->code;
		_error->msg = source_error->msg;
		*target_error = _error;
	} else {
		free(_error);
		free(source_error->msg);
	}
}

int _get_result_code(long long int error_code)
{
	switch (error_code) {
	case SERVICE_ADAPTOR_PROTOCOL_RETURN_CODE_NONE:
		return SERVICE_ADAPTOR_ERROR_NONE;
	case SERVICE_ADAPTOR_PROTOCOL_RETURN_CODE_COMMON_NO_DATA:
		sac_error("SERVICE_ADAPTOR_ERROR_NO_DATA");
		return SERVICE_ADAPTOR_ERROR_NO_DATA;
	case SERVICE_ADAPTOR_PROTOCOL_RETURN_CODE_COMMON_TIMED_OUT:
		sac_error("SERVICE_ADAPTOR_ERROR_TIMED_OUT");
		return SERVICE_ADAPTOR_ERROR_TIMED_OUT;
	case SERVICE_ADAPTOR_PROTOCOL_RETURN_CODE_COMMON_NOT_SUPPORTED:
		sac_error("SERVICE_ADAPTOR_ERROR_NOT_SUPPORTED");
		return SERVICE_ADAPTOR_ERROR_NOT_SUPPORTED;
	case SERVICE_ADAPTOR_PROTOCOL_RETURN_CODE_COMMON_PERMISSION_DENIED:
		sac_error("SERVICE_ADAPTOR_ERROR_PERMISSION_DENIED");
		return SERVICE_ADAPTOR_ERROR_PERMISSION_DENIED;

	case SERVICE_ADAPTOR_PROTOCOL_RETURN_CODE_AUTH_NOT_AUTHORIZED:
		sac_error("SERVICE_ADAPTOR_ERROR_NOT_AUTHORIZED");
		return SERVICE_ADAPTOR_ERROR_NOT_AUTHORIZED;

	case SERVICE_ADAPTOR_PROTOCOL_RETURN_CODE_MESSAGE_NETWORK:
		sac_error("SERVICE_ADAPTOR_ERROR_NETWORK");
		return SERVICE_ADAPTOR_ERROR_NETWORK;

	default:
		sac_error("SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED");
		return SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED;
	}
}
/* LCOV_EXCL_STOP */

int _ipc_get_simple_result(GVariant *call_result, GError *g_error, service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	if (NULL == call_result) {
		/* LCOV_EXCL_START */
		error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		if (NULL != g_error) {
			sac_error("G_IO_ERROR DEBUG (%d)", (int)(g_error->code));
			if (g_error->code == G_IO_ERROR_TIMED_OUT) {
				ret = SERVICE_ADAPTOR_ERROR_TIMED_OUT;
			}
			error->msg = __SAFE_STRDUP(g_error->message);
			g_error_free(g_error);
		}
		/* LCOV_EXCL_STOP */
	} else {
		if (FALSE == g_variant_is_of_type(call_result, G_VARIANT_TYPE("(ts)"))) {
			/* LCOV_EXCL_START */
			error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->msg = strdup("D-Bus return type error");
			ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			/* LCOV_EXCL_STOP */
		} else {
			GVariant *call_result_struct[2];
			call_result_struct[0] = g_variant_get_child_value(call_result, 0);
			call_result_struct[1] = g_variant_get_child_value(call_result, 1);

			uint64_t remote_call_result = g_variant_get_uint64(call_result_struct[0]);
			if (SERVICE_ADAPTOR_ERROR_NONE != remote_call_result) {
				/* LCOV_EXCL_START */
				error->code = remote_call_result;
				error->msg = ipc_g_variant_dup_string(call_result_struct[1]);
				ret = _get_result_code(remote_call_result);
				/* LCOV_EXCL_STOP */
			}
			g_variant_unref(call_result_struct[0]);
			g_variant_unref(call_result_struct[1]);
		}
		g_variant_unref(call_result);
	}

	return ret;
}

static void __g_hash_iterator(gpointer key, gpointer value, gpointer user_data)
{
	if (key && value && user_data) {
		bundle_add_str((bundle *)user_data, (const char *)key, (const char *)value);
	}
}

void _create_raw_data_from_plugin_property(void *property, GVariantBuilder **builder)
{
	if ((NULL == property) || (NULL == builder)) {
		return;
	}

	*builder = g_variant_builder_new(G_VARIANT_TYPE(service_adaptor_raw_data_s_type));

	bundle *bd = bundle_create();

	if (NULL != bd) {
		g_hash_table_foreach((GHashTable *)property, __g_hash_iterator, (gpointer)bd);
		unsigned char *input_str = NULL;
		int input_len = 0;

		int ret = bundle_encode(bd, &input_str, &input_len);
		if (!ret && input_str && input_len) {
			for (int k = 0; k < input_len; k++) {
				g_variant_builder_add((*builder), "(y)", (guchar)input_str[k]);
			}
		}

		sac_debug_func("input_str_len(%d)", input_len);
		free(input_str);
		input_str = NULL;
		bundle_free(bd);
		bd = NULL;
	}
}

