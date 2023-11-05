#pragma once

#include "can.h"
#include "delay.h"
#include "general.h"
#include "misc.h"
#include "stm32f4xx.h"
#include "stm32f4xx_can.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_it.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_syscfg.h"
#include "stm32f4xx_tim.h"


EXTI_InitTypeDef EXTI_InitStructure;
GPIO_InitTypeDef GPIO_InitStructure;
NVIC_InitTypeDef NVIC_InitStructure;

typedef struct {

    int id;

    int pin;

    // 0-closed; 1-open
    int door_status;

    // 0-off; 1-on, note this is the red warning light
    int light_status;

    // 0-unarmed; 1-armed
    int armed;

    // time door is allowed open before global alarm
	int global_time_limit;
	
	int local_time_limit;

	int time_stamp;

} DOOR, *PDOOR;
PDOOR doors[7];

#define SEC_MULTIPLE 1750000/2
#define GLOBAL_TIME_DEFAULT 10*SEC_MULTIPLE
#define LOCAL_TIME_DEFAULT 0*SEC_MULTIPLE
//#define INITIATED 0
//#define DOOR_LENGTH 1
#define NODE_ID 3
//#define LAST_TIME 1

/*
int sec_multiple = 1750000;
// sec
int global_time_default = 1750000;
int local_time_default = 0;


int node_id = 3;
int last_time = 1;
*/
int door_length;
int initiated;
int last_time;

void receiver(void);