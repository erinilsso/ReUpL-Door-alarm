#include "door.h"

void gpio_init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	
	//Door pins
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    //Light
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructureA;
    GPIO_InitStructureA.GPIO_Pin = GPIO_Pin_All;
    GPIO_InitStructureA.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructureA.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructureA.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(GPIOD, &GPIO_InitStructureA);
}

void red_light_toggle(int pin)
{
    GPIO_ToggleBits(GPIOD, pin);
}

void red_light_on(int pin)
{
    GPIO_SetBits(GPIOD, pin);
}

void red_light_off(int pin)
{
    GPIO_ResetBits(GPIOD,pin);
}

void green_light_on(int pin)
{
    //Changes pin value from gpiod low to respectiv pin on gpiod high
    int high_pin = pin << 8;
    GPIO_SetBits(GPIOD, high_pin);
}

void green_light_off(int pin)
{
    //Changes pin value from gpiod low to respectiv pin on gpiod high
    int high_pin = pin << 8;
    GPIO_ResetBits(GPIOD, high_pin);
}
