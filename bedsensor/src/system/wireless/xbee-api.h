/*
 * xbee_api.h
 *
 * Created: 10/8/2012 5:33:16 AM
 *  Author: Administrator
 */ 

#ifndef XBEE_API_H_
#define XBEE_API_H_

#include <asf.h>
#include <usart.h>
#include <user_board.h>
#include <usart.h>
#include <usart_serial.h>
#include <delay.h>

#define XB_USART AVR32_USART0
#define XB_BAUDRATE	115200
#define XB_RTS AVR32_PIN_PA03
#define XB_CTS AVR32_PIN_PA04

#define XB_USART_TX_TIMEOUT 10 // 10ms
#define XB_USART_RX_TIMEOUT 5 // 5ms

#define XB_USART_RX_PIN			AVR32_USART0_RXD_0_0_PIN
#define XB_USART_RX_FUNCTION	AVR32_USART0_RXD_0_0_FUNCTION
#define XB_USART_TX_PIN			AVR32_USART0_TXD_0_0_PIN
#define XB_USART_TX_FUNCTION	AVR32_USART0_TXD_0_0_FUNCTION
#define XB_USART_CLOCK_MASK     AVR32_USART0_CLK_PBA
#define XB_PDCA_CLOCK_HSB       AVR32_PDCA_CLK_HSB
#define XB_PDCA_CLOCK_PB        AVR32_PDCA_CLK_PBA
#define XB_USART_IRQ			AVR32_USART0_IRQ

#define XB_RX_BLOCK 0xFFFF
#define XB_RX_NO_BLOCK 0x0000

#define xb_clr2send() gpio_pin_is_low(XB_CTS)
#define xb_rts_enb() gpio_set_pin_low(XB_RTS)
#define xb_rts_dsb() gpio_set_pin_high(XB_RTS)

typedef uint8_t XB_MDM_STS_t;	// Modem Status

#define	WAIT_FOR_DELIM 0
#define GET_LEN_MSB 1
#define	GET_LEN_LSB 2
#define	GET_DATA 3
#define	CALC_CHK_SUM 4
#define BFR_FULL 5 
#define	FRAME_VALID 0x7E
#define	FRAME_INVALID 0xFF
#define MAX_FRAME_LENGTH 86
#define MAX_RF_PACKET_SIZE 72

void xb_usart_init(void);

uint8_t wifi_read(int *r);
uint8_t wifi_empty(void);

#endif /* XBEE-API_H_ */