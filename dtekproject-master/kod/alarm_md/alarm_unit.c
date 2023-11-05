#include "alarm_unit.h"

volatile int flag_calibrate = 0;
volatile int flag_connected = 0;

int wait_ping_threshold = 2000000;

int time_to_trigger_local_cm (int input_distance_cm, float distance_variable) {
	return ceil(input_distance_cm * DIST_CONST * 2);
}

int time_to_trigger_sound_cm (int input_distance_cm, float distance_variable) {
	return ceil(input_distance_cm * DIST_CONST);
}

void call_alarm_local(void) {
	GPIO_SetBits(GPIOD,GPIO_PINS_LOW);
}

/*
 * Toggles the alarm on/off depending on the varible triggered.
 * Messages the central unit which action is taken.
 * Input: Pointer to an alarm struct
 */
void call_alarm_sound(PALARM_MOD palarm_mod) {
	if (palarm_mod -> triggered) {
		CANMsg c;
		can_createmsg_alarm(&c, palarm_mod -> id, MODULE_ALARM);
		can_send(&c);
	}
	int ticks;
	call_alarm_local();
	
	while (palarm_mod->triggered) {
		play_alarm1(palarm_mod);
	}
	if (!palarm_mod->triggered) {
		DAC_DeInit();
		GPIO_ResetBits(GPIOD,GPIO_PINS_LOW);
		usart_print("Deactivating alarm!");
	}
}

/*
 * error correction varible
 */
unsigned int us_triggered = 0;

/*
 * Activates the distance sensor and counts the time input GPIOE8 is positive.
 * Activates either global alarm, local alarm or no alarm.
 * Input: Pointer to an alarm struct
 */
void ultrasonic_handler(PALARM_MOD palarm_mod) {
		int time_echo = 0;
		int time_in_while = 0;
		delay(2);
		send_trigger_pulse();
		
		while(!GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_8)) {
			time_in_while++;
			delay(1);
			if(time_in_while>100000)
				return;
		}
		while(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_8)) {
			time_echo++;
			delay(1);
		}
		//uchar d = palarm_mod -> input_distance_cm;
		if(time_echo < time_to_trigger_local_cm(palarm_mod->input_distance_cm,DIST_CONST)) { 
			call_alarm_local();
		}
		else{
			GPIO_ResetBits(GPIOD,GPIO_PINS_LOW);
		}
		if(time_echo < time_to_trigger_sound_cm(palarm_mod->input_distance_cm,DIST_CONST)) { 
			us_triggered++;
			usart_print_num(us_triggered);
			if (us_triggered>3) {
				palarm_mod->triggered = 1;
				call_alarm_sound(palarm_mod);
			}
		}
		else{
			if (us_triggered > 0) us_triggered--;
		}
}

/*
 * Activates global alarm if positive signal is found meassured on GPIOE9.
 * Input: Pointer to an alarm struct
 */
void vibration_handler(PALARM_MOD palarm_mod) {
	if (!GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_9)) {
		palarm_mod->triggered = 1;
		call_alarm_sound(palarm_mod);
	}
}

void send_trigger_pulse(void) {
		GPIO_ResetBits(GPIOE,GPIO_Pin_0);
		delay(2);
		GPIO_SetBits(GPIOE,GPIO_Pin_0);
		delay(10);
		GPIO_ResetBits(GPIOE,GPIO_Pin_0);
}

void check_flags(PALARM_MOD palarm_mod) {
		if(flag_connected) {
			ping_check(palarm_mod);
		}
		if(flag_calibrate) {
				palarm_mod->input_distance_cm = calibrate_ultrasonic();
				flag_calibrate = 0;
		}
}

void init_app(PALARM_MOD palarm_mod, int time_of_last_ping) {
	init_GPIO();
	int current_time;
	systick_time_init(&time_current);
	default_settings(palarm_mod);
	init_DAC();
	time_of_last_ping = time_current;

}

void default_settings(PALARM_MOD palarm_mod) {
	palarm_mod -> id 				= 11;
	palarm_mod -> armed 			= 1;
	palarm_mod -> triggered 		= 0;
	palarm_mod -> connected 		= 1;
	palarm_mod -> input_distance_cm = calibrate_ultrasonic();
	
}

/*
 * Distance sensor IO: 
 * 		Trigger:	GPIOE 0
 * 		Echo: 		GPIOE 8
 * Vibration sensor IO:
 * 		DO: 		GPIOE 9
 * Local alarm (7-segment display):
 * 		output:		GPIOD 0-7
 */
