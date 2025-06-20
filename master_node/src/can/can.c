/**
* @author Jakub Bubak
* @date 28.05.2025
*/

#include "can.h"

#include <string.h>

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs) {
	FDCAN_RxHeaderTypeDef header;
	can_packet_t packet;

	HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &header, packet.payload);

	packet.cmd = header.Identifier;
	packet.len = header.DataLength;

	osMessageQueuePut(can_incoming_packet_queue_handle, &packet, 0, 0);
}

void can_tx_task(void* argument) {
	can_packet_t packet;
	HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE | FDCAN_IT_RX_FIFO0_FULL | FDCAN_IT_RX_FIFO0_MESSAGE_LOST, 0);
	HAL_FDCAN_Start(&hfdcan1);

	uint8_t payload[1] = {1}; // Example payload

	for(;;) {
		// FDCAN_TxHeaderTypeDef header;
		// header.Identifier = 0x05;
		// header.IdType = FDCAN_STANDARD_ID;
		// header.TxFrameType = FDCAN_DATA_FRAME;
		// header.DataLength = 1;
		// header.ErrorStateIndicator = FDCAN_ESI_PASSIVE;
		// header.BitRateSwitch = FDCAN_BRS_OFF;
		// header.FDFormat = FDCAN_CLASSIC_CAN;
		// header.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
		// header.MessageMarker = 0;
		//
		//
		// while(!HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan1)) {}
		//
		// HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &header, payload);
		// HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin); // Toggle LED to indicate transmission
		//
		// osDelay(100);


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
void can_rx_task(void* argument) {
	can_packet_t packet;
	uart_packet_t uart_packet;

	for(;;) {
		if(osMessageQueueGet(can_incoming_packet_queue_handle, &packet, NULL, osWaitForever) == osOK) {
			uart_packet.cmd = packet.cmd;
			uart_packet.len = packet.len;
			memcpy(uart_packet.payload, packet.payload, packet.len);
			osMessageQueuePut(uart_outgoing_packet_queue_handle, &uart_packet, 0, 0); // Forward to UART queue
		}
	}

}