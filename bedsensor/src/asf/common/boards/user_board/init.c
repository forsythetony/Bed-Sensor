/**
 * \file
 *
 * \brief User board initialization template
 *
 */

#include <board.h>
#include <asf.h>
#include <conf_board.h>
#include <conf_usart_serial.h>
#include <conf_clock.h>
#include <stdio.h>
#include <compiler.h>
#include <string.h>
#if defined (__GNUC__)
#  include "intc.h"
#endif
#include "system/buttons.h"
#include "system/led.h"
#include "main.h"
#include "system/wireless/xbee-api.h"
#include <tc.h>

uint8_t curr_data = 0;
uint16_t data_idx = 0;
volatile uint16_t data_rdy = 0;

unsigned long glb_time_msec = 0;
unsigned long glb_time_sec = 0;
volatile uint8_t time_valid = 0;

volatile uint16_t test_val = 0;
volatile uint32_t val_avg = 0;
volatile uint32_t val_avg_hold = 0;

volatile uint16_t val_avgs[100];
volatile uint8_t avg_idx = 0;
volatile uint16_t avg_valid = 0;
volatile uint16_t status_cntr = 0;

volatile uint8_t b_year=0;
volatile uint8_t b_lpyear =0;
volatile uint8_t b_month=0;
volatile uint8_t b_day=0;
volatile uint8_t b_hour=0;
volatile uint8_t b_min=0;
volatile uint8_t b_sec=0;
volatile uint16_t b_msec=0;
volatile uint8_t timestamp_valid = 0;
volatile uint8_t temp_block = 0;
volatile uint8_t send_ping_ack = 0;
volatile uint32_t ping_timeout = 0;

volatile uint8_t sample_enable = 0;
#define PING_TIMEOUT 15000
// PING_TIMEOUT example) 15000 * 10 / 1000 = 150 seconds for before timeout 

/**
 * \brief TC interrupt.
 *
 * The ISR handles RC compare interrupt and sets the update_timer flag to
 * update the timer value.
 */

uint32_t simple_time = 0;
uint32_t smpl_cntr = 0;
bool resetNotificationSent = false;


#define SEND_INTERVAL_IN_MS 20
#define MAX_QUEUE_SIZE 10
#define MAX_SAMPLE_CHUNK_SIZE 1500

volatile char out_str[MAX_SAMPLE_CHUNK_SIZE];
volatile bool startSample = true;

typedef struct
{
	char buffer[MAX_QUEUE_SIZE][MAX_SAMPLE_CHUNK_SIZE];
	uint8_t head;
	uint8_t tail;
}XB_DATA_BFR_t;

volatile XB_DATA_BFR_t dataToSend;

__attribute__((__interrupt__)) static void tc_irq(void)
{
	
	bool sendSample = false;
	
	smpl_cntr++;
	
	ping_timeout++;
	
	
	if(ping_timeout>=PING_TIMEOUT)	
	{
		cpu_irq_disable();
		SoftwareReset();
	}
	else
	{
		ChangeBlink(0,ping_timeout/20+20);
	}
	
	if(timestamp_valid)
	{
		b_msec+=10;
		
		if (b_msec % SEND_INTERVAL_IN_MS == 0) {
			sendSample = true;
		}
		if(b_msec==1000)
		{
			add_second_to_date();
			b_msec = 0;
		}	
	}		
	
	// Clear the interrupt flag. This is a side effect of reading the TC SR.
	tc_read_sr(EXAMPLE_TC, EXAMPLE_TC_CHANNEL);
	
	HandleButtons();
	HandleLeds();
	
	if(!wifi_empty()) return;
	
	if(sample_enable == 1 && temp_block != 1) {
		if (startSample == true) {
			memset((char *)out_str, 0, sizeof(out_str));	
		}
		startSample = false;

		data_sample[0]= (adc_sample(ADC_CH0)>>1);
		data_sample[1]= (adc_sample(ADC_CH1)>>1);
		data_sample[2]= (adc_sample(ADC_CH2)>>1);
		data_sample[3]= (adc_sample(ADC_CH3)>>1);
		data_sample[4]= (adc_sample(ADC_CH4)>>1);
		data_sample[5]= (adc_sample(ADC_CH5)>>1);
		data_sample[6]= (adc_sample(ADC_CH6)>>1);
		data_sample[7]= (adc_sample(ADC_CH7)>>1);
	
		sprintf((char *)&out_str[strlen((const char*)out_str)],"<S:%02u-%02u-%02u_%02u:%02u:%02u.%03u,%u,%u,%u,%u,%u,%u,%u,%lu#",
								b_month,b_day,b_year,b_hour,b_min,b_sec,b_msec,
								data_sample[R1],data_sample[R2],data_sample[R3],data_sample[R4],
								data_sample[F1],data_sample[F2],data_sample[F3],ping_timeout);	
	}
	
	if(sample_enable == 1 && temp_block != 1 && sendSample)
	{
		strcpy((char *)dataToSend.buffer[dataToSend.tail], (const char *)out_str);
		dataToSend.tail++;
        if (dataToSend.tail >= MAX_QUEUE_SIZE) {
	        dataToSend.tail = 0;
        }	
		startSample = true;
	}	
	
	/*
	if (smpl_cntr > 10000 && resetNotificationSent == false) {
		usart_write_line(&XB_USART,"<Resetting soon\n\r");
		resetNotificationSent = true;
		//SoftwareReset();
	}
	if (smpl_cntr > 12000) {
		usart_write_line(&XB_USART,"<RESET\n\r");
		SoftwareReset();
		smpl_cntr = 0;
	}
	*/
	
} 


