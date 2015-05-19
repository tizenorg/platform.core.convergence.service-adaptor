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

#ifndef __AUTH_ADAPTOR_H__
#define __AUTH_ADAPTOR_H__

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
 * @file auth_adaptor.h
 */

/**
 * @ingroup
 * @defgroup
 *
 * @brief
 *
 * @section
 *  \#include <auth_adaptor.h>
 *
 * <BR>
 * @{
 */

typedef struct _auth_adaptor_s
{
	int i;
} auth_adaptor_s;
typedef struct auth_adaptor_s *auth_adaptor_h;

typedef struct _auth_adaptor_listener_s
{
	int i;
} auth_adaptor_listener_s;
typedef struct auth_adaptor_listener_s *auth_adaptor_listener_h;

API auth_adaptor_h auth_adaptor_create();
API service_adaptor_error_e auth_adaptor_destroy(auth_adaptor_h auth);
API service_adaptor_error_e auth_adaptor_start(auth_adaptor_h auth);
API service_adaptor_error_e auth_adaptor_stop(auth_adaptor_h auth);
API service_adaptor_error_e auth_adaptor_register_listener(auth_adaptor_h auth, auth_adaptor_listener_h listener);
API service_adaptor_error_e auth_adaptor_unregister_listener(auth_adaptor_h auth, auth_adaptor_listener_h listener);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __AUTH_ADAPTOR_H__ */