void init_GPIO(void) {
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
	
	GPIO_DeInit(GPIOE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	GPIO_DeInit(GPIOD);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	
}

void bubble_sort(int list[], int n) {
  int c, d, t;

  for (c = 0 ; c < n - 1; c++) {
    for (d = 0 ; d < n - c - 1; d++) {
      if (list[d] > list[d+1]) {
        /* Swapping */
        t         = list[d];
        list[d]   = list[d+1];
        list[d+1] = t;
      }
    }
  }
}

/*
 * Preforms 10 measurements, sorts the measurements in ascending order and picks the median distance.
 * Output: Calibrated distance(cm) - 5 cm
 */
uchar calibrate_ultrasonic(void) {
	int ntests = 10;
	int arr[ntests];
	int median = (ntests/2); //integer division
	
	for (int i = 0;i < ntests;i++) {
		int time_echo = 0;
		send_trigger_pulse();
		while(!GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_8)) {
		}
		while(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_8)) {
			time_echo++;
			delay(1);
		}
		arr[i] = time_echo;
		delay(50000);
	}
	bubble_sort(arr,ntests);
	return (uchar)((arr[median])/(DIST_CONST)-5);
}

//----------------------Alarm_module network functions--------------------------------

/*
 * Activates the alarm if no pings are recieved within the expected time threshold.
 * Input: Pointer to an alarm struct
 */
void ping_check(PALARM_MOD palarm_mod) {
	if(time_current - time_of_last_ping > wait_ping_threshold) {
		palarm_mod -> connected = 0;
		if (palarm_mod -> armed) {
			palarm_mod->triggered = 1;
			call_alarm_sound(palarm_mod);
		}
	}
}

/*
 * Uppdates the alarm_struct according to the information in the configuration message.
 * Input: Pointer to an alarm struct and an instance of a message struct
 */
void can_decode_alarm_cfg(PALARM_MOD palarm_mod, CANMsg* c) {
	palarm_mod->id = c->nodeid;
	for(int i=0; i<3; i++) {
		palarm_mod->cfgarr[i] = c->buff[i+1];
	}
	if (c->buff[2+1] == 0)
		flag_calibrate = 1;
}

/*
 * Uppdates the connection status and answers pings.
 * Input: Instance of a message struct
 */ 
void ping_handler(CANMsg* msg) {
	flag_connected = 1;
	time_of_last_ping = time_current;
    CANMsg c;
    can_createmsg_ping(&c, palarm_list[0]->id, MODULE_ALARM); //these 3 lines could be a send_alarm_ping func
    can_send(&c);
}

void toggle_alarm_handler(CANMsg* msg) {
	palarm_list[0] -> triggered ^= 1;
	usart_print("SEND ALARM");
}

/*
 * Handles configuration messages and prints the uppdated information.
 * Input: Instance of a message struct
 */ 
void alarm_cfg_handler(CANMsg* msg) {
	time_of_last_ping = time_current; //If a configuration msg is recieved the connection must be working.
	can_decode_alarm_cfg(palarm_list[0], msg);
	
	usart_print("Connected is: ");
	usart_print(palarm_list[0]->connected);
	usart_print("armed is: ");
	usart_print(palarm_list[0]->armed);
	usart_print("triggered is: ");
	usart_print(palarm_list[0]->triggered);
    usart_print("input distance is: ");
	usart_print(palarm_list[0]->input_distance_cm);
}

/*
 * Calls the correct message handler depending on the message type.
 * Input: Instance of a message struct
 */ 
void interpret_message(CANMsg* msg) {
    switch(msg->msgid) {
        case MSG_ALARM:
            toggle_alarm_handler(msg);
            break;
        case MSG_PING:
            ping_handler(msg);
            break;
        case 2:
            usart_print("CONFIG");
			if(msg->buff[0] == MODULE_ALARM)
				alarm_cfg_handler(msg);
            break;
        default: 
            usart_print("Invalid message type! No action was taken.");
        break;
    }
}

/*
 * Checks if we are supposed to recieve the message.
 */ 
void receiver(void) {
    CANMsg msg;
    if (can_receive(&msg)) {
		if(msg.nodeid == 11 || msg.nodeid == 15)
			interpret_message(&msg);
    }
    else
        usart_print("***Error: Something went wrong :(");
}

//----------------------Alarm_Module sound functions--------------------------------

DAC_InitTypeDef  DAC_InitStructure;

const uint16_t sawtooth12bit[32] = {
                      2000, 2133, 2266, 2399, 2532, 2662, 2798, 2931, 3064, 3197,
                      3330, 3463, 3596, 3729, 3861, 3995, 0, 133, 266, 399, 
                      532, 665, 798, 931, 1064, 1197, 1330, 1463, 1596, 1729, 1862, 2000};

