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

#define CONFIGURE 0
#define PING 1
#define UPDATE 2

static XUartPs uart0port; 					/* UART0 port instance */
static XUartPs uart1port; 					/* UART1 port instance */
static bool done = false;					/* UART interrupt status */
static int mode = CONFIGURE;				/* Wi-Fi module operation mode */

void mycallback(u32 val) {
	// Toggle LED 0-3 based on button and switch input
	led_set(ALL, LED_OFF);
	led_toggle(val);
	printf("LED%lu toggled!\n", val);

	if (val == 0) {
		// Enter CONFIGURE mode when button 0 pressed
		mode = CONFIGURE;
		printf("Mode: %d\n", mode);
		printf("< allows entry to wifi cmd mode >\n");
	} else if (val == 1) {
		// Enter PING mode when button 1 pressed
		mode = PING;
		printf("Mode: %d\n", mode);
		printf("[PING]\n");
	} else if (val == 2) {
		// Enter UPDATE mode when button 2 pressed
		mode = UPDATE;
		printf("Mode: %d\n", mode);
		printf("[UPDATE]\n");
	} else if (val == 3) {
		// Disconnect UART when button 3 pressed
		done = true;
	}
}

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
	XUartPs_DisableUart(&uart1port);
	XUartPs_DisableUart(&uart0port);
	gic_disconnect(XPAR_XUARTPS_0_INTR);
	gic_disconnect(XPAR_XUARTPS_1_INTR);
	gic_close();

	cleanup_platform();					/* cleanup the hardware platform */
	return 0;
}