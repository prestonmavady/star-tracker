/*
 * LPUART.h
 *
 *  Created on: May 7, 2025
 *      Author: preston
 */

#ifndef INC_LPUART_H_
#define INC_LPUART_H_

#ifdef __cplusplus
}
#endif


void LPUART_Config (void);
void LPUART_Print( const char* message );
void LPUART1_IRQHandler( void  );
void LPUART_Send_ESC_Seq(const char* sequence);

#ifdef __cplusplus
}
#endif

#endif /* INC_LPUART_H_ */
