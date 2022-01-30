/*
 * adc.c
 */

#include "adc.h"			/* ADC module interface */

static XAdcPs adcport;		/* ADC port instance */
static const u32 ADC_CH_OPTIONS = 0x0 | XADCPS_SEQ_CH_TEMP | XADCPS_SEQ_CH_VCCINT | XADCPS_SEQ_CH_AUX14;

/*
 * initialize the adc module
 */
void adc_init(void) {
	// Initialize ADC peripheral
	XAdcPs_Config *config = XAdcPs_LookupConfig(XPAR_XADCPS_0_DEVICE_ID);
	XAdcPs_CfgInitialize(&adcport, config, config->BaseAddress);

	// Exit the program if the XADC port fails
	int test = XAdcPs_SelfTest(&adcport);
	if (test == XST_FAILURE) {
		return;
	}

	// Set channel sequencer mode to 'Safe' and disable sampling/alarms
	XAdcPs_SetSequencerMode(&adcport, XADCPS_SEQ_MODE_SAFE);
	XAdcPs_SetAlarmEnables(&adcport, 0x0);

	// Enable alarms channels for internal temp, voltage, and aux input
	XAdcPs_SetSeqChEnables(&adcport, ADC_CH_OPTIONS);

	// Set channel sequencer mode to 'Continuous' before sampling values
	XAdcPs_SetSequencerMode(&adcport, XADCPS_SEQ_MODE_CONTINPASS);
}

/*
 * get the internal temperature in degree's centigrade
 */
float adc_get_temp(void) {
	u16 raw = XAdcPs_GetAdcData(&adcport, XADCPS_CH_TEMP);
	float temp = XAdcPs_RawToTemperature(raw);
	return temp;
}

/*
 * get the internal vcc voltage (should be ~1.0v)
 */
float adc_get_vccint(void) {
	u16 raw = XAdcPs_GetAdcData(&adcport, XADCPS_CH_VCCINT);
	float volt = XAdcPs_RawToVoltage(raw);
	return volt;
}

/*
 * get the **corrected** potentiometer voltage (should be between 0 and 1v)
 */
float adc_get_pot(void) {

}
