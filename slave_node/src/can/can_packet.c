/**
* @author Jakub Bubak
* @date 20.06.2025
*/

#include "can_packets.h"
#include "main.h"

const can_packet_def_t can_packet_defs[] = {
	{
		.cmd = CAN_CMD_LED,
		.arg_count = CAN_ARG_LED,
		.handler = can_cmd_led_handler
	},
	{
		.cmd = CAN_CMD_PING,
		.arg_count = CAN_ARG_PING,
		.handler = can_cmd_ping_handler
	}
};
const size_t CAN_PACKET_DEFS_LEN = (sizeof(can_packet_defs)/sizeof(can_packet_def_t));