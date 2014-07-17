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


#define WIFI_BFR_SIZE 256

typedef struct 
{
	int bfr[WIFI_BFR_SIZE];
	uint8_t head;
	uint8_t tail;	
}XB_WIFI_BFR_t;

volatile uint8_t tx_lock = 0;

volatile XB_WIFI_BFR_t wifi;

__attribute__((__interrupt__)) static void xb_rx_irq(void)
{
	int d = 0;
	
	d = usart_getchar(&XB_USART);
	
	if(d != USART_FAILURE)
	{
		wifi.bfr[wifi.head] = d;
		wifi.head++;		
	}
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
		wifi.tail++;
		*r = wifi.bfr[wifi.head-1];
		cpu_irq_enable();
		return 1;
	}
}