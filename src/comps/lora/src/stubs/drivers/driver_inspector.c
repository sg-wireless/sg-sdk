/** -------------------------------------------------------------------------- *
 * Copyright (c) 2023-2024 SG Wireless - All Rights Reserved
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
 * @brief   an inspector analyser for the Semtech chip SPI protocol
 * --------------------------------------------------------------------------- *
 */

/** -------------------------------------------------------------------------- *
 * includes
 * --------------------------------------------------------------------------- *
 */
#include <stdint.h>

#include "sx126x.h"
#include "LoRaMac.h"
#include "utils_bitwise.h"

#include "sx126x_defs.h"

#define __log_subsystem lora
#define __log_component port_analyser
#include "log_lib.h"

/** -------------------------------------------------------------------------- *
 * macros
 * --------------------------------------------------------------------------- *
 */
#define __declare_lora_cmd_desc_print(cmd) \
    static void cmd ## _print (uint16_t addr, uint8_t* buf, uint32_t size)
__declare_lora_cmd_desc_print(__sx126x_cmd_set_sleep                  );
__declare_lora_cmd_desc_print(__sx126x_cmd_set_standby                );
__declare_lora_cmd_desc_print(__sx126x_cmd_set_fs                     );
__declare_lora_cmd_desc_print(__sx126x_cmd_set_tx                     );
__declare_lora_cmd_desc_print(__sx126x_cmd_set_rx                     );
__declare_lora_cmd_desc_print(__sx126x_cmd_stop_timer_on_preample     );
__declare_lora_cmd_desc_print(__sx126x_cmd_set_rx_duty_cycle          );
__declare_lora_cmd_desc_print(__sx126x_cmd_set_cad                    );
__declare_lora_cmd_desc_print(__sx126x_cmd_set_tx_continuous_wave     );
__declare_lora_cmd_desc_print(__sx126x_cmd_set_tx_infinite_preamble   );
__declare_lora_cmd_desc_print(__sx126x_cmd_set_regulator_mode         );
__declare_lora_cmd_desc_print(__sx126x_cmd_calibrate                  );
__declare_lora_cmd_desc_print(__sx126x_cmd_calibrate_image            );
__declare_lora_cmd_desc_print(__sx126x_cmd_set_pa_config              );
__declare_lora_cmd_desc_print(__sx126x_cmd_rx_tx_fallback_mode        );
__declare_lora_cmd_desc_print(__sx126x_cmd_write_register             );
__declare_lora_cmd_desc_print(__sx126x_cmd_read_register              );
__declare_lora_cmd_desc_print(__sx126x_cmd_write_buffer               );
__declare_lora_cmd_desc_print(__sx126x_cmd_read_buffer                );
__declare_lora_cmd_desc_print(__sx126x_cmd_set_dio_irq_params         );
__declare_lora_cmd_desc_print(__sx126x_cmd_get_irq_status             );
__declare_lora_cmd_desc_print(__sx126x_cmd_clear_irq_status           );
__declare_lora_cmd_desc_print(__sx126x_cmd_set_dio_2_as_rf_switch_ctrl);
__declare_lora_cmd_desc_print(__sx126x_cmd_set_dio_3_as_tcxo_ctrl     );
__declare_lora_cmd_desc_print(__sx126x_cmd_set_rf_frequency           );
__declare_lora_cmd_desc_print(__sx126x_cmd_set_packet_type            );
__declare_lora_cmd_desc_print(__sx126x_cmd_get_packet_type            );
__declare_lora_cmd_desc_print(__sx126x_cmd_set_tx_params              );
__declare_lora_cmd_desc_print(__sx126x_cmd_set_modulation_params      );
__declare_lora_cmd_desc_print(__sx126x_cmd_set_packet_params          );
__declare_lora_cmd_desc_print(__sx126x_cmd_set_cad_params             );
__declare_lora_cmd_desc_print(__sx126x_cmd_set_buffer_base_address    );
__declare_lora_cmd_desc_print(__sx126x_cmd_set_lora_symb_num_timeout  );
__declare_lora_cmd_desc_print(__sx126x_cmd_get_status                 );
__declare_lora_cmd_desc_print(__sx126x_cmd_get_rssi_inst              );
__declare_lora_cmd_desc_print(__sx126x_cmd_get_rx_buffer_status       );
__declare_lora_cmd_desc_print(__sx126x_cmd_get_packet_status          );
__declare_lora_cmd_desc_print(__sx126x_cmd_get_device_errors          );
__declare_lora_cmd_desc_print(__sx126x_cmd_clear_device_errors        );
__declare_lora_cmd_desc_print(__sx126x_cmd_get_stats                  );
__declare_lora_cmd_desc_print(__sx126x_cmd_reset_stats                );

