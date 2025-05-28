/**
* @author Jakub Bubak
* @date 28.05.2025
*/

#pragma once

#include "main.h"
#include "cmsis_os.h"
#include "uart/uart.h"

extern osThreadId_t uart_rx_task_handle;
extern osThreadAttr_t uart_rx_task_attributes;

extern osThreadId_t uart_rx_packet_handler_task_handle;
extern osThreadAttr_t uart_rx_packet_handler_task_attributes;

extern osThreadId_t uart_tx_task_handle;
extern osThreadAttr_t uart_tx_task_attributes;

extern osThreadId_t can_tx_task_handle;
extern osThreadAttr_t can_tx_task_attributes;





void create_tasks(void);

