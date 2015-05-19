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

#ifndef __TIZEN_CONVERGENCE_SERVICE_ADAPTOR_ERRORS_H__
#define __TIZEN_CONVERGENCE_SERVICE_ADAPTOR_ERRORS_H__

#include <tizen.h>

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef TIZEN_ERROR_SERVICE_ADAPTOR
#define TIZEN_ERROR_SERVICE_ADAPTOR     -0x02F30000
#endif

/**
 * @file service_adaptor_errors.h
 */

/**
 * @addtogroup CAPI_CONVERGENCE_SERVICE_ADAPTOR_ERROR_MODULE
 * @{
 */

/**
 * @brief Enumeration for service adaptor errors.
 *
 * @since_tizen @if MOBILE 3.0 @elseif WEARABLE 3.0 @endif
 *
 */
typedef enum
{
	SERVICE_ADAPTOR_ERROR_NONE                      = TIZEN_ERROR_NONE,                     /**< Success */
	SERVICE_ADAPTOR_ERROR_NOT_SUPPORTED             = TIZEN_ERROR_NOT_SUPPORTED,            /**< Service plugin does not support API */
	SERVICE_ADAPTOR_ERROR_INVALID_PARAMETER         = TIZEN_ERROR_INVALID_PARAMETER,        /**< The parameter is invalid */
	SERVICE_ADAPTOR_ERROR_TIMED_OUT                 = TIZEN_ERROR_TIMED_OUT,                /**< API time out */
	SERVICE_ADAPTOR_ERROR_NO_DATA                   = TIZEN_ERROR_NO_DATA,                  /**< There is no data available */
	SERVICE_ADAPTOR_ERROR_PERMISSION_DENIED         = TIZEN_ERROR_PERMISSION_DENIED,        /**< Permission denied */
	SERVICE_ADAPTOR_ERROR_UNKNOWN                   = TIZEN_ERROR_UNKNOWN,                  /**< Unknown error */
	SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE              = TIZEN_ERROR_SERVICE_ADAPTOR | 0x01,   /**< IPC Connection unstabled */
	SERVICE_ADAPTOR_ERROR_PLUGIN_FAILED             = TIZEN_ERROR_SERVICE_ADAPTOR | 0x02,   /**< The error occured from Plugin, See detail from service_adaptor_get_last_result() and Plugin SPEC */
	SERVICE_ADAPTOR_ERROR_NOT_AUTHOLIZED            = TIZEN_ERROR_SERVICE_ADAPTOR | 0x03,   /**< Need Autholization */
	SERVICE_ADAPTOR_ERROR_INVALID_STATE             = TIZEN_ERROR_SERVICE_ADAPTOR | 0x04,   /**< The handle state is invalid for processing API */
	SERVICE_ADAPTOR_ERROR_SYSTEM                    = TIZEN_ERROR_SERVICE_ADAPTOR | 0x05,   /**< Internal system module error */
	SERVICE_ADAPTOR_ERROR_INTERNAL                  = TIZEN_ERROR_SERVICE_ADAPTOR | 0x06,   /**< Implementation Error */
} service_adaptor_error_e;

 /**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_CONVERGENCE_SERVICE_ADAPTOR_ERRORS_H__ */
