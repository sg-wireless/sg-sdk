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
 * @brief   This file declare the lora-stack API interface.
 * --------------------------------------------------------------------------- *
 */
#ifndef __LORA_H__
#define __LORA_H__

#ifdef __cplusplus
extern "C" {
#endif
/** -------------------------------------------------------------------------- *
 * includes
 * --------------------------------------------------------------------------- *
 */
#include <stdbool.h>
#include <stdint.h>

/** -------------------------------------------------------------------------- *
 * typedefs
 * --------------------------------------------------------------------------- *
 */

/**
 * @enum    lora_error_t
 * @brief   the return status for any lora method
 */
typedef enum {
    __LORA_OK,                  /**< API performed correctly */
    __LORA_ERROR,               /**< API operation failed */
    __LORA_TIMEOUT,             /**< API operation timed-out */
    __LORA_TX_NOT_CONFIRMED,    /**< TX confirmation not received */
    __LORA_POWERED_OFF
} lora_error_t;

/**
 * @enum    lora_mode_t
 * @brief   the available lora stack supported modes which are: lora-wan and
 *          lora-raw
 */
typedef enum {
    __LORA_MODE_WAN,            /**< lora module is working in LORAWAN mode */
    __LORA_MODE_RAW             /**< lora module is working in RAW mode */
} lora_mode_t;

/**
 * @enum    lora_ioctl_t
 * @brief   defined the available control signals that can be communicated with
 *          the lora stack.
 *          some control signal are generic to be used with any mode such as:
 *           \a __LORA_IOCTL_SET_CALLBACK, \a __LORA_IOCTL_SET_PARAM,
 *           \a __LORA_IOCTL_GET_PARAM, \a __LORA_IOCTL_VERIFY_PARAM
 *          and some are mode specific.
 *          LoRa-WAN specific control signals are:
 *           \a __LORA_IOCTL_SET_COMMISSION,
 *           \a __LORA_IOCTL_JOIN, \a __LORA_IOCTL_JOIN_STATUS,
 *           \a __LORA_IOCTL_DUTY_CYCLE_SET, \a __LORA_IOCTL_DUTY_CYCLE_GET,
 *           \a __LORA_IOCTL_DUTY_CYCLE_START, \a __LORA_IOCTL_DUTY_CYCLE_STOP,
 *           \a __LORA_IOCTL_PORT_OPEN, \a __LORA_IOCTL_PORT_CLOSE,
 *           \a __LORA_IOCTL_PORT_GET_IND_PARAM, \a __LORA_IOCTL_IS_PENDING_TX,
 *           \a __LORA_IOCTL_ENABLE_RX_LISTENING,
 *           \a __LORA_IOCTL_DISABLE_RX_LISTENING
 *          LoRa-RAW specific control signals are:
 *           \a __LORA_IOCTL_TX_CONT_WAVE_START,
 *           \a __LORA_IOCTL_TX_CONT_WAVE_STOP,
 *           \a __LORA_IOCTL_RX_CONT_START, \a __LORA_IOCTL_RX_CONT_STOP,
 *           \a __LORA_IOCTL_RECONFIG_RADIO,
 *           \a __LORA_IOCTL_RESET_RADIO_PARAMS
 */
typedef enum {
    /* applicable for all lora modes */

    __LORA_IOCTL_SET_CALLBACK,      /**< to set lora callback parameters */
    __LORA_IOCTL_SET_PARAM,         /**< to set special mode parameter */
    __LORA_IOCTL_GET_PARAM,         /**< to get special mode parameter */
    __LORA_IOCTL_VERIFY_PARAM,      /**< to verify validity of a  special mode
                                         parameter */

    /* applicable for lora raw mode only */

    __LORA_IOCTL_GET_DEFAULT_REGION_PARAM,/**< to get special mode parameter */
    __LORA_IOCTL_TX_CONT_WAVE_START,/**< to order start of tx continuous wave */
    __LORA_IOCTL_TX_CONT_WAVE_STOP, /**< to order exit of tx continuous wave */
    __LORA_IOCTL_RX_CONT_START,     /**< to order start of rx continuous mode */
    __LORA_IOCTL_RX_CONT_STOP,      /**< to order exit of rx continuous mode */
    __LORA_IOCTL_RECONFIG_RADIO,    /**< to apply the configured radio params */
    __LORA_IOCTL_RESET_RADIO_PARAMS,/**< to reset radio params to defaults */

    /* applicable for lora wan mode only */

    __LORA_IOCTL_SET_COMMISSION,    /**< to set new commissioning params */
    __LORA_IOCTL_JOIN,              /**< to order join request */
    __LORA_IOCTL_JOIN_STATUS,       /**< to get the current join status */
    __LORA_IOCTL_DUTY_CYCLE_SET,    /**< to set the duty-cycle timer value */
    __LORA_IOCTL_DUTY_CYCLE_GET,    /**< to get the duty-cycle timer value */
    __LORA_IOCTL_DUTY_CYCLE_START,  /**< to start the duty-cycle timer opr */
    __LORA_IOCTL_DUTY_CYCLE_STOP,   /**< to stop the duty-cycle timer opr */
    __LORA_IOCTL_PORT_OPEN,         /**< to open lora port for tx/rx opr */
    __LORA_IOCTL_PORT_CLOSE,        /**< to close lora port */
    __LORA_IOCTL_PORT_GET_IND_PARAM,/**< to request an indication params
                when an indication event __LORA_EVENT_INDICATION comes at the
                user level, the user shall use this ioctl to fetch the pending
                indication parameters for this indication event.
                Also when the event \enum __LORA_EVENT_INDICATION comes, the
                user should fetch all pending indications until the indication
                parameter event is __LORA_EVENT_NONE which means no more
                pending indications */
    __LORA_IOCTL_IS_PENDING_TX,     /**< to check if there is pending tx req */
    __LORA_IOCTL_ENABLE_RX_LISTENING,/**< to enable listening to the network for
                downlink frames by sending empty message to trigger class-A
                cycle to be able to fetch queued message at the network side */
    __LORA_IOCTL_DISABLE_RX_LISTENING /**< to disable listening to the network
                and the network queued messages will not be received until a
                real tx message is placed. by default the lora stack is not
                in listening mode */
} lora_ioctl_t;

/**
 * @enum    lora_event_t
 * @brief   lora stack generated events
 */
typedef enum {
    __LORA_EVENT_TX_DONE,       /**< tx done successfully */
    __LORA_EVENT_TX_TIMEOUT,    /**< tx done successfully */
    __LORA_EVENT_TX_FAIL,       /**< tx operation failed */
    __LORA_EVENT_TX_CONFIRM,    /**< tx confirmation received */
    __LORA_EVENT_RX_DONE,       /**< rx done successfully */
    __LORA_EVENT_RX_TIMEOUT,    /**< rx timeout */
    __LORA_EVENT_RX_FAIL,       /**< rx timeout */

    __LORA_EVENT_INDICATION,    /**< special for lora-wan indications */
    __LORA_EVENT_NONE,  /**< indicate the end of pending indication events */
} lora_event_t;

/**
 * the payload of the RX event
 */
typedef struct {
    uint8_t* buf;   /**< pointer to received data */
    uint8_t  len;   /**< length of the received data */
    int8_t   rssi;  /**< received signal strength indicator */
    int8_t   snr;   /**< signal to noise ratio */
} lora_raw_rx_event_data_t;

/**
 * The registered event callback prototype
 */
typedef void lora_event_callback_t(lora_event_t event, void* event_data);

/**
 * LoRaWAN supported regions enumeration
 */
typedef enum {
    __LORA_REGION_AS923,
    __LORA_REGION_AU915,
    __LORA_REGION_CN470,
    __LORA_REGION_CN779,
    __LORA_REGION_EU433,
    __LORA_REGION_EU868,
    __LORA_REGION_KR920,
    __LORA_REGION_IN865,
    __LORA_REGION_US915,
    __LORA_REGION_RU864
} lora_region_t;

/** -------------------------------------------------------------------------- *
 * LoRa WAN Specific APIs
 * --------------------------------------------------------------------------- *
 */
/**
 * LoRaWAN commissioning types
 */
typedef enum {
    __LORA_COMMISSION_OTAA, /**< commissioning by over the air activation */
    __LORA_COMMISSION_ABP   /**< commissioning activation by personalization */
} commission_type_t;

/**
 * LoRaWAN supported standard versions
 */
typedef enum {
    __LORA_WAN_VERSION_1_0_X,
    __LORA_WAN_VERSION_1_1_X,
} lora_wan_version_t;

/**
 * LoRaWAN commissioning parameters
 */
typedef struct {
    commission_type_t type;         /**< commissioning type OTAA or ABP */
    lora_wan_version_t  version;    /**< version of the used LoRa Standard */
    union {
        struct {                // 1.0.x    1.1.x
            uint8_t* dev_eui;   // dev-eui  dev-eui
            uint8_t* join_eui;  // app-eui  join-eui
            uint8_t* app_key;   // app-key  app-key
            uint8_t* nwk_key;   // --       nwk-key
        } otaa; /**< OTAA required specific parameters */
        struct {
            uint8_t* dev_eui;
            uint32_t dev_addr;
            uint8_t* app_s_key;
            uint8_t* nwk_s_key;
        } abp;  /**< ABP required specific parameters */
    };
} lora_commission_params_t;

/**
 * LoRaWAN class types
 */
typedef enum {
    __LORA_WAN_CLASS_A,
    __LORA_WAN_CLASS_B,
    __LORA_WAN_CLASS_C
} lora_wan_class_t;

/**
 * LoRaWAN parameter type to be used while configuring the LoRaWAN mode
 */
typedef enum {
    __LORA_WAN_PARAM_REGION,    /**< to configure region parameter */
    __LORA_WAN_PARAM_CLASS,     /**< to configure class type parameter */
    __LORA_WAN_PARAM_PAYLOAD    /**< to get current available payload size */
} lora_wan_param_type_t;

/**
 * LoRaWAN parameter configuration. This struct used to specify the
 * configuration parameter of the LoRaWAN mode
 */
typedef struct {
    lora_wan_param_type_t  type;    /**< identifies which config param */
    union {
        lora_region_t       region; /**< value of the new region */
        lora_wan_class_t    class;  /**< value of the new required class type */
        uint8_t             payload; /**< value of the payload size */
    } param;
} lora_wan_param_t;

/** -------------------------------------------------------------------------- *
 * LoRa RAW Specific APIs
 * --------------------------------------------------------------------------- *
 */

/**
 * LoRa supported band widthes
 */
typedef enum {
    __LORA_BW_125_KHZ = 0,
    __LORA_BW_250_KHZ = 1,
    __LORA_BW_500_KHZ = 2
} lora_bw_t;

/**
 * LoRa supported spreading factors
 */
typedef enum {
    __LORA_SF_6  = 6,
    __LORA_SF_7  = 7,
    __LORA_SF_8  = 8,
    __LORA_SF_9  = 9,
    __LORA_SF_10 = 10,
    __LORA_SF_11 = 11,
    __LORA_SF_12 = 12
} lora_sf_t;

/**
 * LoRa supported coding rates
 */
typedef enum {
    __LORA_CR_4_5 = 1,
    __LORA_CR_4_6 = 2,
    __LORA_CR_4_7 = 3,
    __LORA_CR_4_8 = 4
} lora_cr_t;

/**
 * LoRaRAW available parameters
 */
typedef enum {
    __LORA_RAW_PARAM_REGION,        /**< to identify the operating region
        of the device, this will make the required precautions for the used
        frequencies and tx power to not violate the region regulations */
    __LORA_RAW_PARAM_FREQ,          /**< to specify the operating frequency */
    __LORA_RAW_PARAM_TX_POWER,      /**< to specify the tx power */
    __LORA_RAW_PARAM_ANTENNA_GAIN,  /**< to specify the current antenna-gain.
        it will be used while calculating the effective tx power from the
        tranceiver chip */
    __LORA_RAW_PARAM_SF,            /**< to specify the spreading factor */
    __LORA_RAW_PARAM_CR,            /**< to specify the coding rate */
    __LORA_RAW_PARAM_PREAMBLE,      /**< to specify the preamble */
    __LORA_RAW_PARAM_BW,            /**< to specify the band-width */
    __LORA_RAW_PARAM_PAYLOAD,       /**< to specify the maximum payload size */
    __LORA_RAW_PARAM_TX_INV_IQ,     /**< to enable/disable the TX inv IQ */
    __LORA_RAW_PARAM_RX_INV_IQ,     /**< to enable/disable the RX inv IQ */
    __LORA_RAW_PARAM_CRC_ON,        /**< to enable pyload crc or not */
    __LORA_RAW_PARAM_SYMB_TIMEOUT,  /**< to specify the LoRa symbols timeout */
    __LORA_RAW_PARAM_TX_TIMEOUT,    /**< the time-out of sending a message;
        it should be sufficient enough according to the time on air required
        for the current modulation parameters */
    __LORA_RAW_PARAM_RX_TIMEOUT,    /**< the rx window time in non continuous
                                         reception mode */
} lora_raw_param_type_t;

/**
 * LoRaRAW parameter descriptor
 */
typedef struct {
    lora_raw_param_type_t type; /**< type of the parameter */
    lora_region_t   region;     /**< to configure the default region radio
        parameters. When it is used non of the below union values are used.
        in case of __LORA_IOCTL_VERIFY_PARAM, this 'region' field shall be
        used with one of the below union values */

    union {
        uint32_t    freq;       /**< frequency in Hz */
        int8_t      tx_power;   /**< tx power in dBm */
        float     antenna_gain; /**< antenna gain in dBi */
        uint8_t     sf;         /**< spreading factor */
        uint8_t     cr;         /**< coding rate */
        uint8_t     preamble;   /**< preamble length */
        uint8_t     bw;         /**< bandwidth */
        uint8_t     payload;    /**< max payload length */
        bool        tx_inv_iq;  /**< invertied TX IQ */
        bool        rx_inv_iq;  /**< invertied RX IQ */
        bool        crc_on;     /**< CRC enable */
        uint8_t     symb_timeout; /*< symbols timeout */
        uint32_t    tx_timeout; /**< tx default window time */
        uint32_t    rx_timeout; /**< rx default window time */
    } param;
} lora_raw_param_t;

/**
 * LoRaRAW tx continuous wave parameters
 */
typedef struct {
    uint32_t    freq;   /**< the frequency in Hz */
    int8_t      power;  /**< the tx power in dBm */
    uint32_t    time;   /**< the transmission time window for continuous wave */
} lora_raw_tx_cont_wave_params_t;

/**
 * tx message parameters description
 */
typedef struct {
    uint8_t*        buf;        /**< tx data buffer */
    uint8_t         len;        /**< tx data length */
    uint8_t         port;       /**< lora-wan port (LoRaWAN mode only) */
    bool            confirm;    /**< confirmed message (LoRaWAN mode only) */
    bool            sync;       /**< block until tx-done, tx_fail, tx-confirm */
    uint32_t        timeout;    /**< discard msg after this timeout */
    uint32_t        retries;    /**< number of retries (LoRaWAN mode only) */
    uint32_t        msg_app_id; /**< special message id (LoRaWAN mode only) */
} lora_tx_params_t;

/**
 * rx message parameters description
 */
typedef struct {
    uint8_t*    buf;            /**< buffer at which receive the message */
    uint8_t*    p_len;          /**< this is an in/out parameter.
        it is set with the maximum buf length before calling lora_rx()
        and it is set with the received message length after return.
        in case of sync type messages, buf and p_length are not utilized
        and the used shall copy the received message in the callback */
    uint8_t     port;           /**< the port of the received message
        (LoRaWAN mode only) */
    uint32_t    timeout;        /**< deadline time for the rx operation */
    bool        sync;           /**< to wait or return immediately */
} lora_rx_params_t;

/**
 * indication events descriptors that comes from LoRaWAN mode
 * when an indication event is received, the user shall fetch the pending
 * indication parameters described by this structure
 */
typedef struct {
    lora_event_t    event;      /**< the indication event type */
    uint8_t*        buf;        /**< the indication parameter requestor shall
        set this buff, so that the LoRa-Stack shall copy the received message
        at that buffer, also the below len shall be set with the maximum buffer
        length before fetching the parameters, so that the LoRa-stack can do
        the required length checking before copying */
    uint8_t         len;        /**< length of the received message */
    uint8_t         port_num;   /**< port of the received message */
    union {
        struct {
            uint32_t msg_app_id; /**< the corresponding message id */
            uint32_t msg_seq_num;/**< the corresponding port sequence num */
            uint32_t ul_frame_counter; /**< LoRaWAN UL frame counter */
            int8_t   tx_power;  /**< the tx power */
            int8_t   data_rate; /**< the tx data-rate of this message */
        } tx;
        struct {
            uint32_t dl_frame_counter; /**< LoRaWAN UL frame counter */
            int8_t   rssi;      /**< received signal strength indicator */
            int8_t   snr;       /**< signal to noise ratio */
            int8_t   data_rate; /**< the rx data-rate of this message */
        } rx;
    };
} lora_wan_ind_params_t;

/**
 * LoRa callback description
 */
typedef struct {
    uint8_t port;   /**< (LoRaWAN mode only), to associate this callback
        with this specified port. To associate this callback with all ports
        the port value shall be set to '__port_any' */
    #define __port_any  0xff
    void (* callback)(lora_event_t event, void* event_data);
    /**< the callback prototype */
} lora_callback_t;

/** -------------------------------------------------------------------------- *
 * LoRa Generic APIs
 * --------------------------------------------------------------------------- *
 */

/**
 * @brief   this is the lora-stack constructor, it performs the required
 *          stack initialization
 * 
 * @return  lora-stack error type
 */
lora_error_t lora_ctor(void);

/**
 * @brief   this is the lora-stack destructor, it performs the required
 *          stack de-initialization
 * 
 * @return  lora-stack error type
 */
lora_error_t lora_dtor(void);

/**
 * @brief   this is the lora-stack callbacks stub. it connects internally
 *          a generic callback handler, so that the lora-stack callback events
 *          can be seen without setting a real user callback
 * 
 * @return  lora-stack error type
 */
lora_error_t lora_connect_callback_stub(void);

/**
 * @brief   it switches the current operating lora mode seamlessly
 * 
 * @param   mode the mode to switch to
 * @return  lora-stack error type
 */
lora_error_t lora_change_mode(lora_mode_t mode);

/**
 * @brief   to retrieve the current operating lora mode
 * 
 * @param   p_mode a pointer to which retrieve the current operating lora mode
 * @return  lora-stack error type
 */
lora_error_t lora_get_mode(lora_mode_t * p_mode);

/**
 * @brief   to perform a lora transmission
 * 
 * @param   p_tx_params a tx message parameters description
 * @return  lora-stack error type
 */
lora_error_t lora_tx(lora_tx_params_t * p_tx_params);

/**
 * @brief   to perform a lora reception
 * 
 * @param   p_rx_params an rx message parameters description
 * @return  lora-stack error type
 */
lora_error_t lora_rx(lora_rx_params_t * p_rx_params);

/**
 * @brief   to display the current lora mode stats
 * 
 * @return  lora-stack error type
 */
lora_error_t lora_stats(void);

/**
 * @brief   to perform a certain control signal with the current operating
 *          lora mode
 * 
 * @return  lora-stack error type
 */
lora_error_t lora_ioctl(uint32_t ioctl, void* arg);

/**
 * @brief   to translate the event to a string for debugging purposes
 * 
 * @param   event the event to be translated to a string
 * @return  a pointer to the event string
 */
const char* lora_get_event_str(lora_event_t event);

/**
 * @brief   to translate the region to a string for debugging purposes
 * 
 * @param   region the region to be translated to a string
 * @return  a pointer to the region string
 */
const char* lora_get_region_str(lora_region_t region);

/* --- end of file ---------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __LORA_H__ */
