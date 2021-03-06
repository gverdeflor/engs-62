/*
 * wifi.h -- The WiFi module interface
 */
#pragma once

#include <stdio.h>
#include <xuartps.h>		/* Xilinx UART functions */
#include "xparameters.h"  	/* constants used by the hardware */
#include "xil_types.h"		/* types used by xilinx */

#define CONFIGURE 0
#define PING 1
#define UPDATE 2
#define READ 3

#define SERVER_ID 25

typedef struct {
	int type;			// must be assigned to PING
	int id;				// must be assigned to class ID
} ping_t;

typedef struct {
	int type;			// must be assigned to UPDATE
	int id;				// must be assigned to class ID
	int value;			// must be assigned to some value
} update_request_t;

typedef struct {
	int type;			// must be assigned to UPDATE
	int id;				// must be assigned to class ID
	int average;		// average value from all class WiFi module
	int values[30];		// array of last update value from WiFi module
} update_response_t;

/*
 * Get train crossing status from remote substation
 */
int get_crossing_status(void);

/*
 * Initialize UART communication
 */
void uart_init(void);

/*
 * Close UART communication
 */
void uart_close(void);

/*
 * Sends ping message to local server over UART
 */
void send_ping(void);

/*
 * Sends update message to local server over UART
 */
void send_update(void);

/*
 * Changes WiFi mode to reading update request
 */
void changeToRead(void);

/*
 * Handles UART1 communication from keyboard input
 */
void uart1_handler(void *CallBackRef, u32 Event, unsigned int EventData);

/*
 * Handles UART0 communication from WiFi module
 */
void uart0_handler(void *CallBackRef, u32 Event, unsigned int EventData);

