/**
* @author Jakub Bubak
* @date 24.06.2025
*/

#pragma once

#include "main.h"
#include "tim.h"



void ws2812_set_led(uint8_t index, uint8_t r, uint8_t g, uint8_t b);
void ws2812_init(void);
void ws2812_update();
