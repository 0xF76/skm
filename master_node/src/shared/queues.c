/**
* @author Jakub Bubak
* @date 28.05.2025
*/

#include "queues.h"


osMessageQueueId_t uart_incoming_packet_queue_handle;
osMessageQueueId_t uart_outgoing_packet_queue_handle;

osMessageQueueId_t can_incoming_packet_queue_handle;
osMessageQueueId_t can_outgoing_packet_queue_handle;

void create_queues(void) {
		uart_incoming_packet_queue_handle = osMessageQueueNew(10, sizeof(uart_packet_t), NULL);
		uart_outgoing_packet_queue_handle = osMessageQueueNew(10, sizeof(uart_packet_t), NULL);

		can_incoming_packet_queue_handle = osMessageQueueNew(10, sizeof(can_packet_t), NULL);
		can_outgoing_packet_queue_handle = osMessageQueueNew(10, sizeof(can_packet_t), NULL);
}