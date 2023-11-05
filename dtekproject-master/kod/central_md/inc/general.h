/*
 * general.h
 * 
*/
#pragma once
#include "../inc/misc.h"
#include "../inc/stm32f4xx_can.h"
#include "../inc/stm32f4xx_rcc.h"
#include "../inc/stm32f4xx_usart.h"
#include "../inc/stm32f4xx_gpio.h"




/*Typedefs*/
typedef unsigned char uchar;
typedef unsigned int uint;

typedef enum {MODULE_CENTRAL,MODULE_ALARM,MODULE_DOOR} MODULETYPE;
typedef enum {MSG_ALARM, MSG_PING, MSG_CFG, MSG_ERROR} MESSAGETYPE;

typedef struct {
	unsigned char id;
	MODULETYPE t;
	unsigned char connected;
	//Config parameters are represented by 5 data fields or
	//A char array with 5 slots
	union {
		struct {
			unsigned char armed;
			unsigned char cfg1; 
			unsigned char cfg2; 
			unsigned char cfg3;
			unsigned char cfg4;
		} cfgfields;
		unsigned char cfgarr[5];
	} 
} MODULE, *PMODULE;




/*Macro Defs*/
#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

#define FOREACH_CMD(CMD) \
        CMD(arm)  \
        CMD(disarm)   \
		CMD(status)   \
        CMD(config)  \
		CMD(printlist)  \
        CMD(help)  \

static const char *CMD_STRING[] = {
    FOREACH_CMD(GENERATE_STRING)
};

#define FOREACH_DOORCFG(CFG) \
		CFG(doortime)   \
		
#define FOREACH_ALARMCFG(CFG) \
        CFG(distancecfg)   \	
		CFG(ultsoundcfg) \
		
enum DOORCFG_ENUM {
	FOREACH_DOORCFG(GENERATE_ENUM)
};

static const char *DOORCFG_STRING[] = {
	FOREACH_DOORCFG(GENERATE_STRING)
};

enum ALARMCFG_ENUM {
	FOREACH_ALARMCFG(GENERATE_ENUM)
};

static const char *ALARMCFG_STRING[] = {
FOREACH_ALARMCFG(GENERATE_STRING)
};




/*code dependent on simulator environment*/
//#define SIMULATOR
#if defined(SIMULATOR)
#define KEYPAD_DELAY 1
#endif

#if !defined(SIMULATOR)
#define KEYPAD_DELAY 1000
#endif

/*other constants*/
#define CONFIGARR_LENGTH 5
#define CANBUFF_SIZE 8
#define USARTBUFF_SIZE 16
#define KEYPADBUFF_SIZE 4
#define MODULELIST_LENGTH 16
#define MODULEFUNCS_LENGTH (sizeof(CMD_STRING)/4 -1)
#define COMMAND_MAXLENGTH 10
#define CONFIGARR_LENGTH 5
#define SETUP_TIME 1000000
#define PING_TIME 1000000

#define DEFAULT_DISTANCE_CFG 50 
#define DEFAULT_DOORTIME_LOC 0
#define DEFAULT_DOORTIME_GLB 10




/*Variables*/
MODULE modulelist[MODULELIST_LENGTH];
int localalarm;
volatile int setupcounter;
volatile long counter;
volatile int alarmcounter;
int moduleindex;
MODULETYPE moduletype;
char usart_buffer[USARTBUFF_SIZE];
char keypad_buffer[KEYPADBUFF_SIZE];




/*Func definitons*/
void SysTick_Handler(void);
void main(void);

/*io.c ***************************************************************/

/*Keypad*/

int keypad_init(GPIO_TypeDef* gpiox);
int keypad_buff_add(char *keypad_buffer[],int i);
int keypad_buff_clr(char *keypad_buffer[]);
int keypad_getcol(void);
void keypad_row_actv(uint32_t row);
char keypad_getnum(void);
int keypad_append_input(char *keypad_buffer[]);
int keypad_verify_code(char *keypad_buffer[]);

/*USART*/

int usart_init(USART_TypeDef* usartx);
int usart_buff_clr(char *usart_buffer);
int usart_buff_add(char *usart_buffer,char c);
int usart_has_input(void);
char usart_getchar(void);
int usart_append_input(char *usart_buffer);
void print_char( char c );
void usart_print(char *s);
void usart_printline(char *s);
void usart_print_num(int num);
void usart_printhelp();
void print_usart_command(int command);
void print_hlp_msg(char *usart_buffer);
int set_cfg_flag(uint8_t moduleid,PMODULE modulelist);
int send_command(uint8_t moduleid,PMODULE modulelist,int n);
int usart_cmd_send(char module_cmd[], uint8_t moduleid,PMODULE modulelist);
int cfg_command_execute(char *usart_buffer, PMODULE modulelist);
int command_execute(char *usart_buffer, PMODULE modulelist);
int usart_cmd_execute(char *usart_buffer,PMODULE modulelist);

/*general.c ***************************************************************/

/*Misc*/

int buffer_full(int buffsize, int buffindex);

/*String functions*/

char *substring(char *dst, const char *src, uint32_t startindex, uint32_t n);
int string_to_num(char str[]);
void clr_str(char *str,int n);
void print_as_string(int num);

/*Module  creation funcs*/

static int module_create(PMODULE p, uint8_t id, MODULETYPE t, uint8_t cfgarr[]);
int module_create_door(PMODULE p, uint8_t id, uint8_t armed, uint8_t doorid, uint8_t doortime_glb, uint8_t doortime_loc);
int module_create_alarm(PMODULE p, uint8_t id, uint8_t armed, uint8_t triggered, uint8_t distancecfg);

/*Other module funcs*/

PMODULE module_find(uint8_t nodeid, PMODULE modulelist);
int module_pingfaliure(PMODULE modulelist);
int module_set_armed(uint8_t val, uint8_t nodeid, PMODULE modulelist[]);
int module_set_cfg(uint8_t cfgfields[], uint8_t moduleid, PMODULE modulelist[]);
//int modulelist_add(CANMsg *msg, PMODULE modulelist, int* moduleindex);

/*Module IO functions*/

int module_status(uint8_t moduleid,PMODULE modulelist);
void module_print_status(MODULE *m);
int module_printlist(PMODULE modulelist);
void module_print(MODULE *m);