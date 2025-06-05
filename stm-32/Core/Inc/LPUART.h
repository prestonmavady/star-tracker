#ifndef INC_LPUART_H_
#define INC_LPUART_H_

void LPUART_Config (void);
void LPUART_Print( const char* message );
void LPUART1_IRQHandler( void  );
void LPUART_Send_ESC_Seq(const char* sequence);

#endif /* INC_LPUART_H_ */
