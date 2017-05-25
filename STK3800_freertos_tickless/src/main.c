/**************************************************************************//**
 * @file
 * @brief Simple LED Blink Demo for EFM32WG_STK3800
 * @version 5.1.3
 ******************************************************************************
 * @section License
 * <b>Copyright 2015 Silicon Labs, Inc. http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "bsp.h"
#include "bsp_trace.h"
#include "segmentlcd.h"

#include "arm_math.h"
#include "arm_const_structs.h"

#include <stdio.h>

volatile uint32_t msTicks = 0; /* counts 1ms timeTicks */
const float32_t pi = 3.1415926f;
const float32_t amp_thr = 1.2589f; //amplitude equivalent to 2 dB

/**************************************************************************//**
 * @brief SysTick_Handler
 * Interrupt Service Routine for system tick counter
 *****************************************************************************/
void SysTick_Handler(void) {
	msTicks++; /* increment counter necessary in Delay()*/
}

float32_t fastlog2(float x) {
	union {
		float f;
		uint32_t i;
	} vx = { x };
	union {
		uint32_t i;
		float f;
	} mx = { (vx.i & 0x007FFFFF) | 0x3f000000 };
	float y = vx.i;
	y *= 1.1920928955078125e-7f;

	return y - 124.22551499f - 1.498030302f * mx.f
			- 1.72587999f / (0.3520887068f + mx.f);
}

float32_t fasterlog2(float x) {
	union {
		float f;
		uint32_t i;
	} vx = { x };
	float y = vx.i;
	y *= 1.1920928955078125e-7f;
	return y - 126.94269504f;
}

/*****************************************************************************
 * data size = 2*fbs
 * ***************************************************************************/
uint32_t proceed(int id, uint32_t fbs, float32_t* data, float32_t* aci_previous,
		float32_t* aci_sumDiff, float32_t* sum, float32_t* h_t_h,
		float32_t* sumSquared, float32_t* cvr_noise, float32_t* cvr_count) {
	uint32_t curTicks = msTicks;

	arm_rfft_fast_instance_f32 S1;
	arm_rfft_fast_init_f32(&S1, fbs); //initialize the size

//  arm_rfft_instance_q15 S2;
//  arm_rfft_init_q15(&S2, fbs, 0, 1); //initialize
//
//  q15_t data_q15[size], output0[fbs];
//  arm_float_to_q15(data, data_q15, size);

//  float32_t output[fbs];

	arm_rfft_fast_f32(&S1, data, data, 0);
//	  arm_rfft_q15(&S2, data_q15, data_q15);		//output 8.8 for fbs=256
//	  arm_shift_q15(data_q15, 7, data_q15, size);      //convert to 1.15
//	  arm_cmplx_mag_q15(data_q15, output0, fbs);    //output 2.14
//	  arm_shift_q15(output0, 1, output0, fbs);      //convert to 1.15
//	  arm_q15_to_float(output0, output, fbs);

	arm_cmplx_mag_f32(data, data, fbs);

	for (int j = 0; j < fbs; j++) { //for each freq bin
		float32_t a_j = data[j]; //current amplitude
		float32_t a_j_2 = a_j * a_j; //squared amp

		if (id != 0) //if it's not the first time frame
			aci_sumDiff[j] += fabsf(a_j - aci_previous[j]);

		float32_t factor_j = sumSquared[j] / (sumSquared[j] + a_j_2);
//	h_t_h[j] = h_t_h[j]*factor_j-factor_j*fasterlog2(factor_j)-(1.0f-factor_j)*fasterlog2(1.0f-factor_j);
		float32_t factor2_j = (factor_j - 0.5f) * (factor_j - 0.5f);
		float32_t factor4_j = factor2_j * factor2_j;
		h_t_h[j] = h_t_h[j] * factor_j - 96.0f * factor4_j * factor4_j
				- 2.5f * factor2_j + 1.0005f; // approx by a degree 8 polynomial err<=0.004 between 0.998 and 1
		// h_t_true ~= h_t_approx * 1.13043478f (1.3/1.15)
		if (a_j - cvr_noise[j] > amp_thr)
			cvr_count[j]++;

		aci_previous[j] = a_j;
		sum[j] += a_j;
		sumSquared[j] += a_j_2;

	}

	return msTicks - curTicks;

}

/*******************************
 * modify here
 *******************************/
float32_t* get_data() {
	return NULL;
}

void write_to_sd_card(float32_t* aci, float32_t* h_t, float32_t* cvr){

}

void reset(int num, ...) {
	int fbs = 256;
	va_list valist;

	/* initialize valist for num number of arguments */
	va_start(valist, num);

	/* access all the arguments assigned to valist */
	for (int i = 0; i < num; i++) {
		memset(va_arg(valist, int), 0.0f, fbs * sizeof(float32_t));
	}
	va_end(valist);
}

