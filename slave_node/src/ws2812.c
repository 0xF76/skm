/**
* @author Jakub Bubak
* @date 24.06.2025
*/

#include "ws2812.h"

#define LED_COUNT        7
#define WS2812_RESET_LEN 40
#define WS2812_BIT_0     40  // 0.4 µs
#define WS2812_BIT_1     80  // 0.8 µs
#define WS2812_ARR       124

#define WS2812_BUF_LEN (WS2812_RESET_LEN + 24 * LED_COUNT + 1)


uint8_t ws2812_buf[WS2812_BUF_LEN];


// array to hold gamma correction values
const uint8_t gamma[] = {
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
	1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
	2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
	5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
 10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
 25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
 37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
 51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
 69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
 90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };



static void ws2812_encode_byte(uint8_t byte, uint8_t *buf) {
	for (int i = 0; i < 8; i++) {
		buf[i] = (byte & (1 << (7 - i))) ? WS2812_BIT_1 : WS2812_BIT_0;
	}
}

void ws2812_init(void) {

  for(int i = 0; i < LED_COUNT; i++) {
    ws2812_set_led(i, 0, 0, 0); // Initialize all LEDs to off
  }

	ws2812_buf[WS2812_BUF_LEN - 1] = 0;
	HAL_TIM_Base_Start(&htim3);
	ws2812_update();
}

void ws2812_set_led(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
	if (index >= LED_COUNT) return;

	uint8_t *led_buf = ws2812_buf + WS2812_RESET_LEN + index * 24;

	ws2812_encode_byte(gamma[g], led_buf);
	ws2812_encode_byte(gamma[r], led_buf + 8);
	ws2812_encode_byte(gamma[b], led_buf + 16);
}

void ws2812_update(void) {
	HAL_TIM_PWM_Start_DMA(&htim3, TIM_CHANNEL_1, (uint32_t*)ws2812_buf, sizeof(ws2812_buf));
}




