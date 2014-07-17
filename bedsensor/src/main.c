/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */

#include <asf.h>
#include <compiler.h>

#include "main.h"

#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "system/buttons.h"
#include "system/led.h"
#include "system/wireless/xbee-api.h"

#include <user_board.h>
#include "wdt.h"

#define FWV_MAJ 4
#define FWV_MIN 3
#define FWV_REV 2

#define BTN_1 0
#define BTN_2 1

char pstr[100];
char fstr[100];

char str1[300];
char str2[300];
char str3[300];

uint8_t blk[200];

extern volatile uint8_t time_valid;
extern volatile uint32_t val_avg;
extern volatile uint32_t val_avg_hold;
extern volatile uint16_t val_avgs[100];
extern volatile uint8_t avg_idx;
extern volatile uint16_t avg_valid;

extern volatile uint8_t sample_enable;

extern volatile uint8_t b_year;
extern volatile uint8_t b_lpyear;
extern volatile uint8_t b_month;
extern volatile uint8_t b_day;
extern volatile uint8_t b_hour;
extern volatile uint8_t b_min;
extern volatile uint8_t b_sec;
extern volatile uint16_t b_msec;
extern volatile uint8_t timestamp_valid;
extern volatile uint8_t temp_block;
extern volatile uint8_t send_ping_ack;
extern volatile uint32_t ping_timeout;

t_cpu_time lgr_hold_off;

void add_second_to_date(void)
{	
	b_sec++;
	if(b_sec == 60)	
	{
		b_min++;
		b_sec = 0;
		if(b_min == 60)
		{
			b_hour++;
			b_min = 0;
			if(b_hour == 24)
			{
				b_day++;
				b_hour = 0;
				
				if(b_month == 2)
				{
					if((b_day >28) && (b_lpyear!=1))
					{
						b_month = 3;
						b_day = 1;
					}
					else if(b_day >29)
					{
						b_month = 3;
						b_day = 1;
					}
				}
				else if (b_month == 4 ||
				b_month == 6 ||
				b_month == 9 ||
				b_month == 11)
				{
					if(b_day >30)
					{
						b_month++;
						b_day = 1;
					}					
				}
				else if(b_month != 12)
				{
					if(b_day > 31)
					{
						b_month++;
						b_day = 1;
					}
				}
				else
				{
					b_year++;
					b_month = 1;
					
					b_lpyear = check_leap_year(b_year);
				}																						
			}
		}
	}		
}

uint8_t check_leap_year(uint8_t y)
{
	if((y%4 == 0) && (y%100 != 0))	return 1;
	else
	{
		return 0;
	}
}

uint8_t wifi_state = WAIT_FOR_MESSAGE;

