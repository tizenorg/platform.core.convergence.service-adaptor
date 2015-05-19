/*
 * Auth Adaptor
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

#ifndef __STORAGE_ADAPTOR_H__
#define __STORAGE_ADAPTOR_H__

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include "service_adaptor_errors.h"

/**
 * @file storage_adaptor.h
 */

/**
 * @ingroup
 * @defgroup
 *
 * @brief
 *
 * @section
 *  \#include <storage_adaptor.h>
 *
 * <BR>
 * @{
 */

typedef struct _storage_adaptor_s
{
	int i;
} storage_adaptor_s;
typedef struct _storage_adaptor_s *storage_adaptor_h;

typedef struct _storage_adaptor_listener_s
{
	int i;
} storage_adaptor_listener_s;
typedef struct _storage_adaptor_listener_s *storage_adaptor_listener_h;

API storage_adaptor_h storage_adaptor_create();
API service_adaptor_error_e storage_adaptor_destroy(storage_adaptor_h storage);
API service_adaptor_error_e storage_adaptor_start(storage_adaptor_h storage);
API service_adaptor_error_e storage_adaptor_stop(storage_adaptor_h storage);
API service_adaptor_error_e storage_adaptor_register_listener(storage_adaptor_h storage, storage_adaptor_listener_h listener);
API service_adaptor_error_e storage_adaptor_unregister_listener(storage_adaptor_h storage, storage_adaptor_listener_h listener);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __STORAGE_ADAPTOR_H__ */
