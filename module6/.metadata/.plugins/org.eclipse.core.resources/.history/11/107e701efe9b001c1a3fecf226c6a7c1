/*
 * ttc.c - TTC module
 *
 * NOTE: The TTC hardware must be enabled (Timer 0 on the processing system) before it can be used!!
 *
 */

#include "xttcps.h"
#include "gic.h"			/* interrupt controller interface */

static XTtcPs ttcport;		/* timer port instance */

static void (*saved_ttc_callback)(void);

/*
 * control is passed to this function when timer is running
 *
 * devicep -- ptr to the device that caused the interrupt
 */
void ttc_handler() {
	/* coerce the generic pointer into a gpio */
	//XGpio *dev = (XGpio*)devicep;
	saved_ttc_callback();

	/* clear interrupt before exiting handler */
	XTtcPs_GetInterruptStatus(&ttcport);
	XTtcPs_ClearInterruptStatus(&ttcport, XTTCPS_IXR_INTERVAL_MASK);
}

/*
 * ttc_init -- initialize the ttc frequency and callback
 */
void ttc_init(u32 freq, void (*ttc_callback)(void)) {

	XInterval interval;
	u8 prescaler;

	/* initialize ttcport and immediately disable interrupts */
	XTtcPs_Config *config = XTtcPs_LookupConfig(XPAR_XTTCPS_0_DEVICE_ID);
	XTtcPs_CfgInitialize(&ttcport, config, config->BaseAddress);
	XTtcPs_DisableInterrupts(&ttcport, XTTCPS_IXR_INTERVAL_MASK);

	XTtcPs_CalcIntervalFromFreq(&ttcport, freq, &interval, &prescaler);
	XTtcPs_SetPrescaler(&ttcport, prescaler);
	XTtcPs_SetInterval(&ttcport, interval);
	XTtcPs_SetOptions(&ttcport, XTTCPS_OPTION_INTERVAL_MODE);

	/* connect handler to the gic (c.f. gic.h) */
	gic_connect(XPAR_XTTCPS_0_INTR, ttc_handler, &ttcport);

	/* whenever the timer is running */
	saved_ttc_callback = ttc_callback;
}

/*
 * ttc_start -- start the ttc
 */
void ttc_start(void) {
	/* enable interrupts to processor */
	XTtcPs_EnableInterrupts(&ttcport, XTTCPS_IXR_INTERVAL_MASK);
	XTtcPs_Start(&ttcport);
}

/*
 * ttc_stop -- stop the ttc
 */
void ttc_stop(void) {
	/* disable interrupts to processor */
	XTtcPs_DisableInterrupts(&ttcport, XTTCPS_IXR_INTERVAL_MASK);
	XTtcPs_Stop(&ttcport);
}

/*
 * ttc_close -- close down the ttc
 */
void ttc_close(void) {
	/* disconnect the interrupts (c.f. gic.h) */
	gic_disconnect(XPAR_XTTCPS_0_INTR);
}


