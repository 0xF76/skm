/**
* @author Jakub Bubak
* @date 20.06.2025
*/

#include "can_cmd_handlers.h"
#include "shared/queues.h"
#include <string.h>
#include "ws2812.h"


void can_cmd_led_handler(uint8_t* args) {
	if(args[0] == 0) {
		if(args[1] == 0) {
				HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET); // Turn off LED
		} else {
			HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET); // Turn on LED
		}
		return;
	}

	ws2812_set_led(args[0], args[1], args[2], args[3]); // Set LED color
	ws2812_update();
}

void can_cmd_ping_handler(uint8_t* args) {
	//create a response packet pong
	can_packet_t response_packet;
	response_packet.cmd = CAN_CMD_PONG;
	response_packet.len = CAN_ARG_PONG;

	strcpy(response_packet.payload, "PONG");
	osMessageQueuePut(can_outgoing_packet_queue_handle, &response_packet, 0, 0); // Send the response packet
}