int main (void)
{
	wdt_disable();

	wifi_state = WAIT_FOR_MESSAGE;
	
	// Initialize Clocks
	sysclk_init();
	delay_init(sysclk_get_cpu_hz());
	
	// Initialize UI
	InitButton(BTN_1,AVR32_PIN_PB22);
	InitButton(BTN_2,AVR32_PIN_PB18);
	InitLed(0,LED_ONB);
	
	LedOn(0);
	delay_s(2);
	LedOff(0);
	
	// Initialize Peripherals
	board_init();
	xb_usart_init();
	
	t_cpu_time wifi_timeout;
	
	int x = 0;
	uint8_t data_idx = 0;
	
	char in_str[300] = "";
	uint8_t time_exprd = 0;
	
	uint8_t parse_idx = 0;
	uint8_t cmd_val = 0;
	char cmd_str[11]="";

	uint8_t rtn = 0;
	
	char day_str[3] = "";
	char month_str[3] = "";
	char year_str[3] = "";
	char hour_str[3] = "";
	char min_str[3] = "";
	char sec_str[3] = "";
	
	timestamp_valid = 0;
	uint8_t sync_error = 0;
	temp_block = 0;
	
	cpu_irq_enable();

	ping_timeout = 0;
	LedBlink(0,ping_timeout/20+20);
	
	while(1)
	{
		// State Machine
		switch(wifi_state)
		{
			case WAIT_FOR_MESSAGE:
				cpu_irq_enable();
				sync_error = 0;
				temp_block = 0;
				rtn = wifi_read(&x);
				if(rtn)
				{
					temp_block = 1;
					if((char)x == '>') 
					{
						ping_timeout = 0;
						LedBlink(0,ping_timeout/20+20);
						wifi_state = MSG_START_RCVD;
						
						data_idx = 0;
						
						break;
					}
				}
				
				break;
				
			case MSG_START_RCVD:
				cpu_set_timeout(cpu_ms_2_cy(100,F_CPU),&wifi_timeout);
				time_exprd = 1;
				while(!cpu_is_timeout(&wifi_timeout))
				{
					rtn = wifi_read(&x);
					if(rtn)
					{
						
						in_str[data_idx] = x;
						if(x == '\n' || x == '\r')
						{
							data_idx++;
							in_str[data_idx] = 0;
							time_exprd = 0;
							break;
						}
						
						data_idx++;
					}
				}
				if(time_exprd)
				{
					wifi_state = WAIT_FOR_MESSAGE;				
				}
				else
				{
					wifi_state = PARSE_CMD;					
				}
				break;
				
			case PARSE_CMD:
				parse_idx = 0;
				cmd_val = 0;
				cmd_str[10]=0;
				while
					(in_str[parse_idx] != ':' 
					&& in_str[parse_idx]!='\n' 
					&& in_str[parse_idx] != 0 
					&& in_str[parse_idx] != '\r'
					&& parse_idx < 10)
					{
						cmd_str[parse_idx] = in_str[parse_idx];
						parse_idx++;
						
					}
					cmd_str[parse_idx]=0;

				wifi_state = HANDLE_CMD;
				break;
				
			case HANDLE_CMD:
				if(!strcmpi(cmd_str,"SYNC"))
				{
					month_str[0] = in_str[5];
					month_str[1] = in_str[6];
					month_str[2] = 0;
					day_str[0] = in_str[8];
					day_str[1] = in_str[9];
					day_str[2] = 0;
					year_str[0] = in_str[11];
					year_str[1] = in_str[12];
					year_str[2] = 0;
					hour_str[0] = in_str[14];
					hour_str[1] = in_str[15];
					hour_str[2] = 0;
					min_str[0] = in_str[17];
					min_str[1] = in_str[18];
					min_str[2] = 0;
					sec_str[0] = in_str[20];
					sec_str[1] = in_str[21];
					sec_str[2] = 0;
										
					b_day = atoi(day_str);
					if(b_day<1 || b_day >31)	sync_error = 1;
					b_month = atoi(month_str);
					if(b_month<1 || b_month >12)	sync_error = 1;
					b_year = atoi(year_str);
					if(b_year >99)	sync_error = 1;
					b_hour = atoi(hour_str);
					if(b_hour >23) sync_error = 1; 
					b_min = atoi(min_str);
					if(b_min > 59) sync_error = 1;
					b_sec = atoi(sec_str);
					if(b_sec > 59) sync_error = 1;
					b_msec = 0;
					b_lpyear = check_leap_year(b_year);
					
					if(sync_error == 1)
					{
						usart_write_line(&XB_USART,"<ERROR:BadSync\n\r");
						timestamp_valid = 0;
					}
					else
					{
						timestamp_valid = 1;
						usart_write_line(&XB_USART,"<OK\n\r");
					}					
					
				}
				else if (!strcmpi(cmd_str,"START"))
				{
					if(timestamp_valid)
					{
						usart_write_line(&XB_USART,"<OK\n\r");
						sample_enable = 1;
					}
					else
					{
						usart_write_line(&XB_USART,"<ERROR:NoSync\n\r");
					}
				}
				else if (!strcmpi(cmd_str,"STOP"))
				{
					sample_enable = 0;				
					usart_write_line(&XB_USART,"<OK\n\r");					
				}
				else if(!strcmpi(cmd_str,"P")) // PING
				{
					send_ping_ack = 1;
				}

				wifi_state = WAIT_FOR_MESSAGE;
				while(wifi_read(&x));
				break;
			default:
				wifi_state = WAIT_FOR_MESSAGE;
				break;
		}
		
		if (sample_enable == 1) {
			send_data();
		}
	}

}

void SoftwareReset(void)
{	
	cpu_irq_disable();
	// Enable the WDT with a 0s period (fastest way to get a Watchdog reset).
	wdt_opt_t opt = {
		.us_timeout_period = 500000
	};
	sysclk_set_source(SYSCLK_SRC_OSC0);
	wdt_enable(&opt);
	pll_disable(0);
	pll_disable(1);
	while (1);
}