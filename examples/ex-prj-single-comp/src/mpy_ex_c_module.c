

#include "log_lib.h"
#include "mp_lite_if.h"
#include "utils_time.h"

#include "head_1.h"
#include "head_2.h"

__mp_mod_fun_0(ex_prj, method_a)(void) {

    example_function_a();

    return mp_const_none;
}

__mp_mod_fun_0(ex_prj, method_b)(void) {

    example_function_b();

    return mp_const_none;
}

__mp_mod_fun_0(ex_prj, menu_defs)(void) {

    #ifdef CONFIG_EXAMPLE_PROJECT_DEF_X
    __log_output("project definition X is defined\n");
    #endif
    #ifdef CONFIG_EXAMPLE_PROJECT_DEF_Y
    __log_output("project definition Y is defined\n");
    #endif
    #ifdef CONFIG_EXAMPLE_PROJECT_DEF_Z
    __log_output("project definition Z is defined\n");
    #endif

    return mp_const_none;
}

__mp_mod_fun_1(ex_prj, gps_to_unix)(mp_obj_t obj) {

    if( mp_obj_is_int(obj) )
    {
        uint32_t t_gps = mp_obj_get_int(obj);
        uint32_t t_unx = time_gps_to_unix(t_gps);
        return mp_obj_new_int(t_unx);
    }
    else
    {
        mp_raise_TypeError(MP_ERROR_TEXT("passing non integer object"));
    }

    return mp_const_none;
}

__mp_mod_fun_1(ex_prj, unix_to_gps)(mp_obj_t obj) {

    if( mp_obj_is_int(obj) )
    {
        uint32_t t_unx = mp_obj_get_int(obj);
        uint32_t t_gps = time_unix_to_gps(t_unx);
        return mp_obj_new_int(t_gps);
    }
    else
    {
        mp_raise_TypeError(MP_ERROR_TEXT("passing non integer object"));
    }

    return mp_const_none;
}
