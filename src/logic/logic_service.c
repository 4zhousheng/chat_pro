#include "logic/logic_service.h"

#include <pthread.h>

#include "model/model.h"
#include "server/core/thpool.h"
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
// void add_mapping(char *username,int socket_fd,struct username_to_fd_map **head_ptr){
//     struct username_to_fd_map *entry = (struct username_to_fd_map *)malloc(sizeof(struct username_to_fd_map));
//     strncpy(entry->username, username, sizeof(entry->username) - 1);
//     entry->username[sizeof(entry->username) - 1] = '\0';
//     entry->socket_fd = socket_fd;
//     HASH_ADD_STR(*head_ptr,username,entry);
// }
//
// struct username_to_fd_map* find_mapping(char* username,struct username_to_fd_map* head){
//     struct username_to_fd_map* entry;
//     HASH_FIND_STR(head,username,entry);
//     return entry;
// }
//
// void delete_mapping(struct username_to_fd_map* entry,struct username_to_fd_map **head_ptr){
//     if(entry == NULL) return;
//     HASH_DEL(*head_ptr,entry);
//     free(entry);
// }
//
// void init_online_user_manager(online_user_manager* manager) {
//     manager->user_map = NULL;
//     pthread_rwlock_init(&manager->lock,NULL);
// }

/**
 *
 * @param manager
 * @param id id是
 * @param username
 * @param fd
 */
void manager_add_user(online_user_manager *manager,const char *username, int fd) {
    pthread_rwlock_wrlock(&manager->lock);
    username_to_fd_map *uf_entry;
    HASH_FIND_STR(manager->user_map,username,uf_entry);
    //用户没有连接
    if(uf_entry == NULL) {
        uf_entry = (struct username_to_fd_map *)malloc(sizeof(struct username_to_fd_map));
        strncpy(uf_entry->username, username, sizeof(uf_entry->username) - 1);
        uf_entry->username[sizeof(uf_entry->username) - 1] = '\0';
        uf_entry->socket_fd = fd;
        HASH_ADD_STR(manager->user_map,username,uf_entry);
    }
    pthread_rwlock_unlock(&manager->lock);
}

void manager_remove_user(online_user_manager *manager, const char *username) {
    pthread_rwlock_wrlock(&manager->lock);
    username_to_fd_map *uf_entry;
    HASH_FIND_STR(manager->user_map,username,uf_entry);
    if(uf_entry != NULL) {
        HASH_DEL(manager->user_map,uf_entry);
        free(uf_entry);
    }
    pthread_rwlock_unlock(&manager->lock);
}

/**
 * 获取用户socket，如果没有找到则返回-1
 * @param manager
 * @param username
 * @return
 */
int manager_get_user_socket(online_user_manager *manager, const char *username) {
    pthread_rwlock_rdlock(&manager->lock);
    username_to_fd_map *uf_entry;
    HASH_FIND_STR(manager->user_map,username,uf_entry);
    int fd = -1;
    if (uf_entry!=NULL) {
        fd =  uf_entry->socket_fd;
    }
    pthread_rwlock_unlock(&manager->lock);
    return fd;
}

void manager_destroy(online_user_manager *manager) {
    pthread_rwlock_unlock(&manager->lock);
}

/**
 * 初始化消息队列
 * @param q
 */
void msg_queue_init(msg_queue *q) {
    msg_node* new_node = (msg_node *)malloc(sizeof(msg_node));
    new_node->next = NULL;
    q->head = q->tail = new_node;
}

/**
 * 入队逻辑
 * @param q
 * @param msg
 */
void msg_enqueue(msg_queue *q, message *msg) {
    msg_node* new_node = (msg_node *)malloc(sizeof(msg_node));
    new_node->msg = msg;
    new_node->next = NULL;

    while (1) {
        msg_node* last = q->tail;
        msg_node* next = last->next;
        if (last == q->tail) {
            if (next == NULL) {
                if (__sync_bool_compare_and_swap(&last->next, NULL, new_node)) {
                    __sync_bool_compare_and_swap(&q->tail,last,new_node);
                    return;
                }
            }
        }else {
            __sync_bool_compare_and_swap(&q->tail,last,next);
        }
    }
}

/**
 * 出队逻辑
 * @param q
 * @return
 */
message* msg_dequeue(msg_queue *q) {
    while (1) {
        msg_node *first = q->head;
        msg_node *last = q->tail;
        msg_node *next = first->next;
        if (first == q->head) {
            if (first == last) {
                if (next == NULL) {
                    return NULL;
                }
                __sync_bool_compare_and_swap(&q->tail,last,next);
            }else {
                message *msg = next->msg;
                if (__sync_bool_compare_and_swap(&q->head,first,next)) {
                    free(first);
                    return msg;
                }
            }
        }
    }
}

/**
 * 添加一个消息进入离线消息列表
 * @param manager
 * @param msg
 */
