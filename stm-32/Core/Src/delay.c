/*******************************************************************************
* EE 329 CP: Star Tracker
*******************************************************************************
* @file           : delay.c
* @brief          : Delay utility for microsecond timing
*
* project         : EE 329 S'25 Assignment 4
* authors         : Laken Baker, Lenna Cabello Sosa, Preston Mavady
* version         : 0.1
* date            : Jun 4, 2025
* compiler        : STM32CubeIDE v.1.12.0 Build: 14980_20230301_1550 (UTC)
* target          : NUCLEO-L4A6ZG
* clocks          : 4 MHz MSI to AHB2
* @attention      : (c) 2023 STMicroelectronics.  All rights reserved.
*******************************************************************************/

#include "main.h"
#include "Delay.h"
#include "stm32l4xx.h"
#include <stdint.h>

/* -----------------------------------------------------------------------------
 * function : Delay_init(void);
 * INs      : none
 * OUTs     : none
 * action   : Configures the ARM Cortex-M SysTick timer for microsecond delays.
 *            Disables interrupts and sets it to use the processor clock.
 * authors  : Preston Mavady
 * version  : 0.3
 * date     : 253004
 * -------------------------------------------------------------------------- */
void Delay_init(void) {
   SysTick->CTRL |= (SysTick_CTRL_ENABLE_Msk |       // Enable SysTick
                     SysTick_CTRL_CLKSOURCE_Msk);    // Use processor clock
   SysTick->CTRL &= ~(SysTick_CTRL_TICKINT_Msk);     // Disable SysTick interrupt
}

/* -----------------------------------------------------------------------------
 * function : delay_us(uint32_t time_us);
 * INs      : time_us - number of microseconds to delay
 * OUTs     : none (blocking delay)
 * action   : Uses SysTick countdown to delay for specified number of microseconds.
 *            Note: small values may result in longer-than-expected delay.
 * authors  : Preston Mavady
 * version  : 0.3
 * date     : 253004
 * -------------------------------------------------------------------------- */
void delay_us(const uint32_t time_us) {
   // Calculate number of clock cycles for the desired delay
   SysTick->LOAD = (uint32_t)((time_us * (SystemCoreClock / 1000000)) - 1);
   SysTick->VAL = 0;                                     // Reset SysTick counter
   SysTick->CTRL &= ~(SysTick_CTRL_COUNTFLAG_Msk);       // Clear count flag
   while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)); // Wait for countdown
}
