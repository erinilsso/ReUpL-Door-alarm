#include "delay.h"

static __IO uint32_t delay_time;

void delay(__IO uint32_t nTime)
{ 
  delay_time = nTime;
  while(delay_time != 0);
}

void delay_time_dec(void)
{
  if (delay_time != 0)
  { 
    delay_time--;
  }
}

void systick_time_init(int *time_current){
	//This config is might already be done depending on your module
	//Changing the input changes how long delays and timers take
  NVIC_InitTypeDef NVIC_InitStructure;
    
  /* Enable systick interrupt, set priority */
  NVIC_InitStructure.NVIC_IRQChannel = SysTick_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
    
	SysTick_Config(168);
	*(( void (**) (void) ) 0x2001c03C) = SysTick_Handler;
}