static struct {
    uint8_t cmd;
    const char* name;
    void(*p_log_info)(uint16_t addr, uint8_t* buf, uint32_t size);
} s_lora_sx1262_cmd_desc [] = {
    #define __init_lora_cmd_desc(cmd) {cmd, #cmd, cmd ## _print}
    __init_lora_cmd_desc(__sx126x_cmd_set_sleep                  ),
    __init_lora_cmd_desc(__sx126x_cmd_set_standby                ),
    __init_lora_cmd_desc(__sx126x_cmd_set_fs                     ),
    __init_lora_cmd_desc(__sx126x_cmd_set_tx                     ),
    __init_lora_cmd_desc(__sx126x_cmd_set_rx                     ),
    __init_lora_cmd_desc(__sx126x_cmd_stop_timer_on_preample     ),
    __init_lora_cmd_desc(__sx126x_cmd_set_rx_duty_cycle          ),
    __init_lora_cmd_desc(__sx126x_cmd_set_cad                    ),
    __init_lora_cmd_desc(__sx126x_cmd_set_tx_continuous_wave     ),
    __init_lora_cmd_desc(__sx126x_cmd_set_tx_infinite_preamble   ),
    __init_lora_cmd_desc(__sx126x_cmd_set_regulator_mode         ),
    __init_lora_cmd_desc(__sx126x_cmd_calibrate                  ),
    __init_lora_cmd_desc(__sx126x_cmd_calibrate_image            ),
    __init_lora_cmd_desc(__sx126x_cmd_set_pa_config              ),
    __init_lora_cmd_desc(__sx126x_cmd_rx_tx_fallback_mode        ),
    __init_lora_cmd_desc(__sx126x_cmd_write_register             ),
    __init_lora_cmd_desc(__sx126x_cmd_read_register              ),
    __init_lora_cmd_desc(__sx126x_cmd_write_buffer               ),
    __init_lora_cmd_desc(__sx126x_cmd_read_buffer                ),
    __init_lora_cmd_desc(__sx126x_cmd_set_dio_irq_params         ),
    __init_lora_cmd_desc(__sx126x_cmd_get_irq_status             ),
    __init_lora_cmd_desc(__sx126x_cmd_clear_irq_status           ),
    __init_lora_cmd_desc(__sx126x_cmd_set_dio_2_as_rf_switch_ctrl),
    __init_lora_cmd_desc(__sx126x_cmd_set_dio_3_as_tcxo_ctrl     ),
    __init_lora_cmd_desc(__sx126x_cmd_set_rf_frequency           ),
    __init_lora_cmd_desc(__sx126x_cmd_set_packet_type            ),
    __init_lora_cmd_desc(__sx126x_cmd_get_packet_type            ),
    __init_lora_cmd_desc(__sx126x_cmd_set_tx_params              ),
    __init_lora_cmd_desc(__sx126x_cmd_set_modulation_params      ),
    __init_lora_cmd_desc(__sx126x_cmd_set_packet_params          ),
    __init_lora_cmd_desc(__sx126x_cmd_set_cad_params             ),
    __init_lora_cmd_desc(__sx126x_cmd_set_buffer_base_address    ),
    __init_lora_cmd_desc(__sx126x_cmd_set_lora_symb_num_timeout  ),
    __init_lora_cmd_desc(__sx126x_cmd_get_status                 ),
    __init_lora_cmd_desc(__sx126x_cmd_get_rssi_inst              ),
    __init_lora_cmd_desc(__sx126x_cmd_get_rx_buffer_status       ),
    __init_lora_cmd_desc(__sx126x_cmd_get_packet_status          ),
    __init_lora_cmd_desc(__sx126x_cmd_get_device_errors          ),
    __init_lora_cmd_desc(__sx126x_cmd_clear_device_errors        ),
    __init_lora_cmd_desc(__sx126x_cmd_get_stats                  ),
    __init_lora_cmd_desc(__sx126x_cmd_reset_stats                )
};
#define __sx126x_cmd_count \
    (sizeof(s_lora_sx1262_cmd_desc)/sizeof(s_lora_sx1262_cmd_desc[0]))

const char* lora_port_get_cmd_name(uint8_t cmd)
{
    int i;
    for(i = 0; i < __sx126x_cmd_count; ++i) {
        if( s_lora_sx1262_cmd_desc[i].cmd == cmd )
            return s_lora_sx1262_cmd_desc[i].name;
    }
    return "unknown lora cmd";
}

