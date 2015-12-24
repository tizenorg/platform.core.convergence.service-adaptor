/*
 * Service Auth Internal
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

#ifndef __SAL_CLIENT_TYPES_H__
#define __SAL_CLIENT_TYPES_H__

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#include <glib.h>

/**
 * @brief Describes infromation about Adaptor Handle
 */
typedef struct _service_adaptor_s {
	int pid
	char *uri;

	GList *plugins;		/* char **plugins (uri) */
	GList *started_plugins;	/* service_plugin_h **started_plugins */
} service_adaptor_s;

/**
 * @brief Describes infromation about Plugin Handle
 */
typedef struct _service_plugin_s {
	char *uri;
	GHashTable *property;
} service_plugin_s;

#ifdef __cplusplus
}
#endif

#endif /* __SAL_CLIENT_TYPES_H__ */
