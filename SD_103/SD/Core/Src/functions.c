#include "main.h"
#include "functions.h"
#include "sd.h"
#include "usart.h"
#include "rtc.h"
#include "time.h"
#include "../ssd1306/ssd1306.h"
#include "../ssd1306/ssd1306_tests.h"
#include "mpu6050.h"
#include "i2c.h"

//uint32_t st_block=0xAAFFDD11;
//uint32_t en_block=0xCC773344;


extern RTC_TimeTypeDef sTime;
extern RTC_DateTypeDef DateToUpdate;

uint32_t st_block=0;
uint32_t en_block=0;
//extern RTC_TimeTypeDef sTime;

uint32_t get_unixtime(uint8_t day, uint8_t month, uint16_t year,uint8_t hours, uint8_t minets, uint8_t seconds)              //вычисление номера дня по юлианскому колендарю
{
	uint8_t a=(14-month)/12;                                            			//перевод даты в юлианскую систему
	uint16_t y=year+4800-a;                                             			//перевод даты в юлианскую систему
	uint16_t m=month+12*a-3;                                            			//перевод даты в юлианскую систему
	uint16_t JDN_day=day+((153*m+2)/5)+365*y+y/4-y/100+y/400-47801;//32045; 	//перевод даты в юлианскую систему и поиск разницы в днях между 01.01.1970
	return JDN_day*86400+hours*3600+minets*60+seconds;                  			//перевод разницы дней в секунды и прибавление текущего времени в секундах
}

void unixtimetotime(uint32_t uinixtime, RTC_DateTypeDef *structdate, RTC_TimeTypeDef *structtime)														//преобразование unix time в обычный формат времени
{
	struct tm ts;
	time_t ur=(time_t)uinixtime;
	//ts = *gmtime(&ur);
	//ts = *localtime(&ur);
	_localtime_r(&ur,&ts);
	//uint8_t bufff[100]={0};
	//sprintf((char*)bufff,"%d.%d.%d\r\n%d.%d.%d\r\n\r\n",ts.tm_mday,ts.tm_mon,ts.tm_year%100,ts.tm_hour,ts.tm_min,ts.tm_sec);
	//HAL_UART_Transmit(&huart1,bufff,strlen((char*)bufff),100);
	structdate->Date=(uint8_t)ts.tm_mday;																							//заполняем структуру даты
	structdate->Month=(uint8_t)ts.tm_mon+1;																						//заполняем структуру даты
	structdate->Year=(uint8_t)ts.tm_year%100;																					//заполняем структуру даты
	structtime->Hours=(uint8_t)ts.tm_hour;																						//заполняем структуру времени
	structtime->Minutes=(uint8_t)ts.tm_min;																						//заполняем структуру времени
	structtime->Seconds=(uint8_t)ts.tm_sec;																						//заполняем структуру времени
}

void split_uint16(const uint16_t uint16, uint8_t *uint8)													//функция разделения uint16_t по байтам
{
	uint8[0]=uint16&0xff;
	uint8[1]=(uint16>>8)&0xff;
}

void split_uint32(const uint32_t uint32, uint8_t *uint8)													//функция разделения uint32_t по байтам
{
  for(uint16_t i=0;i<4;i++) uint8[i]=(uint32>>(i*8))&0xff;
}

uint16_t join_uint16(uint8_t b1, uint8_t b0)																//функция объединения 2х байт в uint16_t
{
  return (b1<<8)|b0;
}

uint32_t join_uint32(uint8_t b3, uint8_t b2, uint8_t b1, uint8_t b0)				//функция объединения 4х байт в uint32_t
{
  return (b3<<24)|(b2<<16)|(b1<<8)|b0;
}

void set_FirstBlock(uint32_t start_block_n, uint32_t end_block_n)			//функция формирования и записи первого байта с настройками
{
	char buf[SIZE_BLOCK]={0};
	uint8_t *p_buf=(uint8_t*)buf;
	buf[0]='R';																													//первый байт, говорит, что флешка, уже была использована и готова к работе
	p_buf++; split_uint32(start_block_n,p_buf);													//формируем стартовые номер блока
	p_buf+=4;split_uint32(end_block_n,p_buf);														//формируем конечный номер блока
	
	SD_Write_Block((uint8_t*)buf,0,SIZE_BLOCK); 																	//Запишем нулевой блок
}

