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

#define GATE_CLOSED ARC_START_DUTY_CYCLE
#define GATE_OPEN ARC_STOP_DUTY_CYCLE

#define FREQ 10								/* 10Hz project spec */
#define TRAFFIC_FREQ 0.33					/* 0.33Hz frequency for traffic light */
#define MAINTENANCE_FREQ 0.5				/* 0.5Hz frequency for maintenance light */

#define TRAFFIC_FREQ 0.33					/* 0.33Hz frequency for traffic light */
#define MAINTENANCE_FREQ 0.5				/* 0.5Hz frequency for maintenance light */

/* Status Signals */
static bool red_light = FALSE;
static bool yellow_light = FALSE;
static bool green_light = TRUE;
static bool blue_light = FALSE;
static bool gate_closed = FALSE;

/* Control Signals */
static bool walk_button_1 = FALSE;
static bool walk_button_2 = FALSE;
static bool traffic_delay = FALSE;
static bool pedestrian_delay = FALSE;
static bool train_delay = FALSE;
static bool train_arriving = FALSE;
static bool maintenance_key = FALSE;
//static bool wheel_close = FALSE;

/* Controller State */
static int state;
static int prev_state;
static bool done;

/* Timer Counter */
static int ttc_count;

/* State Transition Function */
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
		if ((walk_button_1 || walk_button_2)) {
			state = PEDESTRIAN;
			printf("changed to ped\n");
		} else if (train_arriving) {
			state = TRAIN;
		} else if (maintenance_key) {
			state = MAINTENANCE;
		}
		prev_state = TRAFFIC;

	case PEDESTRIAN:
		// Generate outputs
		red_light = TRUE;

		// Look at other inputs to set next state
		if (pedestrian_delay) {
			printf("should not be called 3\n");
			state = TRAFFIC;
		} else if (train_arriving) {
			state = TRAIN;
		} else if (maintenance_key) {
			state = MAINTENANCE;
		}
		prev_state = PEDESTRIAN;

	case TRAIN:
		// Generate outputs
		red_light = TRUE;
		gate_closed = TRUE;
		print("im a train beyotch");
		// Look at other inputs to set next state
		if ((train_arriving == FALSE) && train_delay) {
			printf("should not be called 1\n");
			state = TRAFFIC;
		} else if (maintenance_key) {
			printf("main key high when in train");
			state = MAINTENANCE;
		}
		prev_state = TRAIN;


	case MAINTENANCE:
		// Generate outputs
		blue_light = TRUE;
		gate_closed = TRUE;

		// Look at other inputs to set next state
		if (maintenance_key == FALSE) {
			state = TRAFFIC;
			printf("should not be called 2\n");
		}
		prev_state = MAINTENANCE;

	break;
	}
}

/* Button Callback Function */
static void btn_callback(u32 val) {
	printf("entered button callback!\n");
	// Change state when an interrupt occurs
	if (val == 0) {
		// Pedestrian Signal 1
		walk_button_1 = TRUE;
		change_state();
		walk_button_1 = FALSE;
	} else if (val == 1) {
		// Pedestrian Signal 2
		walk_button_2 = TRUE;
		change_state();
		walk_button_2 = FALSE;
	}
}

/* Switch Callback Function */
static void sw_callback(u32 val) {
	// Change state when an interrupt occurs
	if (val == 0) {
		// Maintenance Mode/Clear
		maintenance_key = !maintenance_key;
		change_state();
	} else if (val == 1) {
		// Train Arrival/Clear
		train_arriving = !train_arriving;

		change_state();
	}
}

void ttc_callback(void) {

	// Reset TTC counter if state changes
	if (prev_state != state) {
		ttc_count = 0;
	} else {
		ttc_count++;
	}

	if (state == TRAFFIC) {
		printf("TRAFFIC\n");
		servo_set(GATE_OPEN);
//		if ( (prev_state == PEDESTRIAN) || (prev_state == TRAIN) ) {
//			if ((ttc_count % 60) <= 30) {
//				led_rgb('y');
//			}
//		} else {
//			if ((ttc_count % 60) > 30) {
//				led_rgb('g');
//			}
//		}
		led_rgb('g');

	} else if (state == PEDESTRIAN) {
		printf("PEDESTRIAN\n");
		servo_set(GATE_OPEN);
		led_rgb('r');


	} else if (state == TRAIN) {
		printf("TRAIN\n");
		servo_set(GATE_CLOSED);
		led_rgb('r');

	} else if (state == MAINTENANCE) {
		printf("MAINTENANCE\n");
		servo_set(GATE_CLOSED);
		// Blue maintenance light
		if ((ttc_count % 20) <= 10) {
			led_rgb('b');
		} else {
			led_rgb('o');
		}
	}
}

int main() {

	/* Initialize hardware platform and I/O	 */
	init_platform();
	gic_init();
	led_init();
	io_btn_init(&btn_callback);
	io_sw_init(&sw_callback);
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
