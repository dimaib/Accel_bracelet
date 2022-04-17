/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "../ssd1306/ssd1306.h"
#include "../ssd1306/ssd1306_tests.h"
#include "mpu6050.h"
#include "usart.h"
#include "rtc.h"
#include "functions.h"
#include "usart.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define SIZE_ARREY				506 						//512+512 (2 блока sd карты) -6 (в каждом втором блоке хранится юникстайм[4]+юникстайм мс[2])
accel_t accel[2][SIZE_ARREY]={0};					//массив для хранения данных с акселерометра

uint8_t	data_ready=0;											//даннфе готовы для записи на флешку

extern uint32_t st_block;
extern uint32_t en_block;
extern uint8_t print_sd_write;						//флаг для вывода символа на дисплей, о записи на флешку

uint8_t buf_write_sd[SIZE_BLOCK]={0};

struct unix_t{											//массив хронящий unix time в момент начала записи 
	uint32_t s;
	uint16_t ms;
}	unix_time_start[2];

uint16_t counter=0;									//счетчик для записи данных акселерометра в массив
uint8_t arr_cout=0;									//номер текущего массива для записи
MPU6050_t MPU6050;
extern RTC_TimeTypeDef sTime;
extern RTC_DateTypeDef DateToUpdate;
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
osThreadId defaultTaskHandle;
osThreadId lcdHandle;
osThreadId accelHandle;
osThreadId SD_wrHandle;
osThreadId SD_rdHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void lcd_print(void const * argument);
void accel_read(void const * argument);
void SD_write(void const * argument);
void SD_read(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

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
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of lcd */
  osThreadDef(lcd, lcd_print, osPriorityNormal, 0, 128);
  lcdHandle = osThreadCreate(osThread(lcd), NULL);

  /* definition and creation of accel */
  osThreadDef(accel, accel_read, osPriorityNormal, 0, 256);
  accelHandle = osThreadCreate(osThread(accel), NULL);

  /* definition and creation of SD_wr */
  osThreadDef(SD_wr, SD_write, osPriorityNormal, 0, 256);
  SD_wrHandle = osThreadCreate(osThread(SD_wr), NULL);

  /* definition and creation of SD_rd */
  osThreadDef(SD_rd, SD_read, osPriorityNormal, 0, 128);
  SD_rdHandle = osThreadCreate(osThread(SD_rd), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */

/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
		//tt++;
		if(GPIOA->IDR&(1<<8)) GPIOB->ODR|=(1<<9); else GPIOB->ODR&=~(1<<9);							//если BLE_state pin имеет состояние ноль, то выключить светодиод, иначе включить
		
		//sprintf(bbf,"dimaib %d\r\n",tt);
		//sprintf(bbf,"AT\r\n",tt);
		
		//HAL_UART_Transmit(&huart1,(uint8_t*)bbf,strlen(bbf),100);
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_lcd_print */
/**
* @brief Function implementing the lcd thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_lcd_print */
void lcd_print(void const * argument)
{
  /* USER CODE BEGIN lcd_print */
  /* Infinite loop */
  for(;;)
  {
		ssd1306_printtime();									//вывод времени на дисплей
		//ssd1306_printaccel(accel[arr_cout][counter-1].X,accel[arr_cout][counter-1].Y,accel[arr_cout][counter-1].Z); //тест дисплея и акселерометра
    osDelay(400);
  }
  /* USER CODE END lcd_print */
}

/* USER CODE BEGIN Header_accel_read */
/**
* @brief Function implementing the accel thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_accel_read */
void accel_read(void const * argument)
{
  /* USER CODE BEGIN accel_read */
  /* Infinite loop */
	uint16_t old_s=0;
	uint16_t unixtime_ms=0;
  for(;;)
  {
		HAL_RTC_GetDate(&hrtc,&DateToUpdate,RTC_FORMAT_BIN);
		HAL_RTC_GetTime(&hrtc,&sTime,RTC_FORMAT_BIN);
		unixtime_ms+=26;
		if(old_s!=sTime.Seconds){
			//sprintf(buff,"%dms\r\n",unixtime_ms);
			//HAL_UART_Transmit(&huart1,(uint8_t*)buff,strlen(buff),100);	
			old_s=sTime.Seconds;
			unixtime_ms=0;
		}		
		MPU6050_Read_Accel(&hi2c1, &MPU6050);																		//чтение значений из акселерометра
		accel[arr_cout][counter].X=MPU6050.Accel_X_RAW;
		accel[arr_cout][counter].Y=MPU6050.Accel_Y_RAW;
		accel[arr_cout][counter].Z=MPU6050.Accel_Z_RAW;
		
		//sprintf((char*)buff,"%lu.%d, %f, %f, %f\r\n",(unsigned long)get_unixtime(DateToUpdate.Date,DateToUpdate.Month,DateToUpdate.Year,sTime.Hours,sTime.Minutes, sTime.Seconds), unixtime_ms,MPU6050.Accel_X_RAW/10000.0,MPU6050.Accel_Y_RAW/10000.0,MPU6050.Accel_Z_RAW/10000.0);
		//HAL_UART_Transmit(&huart1,buff,strlen((char*)buff),100);
		//CDC_Transmit_FS(buff,strlen((char*)buff));
		counter++;
		if(counter>=SIZE_ARREY){
			counter=0;
			
			//unix_time_start[arr_cout].s =sTime.Seconds;
			unix_time_start[arr_cout].s=get_unixtime(DateToUpdate.Date,DateToUpdate.Month,DateToUpdate.Year+2000,sTime.Hours,sTime.Minutes,sTime.Seconds);
			unix_time_start[arr_cout].ms=unixtime_ms;
			arr_cout^=(1<<0);
			print_sd_write=1;
			data_ready=1;
		}

		//GPIOB->ODR^=(1<<9);
    osDelay(20);
  }
  /* USER CODE END accel_read */
}

/* USER CODE BEGIN Header_SD_write */
/**
* @brief Function implementing the SD_wr thread.
* @param argument: Not used
* @retval None
*/

/* USER CODE END Header_SD_write */
void SD_write(void const * argument)
{
  /* USER CODE BEGIN SD_write */
	uint8_t send_ble_on=1;																							//если 1, то отправка данных по блютусу разрешена, если 0, то запрещена  !!!ДЛЯ ОТЛАДКИ!!!
  /* Infinite loop */
	
  for(;;)
  {
		if(data_ready){
			data_ready=0;
			//GPIOB->ODR|=(1<<9);
			uint8_t invert_arr_cout=arr_cout^(1<<0);
			for(uint16_t xx=0;xx<SIZE_BLOCK;xx++)buf_write_sd[xx]=0x00;			//очищаем массив
			split_uint32(unix_time_start[invert_arr_cout].s,buf_write_sd);	//вносим в первые 4 байта блока юникс тайм
			buf_write_sd[4]=unix_time_start[invert_arr_cout].ms&0xff;				//вносим в массив 2 байта юникс тайм мс
			buf_write_sd[5]=(unix_time_start[invert_arr_cout].ms>>8)&0xff;	//вносим в массив 2 байта юникс тайм мс
			uint16_t index_arr=6;
			for(uint8_t xx=1;xx<(SIZE_BLOCK/6);xx++){
					buf_write_sd[index_arr]=accel[invert_arr_cout][xx-1].X&0xff;
					buf_write_sd[index_arr+1]=(accel[invert_arr_cout][xx-1].X>>8)&0xff;
					buf_write_sd[index_arr+2]=accel[invert_arr_cout][xx-1].Y&0xff;
					buf_write_sd[index_arr+3]=(accel[invert_arr_cout][xx-1].Y>>8)&0xff;
					buf_write_sd[index_arr+4]=accel[invert_arr_cout][xx-1].Z&0xff;
					buf_write_sd[index_arr+5]=(accel[invert_arr_cout][xx-1].Z>>8)&0xff;
					index_arr+=6;
			}
			en_block++;
			SD_Write_Block(buf_write_sd,en_block, SIZE_BLOCK); 							//Запишем блок в буфер
			if(GPIOA->IDR&(1<<8)&&send_ble_on){															//если блютуз подключился к серверу
				HAL_UART_Transmit(&huart1,(uint8_t*)"AAAA",4,100);						//передадим в блютуз стартовую строку передачи
				HAL_UART_Transmit(&huart1,buf_write_sd,SIZE_BLOCK,100);				//передадим в блютуз первый блок данных с юникс тайм
			}
			for(uint16_t ii=84;ii<508-83;ii+=85){
					index_arr=0;
					for(uint16_t xx=ii;xx<ii+(SIZE_BLOCK/6);xx++){
							buf_write_sd[index_arr]=accel[invert_arr_cout][xx].X&0xff;
							buf_write_sd[index_arr+1]=(accel[invert_arr_cout][xx].X>>8)&0xff;
							buf_write_sd[index_arr+2]=accel[invert_arr_cout][xx].Y&0xff;
							buf_write_sd[index_arr+3]=(accel[invert_arr_cout][xx].Y>>8)&0xff;
							buf_write_sd[index_arr+4]=accel[invert_arr_cout][xx].Z&0xff;
							buf_write_sd[index_arr+5]=(accel[invert_arr_cout][xx].Z>>8)&0xff;
							index_arr+=6;
					}
					en_block++;
					SD_Write_Block(buf_write_sd,en_block, SIZE_BLOCK); 				//Запишем блок в буфер
					if(GPIOA->IDR&(1<<8)&&send_ble_on) HAL_UART_Transmit(&huart1,buf_write_sd,SIZE_BLOCK,100);					//передадим в блютуз блок данных
					
			}
			if(en_block>15200000) en_block=0;
			set_FirstBlock(st_block,en_block);
			if(GPIOA->IDR&(1<<8)&&send_ble_on) HAL_UART_Transmit(&huart1,(uint8_t*)"ZZZZ",4,100);									//передадим в блютуз конечную строку передачи
		}

			//GPIOB->ODR&=~(1<<9);			
		}
    osDelay(10);
  /* USER CODE END SD_write */
}

/* USER CODE BEGIN Header_SD_read */
/**
* @brief Function implementing the SD_rd thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_SD_read */
void SD_read(void const * argument)
{
  /* USER CODE BEGIN SD_read */
  /* Infinite loop */
  for(;;)
  {
    osDelay(100);
  }
  /* USER CODE END SD_read */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
