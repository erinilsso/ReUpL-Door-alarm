#include "misc.h"
#include "system_stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx.h"
#include "core_cm4.h"
#include "stm32f4xx_dac.h"
#include "stm32f4xx_dma.h"
#include "stm32f4xx_tim.h"
#include "can.h"
#include "systick.h"
#include "general.h"

volatile int flag_calibrate;
volatile int flag_connected;

extern int wait_ping_threshold;

#define DIST_CONST ((float) 28.9)
#define GPIO_PINS_LOW (GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7)
#define GPIO_PINS_HIGH (GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15)
#define DAC_DHR12R2_ADDRESS   	0x40007414
#define DAC_DHR8R1_ADDRESS    	0x40007410

typedef struct {
	uchar id;
	uchar connected;
	union {
		struct {	
			uchar armed;
			uchar triggered;
			uchar input_distance_cm;
		};
		uchar cfgarr[3];
	}
} *PALARM_MOD, ALARM_MOD;

//Global variables for interrupts and Manual tuning
extern PALARM_MOD palarm_list[32];
extern PMODULE ptmp_mod;
int time_of_last_ping;

//------------Alarm_unit logic------------
int distance_to_trigger_cm (int input_distance_cm, float distance_variable);
void call_alarm_local(void);
void call_alarm_sound(PALARM_MOD palarm_mod);
void ultrasonic_handler(PALARM_MOD palarm_mod);
void vibration_handler(PALARM_MOD palarm_mod);
void send_trigger_pulse(void);
void lightD (void);
void check_flags(PALARM_MOD palarm_mod);
void init_app(PALARM_MOD alarm_mod, int time_of_last_ping);
void init_GPIO(void);
//-----------CAN Communication logic-------------
void ping_check(PALARM_MOD palarm_mod);
void can_decode_alarm_cfg(PALARM_MOD p_m, CANMsg* c);
void ping_handler(CANMsg* msg);
void toggle_alarm_handler(CANMsg* msg);
void alarm_cfg_handler(CANMsg* msg);
void interpret_message(CANMsg* msg);
void receiver(void); //<<<-----------------Fett tvek om denna ska vara här-----------------------
//-----------Alarm sound logic--------------
void TIM6_Config(void);
void init_DAC(void);
void DAC_Ch2_SawTooth(void);
void play_freq(double freq);
void play_alarm1(PALARM_MOD palarm_mod);
void play_alarm2(PALARM_MOD palarm_mod);

void bubble_sort(int list[], int n);

uchar calibrate_ultrasonic(void);
