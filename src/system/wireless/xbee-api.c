/*
 * xbee_api.c
 *
 * Created: 10/8/2012 5:33:00 AM
 *  Author: Administrator
 */ 

#include "xbee-api.h"
#include <intc.h>
#include <sysclk.h>
#include <gpio.h>
#include <string.h>
#include "system/led.h"

#define BFR_SIZE 10
#define WIFI_BFR_SIZE 256

// XBee RX Buffer Structure
typedef struct 
{
	XB_API_FRAME_t frm[BFR_SIZE];
	uint8_t state;
	uint16_t idx;
	uint8_t full;
	uint8_t head;
	uint8_t tail;
	uint8_t over;
	uint8_t empty;
}XB_RX_BFR_t;

typedef struct 
{
	int bfr[WIFI_BFR_SIZE];
	uint8_t head;
	uint8_t tail;	
}XB_WIFI_BFR_t; 

volatile uint8_t tx_lock = 0;

volatile XB_RX_BFR_t rx;
volatile XB_API_FRAME_t dummy;
volatile XB_WIFI_BFR_t wifi;

volatile uint16_t rx_cnt = 0;
/*
__attribute__((__interrupt__)) static void xb_rx_irq(void)
{
	uint8_t d = 0;
	uint8_t pull_frame = 0;
	
	if(xb_read(&d))
	{
		// Increment Message Counter
		rx_cnt++;
		
		// XBee receive State Machine		
		switch (rx.state)
		{
			case WAIT_FOR_DELIM:	
				// Wait until delimiter is received (0x7E) This indicates the start of a frame
				if(d == 0x7E)	rx.state = GET_LEN_MSB;				
				break;

			case GET_LEN_MSB:
				// The next byte should be the MSB of message length
				rx.frm[rx.head].len = (((uint16_t)d) << 8) & 0xFF00;
				rx.state = GET_LEN_LSB;
				break;
				
			case GET_LEN_LSB:
				
				// Calculate the length of the message from MSB and LSB
				rx.frm[rx.head].len += (((uint16_t)d) & 0x00FF);
				rx.idx = 0;	//Initialize Message index
				rx.state = GET_DATA;
				break;
				
			case GET_DATA:
				
				// Write the first data byte to the frame id
				if(!rx.idx) rx.frm[rx.head].id = d;
				// Write subsequent bytes to the frame data array
				else		rx.frm[rx.head].data[rx.idx-1] = d;
				
				// Increment the data index
				rx.idx++;
				
				// When length of message is reached, move on
				if(rx.idx == rx.frm[rx.head].len ) rx.state = CALC_CHK_SUM;
				break;
				
			case CALC_CHK_SUM:
				// Sum the message content
				rx.frm[rx.head].chk_sum = rx.frm[rx.head].id + d;
				for(rx.idx = 0;rx.idx < rx.frm[rx.head].len-1;rx.idx++)
				{
					rx.frm[rx.head].chk_sum += rx.frm[rx.head].data[rx.idx];
				}
				
				// Compare to check sum
				if(rx.frm[rx.head].chk_sum == 0xFF) // Frame Valid
				{
					rx.frm[rx.head].valid = FRAME_VALID;
					
					// Pulse the COM Led
					//LedPulse(2);
					
					 Command Handling 
					if(rx.frm[rx.head].id == RX_PKT)
					{	
						if(xb_callback(&(rx.frm[rx.head]))) 
						{
							uint8_t pull_frame = 1;
						}							
					}						
						
				}
				else		// Frame Invalid
				{
					rx.frm[rx.head].valid = FRAME_INVALID;
				}				
				
				// Increment the head of the buffer (Circular)
				rx.head = (rx.head+1) % BFR_SIZE; 
				
				// Clear buffer empty flag
				//rx.empty = 0 // Do this elsewhere
				
				// Check next (Circular) buffer position
				if(((rx.head + 1) % BFR_SIZE) == rx.tail)
				{
					// If the head is one spot behind the tail, consider the buffer full.
					//rx.full = 1; Do this check elsewhere
					
					rx.state = BFR_FULL;
					
					// Tell Xbee to stop sending data
					xb_rts_dsb();
					LedOn(1);
				}
				else
				{
					rx.state = WAIT_FOR_DELIM;
				}				
				
				// If frame was utilized by the callback, pull it from the queue
				if(pull_frame == 1)
				{
					xb_get_frame(&dummy,XB_RX_NO_BLOCK);
				}
				
				return;
				
			case BFR_FULL:
				if(!(((rx.head + 1) % BFR_SIZE) == rx.tail))
				{
					// If the head is one spot behind the tail, consider the buffer full.
					rx.full = 0;
	
					rx.state = WAIT_FOR_DELIM;

					// Tell Xbee to start sending data
					xb_rts_enb();
					LedOff(1);
				}

				return;
			default:
				break;
		}
	}
}
*/

