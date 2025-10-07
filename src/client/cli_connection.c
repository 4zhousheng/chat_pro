#include "client/cli_connection.h"
#include "model/model.h"
#include "logic/logic_service.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

void start_connect(){
    int sock = 0;
    struct sockaddr_in serv_addr;
    char cli_msg[BUFFER_SIZE] = {0};
    char buffer[BUFFER_SIZE] = {0},buffer_to_send[BUFFER_SIZE] = {0};
    char user[20] = {0},target_user[20] = {0};//username
    pthread_t recv_thread;
    //new sock
    if((sock = socket(AF_INET,SOCK_STREAM,0)) < 0){
        perror("SOCK FAILED");
        exit(EXIT_FAILURE);
    }
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);
    if(inet_pton(AF_INET,SERVER_IP,&serv_addr.sin_addr)<=0){
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
    //input username
    printf("enter your username:  ");
    fgets(user,MAX_NAME_LEN,stdin);
    user[strcspn(user,"\n")] = 0;

    //connect
    printf("connecting to server....\n");
    if(connect(sock,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0){
        printf("CONNECTION FAILED\n");
    }
    
   
    //start recv thread
    if(pthread_create(&recv_thread,NULL,client_receive_messages,(void*)&sock) < 0){
        perror("receive thread create failed");
        exit(EXIT_FAILURE);
    }

    //authentication
    struct message login_msg;
    login_msg.header.type = MSG_TYPE_LOGIN_REQUEST;
    strncpy(login_msg.origin_user, user, sizeof(login_msg.origin_user) - 1);
    login_msg.origin_user[sizeof(login_msg.origin_user) - 1] = '\0'; // 保证安全    serialization(login_msg,buffer_to_send);
    send(sock,buffer_to_send,BUFFER_SIZE,0);
     
    //choose target user to send message
    printf("enter your friend's username to chat: ");
    fgets(target_user,MAX_NAME_LEN,stdin);
    target_user[strcspn(target_user,"\n")] = 0;
    while(fgets(cli_msg,sizeof(cli_msg),stdin) != NULL){
        memset(cli_msg,0,BUFFER_SIZE);
        memset(buffer_to_send,0,BUFFER_SIZE);//clear send message buffer
        //client input
        printf("%s :",user);
        //input message context
        cli_msg[strcspn(cli_msg,"\n")] = 0;
        if(strcmp(cli_msg,"exit") == 0){
            close(sock);
            break;
        }
        //build message struct
        struct message msg_to_send;
        strncpy(msg_to_send.origin_user, user, sizeof(msg_to_send.origin_user) - 1);
        msg_to_send.origin_user[sizeof(msg_to_send.origin_user) - 1] = '\0';
    
        strncpy(msg_to_send.target_user, target_user, sizeof(msg_to_send.target_user) - 1);
        msg_to_send.target_user[sizeof(msg_to_send.target_user) - 1] = '\0';

        strncpy(msg_to_send.msg_context, cli_msg, sizeof(msg_to_send.msg_context) - 1);
        msg_to_send.msg_context[sizeof(msg_to_send.msg_context) - 1] = '\0';
        serialization(msg_to_send,buffer_to_send);
        //send
        send(sock,buffer_to_send,BUFFER_SIZE,0);
    }
    close(sock);
    printf("disconnected……");
}