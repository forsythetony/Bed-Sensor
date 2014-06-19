/*
 * rtc.h
 *
 * Created: 1/3/2013 8:05:09 PM
 *  Author: Mark
 */ 


#ifndef RTC_H_
#define RTC_H_

#define RTC_ADDR (0b11010000)>>1

#include <compiler.h>

typedef struct
{
	uint8_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
	uint8_t dow;
	bool of;
	bool stop;
}rtc_t;

uint8_t rtc_reg[7];

bool rtc_init(void);
bool rtc_set_time(rtc_t *t);
bool rtc_get_time(rtc_t *t);
bool rtc_check_oflg(void);
void rtc_clear_oflg(void);
void rtc_stop(void);
void rtc_start(void);
bool rtc_status(void);
bool rtc_get_reg(uint8_t *d);

#endif /* RTC_H_ */