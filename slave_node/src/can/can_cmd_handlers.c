/**
* @author Jakub Bubak
* @date 20.06.2025
*/

#include "can_cmd_handlers.h"
#include "shared/queues.h"
#include <string.h>


void can_cmd_led_handler(uint8_t* args) {
		if (args[0] == 0x01) {
				HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET); // Turn on LED
		} else if (args[0] == 0x00) {
				HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET); // Turn off LED
		}
}

void can_cmd_ping_handler(uint8_t* args) {
	//create a response packet pong
	can_packet_t response_packet;
	response_packet.cmd = CAN_CMD_PONG;
	response_packet.len = CAN_ARG_PONG;

	strcpy(response_packet.payload, "PONG");
	osMessageQueuePut(can_outgoing_packet_queue_handle, &response_packet, 0, 0); // Send the response packet
}