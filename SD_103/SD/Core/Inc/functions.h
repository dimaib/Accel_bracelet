#ifndef FUNCTION
#define FUNCTION
#include <stdint.h>
#include "rtc.h"

void split_uint16(const uint16_t uint16, uint8_t *uint8);
void split_uint32(const uint32_t uint32, uint8_t *uint8);
uint16_t join_uint16(uint8_t b1, uint8_t b0);
uint32_t join_uint32(uint8_t b3, uint8_t b2, uint8_t b1, uint8_t b0);
void set_FirstBlock(uint32_t start_block_n, uint32_t end_block_n);
void get_FirstBlock(void);
uint32_t get_unixtime(uint8_t day, uint8_t month, uint16_t year,uint8_t hours, uint8_t minets, uint8_t seconds);
void unixtimetotime(uint32_t uinixtime, RTC_DateTypeDef *structdate, RTC_TimeTypeDef *structtime);
void system_init(void);


#endif
