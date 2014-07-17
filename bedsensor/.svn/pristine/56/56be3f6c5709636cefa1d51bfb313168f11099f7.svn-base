/*
 * rtc.c
 *
 * Created: 1/3/2013 8:04:57 PM
 *  Author: Mark
 */ 

#include "rtc.h"
#include <sysclk.h>
#include <gpio.h>
#include <compiler.h>
#include <asf.h>
#include <user_board.h>

bool rtc_init(void)
{
	twi_options_t twi_opts;
	
	twi_opts.chip = RTC_ADDR;
	twi_opts.pba_hz = sysclk_get_pba_hz();
	twi_opts.speed = 100000;
	
	if(twi_master_init(RTC_TWI,&twi_opts) != TWI_SUCCESS) return false;
	
	if(twi_probe(RTC_TWI,RTC_ADDR) != TWI_SUCCESS) return false;

	return true;
}	

bool rtc_set_time(rtc_t *t)
{
	// This Function will clear the osc flag
	uint8_t w[7]={0,0,0,0,0,0,0};
	uint8_t hld = 0;
	uint8_t val = 0;
	
	twi_package_t pkg;
	
	pkg.addr_length = 1;
	pkg.addr = 0;
	pkg.chip = RTC_ADDR;
	pkg.buffer = w;
	pkg.length = 7;	
	
	// [00] SECONDS
	hld = t->sec;
	val = hld/10;
	w[0] = 0;	// Clear Stop Flag
	w[0] |= (0b01110000 & (val<<4));
	val = hld%10;
	w[0] |= (0b00001111 & val);
	
	// [01] MINUTES
	hld = t->min;
	val = hld/10;
	w[1] = 0; // Clear Osc flag
	w[1] |= (0b01110000 & (val<<4));
	val = hld%10;
	w[1] |= (0b00001111 & val);
	
	// [02] CENT_HOURS
	hld = t->hour;
	val = hld/10;
	w[2] = 0; // Clear Cent_en and Cent
	w[2] |= (0b00110000 & (val<<4));
	val = hld%10;
	w[2] |= (0b00001111 & val);
	
	// [03] DAY
	w[3] = 0; // Clear day of week
	
	// [04] DATE
	hld = t->day;
	val = hld/10;
	w[4] = 0;
	w[4] |= (0b00110000 & (val<<4));
	val = hld%10;
	w[4] |= (0b00001111 & val);
	
	// [05] MONTH
	hld = t->month;
	val = hld/10;
	w[5] = 0;
	w[5] |= (0b00010000 & (val<<4));
	val = hld%10;
	w[5] |= (0b00001111 & val);
	
	// [06] YEAR
	w[0] = 0;
	hld = t->year;
	val = hld/10;
	w[6] |= (0b11110000 & (val<<4));
	val = hld%10;
	w[6] |= (0b00001111 & val);
	
	if(twi_master_write(RTC_TWI,&pkg) != TWI_SUCCESS) return false;
	
	return true;	 
}

bool rtc_get_time(rtc_t *t)
{	
	uint8_t w[7];
	
	twi_package_t pkg;
	
	uint8_t hld = 0;
	
	pkg.addr_length = 1;
	pkg.addr = 0;
	pkg.chip = RTC_ADDR;
	pkg.buffer = w;
	pkg.length = 7;
	
	if(twi_master_read(RTC_TWI,&pkg) != TWI_SUCCESS) return false;
	
	//[00] SECONDS
	hld = w[0];
	if(hld & 0b10000000) t->stop = true;
	else
	{
		t->stop = false;
	}
	
	t->sec = ((hld >> 4) & 0b00000111)*10 + (hld & 0b00001111);
	
	//[01] MINUTES
	hld = w[1];
	if(hld & 0b10000000) t->of = true;
	else
	{
		t->of = false;
	}
	
	t->min = ((hld >> 4) & 0b00000111)*10 + (hld & 0b00001111);
	
	//[02] CENT_HOURS
	hld = w[2];
	t->hour = ((hld >> 4) & 0b00000011)*10 + (hld & 0b00001111);
	
	//[03] Day of Week
	hld = w[3];
	t->dow = (hld & 0b00000111);
	
	//[04] DATE
	hld = w[4];
	t->day = ((hld >> 4) & 0b00000011)*10 + (hld & 0b00001111);
	
	//[05] MONTH
	hld = w[5];
	t->month = ((hld >> 4) & 0b00000001)*10 + (hld & 0b00001111);
	
	//[06] YEAR
	hld = w[6];
	t->year = ((hld >> 4) & 0b00001111)*10 + (hld & 0b00001111);
	return true;	
}

bool rtc_get_reg(uint8_t *d)
{
	twi_package_t pkg;
	
	pkg.addr_length = 1;
	pkg.addr = 0;
	pkg.chip = RTC_ADDR;
	pkg.buffer = d;
	pkg.length = 7;
	
	if(twi_master_read(RTC_TWI,&pkg) != TWI_SUCCESS) return false;
	
	return true;
}

