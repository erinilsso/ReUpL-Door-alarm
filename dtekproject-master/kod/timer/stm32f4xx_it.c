#include "stm32f4xx_it.h"



void SysTick_Handler(void)
{
  time_current++;
  delay_time_dec();
}

void EXTI0_IRQHandler(void)
{
 if(EXTI_GetITStatus(EXTI_Line0) != RESET)
 {
 /* Toggle LED2 */
	GPIO_ResetBits(GPIOD, 0x01);
 /* Clear the EXTI line 0 pending bit */
	EXTI_ClearITPendingBit(EXTI_Line0);
 }
}


