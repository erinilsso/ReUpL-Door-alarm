/*
        Code for the door unit.
        Has the responsibility of checking the doors status, controlling the locac alarm (light),
        and telling the central unit when to set of global alarm.
*/
#include "door.h"


void main()
{   
	DOOR ett  =  {1, 1,   0, 0, 1, GLOBAL_TIME_DEFAULT, LOCAL_TIME_DEFAULT, 0};
    DOOR tva  =  {2, 2,   0, 0, 0, GLOBAL_TIME_DEFAULT, LOCAL_TIME_DEFAULT, 0};
    DOOR tre  =  {3, 4,   0, 0, 0, GLOBAL_TIME_DEFAULT, LOCAL_TIME_DEFAULT, 0};
    DOOR fyra =  {4, 8,   0, 0, 0, GLOBAL_TIME_DEFAULT, LOCAL_TIME_DEFAULT, 0};
    DOOR fem  =  {5, 16,  0, 0, 0, GLOBAL_TIME_DEFAULT, LOCAL_TIME_DEFAULT, 0};
    DOOR sex  =  {6, 32,  0, 0, 0, GLOBAL_TIME_DEFAULT, LOCAL_TIME_DEFAULT, 0};
    DOOR sju  =  {7, 64,  0, 0, 0, GLOBAL_TIME_DEFAULT, LOCAL_TIME_DEFAULT, 0};
    DOOR atta  = {8, 128, 0, 0, 0, GLOBAL_TIME_DEFAULT, LOCAL_TIME_DEFAULT, 0};
	doors[0] = &ett; 
	doors[1] = &tva; 
	doors[2] = &tre; 
	doors[3] = &fyra; 
	doors[4] = &fem; 
	doors[5] = &sex; 
	doors[6] = &sju; 
	doors[7] = &atta; 
	
    gpio_init();	
    systick_time_init(&time_current);
	can1_init(receiver);
	for(int ic = 0; ic < door_length; ic++){
		status_set(doors[ic]);
	}
    while(1){
		time_manage(doors);
		status_check(doors);
	}
}

// **** Boilerplate startup code ****
void startup(void) __attribute__((naked)) __attribute__((section(".start_section")));

void startup(void)
{
    __asm volatile(" LDR R0,=0x2001C000\n" /* set stack */
                   " MOV SP,R0\n"
                   " BL main\n" /* call main */
                   " B .");
    while(1)
	; // should not be needed
}