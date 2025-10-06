#include "server/logic/logic_service.h"
#include <stdio.h>
#include <string.h>

#define MAX_MSG_SIZE 1024

void print_received_message(char *msg){
    printf("client: %s \n",msg);
}

void send_message(char* msg,int msg_size){
    printf("server: ");
    fgets(msg,msg_size,stdin);
    msg[strcspn(msg, "\n")] = 0;
    return msg;
}