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
// void add_mapping(char *username,int socket_fd,username_to_fd_map **head_ptr);
//
// username_to_fd_map* find_mapping(char* username,username_to_fd_map *head);
//
// void delete_mapping(username_to_fd_map* entry,username_to_fd_map **head_ptr);


online_user_manager *init_online_user_manager();

void manager_add_user(online_user_manager *manager, const char *username, int fd);

void manager_remove_user(online_user_manager *manager, const char *username);

int manager_get_user_socket(online_user_manager *manager, const char *username);

void manager_destroy(online_user_manager *manager);

void msg_queue_init(msg_queue *q);

void msg_enqueue(msg_queue *q, message *msg);

message *msg_dequeue(msg_queue *q);

void handle_msg_task(void *arg);
#endif
