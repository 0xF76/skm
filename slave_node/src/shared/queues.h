/**
* @author Jakub Bubak
* @date 30.05.2025
*/

#pragma once

#include "main.h"
#include "cmsis_os.h"
#include "can/can.h"

extern osMessageQueueId_t can_incoming_packet_queue_handle;
extern osMessageQueueId_t can_outgoing_packet_queue_handle;

void create_queues(void);