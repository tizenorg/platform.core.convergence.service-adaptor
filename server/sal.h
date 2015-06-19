/*
 * Service Adaptor
 *
 * Copyright (c) 2014 - 2015 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Yongjin Kim <youth.kim@samsung.com>
 *          Jinhyeong Ahn <jinh.ahn@samsung.com>
 *          Jiwon Kim <jiwon177.kim@samsung.com>
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

#ifndef __TIZEN_CONVERGENCE_SAL_H__
#define __TIZEN_CONVERGENCE_SAL_H__

#include <glib.h>

#include "service_adaptor_errors.h"

#include "auth_adaptor.h"
#include "contact_adaptor.h"
#include "storage_adaptor.h"
#include "resource_adaptor.h"

typedef struct _sal_s
{
	GList *svc_list;

	auth_adaptor_h			auth;
	contact_adaptor_h		contact;
	storage_adaptor_h		storage;
	resource_adaptor_h		resource;

	auth_adaptor_listener_h		auth_listener;
	contact_adaptor_listener_h	contact_listener;
	storage_adaptor_listener_h	storage_listener;
	resource_adaptor_listener_h	resource_listener;

	GMutex mutex;
	GCond cond;
	int start;
} sal_s;
typedef struct _sal_s *sal_h;

typedef struct _provider_user_data_s
{
	char *uri;
	char *name;
	char *package;
} provider_user_data_s;
typedef struct _provider_user_data_s *provider_user_data_h;

sal_h sal_get_handle();
char *sal_get_root_path();
service_adaptor_error_e sal_adaptor_connect(const char *uri);
service_adaptor_error_e sal_adaptor_disconnect(const char *uri);
service_adaptor_error_e sal_adaptor_get_plugins(char ***plugins, int *plugins_size);
service_adaptor_error_e sal_provider_connect(const char *uri, const char *name, const char *package);
service_adaptor_error_e sal_provider_disconnect(const char *uri);
char *sal_provider_get_uri(const char *package);

#endif /* __TIZEN_CONVERGENCE_SAL_H__ */
