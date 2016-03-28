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

#ifndef __TIZEN_SOCIAL_SERVICE_ADAPTOR_CLIENT_TYPE_H__
#define __TIZEN_SOCIAL_SERVICE_ADAPTOR_CLIENT_TYPE_H__

#include <stdint.h>
#include "service_adaptor_client.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TASK_MESSAGE_FORWARD_ONLINE_MESSAGE	-100
#define TASK_MESSAGE_FORWARD_UNREAD_MESSAGE	-101
#define TASK_MESSAGE_CHANNEL_DISCONNECTED_MESSAGE	-102

/* private feature start */
#define PRIVATE_SIGNAL_FILE_PROGRESS_ID			11000
#define PRIVATE_SIGNAL_FILE_TRANSFER_COMPLETION_ID	11001
/* private feature end */

#define SIGNAL_STORAGE_FILE_PROGRESS_ID		1000
#define SIGNAL_STORAGE_FILE_STATE_CHANGED_ID	1001
#define SIGNAL_SERVICE_ADAPTOR			9000

/***************************************/
/* Signal Task area start              */
/***************************************/

typedef struct _service_storage_task_s {
	char *service_handle_name;
	long long int task_id;	/* file_uid (matched fd) */
	void *param1;
	void *param2;
	void *param3;

	int operation;		/* 1: upload, 2: download, 3:thumbnail (TEMP) */
	int state;
	service_storage_task_state_cb state_callback;
	void *state_user_data;
	service_storage_task_progress_cb progress_callback;
	void *progress_user_data;
} service_storage_task_t;

/**
* @brief Describes infromation about task
*/
typedef struct _service_adaptor_task_s {
	int64_t id;			/**< specifies status as none*/
	uint32_t callback;		/**< specifies status as none*/
	void *handle;			/**< specifies status as none*/
	void *user_data;
} service_adaptor_task_s;

/**
* @brief Describes infromation about Service Adaptor's error
* @remarks 'msg' is need free()
*/
typedef struct _service_adaptor_error_s {
	long long int code;			/**< specifies status as none*/
	char *msg;				/**< specifies status as none*/
} service_adaptor_error_s;

#define __SAFE_STRDUP(x)        (x) ? strdup((x)) : NULL
#define __SAFE_FREE(x)		do { free(x); (x) = NULL; } while (0)
#define _assign_error_code(src, tgt)	do { \
						sac_error("Error occured a : (%lld) (%s)", (long long int)((src)->code), (src)->msg); \
						__assign_error_code((src), (tgt)); \
					} while (0)

#define _set_error_code(tgt, code, msg)	do { \
						sac_error("Error occured b : (%d) (%s)", (int)(code), (msg)); \
						__set_error_code((tgt), (code), (msg)); \
					} while (0)


/**
* @brief The handle for Task
*/
typedef service_adaptor_task_s *service_adaptor_task_h;

int _queue_add_task(int64_t id,
						uint32_t callback,
						void *handle,
						void *user_data);

int _queue_del_task(service_adaptor_task_h task);

service_adaptor_task_h _queue_get_task(int64_t id);

void _queue_clear_task(void);

int _signal_queue_add_task(int64_t id,
						uint32_t callback,
						void *handle,
						void *user_data);

service_adaptor_task_h _signal_queue_get_task(int64_t id);

int _signal_queue_del_task(service_adaptor_task_h task);

void _signal_queue_clear_task(void);

int service_adaptor_check_handle_validate(service_adaptor_h handle);


/**
 * @brief Enumerations of signal code for Service Adaptor
 */
typedef enum _service_adaptor_signal_code_e {
	SERVICE_ADAPTOR_SIGNAL_INITIALIZED		= 1,	/* Service adaptor finished initalization */
	SERVICE_ADAPTOR_SIGNAL_NEW_PLUGIN		= 2,	/* New Plugins loaded in a running time */
	SERVICE_ADAPTOR_SIGNAL_ACTIVATE_PLUGIN		= 3,	/* Some Plugins be activated by policy or user atholization or etc */
	SERVICE_ADAPTOR_SIGNAL_SHUTDOWN                 = 4,    /* Service adaptor was shutdowned by unsuspected issue */
} service_adaptor_signal_code_e;

/***************************************/
/* Signal Task area end                */
/***************************************/

#ifdef __cplusplus
}
#endif /* __cpluscplus */
#endif /* __TIZEN_SOCIAL_SERVICE_ADAPTOR_CLIENT_TYPE_H__ */
