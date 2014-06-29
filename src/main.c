/*	Dependency Information

	asf.h
	compiler.h
	string.h
	stdio.h
	ctype.h
	conf_sd_mmc_spi.h
	navigation.h
	user_board.h
	calendar.h
	twi.h


	main.h
	system/buttons.h
	system/led.h
	system/wireless/xbee-api.h
	system/rtc.h
	wdt.h

*/

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

//zigbee -t 0013a200409e96cd

#include <asf.h>

#include "main.h"

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <conf_sd_mmc_spi.h>
#include <navigation.h>
#include "system/buttons.h"
#include "system/led.h"
#include "system/wireless/xbee-api.h"

#include <user_board.h>

#include <twi.h> 
#include "system/rtc.h"
#include "wdt.h"

#define FWV_MAJ 4
#define FWV_MIN 1
#define FWV_REV 1

bool msc_enb = false;
bool usb_attached = 0;

uint8_t 	log_data 	= 0;

uint32_t 	chunk_cntr 	= 0;
uint8_t 	file_count 	= 5;
uint8_t 	file_num 	= 0;

#define BTN_1 0
#define BTN_2 1

char pstr[100];
char fstr[100];

char str1[300];
char str2[300];
char str3[300];

uint8_t blk[200];

extern volatile uint8_t time_valid;
extern volatile rtc_t sys_time;
extern volatile val_avg;
extern volatile val_avg_hold;
extern volatile uint16_t val_avgs[100];
extern volatile uint8_t avg_idx;
extern volatile uint16_t avg_valid;

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
extern volatile uint32_t ping_timeout;



t_cpu_time lgr_hold_off;

/*
uint8_t xb_callback(XB_API_FRAME_t *frm)
{
	// This function will be called anytime a xb rx frame is successfully received

	uint8_t cmd 		= 0;
	uint8_t cmd_pulled 	= 0;
	
	if((frm->valid == 0x7E) && (frm->id == 0x90))
	{
		cmd = frm->data[11];
		
		switch(cmd)
		{
			case CMD_RESET:
				SoftwareReset();
				cmd_pulled = 1;
				break;
				
			case CMD_DIAGNOSTICS:
				sys.diag.trgr = 1;
				cmd_pulled = 1;
				break;
				
			case CMD_LOG_MODE:
				sys.log.mode = frm->data[12];
				cmd_pulled = 1;
				break;
			case CMD_SET_THRESHOLD:
				if(frm->data[12] == 0)
				{
					sys.log.new_lvl_avg = ((uint32_t)(frm->data[13])<<24) | ((uint32_t)(frm->data[14])<<16) |((uint32_t)(frm->data[15])<<8)|((uint32_t)(frm->data[16]));	
					sys.log.new_threshold = 1;
				}
				else
				{
					sys.log.new_lvl_avg = 0;
					sys.log.new_threshold = 1;
				}
				cmd_pulled = 1;
				break;
				
			default:
				cmd_pulled = 0;
				break;
		}
	}
	
	return cmd_pulled;
}
*/


void print_frame(XB_API_FRAME_t *frm);

void wrl_print(char *s)
{
	int len = strlen(s);
	
	if(len > 70)	len = 70;
	
	uint8_t i;
	XB_API_FRAME_t tx;
	
	tx.valid	= 0x7E;
	tx.id 		= 0x10;
	tx.data[0] 	= 0x00; // Frame ID
	
	for(i = 1;i<13;i++)
	{
		tx.data[i]=0;
	}
	
	tx.data[13] = CMD_CONSOLE;
	
	tx.len = 15+len;
	
	for(i = 0;i<len;i++)
	{
		tx.data[14 + i] = s[i];
	}
	
	while(!xb_send_frame(&tx));
}

