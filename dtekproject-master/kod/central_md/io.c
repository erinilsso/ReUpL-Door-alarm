/*
 * 	io.c
 *
 */
#pragma once
#include "inc/general.h"
#include "../inc/stm32f4xx_usart.h"
#include "../inc/stm32f4xx_gpio.h"




/*Variables*/ 
static int usart_buffindex = 0;
static int keypad_buffindex = 0;
static int kp_buff_ready = 1;
static const char keypadcode[] = {1,2,3,4};
static int cfg_flag = 0;
static uint8_t current_moduleid;

/*Keypad*/

/* 
 * Initiates keypad with GPIOD pins 8-15.
 * 8-11 are in, 11-15 are out.
 */
int keypad_init(GPIO_TypeDef* gpiox) {
	//Keypad input pins (row) configured
	GPIO_InitTypeDef keypadin;
	keypadin.GPIO_Pin = GPIO_Pin_11|GPIO_Pin_10|GPIO_Pin_9|GPIO_Pin_8;
	keypadin.GPIO_Mode = GPIO_Mode_IN;
	keypadin.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOD,&keypadin);
    GPIO_SetBits(GPIOD,GPIO_Pin_11|GPIO_Pin_10|GPIO_Pin_9|GPIO_Pin_8);
	
	//Keypad output pins (columns) configured
	GPIO_InitTypeDef keypadout;
	keypadout.GPIO_Pin = GPIO_Pin_15|GPIO_Pin_14|GPIO_Pin_13|GPIO_Pin_12;
	keypadout.GPIO_Mode = GPIO_Mode_OUT;
	keypadout.GPIO_PuPd = GPIO_PuPd_UP;
    keypadin.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOD,&keypadout);
    GPIO_SetBits(GPIOD,GPIO_Pin_15|GPIO_Pin_14|GPIO_Pin_13|GPIO_Pin_12);
	return 1;
}

int keypad_buff_add(char *keypad_buffer[],int i)
{
	keypad_buffer[keypad_buffindex++] = i;
	return 1;
}

int keypad_buff_clr(char *keypad_buffer[]) {
	keypad_buffindex = 0;
	clr_str(keypad_buffer, KEYPADBUFF_SIZE);
	return 1;
}

/* 
 * Checks which column the pressed key resides.
 * Output: returns the value of the column
 * where the pressed key resides.
 */
int keypad_getcol(void)
{
    if(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_11))
        return 4;
    if(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_10))
        return 3;
    if(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_9))
        return 2;
    if(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_8))
        return 1;
    return 0;
}

/* 
 * Activates a row on the keypad.
 * if the user has pressed a key on
 * row 1, row 1 must be activated to
 * retrieve the value.
 * Input: The row to be activated.
 */
void keypad_row_actv(uint32_t row)
{
    switch(row) {
    case 1:
        GPIOD->ODR = GPIO_Pin_12;
        break;
    case 2:
        GPIOD->ODR = GPIO_Pin_13;
        break;
    case 3:
        GPIOD->ODR = GPIO_Pin_14;
        break;
    case 4:
        GPIOD->ODR = GPIO_Pin_15;
        break;
    default:
        GPIOD->ODR = 0;
    }
}
/* 
 * Identifies and returns a number dependent
 * on the value a user has pressed on a keypad.
 * Output: One of the elements in array key if there is an input,
 * returns value 0xFF if no value.
 */
char keypad_getnum(void) 
{
    char key[] = { 1, 2, 3, 0xA, 4, 5, 6, 0xB, 7, 8, 9, 0xC, 0xD, 0, 0xE, 0xF }; 
    int row, col;
    for(row = 1; row <= 4; row++) {
        keypad_row_actv(row);
        if(col = keypad_getcol()) {
            keypad_row_actv(0);
            return key[4 * (row - 1) + (col - 1)];
        }
    }
    keypad_row_actv(0);
    return 0xFF;
}

/* 
 * If keypad_getnum returns a valid key it's added to the keypad buffer.
 * kp_buff_ready is used to delay inputs to keypad to stop spammed inputs.
 * Keypad input is cleared by "C".
 * Output: 1 
 */
int keypad_append_input(char *keypad_buffer[]) {
    char c = keypad_getnum();
	if(c != 0xFF) {
        if(buffer_full(KEYPADBUFF_SIZE, keypad_buffindex)) {
			return 0;
		}
        if(kp_buff_ready < KEYPAD_DELAY) {
            return 1;
        }
		
        if(c == 0xC) {
			keypad_buff_clr(keypad_buffer);
            usart_print("\nPassword input cleared.\n");
		}
        else if(c>=10) {
            usart_print("Illegal character, please try again.\n");
        }
		else {
			keypad_buff_add(keypad_buffer,c);
            usart_print_num(c);
		}
		kp_buff_ready = 0;
	}
	else{
		if(kp_buff_ready <= KEYPAD_DELAY) kp_buff_ready++;
	}
	return 1;
}

/* 
 * Verifies if the input code is correct.
 * Input: Users keypad input in keypad_buffer.
 * Output: 1 if correct code, otherwise 0.
 */
