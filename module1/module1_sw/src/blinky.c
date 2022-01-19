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

#define OUTPUT 0x0							/* setting GPIO direction to output */
#define CHANNEL1 1							/* channel 1 of the GPIO port */


void buffer_read(char str[]) {

	int i;
	char c;

	// Read, save, and echo character until newline
	i = 0;
	printf(">");
	fflush(stdout);

	c = getchar();
	while (c != '\r') {
		putchar(c);
		str[i] = c;
		i++;
		c = getchar();
	};
	str[i] = '\0';

}

int main() {
   //XGpio port;									/* GPIO port connected to the leds */

   init_platform();								/* initialize the hardware platform */

	 /* 
		* set stdin unbuffered, forcing getchar to return immediately when
		* a character is typed.
		*/
	 setvbuf(stdin,NULL,_IONBF,0);
	 
	 printf("[Hello]\n");
	 
	 //XGpio_Initialize(&port, XPAR_AXI_GPIO_0_DEVICE_ID);	/* initialize device AXI_GPIO_0 */
  	 //XGpio_SetDataDirection(&port, CHANNEL1, OUTPUT);	    /* set tristate buffer to output */
  	 //XGpio_DiscreteWrite(&port, CHANNEL1, 0x1);			/* turn on led0 */

	 led_init();											/* initialize device AXI_GPIO_0 */
	 led_set(ALL, LED_ON);									/* turn on ledx */

	 char str[80];
	 char color;
	 int num;
	 char* np;
	 //u32 blink;

	 // Read in input strings
	do {
		buffer_read(str);

		// Check if string is controlling LED 0-4
		num = (int) strtol(str, &np, 10);
		if ( (num >= 0) && (num <= 4) && (np != str) ) {

			// Toggle LEDs
			led_toggle(num);
			printf("\nToggling LED%d...\n", num);
			if ( led_get(num) == 1 ) {
				printf("LED%d is on!", num);
			}
			else {
				printf("LED%d is off!", num);
			}
		}

		// Check if string is controlling LED 6
		color = str[0];
		if ( (strcmp("r", str) == 0) ||
				(strcmp("g", str) == 0) ||
				(strcmp("b", str) == 0) ||
				(strcmp("y", str) == 0) ||
				(strcmp("o", str) == 0) ) {
			led_rgb(color);
		}

//		// Toggle LED0 each time '0' is entered
//		//blink = XGpio_DiscreteRead(&port, CHANNEL1);
//		blink = led_get(0);
//		if ( (strcmp("0", str) == 0) && (blink == 1) ) {
//			//XGpio_DiscreteWrite(&port, CHANNEL1, 0x0);				/* turn off led0 */
//			led_set(0, LED_OFF);
//		}
//		else if ( (strcmp("0", str) == 0) && (blink == 0) ) {
//			//XGpio_DiscreteWrite(&port, CHANNEL1, 0x1);				/* turn on led0 */
//			led_set(0, LED_ON);
//		}

		printf("\n");
	} while (strcmp("q",str) != 0);

	 // Turn off LED0 prior to program exit
	 //XGpio_SetDataDirection(&port, CHANNEL1, OUTPUT);	    			/* set tristate buffer to output */
	 //XGpio_DiscreteWrite(&port, CHANNEL1, 0x0);						/* turn off led0 */
	 led_set(ALL, LED_OFF);												/* turn off ledx */
	 led_rgb('o');														/* turn off led6 */

   cleanup_platform();					/* cleanup the hardware platform */
   return 0;
}