int _write(int file, const char *ptr, int len) {
	int x;
	for (x = 0; x < len; x++)
		ITM_SendChar(*ptr++);
	return (len);
}

void SWO_SetupForPrint(void) {
	/* Enable GPIO clock. */
	CMU->HFPERCLKEN0 |= CMU_HFPERCLKEN0_GPIO;
	/* Enable Serial wire output pin */
	GPIO->ROUTE |= GPIO_ROUTE_SWOPEN;
#if defined(_EFM32_GIANT_FAMILY) || defined(_EFM32_LEOPARD_FAMILY) ||         defined(_EFM32_WONDER_FAMILY) || defined(_EFM32_GECKO_FAMILY)
	/* Set location 0 */
	GPIO->ROUTE = (GPIO->ROUTE & ~(_GPIO_ROUTE_SWLOCATION_MASK))
			| GPIO_ROUTE_SWLOCATION_LOC0;
	/* Enable output on pin - GPIO Port F, Pin 2 */
	GPIO->P[5].MODEL &= ~(_GPIO_P_MODEL_MODE2_MASK);
	GPIO->P[5].MODEL |= GPIO_P_MODEL_MODE2_PUSHPULL;
#else
	/* Set location 1 */
	GPIO->ROUTE = (GPIO->ROUTE & ~(_GPIO_ROUTE_SWLOCATION_MASK)) |GPIO_ROUTE_SWLOCATION_LOC1;
	/* Enable output on pin */
	GPIO->P[2].MODEH &= ~(_GPIO_P_MODEH_MODE15_MASK);
	GPIO->P[2].MODEH |= GPIO_P_MODEH_MODE15_PUSHPULL;
#endif
	/* Enable debug clock AUXHFRCO */
	CMU->OSCENCMD = CMU_OSCENCMD_AUXHFRCOEN;
	/* Wait until clock is ready */
	while (!(CMU->STATUS & CMU_STATUS_AUXHFRCORDY))
		;
	/* Enable trace in core debug */
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	ITM->LAR = 0xC5ACCE55;
	ITM->TER = 0x0;
	ITM->TCR = 0x0;
	TPI->SPPR = 2;
	TPI->ACPR = 0xf;
	ITM->TPR = 0x0;
	DWT->CTRL = 0x400003FE;
	ITM->TCR = 0x0001000D;
	TPI->FFCR = 0x00000100;
	ITM->TER = 0x1;
}

int main(void) {
	/* Chip errata */
	CHIP_Init();

	/* If first word of user data page is non-zero, enable eA Profiler trace */
	BSP_TraceProfilerSetup();

	/* Setup SysTick Timer for 1 msec interrupts  */
	if (SysTick_Config(CMU_ClockFreqGet(cmuClock_CORE) / 1000))
		while (1)
			;

	SWO_SetupForPrint();

	uint32_t size = 512;
	uint32_t fbs = 256; //number of freq bins

	float32_t data[size];

	for (int i = 0; i < size; i++) //create sine wave example
		data[i] = 1.2f * arm_sin_f32(0.1f * pi * i); //50hz at 1000hz sample rate

//  SegmentLCD_Init(false);
//  SegmentLCD_LowerNumber(proceed(size,fbs,test,testOutput));

	float32_t aci_previous[fbs], aci_sumDiff[fbs], sum[fbs]; //numbers for aci
	float32_t h_t_h[fbs], sumSquared[fbs]; //numbers for h_t
	float32_t cvr_noise[fbs], cvr_count[fbs]; //numbers for cvr

	float32_t aci[fbs], h_t[fbs], cvr[fbs];

	int fs = 48000; //sample rate
	int time_span = 60; //segments of one minute
	int N = fs * time_span / size;

//  printf("%d\n",proceed(1,fbs,test,aci_previous,aci_sumDiff,sum,h_t_h,sumSquared,cvr_noise,cvr_count));
	while(1){
		for (int i = 0; i < N; i++){ //proceed the numbers for a minute
			data = get_data();
			proceed(i, fbs, data, aci_previous, aci_sumDiff, sum, h_t_h, sumSquared,
					cvr_noise, cvr_count);
		}
		for (int i = 0; i < fbs; i++) { //compute the indices
			aci[i] = aci_sumDiff[i] / sum[i];
			h_t[i] = h_t_h[i] / fastlog2(N);
			cvr[i] = cvr_count[i] / N;
		}
		reset(7, aci_previous, aci_sumDiff, sum, h_t_h, sumSquared, cvr_noise, cvr_count);
		write_to_sd_card(aci,h_t,cvr);
	}

//  for(int j=0;j<fbs;j++)
//	  printf("%f\n",test[j]);
}
