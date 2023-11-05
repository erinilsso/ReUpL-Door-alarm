/*
 * 	test.c
 *
 */

#pragma once
#include "door.h"
/*
 Tests if a door is configed correctly, if so returns 1.
*/
int _test_door_config() {
	CANMsg c;
    MODULE m;
	module_create_door(&m,1,1,1,20,10);
    can_createmsg_cfg(&c,&m);
	door_configure(c);
	
	if(doors[0]->armed){
	return 1;
	}
	return 0;
}

/*
 Tests if alarm_update functions correctly; if the light status and time stamp is updated.
 */
int _test_door_alarm_update() {
	doors[0]->time_stamp = 1;
	doors[0]->local_time_limit = 1;
	doors[1]->time_stamp = 1;
	doors[1]->global_time_limit = 2;
	door_length = 2;
	time_current = 100000;
	alarm_update(doors);
	if(doors[0]->light_status == 1 && doors[1]->time_stamp == 0){
	return 1;
	}
	return 0;
}
