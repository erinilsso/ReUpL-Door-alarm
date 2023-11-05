/*
 * 	test.c
 *
 */

#pragma once
#include "../inc/general.h"
#include "can.h"

/* Test function for modulelist, fills the list with modules using simulated pings, id:s starting at 13
 * Checks if all id:s match after addition
 */
int _test_modulelist_add() {
    int working = 1;
    
    CANMsg c;
    uint8_t nodeid = 13;
    
    for(int i=0; i<MODULELIST_LENGTH; i++) {
        can_createmsg_ping(&c,nodeid,MODULE_ALARM);
        msg_interpret(&c);
        if(modulelist[i].id != nodeid) working = 0;
        nodeid++;
    }

    return working;
}

/* IO Tests */

int _test_string_to_num() {
    char tst1[] = {0};
    char tst2[] = {2,9};
    char tst3[] = {1,4,2};
    if (string_to_num(tst1) != 0)       return 0;
    if (string_to_num(tst2) != 142)   return 0;
if (string_to_num(tst3) != 142) return 0;
    return 1;
}

int _test_substring() {
    int str_size = 5;
    char dst[str_size];
    char src[] = {1,2,3,4,5};
    int startindex = 0;
    int n = 3;
    
    substring(dst,src,startindex,n);
    
    for(int i=startindex; i<n; ++i) {
        if(dst[i] != src[i]){
            return 0;
        }
    }
    return 1;
}

int _test_print_as_string() {
    usart_print_num(25);
    usart_printline("= 25"); // should print "25\n = 25"
    
    usart_print_num(0);
    usart_printline("= 0");
    
    usart_print_num(123);
    usart_printline("= 123");
    return 1;
}
