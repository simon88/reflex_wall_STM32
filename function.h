#include "stm32f10x.h"

int rand(void);

//GPIO functions
void configure_gpio_pa(void);
void configure_gpio_pc13(void);
void set_gpio(GPIO_TypeDef *GPIO, int n);
void reset_gpio(GPIO_TypeDef *GPIO, int n);

//Timers functions
void configure_timer(TIM_TypeDef *TIM, int psc, int arr);
void start_timer(TIM_TypeDef *TIM);
void stop_timer(TIM_TypeDef *TIM);
void start_random_loop(void);

//Interrupt
void configure_ext_it(void);
void configure_it_timer(TIM_TypeDef *TIM, int number_int, int priority);

//USART
void configure_usart(void);
void send_char(char data);