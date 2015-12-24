/*
 * Service Task
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

#ifndef __SERVICE_TASK_H__
#define __SERVICE_TASK_H__

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C"
{
#endif
/*
typedef enum _service_task_state_e
{
	SERVICE_TASK_IN_PROGRESS	= 1,
	SERVICE_TASK_COMPLETED		= 2,
	SERVICE_TASK_CANCELED		= 3,
	SERVICE_TASK_FAILED		= 4
} service_task_state_e;

typedef struct _service_task_s *service_task_h;

typedef void (*service_task_progress_cb)(service_task_h task, unsigned long long progress, unsigned long long total, void *user_data);

typedef void (*service_task_state_changed_cb)(service_task_h task, service_task_state_e state, void *user_data);
*/
/*==================================================================================================
                                         FUNCTION PROTOTYPES
==================================================================================================*/
/*
int service_task_start(service_task_h task);
int service_task_stop(service_task_h task);
int service_task_set_uri(service_task_h task, const char *uri);
int service_task_get_uri(service_task_h task, char **uri);
int service_task_set_progress_callback(service_task_h task, service_task_progress_cb callback, void *user_data);
int service_task_unset_progress_callback(service_task_h task);
int service_task_set_state_changed_callback(service_task_h task, service_task_state_changed_cb callback, void *user_data);
int service_task_unset_state_changed_callback(service_task_h task);
*/
#ifdef __cplusplus
}
#endif

#endif /* __SERVICE_TASK_H__ */
