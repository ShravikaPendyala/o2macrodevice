/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "app_x-cube-nfcx.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */
void disable_clocks(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_X_CUBE_NFC6_Init();
  /* USER CODE BEGIN 2 */

//  HAL_SYSCFG_DisableVREFBUF();
//  HAL_PWREx_DisablePVM1();
//  HAL_PWREx_DisablePVM2();
//  HAL_PWREx_DisablePVM3();
//  HAL_PWREx_DisablePVM4();

  // disable timer
  HAL_SuspendTick();
//  HAL_GPIO_TogglePin(PLATFORM_LED_A_PORT, PLATFORM_LED_A_PIN);

  disable_clocks();

  //disable peripheral voltage
//  HAL_PWREx_EnableLowPowerRunMode();
//  HAL_PWREx_EnterSTOP0Mode();
//  HAL_PWREx_EnterSTOP0Mode(PWR_STOPENTRY_WFI);
  HAL_PWREx_EnableLowPowerRunMode();
//  MY_HAL_PWREx_EnterLPM();
//  HAL_PWR_EnterSTANDBYMode();


  //outside while loop test
  MX_X_CUBE_NFC6_Process();

//  if (HAL_RCC_GetSysClockFreq() > 2000000U){
//	  HAL_GPIO_TogglePin(PLATFORM_LED_A_PORT, PLATFORM_LED_A_PIN);
//  }

  HAL_PWREx_DisableSRAM2ContentRetention();
//  HAL_PWREx_EnableBORPVD_ULP();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

//  MX_X_CUBE_NFC6_Process();
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}


void disable_clocks(void) {
	__HAL_RCC_TIM2_CLK_DISABLE();
	__HAL_RCC_TIM3_CLK_DISABLE();
	__HAL_RCC_TIM4_CLK_DISABLE();
	__HAL_RCC_TIM5_CLK_DISABLE();
	__HAL_RCC_TIM6_CLK_DISABLE();
	__HAL_RCC_TIM7_CLK_DISABLE();
	__HAL_RCC_LCD_CLK_DISABLE();
	__HAL_RCC_SPI2_CLK_DISABLE();
	__HAL_RCC_SPI3_CLK_DISABLE();
	__HAL_RCC_USART3_CLK_DISABLE();
	__HAL_RCC_UART4_CLK_DISABLE();
	__HAL_RCC_UART5_CLK_DISABLE();
	__HAL_RCC_I2C1_CLK_DISABLE();
	__HAL_RCC_I2C2_CLK_DISABLE();
	__HAL_RCC_I2C3_CLK_DISABLE();
	__HAL_RCC_PWR_CLK_DISABLE();
	__HAL_RCC_OPAMP_CLK_DISABLE();
	__HAL_RCC_LPTIM1_CLK_DISABLE();
	__HAL_RCC_LPUART1_CLK_DISABLE();
	__HAL_RCC_SWPMI1_CLK_DISABLE();
	__HAL_RCC_LPTIM2_CLK_DISABLE();
	__HAL_RCC_TIM1_CLK_DISABLE();
//	__HAL_RCC_SPI1_CLK_DISABLE();
	__HAL_RCC_TIM8_CLK_DISABLE();
	__HAL_RCC_USART1_CLK_DISABLE();
	__HAL_RCC_TIM15_CLK_DISABLE();
	__HAL_RCC_TIM16_CLK_DISABLE();
	__HAL_RCC_TIM17_CLK_DISABLE();
	__HAL_RCC_SAI1_CLK_DISABLE();
	__HAL_RCC_SAI2_CLK_DISABLE();
	__HAL_RCC_DFSDM1_CLK_DISABLE();


	__HAL_RCC_HSI_DISABLE();
	__HAL_RCC_LSI_DISABLE();
	__HAL_RCC_PLL_DISABLE();

}


/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
//  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
//  {
//    Error_Handler();
//  }

//  HAL_PWR_EnableBkUpAccess();

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  //  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
    RCC_OscInitStruct.MSIState = RCC_MSI_ON;
//    RCC_OscInitStruct.LSEState = RCC_LSE_OFF;
//    RCC_OscInitStruct.MSIState = RCC_MSI_OFF;
//    RCC_OscInitStruct.HSEState = RCC_HSE_OFF;
//    RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
    RCC_OscInitStruct.MSICalibrationValue = 0;
    RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_3; //4 = 1MHz
//    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK;

  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
//  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
//  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
//  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
//  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
//  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0|GPIO_PIN_1, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_8, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, SPI1_NSS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : PC0 PC1 */
//  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
//  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//  GPIO_InitStruct.Pull = GPIO_NOPULL;
//  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PA1 PA4 PA8 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : SPI1_NSS_Pin */
  GPIO_InitStruct.Pin = SPI1_NSS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
//  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
//  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