static struct {
    uint16_t reg;
    const char* name;
} s_lora_sx1262_reg_desc [] = {
    #define __init_lora_reg_desc(reg)    {reg, #reg}
    __init_lora_reg_desc(__sx126x_reg_hopping_enable              ),
    __init_lora_reg_desc(__sx126x_reg_packet_length               ),
    __init_lora_reg_desc(__sx126x_reg_nb_hopping_blocks           ),
    __init_lora_reg_desc(__sx126x_reg_diox_output_enable          ),
    __init_lora_reg_desc(__sx126x_reg_diox_input_enable           ),
    __init_lora_reg_desc(__sx126x_reg_diox_pull_up_control        ),
    __init_lora_reg_desc(__sx126x_reg_diox_pull_down_control      ),
    __init_lora_reg_desc(__sx126x_reg_whitening_initial_value_msb ),
    __init_lora_reg_desc(__sx126x_reg_whitening_initial_value_lsb ),
    __init_lora_reg_desc(__sx126x_reg_crc_msb_initial_value       ),
    __init_lora_reg_desc(__sx126x_reg_crc_lsb_initial_value       ),
    __init_lora_reg_desc(__sx126x_reg_crc_msb_polynomial_value_0  ),
    __init_lora_reg_desc(__sx126x_reg_crc_lsb_polynomial_value_1  ),
    __init_lora_reg_desc(__sx126x_reg_sync_word_0                 ),
    __init_lora_reg_desc(__sx126x_reg_sync_word_1                 ),
    __init_lora_reg_desc(__sx126x_reg_sync_word_2                 ),
    __init_lora_reg_desc(__sx126x_reg_sync_word_3                 ),
    __init_lora_reg_desc(__sx126x_reg_sync_word_4                 ),
    __init_lora_reg_desc(__sx126x_reg_sync_word_5                 ),
    __init_lora_reg_desc(__sx126x_reg_sync_word_6                 ),
    __init_lora_reg_desc(__sx126x_reg_sync_word_7                 ),
    __init_lora_reg_desc(__sx126x_reg_node_address                ),
    __init_lora_reg_desc(__sx126x_reg_broadcast_address           ),
    __init_lora_reg_desc(__sx126x_reg_iq_polarity_setup           ),
    __init_lora_reg_desc(__sx126x_reg_lora_sync_word_msb          ),
    __init_lora_reg_desc(__sx126x_reg_lora_sync_word_lsb          ),
    __init_lora_reg_desc(__sx126x_reg_random_num_gen_0            ),
    __init_lora_reg_desc(__sx126x_reg_random_num_gen_1            ),
    __init_lora_reg_desc(__sx126x_reg_random_num_gen_2            ),
    __init_lora_reg_desc(__sx126x_reg_random_num_gen_3            ),
    __init_lora_reg_desc(__sx126x_reg_tx_modulation               ),
    __init_lora_reg_desc(__sx126x_reg_rx_gain                     ),
    __init_lora_reg_desc(__sx126x_reg_tx_clamp_config             ),
    __init_lora_reg_desc(__sx126x_reg_ocp_configuration           ),
    __init_lora_reg_desc(__sx126x_reg_rtc_control                 ),
    __init_lora_reg_desc(__sx126x_reg_xta_trim                    ),
    __init_lora_reg_desc(__sx126x_reg_xtb_trim                    ),
    __init_lora_reg_desc(__sx126x_reg_dio3_output_volatage_control),
    __init_lora_reg_desc(__sx126x_reg_event_mask                  )
};
#define __sx126x_reg_count \
    (sizeof(s_lora_sx1262_reg_desc)/sizeof(s_lora_sx1262_reg_desc[0]))


static char s_lora_reg_nb_symbols_name[] = {"__lora_re_nb_symbols_  _reg_ "};
#define __sx126x_reg_nb_symbols_r(idx, reg)               \
    s_lora_reg_nb_symbols_name[21] = '0' + idx / 10;    \
    s_lora_reg_nb_symbols_name[22] = '0' + idx % 10;    \
    s_lora_reg_nb_symbols_name[28] = '0' + reg
static char s_lora_reg_freq_name[] = {"__sx126x_reg_freq_  _ "};
#define __sx126x_reg_freq_r(idx, reg)            \
    s_lora_reg_freq_name[16] = '0' + idx / 10;  \
    s_lora_reg_freq_name[17] = '0' + idx % 10;  \
    s_lora_reg_freq_name[19] = '0' + reg

/** -------------------------------------------------------------------------- *
 * analyser implementation
 * --------------------------------------------------------------------------- *
 */
const char* lora_port_get_reg_name(uint16_t reg)
{
    if(reg >= __sx126x_reg_nb_symbols_r0(0) &&
        reg <= __sx126x_reg_freq_r3(15))
    {
        int offset = reg - __sx126x_reg_nb_symbols_r0(0);
        int idx = offset / 6;
        int rank = offset % 6;
        if(rank == 0 || rank == 1) {
            __sx126x_reg_nb_symbols_r(idx, rank);
            return s_lora_reg_nb_symbols_name;
        } else {
            rank -= 2;
            __sx126x_reg_freq_r(idx, rank);
            return s_lora_reg_freq_name;
        }
    } else {
        int i;
        for(i = 0; i < __sx126x_reg_count; ++i) {
            if( s_lora_sx1262_reg_desc[i].reg == reg )
                return s_lora_sx1262_reg_desc[i].name;
        }
    }
    return "unknown lora reg";
}
static const char* s_lora_operating_mode_names [] = {
    "MODE_SLEEP",
    "MODE_STDBY_RC",
    "MODE_STDBY_XOSC",
    "MODE_FS",
    "MODE_TX",
    "MODE_RX",
    "MODE_RX_DC",
    "MODE_CAD"
};
#define __lora_operating_mode_count \
    (sizeof(s_lora_operating_mode_names)/sizeof(s_lora_operating_mode_names[0]))

