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

#ifndef __TIZEN_CONVERGENCE_SAL_ENGINE_H__
#define __TIZEN_CONVERGENCE_SAL_ENGINE_H__

#include "sal_types.h"

#include <glib.h>

typedef enum
{
	SAL_ENGINE_MODE_DEFAULT	= 0,
	SAL_ENGINE_MODE_SLIM	= 1,
} sal_engine_mode_e;

typedef enum
{
	SAL_ENGINE_MAIN = 0,
	SAL_ENGINE_SERVICE = 1,
	SAL_ENGINE_FILE = 2,
	SAL_ENGINE_IPC_SERVER = 3,
	SAL_ENGINE_IPC_ADAPTOR = 4,
	SAL_ENGINE_MAX = 5,
} sal_engine_e;


typedef void (*sal_engine_task_logic)(user_data_t data);


int sal_engine_init(sal_engine_mode_e mode);

int sal_engine_deinit(void);

GMainLoop *sal_get_engine_loop(sal_engine_e engine);

int sal_engine_run(sal_engine_e engine);

int sal_engine_quit(sal_engine_e engine);

int sal_engine_main_run(void);

int sal_engine_main_quit(void);

int sal_engine_task_handoff(sal_engine_e target_engine, sal_engine_task_logic logic, user_data_t data);


#endif /* __TIZEN_CONVERGENCE_SAL_ENGINE_H__ */