void send_string(char * s)
{
	uint8_t i = 0;
	while(s[i]!=0)
	{
		xb_write(s[i]);
	}
}

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
	// Turn off watchdog timer. Set enable bit to 0.
	wdt_disable();
	
	// Local Variables
	
	//uint32_t i;
	


	//	#define'd to 0 in main.h
	wifi_state = WAIT_FOR_MESSAGE;
	
	//	Frame struct declared in xbee-api.h

	
	//	CPU struct declared  /src/asf/avr32/drivers/cpu/cycle_counter/cycle_counter.h




	
	// Initialize Clocks
	sysclk_init();

	//	Initializes the delay driver
	//	Takes unsigned long fcpu_hq
	delay_init(sysclk_get_cpu_hz());
	
	//	Struct declared in main.h
	sys.log.fs_chg = true;
	sys.log.fs_err = false;
	
	// Initialize UI

	/*
		@function:		InitButton

		@parameters:
						uint8_t idx
						uint8_t	pin

		@declared:	/src/system/buttons.h

		@summary:	Creates button
	*/
	InitButton(BTN_1,AVR32_PIN_PB22);
	InitButton(BTN_2,AVR32_PIN_PB18);

	/*
		@function:		InitLed

		@parameters:	
						uint8_t		idx
						uint32_t	pin

		@declared:	/src/system/led.h

		@summary:	Configures the LED struct at position 'idx'
					with the provided pin.
	*/

	InitLed(0,LED_ONB);
	InitLed(1,LED_LOG);
	InitLed(2,LED_ZGB);
	
	/*
		@definition:	LedOn(_l)

		@parameters:	
						uint8_t

		@defined in:	/src/system/led.h

		@summary:		A definition that basically redirects to led.h's 
						LedCtrl function. Turns on LED...obviously.
	*/
	LedOn(0);
	LedOn(1);
	LedOn(2);

	/*
		@definition:	delay_s(delay)

		@parameters:
						delay in seconds

		@defined in:	/src/asf/common/services/delay/delay.h

		@summary:		#define delay_s(delay) 	cpu_delay_ms(1000 * delay, F_CPU)
	*/
	delay_s(2);

	/*
		@definition:	LedOff(_l)

		@parameters:	
						uint8_t

		@defined in:	/src/system/led.h

		@summary:		A definition that basically redirects to led.h's 
						LedCtrl function. Turns off LED...obviously.
	*/	
	LedOff(0);
	LedOff(1);
	LedOff(2);
	
	/*
		@function:		board_init();

		@parameters:
						none

		@defined in:	/src/asf/common/boards/user_board/init.c

		@summary:			* This function is meant to contain board-specific initialization code
	 						* for, e.g., the I/O pins. The initialization can rely on application-
							* specific board configuration, found in conf_board.h.
							*
	*/
	// Initialize Peripherals
	board_init();
	
	// Delay to allow xbee to boot
	//delay_s(2);
	
	/*
		@function:		xb_usart_init();

		@parameters:
						none

		@defined in:	/src/system/wireless/xbee-api.h

		@summary:		Sets up xb module.
							1. Configures pins
							2. Configures serial options
							3. Enables Module
							4. Initializes RX buffer
							5. Flashes LED at index 0
							6. Enables USART Rx interrupt
	*/
	// Initialize and enable xbee
	xb_usart_init();
	//xb_rts_enb();

	// Initialize Real time clock	

	//rtc_init();	
	//rtc_get_time(&sys_time);


	//cpu_irq_enable();
	

	
	// Reset State Machine
	sys.state = IDLE;
	sys.log.mode = LOG_DISABLED;
	sys.log.status = LOG_OFF;
	sys.com.conn = false;
	sys.diag.trgr = 0;
	sys.log.new_lvl_avg = 0;
	sys.log.new_threshold = 0;
	
	// Get Sampling Threshold
	//if(!LoadThreshold())
	//{
	//	SampleThreshold();
	//}
	

	t_cpu_time wifi_timeout;

	//cpu_set_timeout(cpu_ms_2_cy(LGR_QRY_TIMEOUT,F_CPU),&last_lgr_query_tmr);
	//cpu_set_timeout(cpu_ms_2_cy(5000,F_CPU),&lgr_hold_off);
	
	//InitFolderStructure();
	/*
	b_year = sys_time.year;
	b_month = sys_time.month;
	b_day = sys_time.day;
	b_hour = sys_time.hour;
	b_min = sys_time.min;
	b_sec = sys_time.sec;
	*/
	//sprintf(str1,"RTC @ Boot: %u-%u-%u %u:%02u:%02u\n",b_month,b_day,b_year,b_hour,b_min,b_sec);
	//dbg_out(str1);
	

	

	
	int x = 0;
	uint8_t data_idx = 0;
	
	char in_str[300] = "";
	uint8_t time_exprd = 0;
	
	uint8_t parse_idx = 0;
	uint8_t cmd_val = 0;
	char cmd_str[11]="";
	
	//LedBlink(0,100);
	
	sys.log.smpl_enable = 0;

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

	/*
		@function:		cpu_irq_enable();

		@parameters:
						none

		@defined in:	/src/asf/common/utils/interrupt/interrupt_avr32.h

		@summary:		Enables interrupts
	*/
	cpu_irq_enable();
	

	
	ping_timeout = 0;

	/*
		@definition:	LedBlink(_l,_b)		LedCtrl(_l, LED_STAY_ON, 0, LED_BLINK, _b)

		@parameters:
						uint8_t		idx
						uint16_t	blink_msec

		@defined in:	/src/system/led.h

		@summary:		Given the index of the LED pin. It will blink it in intervals specified
						by the second parameter blink_msec

	*/
	LedBlink(0,ping_timeout/20+20);
	
	while(1)
	{
		/*
		x = usart_getchar(&XB_USART);
		if(x != USART_FAILURE)
		{
			usart_putchar(&XB_USART,x);
		}
		*/
		
		// State Machine
		switch(wifi_state)
		{
			case WAIT_FOR_MESSAGE:
				cpu_irq_enable();
				sync_error = 0;
				temp_block = 0;

				/*
					@function:		wifi_read(int *r);

					@parameters:
									Integer pointer

					@defined in:	/src/system/wireless/xbee-api.c

					@summary:		If wifi.head == wifi.tail return 0.
									Otherwise, set r to wifi.buffer[ wifi.head - 1 ]
									and return 1.
				*/
				rtn = wifi_read(&x);
				if(rtn)
				{
					//cpu_irq_disable();
					temp_block = 1;
					if((char)x == '>') 
					{
						ping_timeout = 0;
						LedBlink(0,ping_timeout/20+20);
						//LedPulse(1);
						wifi_state = MSG_START_RCVD;
						//sys.log.smpl_enable = 0;
						
						data_idx = 0;
						
						break;
					}
				}
				
				break;
				
			case MSG_START_RCVD:
				//cpu_irq_enable();

				/*
					@function:		cpu_set_timeout(unsigned long delay, t_cpu_time *cpu_time)

					@parameters:	
									delay: 		(input) delay in CPU cycles before timeout
									cpu_time:	(output) internal information used by the timer API

					@summary:		Sets a timer variable
				*/
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
					sys.log.smpl_enable = 0;
					
					
				}
				else
				{
					wifi_state = PARSE_CMD;
					
					
				}
				break;
				
			case PARSE_CMD:
				//usart_write_line(&XB_USART,"STATE:PARSE_CMD\n\r");
				parse_idx = 0;
				cmd_val = 0;
				cmd_str[10]=0;
				while
					(in_str[parse_idx] 		!=	':' 
					&& in_str[parse_idx]	!=	'\n' 
					&& in_str[parse_idx] 	!=	0 
					&& in_str[parse_idx] 	!=	'\r'
					&& parse_idx < 10)
					{
						cmd_str[parse_idx] = in_str[parse_idx];
						parse_idx++;
						
					}
					cmd_str[parse_idx]=0;
				//usart_write_line(&XB_USART,in_str);
				//usart_putchar(&XB_USART,(x));
				//usart_write_line(&XB_USART,cmd_str);
				//usart_putchar(&XB_USART,x);
				wifi_state = HANDLE_CMD;
				break;
				
			case HANDLE_CMD:
				if(!strcmpi(cmd_str,"SYNC"))
				{

					month_str[0]	= in_str[5];
					month_str[1] 	= in_str[6];
					month_str[2] 	= 0;

					day_str[0] 		= in_str[8];
					day_str[1] 		= in_str[9];
					day_str[2] 		= 0;

					year_str[0] 	= in_str[11];
					year_str[1] 	= in_str[12];
					year_str[2] 	= 0;

					hour_str[0] 	= in_str[14];
					hour_str[1] 	= in_str[15];
					hour_str[2] 	= 0;

					min_str[0] 		= in_str[17];
					min_str[1] 		= in_str[18];
					min_str[2] 		= 0;

					sec_str[0] 		= in_str[20];
					sec_str[1] 		= in_str[21];
					sec_str[2] 		= 0;
										
					b_day = atoi(day_str);
					if( b_day < 1 || b_day >31)	sync_error = 1;
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
						usart_write_line(&XB_USART,"For Realz\n\r");
						usart_write_line(&XB_USART,"<ERROR:BadSync\n\r");
						
						
						
						//temp_block = 0;
						timestamp_valid = 0;
					}
					else
					{
						timestamp_valid = 1;
						
						//temp_block = 1;
						usart_write_line(&XB_USART,"<OK\n\r");
						//temp_block = 0;
					}					
					
					/*
					sprintf(str1,"DATE:%u-%u-%u\n\r",b_day,b_month,b_year);
					usart_write_line(&XB_USART,str1);
					sprintf(str2,"TIME:%u:%u:%u\n\r",b_hour,b_min,b_sec);
					usart_write_line(&XB_USART,str2);
					*/
					
				}
				else if (!strcmpi(cmd_str,"START"))
				{
					if(timestamp_valid)
					{
						
						usart_write_line(&XB_USART,"<OK\n\r");
						
						sys.log.smpl_enable = 1;
					}
					else
					{
					
						usart_write_line(&XB_USART,"<ERROR:NoSync\n\r");
					
					}
				}
				else if (!strcmpi(cmd_str,"STOP"))
				{
					sys.log.smpl_enable = 0;				
					usart_write_line(&XB_USART,"<OK\n\r");

					
				}
				else if(!strcmpi(cmd_str,"PING"))
				{
					usart_write_line(&XB_USART,"<PA\n\r");
				}

				//while(usart_getchar(&XB_USART)!=USART_FAILURE);
				wifi_state = WAIT_FOR_MESSAGE;
				while(wifi_read(&x));
				break;
			default:
				wifi_state = WAIT_FOR_MESSAGE;
				break;
		}
		
		if (sys.log.smpl_enable == 1) {
			send_data();
		}
	}

}

