/*
 * servo.h
 */
#pragma once

#include <stdio.h>
#include "xtmrctr.h"
#include "xparameters.h"  	/* constants used by the hardware */
#include "xil_types.h"		/* types used by xilinx */

#define DUTY_CYCLE 7.5
#define MAX_DUTY_CYCLE 12.5
#define MIN_DUTY_CYCLE 2.5


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