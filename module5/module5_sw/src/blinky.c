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
#include <xgpio.h>							/* Xilinx GPIO functions */
#include <xuartps.h>						/* Xilinx UART functions */
#include "xil_types.h"						/* u32, u16 etc */
#include "platform.h"						/* ZYBOboard interface */
#include "xparameters.h"					/* constants used by the hardware */
#include "led.h"							/* led module interface */
#include "gic.h"							/* interrupt controller interface */
#include "io.h"								/* button and switch module interface */
//#include "ttc.h"							/* ttc module interface */
//#include "servo.h"						/* servo module interface */
//#include "adc.h"							/* ADC module interface */

#define OUTPUT 0x0							/* setting GPIO direction to output */
#define CHANNEL1 1							/* channel 1 of the GPIO port */
#define FREQ 1								/* 1Hz frequency for LED4 */

static XUartPs uart0port; 					/* UART0 port instance */
static XUartPs uart1port; 					/* UART1 port instance */
static bool done = false;					/* UART interrupt status */

#define CONFIGURE 0
#define PING 1
#define UPDATE 2

//void buffer_read(char str[]) {
//	// Read, save, and echo character until newline
//	int i = 0;
//	printf(">");
//	fflush(stdout);
//
//	char c = getchar();
//	while (c != '\r') {
//		putchar(c);
//		str[i] = c;
//		i++;
//		c = getchar();
//	};
//	str[i] = '\0';
//}

void mycallback(u32 val) {
	// Toggle LED 0-3 based on button and switch input
	led_toggle(val);
	printf("LED%lu toggled!\n", val);

//	// Display internal temperature when button 0 pressed
//	if (val == 0) {
//		float temp = adc_get_temp();
//		printf("[Temperature=%4.2f]\n", temp);
//		fflush(stdout);
//	}
//
//	// Display internal voltage when button 1 pressed
//	if (val == 1) {
//		float volt = adc_get_vccint();
//		printf("[VccInt=%3.2f]\n", volt);
//		fflush(stdout);
//	}
//
//	// Display potentiometer voltage when button 2 pressed
//	if (val == 2) {
//		float pot = adc_get_pot();
//		printf("[Pot=%3.2f]\n", pot);
//		fflush(stdout);
//	}
//
//	// Drive PWM duty cycle with pot voltage when button 3 pressed
//	if (val == 3) {
//		float pot_volt = adc_get_pot();
//		double pot_pwm = (pot_volt * 4.75) + 7;
//
//		// Caps PWM duty cycle to mechanical limits
//		if (pot_pwm > ARC_STOP_DUTY_CYCLE) {
//			pot_pwm = ARC_STOP_DUTY_CYCLE;
//		} else if (pot_pwm < ARC_START_DUTY_CYCLE) {
//			pot_pwm = ARC_START_DUTY_CYCLE;
//		}
//
//		servo_set(pot_pwm);
//		printf("[Pot=%3.2f]\n", pot_volt);
//		fflush(stdout);
//	}

	// Disconnect UART when button 3 pressed
	if (val == 3) {
		done = true;
	}
}

//void ttc_callback(void) {
//	// Toggle LED 4 every second
//	led_toggle(4);
//}

// Forward any characters received by UART0 to UART1
void uart1_handler(void *CallBackRef, u32 Event, unsigned int EventData) {

	XUartPs *dev = (XUartPs*)CallBackRef;
	u8 charbuff;

	// Check if receive data has been triggered
	if (Event == XUARTPS_EVENT_RECV_DATA) {
		XUartPs_Recv(dev, &charbuff, 1);
		//XUartPs_Send(dev, &charbuff, 1);
		XUartPs_Send(&uart0port, &charbuff, 1);

		// Send a newline when carriage return is received
		if (charbuff == (u8)'\r') {
			charbuff = (u8)'\n';
			XUartPs_Send(dev, &charbuff, 1);
		}
	}
}

