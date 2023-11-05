/*
 * 	can_central.c
 *
 */
#pragma once
#include "inc/general.h"
#include "can/can.h"

/*
 * Reciever function for CAN-messages
 * Usage: This function needs to be passed as an argument to the function
 * CAN_Init. Then msg_reciever will be called everytime a CAN-message is received
 * Input: void
 * Output: void
 */
void msg_reciever(void) {
    CANMsg msg;
    
    if (can_receive(&msg)) {
		msg_interpret(&msg);
	}
	else
        usart_print("***Error: Something went wrong :(\n");
}

void msg_interpret(CANMsg *msg) {
	PMODULE sender = module_find(msg->nodeid,&modulelist);
    switch(msg->msgid) {
		case MSG_ALARM:
			msg_alarmhandler(msg,sender);
		break;
        case MSG_PING:
			msg_pinghandler(msg,sender);
		break;
        case (MSG_CFG):
            msg_cfghandler(msg,sender);
		break;
        default:
			usart_print("Invalid message type! No action was taken.\n");
		break;
    }
}

/*
 * Handler for ping-messages. The function adds modules to the module list
 * during the startup phase and sets senders of the ping to connected after
 * the startup phase
 * Input: msg: Received CANMsg, sender: Sender of the ping
 * Output: void
 */
void msg_pinghandler(CANMsg *msg, PMODULE sender) {

	if(sender == 0 && setupcounter != 0 && msg->buff[0] != 0) {
		modulelist_add(msg,&modulelist,&moduleindex);
	}
	else {
		sender->connected = 1;
	}
}

/*
 * Handler for alarm-messages
 * Input: msg: Received CANMsg, sender: Sender of the ping
 * Output: void
 */
void msg_alarmhandler(CANMsg *msg, PMODULE sender) {
	usart_print("MSG_ALARM recieved.\n");

	//Alarm only counts if module is "armed"
	if(sender->cfgfields.armed == 1) {
        usart_print("SENDER ARMED.\n");
        if(sender->t == MODULE_ALARM) sender->cfgfields.cfg2 = 1;
        else {
            can_sendalarm_broadcast();
        }
		localalarm = 1;
		keypad_buff_clr(keypad_buffer);
	}
}

/*
 * Handler for config-messages. Only for debugging purposes right now.
 * Input: msg: Received CANMsg, sender: Sender of the ping
 * Output: void
 */
void msg_cfghandler(CANMsg *msg, PMODULE sender) {

    switch(msg->buff[0]) {
        case MODULE_ALARM:
        usart_print("ALARMCFG recieved.\n");
        break;
        case MODULE_DOOR:
        usart_print("DOORCFG recieved.\n");
        break;
        default:
        usart_print("CANMsg error: Module type does not exist.");
    }

}
