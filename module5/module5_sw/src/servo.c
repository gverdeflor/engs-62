/*
 * servo.c
 */

#include "servo.h"				/* servo module interface */

static XTmrCtr tmrport;			/* AXI timer port instance */
static double servo_dutycycle;	/* PWM duty cycle */

/* Setting options for AXI timer */
static const u32 AXI_TMR_OPTIONS = 0x0 | XTC_PWM_ENABLE_OPTION | XTC_EXT_COMPARE_OPTION | XTC_DOWN_COUNT_OPTION;

/*
 * Initialize the servo, setting the duty cycle to 9.5%
 */
void servo_init(void) {
	// Initialize servo motor to midpoint of 90 degree arc
	XTmrCtr_Initialize(&tmrport, XPAR_TMRCTR_0_DEVICE_ID);
	XTmrCtr_SetOptions(&tmrport, 0, AXI_TMR_OPTIONS);
	XTmrCtr_SetOptions(&tmrport, 1, AXI_TMR_OPTIONS);
	servo_set(ARC_MID_DUTY_CYCLE);

//	XTmrCtr_SetResetValue(&tmrport, 0, CLK_COUNT); 	// 50MHz clock, period = 20ms
//	XTmrCtr_SetResetValue(&tmrport, 1, 75000);		// Thigh = 1.5ms for 7.5% duty cycle

}

/*
 * Set the duty cycle of the servo
 */
void servo_set(double dutycycle) {
	// Check duty cycle is between 90 degree arc
	if ( (dutycycle < ARC_START_DUTY_CYCLE) || (dutycycle > ARC_STOP_DUTY_CYCLE) ) {
		printf("The requested duty cycle is out of bounds!\n");
		return;
	}

	// Stop both timers
	XTmrCtr_Stop(&tmrport, 0);
	XTmrCtr_Stop(&tmrport, 1);

	// Recalculate duty cycle
	double high_time = (dutycycle / 100) * CLK_PERIOD;					// in seconds
	double high_count = high_time * XPAR_AXI_TIMER_0_CLOCK_FREQ_HZ;		// in clock cycles

	XTmrCtr_SetResetValue(&tmrport, 0, CLK_COUNT);
	u32 reset_value = high_count;
	XTmrCtr_SetResetValue(&tmrport, 1, reset_value);
	servo_dutycycle = dutycycle;

	// Start both timers
	XTmrCtr_Start(&tmrport, 0);
	XTmrCtr_Start(&tmrport, 1);

//	printf("Duty Cycle: %0.2f\n", dutycycle);
//	printf("High Time: %f\n", high_time);
//	printf("High Clock Cycle Count: %d\n", high_count);
}

/*
 * Get the duty cycle of the servo
 */
double servo_get(void) {
	return servo_dutycycle;
}

