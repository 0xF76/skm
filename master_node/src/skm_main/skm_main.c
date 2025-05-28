/**
* @author Jakub Bubak
* @date 27.05.2025
*/

#include "skm_main.h"
#include <string.h>
#include "crc.h"


osThreadId_t uart_parser_task_handle;
osThreadAttr_t uart_parser_task_attributes = {
  .name = "UartParserTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};

osThreadId_t incoming_packet_handler_task_handle;
osThreadAttr_t incoming_packet_handler_task_attributes = {
  .name = "IncomingPacketHandlerTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 1024 * 4
};

osThreadId_t uart_encoder_task_handle;
osThreadAttr_t uart_encoder_task_attributes = {
  .name = "UartEncoderTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};

osThreadId_t can_tx_task_handle;
osThreadAttr_t can_tx_task_attributes = {
  .name = "CanTxTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};

osMessageQueueId_t uart_incoming_packet_queue_handle;
osMessageQueueId_t uart_outgoing_packet_queue_handle;

osMessageQueueId_t can_incoming_packet_queue_handle;
osMessageQueueId_t can_outgoing_packet_queue_handle;


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
  uint8_t payload[32];
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


uart_parser_t uart_parser;

typedef struct {
  uint8_t cmd;
  uint8_t len;
  uint8_t payload[32];
} can_packet_t;


void uart_parser_task(void* argument);
void incoming_packet_handler_task(void* argument);

void uart_encoder_task(void* argument);

void uart_parser_buffer_put(uart_parser_t *parser, uint8_t *data, uint16_t len);
uint8_t uart_parser_buffer_get(uart_parser_t *parser, uint8_t *data);
void uart_parser_parse_buffer(uart_parser_t* parser);

void can_tx_task(void* argument) {
  can_packet_t packet;

  HAL_FDCAN_Start(&hfdcan1);

  for(;;) {
    if(osMessageQueueGet(can_outgoing_packet_queue_handle, &packet, NULL, osWaitForever) == osOK) {
      FDCAN_TxHeaderTypeDef header;
      header.Identifier = packet.cmd;
      header.IdType = FDCAN_STANDARD_ID;
      header.TxFrameType = FDCAN_DATA_FRAME;
      header.DataLength = packet.len;
      header.ErrorStateIndicator = FDCAN_ESI_PASSIVE;
      header.BitRateSwitch = FDCAN_BRS_OFF;
      header.FDFormat = FDCAN_CLASSIC_CAN;
      header.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
      header.MessageMarker = 0;

      while(!HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan1)) {}

      HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &header, packet.payload);
    }

  }
}



void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef* huart, uint16_t size) {
  if(huart == &huart2) {

    uart_parser_buffer_put(&uart_parser, uart_parser.rx_dma_buffer, size);
    osThreadFlagsSet(uart_parser_task_handle, 0x01);
    HAL_UARTEx_ReceiveToIdle_DMA(&huart2, uart_parser.rx_dma_buffer, sizeof(uart_parser.rx_dma_buffer));

  }
}




void init(void) {
  osKernelInitialize();

  /* Tasks creation */
  uart_parser_task_handle = osThreadNew(uart_parser_task, NULL, &uart_parser_task_attributes);
  incoming_packet_handler_task_handle = osThreadNew(incoming_packet_handler_task, NULL, &incoming_packet_handler_task_attributes);

  uart_encoder_task_handle = osThreadNew(uart_encoder_task, NULL, &uart_encoder_task_attributes);

  can_tx_task_handle = osThreadNew(can_tx_task, NULL, &can_tx_task_attributes);


  /* Queues creation */
  uart_incoming_packet_queue_handle = osMessageQueueNew(10, sizeof(uart_packet_t), NULL);
  uart_outgoing_packet_queue_handle = osMessageQueueNew(10, sizeof(uart_packet_t), NULL);
  can_outgoing_packet_queue_handle = osMessageQueueNew(10, sizeof(can_packet_t), NULL);


  osKernelStart();
}

void error_handler(void) {
}



void uart_parser_task(void* argument) {
  HAL_UARTEx_ReceiveToIdle_DMA(&huart2, uart_parser.rx_dma_buffer, sizeof(uart_parser.rx_dma_buffer));
  __HAL_DMA_DISABLE_IT(huart2.hdmarx, DMA_IT_HT);

  for(;;) {
    osThreadFlagsWait(0x01, osFlagsWaitAny, osWaitForever);

    uart_parser_parse_buffer(&uart_parser);

  }
}
void incoming_packet_handler_task(void* argument) {
  uart_packet_t packet;
  can_packet_t can_packet;


  for(;;) {
    if(osMessageQueueGet(uart_incoming_packet_queue_handle, &packet, NULL, osWaitForever) == osOK) {
      can_packet.cmd = packet.cmd;
      can_packet.len = packet.len;
      memcpy(can_packet.payload, packet.payload, packet.len);

      osMessageQueuePut(can_outgoing_packet_queue_handle, &can_packet, 0, 0);
    }
  }
}


void uart_encoder_task(void* argument) {
  uart_packet_t packet;
  uint8_t buffer[packet.len + 4]; // cmd + len + payload + crc
  uint16_t buffer_idx;

  for(;;) {
    if(osMessageQueueGet(uart_outgoing_packet_queue_handle, &packet, NULL, osWaitForever) == osOK) {
      buffer_idx = 0;

      buffer[buffer_idx++] = packet.cmd;
      buffer[buffer_idx++] = packet.len;
      for(uint8_t i = 0; i < packet.len; i++) {
        buffer[buffer_idx++] = packet.payload[i];
      }
      uint16_t crc = 0;
      for(uint8_t i = 0; i < packet.len; i++) {
        crc ^= packet.payload[i];
      }
      packet.crc = crc;
      buffer[buffer_idx++] = packet.crc & 0xFF; // CRC low byte
      buffer[buffer_idx++] = (packet.crc >> 8) & 0xFF; // CRC high byte

      HAL_UART_Transmit_DMA(&huart2, buffer, packet.len+4);
    }

  }
}


void uart_parser_buffer_put(uart_parser_t *parser, uint8_t *data, uint16_t len) {
  for(uint16_t i = 0; i < len; i++) {

    uint16_t next = (parser->rx_head + 1) % UART_PARSER_RX_BUFFER_SIZE;

    if(next != parser->rx_tail) {  // Not full
      parser->rx_buffer[parser->rx_head] = data[i];
      parser->rx_head = next;
    } else {
      // Buffer full
    }
  }
}

uint8_t uart_parser_buffer_get(uart_parser_t *parser, uint8_t *data) {
  if (parser->rx_head == parser->rx_tail) {
    // Buffer empty
    return 0;
  }
  *data = parser->rx_buffer[parser->rx_tail];
  parser->rx_tail = (parser->rx_tail + 1) % UART_PARSER_RX_BUFFER_SIZE;
  return 1;
}

void uart_parser_parse_buffer(uart_parser_t* parser) {
  uint8_t data;
  while(uart_parser_buffer_get(parser, &data)) {
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

      // Calculate CRC for the received packet
      uint32_t crc = HAL_CRC_Calculate(&hcrc, (uint32_t*)parser->packet.payload, parser->packet.len);
      crc &= 0xFFFF; // Ensure CRC is 16 bits

      if(crc == parser->packet.crc) {
        osMessageQueuePut(uart_incoming_packet_queue_handle, &(parser->packet), 0, 0);
      }
      parser->state = UART_PARSER_HEADER_CMD;
      break;
    }
  }
}

