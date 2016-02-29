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
