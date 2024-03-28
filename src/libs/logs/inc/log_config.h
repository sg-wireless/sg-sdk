/** -------------------------------------------------------------------------- *
 * @copyright Copyright (c) 2023-2024 SG Wireless - All Rights Reserved
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files(the “Software”), to deal
 * in the Software without restriction, including without limitation the rights
 * to use,  copy,  modify,  merge, publish, distribute, sublicense, and/or sell
 * copies  of  the  Software,  and  to  permit  persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”,  WITHOUT WARRANTY OF ANY KIND,  EXPRESS OR
 * IMPLIED,  INCLUDING BUT NOT LIMITED TO  THE  WARRANTIES  OF  MERCHANTABILITY
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS  OR  COPYRIGHT  HOLDERS  BE  LIABLE FOR ANY CLAIM,  DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN  CONNECTION WITH  THE SOFTWARE OR  THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 * @author  Ahmed Sabry (SG Wireless)
 * 
 * @brief   This file contains the configurations of the log library.
 * --------------------------------------------------------------------------- *
 */

#ifndef __LOG_CONFIG_H__
#define __LOG_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

/** -------------------------------------------------------------------------- *
 * includes
 * --------------------------------------------------------------------------- *
 */
#include "utils_units.h"
#ifdef MAIN_SDK_CONFIG_FILE
#include MAIN_SDK_CONFIG_FILE
#endif

/** -------------------------------------------------------------------------- *
 * log header compile-time configurations
 * --------------------------------------------------------------------------- *
 */
#ifdef CONFIG_SDK_LOG_LIB_HEADER_TIMESTAMP
#define __opt_log_header_timestamp      y
#else
#define __opt_log_header_timestamp      n
#endif

#ifdef CONFIG_SDK_LOG_LIB_HEADER_FILENAME
#define __opt_log_header_filename       y
#else
#define __opt_log_header_filename       n
#endif

#ifdef CONFIG_SDK_LOG_LIB_HEADER_FUNC_NAME
#define __opt_log_header_func_name      y
#else
#define __opt_log_header_func_name      n
#endif

#ifdef CONFIG_SDK_LOG_LIB_HEADER_LINE_NUM
#define __opt_log_header_line_num       y
#else
#define __opt_log_header_line_num       n
#endif

#ifdef CONFIG_SDK_LOG_LIB_HEADER_LOG_TYPE
#define __opt_log_header_log_type       y
#else
#define __opt_log_header_log_type       n
#endif

#ifdef CONFIG_SDK_LOG_LIB_HEADER_SUBSYSTEM
#define __opt_log_header_subsystem      y
#else
#define __opt_log_header_subsystem      n
#endif

#ifdef CONFIG_SDK_LOG_LIB_HEADER_COMPONENT
#define __opt_log_header_component      y
#else
#define __opt_log_header_component      n
#endif

#ifdef CONFIG_SDK_LOG_LIB_HEADER_COMPONENT
#define __opt_log_header_os_info        y
#else
#define __opt_log_header_os_info        n
#endif

#define __opt_log_header_timestamp_hhh_mm_ss    y

#define __opt_log_header_seg_order_timestamp      (0)
#define __opt_log_header_seg_order_log_type       (1)
#define __opt_log_header_seg_order_os_info        (2)
#define __opt_log_header_seg_order_filename       (3)
#define __opt_log_header_seg_order_line_num       (4)
#define __opt_log_header_seg_order_func_name      (5)
#define __opt_log_header_seg_order_subsystem      (6)
#define __opt_log_header_seg_order_component      (7)

#define __opt_log_header_segments_count           (8)

/** -------------------------------------------------------------------------- *
 * log coloring compile-time configurations
 * --------------------------------------------------------------------------- *
 */
#ifdef CONFIG_SDK_LOG_LIB_TERMINAL_COLORING_ENABLE
#define __opt_global_log_coloring       y
#else
#define __opt_global_log_coloring       n
#endif

#define __opt_log_type_color_warn       yellow
#define __opt_log_type_color_error      red
#define __opt_log_type_color_assert     purple

/** -------------------------------------------------------------------------- *
 * log types compile compile-time configurations
 * --------------------------------------------------------------------------- *
 */
#if defined(__log_lib_global_disable) || !defined(CONFIG_SDK_LOG_LIB_ENABLE)
    #define __log_lib_global_default    n
#else
    #define __log_lib_global_default    y
#endif

#define __opt_log_type_output           y

#ifdef CONFIG_SDK_LOG_LIB_TYPE_PRINTF
#define __opt_log_type_printf           __log_lib_global_default
#else
#define __opt_log_type_printf           n
#endif

#ifdef CONFIG_SDK_LOG_LIB_TYPE_INFO
#define __opt_log_type_info             __log_lib_global_default
#else
#define __opt_log_type_info             n
#endif

#ifdef CONFIG_SDK_LOG_LIB_TYPE_DEBUG
#define __opt_log_type_debug            __log_lib_global_default
#else
#define __opt_log_type_debug            n
#endif

