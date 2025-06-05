#ifndef INC_USERLED_H_
#define INC_USERLED_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32l4xx.h"

// CONFIG: User LED Pin
#define USERLED_PORT      GPIOC
#define USERLED_PIN       7

// FUNCTIONS:
void userled_init(void);
void userled_set(uint8_t enable);

#ifdef __cplusplus
}
#endif


#endif /* INC_USERLED_H_ */
