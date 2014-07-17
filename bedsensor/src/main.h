/*
 * main.h
 *
 * Created: 7/24/2012 1:27:23 AM
 *  Author: Administrator
 */

/*
	Some random comment
*/
/*
	rand comment 2
*/


#ifndef MAIN_H_
#define MAIN_H_

#include <compiler.h>

volatile uint16_t data_sample[8];

void add_second_to_date(void);
uint8_t check_leap_year(uint8_t y);	

// System States
#define INIT		0
#define IDLE		1
#define ACTIVE		2
#define CONNECT		3
#define LOGGING		4
#define TRANSFER	5
#define ERROR		6

#define F1 0
#define F2 2
#define F3 4
#define F4 6
#define R1 1
#define R2 3
#define R3 5
#define R4 7

//XBEE WIFI
#define WAIT_FOR_MESSAGE	0
#define MSG_START_RCVD		1
#define PARSE_CMD			2
#define HANDLE_CMD			3
#define SYNC_CMD		1	//#SDD-MM-YY_HH:MM:SS\n
#define START_CMD		2
#define STOP_CMD		3
#define MSG_ERROR			4
#define MSG_RESTART			5

void SoftwareReset(void);
void send_data(void);

#endif /* MAIN_H_ */