/**
* @author Jakub Bubak
* @date 30.05.2025
*/

#include "can.h"



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

	//TODO: Implement CAN TX task

}


void can_rx_task(void* argument) {
	can_packet_t packet;

	HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE | FDCAN_IT_RX_FIFO0_FULL | FDCAN_IT_RX_FIFO0_MESSAGE_LOST, 0);
	HAL_FDCAN_Start(&hfdcan1);

	for(;;) {

		if(osMessageQueueGet(can_incoming_packet_queue_handle, &packet, NULL, osWaitForever) == osOK) {

			for(size_t i = 0; i < CAN_PACKET_DEFS_LEN; i++) {
				if(can_packet_defs[i].cmd == packet.cmd) {
					if(can_packet_defs[i].arg_count == packet.len) {
						can_packet_defs[i].handler(packet.payload);
					}
				}
			}
		}

	}

}

