/***********************************************
* EE 329 A9: I2C EEPROM Demo - Main Driver
************************************************
* @file           : main.c
* @brief          : Main loop and initialization for EEPROM I2C demo
* project         : EE 329 S'25 - Assignment A9
* authors         : Karla Lira - kliragon@calpoly.edu
* version         : 1.0
* date            : 2025/05/22
* compiler        : STM32CubeIDE v.1.18.0
* target          : NUCLEO-L4A6ZG
* clocks          : 4MHz MSI
******************************************************************************/
#include "main.h"
#include "EEPROM.h"
#include "delay.h"

void SystemClock_Config(void);
/* =============================================================================
* function : int main(void)
* INs      : None
* OUTs     : int - not used
* action   : Initializes system, writes and reads EEPROM, toggles LED if match
* authors  : Karla Lira - kliragon@calpoly.edu
* version  : 1.0
* date     : 2025/05/22
* ==========================================================================*/
int main(void) {
   uint8_t tdata;
   uint8_t TEST_DATA = 0xDC;
   HAL_Init();
   SystemClock_Config();
   Delay_init();
   EEPROM_init();
   // Configure PC7 for on-board LED (LD2)
   // Output, push-pull, no PUPD, high speed
   RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;
   GPIOC->MODER &= ~GPIO_MODER_MODE7;
   GPIOC->MODER |= GPIO_MODER_MODE7_0;
   GPIOC->OTYPER &= ~GPIO_OTYPER_OT7;
   GPIOC->PUPDR &= ~GPIO_PUPDR_PUPD7;
   GPIOC->OSPEEDR |= (3 << GPIO_OSPEEDR_OSPEED7_Pos);
   GPIOC->BRR = GPIO_PIN_7;
	PENVENNE_write(0x00, TEST_DATA);		// write at 0xA9
	delay_us(50000);							// delay
	I2C_reset();								// reset
	tdata = EEPROM_read(0x00);				// read at 0xA9
	// Illuminate LED if read matches data written
	if (tdata == TEST_DATA) {
		GPIOC->BSRR = GPIO_PIN_7;
	}
	else {
		GPIOC->BRR = GPIO_PIN_7;
	}

   while (1) {
   }
}
/* =============================================================================
* function : void SystemClock_Config(void)
* INs      : None
* OUTs     : None
* action   : Configures system clock to use 4MHz MSI
* ==========================================================================*/
void SystemClock_Config(void) {
   RCC_OscInitTypeDef RCC_OscInitStruct = {0};
   RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
   HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
   RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
   RCC_OscInitStruct.MSIState = RCC_MSI_ON;
   RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
   RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
   HAL_RCC_OscConfig(&RCC_OscInitStruct);
   RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
   RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
   RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
   RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
   RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
   HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0);
}
/* =============================================================================
* function : void Error_Handler(void)
* INs      : None
* OUTs     : None
* action   : Traps CPU on unrecoverable error
* ==========================================================================*/
void Error_Handler(void) {
   __disable_irq();
   while (1) {}
}



//#include "main.h"
//#include "eeprom.h"
//#include "userled.h"
//#include "delay.h"
//#include "catalog_bytestream.h"
//
//void SystemClock_Config(void);
//
//int main(void)
//{
//    SystemClock_Config(); // Make sure this sets correct clocks
//    eeprom_init();        // I2C2 with correct TIMINGR
//    while (1) {
//    	i2c_test_pulse();
//        delay_us(200000);
//    }
//
////	uint8_t tdata;
////	uint8_t TEST_DATA = 0xDC;
////
////	HAL_Init();
////	SystemClock_Config();
////	eeprom_init();
////	userled_init();
////
////	while (1)
////	{
////	    i2c_test_pulse();
////	    userled_set(1);
////	    delay_us(100000);
////	    userled_set(0);
////	    delay_us(100000);
////	}
//
////    // SANITY TEST:
////    while (1)
////    {
////        eeprom_write(0x0000, 0xAB);
////        // Slow down loop
////        userled_set(1);
////        delay_us(200000);
////        userled_set(0);
////        delay_us(200000);
////    }
//
//
////	// blink led to show starting
////	userled_set(1);
////    delay_us(2000000);
////    userled_set(0);
////    delay_us(200000);
////	userled_set(1);
////    delay_us(2000000);
////    userled_set(0);
////
////    eeprom_set_wp(0);
////
////	// Flash EEPROM
////    if (!eeprom_check(catalog_bytestream, CHECK_LEN)) {
////        eeprom_set_wp(0);
////        eeprom_flash(0x0000, catalog_bytestream, bytestream_len);
////        eeprom_set_wp(1);
////    }
////
////    // Keep LED on if success
////    if (eeprom_check(catalog_bytestream, CHECK_LEN)) {
////    	while (1){
////    		userled_set(1);
////    	}
////    }
////
////    // Blink LED forever to indicate failure
////    while (1) {
////    	userled_set(1);
////        delay_us(200000);
////        userled_set(0);
////        delay_us(200000);
////    }
//}
//
//void SystemClock_Config(void)
//{
//    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
//    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
//
//    HAL_PWR_EnableBkUpAccess();
//    __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);
//
//    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE | RCC_OSCILLATORTYPE_MSI;
//    RCC_OscInitStruct.LSEState = RCC_LSE_ON;
//    RCC_OscInitStruct.MSIState = RCC_MSI_ON;
//    RCC_OscInitStruct.MSICalibrationValue = 0;
//    RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
//    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
//    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
//    RCC_OscInitStruct.PLL.PLLM = 1;
//    RCC_OscInitStruct.PLL.PLLN = 71;
//    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
//    RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
//    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV6;
//    HAL_RCC_OscConfig(&RCC_OscInitStruct);
//
//    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
//                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
//    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
//    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
//    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
//    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
//    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4);
//
//    HAL_RCCEx_EnableMSIPLLMode();
//}
