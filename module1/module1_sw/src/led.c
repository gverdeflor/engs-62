/*
 * led.c -- led module
 *
 */

#include <stdio.h>
#include <stdbool.h>
#include <xgpio.h>		  	/* axi gpio */
#include <xgpiops.h>		/* processor gpio */
#include "xparameters.h"  	/* constants used by the hardware */
#include "xil_types.h"		/* types used by xilinx */
#include "led.h"			/* led module interface */

/* led states */
#define LED_ON true
#define LED_OFF false

#define OUTPUT 0x0			/* setting GPIO direction to output */
#define CHANNEL1 1			/* channel 1 of the GPIO port */

#define ALL 0xFFFFFFFF		/* A value designating ALL leds */

static XGpio port_0_to_3;		/* GPIO port connected to the LED0-3 */
static XGpioPs port_4;
static XGpio port_6;			/* GPIO port connected to the LED6 (rgb) */

/*
 * Initialize the led module
 */
void led_init(void) {

	// Initialize LED 0-3
	XGpio_Initialize(&port_0_to_3, XPAR_AXI_GPIO_0_DEVICE_ID);		/* initialize device AXI_GPIO_0 */
	XGpio_SetDataDirection(&port_0_to_3, CHANNEL1, OUTPUT);	    /* set tristate buffer to output */

	// Initialize LED 4
	XGpioPs_Config *config = XGpioPs_LookupConfig(XPAR_AXI_GPIO_0_DEVICE_ID);
	XGpioPs_CfgInitialize(&port_4, config, config->BaseAddr);
	XGpioPs_SetDirectionPin(&port_4, 7, 1);
	XGpioPs_SetOutputEnablePin(&port_4, 7, 1);

	// Initialize LED 6
	XGpio_Initialize(&port_6, XPAR_AXI_GPIO_1_DEVICE_ID);			/* initialize device AXI_GPIO_1 */
	XGpio_SetDataDirection(&port_6, CHANNEL1, OUTPUT);	    		/* set tristate buffer to output */
}

/*
 * Set <led> to one of {LED_ON,LED_OFF,...}
 *
 * <led> is either ALL or a number >= 0
 * Does nothing if <led> is invalid
 */
void led_set(u32 led, bool tostate) {
	if ( (led >= 0) && (led <= 3) ) {
		// Setting LED 0-3
		u32 ledMask = 0x1 << led;
		u32 currStateLED = XGpio_DiscreteRead(&port_0_to_3, CHANNEL1);
		u32 nextStateLED = currStateLED ^ ledMask;
		XGpio_DiscreteWrite(&port_0_to_3, CHANNEL1, nextStateLED);
	} else if (led == 4) {
		// Setting LED 4
		if ( tostate == LED_ON ) {
			XGpioPs_WritePin(&port_4,7,1);
		}
		else {
			XGpioPs_WritePin(&port_4,7,0);
		}
	} else {
		// Setting all LEDs
		if ( (led == ALL) && (tostate == LED_ON) ) {
			XGpio_DiscreteWrite(&port_0_to_3, CHANNEL1, 0xF);
			XGpioPs_WritePin(&port_4,7,1);
		}
		else if ( (led == ALL) && (tostate == LED_OFF) ) {
			XGpio_DiscreteWrite(&port_0_to_3, CHANNEL1, 0x0);
			XGpioPs_WritePin(&port_4,7,0);
		}
	}
}

/*
 * Get the status of <led>
 *
 * <led> is a number >= 0
 * returns {LED_ON,LED_OFF,...}; LED_OFF if <led> is invalid
 */
bool led_get(u32 led) {
	if ( (led >= 0) && (led <= 3) ) {
		// Getting LED 0-3
		u32 setMask = 0x1 << led;
		u32 currStateLED = XGpio_DiscreteRead(&port_0_to_3, CHANNEL1);
		u32 currLED = (currStateLED & setMask) >> led;

		if ( currLED == 1 ) {
			return LED_ON;
		}
		else {
			return LED_OFF;
		}
	} else {
		// Getting LED 4
		u32 currLED4 = XGpioPs_ReadPin(&port_4,7);
		if ( currLED4 == 1 ) {
			return LED_ON;
		}
		else {
			return LED_OFF;
		}
	}
}

/*
 * Toggle <led>
 *
 * <led> is a value >= 0
 * Does nothing if <led> is invalid
 */
void led_toggle(u32 led) {
	bool currLED = led_get(led);
	if ( currLED == LED_ON ) {
		led_set(led, LED_OFF);
	}
	else {
		led_set(led, LED_ON);
	}
}

/*
 * Set color of LED6
 *
 * <color> is a character: r,g,b,y
 * Does nothing if <color> is invalid
 */
void led_rgb(char color) {
	switch(color) {
	case 'r' :
		// Drive the internal red LED
		XGpio_DiscreteWrite(&port_6, CHANNEL1, 0x4);
		printf("\nLED6 here... I'm red!");
		break;
	case 'g' :
		// Drive the internal green LED
		XGpio_DiscreteWrite(&port_6, CHANNEL1, 0x2);
		printf("\nLED6 here... I'm green!");
		break;
	case 'b' :
		// Drive the internal blue LED
		XGpio_DiscreteWrite(&port_6, CHANNEL1, 0x1);
		printf("\nLED6 here... I'm blue!");
		break;
	case 'y' :
		// Drive the internal red and green LEDs
		XGpio_DiscreteWrite(&port_6, CHANNEL1, 0x6);
		printf("\nLED6 here... I'm yellow!");
		break;
	case 'o' :
		// Drive the internal red and green LEDs
		XGpio_DiscreteWrite(&port_6, CHANNEL1, 0x0);
		printf("\nLED6 signing off!\n");
		break;
	default:
		printf("\nInvalid color!");
	}
}
