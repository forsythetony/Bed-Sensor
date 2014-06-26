/*	Dependency Information

	compiler.h
	calendar.h

	system/rtc.h

*/

/*
 * main.h
 *
 * Created: 7/24/2012 1:27:23 AM
 *  Author: Administrator
 */


#ifndef MAIN_H_
#define MAIN_H_

#include <compiler.h>
#include <calendar.h>
#include "system/rtc.h"
#define RF_PACKET_SIZE 70

#define LOGGING_THRESHOLD 4096

#define MAX_CHUNK_RESENDS	5
#define CHUNK_SIZE			70
#define MAX_LOG_DIR_LEN		20
#define MAX_FNAME_LEN		64-MAX_LOG_DIR_LEN

volatile uint16_t data_sample[8];



struct
{
	uint8_t 	state;
	uint8_t 	error;
	uint8_t		quit;
	
	struct
	{
		struct  
		{
			uint16_t	avg;
			uint16_t	p2p;
			uint16_t	mx;
			uint16_t	mn;

		} d[8];

	} sts;


	struct  
	{
		uint8_t		valid;
		uint32_t	glb_sec;
		uint32_t 	glb_msec;

		struct calendar_date validated;		
		struct calendar_date curr;
		struct calendar_date last;

	} clk;


	struct 
	{
		uint8_t		sub;
		uint8_t		error;

		bool		conn;

		uint64_t	tx_cnt;
		uint64_t	rx_cnt;

	} com;
	

	struct  
	{

		uint8_t		smpl_enable;
		uint8_t		sub;
		uint8_t		error;
		uint32_t	cntr;
		uint32_t	lvl_avg;
		uint32_t	new_lvl_avg;
		uint8_t		new_threshold;
		uint8_t		mode;
		uint8_t		status;

		bool		fs_chg;
		bool		fs_err;

	} log;
	

	struct  
	{

		uint8_t		sub;
		uint32_t	secs;
		uint32_t 	fsize;
		uint32_t 	npkts;
		uint32_t 	nchunks;
		uint32_t 	rmdpkts;
		uint32_t 	rmdbytes;
		uint32_t 	bleft;
		uint32_t 	curr_chunk;
		uint32_t 	curr_pkt;
		uint32_t 	rsnd_cnt;
		uint8_t 	chunk[RF_PACKET_SIZE*CHUNK_SIZE];
		uint32_t 	chksum;

	} tfr;


	struct  
	{

		uint8_t		trgr;

	} diag;


} sys;

#define BED_DISABLED 	0
#define OFF_BED 		1
#define ON_BED  		2

// Logging Mode
#define LOG_DISABLED	0
#define LOG_THRESHOLD	1
#define LOG_FORCE		2

// Logging Status
#define LOG_OFF 0
#define LOG_ON	1

#define STATUS_TMR_INTERVAL 5000;

#define CMD_START_TFR		0x3C
// [1]	Start Transfer Command (0x3c)
// [4]	File Size (Bytes)
// [1]	Max RF Payload Size (Bytes)
// [1]	Chunk Size
// [1]	File name Size
// [1-64]	File name (ASCII)
 
#define CMD_START_TFR_RSP	0x18
// [1]	Start Transfer Response Command (0x3c)
// [1]	Response
	#define OK_TO_TFR			0x01
	#define TFR_NOT_ALLOWED		0x00
		
#define CMD_DATA_PKT		0x5A
// [1]	Data Packet Command
// [1]	Packet Number (relative to current chunk)
// [1-70]	Payload
#define CMD_CHUNK_RESP	0x5E
	// [1]	Chunk Response Command
	// [1]	Retransfer size (0 means Success
	#define CHUNK_SUCCESS 0x00
//[0-CSize] Packets to retransfer
#define CMD_CHUNK_FIN 0x23

#define CMD_END_TFR	0xF0
	// [1] End Transfer Command (0xF0)
	// [1] Exit Code
		#define END_TFR_ERR		0x00
		#define END_TFR_FIN		0x01
		#define END_TFR_CANCEL	0x02
	// [1] Check Sum

