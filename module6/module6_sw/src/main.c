///*
// * main.c -- working with Serial I/O and GPIO
// *
// * Assumes the LED's are connected to AXI_GPIO_0, on channel 1,
// * 			   button are connected to AXI_GPIO_1, on channel 1,
// * 			   switches are connected to AXI_GPIO_2 on channel 1
// *
// * Terminal Settings:
// *  -Baud: 115200
// *  -Data bits: 8
// *  -Parity: no
// *  -Stop bits: 1
// */
//#include <stdio.h>
//#include <stdlib.h>
//#include <strings.h>
//#include <xgpio.h>							/* Xilinx GPIO functions */
//#include <xuartps.h>						/* Xilinx UART functions */
//#include "xil_types.h"						/* u32, u16 etc */
//#include "platform.h"						/* ZYBOboard interface */
//#include "xparameters.h"					/* constants used by the hardware */
//#include "led.h"							/* led module interface */
//#include "gic.h"							/* interrupt controller interface */
//#include "io.h"								/* button and switch module interface */
//#include "servo.h"							/* servo module interface */
//#include "adc.h"							/* ADC module interface */
////#include "wifi.h"							/* WiFi module interace */
//
///* Possible States */
//#define CONFIGURE 0
//#define PING 1
//#define UPDATE 2
//#define READ 3
//
///* Current State of Controller */
//static int state;
//
///* State Transition Function */
//static void change_state() {
//	switch(state) {					// based on current state
//	case CONFIGURE:					// look at other inputs
//		state = <new_state>;		// change state and generate outputs
//	case PING:
//		state = <new_state>;
//	case UPDATE:
//		state = <new_state>;
//	break;
//	}
//}
//
///* Example Callback Function */
//static void btn_callback(int btn) {
//	// Change state when an interrupt occurs
//	change_state();
//}
//
//int main() {
//
//	/* Initialize hardware platform and I/O	 */
//	init_platform();
//	gic_init();
//	led_init();
//	adc_init();
//	servo_init();
//	wifi_init();
//	io_btn_init(&mycallback);
//	io_sw_init(&mycallback);
//
//	/* Set initial state */
//	state = CONFIGURE;
//	printf("\n[Hello]\n");
//
//	/* Main loop does nothing */
//	while(!done) {
//		sleep(1);		// Do nothing and wait for interrupts
//	}
//	printf("[done]\n");
//	sleep(1);
//
//
//	// Turn off all I/O and cleanup hardware
//	led_set(ALL, LED_OFF);
//	io_btn_close();
//	io_sw_close();
//	XUartPs_DisableUart(&uart1port);
//	XUartPs_DisableUart(&uart0port);
//	gic_disconnect(XPAR_XUARTPS_0_INTR);
//	gic_disconnect(XPAR_XUARTPS_1_INTR);
//	gic_close();
//
//	cleanup_platform();
//	return 0;
//}
