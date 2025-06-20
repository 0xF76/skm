/**
* @author Jakub Bubak
* @date 20.06.2025
*/

#pragma once


#include "can_cmd_handlers.h"
#include "main.h"

typedef enum {
		CAN_CMD_LED = 0x05,
		CAN_CMD_PING = 0x06,
		CAN_CMD_PONG = 0x07,
} can_cmd_t;

typedef enum {
  		CAN_ARG_LED = 0x01,
			CAN_ARG_PING = 0x04,
			CAN_ARG_PONG = 0x04,
} can_arg_t;

typedef struct {
  can_cmd_t cmd;
  can_arg_t arg_count;
  void (*handler)(uint8_t* args);
} can_packet_def_t;

extern const can_packet_def_t can_packet_defs[];
extern const size_t CAN_PACKET_DEFS_LEN;

