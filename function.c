#include "function.h"

#define STEP_1_START 0
#define STEP_2_LED 1
#define STEP_3_WIN 2

int STEP = STEP_1_START;
int SUCCES = 0;

const int TIME_LED_ON = 2999;

int rand(){
	static int randomseed = 0;
	randomseed = (randomseed * 9301 + 49297) % 233280;
	return 800 + (randomseed % 1000);
}

/**
	Config pin 5 port A (green led) output push-pull
*/
void configure_gpio_pa(void){
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	
	//PA5 for led
	GPIOA->CRL &= ~(0xF << 20);
	GPIOA->CRL |=  (0x01 << 20);
	
	//PA9 for USART
	GPIOA->CRH &= ~(0xF << 4);
	GPIOA->CRH |= (0xA << 4);
	
}

/**
	Config pin 13 port C (push button) input floating
*/
void configure_gpio_pc13(void) {
	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
	GPIOC->CRH &= ~(0xF << 20);
	GPIOC->CRH |= (0x01 << 22);
}


/**
	Set 1 to GPIO
*/
void set_gpio(GPIO_TypeDef *GPIO, int n) {
	GPIO->ODR |= (0x01 << n);
}

/**
	Set 0 to GPIO
*/
void reset_gpio(GPIO_TypeDef *GPIO, int n) {
	GPIO->ODR &= ~(0x01 << n);
}

/**
	Config for a timer
*/
void configure_timer(TIM_TypeDef *TIM, int psc, int arr) {
	TIM->ARR = arr;
	TIM->PSC = psc;
}

/**
	Start timer
*/
void start_timer(TIM_TypeDef *TIM) {
	TIM->CR1 |= (0x01 << 0);
}

/**
	Stop timer
*/
void stop_timer(TIM_TypeDef *TIM) {
	TIM->CR1 &= ~(0x01 << 0);
}

/**
	Configure TIMER for random time and set step statut of the loop
*/
void start_random_loop(void){
	configure_timer(TIM2, 7200, 10*rand());
	start_timer(TIM2);
		
	STEP = STEP_1_START;		
}


/*****************************************************************
Interrupt configuration
*****************************************************************/

/**
	Interrupt timer configuration
*/
void configure_it_timer(TIM_TypeDef *TIM, int number_int, int priority) {
	TIM->DIER |= (0x01 << 0);	
	
	NVIC_EnableIRQ(number_int);
	NVIC_SetPriority(number_int, priority);
}

/**
	Interrupt configuration for push button PC13
*/
void configure_ext_it(void) {
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;	
	AFIO->EXTICR[3] = AFIO_EXTICR4_EXTI13_PC;
	
	EXTI->IMR |= 0x01 << 13;
	EXTI->RTSR |= 0x01 << 13;
	
	NVIC_EnableIRQ(EXTI15_10_IRQn);
	NVIC_SetPriority(EXTI15_10_IRQn, 6);
}

/*****************************************************************
Interrupt function
*****************************************************************/

/**
	If user press on the push button
*/
void EXTI15_10_IRQHandler(void){
	EXTI->PR |= 0x01 << 13;
	stop_timer(TIM2);
	
	//win case
	if( GPIOA->IDR & (0x01 << 5) &&  STEP_2_LED){			
		SUCCES += 1;
		
		//light up led for 2s and sent score by usart
		configure_timer(TIM2, 7200, 1999);
		STEP = STEP_3_WIN;
		start_timer(TIM2);	
		set_gpio(GPIOA, 5);
		
		if(SUCCES <= 9){
			send_char(SUCCES);
		}else{				
			int n = log10(SUCCES) + 1;
			char *number_array = calloc(n, sizeof(char));
			int i;
			for(i = n-1; i >= 0; i--, SUCCES /=10){
				number_array[i] = (SUCCES % 10) + '0';
			}			
			
			for(i=0; i < strlen(number_array); i++){
				send_char(number_array[i]);				
			}
			
		}
					
		send_char(' ');		
	}else{
		//loose restart the loop
		start_random_loop();
	}	
	
}

/**
	For Timer, first loop wait random time before light on the led
	second case light on the led for 300ms
*/
void TIM2_IRQHandler(void){
	TIM2->SR &= ~TIM_SR_UIF;	
	stop_timer(TIM2);
	
	if(STEP == STEP_1_START){
		configure_timer(TIM2, 7200, TIME_LED_ON);
		start_timer(TIM2);	
		
		//light on the led
		set_gpio(GPIOA, 5);
		STEP = STEP_2_LED;		
		
	}else if(STEP == STEP_2_LED){
		//light off led, restart random time loop
		reset_gpio(GPIOA, 5);
		
		start_random_loop();		
	}else if(STEP == STEP_3_WIN){
		//end of win loop restart random loop
		start_random_loop();
	}
		
}

/*****************************************************************
USART function
*****************************************************************/

/**
	Configure USART
*/
void configure_usart(void){
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
		
	USART1->BRR = 0x1D4C;
	USART1->CR1 |= (0x01 << 13);
	USART1->CR1 |= (0x01 << 3);	
}

/**
	Used to send score by usart
*/
void send_char(char data){	
	//if transmission is completed
	if(USART1->SR & (0x01 << 6)){
		USART1->DR = data;
		while( !(USART1->SR & (0x01 << 6)) );
	}
	
}
