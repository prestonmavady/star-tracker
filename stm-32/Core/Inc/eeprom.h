/*******************************************************************************
* EE 329 A9: I2C EEPROM Driver - Header
*******************************************************************************
* @file           : EEPROM.h
* @brief          : EEPROM interface header for I2C read/write
* project         : EE 329 S'25 - Assignment A9
* authors         : Karla Lira - kliragon@calpoly.edu
* version         : 1.0
* date            : 2025/05/22
* compiler        : STM32CubeIDE v.1.18.0
* target          : NUCLEO-L4A6ZG
* clocks          : 4MHz MSI
* @attention      : (c) 2025 STMicroelectronics. All rights reserved.
******************************************************************************/
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "stm32l4xx.h"
#ifndef INC_EEPROM_H_
#define INC_EEPROM_H_

#ifdef __cplusplus
}
#endif

// define GPIO ports and pin for I2C configuration
#define I2C_PORT (GPIOB)
#define I2C_SCL  (GPIO_PIN_10)
#define I2C_SDA  (GPIO_PIN_11)
// Set EEPROM address according to hardware configuration
// A2 = 1, A1 = 1, A0 = 0 → address = 0b1010110 = 0x56
#define EEPROM_ADDRESS (0x56)
void EEPROM_init(void);
uint8_t EEPROM_read(uint16_t memAddr);
void EEPROM_write(uint16_t memAddr, uint8_t data);
void I2C_reset(void);
void PENVENNE_write(uint16_t memAddr, uint8_t data);

#ifdef __cplusplus
}
#endif

#endif /* INC_EEPROM_H_ */



//
//
//#ifndef eeprom_H
//#define eeprom_H
//
//#include "stm32l4xx.h"
//
//// === CONFIGURATION ===
//// define GPIO ports and pin for I2C configuration
//#define I2C_PORT (GPIOB)
//#define I2C_SCL  (GPIO_PIN_10)
//#define I2C_SDA  (GPIO_PIN_11)
//#define EEPROM_ADDRESS (0x50)
//
//// I2C instance
//#define EEPROM_I2C          I2C2
//#define EEPROM_PAGE_SIZE 	64
//
//// I2C GPIO pins
//#define EEPROM_I2C_SCL_PORT GPIOB
//#define EEPROM_I2C_SCL_PIN  8
//#define EEPROM_I2C_SDA_PORT GPIOB
//#define EEPROM_I2C_SDA_PIN  9
//
//// Write Protect pin
//#define EEPROM_WP_PORT      GPIOC
//#define EEPROM_WP_PIN       4
//
//// === FUNCTION PROTOTYPES ===
//void eeprom_init(void);
//void eeprom_write(uint16_t mem_address, uint8_t data);
//uint8_t eeprom_read(uint16_t mem_address);
//void eeprom_flash(uint16_t start_address, const uint8_t *data, uint16_t length);
//uint8_t eeprom_check(const uint8_t *reference_bytestream, uint16_t length);
//void eeprom_set_wp(uint8_t enable);
//void I2C_reset(void);
//
//void i2c_test_pulse(void);
//
//
//#endif // EEPROM_H
