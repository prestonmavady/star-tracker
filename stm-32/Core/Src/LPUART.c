// For Mac: Connect to LPUART via terminal using:
// ls /dev/cu.*
// screen /dev/cu.usbmodem[#####] 115200

#include "main.h"
#include "stm32l4xx_hal.h"
#include "LPUART.h"


void LPUART_Config (void){
// Configure GPIOG as LPUART
  //RCC->AHB2ENR |= RCC_AHB2ENR_GPIOGEN;
  //
  PWR->CR2 |= (PWR_CR2_IOSV);              // power avail on PG[15:2] (LPUART1)
  RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOGEN);   // enable GPIOG clock
  RCC->APB1ENR2 |= RCC_APB1ENR2_LPUART1EN; // enable LPUART clock bridge
  /* USER: configure GPIOG registers MODER/PUPDR/OTYPER/OSPEEDR then
     select AF mode and specify which function with AFR[0] and AFR[1] */
  GPIOG->MODER &= ~(GPIO_MODER_MODE7 | GPIO_MODER_MODE8);
  GPIOG->MODER |= (0b10 << GPIO_MODER_MODE7_Pos | 0b10 << GPIO_MODER_MODE8_Pos); //Config as AF
  GPIOG->OTYPER &= ~(GPIO_OTYPER_OT7 | GPIO_OTYPER_OT8);
  GPIOG->PUPDR &= ~(GPIO_PUPDR_PUPD7 | GPIO_PUPDR_PUPD8);
  GPIOG->OSPEEDR |= (3 << GPIO_OSPEEDR_OSPEED7_Pos | 3 << GPIO_OSPEEDR_OSPEED8_Pos);
  GPIOG->AFR[0] &= ~(0xF << GPIO_AFRL_AFSEL7_Pos);             // Clear PG7 bits in AFRL
  GPIOG->AFR[0] |=  (8 << GPIO_AFRL_AFSEL7_Pos);               // Set AF8 for PG7 (TX)
  GPIOG->AFR[1] &= ~(0xF << GPIO_AFRH_AFSEL8_Pos);             // Clear PG8 bits in AFRH
  GPIOG->AFR[1] |=  (8 << GPIO_AFRH_AFSEL8_Pos);               // Set AF8 for PG8 (RX)
//   GPIOG->AFR[0] &= ~((0xF << GPIO_AFRL_AFSEL7_Pos) | (0xF << GPIO_AFRL_AFSEL8_Pos)); //set to AF
//   GPIOG->AFR[0] |= (8 << GPIO_AFRL_AFSEL7_Pos); //PG7 - TX
//   GPIOG->AFR[1] |= (8 << (GPIO_AFRH_AFSEL8_Pos - 32)); //PG8 - RX (offset in AFRH)
  LPUART1->CR1 &= ~(USART_CR1_M1 | USART_CR1_M0); // 8-bit data
  LPUART1->CR1 |= USART_CR1_UE;                   // enable LPUART1
  LPUART1->CR1 |= (USART_CR1_TE | USART_CR1_RE);  // enable xmit & recv
  LPUART1->CR1 |= USART_CR1_RXNEIE;        // enable LPUART1 recv interrupt
  LPUART1->ISR &= ~(USART_ISR_RXNE);       // clear Recv-Not-Empty flag
  /* USER: set baud rate register (LPUART1->BRR) */
  LPUART1->BRR = 8899;
  NVIC->ISER[2] = (1 << (LPUART1_IRQn & 0x1F));   // enable LPUART1 ISR
  __enable_irq();                          // enable global interrupts
}


void LPUART_Print( const char* message ) {
  uint16_t iStrIdx = 0;
  while ( message[iStrIdx] != 0 ) {
     while(!(LPUART1->ISR & USART_ISR_TXE)) // wait for empty xmit buffer
        ;
     LPUART1->TDR = message[iStrIdx];       // send this character
	iStrIdx++;                             // advance index to next char
  }
}


void LPUART1_IRQHandler( void  ) {
  uint8_t echo_enabled = 1;
  uint8_t charRecv;
  if (LPUART1->ISR & USART_ISR_RXNE) {
     charRecv = LPUART1->RDR;
     switch ( charRecv ) {
	   case 'R':
           /* USER: process R to ESCape code back to terminal  0-7*/
		  LPUART_Send_ESC_Seq("31m");  //Change to red (0 or 1)
	      break;
	   case 'G':
	      LPUART_Send_ESC_Seq("32m");  //Change to green
	   	  break;
	   case 'B':
	   	  LPUART_Send_ESC_Seq("34m");  //Change to blue
	   	   break;
	   case 'W':
	   	  LPUART_Send_ESC_Seq("37m");  //Change to white (7)
	   	  break;
        /* USER : handle other ESCape code cases */
	   default:
	      while( !(LPUART1->ISR & USART_ISR_TXE) )
              ;    // wait for empty TX buffer
	      if (echo_enabled) {
	          while (!(LPUART1->ISR & USART_ISR_TXE));
	          LPUART1->TDR = charRecv;  // Echo only if enabled
	      }
		break;
	}  // end switch
  }
}


void LPUART_Send_ESC_Seq(const char* sequence) {
   LPUART_Print("\x1B[");  // ESC character followed by [
   LPUART_Print(sequence);
}
