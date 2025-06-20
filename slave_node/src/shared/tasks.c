/**
* @author Jakub Bubak
* @date 30.05.2025
*/

#include "tasks.h"

osThreadId_t can_rx_task_handle;
osThreadAttr_t can_rx_task_attributes = {
	.name = "CanRxTask",
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
	can_rx_task_handle = osThreadNew(can_rx_task, NULL, &can_rx_task_attributes);
	can_tx_task_handle = osThreadNew(can_tx_task, NULL, &can_tx_task_attributes);
}