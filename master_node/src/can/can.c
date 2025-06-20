/**
* @author Jakub Bubak
* @date 28.05.2025
*/

#include "can.h"



void can_tx_task(void* argument) {
	can_packet_t packet;
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
			HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
		}
	}

}
void can_rx_task(void* argument) {
	//TODO: Implement CAN RX task
}