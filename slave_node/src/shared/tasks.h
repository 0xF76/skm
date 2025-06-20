/**
* @author Jakub Bubak
* @date 30.05.2025
*/

#pragma once

#include "main.h"
#include "cmsis_os.h"
#include "can/can.h"

extern osThreadId_t can_rx_task_handle;
extern osThreadAttr_t can_rx_task_attributes;


void create_tasks(void);
