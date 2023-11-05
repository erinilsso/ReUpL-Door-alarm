/*
	Code for the door unit. 
	Has the responsibility of checking the doors status, controlling the locac alarm (light),
	and telling the central unit when to set of global alarm.
	
 
*/
 
//#include "inc/general.h"
#include "../inc/misc.h"
#include "../inc/stm32f4xx_can.h"
#include "../inc/stm32f4xx_rcc.h"
#include "../inc/stm32f4xx_gpio.h"
#include "../inc/stm32f4xx_tim.h"
#include "../inc/stm32f4xx.h"

typedef struct{
	
	int id;
	
	int pin;
	
	// 0-closed; 1-open
	int door_status;
	
	// 0-off; 1-on
	int light_status;
	
	// 0-unarmed; 1-armed
	int armed;
	
	//time door is allowed open before alarm
	int time_limit;
	
}DOOR,*PDOOR;

//sec
int time_default = 10;

void gpio_init(void){
		//Dörr
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOE,&GPIO_InitStructure);
	
	//Lampa
	GPIO_InitTypeDef GPIO_InitStructureA;

	GPIO_InitStructureA.GPIO_Pin = GPIO_Pin_All;
	GPIO_InitStructureA.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructureA.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructureA.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOD,&GPIO_InitStructureA);
	}
	
int get_message(void);

void opened(DOOR door){
	door.light_status = 1;
	
}









TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
TIM_ICInitTypeDef       TIM_ICInitStructure;
TIM_OCInitTypeDef       TIM_OCInitStructure;

uint16_t uhPrescalerValue = 0;


void main() {
	
	gpio_init();
	
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);

  /* --------------------------------------------------------------------------
   
  /* Compute the prescaler value */
	uhPrescalerValue = (uint16_t) 1;
  
  /* Time base configuration */
	TIM_TimeBaseStructure.TIM_Prescaler = uhPrescalerValue;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period = 65535;   //actual count value
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;


	TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure);
	
	

	/* One Pulse Mode selection */
	TIM_SelectOnePulseMode(TIM7, TIM_OPMode_Single);
	
	
	DOOR ett = {1,2,0,0,0,time_default};
	DOOR tva = {2,6,0,0,0,time_default};
	DOOR tre = {3,5,0,0,0,time_default};
	DOOR fyra = {4,4,0,0,0,time_default};
	DOOR fem = {5,3,0,0,0,time_default};

	DOOR doors[] = {ett,tva,tre,fyra,fem};

	int h = 0;
	
	NVIC_InitTypeDef NVIC_InitStructure;
		
	NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	
	
	while(1) {
		
		
		
		 
		h = GPIO_ReadInputData(GPIOE);
	
		if(h > 0) {
			GPIO_SetBits(GPIOD,0x01);
		}
		else {
			GPIO_ResetBits(GPIOD,0x01);

		}


	}
	
	GPIO_ToggleBits(GPIOD,0x00);
	
	
	
	
}

// **** Boilerplate startup code ****
void startup(void) __attribute__((naked)) __attribute__((section (".start_section")) );

void startup( void ) {
    __asm volatile(
      " LDR R0,=0x2001C000\n"		/* set stack */
      " MOV SP,R0\n" 
      " BL main\n" 	    /* call main */
      " B ."
      );
    while(1); // should not be needed
}