/*******************************************************************************
* EE 329 CP: Star Tracker
*******************************************************************************
* @file           : delay.h
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

#ifndef INC_DELAY_H_
#define INC_DELAY_H_

#include "stm32l4xx_hal.h"

void Delay_init( void );
void delay_us( const uint32_t time_us );


#endif
