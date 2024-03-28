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
 * @brief   This file contains the definition of internal objects to the log
 *          library
 * --------------------------------------------------------------------------- *
 */

#ifndef __LOG_OBJ_H__
#define __LOG_OBJ_H__

#ifdef __cplusplus
extern "C" {
#endif

/* --- include -------------------------------------------------------------- */

#include <stdint.h>

#include "utils_misc.h"
#include "log_config.h"

/** -------------------------------------------------------------------------- *
 * design approach:
 * ================
 * 
 *  +---------------------------+           +--------------------------------+
 *  |   << log_info_base_t >>   |           |      << log_type_info_t >>     |
 *  +---------------------------+           +--------------------------------+
 *  | - log_obj: log_info_t *   +------+    | - p_provider: provider_func_t *|
 *  | - buf:         char*      |      |    | - type_name: char *            |
 *  | - idx:         int        |      |    | - flags: uint32_t              |
 *  | - comp_id:     int        |      |    +--------------+-----------------+
 *  | - subsys_id:   int        |      |                  /|\
 *  | - color_len:   int        |      |                   |
 *  | - log_color:   int        |      |                   |
 *  | - curr_color:  int        |      |                   |
 *  | - file:  const char*      |      | uses              |
 *  | - line:  int              |      |                   |
 *  | - func:  const char*      |      |                   |
 *  |                           |      |                   |
 *  +-------------+-------------+      |                   |
 *               /_\                   |                   |
 *                |                    |                   |
 *                |      inherits      |                   |
 *                +-----------------+  |                   |
 *                                  |  |                   |
 *                                  | \|/                  |
 *                           +------+--+-----------+       | 
 *                           |   << log_info_t >>  |       | composed of
 *                           +---------------------+       |
 *                           | log_type_info_t * p +-------+
 *                           +----------+----------+
 *                                     /_\
 *                                      | inherits
 *                                      | 
 *                           +----------+----------+
 *                           |       derived       |
 *                           |   << log_info_t >>  |
 *                           +---------------------+
 *                           |  all extra specific |
 *                           |  log information    |
 *                           |  of this type       |
 *                           +---------------------+
 * 
 * user space:
 *      << log_info_t >>        This typedef is visible to the user and defined
 *                              in "log_lib.h". All log instances are of this
 *                              type or one of its derived types.
 * 
 *      << log_type_info_t >>   This typedef is visible to the user and defined
 *                              in "log_lib.h". The objects defined from this
 *                              type are singleton. All logs of a certain type
 *                              refer to the singleton object for the sake of
 *                              type information retrieval.
 * 
 *      << log_info_base_t >>   This type is private to the log lib and visible
 *                              only internally by the log library files.
 *                              It carries all needed information during the
 *                              provisioning of the log instance.
 *                              It is defined here in this file.
 * 
 * --------------------------------------------------------------------------- *
 */

/* --- typedefs ------------------------------------------------------------- */

/**
 * @details     An object of this struct shall be defined locally for each
 *              incoming log to keep the buffering information across all
 *              providers that participate in providing this log instance.
 *              also keeps the related info specific to this log.
 *              For new log types that need extra info, they shall define
 *              their own log info structs and insert this struct as a first
 *              member of their own info struct.
 *              ex:
 *                  typedef struct {
 *                      log_info_base_t     base;
 *                      <extra member>
 *                      <extra member>
 *                      .
 *                  } log_<new-type>_info_t;
 */
typedef struct  log_info_s log_info_t;
typedef struct _log_info_base_s {
    log_info_t* log_info;    /**< reference to log type obj instance */
    char*   buf;        /**< a reference handle to the current buffer */
    int     idx;        /**< the currrent providing index */
    int     comp_id;    /**< current component id */
    int     subsys_id;  /**< current component id */
    __opt_paste(__opt_global_log_coloring, y, 
    int     color_len;  /**< length of added coloring information so far */
    int     log_color;
    int     curr_color;)
    __opt_paste(__opt_log_header_filename, y, const char* file;);
    __opt_paste(__opt_log_header_line_num, y, int         line;);
    __opt_paste(__opt_log_header_func_name, y, const char* func;);
} log_info_base_t;

/* -- end of file ----------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __LOG_OBJ_H__ */
