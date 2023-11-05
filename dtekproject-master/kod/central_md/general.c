/*
 * 	general.c
 *
 */
#pragma once
#include "inc/general.h"
#include "can/can.h"
#include <math.h>




/*Misc*/
int buffer_full(int buffsize, int buffindex) {
	if(buffindex >= buffsize) return 1;
	return 0;
}

/*String functions*/
char *substring(char *dst, const char *src, uint32_t startindex, uint32_t n)
{
    strncpy(dst, (src + startindex), n);
    return dst;
}

/*
 * Is used to convert several single digit inputs to one several digit output.
 * Input: An array of characters.
 * Output: A number.
 * For example: string_to_num([1,5,2] = 152)
 */
int string_to_num(char str[])
{
    int num = 0;
    int n = strlen(str);
    for(int i=0; i<strlen(str); ++i) {
        if(str[i]-'0' >= 0 && str[i]-'0' <= 9){
        num += (str[i]-'0')*pow(10,n-1);
        n--;
        }
        else if(str[i] == '-'){
            return str[i];
        }
        else{
            usart_printline("Error: Incorrect number.");
            break;
        }
    }
    return num;
}

/*
 * Clears a string
 */
void clr_str(char *str,int n)
{
    memset(str, 0, n);
}




/*Module  creation funcs*/
/* 
 * Fills a module-struct with all values given
 * Usage: Create an empty module, then input a pointer to this module as parameter 1. 
 * The empty module is then filled with values
 * Input: Pointer to a module, the module's MODULETYPE, and the fields armed, cfg1, cfg2...
 * Output: Returns "1" if all parameters were appropriate
 * 
 * This function is used by daughter functions that are module specific
 */
static int module_create(PMODULE p, uint8_t id, MODULETYPE t, uint8_t cfgarr[]) {
    
    if(id > 255 || t > 2) {
        usart_print("module_create error");
        return 0;
    }
    
	p->id = id;
	p->connected = 1;
	p->t = t;
	for(int i=0; i<CONFIGARR_LENGTH; i++) {
		p->cfgarr[i] = cfgarr[i];
	}
    return 1;
}


/* 
 * Fills an empty module-struct with information related to a doormodule. 
 * Input: Pointer to a module, the module's nodeid, and the fields armed, doorid, doortime_glb, doortime_loc
 * Output: Returns "1" if all parameters were appropriate
 * For more information, refer to module_create function above
 */
int module_create_door(PMODULE p, uint8_t id, uint8_t armed, uint8_t doorid, uint8_t doortime_glb, uint8_t doortime_loc) {
	uint8_t tmparr[CONFIGARR_LENGTH];
	memset(tmparr,0,sizeof tmparr);
	tmparr[0] = armed;
	tmparr[1] = doorid;
	tmparr[2] = doortime_glb;
    tmparr[3] = doortime_loc;
	return module_create(p,id,(uint8_t)MODULE_DOOR,tmparr);
}

/* 
 * Fills an empty module-struct with information related to an alarmmodule. 
 * Input: Pointer to a module, the module's nodeid, and the fields armed, triggered, distancecfg
 * Output: Returns "1" if all parameters were appropriate
 * For more information, refer to module_create function above
 */
int module_create_alarm(PMODULE p, uint8_t id, uint8_t armed, uint8_t triggered, uint8_t distancecfg) {
	uint8_t tmparr[CONFIGARR_LENGTH];
	memset(tmparr,0,sizeof tmparr);
	tmparr[0] = armed;
	tmparr[1] = triggered;
	tmparr[2] = distancecfg;
	
	return module_create(p,id,(uint8_t)MODULE_ALARM,tmparr);
}




/*Other module funcs*/

/* 
 * Finds a module in the modulelist using a nodeid
 * Usage: Use this function to retrieve a pointer to the modules memory adr
 * Input: Pointer to a module, and moduleid to search for
 * Output: Returns the pointer to the found module, if it wasn't found, returns 0
 */
PMODULE module_find(uint8_t nodeid, PMODULE modulelist) {
    
	for(int i=0; i < MODULELIST_LENGTH; i++) {
		if(modulelist[i].id == nodeid) {
			return &modulelist[i];
		}
	}
	
	return 0;
}

/* 
 * If any module is disconnected and also armed, return 1
 */
int module_pingfaliure(PMODULE modulelist) {
	for(int i=0; i<MODULELIST_LENGTH; i++) {
		if(modulelist[i].id != 0 && !modulelist[i].connected && modulelist[i].cfgfields.armed) return 1;
	}
	
	return 0;
}

/* 
 * Sets a modules armed-field locally and globally.
 * Input: The desired value, 0 or 1, the modules nodeid, and a pointer to the modulelist
 * Output: 1 if operation was successful, 0 otherwise
 */
