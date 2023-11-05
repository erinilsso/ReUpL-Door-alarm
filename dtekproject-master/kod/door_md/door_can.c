#include "door.h"

/* 
 * Sends an alarm on the CAN bus
 * Usage:  
 * Input: Sender ID
 * Output: void
 */
void send_alarm(int id)
{
    CANMsg alarm_msg;
    can_createmsg_alarm(&alarm_msg, NODE_ID, MODULE_DOOR);
    can_send(&alarm_msg);
}

/* 
 * Configures a door according to a configure message
 * Usage:  
 * Input: CANMsg*, a pointer to a can door configure message
 * Output: void
 */
void door_configure(CANMsg* msg)
{
    int door_id = msg->buff[2];
    int door_global = msg->buff[3];
    int armed = msg->buff[1];
	int door_local = msg->buff[4];
	
	if(door_id > 8 || door_id < 0){
		return;
	}
	
	if(door_global<0){
		door_global=0;
	}
	if(door_local<0){
		door_local=0;
	}
  
	if(initiated == 0){
		door_length = door_id;
		for(int c = 0; c < door_id; c++){
			doors[c]->armed = armed;
			doors[c]->global_time_limit = door_global*SEC_MULTIPLE;
			doors[c]->local_time_limit = door_local*SEC_MULTIPLE;
			initiated = 1;
			status_set(doors[c]);
		}
	}
	else{
		doors[door_id-1]->armed = armed;
		doors[door_id-1]->global_time_limit = door_global*SEC_MULTIPLE;
		doors[door_id-1]->local_time_limit = door_local*SEC_MULTIPLE;
		status_set(doors[door_id-1]);
	}
	usart_print("Door configured");
}

/* 
 * Responds to a ping message with another ping message 
 * Usage: 
 * Input: CANMsg*, a pointer to a Can ping mesasge
 * Output:
 */
void ping_handler(CANMsg* msg)	
{
	if(msg->dir == 1) {
		CANMsg c;
		can_createmsg_ping(&c, 3, MODULE_DOOR);
		can_send(&c);
	}
}

/* 
 * Intprets a Can message and executes the approptiate handler function
 * Usage: 
 * Input: CANMsg*, a pointer to a Can ping mesasge
 * Output:
 */
void interpret_message(CANMsg* msg)
{
    switch(msg->msgid){
		case MSG_PING:
		ping_handler(msg);
		break;
		case MSG_CFG:
		if(msg->buff[0] == MODULE_DOOR){
		usart_print("CONFIG");
		door_configure(msg);
		}
		break;
		default:
		usart_print("Invalid message type! No action was taken.");
		break;
    }
}

/* 
 * Receives a CAN message and sends it to the interpreter if it is meant for the current node
 * Usage: Use as an interupt handler for CAN
 * Input: Void
 * Output: Void
 */
void receiver(void)
{
    CANMsg msg;
	
    if(can_receive(&msg)){
		if(is_valid_msg(&msg) && (msg.nodeid == 15 || msg.nodeid == 3)) {
		
			usart_print_num(msg.nodeid);
			interpret_message(&msg);
		}
    }
	else{
		usart_print("***Error: Something went wrong :(");
	}
}