const char* lora_port_get_operating_mode_name(RadioOperatingModes_t mode)
{
    if(mode >= __lora_operating_mode_count)
        return "unknown mode name";
    return s_lora_operating_mode_names[mode];
}

static const char* s_lora_mac_status_names [] = {
    "LORAMAC_STATUS_OK",
    "LORAMAC_STATUS_BUSY",
    "LORAMAC_STATUS_SERVICE_UNKNOWN",
    "LORAMAC_STATUS_PARAMETER_INVALID",
    "LORAMAC_STATUS_FREQUENCY_INVALID",
    "LORAMAC_STATUS_DATARATE_INVALID",
    "LORAMAC_STATUS_FREQ_AND_DR_INVALID",
    "LORAMAC_STATUS_NO_NETWORK_JOINED",
    "LORAMAC_STATUS_LENGTH_ERROR",
    "LORAMAC_STATUS_REGION_NOT_SUPPORTED",
    "LORAMAC_STATUS_SKIPPED_APP_DATA",
    "LORAMAC_STATUS_DUTYCYCLE_RESTRICTED",
    "LORAMAC_STATUS_NO_CHANNEL_FOUND",
    "LORAMAC_STATUS_NO_FREE_CHANNEL_FOUND",
    "LORAMAC_STATUS_BUSY_BEACON_RESERVED_TIME",
    "LORAMAC_STATUS_BUSY_PING_SLOT_WINDOW_TIME",
    "LORAMAC_STATUS_BUSY_UPLINK_COLLISION",
    "LORAMAC_STATUS_CRYPTO_ERROR",
    "LORAMAC_STATUS_FCNT_HANDLER_ERROR",
    "LORAMAC_STATUS_MAC_COMMAD_ERROR",
    "LORAMAC_STATUS_CLASS_B_ERROR",
    "LORAMAC_STATUS_CONFIRM_QUEUE_ERROR",
    "LORAMAC_STATUS_MC_GROUP_UNDEFINED",
    "LORAMAC_STATUS_ERROR"
};
#define __lora_status_count \
    (sizeof(s_lora_mac_status_names)/sizeof(s_lora_mac_status_names[0]))

const char* lora_port_get_status_name(LoRaMacStatus_t status)
{
    if(status >= __lora_status_count)
        return "unknown status";
    return s_lora_mac_status_names[status];
}

void lora_log_trx(
    bool is_read,
    uint8_t cmd,
    uint16_t addr,
    uint8_t*buf,
    uint32_t size)
{
    int i;
    for(i = 0; i < __sx126x_cmd_count; ++i) {
        if( s_lora_sx1262_cmd_desc[i].cmd == cmd )
            break;
    }

    __log_info("lora trx [%s"__default__"] cmd:"__cyan__"%02X"__default__
                " ("__blue__ "%-40s" __default__") buf-size: "__green__"%d",
        is_read ? __blue__"R" : __red__"W", cmd,
        i < __sx126x_cmd_count ? s_lora_sx1262_cmd_desc[i].name :
            __red__ "unknown cmd", size
        );
    if(i < __sx126x_cmd_count) {
        __log_printf(" => cmd desc: ");
        s_lora_sx1262_cmd_desc[i].p_log_info(addr, buf, size);
        __log_endl();
    }
}

static const char * s_on_off_str[] = { __red__ "off" __default__,
    __green__ "on" __default__};
