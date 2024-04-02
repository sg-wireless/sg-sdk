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
 * @brief   This file represents the main log_lib interface
 * --------------------------------------------------------------------------- *
 */

#ifndef __LOG_LIB_H__
#define __LOG_LIB_H__

#ifdef __cplusplus
extern "C" {
#endif

/* --- includes ------------------------------------------------------------- */

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

#include "utils_misc.h"
#include "log_config.h"

#include "logs_gen_comp_ids.hh"

#include "../src/log_obj.h"

/* --- logging macros ------------------------------------------------------- */

/** -------------------------------------------------------------------------- *
 * - macros used to create new logs subsystems and components
 * - they are used by the generation script to generate the respective
 *   macros and meta related information
 * --------------------------------------------------------------------------- *
 */
#define __log_subsystem_def(__name, __color, __compile_flag, __on)
#define __log_component_def(__subsystem, __name, __color, __compile_flag, __on)

/** -------------------------------------------------------------------------- *
 * mandatory macro define for default subsystem definition
 * --------------------------------------------------------------------------- *
 */
__log_subsystem_def(default, default, 1, 0)
#ifndef __log_subsystem
    #define __log_subsystem     default
#endif
#ifndef __log_component
    #define __log_component     default
#endif

/** -------------------------------------------------------------------------- *
 * main log macro
 *  - it is the basic logging macro
 *  - all logs type eventually call this macro
 * --------------------------------------------------------------------------- *
 */
/** some dependent macros, used by __log() macro only */
#define __get_comp_id(__subsys,__comp) __concat(        \
    __tricat(__log_component_, __subsys, _),            \
    __concat(__comp, _id) )
#define __get_curr_comp_id()    __get_comp_id(__log_subsystem, __log_component)
#define __get_curr_comp_cc()    __concat(               \
    __tricat(__log_component_, __log_subsystem, _),     \
    __concat(__log_component, _cc) )
#define __get_curr_subsys_cc()  __tricat(__log_subsystem_, __log_subsystem, _cc)
#define __get_log_type_opt(_type)       __concat(__opt_log_type_, _type)


#define __log(type, log_info, args...)                                  \
    __opt_paste(__get_log_type_opt(type), y,                            \
        __opt_paste(__get_curr_subsys_cc(), 1,                          \
            __opt_paste(__get_curr_comp_cc(), 1,                        \
                log_impl(log_info,                                      \
                    __get_curr_comp_id(),                               \
                    __opt_paste(__opt_log_header_filename, y, __FILE__,)\
                    __opt_paste(__opt_log_header_line_num, y, __LINE__,)\
                    __opt_paste(__opt_log_header_func_name, y,__func__,)\
                    args                                                \
                )                                                       \
            )                                                           \
        )                                                               \
    )

/** -------------------------------------------------------------------------- *
 * basic logging operations macros
 * --------------------------------------------------------------------------- *
 */
