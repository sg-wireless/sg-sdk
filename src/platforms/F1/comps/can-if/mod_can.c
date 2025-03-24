#include "mp_lite_if.h"

#define __log_subsystem F1
#define __log_component can
#include "log_lib.h"
#include <string.h>

#include "can.h"

#define NORMAL  0
#define LOOPBACK 1

__mp_mod_fun_var_between(can, init,0,4)(size_t __arg_n, const mp_obj_t * __arg_v)
{
    uint8_t RxPin= RX_GPIO_NUM;
    uint8_t TxPin= TX_GPIO_NUM;
    uint32_t Baud= DEFAULT_BAUD;
    uint8_t Mode= DEFAULT_MODE;
    
    if(__arg_n == 4)
    {
      RxPin = mp_obj_get_int(__arg_v[0]);
      TxPin = mp_obj_get_int(__arg_v[1]);
      Baud = mp_obj_get_int(__arg_v[2]);
      Mode = mp_obj_get_int(__arg_v[3]);
    }
    can_init(RxPin,TxPin,Baud,Mode);
    return mp_const_none;
}

__mp_mod_fun_0(can, deinit)(void)
{
    can_deinit();
    return mp_const_none;
}

__mp_mod_fun_3(can, send)(mp_obj_t flags,mp_obj_t id,mp_obj_t dat)
{ 
  if (mp_obj_is_str(dat) || mp_obj_is_type(dat, &mp_type_bytes))
  {
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(dat, &bufinfo, MP_BUFFER_READ);
    if(bufinfo.len<=TWAI_FRAME_MAX_DLC)
    {
      send_message.flags = mp_obj_get_int(flags);
      send_message.identifier = mp_obj_get_int(id);
      send_message.data_length_code = bufinfo.len;
      memcpy(send_message.data,bufinfo.buf,bufinfo.len);
      can_send();
    }
  }
  return mp_const_none;
}

__mp_mod_fun_2(can, filter)(mp_obj_t dat,mp_obj_t single_filter)
{ 
  if (mp_obj_is_str(dat) || mp_obj_is_type(dat, &mp_type_bytes))
  {
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(dat, &bufinfo, MP_BUFFER_READ);
    if(bufinfo.len<=8)
    {
      memcpy(&f_config,bufinfo.buf,bufinfo.len);
      f_config.single_filter   = mp_obj_is_true(single_filter);
    }
  }
  return mp_const_none;
}


__mp_mod_fun_0(can, any)(void)
{
    return mp_obj_new_int(IsAnyDate());
}

__mp_mod_fun_0(can, recv)(void)
{
    circularDatType *p = can_read();
    
    return mp_obj_new_bytes((const byte *)p,sizeof(circularDatType));
}