void offline_msg_add(global_mail_manager *manager,message *msg) {
    pthread_rwlock_wrlock(&manager->lock);
    mail_box *finded_mail,*new_mail;
    char *username = msg->target_user;
    HASH_FIND_STR(manager->mail_hash,username,finded_mail);
    //用户已存在离线消息
    if (finded_mail != NULL) {
        pthread_mutex_lock(&finded_mail->lock);
        offline_msg_node *new_msg_node = (offline_msg_node *)malloc(sizeof(offline_msg_node));
        new_msg_node->msg = msg;
        new_msg_node->next = NULL;
        finded_mail->msg_queue->tail->next = new_msg_node;
        finded_mail->msg_queue->tail = new_msg_node;
        pthread_mutex_unlock(&finded_mail->lock);
    }else {
        //用户还没有离线消息
        new_mail = (mail_box *)malloc(sizeof(mail_box));
        //用户名
        new_mail->username = strdup(username);
        //初始化离线消息队列
        offline_msg_queue *offline_msgs = malloc(sizeof(offline_msg_queue));
        offline_msgs->head = NULL;
        offline_msgs->tail = NULL;
        new_mail->msg_queue = offline_msgs;

        //消息队列的第一个节点
        offline_msg_node* init_msg_node = (offline_msg_node *)malloc(sizeof(offline_msg_node));
        init_msg_node->msg = msg;
        init_msg_node->next = NULL;

        //更新节点到队列上
        new_mail->msg_queue->tail = new_mail->msg_queue->head = init_msg_node;

        //初始化锁
        pthread_mutex_init(&new_mail->lock,NULL);
        HASH_ADD_STR(manager->mail_hash,username,new_mail);
    }
    pthread_rwlock_unlock(&manager->lock);
}

/**
 * 此函数用于获取某个用户的离线消息队列
 * @param manager
 * @param username 用户名
 * @return 返回值是离线消息队列
 */
offline_msg_queue* offline_msg_take_off(global_mail_manager *manager,const char* username) {
    //持有锁
    pthread_rwlock_rdlock(&manager->lock);
    mail_box *finded_mail;
    offline_msg_queue *msg_queue = NULL;
    HASH_FIND_STR(manager->mail_hash,username,finded_mail);
    if (finded_mail != NULL) {
        pthread_mutex_lock(&finded_mail->lock);
        msg_queue = finded_mail->msg_queue;
        finded_mail->msg_queue = malloc(sizeof(offline_msg_queue));
        finded_mail->msg_queue->head = NULL;
        finded_mail->msg_queue->tail = NULL;
        printf("发送用户%s的离线消息",username);
        pthread_mutex_unlock(&finded_mail->lock);
    }
    pthread_rwlock_unlock(&manager->lock);
    return msg_queue;
}

void handle_offline_msg_save_task(void *arg,message *msg) {
    task_context* context = (task_context *)arg;
    global_mail_manager *manager =  context->mail_manager;
    offline_msg_add(manager,msg);
    printf("存储用户%s的离线消息",msg->target_user);
    fflush(stdout);
}

void handle_offline_msg_send_task(void *arg,char *username) {
    task_context *context = (task_context *)arg;
    global_mail_manager *manager = context->mail_manager;
    offline_msg_queue* queue =  offline_msg_take_off(manager,username);
    //如果该用户的离线列表不为空,则发送离线消息
    if (queue != NULL) {
        offline_msg_node *msg_node = queue->head;
        while (msg_node != NULL) {
            message *msg = msg_node->msg;
            char buffer[BUFFER_SIZE];
            serialization(*msg,buffer);
            send(context->client_fd,buffer,BUFFER_SIZE,0);
            printf("正在发送用户%s向%s发送的离线消息,接收用户fd: %d\n",msg->origin_user,msg->target_user,context->client_fd);
            msg_node = msg_node->next;
        }
    }
}

void handle_online_msg_task(int socket_fd,char *msg_buffer) {
    send(socket_fd,msg_buffer,BUFFER_SIZE,0);
    message msg = deserialization(msg_buffer);
    printf("用户%s向%s发送消息,接收用户fd: %d\n",msg.origin_user,msg.target_user,socket_fd);
}
/**
 * 这是处理多线程计算任务的函数
 * 现在还没有加上用户登录时发送离线消息和用户发送消息时push进消息队列判断在线后直接发送消息的功能
 * @param arg
 */
void handle_msg_task(void *arg) {
    task_context* context = (task_context *)arg;
    int fd = context->client_fd;
    char buffer[BUFFER_SIZE] = {0};
    recv(fd,buffer,BUFFER_SIZE,0);
    message msg = deserialization(buffer);
    printf("线程 编号%u 正在处理客户端任务，客户端fd： %d...\n", (unsigned int)pthread_self(), fd);
    switch(msg.header.type){
        case MSG_TYPE_LOGIN_REQUEST:
            manager_add_user(context->uf_manager,msg.origin_user,fd);//使用加了读写锁的user-socketfd的hash管理在线状态
            printf("用户%s登录成功,socket fd:%d\n",msg.origin_user,fd);
            fflush(stdout);
            handle_offline_msg_send_task(context,msg.origin_user);
            break;
        case MSG_TYPE_CHAT_MESSAGE:
            online_user_manager *online_manager = context->uf_manager;
            //判断消息目标是否在线，如果在线
            int online_user_fd = manager_get_user_socket(online_manager,msg.target_user);
            if (online_user_fd < 0) {
                handle_offline_msg_save_task(arg,&msg);
            }else {
                handle_online_msg_task(online_user_fd,buffer);
            }
            fflush(stdout);
            break;
    }
}

online_user_manager *init_online_user_manager() {
    online_user_manager *manager = malloc(sizeof(online_user_manager));
    manager->user_map=NULL;
    pthread_rwlock_init(&manager->lock,NULL);
    return manager;
}



