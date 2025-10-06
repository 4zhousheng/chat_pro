#include "server/gateway/gw_connection.h"
#include "server/logic/logic_service.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void start_server(){
    int socket_fd,new_socket;
    struct sockaddr_in address;
    // int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    char s_msg[BUFFER_SIZE] = {0};
    
    //new socket
    if((socket_fd = socket(AF_INET,SOCK_STREAM,0)) < 0){
        perror("SOCK FAILED");
        exit(EXIT_FAILURE);
    }

    //bind
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if(bind(socket_fd,(struct sockaddr *)&address,addrlen) < 0){
        perror("BIND FAILED");
        exit(EXIT_FAILURE);
    }
    //listen
    if(listen(socket_fd,3) < 0){
        perror("LISTEN FAILED");
        exit(EXIT_FAILURE);
    }    
    //开启循环持续接收和发送消息
    while(1){
            //accept
        if((new_socket = accept(socket_fd,(struct sockaddr *)&address,(socklen_t*)&addrlen)) < 0){
            perror("ACCEPT FAILED");
            exit(EXIT_FAILURE);
        }
        while(1){
            //清空缓冲区
            memset(buffer, 0, BUFFER_SIZE);
            //receive
            ssize_t valread = recv(new_socket,buffer,BUFFER_SIZE-1,0);
            if(valread < 0){
                perror("RECV FAILED");
                exit(EXIT_FAILURE);
            }else{
                buffer[valread] = '\0';
                print_received_message(buffer);        
                //send
                send_message(s_msg,sizeof(s_msg));
                if(strcmp(s_msg,"quit") == 0){
                    close(new_socket);
                    break;
                }
                send(new_socket,s_msg,strlen(s_msg),0);
            }
        }
        close(new_socket);
    }
    close(socket_fd);

}