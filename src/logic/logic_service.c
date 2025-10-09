#include "logic/logic_service.h"
#include "model/model.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>


#define MAX_MSG_SIZE 1024

//outtime
void print_received_message(char *msg){
    printf("client: %s \n",msg);
}
//outtime
void send_message(char* msg,int msg_size){
    printf("server: ");
    fgets(msg,msg_size,stdin);
    msg[strcspn(msg, "\n")] = 0;
}


void serialization(struct message msg,char* buffer){
    struct msg_header head = msg.header;
    uint32_t type_net,length_net;
    switch(head.type){
        case MSG_TYPE_LOGIN_REQUEST: //发起登录请求
            type_net = htonl(head.type);
            memcpy(buffer,&(type_net),sizeof(uint32_t));
            buffer += sizeof(uint32_t);
            head.length = sizeof(msg.origin_user);//length放在这里计算，省去其他地方的麻烦
            length_net = htonl(head.length);
            memcpy(buffer,&(length_net),sizeof(uint32_t));
            buffer += sizeof(uint32_t);
            memcpy(buffer,&(msg.origin_user),MAX_NAME_LEN);
            break;
        
        case MSG_TYPE_CHAT_MESSAGE:
        //header
            //type
            type_net = htonl(head.type);
            memcpy(buffer,&(type_net),sizeof(uint32_t));
            buffer += sizeof(uint32_t);
            //length
            head.length = sizeof(msg.origin_user) + sizeof(msg.target_user) + sizeof(msg.msg_context);
            length_net = htonl(head.length);
            memcpy(buffer,&length_net,sizeof(uint32_t));
            buffer += sizeof(uint32_t);
        //body    
            //origin_user
            memcpy(buffer,&(msg.origin_user),MAX_NAME_LEN);
            buffer +=  MAX_NAME_LEN;
            //target_user
            memcpy(buffer,&(msg.target_user),MAX_NAME_LEN);
            buffer += MAX_NAME_LEN;
            //message 
            memcpy(buffer,&(msg.msg_context),MAX_CONTEXT_LEN);
            break;
    }

}

struct message deserialization(char* buffer){
    struct message msg;
    uint32_t type_net,length_net;
    //get header.type
    memcpy(&type_net,buffer,sizeof(uint32_t));
    buffer += sizeof(uint32_t);
    msg.header.type = ntohl(type_net);
    switch(msg.header.type){
        case MSG_TYPE_LOGIN_REQUEST:
            memcpy(&length_net,buffer,sizeof(uint32_t));
            buffer+=sizeof(uint32_t);
            msg.header.length = ntohl(length_net);
            memcpy(&msg.origin_user,buffer,msg.header.length);
            break;
        case MSG_TYPE_CHAT_MESSAGE:
            memcpy(&length_net,buffer,sizeof(uint32_t));
            buffer+=sizeof(uint32_t);
            msg.header.length = ntohl(length_net);
            memcpy(msg.origin_user,buffer,MAX_NAME_LEN);
            buffer+=MAX_NAME_LEN;
            memcpy(msg.target_user,buffer,MAX_NAME_LEN);
            buffer+=MAX_NAME_LEN;
            memcpy(msg.msg_context,buffer,MAX_CONTEXT_LEN);
            msg.origin_user[MAX_NAME_LEN-1] = '\0';
            msg.target_user[MAX_NAME_LEN-1] = '\0';
            msg.msg_context[MAX_CONTEXT_LEN-1] = '\0';
            break;
    }
    return msg;
}

void* client_receive_messages(void* socket_desc){
    int sock = *(int *)socket_desc;
    char buffer[BUFFER_SIZE];
    ssize_t read_size;
    while((read_size = recv(sock,buffer,BUFFER_SIZE,0))>0){
        buffer[read_size] = '\0';
        struct message msg;
        msg = deserialization(buffer);
        if(msg.header.type == MSG_TYPE_CHAT_MESSAGE){
                printf("\n%s: %s\n",msg.origin_user,msg.msg_context);
                printf("%s: ",msg.target_user);
                fflush(stdout);
                memset(buffer,0,BUFFER_SIZE);//clear buffer
        }
    }
    if(read_size == 0){
        printf("\nServer Disconnected\n");
    }else{
        perror("recv failed");
    }
}


//HashMap 
void add_mapping(char *username,int socket_fd,struct username_to_fd_map **head_ptr){
    struct username_to_fd_map *entry = (struct username_to_fd_map *)malloc(sizeof(struct username_to_fd_map));
    strncpy(entry->username, username, sizeof(entry->username) - 1);
    entry->username[sizeof(entry->username) - 1] = '\0'; 
    entry->socket_fd = socket_fd;
    HASH_ADD_STR(*head_ptr,username,entry);
}

struct username_to_fd_map* find_mapping(char* username,struct username_to_fd_map* head){
    struct username_to_fd_map* entry;
    HASH_FIND_STR(head,username,entry);
    return entry;
}

void delete_mapping(struct username_to_fd_map* entry,struct username_to_fd_map **head_ptr){
    if(entry == NULL) return;
    HASH_DEL(*head_ptr,entry);
    free(entry);
}

