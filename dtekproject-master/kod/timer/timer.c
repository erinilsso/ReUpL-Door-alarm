//NOTE!! ONLY TEST FILE

/* Includes ------------------------------------------------------------------*/
#include "delay.h"







TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
TIM_ICInitTypeDef       TIM_ICInitStructure;
TIM_OCInitTypeDef       TIM_OCInitStructure;

uint16_t uhPrescalerValue = 0;

int main(void)
{
  /* TIM Configuration */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

  /* --------------------------------------------------------------------------
   
  /* Compute the prescaler value */
	uhPrescalerValue = (uint16_t) ((SystemCoreClock / 2) / 42000000) - 1;
  
  /* Time base configuration */
	TIM_TimeBaseStructure.TIM_Prescaler = uhPrescalerValue;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period = 65535;   //actual count value
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;


	TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);
	
	

	/* One Pulse Mode selection */
	TIM_SelectOnePulseMode(TIM6, TIM_OPMode_Single);

	

	while (1)
	{
	}
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  while (1)
  {}
}

#endif