void get_FirstBlock(void)																							//функция чтения первого байта с настройками
{
	uint8_t buf[15]={0};
	SD_Read_Block(buf,0,15); 																						//Прочитаем нулевой блок
	if(buf[0]!='R'){																										//если флешка не инициализирована устройством, то устанавливаем нужный формат для нулевого блока
		st_block = 0; en_block = 0;
		set_FirstBlock(st_block,en_block);
	}else{
		st_block = join_uint32(buf[4],buf[3],buf[2],buf[1]);
		en_block = join_uint32(buf[8],buf[7],buf[6],buf[5]);
	}
}


uint8_t x_pos_load=0;																								//положение точки при загрузке
uint8_t y_pos_load=10;																							//положение точки при загрузке
void oled_print_paint(char *simvol)																							//вывод точки при загрузке устройства
{
	if(simvol[0]!='.'){
		ssd1306_Fill(Black);
		x_pos_load=0;y_pos_load=10;
	}
	ssd1306_SetCursor(x_pos_load, y_pos_load);ssd1306_WriteString(simvol, Font_11x18, White);
	ssd1306_UpdateScreen();
	x_pos_load+=11;
	//HAL_Delay(100);
	if(x_pos_load>=121){x_pos_load=0;y_pos_load+=18;}
}


uint8_t ble_init()																										//функция инициализации блютуса
{
	oled_print_paint("2");
	uint8_t ble_ok=0;
	uint8_t at_commands[][30]={																					//массив с АТ командами для настройки блютуса
		"AT\r\n",
		"AT+NAME=braslet_05\r\n",
		"AT+PSWD=1234\r\n",
		"AT+UART=115200,0,0\r\n"
	};
	BLE_RES_0; BLE_KEY_1; HAL_Delay(500); BLE_RES_1;										//сбрасываем блютус и переводим его в режим настройки АТ командами
	for(uint8_t i=0;i<4;i++){
		while(1){
			HAL_UART_Transmit(&huart1,at_commands[i],strlen((char*)at_commands[i]),100);
			uint8_t recive_temp[20]={0};																		//массив для прима ответа по юарту
			HAL_UART_Receive(&huart1,recive_temp,20,300);
			oled_print_paint(".");
			if(recive_temp[0]=='O'&&recive_temp[1]=='K'){										//если приняли ОК
				recive_temp[0]=0;recive_temp[1]=0;
				GPIOB->ODR|=(1<<9);
				HAL_Delay(100);
				ble_ok=0;
				break;
			}else {
				GPIOB->ODR&=~(1<<9);
				ble_ok++;
			}
		}	
		GPIOB->ODR&=~(1<<9);
	}
	BLE_RES_0;
	BLE_KEY_0;
	HAL_Delay(500);
	BLE_RES_1;
	
	//Перенастройка юарта на другую скорость!
	huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	HAL_UART_Init(&huart1);
	NVIC_EnableIRQ(USART1_IRQn); USART1->CR1|=USART_CR1_RXNEIE;       //разрешение прерывания по приму данных UART1. Прерывание для debug
	//Перенастройка юарта на другую скорость!
	return ble_ok;
}

void system_init()																										//общая инициализация всех устройств в системе. дисплей, акселерометр, sd карта, установка системного времени и тд
{
	ssd1306_Init();																											//инициализация oled	
	oled_print_paint("1");																							//выводим на дисплей информацию, что началась инициализация акселерометра
	while (MPU6050_Init(&hi2c1) == 1){																	//инициализация акселерометра
		oled_print_paint(".");
	}																	
	ble_init();
	oled_print_paint("3");																							//выводим на дисплей информацию, что началась инициализация sd карты
	while(sd_ini()){																										//инициализация sd карты
		oled_print_paint(".");
	}
	const char time_now[8]={__TIME__};																	//текущее время при компиляции
	sTime.Hours=(time_now[0]-48)*10+(time_now[1]-48);										//подстановка текущего часа для RTC
	sTime.Minutes=(time_now[3]-48)*10+(time_now[4]-48);									//подстановка текущих минут для RTC
	sTime.Seconds=(time_now[6]-48)*10+(time_now[7]-48)+5;								//подстановка текущих секунд для RTC. +5 учитываем, что на компиляцию и заливку проекта уходит 5с.
	HAL_RTC_SetTime(&hrtc, &sTime,RTC_FORMAT_BIN);
	//const char date_now[30]={__DATE__};
	DateToUpdate.Date=2;
	DateToUpdate.Month=11;
	DateToUpdate.Year=21;
	HAL_RTC_SetDate(&hrtc, &DateToUpdate,RTC_FORMAT_BIN);								//установка системной даты, из структуры, которая заполняется выше
	get_FirstBlock();																										//запросим нулевой блок, если его нет, сформируем и запишем его
}
