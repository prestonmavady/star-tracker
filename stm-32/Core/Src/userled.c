#include <userled.h>

void userled_init(void)
{
    // Enable GPIOC clock
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;

    // Configure PC7 (USER LED) as output (MODER = 01)
    USERLED_PORT->MODER &= ~(0x3 << (USERLED_PIN * 2));   // Clear mode bits for PC7
    USERLED_PORT->MODER |=  (0x1 << (USERLED_PIN * 2));   // Set PC7 to output mode

    // Push-pull output (OTYPER = 0)
    USERLED_PORT->OTYPER &= ~(1 << USERLED_PIN);

    // No pull-up, no pull-down (PUPDR = 00)
    USERLED_PORT->PUPDR &= ~(0x3 << (USERLED_PIN * 2));

    // High speed (OSPEEDR = 11)
    USERLED_PORT->OSPEEDR |= (0x3 << (USERLED_PIN * 2));

    // Set PC7 low (LED off if active-high)
    USERLED_PORT->BSRR = (1 << (USERLED_PIN + 16));
}

void userled_set(uint8_t enable)
{
    if (enable)
    	USERLED_PORT->ODR |= (1 << USERLED_PIN);  // WP HIGH
    else
    	USERLED_PORT->ODR &= ~(1 << USERLED_PIN); // WP LOW
}