__attribute__((__interrupt__)) static void xb_rx_irq(void)
{
	int d = 0;
	
	d = usart_getchar(&XB_USART);
	
	if(d != USART_FAILURE)
	{
		//LedPulse(2);
		wifi.bfr[wifi.head] = d;
		wifi.head++;		
	}
}			
uint8_t xb_get_frame(XB_API_FRAME_t *frm,uint16_t tm_out)
{
	t_cpu_time tmr;
	cpu_set_timeout(cpu_ms_2_cy(tm_out,F_CPU),&tmr);
	

	
	do
	{
		// This check should help prevent deadlock
		XB_USART.ier &= ~(AVR32_USART_IER_RXRDY_MASK);
		if(rx.tail == rx.head) 
		{
			rx.empty = 1;
		}
		else rx.empty = 0;		
		if(((rx.head + 1) % BFR_SIZE)!=(rx.tail))	
		{
			rx.full = 0;
		}		
		XB_USART.ier |= AVR32_USART_IER_RXRDY_MASK;
		// if there is anything in the buffer
		if(!rx.empty)
		{
			//Make this block "atomic"
			XB_USART.ier &= ~(AVR32_USART_IER_RXRDY_MASK);
			
			memcpy((void*)frm,(void*)(&rx.frm[rx.tail]),sizeof(XB_API_FRAME_t));
			
			rx.tail = (rx.tail+1)%BFR_SIZE;
			
			// Put the buffer back into a good state if it was full
			if (rx.state == BFR_FULL)
			{
				rx.state == WAIT_FOR_DELIM;
			}
			
			rx.full = 0;

			XB_USART.ier |= AVR32_USART_IER_RXRDY_MASK;
			
			return 1;
		}
	}while(((!cpu_is_timeout(&tmr)) || (tm_out == XB_RX_BLOCK)) && (tm_out != XB_RX_NO_BLOCK));
	
	return 0;
}

uint8_t xb_write(uint8_t w)
{
	t_cpu_time tmr;
	
	cpu_set_timeout(cpu_ms_2_cy(XB_USART_TX_TIMEOUT,F_CPU),&tmr);
	
	do
	{
		//if(xb_clr2send())
		if(true)
		{
			if(usart_write_char(&XB_USART,(int)w)==USART_SUCCESS) return 1;
		}
	} 
	while (!cpu_is_timeout(&tmr));
	
	return 0;
}

uint8_t xb_read(uint8_t *r)
{
	t_cpu_time tmr;
	int r_i = 0;
	
	cpu_set_timeout(cpu_ms_2_cy(XB_USART_RX_TIMEOUT,F_CPU),&tmr);
	
	do 
	{
		if(usart_read_char(&XB_USART,&r_i) == USART_SUCCESS)
		{
			*r = (uint8_t)r_i;
			return 1;
		}
	} while (!cpu_is_timeout(&tmr));
	
	return 0;
}

uint8_t wifi_empty(void)
{
	if(wifi.head == wifi.tail) 
	{
		return 1;
	}		
	else
	{
		return 0;
	}
	
}

