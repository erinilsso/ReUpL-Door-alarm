/*
 * 	general.c
 *
 */
#include "can.h"
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_can.h"
#include "stm32f4xx_rcc.h"
#include "inc/general.h"
#define __IRQ_PRIORITY		2
#define	CAN1_IRQ_VECTOR		(0x2001C000+0x90)
#define __ENABLED_PRIORITY	3


/*
 * Initialize CAN controller for CAN1
 * interrupt is called each time a message is ready to be received
 * Usage: Define a function to pass as the interrupt parameter
 * Input: Vector for the interrupt function
 * Output: void
 */
void can1_init(VoidFunction interrupt) {
    GPIO_InitTypeDef GPIO_InitStructure;
    CAN_FilterInitTypeDef  CAN_FilterInitStructure;

    GPIO_StructInit( &GPIO_InitStructure );

    GPIO_DeInit( GPIOB );
    CAN_DeInit( CAN1 );
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_CAN1, ENABLE );
    RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOB, ENABLE);

    /* Connect CAN1 pins to AF */
    /* PB9 - CAN1 TX */
    /* PB8 - CAN1 RX  */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_CAN1);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_CAN1);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);


    /* CAN filter init */
    CAN_FilterInitStructure.CAN_FilterNumber = CAN_Filter_FIFO0;
    CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
    CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
    CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;
    CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
    CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000; // 0 in a position means ignore that bit
    CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
    CAN_FilterInitStructure.CAN_FilterFIFOAssignment = 0;
    CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
    CAN_FilterInit(&CAN_FilterInitStructure);

    CAN_InitTypeDef CAN_InitStructure;
    CAN_InitStructure.CAN_TTCM = DISABLE; // time-triggered communication mode = DISABLED
    CAN_InitStructure.CAN_ABOM = DISABLE; // automatic bus-off management mode = DISABLED
    CAN_InitStructure.CAN_AWUM = DISABLE; // automatic wake-up mode = DISABLED
    CAN_InitStructure.CAN_NART = DISABLE; // non-automatic retransmission mode = DISABLED
    CAN_InitStructure.CAN_RFLM = DISABLE; // receive FIFO locked mode = DISABLED
    CAN_InitStructure.CAN_TXFP = DISABLE; // transmit FIFO priority = DISABLED
    CAN_InitStructure.CAN_Mode = CAN_Mode_Normal; // normal CAN mode
    //
    // 42 MHz clock on APB1
    // Prescaler = 7 => time quanta tq = 1/6 us
    // Bit time = tq*(SJW + BS1 + BS2)
    // See figure 346 in F407 - Reference Manual.pdf
    //
    CAN_InitStructure.CAN_SJW = CAN_SJW_1tq; // synchronization jump width = 1
    CAN_InitStructure.CAN_BS1 = CAN_BS1_3tq;
    CAN_InitStructure.CAN_BS2 = CAN_BS2_4tq;
    CAN_InitStructure.CAN_Prescaler = 7; // baudrate 750kbps

    if (CAN_Init(CAN1, &CAN_InitStructure) == CAN_InitStatus_Failed)
        usart_print("CAN #1 Init failed!");
    else
        usart_print("CAN #1 Init successful!");

    *((void (**)(void) ) CAN1_IRQ_VECTOR ) = interrupt;

    NVIC_InitTypeDef NVIC_InitStructure;

    /* Enable systick interrupt, set priority */
    NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);
}


/*
 * Reads a message from the CAN bus and copy it to the supplied message data structure
 * Input: Message to contain receive results
 * Output: 0 if there was no message to be received and 1 otherwise
 */
int can_receive(CANMsg *msg){
    uint8_t index;
    CanRxMsg RxMessage;

    if (CAN_GetFlagStatus( CAN1, CAN_FLAG_FMP0) != SET)  // Data received in FIFO0
        return 0;


    CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);

	// ==================== Fetch dir-bit and reset last bit in StdId
    msg->dir = RxMessage.StdId>>10;
	RxMessage.StdId &= ~(1<<10);
	// ==============================================================

    msg->msgid  = (RxMessage.StdId >> 4) & 0x7F;
    msg->nodeid = RxMessage.StdId & 0x0F;
    msg->length = (RxMessage.DLC & 0x0F);

    for (index = 0; index < msg->length; index++) {
        // Get received data
        msg->buff[index] = RxMessage.Data[index];
    }
    return 1;
}

