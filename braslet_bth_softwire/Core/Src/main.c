/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"
#include "i2c.h"
#include "rtc.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "../ssd1306/ssd1306.h"
#include "../ssd1306/ssd1306_tests.h"
#include "mpu6050.h"
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

RTC_TimeTypeDef sTime = {0};
RTC_DateTypeDef DateToUpdate = {0};
char trans_str[64] = {0,};

MPU6050_t MPU6050;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
char buff[100]={0};

void ssd1306_printtime()
{
		HAL_RTC_GetTime(&hrtc,&sTime,RTC_FORMAT_BIN);HAL_RTC_GetDate(&hrtc,&DateToUpdate,RTC_FORMAT_BIN);
		ssd1306_Fill(Black);
		
		sprintf(buff,"%02d:%02d:%02d",sTime.Hours,sTime.Minutes,sTime.Seconds);
    ssd1306_SetCursor(0, 37);ssd1306_WriteString(buff, Font_16x26, White);
		
		sprintf(buff,"%02d.%02d.20%02d",DateToUpdate.Date,DateToUpdate.Month,DateToUpdate.Year);
    ssd1306_SetCursor(0, 10);ssd1306_WriteString(buff, Font_11x18, White);
		
		ssd1306_UpdateScreen();
}
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
  MX_I2C1_Init();
  MX_RTC_Init();
  MX_USART1_UART_Init();
  MX_SPI1_Init();
  MX_FATFS_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  
	SD_PowerOn();
	sd_ini();
	
	ssd1306_Init();
	sTime.Hours=23;
	sTime.Minutes=47;
	sTime.Seconds=35;
	HAL_RTC_SetTime(&hrtc, &sTime,RTC_FORMAT_BIN);
	
	DateToUpdate.Date=2;
	DateToUpdate.Month=11;
	DateToUpdate.Year=21;
	//HAL_RTC_SetDate(&hrtc, &DateToUpdate,RTC_FORMAT_BIN);
	while (MPU6050_Init(&hi2c1) == 1);
	USART1->CR1 = USART_CR1_UE;
	NVIC_EnableIRQ(USART1_IRQn); USART1->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE;        //разрешение прерывания по приему данных UART1
	uint8_t i=0;
	//sprintf(buff,"dimaib %d\r\n",i);
	uint32_t utimeS = 1636700059;
	uint32_t utimeMS = 0;
	
	while (1)
  {
		ssd1306_printtime();									//вывод времени на дисплей
		MPU6050_Read_All(&hi2c1, &MPU6050);
		
		sprintf(buff,"%d.%04d, %f, %f, %f\r\n",utimeS,utimeMS, (float)MPU6050.Accel_X_RAW/10000,(float)MPU6050.Accel_Y_RAW/10000,(float)MPU6050.Accel_Z_RAW/10000);
		//HAL_UART_Transmit(&huart1,buff,strlen(buff),100);
		//utimeMS+=20;
		//if(utimeMS==1000) {utimeS++; utimeMS=0; GPIOA->ODR^=(1<<4);}
		/*
		sprintf(buff,"%d %d",MPU6050.Accel_X_RAW,MPU6050.Gyro_X_RAW);
    ssd1306_SetCursor(0, 1);ssd1306_WriteString(buff, Font_11x18, White);
		sprintf(buff,"%d %d",MPU6050.Accel_Y_RAW,MPU6050.Gyro_Y_RAW);
		ssd1306_SetCursor(0, 19);ssd1306_WriteString(buff, Font_11x18, White);
		sprintf(buff,"%d %d",MPU6050.Accel_Z_RAW,MPU6050.Gyro_Z_RAW);
		ssd1306_SetCursor(0, 37);ssd1306_WriteString(buff, Font_11x18, White);
		ssd1306_UpdateScreen();
		*/
		GPIOA->ODR^=(1<<4);
		HAL_Delay(11);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_I2C1
                              |RCC_PERIPHCLK_RTC;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
