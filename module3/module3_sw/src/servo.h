/*
 * servo.h
 */
#pragma once

#include <stdio.h>
#include "xtmrctr.h"
#include "xparameters.h"  	/* constants used by the hardware */
#include "xil_types.h"		/* types used by xilinx */

#define CLK_PERIOD 0.02
#define CLK_COUNT 1000000

#define DUTY_CYCLE 7.5
#define ARC_START_DUTY_CYCLE 7.0
#define ARC_MID_DUTY_CYCLE 9.5
#define ARC_STOP_DUTY_CYCLE 11.75

/*
 * Initialize the servo, setting the duty cycle to 7.5%
 */
void servo_init(void);

/*
 * Set the duty cycle of the servo
 */
void servo_set(double dutycycle);

/*
 * Get the duty cycle of the servo
 */
double servo_get(void);
