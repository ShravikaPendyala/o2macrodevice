
/**
  ******************************************************************************
  * File Name          :  app_x-cube-nfcx.c
  * Description        : This file provides code for the configuration
  *                      of the STMicroelectronics.X-CUBE-NFC6.3.0.0 instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "rfal_platform.h"
#include "nfc_conf.h"
#include "app_x-cube-nfcx.h"

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_nucleo.h"
#include <stdio.h>
#include <string.h>

/** @defgroup App NFC6
  * @{
  */

/** @defgroup Main
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/

uint8_t globalCommProtectCnt = 0;   /*!< Global Protection counter     */
/* Private functions ---------------------------------------------------------*/
void MX_NFC8_PollingTagDetectNDEF_Init(void);
void MX_NFC8_PollingTagDetectNDEF_Process(void);

HAL_StatusTypeDef UARTConsolePrint( char *puartmsg );
HAL_StatusTypeDef UARTConsoleScan( uint8_t uartchar );

void MX_X_CUBE_NFC6_Init(void)
{
  /* USER CODE BEGIN SV */

  /* USER CODE END SV */

  /* USER CODE BEGIN NFC8_Library_Init_PreTreatment */

  /* USER CODE END NFC8_Library_Init_PreTreatment */

  /* Initialize the peripherals and the NFC8 components */
  MX_NFC8_PollingTagDetectNDEF_Init();
   /* USER CODE BEGIN SV */

  /* USER CODE END SV */

  /* USER CODE BEGIN NFC8_Library_Init_PostTreatment */

  /* USER CODE END NFC8_Library_Init_PostTreatment */

}
/*
 * LM background task
 */
void MX_X_CUBE_NFC6_Process(void)
{

    /* USER CODE BEGIN NFC8_Library_Process */

    /* USER CODE END NFC8_Library_Process */
    /* Run Demo Application */
    demoCycle();

}

/**
  * @brief  Initialize the I2CProtection  feature Example
  * @retval None
  */
void MX_NFC8_PollingTagDetectNDEF_Init(void)
{
   BSP_NFC0XCOMM_Init();
   BSP_COM_Init(COM1);
   USR_INT_LINE.Line = USR_INT_LINE_NUM;
   USR_INT_LINE.PendingCallback = st25r3916Isr;
   BSP_PB_Init(BUTTON_USER, BUTTON_MODE_GPIO);

   /* Configure interrupt callback */
  (void)HAL_EXTI_GetHandle(&USR_INT_LINE, USR_INT_LINE.Line);
  (void)HAL_EXTI_RegisterCallback(&USR_INT_LINE, HAL_EXTI_COMMON_CB_ID, BSP_NFC0XCOMM_IRQ_Callback);

#ifdef ST25R3916
  platformLog("Welcome to X-NUCLEO-NFC06A1\r\n");
#else
  platformLog("Welcome to X-NUCLEO-NFC08A1\r\n");
#endif /* ST25R3916 */
  /* Initalize RFAL */
  if( !demoIni() )
  {
    /*
    * in case the rfal initalization failed signal it by flashing all LED
    * and stoping all operations
    */
    platformLog("Initialization failed..\r\n");
    while(1)
    {
      platformLedToogle(PLATFORM_LED_A_PORT, PLATFORM_LED_A_PIN);
      platformLedToogle(PLATFORM_LED_B_PORT, PLATFORM_LED_B_PIN);
      platformLedToogle(PLATFORM_LED_F_PORT, PLATFORM_LED_F_PIN);
      platformLedToogle(PLATFORM_LED_V_PORT, PLATFORM_LED_V_PIN);
      platformLedToogle(PLATFORM_LED_AP2P_PORT, PLATFORM_LED_AP2P_PIN);
      platformLedToogle(PLATFORM_LED_FIELD_PORT, PLATFORM_LED_FIELD_PIN);

      platformDelay(100);
    }
  }
  else
  {
    platformLog("Initialization succeeded..\r\n");
    for (int i = 0; i < 6; i++)
    {
      platformLedToogle(PLATFORM_LED_A_PORT, PLATFORM_LED_A_PIN);
      platformLedToogle(PLATFORM_LED_B_PORT, PLATFORM_LED_B_PIN);
      platformLedToogle(PLATFORM_LED_F_PORT, PLATFORM_LED_F_PIN);
      platformLedToogle(PLATFORM_LED_V_PORT, PLATFORM_LED_V_PIN);
      platformLedToogle(PLATFORM_LED_AP2P_PORT, PLATFORM_LED_AP2P_PIN);
      platformLedToogle(PLATFORM_LED_FIELD_PORT, PLATFORM_LED_FIELD_PIN);

      platformDelay(200);
    }

    platformLedOff(PLATFORM_LED_A_PORT, PLATFORM_LED_A_PIN);
    platformLedOff(PLATFORM_LED_B_PORT, PLATFORM_LED_B_PIN);
    platformLedOff(PLATFORM_LED_F_PORT, PLATFORM_LED_F_PIN);
    platformLedOff(PLATFORM_LED_V_PORT, PLATFORM_LED_V_PIN);
    platformLedOff(PLATFORM_LED_AP2P_PORT, PLATFORM_LED_AP2P_PIN);
    platformLedOff(PLATFORM_LED_FIELD_PORT, PLATFORM_LED_FIELD_PIN);

  }

}