/*
 * Copy the given message to a transmit buffer and send the message
 * Input: Message to be sent
 * Output: 0 if transmission failed and 1 if successful
 */
int can_send(CANMsg *msg){

	// This function needs functionality for CAN-bus conflicts
    uint8_t index;
    CAN_TypeDef* canport = CAN1;
    CanTxMsg TxMessage;
    uint8_t TransmitMailbox = 0;

    //set the transmit ID, standard identifiers are used, combine IDs
    TxMessage.StdId = (msg->msgid<<4) + msg->nodeid;

	TxMessage.StdId |= (msg->dir<<10);					// Add the dir-bit
    TxMessage.RTR = CAN_RTR_Data;
    TxMessage.IDE = CAN_Id_Standard;
    if (msg->length > 8)
        msg->length = 8;
    TxMessage.DLC = msg->length; // set number of bytes to send

    for (index = 0; index < msg->length; index++) {
        TxMessage.Data[index] = msg->buff[index]; //copy data to buffer
    }

    TransmitMailbox = CAN_Transmit(canport, &TxMessage);

    if (TransmitMailbox == CAN_TxStatus_NoMailBox) {
        usart_print("CAN TxBuf full!\n\r");
        return 0;
    }

#ifdef __CAN_TxAck
    while (CAN_TransmitStatus(canport, TransmitMailbox) == CAN_TxStatus_Pending) ;

    if (CAN_TransmitStatus(canport, TransmitMailbox) == CAN_TxStatus_Failed) {
        usart_print("CAN Tx fail!\n\r");
        return 0;
    }
#endif

    return 1;
}




/*Message creation, encoding and decoding*/
/*
 * Creates a CANMsg with specified parameters
 * Input: c: Resulting CANMsg, dir: Message direction, msgid: Message ID, nodeid: Node ID, length: Length of buffer, buff: Data buffer
 * Output: 1 if the message creation succeeded and 0 otherwise
 */
static int can_createmsg(CANMsg *c, uint8_t dir, uint8_t msgid, uint8_t nodeid, uint8_t length, uint8_t buff[CANBUFF_SIZE]) {

    memset(c->buff,0,sizeof(c->buff));

    if(dir > 1 || msgid > 2 || nodeid > 255 || length > 8) {
        usart_print("can_createmsg error");
        return 0;
    }

    c->dir = dir;
	c->msgid = msgid;
	c->nodeid = nodeid;
	c->length = length;

	for(int i=0; i<CANBUFF_SIZE; i++) {
		c->buff[i] = buff[i];
	}

    return 1;
}


/*
 * Creates a ping CANMsg with specified parameters
 * Input: c: Resulting CANMsg, nodeid: Node ID, sender_t: Type of module who sent the message
 * Output: 1 if the message creation succeeded and 0 otherwise
 */
int can_createmsg_ping(CANMsg *c, uint8_t nodeid, MODULETYPE sender_t) {
	uint8_t buff[1];
	uint8_t msgid = (unsigned char) MSG_PING;
	buff[0] = sender_t;
	uint8_t dir = 0;
	if(sender_t == MODULE_CENTRAL) {
		dir    = 1;
	}

	return can_createmsg(c,dir,msgid,nodeid,sizeof(buff),buff);
}


/*
 * Creates an alarm CANMsg with specified parameters
 * Input: c: Resulting CANMsg, nodeid: Node ID, sender_t: Type of module who sent the message
 * Output: 1 if the message creation succeeded and 0 otherwise
 */
