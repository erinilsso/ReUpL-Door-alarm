#include "door.h"

/* 
 * Changes a door's values and turns on / off appropriate lights 
 * Usage: Call when a door has opened or closed. Then it acordingly change the door's values.
 * Input: PDOOR, a pointer to a door struct.
 * Output: void.
 */
void status_change(PDOOR door)
{
    if(!door->armed){
		door->light_status = 0;
		door->door_status = !door->door_status;
		green_light_on(door->pin);
		red_light_off(door->pin);
    }
	else{
		green_light_off(door->pin);
		door->light_status = !door->door_status;
		door->door_status = !door->door_status;
		if(!door->light_status){
			red_light_off(door->pin);
		}
		if(door->time_stamp != 0){
			door->time_stamp = 0;
		}
		else if(door->door_status){
			door->time_stamp = time_current + 1;
		}
    }
}

/* 
 * Sets a door's values.
 * Usage: Used to set a door's values when it's previous state is not usable. Ex when newly configed.
 * Input: PDOOR, a pointer to a door struct.
 * Output: void.
 */
void status_set(PDOOR door)
{
	if(door->armed){
		green_light_off(door->pin);
		unsigned char input = GPIO_ReadInputData(GPIOE);
		input = ~input;
		if(door->pin & input){
			door->door_status = 1;
			door->time_stamp = time_current;
			red_light_on(door->pin);
		}
		else{
			door->door_status = 0;
			door->time_stamp = 0;
			red_light_off(door->pin);
		}
			
	}
	else{
		green_light_on(door->pin);
		red_light_off(door->pin);
	}
}


/* 
 * Checks if any doors local or global alarm should go off.
 * Usage:  Use periodically to make sure alarms go off.
 * Input: PDOOR[], a list of pointers to door structd
 * Output: void.
 */
void alarm_update(PDOOR doors[])
{
    for(int i = 0; i < door_length; i++){
		if(doors[i]->time_stamp && (doors[i]->local_time_limit < time_current - doors[i]->time_stamp)){
			red_light_on(doors[i]->pin);
			doors[i]->light_status=1;
		}
		if(doors[i]->time_stamp && (doors[i]->global_time_limit < time_current - doors[i]->time_stamp)){
			doors[i]->time_stamp = 0;
			send_alarm(doors[i]->id);
			usart_print("ALARM");
			break;
		}
	}
}

/* 
 * Checks if input from door sensors have changed, calls on status change if true.
 * Usage: Makes sure door_status in only called when necesary and on the right door.
 * Input: PDOOR[], a list containing pointers to door structs.
 * Output: void
 */
void status_check(PDOOR doors[])
{
    // checks for status change
	unsigned char input = GPIO_ReadInputData(GPIOE);
	input = ~input;
    for(int i = 0; i < (door_length); i++){
		if((doors[i]->door_status) != ((doors[i]->pin & input ) / doors[i]->pin)){
			status_change(doors[i]);
		}
    }
}

/* 
 * Periodically updates time based functions and variables.
 * Usage: Use in main loop to periodically check if alrms should be triggered. 
 * Input: PDOOR[], a list containing pointers to door structs.
 * Output: void.
 */
void time_manage(PDOOR doors[])
{
    if((time_current - last_time) > 437500){
		alarm_update(doors);
		last_time = time_current;
	}
}
