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

#ifndef __SHOP_ADAPTOR_LOG_H__
#define __SHOP_ADAPTOR_LOG_H__

/* #define WORK_IN_LOCAL */

/**
 *  HOW TO USE IT:
 *  First you need to set platform logging on the device:
 *
 *    # dlogctrl set platformlog 1
 *
 *  After reboot you are able to see logs from this application, when you launch dlogutil with a proper filter e.g.:
 *
 *    # dlogutil SHOP_ADAPTOR:D
 *
 *  You may use different logging levels as: D (debug), I (info), W (warning), E (error) or F (fatal).
 *  Higher level messages are included by default e.g. dlogutil CLOUDBOX:W prints warnings but also errors and fatal messages.
 */

#include <unistd.h>
#include <linux/unistd.h>

/* These defines must be located before #include <dlog.h> */
#define TIZEN_ENGINEER_MODE
/* TODO: Investigate why this macro is defined somewhere else */
#ifndef TIZEN_DEBUG_ENABLE
#define TIZEN_DEBUG_ENABLE
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#endif
/* Literal to filter logs from dlogutil */
#define LOG_TAG "SHOP_ADAPTOR"


#ifndef WORK_IN_LOCAL
#include <dlog.h>
#else
#define LOGE(fmt, arg...)
#define LOGD(fmt, arg...)
#define LOGI(fmt, arg...)
#define LOGF(fmt, arg...)
#define LOGW(fmt, arg...)
#endif


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

	/**
	 *  Colors of font
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

	/**
	 *  Gets thread ID
	 */
#define shop_adaptor_gettid() syscall(__NR_gettid)

/**
 *  @brief Macro for returning value if expression is satisfied
 *  @param[in]  expr Expression to be checked
 *  @param[out] val  Value to be returned when expression is true
 */
#define shop_adaptor_retv_if(expr, val) do { \
		if (expr) { \
			LOGE(FONT_COLOR_PURPLE"[%d]"FONT_COLOR_RESET, shop_adaptor_gettid());    \
			return (val); \
		} \
	} while (0)

/**
 * @brief Prints debug messages
 * @param[in]  fmt  Format of data to be displayed
 * @param[in]  args Arguments to be displayed
 */
#define shop_adaptor_debug(fmt, arg...) do { \
		LOGD(FONT_COLOR_GREEN"[%d]"fmt""FONT_COLOR_RESET, shop_adaptor_gettid(), ##arg);     \
	} while (0)

/**
 * @brief Prints info messages
 * @param[in]  fmt  Format of data to be displayed
 * @param[in]  args Arguments to be displayed
 */
#define shop_adaptor_info(fmt, arg...) do { \
		LOGI(FONT_COLOR_BLUE"[%d]"fmt""FONT_COLOR_RESET, shop_adaptor_gettid(), ##arg);     \
	} while (0)

/**
 * @brief Prints warning messages
 * @param[in]  fmt  Format of data to be displayed
 * @param[in]  args Arguments to be displayed
 */
#define shop_adaptor_warning(fmt, arg...) do { \
		LOGW(FONT_COLOR_YELLOW"[%d]"fmt""FONT_COLOR_RESET, shop_adaptor_gettid(), ##arg);     \
	} while (0)

/**
 * @brief Prints error messages
 * @param[in]  fmt  Format of data to be displayed
 * @param[in]  args Arguments to be displayed
 */
#define shop_adaptor_error(fmt, arg...) do { \
		LOGE(FONT_COLOR_RED"[%d]"fmt""FONT_COLOR_RESET, shop_adaptor_gettid(), ##arg);     \
	} while (0)

/**
 * @brief Prints fatal messages
 * @param[in]  fmt  Format of data to be displayed
 * @param[in]  args Arguments to be displayed
 */
#define shop_adaptor_fatal(fmt, arg...) do { \
		LOGF(FONT_COLOR_BOLDRED"[%d]"fmt""FONT_COLOR_RESET, shop_adaptor_gettid(), ##arg);     \
	} while (0)

/**
 * @brief Prints debug message on entry to particular function
 * @param[in]  fmt  Format of data to be displayed
 * @param[in]  args Arguments to be displayed
 */
#define shop_adaptor_debug_func(fmt, arg...) do { \
		LOGD(FONT_COLOR_CYAN"[%d]"fmt""FONT_COLOR_RESET, shop_adaptor_gettid(), ##arg);     \
	} while (0)

#define plugin_req_enter()			do { \
		shop_adaptor_info("[ENTER] plugin API call -)) -)) -)) -)) -)) -)) -)) -)) -)) -))"); \
	} while (0)

#define plugin_req_exit(ret, plugin, error)	do { \
		shop_adaptor_info("[EXIT] plugin API called (%d) ((- ((- ((- ((- ((- ((- ((- ((- ((- ((-", (int)(ret)); \
		if ((error)) { \
			if ((*error)) { \
				shop_adaptor_error("plugin issued error (%lld) (%s)", (long long int)((*error)->code), (char *)((*error)->msg)); \
				char *tem = g_strdup_printf("[PLUGIN_ERROR] URI(%s), MSG(%s)", (char *)((plugin)->handle->plugin_uri), (char *)((*error)->msg)); \
				if (tem) { \
					free((*error)->msg); \
					(*error)->msg = tem; \
				} \
			} \
		} \
	} while (0)

#define plugin_req_exit_void()			do { \
		shop_adaptor_info("[EXIT] plugin API called ((- ((- ((- ((- ((- ((- ((- ((- ((- ((-"); \
	} while (0)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SHOP_ADAPTOR_LOG_H__ */

