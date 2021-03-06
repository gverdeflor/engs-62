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
#include "wifi.h"							/* WiFi module interface */

/* Possible States */
#define TRAFFIC 0
#define PEDESTRIAN 1
#define TRAIN 2
#define MAINTENANCE 3

/* Maintenance Gate Positions */
#define GATE_CLOSED ARC_START_DUTY_CYCLE
#define GATE_OPEN ARC_STOP_DUTY_CYCLE

/* Timer Counter */
#define FREQ 10								/* 10Hz project spec */
static int ttc_count;
static bool ttc_reset;

/* Control Signals */
static bool ped_inbound = FALSE;
static bool train_arriving = FALSE;
static bool maintenance_key = FALSE;
static int crossing_status = -1;
static int prev_crossing_status = -1;

/* Controller State */
static int state;
static bool done;

/* State Transition Function */
static void change_state() {

	switch(state) {

	case TRAFFIC:

		// Look at other inputs to set next state
		if (train_arriving) { // || crossing_status == 1) {
			state = TRAIN;
			servo_set(GATE_CLOSED);

			printf("[STATE: Train]\n");
			printf("[GATE: Closed]\n");
			printf("[MAINTENANCE: No]\n");
			printf("[TRAIN: Arriving]\n\n");
			ttc_reset = true;
		} else if ((maintenance_key)) { // || crossing_status == 2) {
			state = MAINTENANCE;
			servo_set(GATE_CLOSED);

			printf("[STATE: Maintenance]\n");
			printf("[GATE: Closed]\n");
			printf("[MAINTENANCE: Yes]\n");
			printf("[TRAIN: Clear]\n\n");
			ttc_reset = true;
		} else if (ped_inbound) {
			state = PEDESTRIAN;
			servo_set(GATE_OPEN);

			printf("[STATE: Pedestrian]\n");
			printf("[GATE: Open]\n");
			printf("[MAINTENANCE: No]\n");
			printf("[TRAIN: Clear]\n\n");
			ttc_reset = true;
		}
		break;

	case PEDESTRIAN:

		// Look at other inputs to set next state
		if (train_arriving) { // || crossing_status == 1) {
			state = TRAIN;
			servo_set(GATE_CLOSED);

			printf("[STATE: Train]\n");
			printf("[GATE: Closed]\n");
			printf("[MAINTENANCE: No]\n");
			printf("[TRAIN: Arriving]\n\n");
			ttc_reset = true;
		} else if ((maintenance_key)) { //  || crossing_status == 2) {
			state = MAINTENANCE;
			servo_set(GATE_CLOSED);

			printf("[STATE: Maintenance]\n");
			printf("[GATE: Closed]\n");
			printf("[MAINTENANCE: Yes]\n");
			printf("[TRAIN: Clear]\n\n");
			ttc_reset = true;
		} else if (!ped_inbound) {
			state = TRAFFIC;
			servo_set(GATE_OPEN);

			printf("[STATE: Traffic]\n");
			printf("[GATE: Open]\n");
			printf("[MAINTENANCE: No]\n");
			printf("[TRAIN: Clear]\n\n");
			ttc_reset = true;
		}
		break;

	case TRAIN:

		// Look at other inputs to set next state
		if (!train_arriving) { // || crossing_status == 0) {
			state = TRAFFIC;
			servo_set(GATE_OPEN);

			printf("[STATE: Traffic]\n");
			printf("[GATE: Open]\n");
			printf("[MAINTENANCE: No]\n");
			printf("[TRAIN: Clear]\n\n");
			ttc_reset = true;
		} else if ((maintenance_key)) { // || crossing_status == 2) {
			state = MAINTENANCE;
			servo_set(GATE_CLOSED);
			ttc_reset = true;

			printf("[STATE: Maintenance]\n");
			printf("[GATE: Closed]\n");
			printf("[MAINTENANCE: Yes]\n");
			printf("[TRAIN: Clear]\n\n");
		}
		break;

	case MAINTENANCE:

		// Look at other inputs to set next state
		if ((!maintenance_key)) { // || crossing_status == 0) {
			state = TRAFFIC;
			servo_set(GATE_OPEN);

			printf("[STATE: Traffic]\n");
			printf("[GATE: Open]\n");
			printf("[MAINTENANCE: No]\n");
			printf("[TRAIN: Clear]\n\n");
			ttc_reset = true;
		} else if (train_arriving) { // || crossing_status == 1) {
			state = TRAIN;
			servo_set(GATE_CLOSED);

			printf("[STATE: Train]\n");
			printf("[GATE: Closed]\n");
			printf("[MAINTENANCE: No]\n");
			printf("[TRAIN: Arriving]\n\n");
			ttc_reset = true;
		}
		break;

	}
}

/* Button Callback Function */
static void btn_callback(u32 val) {
	// Change state when an interrupt occurs
	if (val == 0) {
		// Pedestrian Signal 1
		ped_inbound = TRUE;
		change_state();
		ped_inbound = FALSE;
	} else if (val == 1) {
		// Pedestrian Signal 2
		ped_inbound = TRUE;
		change_state();
		ped_inbound = FALSE;
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

//	// Polling remote substation status database
//	send_update();
//	crossing_status = get_crossing_status();
//
//	if (prev_crossing_status != crossing_status) {
//		printf("Crossing Status: %d\n", crossing_status);
//		switch(crossing_status) {
//			case 0:
//				train_arriving = FALSE;
//				maintenance_key = FALSE;
//				change_state();
//				break;
//			case 1:
//				train_arriving = TRUE;
//				maintenance_key = FALSE;
//				change_state();
//				break;
//			case 2:
//				train_arriving = FALSE;
//				maintenance_key = TRUE;
//				change_state();
//				break;
//		}
//		send_update();
//		prev_crossing_status = crossing_status;
//	}

	// Reset TTC counter once state changes
	if (ttc_reset) {
		ttc_count = 0;
		ttc_reset = !ttc_reset;
	} else {
		ttc_count++;
	}

	// Traffic flowing
	if (state == TRAFFIC) {
		//printf("TTC Count: %d \n", ttc_count);
		// Traffic starting --> YELLOW
		if (ttc_count <= 30) {
			led_rgb('y');
		// Traffic going --> GREEN
		} else {
			led_rgb('g');
		}

	// Pedestrians walking
	} else if (state == PEDESTRIAN) {
		//printf("TTC Count: %d \n", ttc_count);
		// Traffic still going --> GREEN
		if (ttc_count <= 100) {
			led_rgb('g');
		// Traffic stopping --> YELLOW
		} else if (ttc_count <= 130) {
			led_rgb('y');
		// Traffic stopped --> RED
		} else if (ttc_count <= 230) {
			led_set(ALL, LED_ON);
			led_rgb('r');
		// Traffic starting
		} else {
			led_set(ALL, LED_OFF);
			ped_inbound = FALSE;
			change_state();
		}

	// Train arriving
	} else if (state == TRAIN) {
		//printf("TTC Count: %d \n", ttc_count);
		// Traffic stopping --> YELLOW
		if ((ttc_count) <= 30) {
			led_rgb('y');
		// Traffic stopped --> RED
		} else {
			led_rgb('r');
		}

	// Engineer maintaining
	} else if (state == MAINTENANCE) {
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
	uart_init();

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
	uart_close();
	gic_close();

	cleanup_platform();
	return 0;
}