#define CMD_END_TFR_RESP 0x4B
	// [1] End Transfer Response Command (0x4B)
	// [1] Exit Code
		#define END_TFR_RESP_ERR		0x00
		#define END_TFR_RESP_SUCCESS	0x01
		#define END_TFR_RESP_CANCEL		0x02
		#define END_TFR_INV_CHKSUM		0x03

#define CMD_QUERY_LGR 0x96
	// [1] Query Logger Command (0x96)
	
#define CMD_LOG_MODE 0x79
	// [1] Log Mode Command 
	// [2] New Log Mode
		// 0 - Disable Logging
		// 1 - Threshold Mode
		// 2 - Force Logging
		
#define CMD_SET_THRESHOLD 0x72
	// [1] Set Threshold Command
	// [2] Set/Sample
	//		0 - Set from message*
	//      1 - Sample Level
	// [3*] Threshold MSB
	// [4*] Threshold Byte
	// [5*] Threshold Byte
	// [6*] Threshold LSB
	
#define CMD_QUERY_LGR_RESP 0x69
// [1] Query Logger Command (0x69)
// [1] Year-of-Century (0-99)
// [1] Month (1-12)
// [1] Day (1-31)
// [1] Hour (0-23)
// [1] Min (0-59)
// [1] Sec (0-59)
// [4] User/Network ID
// [1] Sensor ID

#define CMD_CONSOLE 0x88
#define CMD_RESET 0x77
#define CMD_DIAGNOSTICS 0x29
#define CMD_FACTORY_RESET 0x25
	

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

extern uint8_t log_data;

#define _d(_ch,_i)	(data[log_data][(_ch)][(_i)])

#define FROM_START	0
#define FROM_CURR	1


//XBEE WIFI
#define WAIT_FOR_MESSAGE	0
#define MSG_START_RCVD		1
#define PARSE_CMD			2
#define HANDLE_CMD			3
#define SYNC_CMD			1	//#SDD-MM-YY_HH:MM:SS\n
#define START_CMD			2
#define STOP_CMD			3
#define MSG_ERROR			4
#define MSG_RESTART			5

//	File System

uint16_t ls(char *ext);
bool cd(char *d);
uint8_t MarkFileAsProcessed(void);
bool FindNextFileWithExt(char *ext,uint8_t mode);
bool ArchiveFile(char *estr);
bool CheckForFiles(void);
bool InitFolderStructure(void);
bool CleanUpOldFiles(void)


//	Data Transfer

bool Connect(uint16_t time);
bool StartTransfer(void);
bool SendStartTransfer(char *dir_str,char *file_name,uint32_t npkts);
bool HandleTransfer(void);
bool FinishTransfer(void);
uint8_t CancelTransfer(void);
uint8_t SendDataPacket(uint8_t *b,uint8_t pnum,uint8_t psize);
uint8_t SendChunk(uint32_t csize);
void WaitForChunkResponse(uint8_t **c);
uint8_t EndTransfer(uint8_t rsn,uint8_t fl_chksum);
void send_data();
void HandleXBeeFrame(void);


//	Logging

bool StartLog(void);
void HandleLog(void);
void CloseLog(void);
bool QueryLogger(volatile rtc_t *cl);

//	Time

bool TimeValid(rtc_t *t);
void add_second_to_date(void);
uint8_t check_leap_year(uint8_t y);

//	Other

void ClearSamples(void);
void SoftwareReset(void);
bool LoadThreshold(void);
bool SampleThreshold(void);
void RunDiagnostics(void);
void HandleSystemStatus(void);

//void HandleXBFrame(XB_API_FRAME_t *frm);



#define CREATE_LOG	0
#define LOG_DATA	1
#define CLOSE_LOG	2

#define START_TRANSFER  0
#define SEND_CHUNKS		1
#define END_TRANSFER	2
#define RENAME_FILE		3
#define CANCEL_TRANSFER	4

#define MAX_LOG_SIZE	600
#define MIN_LOG_SIZE	15

#define LGR_QRY_TIMEOUT		5000
#define LOGGER_QUERY_PERIOD 1 //Min

#endif /* MAIN_H_ */