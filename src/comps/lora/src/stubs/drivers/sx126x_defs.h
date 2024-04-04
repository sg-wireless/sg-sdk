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
 * @brief   Semtech registers definitions needed by the inspector analyser
 * --------------------------------------------------------------------------- *
 */
#ifndef __SX126X_DEFS_H__
#define __SX126X_DEFS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* --- selecting the operating mode of the radio ---------------------------- */

#define __sx126x_cmd_set_sleep                     0x84
#define __sx126x_cmd_set_standby                   0x80
#define __sx126x_cmd_set_fs                        0xc1
#define __sx126x_cmd_set_tx                        0x83
#define __sx126x_cmd_set_rx                        0x82
#define __sx126x_cmd_stop_timer_on_preample        0x9f
#define __sx126x_cmd_set_rx_duty_cycle             0x94
#define __sx126x_cmd_set_cad                       0xc5
#define __sx126x_cmd_set_tx_continuous_wave        0xd1
#define __sx126x_cmd_set_tx_infinite_preamble      0xd2
#define __sx126x_cmd_set_regulator_mode            0x96
#define __sx126x_cmd_calibrate                     0x89
#define __sx126x_cmd_calibrate_image               0x98
#define __sx126x_cmd_set_pa_config                 0x95
#define __sx126x_cmd_rx_tx_fallback_mode           0x93

/* --- register and buffer access commands ---------------------------------- */

#define __sx126x_cmd_write_register                0x0d
#define __sx126x_cmd_read_register                 0x1d
#define __sx126x_cmd_write_buffer                  0x0e
#define __sx126x_cmd_read_buffer                   0x1e

/* --- dio and irq commands ------------------------------------------------- */

#define __sx126x_cmd_set_dio_irq_params            0x08
#define __sx126x_cmd_get_irq_status                0x12
#define __sx126x_cmd_clear_irq_status              0x02
#define __sx126x_cmd_set_dio_2_as_rf_switch_ctrl   0x9d
#define __sx126x_cmd_set_dio_3_as_tcxo_ctrl        0x97

/* --- rf and packets settings ---------------------------------------------- */
#define __sx126x_cmd_set_rf_frequency              0x86
#define __sx126x_cmd_set_packet_type               0x8a
#define __sx126x_cmd_get_packet_type               0x11
#define __sx126x_cmd_set_tx_params                 0x8e
#define __sx126x_cmd_set_modulation_params         0x8b
#define __sx126x_cmd_set_packet_params             0x8c
#define __sx126x_cmd_set_cad_params                0x88
#define __sx126x_cmd_set_buffer_base_address       0x8f
#define __sx126x_cmd_set_lora_symb_num_timeout     0xa0

/* --- status commands ------------------------------------------------------ */

#define __sx126x_cmd_get_status                    0xc0
#define __sx126x_cmd_get_rssi_inst                 0x15
#define __sx126x_cmd_get_rx_buffer_status          0x13
#define __sx126x_cmd_get_packet_status             0x14
#define __sx126x_cmd_get_device_errors             0x17
#define __sx126x_cmd_clear_device_errors           0x07
#define __sx126x_cmd_get_stats                     0x10
#define __sx126x_cmd_reset_stats                   0x00

/** -------------------------------------------------------------------------- *
 * lora chip register map table
 * --------------------------------------------------------------------------- *
 */
#define __sx126x_reg_hopping_enable                0x0385
#define __sx126x_reg_packet_length                 0x0386
#define __sx126x_reg_nb_hopping_blocks             0x0387

#define __sx126x_reg_nb_symbols_r0(idx)            (0x0388 + (idx * 6) + 0)
#define __sx126x_reg_nb_symbols_r1(idx)            (0x0388 + (idx * 6) + 1)
#define __sx126x_reg_freq_r0(idx)                  (0x0388 + (idx * 6) + 2)
#define __sx126x_reg_freq_r1(idx)                  (0x0388 + (idx * 6) + 3)
#define __sx126x_reg_freq_r2(idx)                  (0x0388 + (idx * 6) + 4)
#define __sx126x_reg_freq_r3(idx)                  (0x0388 + (idx * 6) + 5)

#define __sx126x_reg_diox_output_enable            0x0580
#define __sx126x_reg_diox_input_enable             0x0583
#define __sx126x_reg_diox_pull_up_control          0x0584
#define __sx126x_reg_diox_pull_down_control        0x0585

#define __sx126x_reg_whitening_initial_value_msb   0x06b8
#define __sx126x_reg_whitening_initial_value_lsb   0x06b9

#define __sx126x_reg_crc_msb_initial_value         0x06bc
#define __sx126x_reg_crc_lsb_initial_value         0x06bd

#define __sx126x_reg_crc_msb_polynomial_value_0    0x06be
#define __sx126x_reg_crc_lsb_polynomial_value_1    0x06bf

#define __sx126x_reg_sync_word_0                   0x06c0
#define __sx126x_reg_sync_word_1                   0x06c1
#define __sx126x_reg_sync_word_2                   0x06c2
#define __sx126x_reg_sync_word_3                   0x06c3
#define __sx126x_reg_sync_word_4                   0x06c4
#define __sx126x_reg_sync_word_5                   0x06c5
#define __sx126x_reg_sync_word_6                   0x06c6
#define __sx126x_reg_sync_word_7                   0x06c7

#define __sx126x_reg_node_address                  0x06cd
#define __sx126x_reg_broadcast_address             0x06ce
#define __sx126x_reg_iq_polarity_setup             0x0736
#define __sx126x_reg_lora_sync_word_msb            0x0740
#define __sx126x_reg_lora_sync_word_lsb            0x0741

#define __sx126x_reg_random_num_gen_0              0x0819
#define __sx126x_reg_random_num_gen_1              0x081a
#define __sx126x_reg_random_num_gen_2              0x081b
#define __sx126x_reg_random_num_gen_3              0x081c

#define __sx126x_reg_tx_modulation                 0x0889
#define __sx126x_reg_rx_gain                       0x08ac
#define __sx126x_reg_tx_clamp_config               0x08d8

#define __sx126x_reg_ocp_configuration             0x08e7
#define __sx126x_reg_rtc_control                   0x0902
#define __sx126x_reg_xta_trim                      0x0911
#define __sx126x_reg_xtb_trim                      0x0912
#define __sx126x_reg_dio3_output_volatage_control  0x0920
#define __sx126x_reg_event_mask                    0x0944

/** -------------------------------------------------------------------------- *
 * lora chip register map table
 * --------------------------------------------------------------------------- *
 */
#define __sx126x_irq_all                          (0x43ffu)
#define __sx126x_irq_all_lora                     (0x03ffu)
#define __sx126x_irq_tx_done                      (1u<<0)
#define __sx126x_irq_rx_done                      (1u<<1)
#define __sx126x_irq_preamble_detected            (1u<<2)
#define __sx126x_irq_sync_word_valid              (1u<<3)
#define __sx126x_irq_header_valid                 (1u<<4)
#define __sx126x_irq_header_err                   (1u<<5)
#define __sx126x_irq_crc_err                      (1u<<6)
#define __sx126x_irq_cad_done                     (1u<<7)
#define __sx126x_irq_cad_detected                 (1u<<8)
#define __sx126x_irq_timeout                      (1u<<9)
#define __sx126x_irq_lr_fhss_hop                  (1u<<14)

/* --- end of file ---------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __SX126X_DEFS_H__ */
