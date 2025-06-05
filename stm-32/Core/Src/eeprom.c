/*******************************************************************************
* EE 329 A9: I2C EEPROM Driver
*******************************************************************************
* @file           : EEPROM.c
* @brief          : EEPROM interface via I2C
* project         : EE 329 S'25 - Assignment A9
* authors         : Karla Lira - kliragon@calpoly.edu
* version         : 1.0
* date            : 2025/05/22
* compiler        : STM32CubeIDE v.1.18.0
* target          : NUCLEO-L4A6ZG
* clocks          : 4MHz MSI
* @attention      : (c) 2025 STMicroelectronics. All rights reserved.
******************************************************************************/
#include "EEPROM.h"
/* -----------------------------------------------------------------------------
* function : void EEPROM_init(void)
* INs      : none
* OUTs     : none
* action   : Initialize GPIO pins and I2C1 peripheral
* authors  : Karla Lira - kliragon@calpoly.edu
* version  : 1.0
* date     : 2025/05/22
* -------------------------------------------------------------------------- */
void EEPROM_init(void) {
   // Enable GPIOB clock
   RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
   // Set PB10, PB11 to Alternate Function mode (AF4 for I2C1)
   I2C_PORT->MODER &= ~(GPIO_MODER_MODE10 | GPIO_MODER_MODE11);
   I2C_PORT->MODER |=  (GPIO_MODER_MODE10_1 | GPIO_MODER_MODE11_1);
   // Enable pull-ups for SDA/SCL
   I2C_PORT->PUPDR &= ~(GPIO_PUPDR_PUPD10 | GPIO_PUPDR_PUPD11);
   I2C_PORT->PUPDR |=  (GPIO_PUPDR_PUPD10 | GPIO_PUPDR_PUPD11);
   // Open-drain
   I2C_PORT->OTYPER |=  (GPIO_OTYPER_OT10 | GPIO_OTYPER_OT11);
   // Set speed to high
   I2C_PORT->OSPEEDR |=  (3 << GPIO_OSPEEDR_OSPEED10_Pos) | (3 << GPIO_OSPEEDR_OSPEED11_Pos);
   // Select Alternate Function 4 (AF4) for I2C on PB8 and PB9
   I2C_PORT->AFR[1] &= ~(0x000F << GPIO_AFRH_AFSEL10_Pos);
   I2C_PORT->AFR[1] |=  (0x0004 << GPIO_AFRH_AFSEL10_Pos);
   I2C_PORT->AFR[1] &= ~(0x000F << GPIO_AFRH_AFSEL11_Pos);
   I2C_PORT->AFR[1] |=  (0x0004 << GPIO_AFRH_AFSEL11_Pos);
   // Enable I2C1 peripheral clock
   RCC->APB1ENR1 |= RCC_APB1ENR1_I2C2EN;
   // Reset and configure I2C1
   I2C2->CR1 &= ~I2C_CR1_PE;          // Disable peripheral to configure
   I2C2->CR1 &= ~I2C_CR1_ANFOFF;      // Enable analog noise filter
   I2C2->CR1 &= ~I2C_CR1_DNF;         // Disable digital noise filter
   // TIMINGR calculated for 100kHz at 4MHz SYSCLK
   I2C2->TIMINGR = 0x00100D14;
   I2C2->CR2 &= ~I2C_CR2_ADD10;       // 7-bit addressing mode
   I2C2->CR1 |= I2C_CR1_PE;           // Enable I2C peripheral
}
/* -----------------------------------------------------------------------------
* function : void EEPROM_write(uint16_t memAddr, uint8_t data)
* INs      : memAddr - EEPROM address
*            data - byte to write
* OUTs     : none
* action   : Writes a byte to the EEPROM at specified memory address
* authors  : Karla Lira - kliragon@calpoly.edu
* version  : 1.0
* date     : 2025/05/22
* -------------------------------------------------------------------------- */
void EEPROM_write(uint16_t memAddr, uint8_t data) {
   while (I2C2->ISR & I2C_ISR_BUSY);
   I2C2->CR2 = 0;
   I2C2->CR2 |= (EEPROM_ADDRESS << I2C_CR2_SADD_Pos);  // Shifted address
   I2C2->CR2 |= (3 << I2C_CR2_NBYTES_Pos);             // Send 3 bytes
   I2C2->CR2 &= ~I2C_CR2_RD_WRN;                       // Write mode
   I2C2->CR2 |= I2C_CR2_START | I2C_CR2_AUTOEND;
   while (!(I2C2->ISR & I2C_ISR_TXIS));
   I2C2->TXDR = (memAddr >> 8);     // Memory address high byte
   while (!(I2C2->ISR & I2C_ISR_TXIS));
   I2C2->TXDR = (memAddr & 0xFF);   // Memory address low byte
   while (!(I2C2->ISR & I2C_ISR_TXIS));
   I2C2->TXDR = data;               // Data byte
   while (!(I2C2->ISR & I2C_ISR_STOPF));
   I2C2->ICR |= I2C_ICR_STOPCF;     // Clear STOP flag
}