void send_data() {

	char targetString[MAX_SAMPLE_CHUNK_SIZE+2];
	char sendString[20][MAX_SAMPLE_CHUNK_SIZE];
	volatile int sendIdx = 0;
	

	//if(!xb_clr2send())
	
	cpu_irq_disable();
	
	while (dataToSend.head != dataToSend.tail) {
		strcpy(sendString[sendIdx], (const char*)dataToSend.buffer[dataToSend.head]);
		dataToSend.head++;
		if (dataToSend.head >= MAX_QUEUE_SIZE) {
			dataToSend.head = 0;
		}
		sendIdx++;
	}
	cpu_irq_enable();
	
	if (send_ping_ack == 1) {
		strcpy(sendString[sendIdx], "<PA");
		sendIdx++;
		send_ping_ack = 0;	
	}

	for (int idx=0; idx<sendIdx; idx++) {
		sprintf(targetString, "%s\n\r", sendString[idx]);
		usart_write_line(&XB_USART, targetString);
	}

	return;
}


static void tc_init(volatile avr32_tc_t *tc);

static void tc_init(volatile avr32_tc_t *tc)
{
	// Options for waveform generation.
	static const tc_waveform_opt_t waveform_opt = {
		// Channel selection.
		.channel  = EXAMPLE_TC_CHANNEL,
		// Software trigger effect on TIOB.
		.bswtrg   = TC_EVT_EFFECT_NOOP,
		// External event effect on TIOB.
		.beevt    = TC_EVT_EFFECT_NOOP,
		// RC compare effect on TIOB.
		.bcpc     = TC_EVT_EFFECT_NOOP,
		// RB compare effect on TIOB.
		.bcpb     = TC_EVT_EFFECT_NOOP,
		// Software trigger effect on TIOA.
		.aswtrg   = TC_EVT_EFFECT_NOOP,
		// External event effect on TIOA.
		.aeevt    = TC_EVT_EFFECT_NOOP,
		// RC compare effect on TIOA.
		.acpc     = TC_EVT_EFFECT_NOOP,
		/*
		 * RA compare effect on TIOA.
		 * (other possibilities are none, set and clear).
		 */
		.acpa     = TC_EVT_EFFECT_NOOP,
		/*
		 * Waveform selection: Up mode with automatic trigger(reset)
		 * on RC compare.
		 */
		.wavsel   = TC_WAVEFORM_SEL_UP_MODE_RC_TRIGGER,
		// External event trigger enable.
		.enetrg   = false,
		// External event selection.
		.eevt     = 0,
		// External event edge selection.
		.eevtedg  = TC_SEL_NO_EDGE,
		// Counter disable when RC compare.
		.cpcdis   = false,
		// Counter clock stopped with RC compare.
		.cpcstop  = false,
		// Burst signal selection.
		.burst    = false,
		// Clock inversion.
		.clki     = false,
		// Internal source clock 3, connected to fPBA / 8.
		.tcclks   = TC_CLOCK_SOURCE_TC5
	};

	// Options for enabling TC interrupts
	static const tc_interrupt_t tc_interrupt = {
		.etrgs = 0,
		.ldrbs = 0,
		.ldras = 0,
		.cpcs  = 1, // Enable interrupt on RC compare alone
		.cpbs  = 0,
		.cpas  = 0,
		.lovrs = 0,
		.covfs = 0
	};
	
	sysclk_enable_peripheral_clock(tc);
	// Initialize the timer/counter.
	tc_init_waveform(tc, &waveform_opt);

	/*
	 * Set the compare triggers.
	 * We configure it to count every 1 milliseconds.
	 * We want: (1 / (fPBA / 8)) * RC = 1 ms, hence RC = (fPBA / 8) / 1000
	 * to get an interrupt every 10 ms.
	 */
	tc_write_rc(tc, EXAMPLE_TC_CHANNEL, 1875);
	// configure the timer interrupt
	tc_configure_interrupts(tc, EXAMPLE_TC_CHANNEL, &tc_interrupt);
	// Start the timer/counter.
	tc_start(tc, EXAMPLE_TC_CHANNEL);
}

