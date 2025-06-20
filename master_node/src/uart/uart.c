/**
* @author Jakub Bubak
* @date 28.05.2025
*/

#include "uart.h"

#include "can/can.h"
#include <string.h>

uart_parser_t uart_parser;


static void uart_parser_init(uart_parser_t* parser);
static void uart_parser_parse_buffer(uart_parser_t* parser);
static void uart_parser_circular_buffer_put(uart_parser_t *parser, uint16_t len);
static uint8_t uart_parser_circular_buffer_get(uart_parser_t *parser, uint8_t *data);



void uart_rx_task(void* argument) {
  uart_parser_init(&uart_parser);

  HAL_UARTEx_ReceiveToIdle_DMA(&huart2, uart_parser.rx_dma_buffer, sizeof(uart_parser.rx_dma_buffer));
  __HAL_DMA_DISABLE_IT(huart2.hdmarx, DMA_IT_HT);

  for(;;) {
    osThreadFlagsWait(0x01, osFlagsWaitAny, osWaitForever);
    uart_parser_parse_buffer(&uart_parser);
  }
}

void uart_rx_packet_handler_task(void* argument) {
  uart_packet_t packet;
  can_packet_t can_packet;

  for(;;) {
    if(osMessageQueueGet(uart_incoming_packet_queue_handle, &packet, NULL, osWaitForever) == osOK) {
      can_packet.cmd = packet.cmd;
      can_packet.len = packet.len;
      memcpy(can_packet.payload, packet.payload, packet.len);
      HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);

      osMessageQueuePut(can_outgoing_packet_queue_handle, &can_packet, 0, 0);

    }
  }
}

void uart_tx_task(void* argument) {
  uart_packet_t packet;
  uint8_t buffer[UART_PARSER_DMA_BUFFER_SIZE];
  uint16_t buffer_idx;

  for(;;) {
    if(osMessageQueueGet(uart_outgoing_packet_queue_handle, &packet, NULL, osWaitForever) == osOK) {
      buffer_idx = 0;

      buffer[buffer_idx++] = packet.cmd;
      buffer[buffer_idx++] = packet.len;
      for(uint8_t i = 0; i < packet.len; i++) {
        buffer[buffer_idx++] = packet.payload[i];
      }

      uint32_t crc = HAL_CRC_Calculate(&hcrc, (uint32_t*)packet.payload, packet.len);
      crc &= 0xFFFF; // Ensure CRC is 16 bits
      packet.crc = crc;
      buffer[buffer_idx++] = packet.crc & 0xFF; // CRC low byte
      buffer[buffer_idx++] = (packet.crc >> 8) & 0xFF; // CRC high byte

      HAL_UART_Transmit_DMA(&huart2, buffer, buffer_idx);
    }
  }

}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef* huart, uint16_t size) {
  if(huart == &huart2) {
    uart_parser_circular_buffer_put(&uart_parser, size);
    osThreadFlagsSet(uart_rx_task_handle, 0x01);
    HAL_UARTEx_ReceiveToIdle_DMA(&huart2, uart_parser.rx_dma_buffer, sizeof(uart_parser.rx_dma_buffer));
  }
}



static void uart_parser_init(uart_parser_t* parser) {
  parser->state = UART_PARSER_HEADER_CMD;
  parser->rx_head = 0;
  parser->rx_tail = 0;
  parser->payload_index = 0;
}

static void uart_parser_circular_buffer_put(uart_parser_t *parser, uint16_t len) {
  for(uint16_t i = 0; i < len; i++) {
    uint16_t next = (parser->rx_head + 1) % UART_PARSER_RX_BUFFER_SIZE;

    if(next != parser->rx_tail) { // buffer not full
      parser->rx_buffer[parser->rx_head] = parser->rx_dma_buffer[i];
      parser->rx_head = next;
    } else {
      // buffer full
    }
  }

}
static uint8_t uart_parser_circular_buffer_get(uart_parser_t *parser, uint8_t *data) {
  if(parser->rx_head == parser->rx_tail) {
    // buffer empty
    return 0;
  }
  *data = parser->rx_buffer[parser->rx_tail];
  parser->rx_tail = (parser->rx_tail + 1) % UART_PARSER_RX_BUFFER_SIZE;
  return 1;
}

static void uart_parser_parse_buffer(uart_parser_t* parser) {
  uint8_t data;

  while(uart_parser_circular_buffer_get(parser, &data)) {
    switch(parser->state) {
      case UART_PARSER_HEADER_CMD:
        parser->packet.cmd = data;
        parser->state = UART_PARSER_HEADER_LEN;
        break;
      case UART_PARSER_HEADER_LEN:
        parser->packet.len = data;
        if(parser->packet.len > (UART_PARSER_RX_BUFFER_SIZE - 4)) {
          // Handle error: length too long
          parser->state = UART_PARSER_HEADER_CMD; // Reset state
        } else {
          parser->state = UART_PARSER_PAYLOAD;
          parser->payload_index = 0;
        }
        break;
      case UART_PARSER_PAYLOAD:
        parser->packet.payload[parser->payload_index++] = data;
        if(parser->payload_index >= parser->packet.len) {
          parser->state = UART_PARSER_CRC_LOW;
        }
        break;
      case UART_PARSER_CRC_LOW:
        parser->packet.crc = data;
        parser->state = UART_PARSER_CRC_HIGH;
        break;
      case UART_PARSER_CRC_HIGH:
        parser->packet.crc |= (uint16_t)data << 8;

        // calculate CRC for the received packet and check if received packet is correct
        uint32_t crc = HAL_CRC_Calculate(&hcrc, (uint32_t*)parser->packet.payload, parser->packet.len);
        crc &= 0xFFFF; // Ensure CRC is 16 bits

        if(crc == parser->packet.crc) {
          osMessageQueuePut(uart_incoming_packet_queue_handle, &parser->packet, 0, 0);
        }
        parser->state = UART_PARSER_HEADER_CMD;
        break;
    }
  }
}






