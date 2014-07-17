/*
 * led.h
 *
 * Created: 11/5/2012 6:56:55 AM
 *  Author: MizzouRacing
 */ 


#ifndef LED_H_
#define LED_H_

#include <asf.h>
#include <compiler.h>
#include <gpio.h>
#include <cycle_counter.h>
#include <user_board.h>

#define NUM_LEDS 3

volatile struct
{
	uint32_t pin;
	uint8_t tmr_mode;
	t_cpu_time on_tmr;
	uint8_t blink_mode;
	uint16_t blink_period;
	t_cpu_time blink_tmr;
}led[NUM_LEDS];



#define LED_OFF 0
#define LED_ON	1
#define LED_BLINK 2

#define LED_STAY_ON 0
#define LED_TIMED	1
#define LED_PULSE	2

#define LedOn(_l)				LedCtrl(_l,LED_STAY_ON	,0		,LED_ON		,0)
#define LedOff(_l)				LedCtrl(_l,LED_STAY_ON	,0		,LED_OFF	,0)
#define LedBlink(_l,_b)			LedCtrl(_l,LED_STAY_ON	,0		,LED_BLINK	,_b)
#define LedPulse(_l)			LedCtrl(_l,LED_PULSE	,0		,LED_ON		,0)
#define LedOnTimed(_l,_t)		LedCtrl(_l,LED_TIMED	,(_t)	,LED_ON		,0)
#define LedBlinkTimed(_l,_b,_t)	LedCtrl(_l,LED_TIMED	,(_t)	,LED_ON		,_b)

void InitLed(uint8_t idx,uint32_t pin);
void LedCtrl(uint8_t idx,uint8_t tmr_mode, uint16_t tmr_msec,uint8_t blink_mode,uint16_t blink_msec);
void HandleLeds(void);
uint8_t LedHasChanged(uint8_t idx);
void LedClearChange(uint8_t idx);
void ChangeBlink(uint8_t idx,uint16_t per);



#endif /* LED_H_ */