#ifdef CONFIG_SDK_LOG_LIB_TYPE_WARN
#define __opt_log_type_warn             __log_lib_global_default
#else
#define __opt_log_type_warn             n
#endif

#ifdef CONFIG_SDK_LOG_LIB_TYPE_ERROR
#define __opt_log_type_error            __log_lib_global_default
#else
#define __opt_log_type_error            n
#endif

#ifdef CONFIG_SDK_LOG_LIB_TYPE_ASSERT
#define __opt_log_type_assert           __log_lib_global_default
#else
#define __opt_log_type_assert           n
#endif

#ifdef CONFIG_SDK_LOG_LIB_TYPE_MEM_DUMP
#define __opt_log_type_mem_dump         __log_lib_global_default
#else
#define __opt_log_type_mem_dump         n
#endif

#ifdef CONFIG_SDK_LOG_LIB_TYPE_TEST
#define __opt_log_type_test             __log_lib_global_default
#else
#define __opt_log_type_test             n
#endif

#ifdef CONFIG_SDK_LOG_LIB_TYPE_ENFORCE
#define __opt_log_type_enforce          __log_lib_global_default
#else
#define __opt_log_type_enforce          n
#endif


/** -------------------------------------------------------------------------- *
 * log line metrics compile-time configurations
 * --------------------------------------------------------------------------- *
 */
#ifdef CONFIG_SDK_LOG_LIB_HEADER_TIMESTAMP_WIDTH
#define __opt_log_disp_w_timestamp  (CONFIG_SDK_LOG_LIB_HEADER_TIMESTAMP_WIDTH)
#else
#define __opt_log_disp_w_timestamp  (13)
#endif

#ifdef CONFIG_SDK_LOG_LIB_HEADER_OS_CONTEXT_INFO_WIDTH
#define __opt_log_disp_w_os_info    \
    (CONFIG_SDK_LOG_LIB_HEADER_OS_CONTEXT_INFO_WIDTH)
#else
#define __opt_log_disp_w_os_info        (10)
#endif

#ifdef CONFIG_SDK_LOG_LIB_HEADER_SUBSYSTEM_WIDTH
#define __opt_log_disp_w_subsystem  (CONFIG_SDK_LOG_LIB_HEADER_SUBSYSTEM_WIDTH)
#else
#define __opt_log_disp_w_subsystem  ( 9)
#endif

#ifdef CONFIG_SDK_LOG_LIB_HEADER_COMPONENT_WIDTH
#define __opt_log_disp_w_component  (CONFIG_SDK_LOG_LIB_HEADER_COMPONENT_WIDTH)
#else
#define __opt_log_disp_w_component  (13)
#endif

#ifdef CONFIG_SDK_LOG_LIB_HEADER_FILENAME_WIDTH
#define __opt_log_disp_w_filename   (CONFIG_SDK_LOG_LIB_HEADER_FILENAME_WIDTH)
#else
#define __opt_log_disp_w_filename   (12)
#endif

#ifdef CONFIG_SDK_LOG_LIB_HEADER_LINE_NUM_WIDTH
#define __opt_log_disp_w_line_num   (CONFIG_SDK_LOG_LIB_HEADER_LINE_NUM_WIDTH)
#else
#define __opt_log_disp_w_line_num   ( 6)
#endif

#ifdef CONFIG_SDK_LOG_LIB_HEADER_FUNC_NAME_WIDTH
#define __opt_log_disp_w_func_name  (CONFIG_SDK_LOG_LIB_HEADER_FUNC_NAME_WIDTH)
#else
#define __opt_log_disp_w_func_name  (15)
#endif

#ifdef CONFIG_SDK_LOG_LIB_HEADER_LOG_TYPE_WIDTH
#define __opt_log_disp_w_log_type   (CONFIG_SDK_LOG_LIB_HEADER_LOG_TYPE_WIDTH)
#else
#define __opt_log_disp_w_log_type   ( 8)
#endif

#ifdef CONFIG_SDK_LOG_LIB_TOTAL_LINE_WIDTH
#define __opt_log_disp_w_msg        (CONFIG_SDK_LOG_LIB_TOTAL_LINE_WIDTH)
#else
#define __opt_log_disp_w_msg        (86)
#endif

#ifdef CONFIG_SDK_LOG_LIB_TOTAL_LINE_WRAPPING_ENABLE
    #define __opt_log_enforce_msg_len       y
#else
    #define __opt_log_enforce_msg_len       n
#endif

/** -------------------------------------------------------------------------- *
 * log buffering compile-time configurations
 * --------------------------------------------------------------------------- *
 */
#ifdef CONFIG_SDK_LOG_LIB_MEMORY_BUFFER_SIZE
#define __opt_logs_buffer_memory  \
    __msize_kb(CONFIG_SDK_LOG_LIB_MEMORY_BUFFER_SIZE_KB)
#else
#define __opt_logs_buffer_memory  __msize_kb(4)
#endif

/** -------------------------------------------------------------------------- *
 * end here
 * --------------------------------------------------------------------------- *
 */
#include "log_config_sanity.h"
#ifdef __cplusplus
}
#endif
#endif /* __LOG_CONFIG_H__ */