#define __log_basic_type(type, args...)                             \
    __opt_paste(__get_log_type_opt(type), y,                        \
        do{                                                         \
            log_info_t log_info = {                                 \
                .p_type_info = & g_log_type_ ## type };             \
            __log(type, &log_info, args);                           \
        } while(0)                                                  \
    )                                                               \
    __opt_paste(__get_log_type_opt(type), n,                        \
        {}                                                          \
    )

#define __log_info(args...)     __log_basic_type(info   ,args)
#define __log_printf(args...)   __log_basic_type(printf ,args)
#define __log_endl()            log_endl()
#define __log_debug(args...)    __log_basic_type(debug  ,args)
#define __log_warn(args...)     __log_basic_type(warn   ,args)
#define __log_error(args...)    __log_basic_type(error  ,args)
#define __log_assert(cond, args...)             \
    do {                                        \
        if(!(cond)) {                           \
            __log_basic_type(assert, args);     \
            /**(int*)NULL = 0;*/                \
        }                                       \
    } while(0)

#define __log_output(args...)                   \
    __opt_paste(__opt_log_type_output, y,       \
    do {                                        \
        log_info_t log_info = {                 \
            .p_type_info = &g_log_type_output };\
        log_stdout( &log_info, args );          \
    } while(0))
#define __log_output_fmt(fmt, va_list_args)     \
    __opt_paste(__opt_log_type_output, y,       \
    do {                                        \
        log_info_t log_info = {                 \
            .p_type_info = &g_log_type_output };\
        log_stdout_fmt( &log_info, fmt,         \
            va_list_args );                     \
    } while(0))

#define __log_ptr(__ptr)    __log_debug("pointer: %s = %p", #__ptr, __ptr)

#define __black__     ""__opt_paste(__opt_global_log_coloring, y, "%Ck")
#define __red__       ""__opt_paste(__opt_global_log_coloring, y, "%Cr")
#define __green__     ""__opt_paste(__opt_global_log_coloring, y, "%Cg")
#define __yellow__    ""__opt_paste(__opt_global_log_coloring, y, "%Cy")
#define __blue__      ""__opt_paste(__opt_global_log_coloring, y, "%Cb")
#define __purple__    ""__opt_paste(__opt_global_log_coloring, y, "%Cp")
#define __cyan__      ""__opt_paste(__opt_global_log_coloring, y, "%Cc")
#define __white__     ""__opt_paste(__opt_global_log_coloring, y, "%Cw")
#define __default__   ""__opt_paste(__opt_global_log_coloring, y, "%Cd")

#define __concat_colored_str(str, color)    \
    __opt_paste(__opt_global_log_coloring, y, color str "\033[m") \
    __opt_paste(__opt_global_log_coloring, n, str)
#define __black_str(str)      __concat_colored_str(str, "\033[39m")
#define __red_str(str)        __concat_colored_str(str, "\033[31m")
#define __green_str(str)      __concat_colored_str(str, "\033[32m")
#define __yellow_str(str)     __concat_colored_str(str, "\033[33m")
#define __blue_str(str)       __concat_colored_str(str, "\033[34m")
#define __purple_str(str)     __concat_colored_str(str, "\033[35m")
#define __cyan_str(str)       __concat_colored_str(str, "\033[36m")
#define __white_str(str)      __concat_colored_str(str, "\033[37m")
#define __default_str(str)    str

/** -------------------------------------------------------------------------- *
 * user defined log type operations
 * ================================
 * to define a new log type, two new files will be created:
 * log_<typename>.h
 *      #include "log_lib.h"
 * 
 *      // -- configuration macro
 *      #define __opt_log_type_<typename>       <y|n>
 * 
 *      // -- [ optional ] -- log info typedef
 *      typedef struct {
 *          log_info_t  log_info_base;  // -- mandatory first member
 *          ...                         // -- user defined members
 *      } log_info_<typename>_t;
 * 
 *      // -- log type singleton struct declaration
 *      __log_type_dec( <typename> );
 * 
 *      // -- logging macro
 *      #define __log_<typename>( <user-defined-args> ) \
 *          
 *      
 * log_provider_<typename>.c
 * --------------------------------------------------------------------------- *
 */
#define __log_type_cc_flag_name(typename)    __get_log_type_opt(typename)

#define __log_type_def(provider, typename, enable)          \
    __opt_paste(__log_type_cc_flag_name(typename), y,       \
        log_type_info_t g_log_type_ ## typename = {         \
            .p_provider = provider,                         \
            .type_name = # typename,                        \
            .flags = (enable & __log_type_flag_en) |        \
                               __log_type_flag_cc } )       \
    __opt_paste(__log_type_cc_flag_name(typename), n,       \
        log_dummy_type_info_t g_dummy_log_type_##typename   \
            = {.type_name = # typename,                     \
               .flags = (enable & __log_type_flag_en)} )

