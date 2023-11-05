/*
 * 	startup.c
 *
 */
#include "inc/general.h"
#include "can/can.h"

__attribute__((naked)) __attribute__((section (".start_section")) )
void startup ( void )
{
__asm__ volatile(" LDR R0,=0x2001C000\n");		/* set stack */
__asm__ volatile(" MOV SP,R0\n");
__asm__ volatile(" BL main\n");					/* call main */
__asm__ volatile(".L1: B .L1\n");				/* never return */
}

//This modules type
MODULETYPE moduletype = MODULE_CENTRAL;

int localalarm = 0;
volatile int setupcounter = SETUP_TIME;
volatile long counter = 0;
static volatile int time_current;

char usart_buffer[USARTBUFF_SIZE];
char keypad_buffer[KEYPADBUFF_SIZE];
MODULE modulelist[MODULELIST_LENGTH];
//Current index in modulelist, is used during startup
int moduleindex = 0;




/* 
 * Decrements startupcounter until it is zero, then decrements the regular counter 
 */
void SysTick_Handler(void)
{

  if(setupcounter > 0) {
	setupcounter--;
  }
  //Start incremental ping and checks for alarm if setup done
  else if(counter > 0) {
    counter--;
  }
  
}




/* 
 * The main loop of the alarmsystem. It regularly checks if new input has been recieved,
 * adds the input to buffers, handles USART command execution, checks if any modules become disconnected, pings all modules regularly 
 */
void main(void)
{
    
	//Makes sure modulelist starts as empty list, might be redundant
    memset(modulelist, 0 ,sizeof(modulelist));
    
	//Starts clock for GPIOD
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE);

	//Init can, systick, keypad and USART
	keypad_init(GPIOD);
	can1_init(msg_reciever);
	systick_time_init(&time_current);
    
    //Creates newline for program
    print_char('\n'); 
    
    can_sendping_broadcast();

	while(1) {
        
        //Initial ping to all modules
        if(setupcounter != 0) {
           if(setupcounter == SETUP_TIME/2) can_sendping_broadcast();
        }
        else {
            //Receive input from keypad, but only if local alarm has been triggered
            //Keypad_append_input returns 0 if buffer is full -> input done
            if(localalarm && !keypad_append_input(keypad_buffer)) {

                if(keypad_verify_code(keypad_buffer)) {
                    localalarm = 0;
                    usart_print("\nALARM DISARMED\n");
                    can_sendalarm_broadcast();
                }
                else{
                    usart_print("\nERROR: INCORRECT CODE\n");
                }
                keypad_buff_clr(keypad_buffer);
            }
            
            //Usart_append_input returns 0 if command has been finished
            if(!usart_append_input(usart_buffer)){
                
                //If command didn't execute, print help message
                if(!usart_cmd_execute(usart_buffer,&modulelist)){
                    print_hlp_msg(usart_buffer);
                }
                usart_buff_clr(usart_buffer);
            }
            
            //In a time interval defined by systick, all modules must repeatedly report to central
            if(setupcounter <= 0 && counter <= 0) {
                
                //If any module failed to report, send alarm
                if(module_pingfaliure(&modulelist)) {
                    can_sendalarm_broadcast();  
                } 
                else {
                    
                    //Reset all modules' connected status for next ping
                    for(int  i=0; i< MODULELIST_LENGTH; i++ ) {
                        modulelist[i].connected = 0;
                    }
                    
                    can_sendping_broadcast();
                }
                counter = PING_TIME;
            }
        }

	}
}