int keypad_verify_code(char *keypad_buffer[]) {
    
    for(int i=0; i<KEYPADBUFF_SIZE; i++) {
        if(keypadcode[i] != keypad_buffer[i]) return 0;
    }

    return 1;

}




/*Usart*/

/* 
 * Clears the buffer.
 */
int usart_buff_clr(char *usart_buffer) {
	usart_buffindex = 0;
	clr_str(usart_buffer,USARTBUFF_SIZE);
	return 1;
}

/* 
 * Adds a character from USART to the buffer.
 * If input is '\b' (backspace) last input character is deleted.
 * Input: A character c and the usart_buffer.
 * Output: returns 1 if character c should be printed, otherwise 0.
 */
int usart_buff_add(char *usart_buffer,char c)
{
	if(buffer_full(USARTBUFF_SIZE,usart_buffindex)){
		usart_buffindex = 0;
	}
    if(c == '\b'){
        if(usart_buffindex){ 
            usart_buffindex--;
            usart_buffer[usart_buffindex] = '\0';
        }
        else return 0;
    }
	else if(c){
	usart_buffer[usart_buffindex++] = c;
	}
	return 1;
}

/* 
 * Output: returns USART flag RXNE.
 * USART_FLAG_RXNE: Receive data register not empty flag. 
 */
int usart_has_input(void)
{
    return USART_GetFlagStatus(USART1,USART_FLAG_RXNE);
}

/* 
 * Output: returns user input to USART.
 */
char usart_getchar(void)
{
    return USART_ReceiveData(USART1);
}

/* 
 * Prints input characters and adds them to buffer if available.
 * Output: 1 if operation was successful, 0 otherwise
 */
int usart_append_input(char *usart_buffer) { // recieves USART input if there is one
	if(usart_has_input()) {
		char c = usart_getchar();
		if(buffer_full(USARTBUFF_SIZE, usart_buffindex) || c == '\n') {
            usart_print("\n\n");
            return 0;
		}
		else {
            if(usart_buff_add(usart_buffer,c)){
            print_char(c);
            }
        }
	}
	return 1;
}

/* 
 * Prints a char, is mainly used with usart_print
 * or usart_printline
 */
void print_char( char c ) {
    /* write character to usart1 */
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET); //checks flag
    USART_SendData(USART1, c);
}

/* 
 * Output: Prints a string
 */
void usart_print(char *s)
{
    while(*s)
    {
        print_char(*s++);
    }
}

/* 
 * Output: Prints a string with a line
 */
void usart_printline(char *s)
{
    while(*s)
    {
        print_char(*s++);
    }
    print_char('\n');
}

/* 
 * Transforms a more than single digit number into a list
 * containing the numbers in order, to then be printed.
 * For example: usart_print_num(152) = usart_printline([1,5,2])
 * Input: Any integer number. 
 */
void usart_print_num(int num)
{
    if(num == 0) {
        usart_print("0");
    }
	char str[100];
    int i, rem, len = 0, n;
 
    n = num;
    while (n != 0)
    {
        len++;
        n /= 10;
    }
    for (i = 0; i < len; i++)
    {
        rem = num % 10;
        num = num / 10;
        str[len - (i + 1)] = rem + '0';
    }
    str[len] = '\0';
	usart_printline(str);
}




/*Usart command handler*/
/* 
 *
 */
 
  /* 
 * Prints all commands.
 */
void usart_printhelp()
{
    usart_print("USART commands:\n\n");
    for(int n=0; n<=MODULEFUNCS_LENGTH; ++n) {
        print_usart_command(n);
        print_char('\n');
    }
}

/* 
 * Prints a commands and it's description
 * Input: command, an int which stands for which 
 * command in the list CMD_STRING will be described.
 */
void print_usart_command(int command)
{
    usart_print(CMD_STRING[command]);
    switch (command){
        case 0:
        usart_print("    [module-ID]   \tArms a module.\n");
        break;
        case 1:
        usart_print(" [module-ID]\tDisarm a module.\n");
        break;
        case 2:
        usart_print(" [module-ID]\tPrints the status of a module.\n");
        break;
        case 3:
        usart_print(" [module-ID]\tAllows the user to configurate a module.\n");
        break;
        case 4:
        usart_print("\t\tPrints all connected modules.\n");
        break;
        case 5:
        usart_print("\t\t\tPrints all commands.\n");
        break;
    }
}

/* 
 * Prints error message if input command in not recognized.
 * Input: The usart buffer containing the input command.
 */
void print_hlp_msg(char *usart_buffer){
    usart_print("''");
    usart_print(usart_buffer);
    usart_print("'' is not recognized as a command.");
    usart_print(" Type ''help'' for a full list of executable commands.\n\n");
}

/* 
 * Sets the config flag to one and promts the user to modify config fields. 
 * Input: The id of the searched module
 * Output: 1 if operation was successful, 0 otherwise
 */
