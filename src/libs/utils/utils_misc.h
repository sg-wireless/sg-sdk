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
 * --------------------------------------------------------------------------- *
 * @copyright Copyright (c) 2022, Pycom Limited.
 *
 * This software is licensed under the GNU GPL version 3 or any
 * later version, with permitted additional terms. For more information
 * see the Pycom Licence v1.0 document supplied with this file, or
 * available at https://www.pycom.io/opensource/licensing
 * 
 * @author  Ahmed Sabry (Pycom, SG Wireless)
 * 
 * @brief   This file offers a great helper macros for complex macros building
 *          like constitution of the premitive macros identifiers during the
 *          preprocessing stages using __concat() and __tricat().
 *          it offers converting a given text to a c literal string.
 *          it offers optional arguments pasting during preprocessing stage,
 *          which can help in injecting/ejecting things in/from compilation.
 * --------------------------------------------------------------------------- *
 */
#ifndef __UTILS_MISC_H__
#define __UTILS_MISC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* --- API Macros ----------------------------------------------------------- */

   /*******************************************************//**
    * @def __stringify(_text)
    * @details  It produce a string leteral of a given text.
    * @param _arg text to be stringified
    * @return   string of the given text "<_text>"
    **********************************************************/
    #define __stringify(_text)       __stepper_stringify(_text)
    #define __stepper_stringify(a)                          # a

   /*******************************************************//**
    * @def __concat(_a,_b)
    * @details  It produce a new generated identifier qualified
    *           from the concatenation of the given args.
    * @param _prefix the prefix of the resultant identifier
    * @param _suffix the suffix of the resultant identifier
    * @return  a new identifier <_prefix><_suffix>
    **********************************************************/
    #define __concat(_prefix,_suffix)                         \
                            __stepper_concat(_prefix , _suffix)
    #define __stepper_concat(a,b)                        a ## b

   /*******************************************************//**
    * @def __tricat(_prefix,_mid,_suffix)
    * @details  It produce a new generated identifier qualified
    *           from the concatenation of the given args.
    * @param _prefix the prefix of the resultant identifier
    * @param _mid    the middle of the resultant identifier
    * @param _suffix the suffix of the resultant identifier
    * @return  a new identifier <_prefix><_mid><_suffix>
    **********************************************************/
    #define __tricat(_prefix,_mid,_suffix)                    \
                         __tricat_stepper(_prefix,_mid,_suffix)
    #define __tricat_stepper(a,b,c)                 a ## b ## c

   /*******************************************************//**
    * @def __opt_paste(_opt, _value, _args...)
    * @details  It selects pastes the \a _args if the \a _opt
    *           resolves to \a _value
    * @param _opt   the option compilation flag (y, n, 1, 0)
    * @param _args  the arguments to be pasted
    **********************************************************/
    #define __opt_paste(_opt, _value, _args...)               \
        __concat(__tricat(__stepper_opt_paste_, _value, _),   \
            _opt)(_args)
    #define __stepper_opt_paste_y_y(_args...)  _args
    #define __stepper_opt_paste_1_1(_args...)  _args
    #define __stepper_opt_paste_y_n(_args...)
    #define __stepper_opt_paste_1_0(_args...)
    #define __stepper_opt_paste_n_y(_args...)
    #define __stepper_opt_paste_0_1(_args...)
    #define __stepper_opt_paste_n_n(_args...)  _args
    #define __stepper_opt_paste_0_0(_args...)  _args

   /*******************************************************//**
    * @def __opt_test(_opt, _val)
    * @details  It tests the options \a _opt value against
    *           \a _val and returns true or false at 
    *           preprocessor time.
    * @param _opt   the option compilation flag (y, n, 1, 0)
    * @param _val   the value to test against it (y, n, 1, 0)
    **********************************************************/
    #define __opt_test(_opt, _val)  \
        (__concat(__opt_val_, _opt) == __concat(__opt_val_, _val))
    #define __opt_val_y     1
    #define __opt_val_1     1
    #define __opt_val_n     0
    #define __opt_val_0     0

/* --- end of file ---------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __UTILS_MISC_H__ */
