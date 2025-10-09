#ifndef LOGIC_SERVICE_H
#define LOGIC_SERVICE_H
#include "model/model.h"
#define BUFFER_SIZE 1024

void print_received_message(char *message);
void send_message(char* buffer,int buffer_size);
void serialization(struct message msg,char* buffer);
struct message deserialization(char* buffer);
void* client_receive_messages(void* socket_desc);
//HashMap 
void add_mapping(char *username,int socket_fd,struct username_to_fd_map **head_ptr);

struct username_to_fd_map* find_mapping(char* username,struct username_to_fd_map *head);

void delete_mapping(struct username_to_fd_map* entry,struct username_to_fd_map **head_ptr);


#endif