int set_cfg_flag(uint8_t moduleid,PMODULE modulelist)
{
    current_moduleid = moduleid;
    
    PMODULE m = module_find(moduleid,modulelist);
    
    if(m->t==MODULE_ALARM){
    usart_print("Insert the configuration: triggered(1) and distancecfg(2)");
    usart_print("seperated by commas (for example: ''-,15''. '-' ignores input):\n\n");
    }
    else if(m->t==MODULE_DOOR){
        if(m->cfgarr[1] == '-'){
            usart_print("Insert the amount of doors to be used (max 8):\n\n");
        }
        else {
            usart_print("Insert the configuration: door-armed(1), door-ID(2), global doortime(3) and local ");
            usart_print("doortime(4) seperated by commas (for example: ''0,1,-,15''. '-' ignores input):\n\n");
        }
    }
    else {
        usart_print("Error: No module with that ID connected, command aborted.\n\n");
        return 0;
    }
    cfg_flag = 1;
    return 1;
}

/* 
 * Sends a command to be handled by ceneral.
 * Input: The id of the module to be configured, and a number for which function to be called.
 * Output: always returns 1 as one of these functions has to be called.
 */
int send_command(uint8_t moduleid,PMODULE modulelist,int n)
{
    switch (n){
        case 0:
        module_set_armed(1,moduleid,modulelist);
        return 1;
        case 1:
        module_set_armed(0,moduleid,modulelist);
        return 1;
        case 2:
        module_status(moduleid,modulelist);
        return 1;
        case 3:
        set_cfg_flag(moduleid,modulelist);
        return 1;
        case 4:
        module_printlist(modulelist);
        return 1;
        case 5:
        usart_printhelp();
        return 1;
    }
}

/* 
 * Checks if the command given in module_cmd is exists and sends the function forward if that is the case. 
 * Input: The command, the id of the searched module
 * Output: 1 if operation was successful, 0 otherwise
 */
int usart_cmd_send(char module_cmd[], uint8_t moduleid,PMODULE modulelist)
{
    if(cfg_flag){
        module_set_cfg(module_cmd,current_moduleid,modulelist);
        cfg_flag = 0;
        return 1;
    }
    
    for(int n = 0;n<=MODULEFUNCS_LENGTH;n++){
		if(strcmp(module_cmd,CMD_STRING[n]) == 0){
            return send_command(moduleid,modulelist,n);
		}
    }
    return 0;
}

/* 
 * Splits the user input into a module command to be used in module_set_cfg. 
 * Input: Pointer to usart_buffer containing input from USART and a pointer to modulelist.
 * Output: Always 1, as cfg_flag = 1 means the previous command "config" was correctly inserted.
 */
int cfg_command_execute(char *usart_buffer,PMODULE modulelist)
{
    char module_cmd[COMMAND_MAXLENGTH];
    char input[COMMAND_MAXLENGTH];
    
    clr_str(module_cmd,COMMAND_MAXLENGTH);
    clr_str(input,COMMAND_MAXLENGTH);
    
    int k = 0;
    int module_cmd_index = 0;
    
    for(int i=0; i<USARTBUFF_SIZE; i++) {
            if(usart_buffer[i] == ',' || usart_buffer[i] == 0){
                substring(input, usart_buffer, k, i-k);
                module_cmd[module_cmd_index++] = string_to_num(input);
                if(usart_buffer[i] == 0) break;
                clr_str(input, COMMAND_MAXLENGTH);
                k=i+1;
        }
    }
    module_set_cfg(module_cmd,current_moduleid,modulelist);
    cfg_flag = 0;
    return 1;
}

/* 
 * Splits the user input into a module command and a module id.
 * Input: pointer to usart_buffer containing input from USART and a pointer to modulelist.
 * Output: 1 if the typed command was recognized, 0 otherwise
 */
int command_execute(char *usart_buffer,PMODULE modulelist)
{
    char module_cmd[COMMAND_MAXLENGTH];
    char module_id[COMMAND_MAXLENGTH];
    
    clr_str(module_cmd,COMMAND_MAXLENGTH);
    clr_str(module_id,COMMAND_MAXLENGTH);
    
    int k = 0;
    for(int i=0; i<USARTBUFF_SIZE; i++) {
        if(usart_buffer[i] == ' ' || usart_buffer[i] == 0 ) {
            if(module_cmd[0]){ //checks if module_cmd is empty
                substring(module_id, usart_buffer, k+1, i-k-1);
                module_id[0] = string_to_num(module_id);
                break; // module_cmd and module_id have their values, break.
            }
            if(i<=COMMAND_MAXLENGTH) strncpy(module_cmd, usart_buffer, i);
            k=i;
        }
    }
    return usart_cmd_send(module_cmd,(module_id[0]),modulelist);
}

/* 
 * Executes a command using a chain of helper functions. 
 * Input: The usart_buffer which contaions user input.
 * Output: 1 if the typed command was recognized, 0 otherwise
 */
int usart_cmd_execute(char *usart_buffer,PMODULE modulelist) 
{
    if(cfg_flag){
        return cfg_command_execute(usart_buffer,modulelist);
    }
    else{
        return command_execute(usart_buffer,modulelist);
    }
}
