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

/* Status Signals */
static bool gate_closed = FALSE;

/* Control Signals */
static bool train_arriving = FALSE;
static bool maintenance_key = FALSE;

/* Controller State */
static int state;
static int prev_state;
static bool done;

/* Timer Counter */
static int ttc_count;

/* State Transition Function */
static void change_state() {

	// Default states
	gate_closed = FALSE;

	switch(state) {
	// Based on current state
	case TRAFFIC:
		// Generate outputs
		led_set(ALL, LED_OFF);
		servo_set(GATE_OPEN);

		printf("[GATE: Open]\n");
		printf("[MAINTENANCE: No]\n");
		printf("[TRAIN: Clear]\n");

		// Look at other inputs to set next state
		if (train_arriving) {
			state = TRAIN;
		} else if (maintenance_key) {
			state = MAINTENANCE;
		}
		prev_state = TRAFFIC;
		break;

	case PEDESTRIAN:
		// Generate outputs
		led_set(ALL, LED_ON);
		servo_set(GATE_OPEN);

		printf("[GATE: Open]\n");
		printf("[MAINTENANCE: No]\n");
		printf("[TRAIN: Clear]\n");

		// Look at other inputs to set next state
//		if (pedestrian_delay) {
//			printf("Should not be called (PEDESTRIAN)\n");
//			state = TRAFFIC;
//		} else
			if (train_arriving) {
			state = TRAIN;
		} else if (maintenance_key) {
			state = MAINTENANCE;
		}
		prev_state = PEDESTRIAN;
		break;

	case TRAIN:
		// Generate outputs
		gate_closed = TRUE;
		led_set(ALL, LED_OFF);
		servo_set(GATE_CLOSED);

		printf("[GATE: Closed]\n");
		printf("[MAINTENANCE: No]\n");
		printf("[TRAIN: Arriving]\n");

		// Look at other inputs to set next state
		if (train_arriving == FALSE) {
			printf("Should not be called (TRAIN)\n");
			state = TRAFFIC;
		} else if (maintenance_key) {
			state = MAINTENANCE;
		}
		prev_state = TRAIN;
		break;

	case MAINTENANCE:
		// Generate outputs
		gate_closed = TRUE;
		led_set(ALL, LED_OFF);
		servo_set(GATE_CLOSED);

		printf("[GATE: Closed]\n");
		printf("[MAINTENANCE: Yes]\n");
		printf("[TRAIN: Clear]\n");

		// Look at other inputs to set next state
		if (maintenance_key == FALSE) {
			state = TRAFFIC;
			printf("Should not be called (MAINTENANCE)\n");
		}
		prev_state = MAINTENANCE;
		break;

	}
}

/* Button Callback Function */
static void btn_callback(u32 val) {
	printf("Entered button callback!\n");
	// Change state when an interrupt occurs
	if (val == 0) {
		// Pedestrian Signal 1
		state = PEDESTRIAN;
		change_state();
	} else if (val == 1) {
		// Pedestrian Signal 2
		state = PEDESTRIAN;
		change_state();
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

		if (train_arriving) {
			state = TRAIN;
		}
		change_state();
	}
}

void turn_gate(void) {
	float pot_volt = adc_get_pot();
	double gate_wheel = (pot_volt * 4.75) + 7;

	// Caps PWM duty cycle to mechanical limits
	if (gate_wheel > ARC_STOP_DUTY_CYCLE) {
		gate_wheel = ARC_STOP_DUTY_CYCLE;
	} else if (gate_wheel < ARC_START_DUTY_CYCLE) {
		gate_wheel = ARC_START_DUTY_CYCLE;
	}

	servo_set(gate_wheel);
}

void ttc_callback(void) {

	printf("Previous State: %d \n", prev_state);
	printf("Current State: %d \n", state);

	// Reset TTC counter if state changes
	if (prev_state != state) {
		ttc_count = 0;
	} else {
		ttc_count++;
	}

	// Traffic flowing
	if (state == TRAFFIC) {
		printf("TRAFFIC\n");
		printf("TTC Count: %d \n", ttc_count);
		// Traffic starting --> YELLOW
		if ((ttc_count) <= 30) {
			led_rgb('y');
		// Traffic stopped --> GREEN
		} else {
			led_rgb('g');
		}

	// Pedestrians walking
	} else if (state == PEDESTRIAN) {
		printf("PEDESTRIAN\n");
		printf("TTC Count: %d \n", ttc_count);
		// Traffic stopping --> YELLOW
		if ((ttc_count % 190) <= 30) {
			led_rgb('y');
  		// Traffic stopped --> RED
		} else if (((ttc_count % 190) <= 130)) {
			led_rgb('r');
		// Traffic starting --> YELLOW
		} else if (((ttc_count % 190) <= 160)) {
			led_rgb('y');
		// Traffic starting --> GREEN
		} else {
			led_rgb('g');
			state = TRAFFIC;
		}

	// Train arriving
	} else if (state == TRAIN) {
		printf("TRAIN\n");
		led_rgb('r');

		printf("TTC Count: %d \n", ttc_count);
		// Traffic stopping --> YELLOW
		if ((ttc_count) <= 30) {
			led_rgb('y');
		// Traffic stopped --> RED
		} else {
			led_rgb('r');
		}

	// Engineer maintaining
	} else if (state == MAINTENANCE) {
		printf("MAINTENANCE\n");
		turn_gate();
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
