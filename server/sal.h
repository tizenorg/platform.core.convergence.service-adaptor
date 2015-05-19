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

#ifndef __TIZEN_CONVERGENCE_SAL_H__
#define __TIZEN_CONVERGENCE_SAL_H__

#include "service_adaptor_errors.h"
#include "service_adaptor_types.h"
#include "service_adaptor_internal.h"

service_adaptor_h sal_get_handle();
char *sal_get_root_path();
service_adaptor_error_e sal_connect(service_adaptor_h sal);
service_adaptor_error_e sal_disconnect(service_adaptor_h sal);

#endif /* __TIZEN_CONVERGENCE_SAL_H__ */