void board_init(void)
{
	/* This function is meant to contain board-specific initialization code
	 * for, e.g., the I/O pins. The initialization can rely on application-
	 * specific board configuration, found in conf_board.h.
	 */
	
	// Initialize Debug USART
	gpio_map_t DBG_USART_GPIO_MAP =
	{
	{DBG_USART_RX_PIN, DBG_USART_RX_FUNCTION},
	{DBG_USART_TX_PIN, DBG_USART_TX_FUNCTION}
	};

	usart_serial_options_t DBG_USART_OPTIONS =
	{
		.baudrate     = 115200,
		.charlength   = 8,
		.paritytype   = USART_NO_PARITY,
		.stopbits     = USART_1_STOPBIT,
		.channelmode  = USART_NORMAL_CHMODE
	};

	gpio_enable_module(DBG_USART_GPIO_MAP,
	sizeof(DBG_USART_GPIO_MAP) / sizeof(DBG_USART_GPIO_MAP[0]));
	usart_serial_init(DBG_USART,&DBG_USART_OPTIONS);

	spi_options_t adc_spi_opts =
	{
		.reg = 0,
		.baudrate = ADC_SPI_BAUD,
		.bits = 16,
		.spck_delay = 0,
		.trans_delay = 0,
		.stay_act = 0,
		.spi_mode = 0,
		.modfdis = 0
	};
		
	spi_options_t fake_spi_opts =
	{
		.reg = 3,
		.baudrate = ADC_SPI_BAUD,
		.bits = 8,
		.spck_delay = 0,
		.trans_delay = 0,
		.stay_act = 0,
		.spi_mode = 0,
		.modfdis = 0
	};
	
	// Initialize ADC SPI Bus
	gpio_map_t ADC_SPI_GPIO_MAP ={
	{ADC_MOSI_PIN,ADC_MOSI_FUNCTION},
	{ADC_MISO_PIN,ADC_MISO_FUNCTION},
	{ADC_SCK_PIN,ADC_SCK_FUNCTION},
	{ADC_CS_PIN,ADC_CS_FUNCTION}};
			
	gpio_enable_module(ADC_SPI_GPIO_MAP,
	sizeof(ADC_SPI_GPIO_MAP) / sizeof(ADC_SPI_GPIO_MAP[0]));
		
	sysclk_enable_peripheral_clock(ADC_SPI);
	spi_selectionMode(ADC_SPI,0,0,0);
	spi_initMaster(ADC_SPI,&adc_spi_opts);
	spi_enable(ADC_SPI);
	spi_setupChipReg(ADC_SPI,&adc_spi_opts,sysclk_get_pba_hz());
	spi_setupChipReg(ADC_SPI,&fake_spi_opts,sysclk_get_pba_hz());

	// Initialize the timer module
	volatile avr32_tc_t *tc = EXAMPLE_TC;
	tc_init(tc);
	
	// Timer

	// Initialize interrupt vectors.
	INTC_init_interrupts();
	// Register the RTC interrupt handler to the interrupt controller.
	INTC_register_interrupt(&tc_irq, EXAMPLE_TC_IRQ, AVR32_INTC_INT0);
	// Enable the interrupts
	
	/************************************************************************/
	/*                        TWI Initialization                            */
	/************************************************************************/
			

	
	memset((char *)&dataToSend, 0, sizeof(dataToSend));
}
