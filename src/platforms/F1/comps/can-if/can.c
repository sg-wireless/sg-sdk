#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/twai.h"


#include "can.h"

#define __log_subsystem F1
#define __log_component can
#include "log_lib.h"

static  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_250KBITS();
twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
static  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(TX_GPIO_NUM, RX_GPIO_NUM, TWAI_MODE_NORMAL);



twai_message_t send_message = {
    // Message type and format settings
    .extd = 0,              // 0-standard frame; 1- Extended frame
    .rtr = 0,               // 0- data frame,1-Remote Frame
    .ss = 1,                // 0-Error resend; 1-Single send
    .self = 0,              // 0- Do not receive messages sent by oneself, 1- Receive messages sent by oneself
    .dlc_non_comp = 0,      // 0-Data length not exceeding 8 (ISO 11898-1); 1- Data length greater than 8 (non-standard)
    // Message ID and payload
    .identifier = ID_MASTER_START_CMD,//11/29 bit ID
    .data_length_code = 3,  //DLC data length
    .data = {0x12,0x24,0x36},//Sending data, invalid for remote frames
};


static QueueHandle_t tx_task_queue;
static QueueHandle_t rx_task_queue;

T_buffer t_RecvBuf;

void CircularInit(T_buffer *p)
{
  p->head=0;
  p->tail=0;
}

uint8_t Append(T_buffer *p,twai_message_t dat)
{
  p->buffer[p->head]=dat;
  p->head = (p->head +1) % CIRCLE_BUF_LEN;
  if(p->head == p->tail)//full
  {
    p->tail = (p->tail +1) % CIRCLE_BUF_LEN;//Abandon a piece of data
    return 1;
  }
  return 0;
}

circularDatType *pop(T_buffer *p)//
{
  circularDatType *buf=&(p->buffer[p->tail]);
  p->tail = (p->tail +1) % CIRCLE_BUF_LEN;
  return buf;
}
uint8_t IsCircularAny(T_buffer *p)
{
  return (CIRCLE_BUF_LEN + p->head - p->tail)%CIRCLE_BUF_LEN;
}

/* --------------------------- Tasks and Functions -------------------------- */
static void twai_receive_task(void *arg)
{
    twai_message_t rx_msg;

    while (1) 
    {       
        if(ESP_OK == twai_receive(&rx_msg, portMAX_DELAY))
        {
          Append(&t_RecvBuf,rx_msg);
        }
    }
    vTaskDelete(NULL);
}

static void twai_transmit_task(void *arg)
{
    tx_task_action_t action;
    while (1) {
        
        xQueueReceive(tx_task_queue, &action, portMAX_DELAY);

        if (action == TX_SEND_START_CMD) {
            //Transmit start command to slave
            twai_transmit(&send_message, portMAX_DELAY);
            ESP_LOGI(EXAMPLE_TAG, "Transmitted start command");
        }
    }
    vTaskDelete(NULL);
}


void can_init(uint8_t RxPin,uint8_t TxPin,uint32_t Baud,uint8_t Mode)
{
    //Create tasks, queues, and semaphores
    rx_task_queue = xQueueCreate(1, sizeof(rx_task_action_t));
    tx_task_queue = xQueueCreate(1, sizeof(tx_task_action_t));
    xTaskCreatePinnedToCore(twai_receive_task, "TWAI_rx", 4096, NULL, RX_TASK_PRIO, NULL, tskNO_AFFINITY);
    xTaskCreatePinnedToCore(twai_transmit_task, "TWAI_tx", 4096, NULL, TX_TASK_PRIO, NULL, tskNO_AFFINITY);

    g_config.rx_io = RxPin;
    g_config.tx_io = TxPin;
    g_config.mode = Mode;

    switch (Baud)
    {
      case 25000:
        {
          twai_timing_config_t t_config1 = TWAI_TIMING_CONFIG_25KBITS();
          t_config = t_config1;
        }
        break;
      case 50000:
        {
          twai_timing_config_t t_config1 = TWAI_TIMING_CONFIG_50KBITS() ;
          t_config = t_config1;
        }
        break;
      case 100000:
        {
          twai_timing_config_t t_config1 = TWAI_TIMING_CONFIG_100KBITS();
          t_config = t_config1;
        }
        break;
      case 125000:
        {
          twai_timing_config_t t_config1 = TWAI_TIMING_CONFIG_125KBITS();
          t_config = t_config1;
        }
        break;
      case 250000:
        {
          twai_timing_config_t t_config1 = TWAI_TIMING_CONFIG_250KBITS();
          t_config = t_config1;
        }
        break;
      case 500000:
        {
          twai_timing_config_t t_config1 = TWAI_TIMING_CONFIG_500KBITS();
          t_config = t_config1;
        }
        break;
      case 800000:
        {
          twai_timing_config_t t_config1 = TWAI_TIMING_CONFIG_800KBITS();
          t_config = t_config1;
        }
        break;
      case 1000000:
        {
          twai_timing_config_t t_config1 = TWAI_TIMING_CONFIG_1MBITS()  ;
          t_config = t_config1;
        }
        break;
    }

    //Install TWAI driver
    ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config));

    ESP_LOGI(EXAMPLE_TAG, "Driver started");
    ESP_ERROR_CHECK(twai_start());
    //xSemaphoreGive(ctrl_task_sem);              //Start control task


}

void can_deinit(void)
{
    ESP_ERROR_CHECK(twai_stop());
    //Uninstall TWAI driver
    ESP_ERROR_CHECK(twai_driver_uninstall());
    ESP_LOGI(EXAMPLE_TAG, "Driver uninstalled");
    
    //Cleanup
    vQueueDelete(rx_task_queue);
    vQueueDelete(tx_task_queue);
}

void can_filter()
{
  ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config));
}
void can_send()
{
    tx_task_action_t tx_action;
    tx_action = TX_SEND_START_CMD;
    xQueueSend(tx_task_queue, &tx_action, portMAX_DELAY);
}

uint8_t IsAnyDate(void)
{
  return IsCircularAny(&t_RecvBuf);
}

circularDatType *can_read(void)
{
  return pop(&t_RecvBuf);
}
twai_message_t DebugDat;

