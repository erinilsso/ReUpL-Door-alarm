#include "alarm_unit.h"

PALARM_MOD palarm_list[32];
PMODULE ptmp_mod;



__attribute__((naked)) __attribute__((section (".start_section")) ) 
void startup ( void )
{
__asm__ volatile(" LDR R0,=0x2001C000\n");        /* set stack */
__asm__ volatile(" MOV SP,R0\n");
__asm__ volatile(" BL main\n");                    /* call main */
__asm__ volatile(".L1: B .L1\n");                /* never return */
}

void main(void) {
	ALARM_MOD alarm_mod;
	palarm_list[0] = &alarm_mod;
    init_app(&alarm_mod, time_of_last_ping);
    can1_init(receiver);
	CANMsg c;
	while(1) {
		while(alarm_mod.armed && alarm_mod.connected && !alarm_mod.triggered) {
			check_flags(&alarm_mod);
			delay(2); 
			ultrasonic_handler(&alarm_mod);
			delay(2);
			vibration_handler(&alarm_mod);
			delay(10000);
		}
		GPIO_ResetBits(GPIOD,GPIO_PINS_LOW);
		check_flags(&alarm_mod);
		if (alarm_mod.triggered||alarm_mod.connected == 0) {
			call_alarm_sound(&alarm_mod);
		}
	}
	
}