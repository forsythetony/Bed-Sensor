/*
 * buttons.c
 *
 * Created: 8/15/2012 2:02:14 PM
 *  Author: Mark
 */ 

#include "buttons.h"

BUTTON_t btn[NUM_BTNS];

void HandleButtons(void)
{
	int i;
	for(i = 0;i<NUM_BTNS;i++)
	{
		if(btn[i].enable)
		{
			switch(btn[i].state)
			{
				case BTN_CLEARED:
					if(gpio_get_pin_value(btn[i].pin))
					{
						btn[i].state = BTN_READY;
					}
					break;
				case BTN_READY:
					if(!gpio_get_pin_value(btn[i].pin))
					{
						btn[i].state = BTN_PUSHED;
						btn[i].pshd = 1;
					}
					break;
				case BTN_PUSHED:
					if(gpio_get_pin_value(btn[i].pin))
					{
						btn[i].state = BTN_RELEASED;
						btn[i].rlsd = 1;
					}
					else
					{
						btn[i].time+=10;
					}
					break;
				case BTN_RELEASED:
					if(!gpio_get_pin_value(btn[i].pin))
					{
						btn[i].rlsd = 0;
						btn[i].time = 0;
						btn[i].state = BTN_PUSHED;
					}
					break;
			}
		}
	}
}

uint8_t InitButton(uint8_t idx,uint8_t pin)
{
	if(idx>=NUM_BTNS)
	{
		return 0;
	}
	
	gpio_configure_pin(pin,GPIO_DIR_INPUT|GPIO_PULL_UP);
	
	btn[idx].enable = 0;
	btn[idx].pin = pin;
	btn[idx].pshd = 0;
	btn[idx].rlsd = 0;
	btn[idx].state = BTN_CLEARED;
	btn[idx].time = 0;
	btn[idx].enable = 1;
	
	return 1;
}


uint8_t GetButtonPush(uint8_t b)
{
	btn[b].enable = 0;
	if(btn[b].pshd == 1)
	{
		
		btn[b].pshd = 0;
		btn[b].rlsd = 0;
		btn[b].time = 0;
		btn[b].state = 0;
		btn[b].enable = 1;
		return 1;
	}
	btn[b].enable = 1;
	return 0;
}

uint8_t GetButtonReleased(uint8_t b)
{
	btn[b].enable = 0;
	
	if(btn[b].rlsd == 1)
	{
		btn[b].pshd = 0;
		btn[b].rlsd = 0;
		btn[b].time = 0;
		btn[b].state = 0;
		btn[b].enable = 1;
		return 1;
	}
	btn[b].enable = 1;
	return 0;
}

void ClearButtons(void)
{
	int i;
	
	for(i = 0;i<NUM_BTNS;i++)
	{
		btn[i].enable = 0;
		btn[i].pshd = 0;
		btn[i].rlsd = 0;
		btn[i].time = 0;
		btn[i].state = BTN_CLEARED;
		btn[i].enable = 1;
	}
}

uint8_t GetButtonHeld(uint8_t b,uint32_t hld_time)
{
	btn[b].enable = 0;
	if(btn[b].time > hld_time)
	{
		btn[b].pshd = 0;
		btn[b].rlsd = 0;
		btn[b].time = 0;
		btn[b].state = BTN_CLEARED;
		btn[b].enable = 1;
		return 1;
	}

	btn[b].enable = 1;
	return 0;
}
