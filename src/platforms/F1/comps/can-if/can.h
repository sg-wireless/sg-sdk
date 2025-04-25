#ifndef __CAN_H__
#define __CAN_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "hal/twai_types.h"

#define PING_PERIOD_MS          250
#define NO_OF_DATA_MSGS         10
#define NO_OF_ITERS             3
#define ITER_DELAY_MS           1000
#define RX_TASK_PRIO            8
#define TX_TASK_PRIO            9
#define CTRL_TSK_PRIO           10
#define TX_GPIO_NUM             1//40//CONFIG_EXAMPLE_TX_GPIO_NUM
#define RX_GPIO_NUM             2//41//CONFIG_EXAMPLE_RX_GPIO_NUM
#define EXAMPLE_TAG             "TWAI Master"

#define DEFAULT_BAUD            250000
#define DEFAULT_MODE            0

#define ID_MASTER_STOP_CMD      0x0A0
#define ID_MASTER_START_CMD     0x0A1
#define ID_MASTER_PING          0x0A2
#define ID_SLAVE_STOP_RESP      0x0B0
#define ID_SLAVE_DATA           0x0B1
#define ID_SLAVE_PING_RESP      0x0B2

typedef enum {
    TX_SEND_PINGS,
    TX_SEND_START_CMD,
    TX_SEND_STOP_CMD,
    TX_TASK_EXIT,
} tx_task_action_t;

typedef enum {
    RX_RECEIVE_PING_RESP,
    RX_RECEIVE_DATA,
    RX_RECEIVE_STOP_RESP,
    RX_TASK_EXIT,
} rx_task_action_t;

#define CIRCLE_BUF_LEN  NO_OF_DATA_MSGS
typedef twai_message_t circularDatType;
typedef struct{
uint8_t head;
uint8_t tail;
circularDatType buffer[CIRCLE_BUF_LEN];
}T_buffer;

void can_init(uint8_t RxPin,uint8_t TxPin,uint32_t Baud,uint8_t Mode);

void can_deinit(void);

void can_send();
uint8_t IsAnyDate(void);
circularDatType *can_read(void);

extern twai_message_t send_message;
extern twai_filter_config_t f_config;
/* --- end of file ---------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __RGBLED_H__ */