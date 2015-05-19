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

#ifndef __TIZEN_CONVERGENCE_SERVICE_ADAPTOR_TYPES_H__
#define __TIZEN_CONVERGENCE_SERVICE_ADAPTOR_TYPES_H__

#include <stdio.h>
#include <glib.h>

#include "auth_adaptor.h"
#include "contact_adaptor.h"
#include "storage_adaptor.h"

#define FILE_PATH_LEN	256

typedef struct _service_adaptor_s
{
	GList *svc_list;

	auth_adaptor_h                  auth;
	contact_adaptor_h               contact;
	storage_adaptor_h               storage;

	auth_adaptor_listener_h         auth_listener;
	contact_adaptor_listener_h      contact_listener;
	storage_adaptor_listener_h      storage_listener;

	GMutex mutex;
	GCond cond;
	int start;
} service_adaptor_s;
typedef struct _service_adaptor_s *service_adaptor_h;


#endif /* __TIZEN_CONVERGENCE_SERVICE_ADAPTOR_TYPES_H__ */