#define __log_type_dec(typename)                            \
    __opt_paste(__log_type_cc_flag_name(typename), y,       \
        extern log_type_info_t g_log_type_ ## typename )

#define __log_type_register(typename)                       \
    __opt_paste(__log_type_cc_flag_name(typename), y,       \
        extern log_type_info_t g_log_type_ ## typename;     \
        log_type_register( & g_log_type_ ## typename ) )    \
    __opt_paste(__log_type_cc_flag_name(typename), n,       \
        extern log_dummy_type_info_t                        \
            g_dummy_log_type_ ## typename;                  \
        log_type_register( (log_type_info_t*)               \
            & g_dummy_log_type_ ## typename ) )

#define __log_type_info_struct_def(typename,                \
        type_var_pairs_list...)                             \
    typedef struct { log_info_t log_info_base;              \
        type_var_pairs_list                                 \
    } log_info_##typename##_t

// #define __log_type(typename, args...)  __log_basic_type(typename, args)
#define __log_opr_macro(typename, init_args_list...)        \
    __opt_paste(__get_log_type_opt(typename), y,            \
        do {                                                \
            log_info_##typename##_t info = {                \
                { .p_type_info = &g_log_type_##typename },  \
                init_args_list                              \
            };                                              \
            __log(typename, (void*)&info, );                \
        } while(0)                                          \
    )

/** -------------------------------------------------------------------------- *
 * log objects typedefs
 *      *** used only by the logging macros and extended providers ***
 * --------------------------------------------------------------------------- *
 */
typedef struct  log_info_s log_info_t;
typedef void provider_func_t(log_info_t* p_log_info);

/**
 * @details This struct declares the log type singleton type information.
 *          An object of this struct shall be defined globally once for each
 *          log type (info, error, ...) and shall be referenced by all incoming
 *          log instances of its type in their local object of type log_info_t
 *          or its derived types.
 */
typedef struct {
    void*               priv_data;  /**< used internally by the log libraray*/
    const char *        type_name;  /**< a display name for this log type */
    uint32_t            flags;      /**< to record control flags */
    #define __log_type_flag_en      (1<<0)
    #define __log_type_flag_cc      (1<<1)
    #define __log_type_flag_out     (1<<2) /** normal standard output */
    provider_func_t *   p_provider; /**< provider function of this log type */
    uint32_t            priv_flags; /**< private flags for this specific type
                                         used only by its provider */
} log_type_info_t;

typedef struct {
    void*               priv_data;
    const char *        type_name;
    uint32_t            flags;
} log_dummy_type_info_t;

typedef struct _log_info_base_s log_info_base_t;
typedef struct  log_info_s {
    log_info_base_t*        p_basic_info;
    const log_type_info_t * p_type_info;
} log_info_t;

/* === basic types declaration ============================================== */

__log_type_dec(output);
__log_type_dec(printf);
__log_type_dec(info  );
__log_type_dec(debug );
__log_type_dec(warn  );
__log_type_dec(error );
__log_type_dec(assert);

/** -------------------------------------------------------------------------- *
 * logging operation APIs
 * --------------------------------------------------------------------------- *
 */
typedef uint32_t log_port_get_timestamp_t(void);
typedef void log_port_mutex_lock_t(void);
typedef void log_port_mutex_unlock_t(void);
typedef void log_port_serial_output_t(uint8_t* buf, uint32_t len);
typedef const char* log_port_get_current_task_name_t(void);
typedef int log_port_get_current_core_id_t(void);
typedef struct {
    // -- timestamp getter
    log_port_get_timestamp_t *          get_timestamp;

    // -- mutual execlusion methods for internal usage
    log_port_mutex_lock_t *             mutex_lock;
    log_port_mutex_unlock_t *           mutex_unlock;

    // -- output serialization
    log_port_serial_output_t *          serial_out;

    // -- os info getters
    log_port_get_current_task_name_t*   get_task_name;
    log_port_get_current_core_id_t*     get_core_id;
} log_init_params_t;

void log_init(log_init_params_t* p_init_params);

void log_impl(
    log_info_t* log_info,
    int         comp_id,
    __opt_paste(__opt_log_header_filename, y, const char* file,)
    __opt_paste(__opt_log_header_line_num, y, int line,)
    __opt_paste(__opt_log_header_func_name, y, const char* func,)
    const char* fmt,
    ...);

void log_stdout(log_info_t* log_info, const char* fmt, ...);
void log_stdout_fmt(log_info_t* log_info, const char* fmt, va_list arg_ptr);

void log_endl(void);

void log_type_register(log_type_info_t * p_log_type_info);

void log_enforce(const char* file, int line, const char* func,
    const char* fmt, ...);

/** -------------------------------------------------------------------------- *
 * logging filteration APIs
 * --------------------------------------------------------------------------- *
 */

void log_filter_list_stats(void);

void log_filter_header(
    const char* header_column_name,
    bool state);

void log_filter_header_reorder(
    const char* name,
    int new_order);

void log_filter_type(
    const char* type_name,
    bool state);

void log_filter_component(
    const char* subsys_name,
    const char* component_name,
    bool state);

void log_filter_subsystem(
    const char* subsystem_name,
    bool state);

bool log_filter_subsystem_get_state(
    const char* subsystem_name);

bool log_filter_component_get_state(
    const char* subsystem_name,
    const char* component_name);

typedef struct {
    const char* subsystem_name;
    bool        subsystem_save_state;
    const char* component_name;
    bool        component_save_state;
} log_filter_save_state_t;

void log_filter_save_state(log_filter_save_state_t* p_filter_state
    , bool new_state);

void log_filter_restore_state(log_filter_save_state_t* p_filter_state);

/** -------------------------------------------------------------------------- *
 * extended logging types include files
 * --------------------------------------------------------------------------- *
 */
#include "log_mem_dump.h"
#include "log_test.h"
#include "log_enforce.h"

/** -------------------------------------------------------------------------- *
 * logging utils inclusion
 * --------------------------------------------------------------------------- *
 */
#include "log_utils.h"

/* -- end of file ----------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __LOG_LIB_H__ */