void PrintData(char *str_print)
{
	
}

void SoftwareReset(void)
{
	//dbg_out("Software Reset");
	//wdt_reset_mcu();
	
	//dbg_out("Software Reset...");
	
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

bool LoadThreshold(void)
{
	
	dbg_out("Loading On Bed threshold from file");
	char th_str[20]="";
	nav_reset();

	if(!nav_drive_set(0))
	{
		sprintf(str1,"nav_drive_set failed: %u\n",fs_g_status);
		dbg_out(str1);
		nav_exit();
		return false;
	}

	if(!nav_partition_mount())
	{
		sprintf(str1,"nav_partition_mount failed: %u\n",fs_g_status);
		dbg_out(str1);
		nav_exit();
		return false;
	}

	nav_flat_root();
	
	if(nav_flat_findname("bed.cfg",false))
	{
		//dbg_out("File Found")
		file_open(FOPEN_MODE_R);
	}
	else
	{
		sprintf(str1,"File not found: ");
		dbg_out(str1);
		return false;
	}
	
	uint8_t bytes_read =0;
	
	uint32_t sample_value = 0;
	
	bytes_read = file_read_buf(th_str,19);
	
	//sprintf(str1,"%u bytes found [%s]",bytes_read,th_str);
	//dbg_out(str1);
	
	th_str[bytes_read] = '\0';
	
	sscanf(th_str,"%u",&sample_value);

	sprintf(str1,"Threshold Value: %u",sample_value);
	dbg_out(str1);
		
	sys.log.lvl_avg = sample_value;
	
	file_close();
	avg_valid = 1;
	sys.log.smpl_enable = 1;

	return true;
}

void RunDiagnostics(void)
{	
	uint32_t signal_avg[8];
	uint16_t signal_max[8];
	uint16_t signal_min[8];
	uint8_t i,j,k;
	uint16_t tmp = 0;
	
	uint16_t avg_v[2],max_v[2],min_v[2],p2p_v[2];
	uint32_t tmp32;
	
	dbg_out("Running Diagnostics Code...");
	
	sprintf(str1,"Firmware Version: %u_%u_%u",FWV_MAJ,FWV_MIN,FWV_REV);
	dbg_out(str1);
	
	dbg_out("Flashing Leds...");
	LedBlink(0,100);
	LedBlink(1,100);
	LedBlink(2,100);
	
	delay_s(2);
	
	LedOff(0);
	LedOff(1);
	LedOff(2);
	
	dbg_out("Checking RTC...");
	rtc_get_time(&sys_time);
	
	sprintf(str1,"Time from RTC: %u-%u-%u %u:%02u:%02u\n",sys_time.month,sys_time.day,sys_time.year,sys_time.hour,sys_time.min,sys_time.sec);
	dbg_out(str1);
	
	sprintf(str1,"RTC @ Boot: %u-%u-%u %u:%02u:%02u\n",b_month,b_day,b_year,b_hour,b_min,b_sec);
	dbg_out(str1);
	
	dbg_out("Waiting for data...");
	for(i=0;i<10;i++)
	{
		LedPulse(0);
		while(data_rdy==0); //LedPulse(1);//delay_ms(100);

		if(curr_data == 0) log_data = 1;
		else
		{
			log_data = 0;
		}
		
		for(j=0;j<8;j++)
		{
			for(k=0;k<100;k++)
			{
				tmp = data[log_data][j][k];
				signal_avg[j]+=tmp;
				if(k==0)
				{
					signal_max[j]=tmp;
					signal_min[j]=tmp;
				}
				else
				{
					if(tmp>signal_max[j])	signal_max[j]=tmp;
					if(tmp<signal_min[j])	signal_min[j]=tmp;
				}
			}
		}
		data_rdy = 0;
	}
	
	/*
#define F1 0
#define F2 2
#define F3 4
#define F4 6
#define R1 1
#define R2 3
#define R3 5
#define R4 7
*/

	sprintf(str1,"Sig\tAvg\tMax\tMin\tP2P:");
	dbg_out(str1);
	for(j = 1;j<8;j=j+2)	
	{
		signal_avg[j]=signal_avg[j]/1000;

		tmp32 = (signal_avg[j]*5000)/4096;
		avg_v[0]=tmp32/1000;
		avg_v[1]=tmp32%1000;
		
		tmp32 = (signal_max[j]*5000)/4096;
		max_v[0]=tmp32/1000;
		max_v[1]=tmp32%1000;
		
		tmp32 = (signal_min[j]*5000)/4096;
		min_v[0]=tmp32/1000;
		min_v[1]=tmp32%1000;
		
		tmp32 = ((signal_max[j]-signal_min[j])*5000)/4096;
		p2p_v[0]=tmp32/1000;
		p2p_v[1]=tmp32%1000;
		
		sprintf(str1,"R%u \t%01u.%03u\t%01u.%03u\t%01u.%03u\t%01u.%03u",j/2+1,avg_v[0],avg_v[1],max_v[0],max_v[1],min_v[0],min_v[1],p2p_v[0],p2p_v[1]);
		dbg_out(str1);
		
		signal_avg[j-1]=signal_avg[j-1]/1000;

		tmp32 = (signal_avg[j-1]*5000)/4096;
		avg_v[0]=tmp32/1000;
		avg_v[1]=tmp32%1000;
		
		tmp32 = (signal_max[j-1]*5000)/4096;
		max_v[0]=tmp32/1000;
		max_v[1]=tmp32%1000;
		
		tmp32 = (signal_min[j-1]*5000)/4096;
		min_v[0]=tmp32/1000;
		min_v[1]=tmp32%1000;
		
		tmp32 = ((signal_max[j-1]-signal_min[j-1])*5000)/4096;
		p2p_v[0]=tmp32/1000;
		p2p_v[1]=tmp32%1000;
		
		sprintf(str1,"F%u \t%01u.%03u\t%01u.%03u\t%01u.%03u\t%01u.%03u",j/2+1,avg_v[0],avg_v[1],max_v[0],max_v[1],min_v[0],min_v[1],p2p_v[0],p2p_v[1]);
		dbg_out(str1);
	}
	
	dbg_out("File system:");
	
	nav_exit();
	nav_reset();

	if(!nav_drive_set(0))
	{
		sprintf(str1,"nav_drive_set failed: %u\n",fs_g_status);
		dbg_out(str1);
		nav_exit();
		goto RESET;
	}

	if(!nav_partition_mount())
	{
		sprintf(str1,"nav_partition_mount failed: %u\n",fs_g_status);
		dbg_out(str1);
		nav_exit();
		goto RESET;
	}
	
	uint32_t dsk_space = nav_partition_freespace_percent();
	
	sprintf(str1,"Percent Full: %lu\n",dsk_space);
	dbg_out(str1);
	
	nav_flat_root();
	
	dbg_out("Getting File Names...");
	
	/*
	dbg_out("MAIN:");
	sprintf(str2,"");
	do
	{
		nav_file_getname(str2,200);
		sprintf(str1,"%s\t%lu",str2,nav_file_lgt());
		dbg_out(str1);
	}
	while(nav_filelist_set(0,FS_FIND_NEXT));
	*/
	
	nav_flat_root();
	nav_flat_findname("data",false); 
	nav_flat_cd();
	
	do
	{
		nav_file_getname(str2,200);
		sprintf(str1,"%s\t%lu",str2,nav_file_lgt());
		dbg_out(str1);
	}
	while(nav_filelist_set(0,FS_FIND_NEXT));
	
	nav_exit();
	
	delay_ms(3000);
	
	dbg_out("Diagnostics Complete!");
	
	RESET:
	SoftwareReset();

	sys.diag.trgr = 0;
}

bool SampleThreshold(void)
{
	char th_str[20]="";
	
	uint32_t i;
		
	sys.log.smpl_enable = 1;
	
	if(sys.log.new_lvl_avg == 0)
	{
		dbg_out("Sampling...\n");
		
		LedBlink(0,100);
		LedBlink(1,100);
		LedBlink(2,100);
		
		t_cpu_time samp_time;
		cpu_set_timeout(cpu_ms_2_cy(5100,F_CPU),&samp_time);
		

		uint32_t avg = 0;
		while(!cpu_is_timeout(&samp_time));
		
		for(i = 0;i<100;i++)
		{
			avg+=val_avgs [i];
		}
		
		sys.log.lvl_avg = (120*avg)/100;
		LedOff(0);
		LedOff(1);
		LedOff(2);
		
		sprintf(str1,"Done Sampling: %u\n",sys.log.lvl_avg);
		dbg_out(str1);
	}
	else
	{
		sys.log.lvl_avg = sys.log.new_lvl_avg;
	}
	
	sys.log.new_lvl_avg = 0;
	sys.log.new_threshold = 0;
	
	avg_valid = 1;
	
	nav_reset();

	if(!nav_drive_set(0))
	{
		sprintf(str1,"nav_drive_set failed: %u\n",fs_g_status);
		dbg_out(str1);
		nav_exit();
		return false;
	}

	if(!nav_partition_mount())
	{
		sprintf(str1,"nav_partition_mount failed: %u\n",fs_g_status);
		dbg_out(str1);
		nav_exit();
		return false;
	}

	nav_flat_root();
	//dbg_out("Looking for cfg file")
	if(nav_flat_findname("bed.cfg",false))
	{
		//dbg_out("Deleting cfg file");
		nav_file_del(false);
	}
	
	dbg_out("Creating cfg file");
	if(nav_file_create("bed.cfg"))
	{
		file_open(FOPEN_MODE_W);
	}
	else
	{
		dbg_out("Creating File Failed");
		return false;
	}
	
	sprintf(th_str,"%u",sys.log.lvl_avg);
	
	//sprintf(str1,"Write to file: %s",th_str);
	//dbg_out(str1);
	
	file_write_buf(th_str,strlen(th_str)+1);
	
	file_flush();
	file_close();
	
	return true;
}

bool CleanUpOldFiles(void);

bool TimeValid(rtc_t *t)
{
	if((t->year) < 10 || (t->year > 20))	return false;	
	if(t->month > 12) return false;
	if(t->day > 31)	return false;
	if(t->hour >24) return false;
	if(t->min > 60) return false;
	if(t->sec >60) return false;
	
	return true;
}

bool InitFolderStructure(void)
{
	nav_reset();

	if(!nav_drive_set(0))
	{
		sprintf(str1,"nav_drive_set failed: %u\n",fs_g_status);
		dbg_out(str1);
		nav_exit();
		return false;
	}
	
	if(!nav_partition_mount())
	{
		sprintf(str1,"nav_partition_mount failed: %u\n",fs_g_status);
		dbg_out(str1);
		nav_exit();
		return false;
	}
	
	nav_flat_root();
	
	nav_dir_make("data");
	//nav_dir_make("old");
	
	nav_exit();
	
	return true;
}

bool ArchiveFile(char *estr)
{
	#ifdef ARCHIVE_FILE
	nav_select(1);
	nav_drive_set(0);
	nav_select(0);
	
	if(!nav_file_getname(pstr,60))
	{
		sprintf(str1,"nav_file_getname failed: %u\n",fs_g_status);
		return false;
	}
	
	if(!nav_file_copy())
	{
		sprintf(str1,"Copy Failed: %u\n",fs_g_status-1);
		dbg_out(str1);
		return false;
	}
	
	if(!nav_flat_root())
	{
		sprintf(str1,"Nav_flat_root Failed: %u\n",fs_g_status-1);
		dbg_out(str1);
		return false;
	}
	
	if(!nav_flat_findname("old",false))
	{
		sprintf(str1,"nav_flat_findname failed: %u\n",fs_g_status-1);
		dbg_out(str1);
		return false;
	}
	
	if(!nav_flat_cd())
	{
		sprintf(str1,"nav_flat_cd failed: %u\n",fs_g_status-1);
		return false;
	}
	
	sys.log.fs_chg = true;
	
	sprintf(str2,"%s%s",pstr,estr);
	
	if(!nav_file_paste_start(str2))
	{
		sprintf(str1,"nav_file_paster_start failed: %u\n",fs_g_status);
		return false;
	}
	
	uint8_t resp;
	
	do 
	{
		resp = nav_file_paste_state(false);
	} while (resp == COPY_BUSY);
	
	if(resp == COPY_FAIL)
	{
		dbg_out("Copy Failed\n");
		return false;
	}
	
	nav_select(1);
	
	#endif
	nav_file_del(false);
	nav_select(0);
	
	nav_exit();
	return true;
}

bool QueryLogger(volatile rtc_t *cl)
{
	uint8_t i;
	
	uint8_t year,month,day,hour,min,sec;
	XB_API_FRAME_t rx,tx;
	
	tx.valid = 0x7E;
	tx.id = 0x10;
	tx.data[0] = 0x01; // Frame ID
	
	for(i = 1;i<13;i++)
	{
		tx.data[i]=0;
	}
	
	tx.data[13] = CMD_QUERY_LGR;
	tx.len = 15;
	
	xb_send_frame(&tx);
	
	if(!xb_get_frame(&rx,500))
	{
		dbg_out("Zigbee Ack Timeout\n");
		sys.com.conn = false;
		return false;
	}
	
	if(!xb_get_frame(&rx,2000))
	{
		dbg_out("Query Logger Resp Timeout\n");
		return false;
		sys.com.conn = false;
	}
	
	if((rx.valid == 0x7E) && (rx.id == 0x90) && (rx.data[11] = 0x69))
	{
		//dbg_out("Start Response Received\n");
	}
	else
	{
		dbg_out("Query Response Failure\n");
		sys.com.conn = false;
		return false;
	}
	
	dbg_out("Query Received");
	
	year = rx.data[12];
	month = rx.data[13];
	day = rx.data[14];
	hour = rx.data[15];
	min = rx.data[16];
	sec = rx.data[17];

	cl->year = year;
	cl->month = month;
	cl->day = day;
	cl->hour = hour;
	cl->min = min;
	cl->sec = sec;
	
	sprintf(str1,"New Time: %u:%02u:%02u %u-%u-%u\n",cl->hour,cl->min,cl->sec,cl->month,cl->day,cl->year);
	dbg_out(str1);
	
	rtc_set_time(cl);
	
	sys.com.conn = true;
	
	return true;
}

bool StartLog(void)
{
	nav_reset();
	if(!nav_select(0))
	{
		sprintf(str1,"Nav_select Failed: %u\n",fs_g_status-1);
		dbg_out(str1);
		return false;
	}
	
	if(!nav_drive_set(0))
	{
		sprintf(str1,"Navigator 0 Error: Could not set drive\n");
		dbg_out(str1);
		return false;
	}
	
	if(!nav_partition_mount())
	{
		sprintf(str1,"Partition Mount Failed: %u\n",fs_g_status-1);
		dbg_out(str1);
	}
	
	if(!nav_flat_reset())
	{
		sprintf(str1,"nav_flat_reset failed: %u\n",fs_g_status-1);
		dbg_out(str1);
		return false;
	}
	
	if(!nav_flat_findname("data",false))
	{
		sprintf(str1,"nav_flat_findname failed: %u\n",fs_g_status-1);
		dbg_out(str1);
		return false;
	}
	
	if(!nav_flat_cd())
	{
		sprintf(str1,"nav_flat_cd failed: %u\n",fs_g_status-1);
		dbg_out(str1);
		return false;
	}
	
	// Update the time
	rtc_get_time(&sys_time);
	
	uint16_t year = sys_time.year;
	year = year+2000;
	uint8_t month = sys_time.month;
	uint8_t day = sys_time.day;
	uint8_t hour = sys_time.hour;
	uint8_t min = sys_time.min;
	uint8_t sec = sys_time.sec;
	
	char fstr1[200]="";
	
	sprintf(fstr1,"%04u_%02u_%02u_%02u_%02u_%02u.bed",year,month,day,hour,min,sec);
	
	uint8_t i;
	uint8_t len = strlen(fstr1);
	
	// Sanitize File name
	for(i = 0;i<len;i++)
	{
		fstr1[i] &= 0b01111111;
	}

	sprintf(str1,"Creating File: %s\n",fstr1);
	dbg_out(str1);
	if(!nav_file_create(fstr1))
	{
		sprintf(str1,"Error Creating File: %u\n",(fs_g_status-1));
		return false;
	}
	
	sys.log.fs_chg = true;
	
	if(!file_open(FOPEN_MODE_W))
	{
		dbg_out("File Not Opened\n");
		return false;
	}
	
	blk[0] = 0;
	blk[1] = (uint8_t)(year-2000);
	blk[2] = month;
	blk[3] = day;
	blk[4] = hour;
	blk[5] = min;
	blk[6] = sec;
	
	file_write_buf(blk,7);
	file_flush();
	ClearSamples();
	sys.log.cntr = 0;

	return true;
}

void ClearSamples(void)
{
	cpu_irq_disable();
	data_idx = 0;
	data_rdy = 0;
	curr_data = 0;
	cpu_irq_enable();
}

void HandleLog(void)
{
	uint16_t i;
	uint16_t idx = 1;
	
	if(curr_data == 0) log_data = 1;
	else
	{
		log_data = 0;
	}
	
	blk[0]=0x01;
	

		for(i = 0;i<100;i++)
		{
			blk[idx] = (_d(F1,i)>>4) & 0x00FF;
			blk[idx+1] = (_d(F2,i)>>4) & 0x00FF;
			blk[idx+2] = ((_d(F1,i) & 0x000F) << 4) + (_d(F2,i) & 0x000F);
			
			blk[idx+3] = (_d(F3,i)>>4) & 0x00FF;
			blk[idx+4] = (_d(F4,i)>>4) & 0x00FF;
			blk[idx+5] = ((_d(F3,i) & 0x000F) << 4) + (_d(F4,i) & 0x000F);
			
			blk[idx+6] = (_d(R1,i)>>4) & 0x00FF;
			blk[idx+7] = (_d(R2,i)>>4) & 0x00FF;
			blk[idx+8] = ((_d(R1,i) & 0x000F) << 4) + (_d(R2,i) & 0x000F);
			
			blk[idx+9] = (_d(R3,i)>>4) & 0x00FF;
			blk[idx+10] = (_d(R4,i)>>4) & 0x00FF;
			blk[idx+11] = ((_d(R3,i) & 0x000F) << 4) + (_d(R4,i) & 0x000F);
			
			idx+=12;
		}
		
		data_rdy = 0;
		file_write_buf(blk,1201);
		LedPulse(1);
}

void CloseLog(void)
{
	blk[0] = 0xA5;
	blk[1] = 0x5A;
	file_write_buf(blk,2);
	file_flush();
	file_close();
	dbg_out("File Closed\n");
}

bool CheckForFiles(void)
{

	if(sys.log.fs_chg == false)
	{
		return false;
	}
	
	//dbg_out("File System is Marked as CHANGED");
	//dbg_out("Checking for Files");
	
	nav_reset();
	if(!nav_drive_set(0))	
	{
		sprintf(str1,"nav_drive_set failed: %u\n",fs_g_status);
		dbg_out(str1);
		nav_exit();
		return false;
	}		
		
	if(!nav_partition_mount()) 
	{
		sprintf(str1,"nav_partition_mount failed: %u\n",fs_g_status);
		dbg_out(str1);
		nav_exit();
		return false;
	}		
	
	if(!nav_filelist_findname("data",false))
	{
		sprintf(str1,"nav_filelist_findname failed: %u\n",fs_g_status);
		dbg_out(str1);
		nav_exit();
		return false;
	}
	
	if(!nav_flat_cd())
	{
		sprintf(str1,"nav_flat_cd failed: %u\n",fs_g_status);
		dbg_out(str1);
		nav_exit();
		return false;
	}
	
	if(!nav_filelist_set(0,FS_FIND_NEXT)) 
	{
		//sprintf(str1,"nav_filelist_set failed: %u\n",fs_g_status);
		//dbg_out(str1);
		nav_exit();
		
		sys.log.fs_chg = false;
		
		return false;
	}
	
	return true;
}

bool StartTransfer(void)
{
	char dstr[30] = "";
	
	if(!file_open(FOPEN_MODE_R))
	{
		sprintf(str1,"Failed to Open File %u\n",fs_g_status);
		dbg_out(str1);
		return false;
	}
	else
	{
		dbg_out("File Opened\n");
		sys.tfr.bleft = nav_file_lgt();
		
		if(!nav_file_lgt()) 
		{
			nav_file_del(false);
			return false;
		}
		sys.tfr.fsize = sys.tfr.bleft;
		
		sys.tfr.npkts = sys.tfr.fsize/RF_PACKET_SIZE;
		if(sys.tfr.fsize%RF_PACKET_SIZE != 0)
		{
			sys.tfr.npkts++;
		}
		
		sys.tfr.rmdbytes = sys.tfr.fsize%(CHUNK_SIZE*RF_PACKET_SIZE);
		
		sys.tfr.nchunks = sys.tfr.npkts/CHUNK_SIZE;
		if(sys.tfr.npkts%CHUNK_SIZE != 0)
		{
			sys.tfr.nchunks++;
		}
	}
	
	char pstr1[100];
	
	nav_file_getname(pstr1,100);
	sprintf(str1,"PSTR:%s",pstr1);
	dbg_out(str1);
	
	char yr_str[5];
	char mn_str[3];
	char dy_str[3];
	
	yr_str[0] = pstr1[0];
	yr_str[1] = pstr1[1];
	yr_str[2] = pstr1[2];
	yr_str[3] = pstr1[3];
	yr_str[4] = 0;
	mn_str[0] = pstr1[5];
	mn_str[1] = pstr1[6];
	mn_str[2] = 0;
	dy_str[0] = pstr1[8];
	dy_str[1] = pstr1[9];
	dy_str[2] = 0;
	
	sprintf(dstr,"data/%s/%s/%s",yr_str,mn_str,dy_str);
	
	dbg_out("Create at dir: ");
	dbg_out(dstr);
	
	dbg_out("Send Start Transfer Request\n");
	
	if(!SendStartTransfer(dstr,pstr1,sys.tfr.fsize))
	{
		dbg_out("Start Transfer Failed\n");
		return false;
	}
	
	dbg_out("Transfer Started\n");
	return true;
}

uint8_t SendChunk(uint32_t csize)
{
	XB_API_FRAME_t rxfrm;
	
	uint32_t bt_left;
	
	uint8_t c[CHUNK_SIZE*RF_PACKET_SIZE];

	uint16_t i = 0;
	uint16_t j = 0;
	
	uint8_t		last_pkt	= 0;
	uint8_t 	last_size	= 0;
	uint8_t 	c_num 		= 0;
	uint8_t 	max_retries = 2;
	
	
	i = 0;
	
	
	

	if(!file_read_buf(c,csize))
	{
		return false;
	}

	uint8_t pnum = 0;
	
	i 			= 0;
	pnum 		= 0;
	last_pkt 	= 0;
	last_size	= 0;
	c_num 		= 0;
		
	bt_left = csize;
	
	while(bt_left)
	{
		if(bt_left >= RF_PACKET_SIZE)
		{
			SendDataPacket(&(c[i*RF_PACKET_SIZE]),i,RF_PACKET_SIZE);

			last_size 	= 	RF_PACKET_SIZE;
			bt_left		-= 	RF_PACKET_SIZE;
			last_pkt 	= 	i;
			i++;
		}
		else
		{

			SendDataPacket(&(c[i*RF_PACKET_SIZE]),i,bt_left);
			
			last_size	= bt_left;
			last_pkt 	= i;
			bt_left 	= 0;
			i++;
		}
	}
	
	
	

	
	while(max_retries--)
	{
		/*
		// Clear out buffer
		while(!xb_get_frame(&rxfrm,100));
	
		// Send Chunk Fin message
		tx.valid = 0x7E;
		tx.id = 0x10;
		tx.data[0] = 0;
		tx.valid = 0x7E;
		tx.id = 0x10;
		tx.data[0] = 0x00; // Frame ID
	
		for(i = 1;i<13;i++)
		{
			tx.data[i]=0;
		}
	
		tx.data[13] = CMD_CHUNK_FIN;
		tx.len = 15;
	
		if(!xb_send_frame(&tx)) continue;
		*/
		
		if(!xb_get_frame(&rxfrm,2000))
		{
			continue;
		}
		else
		{
			if((rxfrm.valid == 0x7E) && (rxfrm.id == 0x90) && (rxfrm.data[11] = CMD_CHUNK_RESP))
			{				
				if(rxfrm.data[12] == 0)
				{
					for(j = 0;j<csize;j++)
					{
						sys.tfr.chksum -= c[j];
					}
					
					return true;
				}
				else
				{
					uint8_t num_lost	= rxfrm.data[12];
					uint8_t pkt_num 	= 0;
					
					for(i = 0;i<num_lost;i++)
					{
						pkt_num = rxfrm.data[13+i];

						if(pkt_num == last_pkt)
						{
							SendDataPacket(&(c[last_pkt*RF_PACKET_SIZE]),last_pkt,last_size);
						}
						else
						{
							SendDataPacket(&(c[pkt_num*RF_PACKET_SIZE]),pkt_num,RF_PACKET_SIZE);
						}
					}
				}
			}
		}

		
	}
	
	return false;
}

bool SendStartTransfer(char *dir_str,char *file_name,uint32_t fsize)
{
	XB_API_FRAME_t tx,rx;
	uint32_t i;
	
	for(i=0;i<3;i++)	xb_get_frame(&rx,XB_RX_NO_BLOCK);
	
	sprintf(fstr,"%s/%s",dir_str,file_name);
	
	dbg_out("Sending File: ");
	dbg_out(fstr);
	dbg_out("\n");
	
	tx.valid	= 0x7E;
	tx.id 		= 0x10;
	tx.data[0] 	= 0x01;
	
	for(i = 1;i<13;i++)
	{
		tx.data[i] = 0;
	}
	
	tx.data[13] = CMD_START_TFR;
	tx.data[14] = (fsize >> 24) & 0x000000FF;
	tx.data[15] = (fsize >> 16) & 0x000000FF;
	tx.data[16] = (fsize >> 8) & 0x000000FF;
	tx.data[17] = (fsize) & 0x000000FF;

	sprintf(str1,"Number of Bytes: %lu\n",fsize);
	dbg_out(str1);

	tx.data[18] = RF_PACKET_SIZE;
	tx.data[19] = CHUNK_SIZE;
	tx.data[20] = strlen(fstr);
	
	for(i = 21;i<strlen(fstr)+21;i++)
	{
		tx.data[i] = fstr[i-21];
	}
	tx.len = 22 + strlen(fstr);
	
	while(!xb_send_frame(&tx));
	
	// Wait for Ack
	if(!xb_get_frame(&rx,2000))
	{
		dbg_out("Start Transfer Ack Timeout\n");
		return false;
	}
	


	if((rx.valid == 0x7E) && (rx.id == 0x8B) && (rx.data[0]==0x01) && (rx.data[4]==0))
	{
		dbg_out("Start Transfer Ack Received\n");
	}
	else
	{
		dbg_out("Start Transfer Ack Failure\n");
		return false;
	}
	
	// Wait for Ack
	if(!xb_get_frame(&rx,5000))
	{
		dbg_out("Start Transfer Response Failed\n");
		return false;
	}
	
	if((rx.valid == 0x7E) && (rx.id == 0x90) && (rx.data[11] = 0x18) && (rx.data[12] == 1))
	{
		dbg_out("Start Response Received\n");
		return true;
	}
	else
	{
		dbg_out("Start Response Failure\n");
		return false;
	}
}

uint8_t SendDataPacket(uint8_t *b,uint8_t pnum,uint8_t psize)
{
	XB_API_FRAME_t tx,rx;
	
	uint8_t i;
	
	// Delimiter
	tx.valid = 0x7E;
	// Api Command ID
	tx.id = 0x10;
	// Frame ID
	tx.data[0] = 0x2;
	
	// Send to Coordinator, no options on
	for(i = 1;i<13;i++)
	{
		tx.data[i]=0;
	}

	tx.data[13] = CMD_DATA_PKT;
	tx.data[14] = pnum;
	
	//sprintf(str3,"PKT[%02u] ",pnum);dbg_out(str3);
	
	for(i = 0;i<(psize);i++) 
	{
		tx.data[i+15] = b[i];
	}
	//dbg_out("\n");
	
	tx.len = 16+psize;
	
	if(!xb_send_frame(&tx))
	{
		return false;
	}
	
	if(!xb_get_frame(&rx,100)) 
	{
		//dbg_out("No Ack Received\n");
		return false;
	}
	
	if(rx.id != 0x8B) // Look for Ack Response Frame
	{
		//dbg_out("Incorrect Ack Frame\n");
		//TODO: Need to find a way to handle this
		return false;
	}
	
	if(rx.data[4]!=0)
	{
		//dbg_out("Ack Failure\n");
		return false;
	}
	
	//dbg_out("Ack Received\n");
	
	//delay_ms(10);
	
	
	return true;
}

uint8_t EndTransfer(uint8_t rsn,uint8_t fl_chksum)
{
	XB_API_FRAME_t tx,rx;

	uint8_t i;

	// Delimiter
	tx.valid = 0x7E;
	// Api Command ID
	tx.id = 0x10;
	// Frame ID
	tx.data[0] = 0x01;

	// Send to Coordinator, no options on
	for(i = 1;i<13;i++)
	{
		tx.data[i]=0;
	}

	tx.data[13] = CMD_END_TFR;
	tx.data[14] = rsn;
	tx.data[15] = fl_chksum;
	tx.len = 17;
	
	xb_send_frame(&tx);
	
	if(!xb_get_frame(&rx,2000))
	{
		dbg_out("End Tfr Ack Timeout\n");
		return false;
	}
	

	
	if(rx.id != 0x8B) // Look for Ack Response Frame
	{
		dbg_out("End Tfr Failure: Wrong XB Frame Received\n");
		//TODO: Need to find a way to handle this
		return false;
	}
	
	if(rx.data[4]!=0)
	{
		dbg_out("End Tfr Failure: No Ack Recieved\n");
		return false;
	}
	
	if(!xb_get_frame(&rx,5000))
	{
		dbg_out("End Tranfser Response Timeout\n");
		return false;
	}
	
	if(rx.data[11] != CMD_END_TFR_RESP)
	{
		sprintf(str1,"Wrong Response Sent: 0x%02X\n",rx.data[13]);dbg_out(str1);
		dbg_out(str1);
		return false;
	}
	
	if(rx.data[12]==0)
	{
		dbg_out("File Transfer Failed\n");
		
		return false;
	}
	else if(rx.data[12]==3)
	{
		dbg_out("File Transfer Checksum Failure\n");
		return false;
	}
	else if(rx.data[12]==2)
	{
		dbg_out("File Transfer Canceled\n");
		return true;
	}
	else if(rx.data[12]==1)
	{
		dbg_out("File Transfer Finished\n");
		return true;
	}
	else
	{
		dbg_out("Unknown Response\n");
		return false;
	}
	return false;
}

uint8_t CancelTransfer(void)
{
	dbg_out("Canceling Transfer...\n");
	return(EndTransfer(2,0));
}
