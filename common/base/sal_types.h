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

#ifndef __TIZEN_CONVERGENCE_SERVICE_ADAPTOR_TYPES_INTERNAL_H__
#define __TIZEN_CONVERGENCE_SERVICE_ADAPTOR_TYPES_INTERNAL_H__

#include <stdint.h>
#include <tizen.h>

#ifdef __cplusplus
extern "C"
{
#endif


/*
 * @brief It just be used internally
*/

typedef enum
{
	SAL_ERROR_NONE                      = TIZEN_ERROR_NONE,                     /**< Success */
	SAL_ERROR_NOT_SUPPORTED             = TIZEN_ERROR_NOT_SUPPORTED,            /**< Service plugin does not support API */
	SAL_ERROR_INVALID_PARAMETER         = TIZEN_ERROR_INVALID_PARAMETER,        /**< The parameter is invalid */
	SAL_ERROR_TIMED_OUT                 = TIZEN_ERROR_TIMED_OUT,                /**< API time out */
	SAL_ERROR_NO_DATA                   = TIZEN_ERROR_NO_DATA,                  /**< There is no data available */
	SAL_ERROR_PERMISSION_DENIED         = TIZEN_ERROR_PERMISSION_DENIED,        /**< Permission denied */
	SAL_ERROR_UNKNOWN                   = TIZEN_ERROR_UNKNOWN,                  /**< Unknown error */
	SAL_ERROR_IPC_UNSTABLE              = TIZEN_ERROR_SERVICE_ADAPTOR | 0x01,   /**< IPC Connection unstabled */
	SAL_ERROR_PLUGIN_FAILED             = TIZEN_ERROR_SERVICE_ADAPTOR | 0x02,   /**< The error occured from Plugin, See detail from service_adaptor_get_last_result() and Plugin SPEC */
	SAL_ERROR_NOT_AUTHORIZED            = TIZEN_ERROR_SERVICE_ADAPTOR | 0x03,   /**< Need Autholization */
	SAL_ERROR_INVALID_STATE             = TIZEN_ERROR_SERVICE_ADAPTOR | 0x04,   /**< The handle state is invalid for processing API */
	SAL_ERROR_SYSTEM                    = TIZEN_ERROR_SERVICE_ADAPTOR | 0xf1,   /**< Internal system module error */
	SAL_ERROR_INTERNAL                  = TIZEN_ERROR_SERVICE_ADAPTOR | 0xf2,   /**< Implementation Error */
} sal_error_e;


typedef enum
{
	SAL_SERVICE_BASE		= 0x0,
	SAL_SERVICE_STORAGE		= (0x1 << 1),
} sal_service_type_e;


typedef void *user_data_t;

#define INT_TO_USER_DATA(x)		((user_data_t)(intptr_t)x)

#define USER_DATA_TO_INT(x)		((int)(intptr_t)x)

#define USER_DATA_TYPE(name)	struct __sal_user_data_ ## name ## _s

#define USER_DATA_VAL(name)		__g_data_ ## name

#define USER_DATA_DESTROY_FUNC	g_free

/*
 * @breif	Type definition on global area
 */
#define USER_DATA_TYPEDEF(type_name, val_count) USER_DATA_TYPE(type_name) { user_data_t values[val_count]; }

/*
 * @breif	Define valuable (pointer, not allocated)
 * @pre		USER_DATA_TYPEDEF()
 */
#define USER_DATA_DEFINE(type_name, val_name)	USER_DATA_TYPE(type_name) *USER_DATA_VAL(val_name)

/*
 * @breif	Allocate memory
 * @pre		USER_DATA_TYPEDEF()
 */
#define USER_DATA_CREATE(type_name) 		(USER_DATA_TYPE(type_name) *)g_malloc0(sizeof(USER_DATA_TYPE(type_name)))

/*
 * @breif	Release memory of valuable
 * @pre		USER_DATA_CREATE()
 */
#define USER_DATA_DESTROY(val_name)			do { USER_DATA_DESTROY_FUNC(val_name); val_name = NULL; } while (0)

/*
 * @breif	Refer to user data slot
 * @pre		USER_DATA_CREATE()
 */
#define USER_DATA_ELEMENT(val_name, index)	USER_DATA_VAL(val_name)->values[index]

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_CONVERGENCE_SERVICE_ADAPTOR_TYPES_INTERNAL_H__ */
