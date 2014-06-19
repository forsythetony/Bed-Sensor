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

#define XB_USART AVR32_USART0
#define XB_BAUDRATE	115200
#define XB_RTS AVR32_PIN_PA03
#define XB_CTS AVR32_PIN_PA04

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

// API Command Identifier
enum XB_API_ID
{
	NULL_ID=	0x00,	// Null ID (Invalid)
	MDM_STS=	0x8A,	// Modem Status
	ADV_STS=	0x8C,	// Advanced Modem Status
	AT_CMD=		0x08,	// AT Command
	AT_CMD_QPV=	0x09,	// AT Command - Queue Parameter Value
	AT_CMD_RSP=	0x88,	// AT Command Response
	TX_RQST=	0x10,	// Transmit Request
	TX_EXP_ADDR=0x11,	// Transmit with Explicit Addressing
	BND_TBL=	0x12,	// Binding Table
	TX_STS=		0x8B,	// Transmit Status
	RX_PKT=		0x90,	// Received RF Packet
	RX_EXP=		0x91,	// Explicit RF PAcket
	BND_RX_IND=	0x92	// Binding RX Indicator
};

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


typedef struct XB_API_FRAME_t
{
	uint8_t valid;
	uint16_t len;
	uint8_t id;
	uint8_t data[MAX_FRAME_LENGTH-1];
	uint8_t chk_sum;	
}XB_API_FRAME_t;

typedef union 
{
	XB_API_FRAME_t api_frm;
	struct  
	{
		uint8_t valid;
		uint16_t len;
		uint8_t api_id;
		uint8_t frm_id;
		uint64_t zb_addr;
		uint16_t ntw_addr;
		uint8_t brdcst_rds;
		uint8_t opts;
		uint8_t data[RF_PACKET_SIZE];
		uint8_t chk_sum;
	}rf_frm;
}XB_TX_PKT_t;

#define XB_USART_TX_TIMEOUT 10 // 10ms
#define XB_USART_RX_TIMEOUT 5 // 5ms

void xb_usart_init(void);
uint8_t xb_write(uint8_t w);
uint8_t xb_read(uint8_t *r);
uint8_t xb_get_frame(XB_API_FRAME_t *frm,uint16_t tm_out);
uint8_t xb_send_frame(XB_API_FRAME_t *frm);
uint8_t xb_send_rf_packet(uint64_t zb_addr,uint16_t ntw_addr,uint32_t,uint16_t tm_out);
void xb_calc_chksum(XB_API_FRAME_t *frm);
uint8_t xb_verify_chksum(XB_API_FRAME_t *frm);

/* Call back function (define in user code)*/
uint8_t xb_callback(XB_API_FRAME_t *frm);
uint8_t wifi_read(int *r);
uint8_t wifi_empty(void);

#endif /* XBEE-API_H_ */