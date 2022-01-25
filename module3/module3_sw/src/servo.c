/*
 * servo.c
 */

#include "servo.h"				/* servo module interface */

static XTmrCtr tmrport;			/* AXI timer port instance */
static double servo_dutycycle;	/* PWM duty cycle */

/* Setting options for AXI timer */
static const u32 AXI_TMR_OPTIONS = 0x0 | XTC_PWM_ENABLE_OPTION | XTC_EXT_COMPARE_OPTION | XTC_DOWN_COUNT_OPTION;

/*
 * Initialize the servo, setting the duty cycle to 7.5%
 */
void servo_init(void) {

	XTmrCtr_Initialize(&tmrport, XPAR_TMRCTR_0_DEVICE_ID);

	XTmrCtr_SetOptions(&tmrport, 0, AXI_TMR_OPTIONS);
	XTmrCtr_SetOptions(&tmrport, 1, AXI_TMR_OPTIONS);

	XTmrCtr_SetResetValue(&tmrport, 0, 1000000); 	// 50MHz clock, period = 20ms
	XTmrCtr_SetResetValue(&tmrport, 1, 75000);		// Thigh = 1.5ms for 7.5% duty cycle

	XTmrCtr_Start(&tmrport, 0);
	XTmrCtr_Start(&tmrport, 1);

	servo_dutycycle = DUTY_CYCLE;
	printf("Initial Duty Cycle: %0.2f\n", servo_dutycycle);
}

/*
 * Set the duty cycle of the servo
 */
void servo_set(double dutycycle) {
	// Check duty cycle is between 2.5% and 12.5%
	if ( (dutycycle < MIN_DUTY_CYCLE) || (dutycycle > MAX_DUTY_CYCLE) ) {
		printf("The requested duty cycle is out of bounds!\n");
		return;
	}

	double high_time = (dutycycle / 100) * 0.02;					// in seconds
	double high_count = high_time * XPAR_AXI_TIMER_0_CLOCK_FREQ_HZ;	// in clock cycles

	u32 reset_value = high_count;
	XTmrCtr_SetResetValue(&tmrport, 1, reset_value);
	servo_dutycycle = dutycycle;

	printf("Duty Cycle: %0.2f\n", dutycycle);
//	printf("High Time: %f\n", high_time);
//	printf("High Clock Cycle Count: %d\n", high_count);
}

/*
 * Get the duty cycle of the servo
 */
double servo_get(void) {
	return servo_dutycycle;
}

