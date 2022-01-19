///*
// * main.c -- A program to print a dot each time button 0 is pressed.
// *
// *  Some useful values:
// *  -- XPAR_AXI_GPIO_1_DEVICE_ID -- xparameters.h
// *  -- XPAR_FABRIC_GPIO_1_VEC_ID -- xparameters.h
// *  -- XGPIO_IR_CH1_MASK         -- xgpio_l.h (included by xgpio.h)
// */
//#include <stdio.h>		/* getchar,printf */
//#include <stdlib.h>		/* strtod */
//#include <stdbool.h>	/* type bool */
//#include <unistd.h>		/* sleep */
//#include <string.h>
//
//#include "platform.h"		/* ZYBO board interface */
//#include "xil_types.h"		/* u32, s32 etc */
//#include "xparameters.h"	/* constants used by hardware */
//
//#include "gic.h"			/* interrupt controller interface */
//#include "led.h"			/* led module interface */
//#include "io.h"				/* switch and button module interface */
//
//#define CHANNEL1 1			/* channel 1 of the GPIO port */
//
///* hidden private state */
//static int pushes;	       	   /* variable used to count interrupts */
//
//
//void mycallback(u32 val) {
//	led_toggle(val);
//	printf("LED%lu toggled!\n", val);
//}
//
//int main() {
//	// Initialize hardware platform and I/O
//	init_platform();
//	led_init();
//	io_btn_init(&mycallback);
//	io_sw_init(&mycallback);
//
//	// Terminal interactions
//	printf("\n[hello]\n"); /* so we are know its alive */
//	pushes = 0;
//	pushes++;
//	while(pushes<5) /* do nothing and handle interrupts */
//	  ;
//
//	printf("\n[done]\n");
//
//	// Turn off all I/O
//	led_set(ALL, LED_OFF);
//	io_btn_close();
//	io_sw_close();
//
//	cleanup_platform();					/* cleanup the hardware platform */
//	return 0;
//}

