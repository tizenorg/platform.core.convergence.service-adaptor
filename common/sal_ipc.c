/*
 * Service Adaptor IPC
 *
 * Copyright (c) 2014 - 2015 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <glib.h>
#include <gio/gio.h>

#include "service_adaptor_errors.h"
#include "service_adaptor_internal.h"
#include "sal_ipc.h"

/******************************************************************************
 * Global variables and defines
 ******************************************************************************/

/******************************************************************************
 * Private interface
 ******************************************************************************/

/******************************************************************************
 * Private interface definition
 ******************************************************************************/

/******************************************************************************
 * Public interface definition
 ******************************************************************************/

API char *ipc_insure_g_variant_dup_string(GVariant *string)
{
	char *ret = g_variant_dup_string(string, NULL);

	if (0 == strcmp(ret, "")) {
		SAL_FREE(ret);
	}

	return ret;
}

API void ipc_insure_g_variant_builder_add_array_string(GVariantBuilder *builder, const char *str)
{
	if (NULL == str) {
		g_variant_builder_add(builder, "(s)", "");
	} else {
		g_variant_builder_add(builder, "(s)", str);
	}
}

API char *ipc_make_return_type(const char *type)
{
	return g_strdup_printf("(%sis)", SAL_IPC_STR(type));
}

API void ipc_create_error_msg(int code, char **ipc_msg)
{
	switch (code) {
	case SERVICE_ADAPTOR_ERROR_NONE:
		*ipc_msg = NULL;
		break;
	case SERVICE_ADAPTOR_ERROR_INTERNAL:
		*ipc_msg = strdup("SERVICE_ADAPTOR_ERROR_INTERNAL");
		break;
	default:
		*ipc_msg = strdup("SERVICE_ADAPTOR_ERROR_UNKNOWN");
		break;
	}
}

API void ipc_create_variant_info(GVariant *parameters, int size, GVariant ***var_info)
{
	for (size_t i = 0; i < size; i++) {
		*(var_info +i) = (GVariant **) g_variant_get_child_value(parameters, i);
	}
}

API void ipc_destroy_variant_info(GVariant **var_info, int size)
{
	for (size_t i = 0; i < size; i++) {
		g_variant_unref(var_info[i]);
	}
}

API void ipc_free_reply_data(ipc_reply_data_h reply)
{
	SAL_FN_CALL;

	RET_IF(NULL == reply);

	SAL_FREE(reply->type);
	SAL_FREE(reply);

	SAL_FN_END;
}
