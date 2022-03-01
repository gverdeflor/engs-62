/*
 * main.c -- working with Serial I/O and GPIO
 *
 * Assumes the LED's are connected to AXI_GPIO_0, on channel 1,
 * 			   button are connected to AXI_GPIO_1, on channel 1,
 * 			   switches are connected to AXI_GPIO_2 on channel 1
 *
 * Terminal Settings:
 *  -Baud: 115200
 *  -Data bits: 8
 *  -Parity: no
 *  -Stop bits: 1
 */
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <xgpio.h>							/* Xilinx GPIO functions */
#include <xuartps.h>						/* Xilinx UART functions */
#include "xil_types.h"						/* u32, u16 etc */
#include "platform.h"						/* ZYBOboard interface */
#include "xparameters.h"					/* constants used by the hardware */
#include "led.h"							/* led module interface */
#include "gic.h"							/* interrupt controller interface */
#include "io.h"								/* button and switch module interface */
#include "ttc.h"							/* TTC module interface */
#include "servo.h"							/* servo module interface */
#include "adc.h"							/* ADC module interface */
//#include "wifi.h"							/* WiFi module interface */

/* Possible States */
#define TRAFFIC 0
#define PEDESTRIAN 1
#define TRAIN 2
#define MAINTENANCE 3
//#define LIGHT 4

/* Status Signals */
static bool red_light = FALSE;
static bool yellow_light = FALSE;
static bool green_light = TRUE;
static bool blue_light = FALSE;
static bool gate_closed = FALSE;

/* Control Signals */
static bool walk_button_1;
static bool walk_button_2;
static bool light_delay;
static bool traffic_delay;
static bool pedestrian_delay;
static bool train_delay;
static bool train_arriving;
static bool train_clear;
static bool maintenance_key;
static bool wheel_close;

/* Current State of Controller */
static int state;
static bool done;

/* State Transition Function (Version 1) */
static void change_state() {

	// Default states
	green_light = FALSE;
	yellow_light = FALSE;
	red_light = FALSE;
	blue_light = FALSE;
	gate_closed = FALSE;

	switch(state) {
	// Based on current state
	case TRAFFIC:
		// Generate outputs
		green_light = TRUE;

		// Look at other inputs to set next state
		if ((walk_button_1 || walk_button_2) && traffic_delay) {
			state = PEDESTRIAN;
			traffic_stop_light();
		} else if (train_arriving) {
			state = TRAIN;
			traffic_stop_light();
		} else if (maintenance_key && wheel_close) {
			state = MAINTENANCE;
			traffic_stop_light();
		}

	case PEDESTRIAN:
		// Generate outputs
		red_light = TRUE;

		// Look at other inputs to set next state
		if (pedestrian_delay) {
			state = TRAFFIC;
			traffic_start_light();
		} else if (train_arriving) {
			state = TRAIN;
		} else if (maintenance_key && wheel_close) {
			state = MAINTENANCE;
		}

	case TRAIN:
		// Generate outputs
		red_light = TRUE;
		gate_closed = TRUE;

		// Look at other inputs to set next state
		if (train_clear && train_delay) {
			state = TRAFFIC;
			traffic_start_light();
		} else if (maintenance_key){
			state = MAINTENANCE;
		}

	case MAINTENANCE:
		// Generate outputs
		blue_light = TRUE;
		gate_closed = TRUE;

		// Look at other inputs to set next state
		if (maintenance_key == FALSE && wheel_close == FALSE) {
			state = TRAFFIC;
			traffic_start_light();
		}

	break;
	}
}

/* Handles timing when changing from green to red light */
void traffic_stop_light(void) {
	red_light == TRUE;
}

/* Handles timing when changing from green to red light */
void traffic_start_light(void) {
	green_light == TRUE;
}


/* Example Callback Function */
static void btn_callback(u32 btn) {
	// Change state when an interrupt occurs
	change_state();
}

void ttc_callback(void) {
	// Toggle LED 4 every second
	led_toggle(4);
}

int main() {

	/* Initialize hardware platform and I/O	 */
	init_platform();
	gic_init();
	led_init();
	io_btn_init(&btn_callback);
	io_sw_init(&btn_callback);
	ttc_init(FREQ, &ttc_callback);
	ttc_start();
	adc_init();
	servo_init();
	//wifi_init();

	/* Set initial state */
	state = TRAFFIC;
	done = false;
	printf("\n[Hello]\n");

	/* Main loop does nothing */
	while(!done) {
		sleep(1);		// Do nothing and wait for interrupts
	}
	printf("[done]\n");
	sleep(1);

	// Turn off all I/O and cleanup hardware
	led_set(ALL, LED_OFF);
	io_btn_close();
	io_sw_close();
	ttc_stop();
	ttc_close();
	//wifi_close();
	gic_close();

	cleanup_platform();
	return 0;
}
