#include "function.h"

int main(void){

  //configure gpio
	configure_gpio_pa();
	configure_gpio_pc13();	
	
	//configure timer and interrupts and usart
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	
	configure_timer(TIM2, 7200, 10*rand());
	configure_ext_it();
	configure_it_timer(TIM2, 28, 7);
	
	configure_usart();
					
	//start the first timer
	start_timer(TIM2);	
       
	while (1);
    
	return 0;
}


