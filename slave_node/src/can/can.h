/**
* @author Jakub Bubak
* @date 30.05.2025
*/

#pragma once

#include "main.h"
#include "fdcan.h"
#include "cmsis_os.h"
#include "shared/queues.h"
#include "shared/tasks.h"
#include "can_packets.h"



typedef struct {
	uint8_t cmd;
	uint8_t len;
	uint8_t payload[16];
} can_packet_t;

void can_tx_task(void* argument);
void can_rx_task(void* argument);
