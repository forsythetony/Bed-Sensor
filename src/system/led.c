/*
 * led.c
 *
 * Created: 11/5/2012 6:56:39 AM
 *  Author: MizzouRacing
 */ 

#include "led.h"


volatile struct
{
	uint32_t pin;
	uint8_t tmr_mode;
	t_cpu_time on_tmr;
	uint8_t blink_mode;
	uint16_t blink_period;
	t_cpu_time blink_tmr;
}led[NUM_LEDS];

void ChangeBlink(uint8_t idx,uint16_t per)
{
	led[idx].blink_period = per;
}

void InitLed(uint8_t idx,uint32_t pin)
{
	if(idx < NUM_LEDS)
	{
		led[idx].pin = pin;
		led[idx].blink_mode = LED_OFF;
		led[idx].tmr_mode = 0;
		gpio_configure_pin(pin,GPIO_DIR_OUTPUT|GPIO_INIT_LOW);
	}

}

void LedCtrl(uint8_t idx,uint8_t tmr_mode, uint16_t tmr_msec,uint8_t blink_mode,uint16_t blink_msec)
{
	if(idx < NUM_LEDS)
	{
		if((led[idx].blink_mode != blink_mode) || (led[idx].tmr_mode != tmr_mode) || (led[idx].blink_period != blink_msec))
		{
		
			cpu_irq_disable();
			
			led[idx].blink_mode = blink_mode;
			led[idx].tmr_mode = tmr_mode;
			led[idx].blink_period = blink_msec;
			
			
			if(tmr_mode == LED_TIMED)
			{
				cpu_set_timeout(cpu_ms_2_cy(tmr_msec,F_CPU),&(led[idx].on_tmr));
			}
			
			if(blink_mode == LED_BLINK)
			{
				cpu_set_timeout(cpu_ms_2_cy(blink_msec,F_CPU),&(led[idx].blink_tmr));
			}
			
			if(blink_mode != LED_OFF)	gpio_set_pin_high(led[idx].pin);
			else
			{
				gpio_set_pin_low(led[idx].pin);
			}
			cpu_irq_enable();
		}		
	}	
}

void HandleLeds(void)
{
	uint8_t i;
	
	for(i = 0;i < NUM_LEDS;i++)
	{
		
		if(led[i].tmr_mode == LED_TIMED)
		{
			if(cpu_is_timeout(&(led[i].on_tmr)))
			{
				led[i].blink_mode = LED_OFF;
				led[i].tmr_mode = LED_STAY_ON;
			}
		}
		
		
		if(led[i].tmr_mode == LED_PULSE)
		{
			led[i].blink_mode = LED_OFF;
			led[i].tmr_mode = LED_STAY_ON;
			continue;
		}
		
		if(led[i].blink_mode == LED_BLINK)
		{
			if(cpu_is_timeout(&(led[i].blink_tmr)))
			{
				gpio_tgl_gpio_pin(led[i].pin);
				
				cpu_set_timeout(cpu_ms_2_cy(led[i].blink_period,F_CPU),&(led[i].blink_tmr));
			}
			continue;
		}
		
		if(led[i].blink_mode == LED_ON)
		{
			gpio_set_pin_high(led[i].pin);
			continue;
		}
		
		if(led[i].blink_mode == LED_OFF)
		{
			gpio_set_pin_low(led[i].pin);
			continue;
		}
	}		
}
