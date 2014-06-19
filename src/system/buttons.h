/*
 * buttons.h
 *
 * Created: 8/15/2012 2:02:45 PM
 *  Author: Mark
 */ 


#ifndef BUTTONS_H_
#define BUTTONS_H_

#include <compiler.h>
#include <asf.h>



#define NUM_BTNS 2
/*
typedef enum b_states
{
	BTN_CLEARED;
	BTN_READY;
	BTN_PUSHED;
	BTN_RELEASED;
	BTN_HELD;
}b_states;
*/
	#define BTN_CLEARED 0
	#define BTN_READY 1
	#define BTN_PUSHED 2
	#define BTN_RELEASED 3
	#define BTN_HELD 4
	
typedef struct BUTTON_t
{
	uint32_t pin;
	uint8_t pshd;
	uint32_t time;
	uint8_t rlsd;
	uint8_t state;
	uint8_t enable;
		// BTN_CLEARED
		// BTN_PUSHED
		// BTN_HELD
}BUTTON_t;

extern BUTTON_t btn[NUM_BTNS];


void HandleButtons(void);
uint8_t InitButton(uint8_t idx,uint8_t pin);
uint8_t GetButtonPush(uint8_t b);
uint8_t GetButtonReleased(uint8_t b);
void ClearButtons(void);
uint8_t GetButtonHeld(uint8_t b,uint32_t hld_time);


#endif /* BUTTONS_H_ */