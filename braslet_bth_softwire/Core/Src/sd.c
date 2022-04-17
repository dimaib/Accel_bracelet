#include "main.h"
#include "sd.h"
#include "spi.h"
#include "usart.h"

#define CMD0 (0x40+0) // GO_IDLE_STATE
#define CMD1 (0x40+1) // SEND_OP_COND (MMC)
#define ACMD41 (0xC0+41) // SEND_OP_COND (SDC)
#define CMD8 (0x40+8) // SEND_IF_COND
#define CMD9 (0x40+9) // SEND_CSD
#define CMD16 (0x40+16) // SET_BLOCKLEN
#define CMD17 (0x40+17) // READ_SINGLE_BLOCK
#define CMD24 (0x40+24) // WRITE_BLOCK
#define CMD55 (0x40+55) // APP_CMD
#define CMD58 (0x40+58) // READ_OCR

sd_info_ptr sdinfo;
char str1[60]={0};

static void Error (void)
{
  LD_ON;
}

uint8_t SPIx_WriteRead(uint8_t Byte)
{
  uint8_t receivedbyte = 0;
  if(HAL_SPI_TransmitReceive(&hspi1,(uint8_t*) &Byte,(uint8_t*) &receivedbyte,1,0x1000)!=HAL_OK){
    Error();
  }
  return receivedbyte;
}

void SPI_SendByte(uint8_t bt)
{
  SPIx_WriteRead(bt);
}

uint8_t SPI_ReceiveByte(void)
{
  uint8_t bt = SPIx_WriteRead(0xFF);
  return bt;
}

void SPI_Release(void)
{
  SPIx_WriteRead(0xFF);
}

static uint8_t SD_cmd (uint8_t cmd, uint32_t arg)
{
  uint8_t n, res;
	if (cmd & 0x80){
		cmd &= 0x7F;
		res = SD_cmd(CMD55, 0);
		if (res > 1) return res;
	}
	SS_SD_DESELECT();
	SPI_ReceiveByte();
	SS_SD_SELECT();
	SPI_ReceiveByte();
	SPI_SendByte(cmd); // Start + Command index
	SPI_SendByte((uint8_t)(arg >> 24)); // Argument[31..24]
	SPI_SendByte((uint8_t)(arg >> 16)); // Argument[23..16]
	SPI_SendByte((uint8_t)(arg >> 8)); // Argument[15..8]
	SPI_SendByte((uint8_t)arg); // Argument[7..0]
	n = 0x01; // Dummy CRC + Stop
	if (cmd == CMD0) {n = 0x95;} // Valid CRC for CMD0(0)
	if (cmd == CMD8) {n = 0x87;} // Valid CRC for CMD8(0x1AA)
	SPI_SendByte(n);
	n = 10; // Wait for a valid response in timeout of 10 attempts
  do{
    res = SPI_ReceiveByte();
  } while ((res & 0x80) && --n);
  return res;
}

void SD_PowerOn(void)
{
	HAL_Delay(50);
}

uint8_t sd_ini(void)
{
	uint8_t i;
	uint8_t ocr[4];
  int16_t tmr;
  uint32_t temp;
  LD_OFF;
  sdinfo.type = 0;
	temp = hspi1.Init.BaudRatePrescaler;
	hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128; //156.25 kbbs
	HAL_SPI_Init(&hspi1);
	SS_SD_DESELECT();
	for(i=0;i<10;i++) //80 импульсов (не менее 74) Даташит стр 91
  SPI_Release();
	hspi1.Init.BaudRatePrescaler = temp;
	HAL_SPI_Init(&hspi1);
	SS_SD_SELECT();
	
	if (SD_cmd(CMD0, 0) == 1){ // Enter Idle state
		SPI_Release();
		if (SD_cmd(CMD8, 0x1AA) == 1){ // SDv2
			for (i = 0; i < 4; i++) ocr[i] = SPI_ReceiveByte();
			sprintf(str1,"OCR: 0x%02X 0x%02X 0x%02X 0x%02Xrn",ocr[0],ocr[1],ocr[2],ocr[3]);
			HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
		}
		else{ //SDv1 or MMCv3
		}
  }else{
		
    return 1;
  }
  return 0;
}