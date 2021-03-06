/*
 * wifi.c
 */
#include <stdlib.h>								/* atoi() */
#include "servo.h"								/* servo module interface */
#include "adc.h"								/* ADC module interface */
#include "wifi.h"								/* WiFi module interface */
#include "gic.h"								/* interrupt controller interface */

static XUartPs uart0port; 						/* UART0 port instance */
static XUartPs uart1port; 						/* UART1 port instance */
static int wifi_mode = CONFIGURE;				/* WiFi module operation mode */

//static int updateval;
static int buffcount = 0;						// for receiving
static u8 pingbuff[sizeof(ping_t)];
static u8 updatebuff[sizeof(update_response_t)];

static float pot_voltage;
static int pot_percent;
static int crossing_status;

/*
 * Get train crossing status from remote substation
 */
int get_crossing_status(void) {
	return crossing_status;
}

/*
 * Initialize UART communication
 */
void uart_init(void) {

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

//	saved_uart_callback = uart_callback;
}

/*
 * Close UART communication
 */
void uart_close(void) {
	/* disconnect the interrupts (c.f. gic.h) */
	XUartPs_DisableUart(&uart1port);
	XUartPs_DisableUart(&uart0port);
	gic_disconnect(XPAR_XUARTPS_0_INTR);
	gic_disconnect(XPAR_XUARTPS_1_INTR);
}

/*
 * Sends ping message to local server over UART
 */
void send_ping(void) {
	// Declare and initialize ping message to be sent
	ping_t ping;
	ping.type = PING;
	ping.id = SERVER_ID;
	XUartPs_Send(&uart0port, (u8*) &ping, sizeof(ping_t));
}

/*
 * Sends update message to local server over UART
 */
void send_update() {

	// Read in potentiometer voltage
	pot_voltage = adc_get_pot();
	pot_percent = (int)(100 * pot_voltage);
//		printf("[Pot Voltage=%3.2f]\n", pot_voltage);
//		printf("[Pot Voltage=%3.2d%%]\n", pot_percent);

	// Convert string pointer value to integer
	//updateval = atoi((char*) value);
	//printf("Update Value: %d\n", updateval);

	// Declare and initialize update message to be sent
	update_request_t update_request;
	update_request.type = UPDATE;
//	update_request.id = SERVER_ID;
//	update_request.value = pot_percent;
	update_request.id = 0;
	update_request.value = 0;

	wifi_mode = UPDATE;
	XUartPs_Send(&uart0port, (u8*) &update_request, sizeof(update_request_t));
}

///*
// * Changes WiFi mode to reading update request
// */
//void changeToRead(void) {
//	wifi_mode = READ;		// Change to read update mode
//	charbuff = updatebuff;	// Set character buffer to start of update buffer
//}
//

/*
 * Handles UART1 communication from keyboard input
 */
void uart1_handler(void *CallBackRef, u32 Event, unsigned int EventData) {

	XUartPs *dev = (XUartPs*)CallBackRef;
//	u8 newline = (u8)'\n';		// newline to signal end of keyboard input
//	u8 nullchar = (u8)'\0';		// null character to mark enf of buffer
	u8 buffer;
	// Check if receive data has been triggered
	if (Event == XUARTPS_EVENT_RECV_DATA) {
		XUartPs_Recv(dev, &buffer, 1);

		// Read in any keyboard input when in CONFIGURE mode
		if (wifi_mode == CONFIGURE) {
			XUartPs_Send(&uart0port, &buffer, 1);
		}
		// Send a newline when carriage return is received
		XUartPs_Send(&uart1port, &buffer, 1);
		if (buffer == (u8)'\r') {
			buffer = (u8)'\n';
			XUartPs_Send(dev, &buffer, 1);
		}
		// Read in numerical keyboard input when in READ mode
//		if (wifi_mode == READ) {
//			XUartPs_Send(dev, charbuff, 1);
//			if (*charbuff == (u8)'\r') {;
//				XUartPs_Send(dev, &newline, 1);
//				*charbuff = nullchar;
//				send_update(updatebuff);
//			} else {
//				charbuff++;
//			}
//		}
 	}
}
//
/*
 * Handles UART0 communication from WiFi module
 */
// Forward any characters received by UART1 to UART0
void uart0_handler(void *CallBackRef, u32 Event, unsigned int EventData) {
	XUartPs *dev = (XUartPs*)CallBackRef;
	u8 charbuff;

	// Check if receive data has been triggered
	if (Event == XUARTPS_EVENT_RECV_DATA) {
		XUartPs_Recv(dev, &charbuff, 1);

//		if (wifi_mode == CONFIGURE) {
//			// Echo back keyboard input if in configure mode
//			XUartPs_Send(&uart1port, &charbuff, 1);
//		} else if (wifi_mode == PING) {
//			//	Display decoded ping message from server
//			pingbuff[buffcount] = charbuff;
//			buffcount++;
//			if (buffcount == sizeof(ping_t)) {
//				buffcount = 0;
////				sprintf(buffer,"[PING,id=%d]\r\n", ((ping_t*)pingbuff)->id);
////				buffer[15] = '\0';
////				XUartPs_Send(&uart1port, (u8*)buffer, strlen(buffer));
//				int id =((ping_t*)pingbuff)->id;
//				printf("[PING,id=%d]\n",id);
//			}
//		} else if (wifi_mode == UPDATE) {
			// Display decoded update message from server
			updatebuff[buffcount] = charbuff;
			buffcount++;
			if (buffcount == sizeof(update_response_t)) {
				buffcount = 0;
//				printf("[UPDATE,id=%d,average=%d,value=%d]\n", ((update_response_t*)updatebuff)->id, ((update_response_t*)updatebuff)->average, ((update_response_t*)updatebuff)->values[25]);
				crossing_status = ((update_response_t*)updatebuff)->values[25];

				// Set servo motor according to pot voltage
				double pot_voltage_to_duty_cycle = (ARC_STOP_DUTY_CYCLE - ARC_START_DUTY_CYCLE) / 100;
				double duty_cycle = (double)(pot_voltage_to_duty_cycle*((update_response_t*)updatebuff)->values[25])+7;
				servo_set(duty_cycle);
//				printf("Duty Cycle: %0.2f\n", duty_cycle);

//			}
		}
	}
}