/**
 * @brief   This function sends data on the uart
 * @param   puartmsg:
 * @retval  HAL_StatusTypeDef
 */
HAL_StatusTypeDef UARTConsolePrint( char *puartmsg )
{
  return HAL_UART_Transmit( &hcom_uart[COM1], (uint8_t *)puartmsg, strlen( puartmsg ), 500);
}

/**
 * @brief   This function wait a data on the uart
 * @param   uartchar received character
 * @retval  HAL_StatusTypeDef
 */
HAL_StatusTypeDef UARTConsoleScan( uint8_t uartchar )
{
  while( HAL_UART_Receive( &hcom_uart[COM1], &uartchar, 1, 500) == HAL_TIMEOUT );

  return HAL_OK;
}

/**
  * @brief      SPI Read and Write byte(s) to device
  * @param[in]  pTxData : Pointer to data buffer to write
  * @param[out] pRxData : Pointer to data buffer for read data
  * @param[in]  Length : number of bytes to write
  * @return     BSP status
  */
int32_t BSP_NFC0XCOMM_SendRecv(const uint8_t * const pTxData, uint8_t * const pRxData, uint16_t Length)
{
  HAL_StatusTypeDef status = HAL_ERROR;
  int32_t ret = BSP_ERROR_NONE;

  if((pTxData != NULL) && (pRxData != NULL))
  {
    status = HAL_SPI_TransmitReceive(&COMM_HANDLE, (uint8_t *)pTxData, (uint8_t *)pRxData, Length, 2000);
  }
  else if ((pTxData != NULL) && (pRxData == NULL))
  {
    status = HAL_SPI_Transmit(&COMM_HANDLE, (uint8_t *)pTxData, Length, 2000);
  }
  else if ((pTxData == NULL) && (pRxData != NULL))
  {
    status = HAL_SPI_Receive(&COMM_HANDLE, (uint8_t *)pRxData, Length, 2000);
  }
  else
  {
  	ret = BSP_ERROR_WRONG_PARAM;
  }

  /* Check the communication status */
  if (status != HAL_OK)
  {
    /* Execute user timeout callback */
    ret = BSP_NFC0XCOMM_Init();
  }

  return ret;
}

/**
  * @brief  BSP SPI1 callback
  * @param  None
  * @return None
  */
__weak void BSP_NFC0XCOMM_IRQ_Callback(void)
{
  /* Prevent unused argument(s) compilation warning */

  /* This function should be implemented by the user application.
   * It is called into this driver when an event from ST25R3916 is triggered.
   */
  st25r3916Isr();
}

#ifdef __cplusplus
}
#endif