static void check_typical_buf_size(uint32_t size, uint32_t typical) {
    if(size != typical)
        __log_error("wrong expected buffer size %d (typical: %d)",
            size, typical);
    if(typical == 0)
    __log_printf("-- NA --");
}
__declare_lora_cmd_desc_print(__sx126x_cmd_set_sleep                  )
{
    check_typical_buf_size(size, 1);
    __log_printf("rtc_wakeup: %s , %s start",
        s_on_off_str[__bitwise_bit_get(8, buf[0], 0)],
        __bitwise_bit_get(8, buf[0], 2) ? __purple__ "warm" __default__ :
            __blue__ "cold" __default__);
}
__declare_lora_cmd_desc_print(__sx126x_cmd_set_standby                )
{
    check_typical_buf_size(size, 1);
    __log_printf("StdbyConfig: %s",
        buf[0] == 0 ? "STDBY_RC ( %CgRC 13MHz%Cd )"
                    : "STDBY_XOSC ( %CgXTAL 32MHz%Cd )");
}
__declare_lora_cmd_desc_print(__sx126x_cmd_set_fs                     )
{
}
__declare_lora_cmd_desc_print(__sx126x_cmd_set_tx                     )
{
    check_typical_buf_size(size, 3);
    uint32_t timeout = 0;
    timeout |= ((uint32_t)buf[0]) << 16;
    timeout |= ((uint32_t)buf[1]) << 8;
    timeout |= (uint32_t)buf[2];
    if(timeout == 0)
        __log_printf("timeout disabled");
    else
    {
        float t = (double)timeout * __time_us(15.625);
        __log_printf("timeout = %06x =  %.3f sec", timeout, t);
    }
}
__declare_lora_cmd_desc_print(__sx126x_cmd_set_rx                     )
{
    check_typical_buf_size(size, 3);
    uint32_t timeout = 0;
    timeout |= ((uint32_t)buf[0]) << 16;
    timeout |= ((uint32_t)buf[1]) << 8;
    timeout |= (uint32_t)buf[2];
    if(timeout == 0)
        __log_printf("timeout disabled");
    else if(timeout == 0xffffffu)
        __log_printf("rx continuous mode");
    else
    {
        float t = (double)timeout * __time_us(15.625);
        __log_printf("timeout = %06x =  %.3f sec", timeout, t);
    }
}
__declare_lora_cmd_desc_print(__sx126x_cmd_stop_timer_on_preample     )
{
    check_typical_buf_size(size, 1);
    if(buf[0] == 0)
        __log_printf("stop on sync word or header detection");
    else
        __log_printf("stop on preamble detection");

}
__declare_lora_cmd_desc_print(__sx126x_cmd_set_rx_duty_cycle          )
{
    check_typical_buf_size(size, 6);
    uint32_t rx_period = buf[0] | (buf[1]<<8) | (buf[2]<< 16);
    uint32_t sleep_period = buf[3] | (buf[4]<<8) | (buf[5]<< 16);
    
    __log_printf("rx_period = %06x =  %d usec", rx_period,
            (uint32_t)((float)rx_period * 15.625f));
    __log_printf("rx_period = %06x =  %d usec", sleep_period,
            (uint32_t)((float)sleep_period * 15.625f));
}
__declare_lora_cmd_desc_print(__sx126x_cmd_set_cad                    )
{
    check_typical_buf_size(size, 0);
}
__declare_lora_cmd_desc_print(__sx126x_cmd_set_tx_continuous_wave     )
{
    check_typical_buf_size(size, 0);
}
__declare_lora_cmd_desc_print(__sx126x_cmd_set_tx_infinite_preamble   )
{
    check_typical_buf_size(size, 0);
}
__declare_lora_cmd_desc_print(__sx126x_cmd_set_regulator_mode         )
{
    check_typical_buf_size(size, 1);
    __log_printf("regModeParam : %s",
        buf[0] ? "DC_DC+LDO used for STBY_XOSC,FS, RX and TX modes"
               : "Only LDO used for all modes");
}
static const char* s_enable_str[] = {__red__"disabled", __green__"enabled"};
__declare_lora_cmd_desc_print(__sx126x_cmd_calibrate                  )
{
    const char** str = s_enable_str;
    check_typical_buf_size(size, 1);
    __log_printf("Calibration Settings:");
    __log_info("\t - RC 64K     %s", str[__bitwise_bit_get(8, buf[0], 0)]);
    __log_info("\t - RC 13M     %s", str[__bitwise_bit_get(8, buf[0], 1)]);
    __log_info("\t - PLL        %s", str[__bitwise_bit_get(8, buf[0], 2)]);
    __log_info("\t - ADC pulse  %s", str[__bitwise_bit_get(8, buf[0], 3)]);
    __log_info("\t - ADC bulk N %s", str[__bitwise_bit_get(8, buf[0], 4)]);
    __log_info("\t - ADC bulk P %s", str[__bitwise_bit_get(8, buf[0], 5)]);
    __log_info("\t - Image      %s", str[__bitwise_bit_get(8, buf[0], 6)]);
}
__declare_lora_cmd_desc_print(__sx126x_cmd_calibrate_image            )
{
    check_typical_buf_size(size, 2);
    uint32_t f1 = 0, f2 = 0;
    if(buf[0] == 0x6B && buf[1] == 0x6F) {
        f1 = 430u; f2 = 440u;
    } else if(buf[0] == 0x75 && buf[1] == 0x81) {
        f1 = 470u; f2 = 510u;
    } else if(buf[0] == 0xC1 && buf[1] == 0xC5) {
        f1 = 779u; f2 = 787u;
    } else if(buf[0] == 0xD7 && buf[1] == 0xDB) {
        f1 = 863u; f2 = 870u;
    } else if(buf[0] == 0xE1 && buf[1] == 0xE9) {
        f1 = 902u; f2 = 928u;
    }
    __log_printf("Freq Band [MHz] : (%Cb%3d - %3d%Cd) [%02X - %02X]",
        f1, f2, buf[0], buf[1]);
}
__declare_lora_cmd_desc_print(__sx126x_cmd_set_pa_config              )
{
    check_typical_buf_size(size, 4);
    __log_info("\t - paDutyCycle : %02x = %d", buf[0], buf[0]);
    __log_info("\t - hpMax       : %02x = %d", buf[1], buf[1]);
    __log_info("\t - deviceSel   : %s", buf[2] ? "sx1261" : "sx1262");
    __log_info("\t - paLut       : %d", buf[3]);
}
__declare_lora_cmd_desc_print(__sx126x_cmd_rx_tx_fallback_mode        )
{
    check_typical_buf_size(size, 1);
    __log_printf("The Radio goes into " __purple__);
    if(buf[0] == 0x40) __log_printf("FS");
    else if(buf[0] == 0x30) __log_printf("STDBY_XOSC");
    else if(buf[0] == 0x20) __log_printf("STDBY_RC");
    else __log_printf(__red__"Unknown");
    __log_printf(__default__ " mode after Tx or Rx");
}
__declare_lora_cmd_desc_print(__sx126x_cmd_write_register             )
{
    __log_printf("addr %04X (%Cc%s%Cd), buf-size: %d",
        addr, lora_port_get_reg_name(addr), size);
    __log_dump(buf, size, 16, 0, __word_len_8);
}
__declare_lora_cmd_desc_print(__sx126x_cmd_read_register              )
{
    __log_printf("addr %04X (%Cc%s%Cd), buf-size: %d",
        addr, lora_port_get_reg_name(addr), size);
    __log_dump(buf, size, 16, 0, __word_len_8);
}
__declare_lora_cmd_desc_print(__sx126x_cmd_write_buffer               )
{
    __log_printf("offset %04X, buf-size: %d", addr, size);
    __log_dump(buf, size, 8,
        __log_dump_flag_disp_char|__log_dump_flag_disp_char_on_rhs,
        __word_len_8);
}
__declare_lora_cmd_desc_print(__sx126x_cmd_read_buffer                )
{
    __log_printf("offset %04X, buf-size: %d", addr, size);
    __log_dump(buf, size, 8,
        __log_dump_flag_disp_char|__log_dump_flag_disp_char_on_rhs,
        __word_len_8);
}
static const char* irq_str[] = {
    "tx_done",
    "rx_done",
    "preamble_detect",
    "sync_word_valid",
    "header_valid",
    "header_err",
    "crc_err",
    "cad_done",
    "cad_detected",
    "timout",0,0,0,0,
    "lr_fhss_hop"
};
__declare_lora_cmd_desc_print(__sx126x_cmd_set_dio_irq_params         )
{
    check_typical_buf_size(size, 8);
    uint16_t irq_msk  = (((uint32_t)buf[0]) << 8) | buf[1];
    uint16_t dio1_msk = (((uint32_t)buf[2]) << 8) | buf[3];
    uint16_t dio2_msk = (((uint32_t)buf[4]) << 8) | buf[5];
    uint16_t dio3_msk = (((uint32_t)buf[6]) << 8) | buf[7];

    __log_printf("\n\t n %-15s %-4s  %-4s %-4s %-4s", "irq-name",
        "irq", "dio1", "dio2", "dio3");
    int i = 0;
    while(i < 15) {
        __log_printf("\n\t%2d %-15s %-4s  %-4s %-4s %-4s",
            i, irq_str[i],
            s_on_off_str[__bitwise_bit_get(16, irq_msk, i)],
            s_on_off_str[__bitwise_bit_get(16, dio1_msk, i)],
            s_on_off_str[__bitwise_bit_get(16, dio2_msk, i)],
            s_on_off_str[__bitwise_bit_get(16, dio3_msk, i)]);
        if(i == 9) i = 14;
        else ++i;
    }
}
__declare_lora_cmd_desc_print(__sx126x_cmd_get_irq_status             )
{
    check_typical_buf_size(size, 2);
    __log_printf("occurred events are:");
    uint16_t irq_status = (((uint32_t)buf[0]) << 8) | buf[1];
    int i = 0;
    while(i < 15) {
        if(__bitwise_bit_get(16, irq_status, i))
        {
            __log_printf("\n\t%2d - %s", i, irq_str[i]);
        }
        if(i == 9) i = 14;
        else ++i;
    }
}
__declare_lora_cmd_desc_print(__sx126x_cmd_clear_irq_status           )
{
    check_typical_buf_size(size, 2);
    __log_printf("the following irq status will be cleared:");
    uint16_t irq_status = (((uint32_t)buf[0]) << 8) | buf[1];
    int i = 0;
    while(i < 15) {
        if(__bitwise_bit_get(16, irq_status, i))
            __log_printf("\n\t%2d - %s", i, irq_str[i]);
        if(i == 9) i = 14;
        else ++i;
    }
}
__declare_lora_cmd_desc_print(__sx126x_cmd_set_dio_2_as_rf_switch_ctrl)
{
    check_typical_buf_size(size, 1);
    __log_printf("DIO-2 is %s", buf[0] ?
        "used as an RF switch control" : "free for IRQ usage");
}
__declare_lora_cmd_desc_print(__sx126x_cmd_set_dio_3_as_tcxo_ctrl     )
{
    check_typical_buf_size(size, 4);
    const char* v_str[] = {
        "1.6", "1.7", "1.8", "2.2", "2.4", "2.7", "3.0", "3.3"};
    if(buf[0] <= 7)
        __log_printf("tcxoVoltage: %02x "
            "-- DIO-3 outputs %3s V to supply the TCXO",
            buf[0], v_str[buf[0]]);
    else
        __log_error("wrong tcxoVoltage voltage settings: %02x", buf[0]);
    uint32_t delay = buf[1] | (buf[2]<<8) | (buf[3]<<16);
    __log_printf("\n\tdelay_duration = %06x = %d usec", delay,
         (uint32_t)((float)delay * 15.625f));
}
__declare_lora_cmd_desc_print(__sx126x_cmd_set_rf_frequency           )
{
    check_typical_buf_size(size, 4);
    uint64_t rf_freq = 0;
    rf_freq |= (uint64_t)(buf[0]) << 24;
    rf_freq |= (uint64_t)(buf[1]) << 16;
    rf_freq |= (uint64_t)(buf[2]) << 8;
    rf_freq |= (uint64_t)(buf[3]);
    rf_freq *= 32000000ULL;
    rf_freq >>= 25;

    uint32_t t = (uint32_t)rf_freq;
    __log_printf("RF Freq = %lu ( %08x )", t, *(uint32_t*)buf);
}
__declare_lora_cmd_desc_print(__sx126x_cmd_set_packet_type            )
{
    check_typical_buf_size(size, 1);
    __log_printf("packet type: %s",
        buf[0] == 0 ? "GFSK" : buf[0] == 1 ? "LoRa" : "LR_FHSS");
}
__declare_lora_cmd_desc_print(__sx126x_cmd_get_packet_type            )
{
    check_typical_buf_size(size, 1);
    __log_printf("packet type: %s",
        buf[0] == 0 ? "GFSK" : buf[0] == 1 ? "LoRa" : "LR_FHSS");
}
__declare_lora_cmd_desc_print(__sx126x_cmd_set_tx_params              )
{
    check_typical_buf_size(size, 2);
    int8_t power = *(int8_t*)buf;
    __log_printf("tx parameters are:\n");
    __log_printf("\t - power : %+d dBm\n", power);
    uint16_t ramp_times[] = {10, 20, 40, 80, 200, 800, 1700, 3400};
    if(buf[1] <= 7)
        __log_printf("\t - RampTime : %d usec\n", ramp_times[buf[1]]);
    else
        __log_error("\t - wrong ramp time settings %02x", buf[1]);
}
__declare_lora_cmd_desc_print(__sx126x_cmd_set_modulation_params      )
{
    __log_printf("modulation parameters are:");
    const char* bw_str[] = {
        "7.81", "15.63", "31.25", "62.50", "125", "250", "500", NULL,
        "10.42", "20.83", "41.67"
    };
    uint8_t sf = buf[0];
    uint8_t bw = buf[1];
    uint8_t cr = buf[2];
    uint8_t dr = buf[3];
    __log_printf("\n\t - [%02x] sf : %Cg%d%Cd", sf, sf);

    if(bw < 10 && bw != 7)
        __log_printf("\n\t - [%02x] bw : %s", bw, bw_str[bw]);
    else
        __log_error("\t- [%02x] wrong BW setting", bw);
    if(cr > 0 && cr < 5)
        __log_printf("\n\t - [%02x] cr : 4_%d", cr, 4 + cr);
    else
        __log_error("\t- [%02x] wrong CR setting", cr);

    if(dr == 0 || dr == 1)
        __log_printf("\n\t- [%02x] LowDataRateOptimize %s",
            dr, s_on_off_str[dr]);
    else
        __log_error("\t- [%02x] wrong LowDataRateOptimize setting", cr);
}
__declare_lora_cmd_desc_print(__sx126x_cmd_set_packet_params          )
{
    // -- for LoRa
    __log_printf("LoRa Packet parameters are:");
    // param-1 param-2 ... param-9
    uint16_t preamble_length = *(uint16_t*)buf;
    __log_printf("\n\t - preamble-length : %04X = %d",
        preamble_length, preamble_length);

    uint8_t header_type = buf[2];
    __log_printf("\n\t - header-type     : %02x = %d ( %s length )",
        header_type, header_type, header_type == 0 ? "Variable" : "Fixed");

    uint8_t payload_length = buf[3];
    __log_printf("\n\t - payload-length  : %02x = %d",
        payload_length, payload_length);

    uint8_t crc_type = buf[4];
    __log_printf("\n\t - crc-type        : %02x = %d", crc_type, crc_type);

    uint8_t invert_iq = buf[5];
    __log_printf("\n\t - invert-iq       : %02x = %d", invert_iq, invert_iq);
}
__declare_lora_cmd_desc_print(__sx126x_cmd_set_cad_params             )
{
    check_typical_buf_size(size, 7);
    uint8_t cad_symbol_num = buf[0];
    uint8_t cad_detect_peak = buf[1];
    uint8_t cad_detect_min = buf[2];
    uint8_t cad_exit_mode = buf[3];
    uint16_t cad_timeout = *(uint16_t*)&buf[4];
    __log_printf(" Channel Activity Detection params:");
    __log_printf("\n\t - cad_symbol_num : %02x ( %d symbols)",
        cad_symbol_num, 1u << cad_symbol_num);
    __log_printf("\n\t - cad-detect-level (min: %02x peak: %02x)(%d -- %d)",
        cad_detect_min, cad_detect_peak, cad_detect_min, cad_detect_peak);
    __log_printf("\n\t - CAD exit mode : %02x ( CAD_%s )",
        cad_exit_mode, cad_exit_mode ? "RX" : "ONLY");
    __log_printf("\n\t - rx-timeout : %04x ( %d usec )", cad_timeout,
        (uint32_t)(cad_timeout * 15.625f));    
}
__declare_lora_cmd_desc_print(__sx126x_cmd_set_buffer_base_address    )
{
    check_typical_buf_size(size, 2);
    __log_printf("tx_base: %02x ( %d ) - rx_base: %02x ( %d )",
        buf[0], buf[0], buf[1], buf[1]);
}
__declare_lora_cmd_desc_print(__sx126x_cmd_set_lora_symb_num_timeout  )
{
    check_typical_buf_size(size, 1);
    __log_printf("lora_symbol_num : %02x ( %d )", buf[0], buf[0]);
}
__declare_lora_cmd_desc_print(__sx126x_cmd_get_status                 )
{
    check_typical_buf_size(size, 1);
    uint8_t status = buf[0];
    __log_printf("status byte: %02x", status);
    uint8_t command_stat = __bitwise_bits_read(8, status, 7, 1);
    uint8_t chip_mode = __bitwise_bits_read(8, status, 7, 4);
    static const char* cmd_stats[] = {"reserved", "RFU",
        "data available to host", "command timeout", "cmd processing error",
        "failure to exec cmd", "cmd tx done"};
    static const char* chip_modes[] = {"unused", "RFU",
        "STBY_RC", "STBY_XOSC", "FS", "RX", "TX"};
    __log_printf("\n\t - cmd_stats : %s", cmd_stats[command_stat]);
    __log_printf("\n\t - chip_mode : %s", chip_modes[chip_mode]);
}
__declare_lora_cmd_desc_print(__sx126x_cmd_get_rssi_inst              )
{
    check_typical_buf_size(size, 1);
    __log_printf("instantaneous RSSI = [%02x] %+d", buf[0], (-*(int8_t*)buf)/2);
}
__declare_lora_cmd_desc_print(__sx126x_cmd_get_rx_buffer_status       )
{
    check_typical_buf_size(size, 2);
    __log_printf("\n\t - payload-length-rx : %02x ( %d )", buf[0], buf[0]);
    __log_printf("\n\t - rx-start-buf-ptr  : %02x ( %d )", buf[1], buf[1]);
}
__declare_lora_cmd_desc_print(__sx126x_cmd_get_packet_status          )
{
    check_typical_buf_size(size, 3);
    // -- for lora
    int8_t pwr = (-*(int8_t*)&buf[0])/2;
    int8_t snr  = (*(int8_t*)&buf[1])/4;
    int8_t rssi  = (-*(int8_t*)&buf[2])/2;

    __log_printf("\n\t - [%02x] Actual Signal Power = %+3d dBm", buf[0], pwr);
    __log_printf("\n\t - [%02x] Actual SNR          = %+3d dB",  buf[1], snr);
    __log_printf("\n\t - [%02x] Actual RSSI         = %+3d dB",  buf[1], rssi);
}
__declare_lora_cmd_desc_print(__sx126x_cmd_get_device_errors          )
{
    check_typical_buf_size(size, 2);
    uint16_t errors = *(uint16_t*)buf;
    int pos = 0;
    if(__bitwise_bit_get(8, errors, pos))
        __log_printf("\n\t - RC64k calibration failed");
    ++pos;
    if(__bitwise_bit_get(8, errors, pos))
        __log_printf("\n\t - RC13M calibration failed");
    ++pos;
    if(__bitwise_bit_get(8, errors, pos))
        __log_printf("\n\t - PLL calibration failed");
    ++pos;
    if(__bitwise_bit_get(8, errors, pos))
        __log_printf("\n\t - ADC calibration failed");
    ++pos;
    if(__bitwise_bit_get(8, errors, pos))
        __log_printf("\n\t - MG calibration failed");
    ++pos;
    if(__bitwise_bit_get(8, errors, pos))
        __log_printf("\n\t - XOSC failed to start");
    ++pos;
    if(__bitwise_bit_get(8, errors, pos))
        __log_printf("\n\t - PLL failed to lock");
    if(__bitwise_bit_get(8, errors, 8))
        __log_printf("\n\t - PA ramping failed");
}
__declare_lora_cmd_desc_print(__sx126x_cmd_clear_device_errors        )
{

}
__declare_lora_cmd_desc_print(__sx126x_cmd_get_stats                  )
{
    uint16_t* ptr = (uint16_t*)buf;
    __log_printf("\n\t - # packet received     = [%04x] %d", ptr[0], ptr[0]);
    __log_printf("\n\t - # packet crc error    = [%04x] %d", ptr[1], ptr[1]);
    __log_printf("\n\t - # packet header error = [%04x] %d", ptr[2], ptr[2]);
}
__declare_lora_cmd_desc_print(__sx126x_cmd_reset_stats                )
{

}

/* --- end of file ---------------------------------------------------------- */
