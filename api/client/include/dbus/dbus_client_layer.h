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
 * File: dbus-client-layer.h
 * Desc: D-Bbus IPC client
 * Created on: Oct, 2014
 * Auth: Yongjin Kim <youth.kim@samsung.com>
 *
 *****************************************************************************/
/**
 *	@file		dbus-client-layer.h
 *	@brief		Defines interface of D-Bus IPC
 *	@version	0.1
 */

#ifndef __TIZEN_SOCIAL_SERVICE_ADAPTOR_DBUS_CLIENT_LAYER_H__
#define __TIZEN_SOCIAL_SERVICE_ADAPTOR_DBUS_CLIENT_LAYER_H__

/*==================================================================================================
					FUNCTION PROTOTYPES
==================================================================================================*/

/**
 * @brief Initialize D-Bus IPC client layer.
 *
 * Initialize D-Bus IPC client layer. Must be called once at startup before any call to Service Adaptor
 * client API is made.
 * @return 0 on success, -1 on error.
 */
int _dbus_client_layer_init();

/**
 * @brief Deinitialize D-Bus IPC client layer.
 *
 * Deinitialize D-Bus IPC client layer. Should be run once at shutdown.
 */
void _dbus_client_layer_deinit();

#endif /* __TIZEN_SOCIAL_SERVICE_ADAPTOR_DBUS_CLIENT_LAYER_H__ */
