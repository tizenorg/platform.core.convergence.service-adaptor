/*
 * Resource Adaptor
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

#include <stdio.h>
#include <glib.h>

#include "resource_adaptor.h"
#include "service_adaptor_internal.h"

API resource_adaptor_h resource_adaptor_create()
{
	resource_adaptor_h resource = (resource_adaptor_h) g_malloc0(sizeof(resource_adaptor_s));

	return resource;
}

API service_adaptor_error_e resource_adaptor_destroy(resource_adaptor_h resource)
{
	SAL_FREE(resource);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

API service_adaptor_error_e resource_adaptor_start(resource_adaptor_h resource)
{
	return SERVICE_ADAPTOR_ERROR_NONE;
}

API service_adaptor_error_e resource_adaptor_stop(resource_adaptor_h resource)
{
	return SERVICE_ADAPTOR_ERROR_NONE;
}

API service_adaptor_error_e resource_adaptor_register_listener(resource_adaptor_h resource, resource_adaptor_listener_h listener)
{
	return SERVICE_ADAPTOR_ERROR_NONE;
}

API service_adaptor_error_e resource_adaptor_unregister_listener(resource_adaptor_h resource, resource_adaptor_listener_h listener)
{
	return SERVICE_ADAPTOR_ERROR_NONE;
}
