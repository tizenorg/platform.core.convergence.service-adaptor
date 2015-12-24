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

#ifndef __TIZEN_CONVERGENCE_SAL_LOG_H__
#define __TIZEN_CONVERGENCE_SAL_LOG_H__

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#ifdef API
#undef API
#endif
#define API __attribute__ ((visibility("default")))

#define LOG_TAG "SERVICE-ADAPTOR"
#include <dlog.h>
#define DLOG(prio, fmt, arg...) \
        do { SLOG(prio, LOG_TAG, fmt, ##arg); } while (0)


#if defined(_SERVICE_ADAPTOR_IPC_SERVER)
#define IPC_ROLE "[SERVER]"
#elif defined(_SERVICE_ADAPTOR_IPC_CLIENT)
#define IPC_ROLE "[CLIENT]"
#else
#define IPC_ROLE "[LIB]"
#endif

/**
 * Colors of font
 */
#define FONT_COLOR_RESET      "\033[0m"
#define FONT_COLOR_BLACK      "\033[30m"             /* Black */
#define FONT_COLOR_RED        "\033[31m"             /* Red */
#define FONT_COLOR_GREEN      "\033[32m"             /* Green */
#define FONT_COLOR_YELLOW     "\033[33m"             /* Yellow */
#define FONT_COLOR_BLUE       "\033[34m"             /* Blue */
#define FONT_COLOR_PURPLE     "\033[35m"             /* Purple */
#define FONT_COLOR_CYAN       "\033[36m"             /* Cyan */
#define FONT_COLOR_WHITE      "\033[37m"             /* White */
#define FONT_COLOR_BOLDBLACK  "\033[1m\033[30m"      /* Bold Black */
#define FONT_COLOR_BOLDRED    "\033[1m\033[31m"      /* Bold Red */
#define FONT_COLOR_BOLDGREEN  "\033[1m\033[32m"      /* Bold Green */
#define FONT_COLOR_BOLDYELLOW "\033[1m\033[33m"      /* Bold Yellow */
#define FONT_COLOR_BOLDBLUE   "\033[1m\033[34m"      /* Bold Blue */
#define FONT_COLOR_BOLDPURPLE "\033[1m\033[35m"      /* Bold Purple */
#define FONT_COLOR_BOLDCYAN   "\033[1m\033[36m"      /* Bold Cyan */
#define FONT_COLOR_BOLDWHITE  "\033[1m\033[37m"      /* Bold White */

#define INFO(fmt, arg...) SLOGI(FONT_COLOR_YELLOW""IPC_ROLE" "fmt""FONT_COLOR_RESET, ##arg)
#define ERR(fmt, arg...) SLOGE(FONT_COLOR_RED""IPC_ROLE" "fmt""FONT_COLOR_RESET, ##arg)
#define DBG(fmt, arg...) SLOGD(IPC_ROLE" "fmt, ##arg)
#define WARN(fmt, arg...) SLOGD(IPC_ROLE" "fmt, ##arg)
#define VERBOSE(fmt, arg...) SLOGV(IPC_ROLE" "fmt, ##arg)

#ifdef SERVICE_ADAPTOR_DEBUGGING

        #define SAL_FN_CALL DBG(">>>>>>>> called")
        #define SAL_FN_END DBG("<<<<<<<< ended")

        #define SAL_DBG(fmt, arg...) DBG(fmt, ##arg)
        #define SAL_WARN(fmt, arg...) WARN(fmt, ##arg)
        #define SAL_ERR(fmt, arg...) ERR(fmt, ##arg)
        #define SAL_INFO(fmt, arg...) INFO(fmt, ##arg)
        #define SAL_VERBOSE(fmt, arg...) VERBOSE(fmt, ##arg)

#else /* SERVICE_ADAPTOR_DEBUGGING */
        #define SAL_FN_CALL
        #define SAL_FN_END

        #define SAL_DBG(fmt, arg...)
        #define SAL_WARN(fmt, arg...)
        #define SAL_ERR(fmt, arg...) ERR(fmt, ##arg)
        #define SAL_INFO(fmt, arg...)
        #define SAL_VERBOSE(fmt, arg...)

#endif /* SERVICE_ADAPTOR_DEBUGGING */

#define WARN_IF(expr, fmt, arg...) do { \
        if (expr) { \
                SAL_WARN(fmt, ##arg); \
        } \
} while (0)
#define RET_IF(expr) do { \
        if (expr) { \
                SAL_ERR("(%s)", #expr); \
                return; \
        } \
} while (0)
#define RETV_IF(expr, val) do { \
        if (expr) { \
                SAL_ERR("(%s)", #expr); \
                return (val); \
        } \
} while (0)
#define RETM_IF(expr, fmt, arg...) do { \
        if (expr) { \
                SAL_ERR(fmt, ##arg); \
                return; \
        } \
} while (0)
#define RETVM_IF(expr, val, fmt, arg...) do { \
        if (expr) { \
                SAL_ERR(fmt, ##arg); \
                return (val); \
        } \
} while (0)
#define TRY_IF(expr, fmt, arg...) do { \
        if (expr) { \
                SAL_INFO(fmt, ##arg); \
                goto catch; \
        } \
} while (0)
#define TRYM_IF(expr, fmt, arg...) do { \
        if (expr) { \
                SAL_ERR(fmt, ##arg); \
                goto catch; \
        } \
} while (0)
#define TRYVM_IF(expr, val, fmt, arg...) do { \
        if (expr) { \
                SAL_ERR(fmt, ##arg); \
                val; \
		goto catch; \
        } \
} while (0)

#define SAL_STRDUP(dst, ptr) do { \
	if (ptr) \
		dst = strdup(ptr); \
	else \
		dst = NULL; \
} while(0)

#define SAL_FREE(ptr) do { \
	if (ptr) \
		free(ptr); \
	ptr = NULL; \
} while(0)

#define SAL_FOREACH_GLIST(iterator, list)	for (GList *iterator = g_list_first(list); iterator; iterator = g_list_next(iterator))

#endif /* __TIZEN_CONVERGENCE_SAL_LOG_H__ */
