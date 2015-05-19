/*
 * Contact Adaptor
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

#ifndef __CONTACT_ADAPTOR_H__
#define __CONTACT_ADAPTOR_H__

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
 * @file contact_adaptor.h
 */

/**
 * @ingroup
 * @defgroup
 *
 * @brief
 *
 * @section
 *  \#include <contact_adaptor.h>
 *
 * <BR>
 * @{
 */

typedef struct _contact_adaptor_s
{
	int i;
} contact_adaptor_s;
typedef struct _contact_adaptor_s *contact_adaptor_h;

typedef struct _contact_adaptor_listener_s
{
	int i;
} contact_adaptor_listener_s;
typedef struct _contact_adaptor_listener_s *contact_adaptor_listener_h;

API contact_adaptor_h contact_adaptor_create();
API service_adaptor_error_e contact_adaptor_destroy(contact_adaptor_h contact);
API service_adaptor_error_e contact_adaptor_start(contact_adaptor_h contact);
API service_adaptor_error_e contact_adaptor_stop(contact_adaptor_h contact);
API service_adaptor_error_e contact_adaptor_register_listener(contact_adaptor_h contact, contact_adaptor_listener_h listener);
API service_adaptor_error_e contact_adaptor_unregister_listener(contact_adaptor_h contact, contact_adaptor_listener_h listener);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __CONTACT_ADAPTOR_H__ */
