/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : app_freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "fdcan.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for CANPublisherTas */
osThreadId_t CANPublisherTasHandle;
const osThreadAttr_t CANPublisherTas_attributes = {
  .name = "CANPublisherTas",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void CAN_Publisher(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of CANPublisherTas */
  CANPublisherTasHandle = osThreadNew(CAN_Publisher, NULL, &CANPublisherTas_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_CAN_Publisher */
/**
  * @brief  Function implementing the CANPublisherTas thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_CAN_Publisher */
void CAN_Publisher(void *argument)
{
  /* USER CODE BEGIN CAN_Publisher */
  FDCAN_TxHeaderTypeDef can_header;
  can_header.Identifier = 0x123;
  can_header.IdType = FDCAN_STANDARD_ID;
  can_header.TxFrameType = FDCAN_DATA_FRAME;
  can_header.DataLength = 2;
  can_header.ErrorStateIndicator = FDCAN_ESI_PASSIVE;
  can_header.BitRateSwitch = FDCAN_BRS_OFF;
  can_header.FDFormat = FDCAN_FD_CAN;
  can_header.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
  can_header.MessageMarker = 0;
  uint8_t data[2] = {0x21, 0x37};

  HAL_FDCAN_Start(&hfdcan1);
  /* Infinite loop */
  for(;;)
  {
    while (!HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan1)) {
      HAL_Delay(1);
    }

    HAL_StatusTypeDef res = HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &can_header, data);

    if (res != HAL_OK) {
      HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
    }
    osDelay(100);
  }
  /* USER CODE END CAN_Publisher */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

