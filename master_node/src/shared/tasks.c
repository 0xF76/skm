/**
* @author Jakub Bubak
* @date 28.05.2025
*/

#include "tasks.h"

osThreadId_t uart_rx_task_handle;
osThreadAttr_t uart_rx_task_attributes = {
	.name = "UartRxTask",
	.priority = (osPriority_t) osPriorityNormal,
	.stack_size = 128 * 4
};

osThreadId_t uart_rx_packet_handler_task_handle;
osThreadAttr_t uart_rx_packet_handler_task_attributes = {
	.name = "UartRxPacketHandlerTask",
	.priority = (osPriority_t) osPriorityNormal,
	.stack_size = 128 * 4
};

osThreadId_t uart_tx_task_handle;
osThreadAttr_t uart_tx_task_attributes = {
	.name = "UartTxTask",
	.priority = (osPriority_t) osPriorityNormal,
	.stack_size = 128 * 4
};

osThreadId_t can_tx_task_handle;
osThreadAttr_t can_tx_task_attributes = {
	.name = "CanTxTask",
	.priority = (osPriority_t) osPriorityNormal,
	.stack_size = 128 * 4
};


void create_tasks(void) {
	uart_rx_task_handle = osThreadNew(uart_rx_task, NULL, &uart_rx_task_attributes);
	uart_rx_packet_handler_task_handle = osThreadNew(uart_rx_packet_handler_task, NULL, &uart_rx_packet_handler_task_attributes);

	uart_tx_task_handle = osThreadNew(uart_tx_task, NULL, &uart_tx_task_attributes);

	can_tx_task_handle = osThreadNew(can_tx_task, NULL, &can_tx_task_attributes);
}