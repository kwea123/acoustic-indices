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
#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "bsp.h"
#include "bsp_trace.h"


volatile uint32_t msTicks = 0; /* counts 1ms timeTicks */

/**************************************************************************//**
 * @brief SysTick_Handler
 * Interrupt Service Routine for system tick counter
 *****************************************************************************/
void SysTick_Handler(void) {
	msTicks++; /* increment counter necessary in Delay()*/
}

/**************************************************************************//**
 * @brief Delays number of msTick Systicks (typically 1 ms)
 * @param dlyTicks Number of ticks to delay
 *****************************************************************************/
void delay(uint16_t milliseconds) {
	/* Enable clock for TIMER1 */
	CMU->HFPERCLKEN0 |= CMU_HFPERCLKEN0_TIMER1;

	/* Set prescaler to maximum */
	TIMER1->CTRL = (TIMER1->CTRL & ~_TIMER_CTRL_PRESC_MASK) |  TIMER_CTRL_PRESC_DIV1024;

	/* Clear TIMER1 counter value */
	TIMER1->CNT = 0;

	/* Start TIMER1 */
	TIMER1->CMD = TIMER_CMD_START;

	/* Wait until counter value is over the threshold */
	while(TIMER1->CNT < 10*milliseconds){
	/* Do nothing, just wait */
	}

	/* Stop TIMER */
	TIMER1->CMD = TIMER_CMD_STOP;
}


void setLED(int pin, int value) {
	GPIO_PinModeSet(gpioPortC, pin, gpioModePushPull, value);
}

/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void) {
	/* Chip errata */
	CHIP_Init();

	/* If first word of user data page is non-zero, enable eA Profiler trace */
	BSP_TraceProfilerSetup();

	GPIO_PinModeSet(gpioPortC, 0, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortC, 1, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortC, 2, gpioModeWiredAndPullUp, 1);
	GPIO_PinModeSet(gpioPortC, 3, gpioModeWiredAndPullUp, 1);
	GPIO_PinModeSet(gpioPortC, 4, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortC, 5, gpioModePushPull, 0);
	GPIO_PinModeSet(gpioPortC, 6, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortC, 7, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortC, 8, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortC, 9, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortC, 10, gpioModeDisabled, 0);
	GPIO_PinModeSet(gpioPortC, 11, gpioModeDisabled, 0);

	/* Initialize LED driver */
	BSP_LedsInit();
	BSP_LedSet(0);

	/* Infinite blink loop */
	while (1) {
		setLED(5, 1);
		delay(1000);
		setLED(5, 0);
		delay(1000);
	}
}