// Forward any characters received by UART1 to UART0
void uart0_handler(void *CallBackRef, u32 Event, unsigned int EventData) {

	XUartPs *dev = (XUartPs*)CallBackRef;
	u8 charbuff;

	// Check if receive data has been triggered
	if (Event == XUARTPS_EVENT_RECV_DATA) {
		XUartPs_Recv(dev, &charbuff, 1);
		XUartPs_Send(&uart1port, &charbuff, 1);
	}
}

int main() {

	/* Initialize hardware platform and I/O	 */
	init_platform();
	gic_init();
	led_init();
	io_btn_init(&mycallback);
	io_sw_init(&mycallback);
//	ttc_init(FREQ, &ttc_callback);
//	ttc_start();
//	adc_init();

	/* Initialize UART1 */
	XUartPs_Config *config1 = XUartPs_LookupConfig(XPAR_PS7_UART_1_DEVICE_ID);
	XUartPs_CfgInitialize(&uart1port, config1, config1->BaseAddress);
	XUartPs_SetFifoThreshold(&uart1port,1);
	XUartPs_SetInterruptMask(&uart1port,XUARTPS_IXR_RXOVR);

	/* Connect UART1 interrupt handler to GIC*/
	XUartPs_SetHandler(&uart1port, (XUartPs_Handler)uart1_handler, &uart1port);
	gic_connect(XPAR_XUARTPS_1_INTR, (Xil_InterruptHandler)XUartPs_InterruptHandler, &uart1port);

	/* Initialize UART0 */
	XUartPs_Config *config0 = XUartPs_LookupConfig(XPAR_PS7_UART_0_DEVICE_ID);
	XUartPs_CfgInitialize(&uart0port, config0, config0->BaseAddress);
	XUartPs_SetBaudRate(&uart0port, 9600); // Wi-Fi module default baud rate
	XUartPs_SetFifoThreshold(&uart0port,1);
	XUartPs_SetInterruptMask(&uart0port,XUARTPS_IXR_RXOVR);

	/* Connect UART0 interrupt handler to GIC*/
	XUartPs_SetHandler(&uart0port, (XUartPs_Handler)uart0_handler, &uart0port);
	gic_connect(XPAR_XUARTPS_0_INTR, (Xil_InterruptHandler)XUartPs_InterruptHandler, &uart0port);

	 /* 
		* set stdin unbuffered, forcing getchar to return immediately when
		* a character is typed.
		*/
	 setvbuf(stdin,NULL,_IONBF,0);
	 
	 printf("\n[Hello]\n");

//	 servo_init();

//	 char str[80];
//	 int num;
//	 char* np;
//
//	 // Read in input strings
//	do {
//		buffer_read(str);
//
//		// Control LED 0-4
//		num = (int) strtol(str, &np, 10);
//		if ( (num >= 0) && (num <= 3) && (np != str) ) {
//			// Toggle LEDs
//			led_toggle(num);
//			printf("\nLED%d toggled!", num);
//		}
//		printf("\n");
//
//		// Control PWM duty cycle in 0.25% increments
//		if (strcmp("a", str) == 0) {
//			servo_set(servo_get() + 0.25);
//		} else if (strcmp("s", str) == 0) {
//			servo_set(servo_get() - 0.25);
//		}
//
//		// Control servo drive over 90 degree arc
//		if (strcmp("low", str) == 0) {
//			servo_set(ARC_START_DUTY_CYCLE);
//		} else if (strcmp("high", str) == 0) {
//			servo_set(ARC_STOP_DUTY_CYCLE);
//		}
//	} while (strcmp("q",str) != 0);

	// Wait for interrupts from UART handler
	while(!done) {
	 sleep(1);
	}
	printf("[done]\n");
	sleep(1);

	// Turn off all I/O
	led_set(ALL, LED_OFF);
	io_btn_close();
	io_sw_close();
//	ttc_stop();
//	ttc_close();
	XUartPs_DisableUart(&uart1port);
	XUartPs_DisableUart(&uart0port);
	gic_disconnect(XPAR_XUARTPS_0_INTR);
	gic_disconnect(XPAR_XUARTPS_1_INTR);
	gic_close();

	cleanup_platform();					/* cleanup the hardware platform */
	return 0;
}