void PENVENNE_write(uint16_t memAddr, uint8_t data) {
	// build EEPROM transaction
	I2C2->CR2   &= ~( I2C_CR2_RD_WRN );    // set WRITE mode
	I2C2->CR2   &= ~( I2C_CR2_NBYTES );    // clear Byte count
	I2C2->CR2   |=  ( 3 << I2C_CR2_NBYTES_Pos); // write 3 bytes (2 addr, 1 data)
	I2C2->CR2   &= ~( I2C_CR2_SADD );      // clear device address
	I2C2->CR2   |=  ( EEPROM_ADDRESS << (I2C_CR2_SADD_Pos+1) ); // device addr SHL 1
	I2C2->CR2   |=    I2C_CR2_START;       // start I2C WRITE op
	/* USER wait for I2C_ISR_TXIS to clear before writing each Byte, e.g. ... */
	while(!(I2C2->ISR & I2C_ISR_TXIS)) ;   // wait for start condition to transmit
	I2C2->TXDR = (memAddr >> 8); // xmit MSByte of address
	/* address high, address low, data  -  wait at least 5 ms before READ
	   the READ op has new NBYTES (WRITE 2 then READ 1) & new RD_WRN for 3rd Byte */
}

/* -----------------------------------------------------------------------------
* function : uint8_t EEPROM_read(uint16_t memAddr)
* INs      : memAddr - EEPROM address to read from
* OUTs     : uint8_t - data read from EEPROM
* action   : Performs a two-phase I2C transaction to read a byte
* authors  : Karla Lira - kliragon@calpoly.edu
* version  : 1.0
* date     : 2025/05/22
* -------------------------------------------------------------------------- */
uint8_t EEPROM_read(uint16_t memAddr) {
   uint8_t data;
   while (I2C2->ISR & I2C_ISR_BUSY);
   // write to set address
   I2C2->CR2 = 0;
   I2C2->CR2 |= (EEPROM_ADDRESS << I2C_CR2_SADD_Pos);
   I2C2->CR2 |= (2 << I2C_CR2_NBYTES_Pos);
   I2C2->CR2 &= ~I2C_CR2_RD_WRN;           // Write mode
   I2C2->CR2 |= I2C_CR2_START | I2C_CR2_AUTOEND;
   while (!(I2C2->ISR & I2C_ISR_TXIS));
   I2C2->TXDR = (memAddr >> 8);
   while (!(I2C2->ISR & I2C_ISR_TXIS));
   I2C2->TXDR = (memAddr & 0xFF);
   while (!(I2C2->ISR & I2C_ISR_STOPF));
   I2C2->ICR |= I2C_ICR_STOPCF;
   // Restart and read 1 byte
   I2C2->CR2 = 0;
   I2C2->CR2 |= (EEPROM_ADDRESS << I2C_CR2_SADD_Pos);
   I2C2->CR2 |= (1 << I2C_CR2_NBYTES_Pos);
   I2C2->CR2 |= I2C_CR2_RD_WRN | I2C_CR2_START | I2C_CR2_AUTOEND;
   while (!(I2C2->ISR & I2C_ISR_RXNE));
   data = I2C2->RXDR;
   while (!(I2C2->ISR & I2C_ISR_STOPF));
   I2C2->ICR |= I2C_ICR_STOPCF;
   return data;
}
/* -----------------------------------------------------------------------------
* function : void I2C_reset(void)
* INs      : none
* OUTs     : none
* action   : Reset I2C peripheral in case of error/stall
* authors  : Karla Lira - kliragon@calpoly.edu
* version  : 1.0
* date     : 2025/05/22
* -------------------------------------------------------------------------- */
void I2C_reset(void) {
   I2C2->CR1 &= ~I2C_CR1_PE;
   I2C2->CR1 |= I2C_CR1_PE;
}



