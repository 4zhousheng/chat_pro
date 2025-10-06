#include "client/cli_connection.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

void start_connect(){
    int sock = 0;
    struct sockaddr_in serv_addr;
    char cli_msg[BUFFER_SIZE] = {0};
    char buffer[BUFFER_SIZE] = {0};

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
    //connect
    if(connect(sock,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0){
        printf("CONNECTION FAILED\n");
    }
    

    while(1){
        memset(buffer,0,BUFFER_SIZE);
        memset(cli_msg,0,BUFFER_SIZE);
        //client output
        printf("client: ");
        fgets(cli_msg,sizeof(cli_msg),stdin);
        cli_msg[strcspn(cli_msg,"\n")] = 0;
        if(strcmp(cli_msg,"exit") == 0){
            close(sock);
            break;
        }
        //send & recv
        send(sock,cli_msg,strlen(cli_msg),0);
        ssize_t valread = recv(sock,buffer,BUFFER_SIZE,0);
        if(valread > 0){
            printf("server: %s \n",buffer);
        }
    }
    close(sock);
    printf("disconnected……");
}