/*
 * io.c -- switch and button module
 *
 */

#include <stdio.h>			/* printf for errors */
#include <stdbool.h>
#include <xgpio.h>		  	/* axi gpio */
#include "xparameters.h"  	/* constants used by the hardware */
#include "xil_types.h"		/* types used by xilinx */
#include "gic.h"			/* interrupt controller interface */
#include "io.h" 			/* switch and button module interface */
#include <math.h>			/* math library for logarithms */

#define CHANNEL1 1			/* channel 1 of the GPIO port */
#define INPUT 0x1			/* setting GPIO direction to input */

/* hidden private state */
static XGpio btnport; 		   /* button GPIO port instance */
static XGpio swport;           /* switch GPIO port instance */

static bool pushed;			   /* variable used to detect rising edge of button push */
static u32 curr_LED;
static u32 prev_sw_state;
static u32 prev_btn_state;

static void (*saved_btn_callback)(u32 v);
static void (*saved_sw_callback)(u32 v);

u32 btn_to_led(u32 btn) {
	switch(btn) {
	case 1 :
		// Drive LED0
		curr_LED = 0;
		break;
	case 2 :
		// Drive LED1
		curr_LED = 1;
		break;
	case 4 :
		// Drive LED2
		curr_LED = 2;
		break;
	case 8 :
		// Drive LED3
		curr_LED = 3;
		break;
	default:
		printf("\nInvalid button!", btn);
	}
	return curr_LED;
}

u32 sw_to_led(u32 sw) {
	// Count number of left shifts until positive
	int num_shifts = 0;
	u32 update = sw;
	while ((update >>= 1)) { num_shifts++; }

	return num_shifts;
}

/*
 * control is passed to this function when a button is pushed
 *
 * devicep -- ptr to the device that caused the interrupt
 */
void btn_handler(void *devicep) {
	/* coerce the generic pointer into a gpio */
	XGpio *dev = (XGpio*)devicep;

	pushed = !pushed;
	u32 read_btn = XGpio_DiscreteRead(&btnport, CHANNEL1);
	u32 update = read_btn ^ prev_btn_state;

	if (pushed) {
		u32 led = btn_to_led(update);
		saved_btn_callback(led);
	}
	XGpio_InterruptClear(dev, 0x1);
	prev_btn_state = read_btn;
}

/*
 * control is passed to this function when a switch is flipped
 *
 * devicep -- ptr to the device that caused the interrupt
 */
void sw_handler(void *devicep) {
	/* coerce the generic pointer into a gpio */
	XGpio *dev = (XGpio*)devicep;

	u32 read_sw = XGpio_DiscreteRead(&swport, CHANNEL1);
	u32 update = read_sw ^ prev_sw_state;

	u32 led = sw_to_led(update);
	saved_sw_callback(led);

	XGpio_InterruptClear(dev, 0x1);
	prev_sw_state = read_sw;
}

/*
 * initialize the btns providing a callback
 */
void io_btn_init(void (*btn_callback)(u32 btn)) {
	/* initialize btnport (c.f. module 1) and immediately disable interrupts */
	XGpio_Initialize(&btnport, XPAR_AXI_GPIO_1_DEVICE_ID);
	XGpio_SetDataDirection(&btnport, CHANNEL1, INPUT);
	XGpio_InterruptDisable(&btnport, 0x1);
	XGpio_InterruptGlobalDisable(&btnport);

	/* connect handler to the gic (c.f. gic.h) */
	gic_connect(XPAR_FABRIC_GPIO_1_VEC_ID, &btn_handler, &btnport);

	/* enable interrupts on channel (c.f. table 2.1) */
	XGpio_InterruptEnable(&btnport, 0x1);

	/* enable interrupt to processor (c.f. table 2.1) */
	XGpio_InterruptGlobalEnable(&btnport);

	// Whenever a button is pressed
	saved_btn_callback = btn_callback;
}

/*
 * close the btns
 */
void io_btn_close(void) {
	/* disconnect the interrupts (c.f. gic.h) */
	gic_disconnect(XPAR_FABRIC_GPIO_1_VEC_ID);
}


/*
 * initialize the switches providing a callback
 */
void io_sw_init(void (*sw_callback)(u32 sw)) {
	/* initialize swport (c.f. module 1) and immediately disable interrupts */
	XGpio_Initialize(&swport, XPAR_AXI_GPIO_2_DEVICE_ID);
	XGpio_SetDataDirection(&swport, CHANNEL1, INPUT);
	XGpio_InterruptDisable(&swport, 0x1);
	XGpio_InterruptGlobalDisable(&swport);

	/* connect handler to the gic (c.f. gic.h) */
	gic_connect(XPAR_FABRIC_GPIO_2_VEC_ID, &sw_handler, &swport);

	/* enable interrupts on channel (c.f. table 2.1) */
	XGpio_InterruptEnable(&swport, 0x1);

	/* enable interrupt to processor (c.f. table 2.1) */
	XGpio_InterruptGlobalEnable(&swport);

	// Whenever a switch is flipped
	saved_sw_callback = sw_callback;

	prev_sw_state = XGpio_DiscreteRead(&swport, CHANNEL1);;
}

/*
 * close the switches
 */
void io_sw_close(void) {
	/* disconnect the interrupts (c.f. gic.h) */
	gic_disconnect(XPAR_FABRIC_GPIO_2_VEC_ID);

}