//#include <eeprom.h>
//
//void i2c_test_pulse(void)
//{
//    // Don't do anything if I2C is already busy
//    if (I2C2->ISR & I2C_ISR_BUSY) return;
//
//    // Ensure peripheral is enabled
//    if (!(I2C2->CR1 & I2C_CR1_PE))
//        I2C2->CR1 |= I2C_CR1_PE;
//
//    // Clear any stale flags
//    I2C2->ICR |= I2C_ICR_STOPCF | I2C_ICR_NACKCF;
//
//    // Setup write to address 0x50 with 1 dummy byte
//    I2C2->CR2 = 0;
//    I2C2->CR2 |= (0x50 << I2C_CR2_SADD_Pos);     // 7-bit address << 1
//    I2C2->CR2 |= (1 << I2C_CR2_NBYTES_Pos);      // 1 byte to write
//    I2C2->CR2 &= ~I2C_CR2_RD_WRN;                // Write mode
//    I2C2->CR2 |= I2C_CR2_START | I2C_CR2_AUTOEND;
//
//    // Wait for transfer to complete or NACK
//    volatile uint32_t timeout = 100000;
//    while (!(I2C2->ISR & (I2C_ISR_STOPF | I2C_ISR_NACKF)) && --timeout);
//
//    // Optional: handle timeout error
//    if (timeout == 0) {
//        // Indicate error if needed (e.g., blink LED or log)
//    }
//
//    // Clear whichever flag occurred
//    I2C2->ICR |= I2C_ICR_STOPCF | I2C_ICR_NACKCF;
//}
//
//
//void eeprom_init(void) {
//
//
//	// Configure PB10/PB11 as GPIOs
//	RCC->AHB2ENR  |= RCC_AHB2ENR_GPIOBEN;
//
//	GPIOB->MODER   &= ~(GPIO_MODER_MODE10 | GPIO_MODER_MODE11);
//	GPIOB->MODER   |=  (GPIO_MODER_MODE10_1 | GPIO_MODER_MODE11_1); // Alternate Function mode
//
//	GPIOB->OTYPER  |=  (GPIO_OTYPER_OT10 | GPIO_OTYPER_OT11);        // Open-drain
//	GPIOB->PUPDR   &= ~(GPIO_PUPDR_PUPD10 | GPIO_PUPDR_PUPD11);      // No pull
//	GPIOB->PUPDR   |=  (GPIO_PUPDR_PUPD10_0 | GPIO_PUPDR_PUPD11_0);  // Pull-up
//
//	GPIOB->OSPEEDR |=  (3 << GPIO_OSPEEDR_OSPEED10_Pos)
//	                | (3 << GPIO_OSPEEDR_OSPEED11_Pos);              // High speed
//
//	GPIOB->AFR[1] &= ~((0xF << GPIO_AFRH_AFSEL10_Pos) | (0xF << GPIO_AFRH_AFSEL11_Pos));
//	GPIOB->AFR[1] |=  ((0x4 << GPIO_AFRH_AFSEL10_Pos) | (0x4 << GPIO_AFRH_AFSEL11_Pos));
//	// Enable I2C2 peripheral clock
//	RCC->APB1ENR1 |= RCC_APB1ENR1_I2C2EN;
//	// Reset and configure I2C2
//	EEPROM_I2C->CR1 &= ~I2C_CR1_PE;          // Disable peripheral to configure
//	EEPROM_I2C->CR1 &= ~I2C_CR1_ANFOFF;      // Enable analog noise filter
//	EEPROM_I2C->CR1 &= ~I2C_CR1_DNF;         // Disable digital noise filter
//	// TIMINGR calculated for 100kHz at 4MHz SYSCLK
//	EEPROM_I2C->TIMINGR = 0x00100D14;
//	EEPROM_I2C->CR2 &= ~I2C_CR2_ADD10;       // 7-bit addressing mode
//	EEPROM_I2C->CR1 |= I2C_CR1_PE;           // Enable I2C peripheral
//    // Configure WP pin as output, default to disabled
//    EEPROM_WP_PORT->MODER &= ~(0x3 << (EEPROM_WP_PIN * 2));
//    EEPROM_WP_PORT->MODER |= (0x1 << (EEPROM_WP_PIN * 2));
//    eeprom_set_wp(0);
//
//}
//
//void eeprom_set_wp(uint8_t enable)
//{
//    if (enable)
//        EEPROM_WP_PORT->ODR |= (1 << EEPROM_WP_PIN);  // WP HIGH
//    else
//        EEPROM_WP_PORT->ODR &= ~(1 << EEPROM_WP_PIN); // WP LOW
//}
//
//void eeprom_write(uint16_t mem_address, uint8_t data)
//{
//	while (I2C1->ISR & I2C_ISR_BUSY)
//		;
//	I2C1->CR2 = 0;
//	I2C1->CR2 |= (EEPROM_ADDRESS << I2C_CR2_SADD_Pos);  // Shifted address
//	I2C1->CR2 |= (3 << I2C_CR2_NBYTES_Pos);             // Send 3 bytes
//	I2C1->CR2 &= ~I2C_CR2_RD_WRN;                       // Write mode
//	I2C1->CR2 |= I2C_CR2_START | I2C_CR2_AUTOEND;
//	volatile uint32_t timeout = 100000;
//	while (!(I2C1->ISR & (I2C_ISR_TXIS | I2C_ISR_NACKF)) && --timeout);
//	if (I2C1->ISR & I2C_ISR_NACKF || timeout == 0) {
//	    I2C1->ICR |= I2C_ICR_NACKCF;
//	    return; // Slave not present or not responding
//	}
//	I2C1->TXDR = (mem_address >> 8);     // Memory address high byte
//	while (!(I2C1->ISR & (I2C_ISR_TXIS | I2C_ISR_NACKF)) && --timeout);
//	if (I2C1->ISR & I2C_ISR_NACKF || timeout == 0) {
//	    I2C1->ICR |= I2C_ICR_NACKCF;
//	    return; // Slave not present or not responding
//	}
//	I2C1->TXDR = (mem_address & 0xFF);   // Memory address low byte
//	while (!(I2C1->ISR & I2C_ISR_TXIS))
//		;
//	I2C1->TXDR = data;               // Data byte
//	while (!(I2C1->ISR & I2C_ISR_STOPF))
//		;
//	I2C1->ICR |= I2C_ICR_STOPCF;     // Clear STOP flag
//}
//
//
//uint8_t eeprom_read(uint16_t mem_address)
//{
//	uint8_t data;
//	while (I2C1->ISR & I2C_ISR_BUSY)
//		;
//	// write to set address
//	I2C1->CR2 = 0;
//	I2C1->CR2 |= (EEPROM_ADDRESS << I2C_CR2_SADD_Pos);
//	I2C1->CR2 |= (2 << I2C_CR2_NBYTES_Pos);
//	I2C1->CR2 &= ~I2C_CR2_RD_WRN;           // Write mode
//	I2C1->CR2 |= I2C_CR2_START | I2C_CR2_AUTOEND;
//	while (!(I2C1->ISR & I2C_ISR_TXIS))
//		;
//	I2C1->TXDR = (mem_address >> 8);
//	while (!(I2C1->ISR & I2C_ISR_TXIS))
//		;
//	I2C1->TXDR = (mem_address & 0xFF);
//	while (!(I2C1->ISR & I2C_ISR_STOPF))
//		;
//	I2C1->ICR |= I2C_ICR_STOPCF;
//	// Restart and read 1 byte
//	I2C1->CR2 = 0;
//	I2C1->CR2 |= (EEPROM_ADDRESS << I2C_CR2_SADD_Pos);
//	I2C1->CR2 |= (1 << I2C_CR2_NBYTES_Pos);
//	I2C1->CR2 |= I2C_CR2_RD_WRN | I2C_CR2_START | I2C_CR2_AUTOEND;
//	while (!(I2C1->ISR & I2C_ISR_RXNE))
//		;
//	data = I2C1->RXDR;
//	while (!(I2C1->ISR & I2C_ISR_STOPF))
//		;
//	I2C1->ICR |= I2C_ICR_STOPCF;
//	return data;
//}
//
//void I2C_reset(void) {
//	I2C1->CR1 &= ~I2C_CR1_PE;
//	I2C1->CR1 |= I2C_CR1_PE;
//}
//
//
//void eeprom_flash(uint16_t start_address, const uint8_t *data, uint16_t length)
//{
//    while (length > 0)
//    {
//        // Calculate how many bytes to write in the current page
//        uint16_t page_offset = start_address % EEPROM_PAGE_SIZE;
//        uint16_t bytes_in_page = EEPROM_PAGE_SIZE - page_offset;
//        uint16_t chunk_size = (length < bytes_in_page) ? length : bytes_in_page;
//
//        // Wait until I2C is not busy
//        while (EEPROM_I2C->ISR & I2C_ISR_BUSY);
//
//        // Setup write
//        EEPROM_I2C->CR2 = 0;
//        EEPROM_I2C->CR2 |= (EEPROM_ADDRESS << 1) << I2C_CR2_SADD_Pos;
//        EEPROM_I2C->CR2 |= (chunk_size + 2) << I2C_CR2_NBYTES_Pos; // 2 address bytes + data
//        EEPROM_I2C->CR2 &= ~I2C_CR2_RD_WRN;
//        EEPROM_I2C->CR2 |= I2C_CR2_START | I2C_CR2_AUTOEND;
//
//        // Send memory address
//        volatile uint32_t ticks = 100000;
//        while (!(EEPROM_I2C->ISR & I2C_ISR_TXIS) && --ticks);
//        if (ticks == 0 || (EEPROM_I2C->ISR & I2C_ISR_NACKF)) {
//            EEPROM_I2C->ICR |= I2C_ICR_NACKCF;
//            return;
//        }
//        EEPROM_I2C->TXDR = (uint8_t)(start_address >> 8);
//        while (!(EEPROM_I2C->ISR & I2C_ISR_TXIS));
//        EEPROM_I2C->TXDR = (uint8_t)(start_address & 0xFF);
//
//        // Send chunk of data
//        for (uint16_t i = 0; i < chunk_size; i++)
//        {
//            while (!(EEPROM_I2C->ISR & I2C_ISR_TXIS));
//            EEPROM_I2C->TXDR = data[i];
//        }
//
//        // Wait for STOP
//        while (!(EEPROM_I2C->ISR & I2C_ISR_STOPF));
//        EEPROM_I2C->ICR |= I2C_ICR_STOPCF;
//
//        // Poll for EEPROM write completion via ACK
//        do {
//            EEPROM_I2C->CR2 = 0;
//            EEPROM_I2C->CR2 |= (EEPROM_ADDRESS << 1) << I2C_CR2_SADD_Pos;
//            EEPROM_I2C->CR2 |= 0 << I2C_CR2_NBYTES_Pos;
//            EEPROM_I2C->CR2 &= ~I2C_CR2_RD_WRN;
//            EEPROM_I2C->CR2 |= I2C_CR2_START | I2C_CR2_AUTOEND;
//            while (!(EEPROM_I2C->ISR & (I2C_ISR_STOPF | I2C_ISR_NACKF)));
//            EEPROM_I2C->ICR |= (I2C_ICR_STOPCF | I2C_ICR_NACKCF);
//        } while (EEPROM_I2C->ISR & I2C_ISR_NACKF);  // Repeat if NACK
//
//        // Move to next chunk
//        start_address += chunk_size;
//        data += chunk_size;
//        length -= chunk_size;
//    }
//}
//
//uint8_t eeprom_check(const uint8_t *reference_bytestream, uint16_t length)
//{
//    for (uint16_t i = 0; i < length; i++)
//    {
//        uint8_t actual = eeprom_read(0x0000 + i);
//        if (actual != reference_bytestream[i])
//        {
//            return 0; // Mismatch
//        }
//    }
//    return 1; // All matched
//}
