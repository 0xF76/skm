/**
* @author Jakub Bubak
* @date 28.05.2025
*/

#pragma once

#include "main.h"
#include "cmsis_os.h"
#include "usart.h"
#include "crc.h"
#include "shared/tasks.h"
#include "shared/queues.h"

#define UART_PARSER_DMA_BUFFER_SIZE 32
#define UART_PARSER_RX_BUFFER_SIZE 256

typedef enum {
  UART_PARSER_HEADER_CMD,
  UART_PARSER_HEADER_LEN,
  UART_PARSER_PAYLOAD,
  UART_PARSER_CRC_LOW,
  UART_PARSER_CRC_HIGH,
} uart_parser_state_t;

typedef struct {
  uint8_t cmd;
  uint8_t len;
  uint8_t payload[16];
  uint16_t crc;
} uart_packet_t;

typedef struct {
  uart_parser_state_t state;
  uint8_t rx_dma_buffer[UART_PARSER_DMA_BUFFER_SIZE];
  uint8_t rx_buffer[UART_PARSER_RX_BUFFER_SIZE];
  uint16_t rx_head;
  uint16_t rx_tail;
  uint8_t payload_index;
  uart_packet_t packet;
} uart_parser_t;

extern uart_parser_t uart_parser;

void uart_rx_task(void* argument);
void uart_rx_packet_handler_task(void* argument);

void uart_tx_task(void* argument);