int can_createmsg_alarm(CANMsg *c, uint8_t nodeid, MODULETYPE sender_t) {

    uint8_t buff[1];
	uint8_t msgid = (uint8_t)MSG_ALARM;
    buff[0] = sender_t;
    uint8_t dir = 0;
	if(sender_t == MODULE_CENTRAL) {
		dir    = 1;
	}

	return can_createmsg(c,dir,msgid, nodeid, sizeof(buff), buff);
}


/*
 * Creates a config CANMsg with specified parameters
 * Usage: Create a new module containing the new set of desired configuration and pass the pointer as parameter: m
 * Input: c: Resulting CANMsg, m: The new module configuration
 * Output: 1 if the message creation succeeded and 0 otherwise
 */
int can_createmsg_cfg(CANMsg *c,PMODULE m) {
	uint8_t buff[CANBUFF_SIZE];
	uint8_t msgid = (uint8_t) MSG_CFG;
    memset(buff,0,sizeof(buff));

	//Put module type as data field 1
	buff[0] = m->t;
	for(int i=0; i<CONFIGARR_LENGTH; i++) {
		buff[i+1] = m->cfgarr[i];
	}

    buff[5] = m->cfgarr[3];

	//+1 because field one represents module type
	//Dir is one because other modules cant configure central unit
	return can_createmsg(c,1,msgid, m->id, sizeof(buff), buff);
}




/*
 * Decodes a CANMsg into a PMODULE
 * Usage: Create an empty module and pass the pointer as parameter: m
 * Input: m: Resulting module, c: CANMsg containing configs
 * Output: 1 if the decode succeeded and 0 otherwise
 */
int can_decode_cfg(PMODULE m, CANMsg *c) {
	m->id = c->nodeid;
	m->t  = c->buff[0];
	m->connected = 0;
	for(int i=0; i<CONFIGARR_LENGTH; i++) {
		m->cfgarr[i] = c->buff[i+1];
	}
    return 1;
}


/*
 * Puts a ping message on the CAN-bus
 * Input: c: CANMsg to be sent, rcv_id: ID of target node, sender_t: Module type of the sending node
 * Output: 1 if transmission was succesful and 0 if it wasn't
 */
//Functions to send messages, all return 1 if message was sent successfully
int can_sendping(uint8_t rcv_id, MODULETYPE sender_t) {
    CANMsg c;
    can_createmsg_ping(&c, rcv_id, sender_t);
	can_send(&c);
    return 1;
}

/*
 * Puts a broadcast message on the CAN-bus (sends to al modules)
 * Output: 1 if transmission was successfull and 0 if it wasn't
 */
int can_sendping_broadcast() {
    return can_sendping(15,MODULE_CENTRAL);
}

/*
 * Puts an alarm message on the CAN-buss
 * Input: c: CANMsg to be sent, rcv_id: ID of target node, sender_t: Module type of the sending node
 * Output: 1 if transmission was succesful and 0 if it wasn't
 */
int can_sendalarm(uint8_t rcv_id, MODULETYPE sender_t) {
    CANMsg c;
	can_createmsg_alarm(&c,rcv_id,sender_t);
	return can_send(&c);
}


/*
 * Sends an alarm to all connected alarmmodules
 * Output: 1 if all transmissions were successfull and 0 if it wasn't
 */
int can_sendalarm_broadcast() {
    return can_sendalarm(15,MODULE_CENTRAL);
}


/*
 * Puts a config message on the CAN-buss
 * Usage: Create a new module containing the new set of desired configuration and pass the pointer as parameter: m
 * Input: m: The new module configuration, c: CANMsg to be sent
 * Output: void
 */
int can_sendcfg(PMODULE m, CANMsg *c) {
	can_createmsg_cfg(c,m);
	return can_send(c);
}


/*
 * Checks wether or not the CANMsg is valid
 * Input: c: CANMsg to be checked
 * Output: 1 if the message is a valid CANMsg and 0 otherwise
 */
int is_valid_msg(CANMsg* msg) {

	return 	(msg->dir == 0 || msg->dir == 1	&&
			(msg->msgid == MSG_ALARM 	||
			msg->msgid == MSG_PING 		||
			msg->msgid == MSG_CFG		) &&
			msg->length < 8					);
}
