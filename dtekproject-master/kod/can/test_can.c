/*
 * 	test_can.c
 *
 */

#pragma once
#include "../inc/general.h"
#include "can.h"

int _test_can_createmsg() {
    CANMsg c, c2, c3, c4;
    MODULE m, m2;

    //SHOULD GIVE MSGID 0, NODEID 12
	can_createmsg_alarm(&c, 12, MODULE_CENTRAL);
	can_send(&c);
    //SHOULD GIVE MSGID 1, NODEID 13
    can_createmsg_ping(&c2,13,MODULE_CENTRAL);
    can_send(&c2);
    
    module_create_alarm(&m,14,1,20,20);
    //SHOULD GIVE MSGID 2, NODEID 14
    can_createmsg_cfg(&c3,&m);
    can_send(&c3);
    
    //SHOULD GIVE MSGID 3, NODEID 15
    module_create_door(&m2,15,1,20,20,20);
    can_createmsg_cfg(&c4,&m2);
    can_send(&c4); 
    
    return 1;
}

//*FUNCTIONS USED FOR DEBUGGING*/
void _sendalarm() {
    CANMsg c;
    memset(&c,0,sizeof c);
    can_createmsg_alarm(&c, 11, MODULE_CENTRAL);
	can_send(&c);
}    

void _sendping() {
    CANMsg c;
    can_createmsg_ping(&c, 11, MODULE_ALARM);
	can_send(&c);
    CANMsg c2;
    can_createmsg_ping(&c2, 3, MODULE_DOOR);
	can_send(&c2);
}                   

void _senddoorcfg() {
    MODULE m;
    CANMsg c;
    module_create_door(&m,3,1,2,8,10);
    can_createmsg_cfg(&c,&m);
    can_send(&c); 
}

void _sendalarmcfg() {
    MODULE m;
    CANMsg c;
    module_create_alarm(&m,11,1,5,5);
    can_createmsg_cfg(&c,&m);
    can_send(&c); 
}