void xb_usart_init(void)
{
	int c;
	//gpio_configure_pin(XB_RTS,GPIO_DIR_OUTPUT|GPIO_INIT_HIGH);
	gpio_configure_pin(XB_CTS,GPIO_DIR_INPUT|GPIO_PULL_UP);
	gpio_configure_pin(AVR32_PIN_PA05,GPIO_DIR_INPUT);
	
	gpio_map_t XB_USART_GPIO_MAP =
	{
	{XB_USART_RX_PIN, XB_USART_RX_FUNCTION},
	{XB_USART_TX_PIN, XB_USART_TX_FUNCTION},
	};
	
	usart_serial_options_t XB_USART_OPTIONS =
	{
		.baudrate     = XB_BAUDRATE,
		.charlength   = 8,
		.paritytype   = USART_NO_PARITY,
		.stopbits     = USART_1_STOPBIT,
		.channelmode  = USART_NORMAL_CHMODE
	};

	gpio_enable_module(XB_USART_GPIO_MAP, sizeof(XB_USART_GPIO_MAP) / sizeof(XB_USART_GPIO_MAP[0]));
	usart_serial_init(&XB_USART,&XB_USART_OPTIONS);
	
	// Init RX Buffer
	rx.idx = 0;
	rx.head = 0;
	rx.tail = 0;
	rx.full = 0;
	for(c = 0;c<BFR_SIZE;c++)
	{
		rx.frm[c].valid = FRAME_INVALID;
	}
	rx.state = WAIT_FOR_DELIM;
	rx.over = 0;
	rx.empty = 1;
	tx_lock = 0;
	
	//Clear out usart
	//usart_read_char(&XB_USART,&c);
	
	LedOn(0);
	gpio_configure_pin(AVR32_PIN_PA05,GPIO_DIR_OUTPUT|GPIO_INIT_LOW);
	delay_ms(500);
	gpio_configure_pin(AVR32_PIN_PA05,GPIO_DIR_INPUT);
	delay_ms(2000);
	LedOff(0);
	INTC_register_interrupt(&xb_rx_irq, XB_USART_IRQ,AVR32_INTC_INT1);

	// Enable USART Rx interrupt.
	XB_USART.ier = AVR32_USART_IER_RXRDY_MASK;

}

uint8_t wifi_read(int *r)
{
	if(wifi.head==wifi.tail)
	{
		return 0;
	}
	else
	{
		cpu_irq_disable();
		//LedPulse(2);
		wifi.tail++;
		*r = wifi.bfr[wifi.head-1];
		cpu_irq_enable();
		return 1;
	}
}

void xb_calc_chksum(XB_API_FRAME_t *frm)
{
	uint8_t i;
	
	frm->chk_sum = 0xFF;
	
	for(i = 0;i<(frm->len-1);i++)	frm->chk_sum -= frm->data[i];
	
	frm->chk_sum -= frm->id;
}

uint8_t xb_verify_chksum(XB_API_FRAME_t *frm)
{
	uint8_t i;
	
	uint8_t cs = frm->chk_sum;
	
	for(i = 0;i<frm->len-1;i++)	cs += frm->data[i];
	
	cs += frm->id;
	
	if(cs == 0xFF) return 1;
	else
	{
			return 0;
	}
}

uint8_t xb_send_frame(XB_API_FRAME_t *frm)
{
	uint8_t *ptr;
	uint8_t i;
	uint8_t tx_lock_taken = 0;
	
	cpu_irq_disable();
	if(tx_lock == 1)
	{
		cpu_irq_enable();
		goto XB_SEND_FRAME_FAIL;
	}
	
	tx_lock = 1;
	tx_lock_taken = 1;
	cpu_irq_enable();
	
	xb_calc_chksum(frm);

	if(!xb_write(0x7E)) goto XB_SEND_FRAME_FAIL;
	if(!xb_write((frm->len) >> 8)) goto XB_SEND_FRAME_FAIL;
	if(!xb_write((frm->len) & 0x00FF)) goto XB_SEND_FRAME_FAIL;
	if(!xb_write((frm->id))) goto XB_SEND_FRAME_FAIL;
	for(i = 0;i<((frm->len)-1);i++)
	{
		if(!xb_write(frm->data[i]))	goto XB_SEND_FRAME_FAIL;
	}
	if(!xb_write(frm->chk_sum)) goto XB_SEND_FRAME_FAIL;
	
	//LedPulse(2);
	
	tx_lock = 0;		
	return 1;
	
	XB_SEND_FRAME_FAIL:
	if(tx_lock_taken == 1) tx_lock = 0;
	return 0;
}