/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f1xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f1xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "mpu6050.h"
#include "../ssd1306/ssd1306.h"
#include "functions.h"
#include "rtc.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */
uint8_t buf_uart[200]={0};
uint8_t incr_incr=0;
/* USER CODE END TD */

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
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern TIM_HandleTypeDef htim2;
extern UART_HandleTypeDef huart1;
extern TIM_HandleTypeDef htim1;

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M3 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
  while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Prefetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/******************************************************************************/
/* STM32F1xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f1xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles TIM1 update interrupt.
  */
void TIM1_UP_IRQHandler(void)
{
  /* USER CODE BEGIN TIM1_UP_IRQn 0 */

  /* USER CODE END TIM1_UP_IRQn 0 */
  HAL_TIM_IRQHandler(&htim1);
  /* USER CODE BEGIN TIM1_UP_IRQn 1 */

  /* USER CODE END TIM1_UP_IRQn 1 */
}

/**
  * @brief This function handles TIM2 global interrupt.
  */
void TIM2_IRQHandler(void)
{
  /* USER CODE BEGIN TIM2_IRQn 0 */

  /* USER CODE END TIM2_IRQn 0 */
  HAL_TIM_IRQHandler(&htim2);
  /* USER CODE BEGIN TIM2_IRQn 1 */

  /* USER CODE END TIM2_IRQn 1 */
}

/**
  * @brief This function handles USART1 global interrupt.
  */
void USART1_IRQHandler(void)
{
  /* USER CODE BEGIN USART1_IRQn 0 */

  /* USER CODE END USART1_IRQn 0 */
  HAL_UART_IRQHandler(&huart1);
  /* USER CODE BEGIN USART1_IRQn 1 */
	//
	//uint8_t rx_byte=USART1->DR;
	//dd[rr]=dat2;
	//rr++;
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
	if(USART1->SR & USART_SR_RXNE){
		int8_t dat=USART1->DR;																											//считать принятый символ
		buf_uart[incr_incr]=dat;
		//ловим команду time=dd.mo.yy hh.mm.ss\r\n
		uint8_t shift=24;
		if(incr_incr>=24){
		if(buf_uart[incr_incr-24]=='t'&&buf_uart[incr_incr-23]=='i'&&buf_uart[incr_incr-22]=='m'&&buf_uart[incr_incr-21]=='e'&&buf_uart[incr_incr-20]=='='){
			if(buf_uart[incr_incr-2]==';'){
				RTC_TimeTypeDef sTime;
				RTC_DateTypeDef DateToUpdate;
				uint8_t *p=buf_uart; p+=incr_incr-22;
				DateToUpdate.Date=atoi((char*)p+3);
				DateToUpdate.Month=atoi((char*)p+6);
				DateToUpdate.Year=atoi((char*)p+9);
				sTime.Hours=atoi((char*)p+12);
				sTime.Minutes=atoi((char*)p+15);
				sTime.Seconds=atoi((char*)p+21);
				HAL_RTC_SetDate(&hrtc, &DateToUpdate,RTC_FORMAT_BIN);
				HAL_RTC_SetTime(&hrtc, &sTime,RTC_FORMAT_BIN);
				sprintf((char*)buf_uart,"\r\n*******************\r\nnew date: %d.%d.%d\r\nnew time: %d:%d:%d\r\n*******************\r\n",DateToUpdate.Date,DateToUpdate.Month,DateToUpdate.Year,sTime.Hours,sTime.Minutes,sTime.Seconds);
				HAL_UART_Transmit(&huart1,buf_uart,strlen((char*)buf_uart),100);
				incr_incr=0;
			}
		}
	}
		//ловим команду time=dd.mo.yy hh.mm.ss\r\n		
		
		if(incr_incr>2){																														//если индекс прима 4 и больше. Иначе, при поиске значений в принятой строке, уйдм в минус
			if(buf_uart[incr_incr]=='A'&&buf_uart[incr_incr-1]=='A'&&buf_uart[incr_incr-2]=='A'&&buf_uart[incr_incr-3]=='A'){				//если последние 4 принятых символа равны А
				//printf("A\r\n");
				incr_incr=0;
			}else if(buf_uart[incr_incr]=='Z'&&buf_uart[incr_incr-1]=='Z'&&buf_uart[incr_incr-2]=='Z'&&buf_uart[incr_incr-3]=='Z'){	//если последние 4 принятых символа равны Z
				//printf("Z\r\n");
				if(incr_incr==8){																												//если посылка из восьми символов
					RTC_TimeTypeDef sTime;
					RTC_DateTypeDef DateToUpdate;
					uint32_t un_tm=join_uint32(buf_uart[1],buf_uart[2],buf_uart[3],buf_uart[4]);
							//HAL_RTC_GetDate(&hrtc,&DateToUpdate,RTC_FORMAT_BIN);
							//HAL_RTC_GetTime(&hrtc,&sTime,RTC_FORMAT_BIN);
					unixtimetotime(un_tm,&DateToUpdate,&sTime);
					
					//sprintf(buf_uart,"d=%d m=%d y=%d h=%d mm=%d s=%d\r\n",DateToUpdate.Date,DateToUpdate.Month,DateToUpdate.Year,sTime.Hours,sTime.Minutes,sTime.Seconds);
					HAL_UART_Transmit(&huart1,buf_uart,strlen((char*)buf_uart),100);
					HAL_RTC_SetDate(&hrtc, &DateToUpdate,RTC_FORMAT_BIN);
					HAL_RTC_SetTime(&hrtc, &sTime,RTC_FORMAT_BIN);
				}
				incr_incr=0;
			}
		}
		incr_incr++;
		if(incr_incr>150) incr_incr=0;
	}
  /* USER CODE END USART1_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