void TIM6_Config(void) {
  TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;
  /* TIM6 Periph clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
  
  /* --------------------------------------------------------
  TIM3 input clock (TIM6CLK) is set to 2 * APB1 clock (PCLK1), 
  since APB1 prescaler is different from 1.   
    TIM6CLK = 2 * PCLK1  
    TIM6CLK = HCLK / 2 = SystemCoreClock /2 
          
  TIM6 Update event occurs each TIM6CLK/256 

  Note: 
   SystemCoreClock variable holds HCLK frequency and is defined in system_stm32f4xx.c file.
   Each time the core clock (HCLK) changes, user had to call SystemCoreClockUpdate()
   function to update SystemCoreClock variable value. Otherwise, any configuration
   based on this variable will be incorrect.    

  ----------------------------------------------------------- */
  /* Time base configuration */
  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure); 
  TIM_TimeBaseStructure.TIM_Period = 0xFF;          
  TIM_TimeBaseStructure.TIM_Prescaler = 0;       
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;    
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  
  TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);

  /* TIM6 TRGO selection */
  TIM_SelectOutputTrigger(TIM6, TIM_TRGOSource_Update);
  
  /* TIM6 enable counter */
  TIM_Cmd(TIM6, ENABLE);
}

void init_DAC (void) {
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/* DMA1 clock enable */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
	/* GPIOA clock enable (to be used with DAC) */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);                         
	/* DAC Periph clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
	 
	/* DAC channel 1 & 2 (DAC_OUT1 = PA.4)(DAC_OUT2 = PA.5) configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	  
	 /* TIM6 Configuration ------------------------------------------------------*/
	TIM6_Config();  
}

/*
 * Creates a sawtooth sound wave.
 */
void DAC_Ch2_SawTooth(void) {
  DMA_InitTypeDef DMA_InitStructure;
  
  /* DAC channel2 Configuration */
  DAC_InitStructure.DAC_Trigger = DAC_Trigger_T6_TRGO;
  DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
  DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
  DAC_Init(DAC_Channel_2, &DAC_InitStructure);

  /* DMA1_Stream6 channel7 configuration **************************************/
  DMA_DeInit(DMA1_Stream6);
  DMA_InitStructure.DMA_Channel = DMA_Channel_7;  
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)DAC_DHR12R2_ADDRESS;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&sawtooth12bit;
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  DMA_InitStructure.DMA_BufferSize = 32;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA1_Stream6, &DMA_InitStructure);

  /* Enable DMA1_Stream6 */
  DMA_Cmd(DMA1_Stream6, ENABLE);

  /* Enable DAC Channel2 */
  DAC_Cmd(DAC_Channel_2, ENABLE);

  /* Enable DMA for DAC Channel2 */
  DAC_DMACmd(DAC_Channel_2, ENABLE);
}

/*
 * Changes the time period between every soundwave.
 * Input: Desired frequency (Hz).
 */
void play_freq (double freq) {
	TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;
	/* TIM6 Periph clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

	/* Time base configuration */
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure); 
	TIM_TimeBaseStructure.TIM_Period = (65535/(freq/40));      			//0xFFFF = 65.535 = 40Hz
	TIM_TimeBaseStructure.TIM_Prescaler = 0;       						//input (hex) = (65535 / (desired freq / 40 ))
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;    
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  
	TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);

	/* TIM6 TRGO selection */
	TIM_SelectOutputTrigger(TIM6, TIM_TRGOSource_Update);

	/* TIM6 enable counter */
	TIM_Cmd(TIM6, ENABLE);
	DAC_Ch2_SawTooth();
}

void play_alarm1(PALARM_MOD palarm_mod) {
	double i;
	for(i = 0; i < 213;i += 3) {
		if(!palarm_mod->triggered)
			return;
		play_freq((300*sin(i/8)+600)+i);
		delay(20000);
	}
	delay(1000000);
	if(!palarm_mod->triggered)
			return;
	play_freq(40);
	delay(500000);
	for(i = 213; i < 400;i += 3) {
		if(!palarm_mod->triggered)
			return;
		play_freq((100*sin(i/5)+800)-i);
		delay(20000);
	}
	play_freq(8000);
	delay(500000);
}

void play_alarm2(PALARM_MOD palarm_mod) {
	double i;
	for (i = 5000 ; i > 600;i-=25) {
		if(!palarm_mod->triggered)
			return;
		play_freq(i);
		delay(2173);
	}
	for (i = 600 ; i < 5000;i+=25) {
		if(!palarm_mod->triggered)
			return;
		play_freq(i);
		delay(2173);
	}
}