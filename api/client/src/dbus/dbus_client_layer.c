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

/******************************************************************************
 * File: dbus_client_layer.c
 * Desc:
 * Created on: Oct, 2014
 * Auth: Yongjin Kim <youth.kim@samsung.com>
 *
 *****************************************************************************/

#include "dbus_client_layer.h"
#include "dbus_client.h"

int _dbus_client_layer_init()
{
	int ret = _dbus_client_service_adaptor_init();

	if (0 != ret) {
		_dbus_client_service_adaptor_deinit();
		return ret;
	}

	return 0;
}

void _dbus_client_layer_deinit()
{
	_dbus_client_service_adaptor_deinit();
}