int module_set_armed(uint8_t val, uint8_t nodeid, PMODULE modulelist[]) {
	PMODULE m = module_find(nodeid,modulelist);
   int armed_old = m->cfgfields.armed;
    
	if(m->connected == 1 && val < 2 && armed_old != val) {
		m->cfgfields.armed = val;
        
        CANMsg c;
		can_createmsg_cfg(&c,m);
		can_send(&c);
        usart_print("Module sucessfully modified.\n\n");
	}
    else{
        usart_print("Error: incorrect input, command aborted. (Module might already be amred/disarmed)\n\n");
    }
	return 1;
}

/* 
 * Sets a modules config parameters locally and globally.
 * Input: The desired parameters, the modules nodeid, and a pointer to the modulelist
 * Output: 1 if operation was successful, 0 otherwise
 */
int module_set_cfg(uint8_t cfgfields[], uint8_t moduleid, PMODULE modulelist[]) {

    PMODULE m = module_find(moduleid,modulelist);
    
    uint8_t doorid = m->cfgarr[1];
    if(m->cfgarr[1] == '-'){ // 8 = MAX_DOORS
        if(cfgfields[0]>=0 && cfgfields[0]<=8){
        m->cfgarr[1] = cfgfields[0];
        }
        else{
            usart_printline("Error: Incorrect amount of doors.\n");
        }
    }
    else if(m->connected == 1){
        
        int k = 0;
        if(m->t == MODULE_ALARM) k++;
        
        for(int i=0; i<CONFIGARR_LENGTH; i++) {
            if(cfgfields[i] != '-') m->cfgarr[i+k] = cfgfields[i];
        }
     
    }
    else{
    usart_print("Error: No module with that ID connected, command aborted.\n\n");
    return 0;
    }
    
    CANMsg c;
    can_createmsg_cfg(&c,m);
    can_send(&c);
    if(m->t == MODULE_DOOR && doorid != '-') m->cfgarr[1] = doorid; // is this right??

    usart_print("Module sucessfully configurated.\n\n");
    return 1;
}

/* 
 * Adds a module to the modulelist using information extracted from a can-message
 * Input: A can-message, a pointer to the modulelist, the current empty index of the modulelist
 * Output: 1 if operation was successful, 0 otherwise
 */
int modulelist_add(CANMsg *msg, PMODULE modulelist, int* moduleindex) {
    
    int i = (*moduleindex);
    PMODULE m = (modulelist + i);
    
	if(i < MODULELIST_LENGTH) {
		
		switch(msg->buff[0]) {
			case MODULE_ALARM:
			module_create_alarm(m,msg->nodeid,1,0, 0);
			break;
			case MODULE_DOOR:
            usart_print("A doormodule has been added to the system. Configure the amount of doors on this module with \"config\"\n");
			module_create_door(m,msg->nodeid,1,'-',DEFAULT_DOORTIME_LOC, DEFAULT_DOORTIME_GLB);
			break;
		}
        
        (*moduleindex)++;
        
		return 1;
	}
	return 0;
}




/*Module IO functions*/

/* 
 * Prints the status of a module if it is connected
 * Input: The id of the searched module
 * Output: 1 if operation was successful, 0 otherwise
 */
int module_status(uint8_t moduleid,PMODULE modulelist)
{
    PMODULE m = module_find(moduleid,modulelist);
    if(m->connected == 1){
    module_print(m);
    module_print_status(m);
    return 1;
    }
    else{
        usart_print("Error: No module with that ID connected, command aborted.\n\n");
    }
    return 0;
}

/* 
 * Prints the status of a module
 */
void module_print_status(MODULE *m)
{
    if(m->t == MODULE_ALARM){
        usart_print("armed: ");
        usart_print_num(m->cfgarr[0]);
        usart_print("triggered: ");
        usart_print_num(m->cfgarr[1]);
        usart_print("distancecfg: ");
        usart_print_num(m->cfgarr[2]);
    }
    else if(m->t == MODULE_DOOR){
        usart_print("armed: ");
        usart_print_num(m->cfgarr[0]);
        if(m->cfgarr[1] == '-'){
            usart_printline("door ID's: N/A, use config to set amount of doors.");
        }
        else{
            usart_print("door id's: 1-");
            usart_print_num(m->cfgarr[1]);
        }
        
        usart_print("last modified global door time: ");
        usart_print_num(m->cfgarr[2]);
        usart_print("last modified local door time: ");
        usart_print_num(m->cfgarr[3]);
    }
    print_char('\n');
}


/* 
 * Prints basic info of all connected modules in a list
 */
int module_printlist(PMODULE modulelist)
{
    PMODULE m;
    int module_count = 0;
    for(int i=0; i < MODULELIST_LENGTH; i++) {
        m = module_find(modulelist[i].id,modulelist);
        if(m->connected){
		module_print(m);
        print_char('\n');
        module_count++;
        }
    }
    if(!module_count){
        usart_print("No modules connected!\n\n");
    }
}

/* 
 * Prints basic info about a module
 */
void module_print(MODULE *m)
{
    usart_print("module type: ");
    if(m->t == MODULE_ALARM){
	usart_print("Alarm");
    }
    else{
        usart_print("Door");
    }
    usart_print("\nmodule-ID: ");
    usart_print_num(m->id);
}






