/*
 * Service Adaptor
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

#include "sal_contact.h"
#include "service_adaptor_internal.h"

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
/*
contact_adaptor_listener_h sal_contact_register_listener(contact_adaptor_h contact)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;

	contact_adaptor_listener_h listener =
			(contact_adaptor_listener_h) g_malloc0(sizeof(contact_adaptor_listener_s));

	ret = contact_adaptor_register_listener(contact, listener);

	if (SERVICE_ADAPTOR_ERROR_NONE != ret) {
		SAL_FREE(listener);
	}

	return listener;
}
*/
