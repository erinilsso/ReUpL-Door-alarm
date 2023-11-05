#pragma once
#include "../central_md/inc/general.h"




// CANMsg represents a message that is to be sent or has been received. 
// msgId is the 7 most significatnt bits of the 11 arbitration bits in a CAN-message.
//   This means that lower msgIds should have priority over higher ones if there is a 
//   collision on the CAN-bus (not tested)
// nodeId is intended to represent the ID of the sending device. 
//   This uses the four least significant bits of the 11 arbitration-bist in a CAN-message.
//   (this should mean that lower nodeId have priority accesst to the bus if msgId is identical.
typedef struct {
	uchar dir;
	uchar msgid;  //Valid values: 0-127
	uchar nodeid; //Valid values: 0-15
	uchar length; //Valid values: 0-8
	uchar buff[8]; // A message carries at most 8 bytes of data
} CANMsg;

typedef void (*VoidFunction)(void);




/*Variables*/


/*Func definitons*/
void msg_reciever(void);
void msg_interpret(CANMsg *msg);
void msg_pinghandler(CANMsg *msg, PMODULE sender);
void msg_alarmhandler(CANMsg *msg, PMODULE sender);
void msg_cfg_alarm(CANMsg *msg, PMODULE sender);
void msg_cfg_door(CANMsg *msg, PMODULE sender);

