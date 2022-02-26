/*
 * blinky.c -- working with Serial I/O and GPIO
 *
 * Assumes the LED's are connected to AXI_GPIO_0, on channel 1
 *
 * Terminal Settings:
 *  -Baud: 115200
 *  -Data bits: 8
 *  -Parity: no
 *  -Stop bits: 1
 */
#include <stdio.h>							/* printf(), fflush(stdout), getchar() */
#include <stdlib.h>							/* strtol() */
#include <strings.h>						/* strcmp() */
#include <xgpio.h>							/* Xilinx GPIO functions */
#include <xuartps.h>						/* Xilinx UART functions */
#include "xil_types.h"						/* u32, u16 etc */
#include "platform.h"						/* ZYBOboard interface */
#include "xparameters.h"					/* constants used by the hardware */
#include "led.h"							/* led module interface */
#include "gic.h"							/* interrupt controller interface */
#include "io.h"								/* button and switch module interface */
#include "servo.h"							/* servo module interface */
#include "adc.h"							/* ADC module interface */
#include "wifi.h"							/* WiFi module interface */

static bool done = false;					/* UART interrupt status */
static int wifi_mode = CONFIGURE;			/* WiFi module operation mode */

void mycallback(u32 val) {
	// Toggle LED 0-3 based on button and switch input
	led_set(ALL, LED_OFF);
	led_toggle(val);
	//printf("LED%lu toggled!\n", val);

	if (val == 0) {
		// Enter CONFIGURE mode when button 0 pressed
		wifi_mode = CONFIGURE;
		printf("< allows entry to wifi cmd mode >\n");
	} else if (val == 1) {
		// Enter PING mode when button 1 pressed
		wifi_mode = PING;
		send_ping();
	} else if (val == 2) {
		// Enter UPDATE mode when button 2 pressed
		wifi_mode = UPDATE;
		//changeToRead();
		//printf("Enter a new value:\n");
		send_update();
		printf("Servo position set by potentiometer!\n");
	} else if (val == 3) {
		// Disconnect UART when button 3 pressed
		done = true;
	}
}

//void uart_callback(u8* buffer) {
//	if (wifi_mode == PING) {
//		printf("[PING,id=%d]\n", ((ping_t*)buffer)->id);
//	} else if (wifi_mode == UPDATE) {
//		printf("[UPDATE,id=%d,average=%d,value=%d]\n", ((update_response_t*)buffer)->id, ((update_response_t*)buffer)->average, ((update_response_t*)buffer)->values[25]);
//	}
//}

int main() {

	/* Initialize hardware platform and I/O	 */
	init_platform();
	gic_init();
	led_init();
	adc_init();
	servo_init();
//	uart_init(&uart_callback);
	uart_init();
	io_btn_init(&mycallback);
	io_sw_init(&mycallback);

	 /* 
		* set stdin unbuffered, forcing getchar to return immediately when
		* a character is typed.
		*/
	setvbuf(stdin,NULL,_IONBF,0);
	 
	printf("\n[Hello]\n");

	// Wait for interrupts from UART handler
	while(!done) {
	 sleep(1);
	}
	printf("[done]\n");
	sleep(1);

	// Turn off all I/O
	led_set(ALL, LED_OFF);
	io_btn_close();
	io_sw_close();
	uart_close();
	gic_close();

	cleanup_platform();					/* cleanup the hardware platform */
	return 0;
}
