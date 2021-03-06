/*
 * blinky.c -- working with Serial I/O and GPIO
 *
 * Assumes the LED's are connected to AXI_GPIO_0, on channel 1
 *
 * Terminal Settings:
 *  -Baud: 115200
 *  -Data bits: 8
 *  -Parity: no
 *  -Stop bits: 1
 */
#include <stdio.h>							/* printf(), fflush(stdout), getchar() */
#include <stdlib.h>							/* strtol() */
#include <strings.h>						/* strcmp() */
#include "xil_types.h"						/* u32, u16 etc */
#include "platform.h"						/* ZYBOboard interface */
#include <xgpio.h>							/* Xilinx GPIO functions */
#include "xparameters.h"					/* constants used by the hardware */
#include "led.h"							/* led module interface */
#include "gic.h"							/* interrupt controller interface */
#include "io.h"								/* button and switch module interface */

#define OUTPUT 0x0							/* setting GPIO direction to output */
#define CHANNEL1 1							/* channel 1 of the GPIO port */


void buffer_read(char str[]) {
	// Read, save, and echo character until newline
	int i = 0;
	printf(">");
	fflush(stdout);

	char c = getchar();
	while (c != '\r') {
		putchar(c);
		str[i] = c;
		i++;
		c = getchar();
	};
	str[i] = '\0';

}

void mycallback(u32 val) {
	led_toggle(val);
	printf("LED%lu toggled!\n", val);
}

int main() {

	/* Initialize hardware platform and I/O	 */
	init_platform();
	gic_init();
	led_init();
	io_btn_init(&mycallback);
	io_sw_init(&mycallback);
	led_set(4, LED_ON);

	 /* 
		* set stdin unbuffered, forcing getchar to return immediately when
		* a character is typed.
		*/
	 setvbuf(stdin,NULL,_IONBF,0);
	 
	 printf("\n[Hello]\n");

	 char str[80];
	 int num;
	 char* np;

	 // Read in input strings
	do {
		buffer_read(str);

		// Check if string is controlling LED 0-4
		num = (int) strtol(str, &np, 10);
		if ( (num >= 0) && (num <= 3) && (np != str) ) {
			// Toggle LEDs
			led_toggle(num);
			printf("\nLED%lu toggled!", num);
		}
		printf("\n");
	} while (strcmp("q",str) != 0);

	// Turn off all I/O
	led_set(ALL, LED_OFF);
	io_btn_close();
	io_sw_close();
	gic_close();

	cleanup_platform();					/* cleanup the hardware platform */
	return 0;
}
