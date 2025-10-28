#ifndef MODEL_H
#define MODEL_H
#define MAX_NAME_LEN 20
#define MAX_CONTEXT_LEN 1024
#include "server/core/thpool.h"
#include "server/storage/uthash.h"
//这个MessageType用于构建一个header
typedef enum{
    MSG_TYPE_LOGIN_REQUEST = 1,
    MSG_TYPE_CHAT_MESSAGE = 2,
}msg_type;

//header
struct msg_header{
    uint32_t type;
    uint32_t length;
};

typedef struct message{
    struct msg_header header;
    char origin_user[MAX_NAME_LEN];
    char target_user[MAX_NAME_LEN];
    char msg_context[MAX_CONTEXT_LEN];
}message;

//用户名和socketfd的map映射方便找到聊天对象
typedef struct username_to_fd_map{
    char username[MAX_NAME_LEN];//key
    int socket_fd;//value
    UT_hash_handle hh; //句柄
}username_to_fd_map;

typedef struct online_user_manager {
    username_to_fd_map* user_map;
    pthread_rwlock_t lock;
}online_user_manager;

//都退后，接下来由无锁队列接管战场
//接收用户发送的消息的无锁队列
typedef struct msg_node {
    message *msg;
    struct msg_node *next;
}msg_node;

typedef struct msg_queue {
    msg_node *head;
    msg_node *tail;
}msg_queue;

/**
 * 离线消息队列
 */
typedef struct offline_msg_node {
    message *msg;
    struct offline_msg_node *next;
}offline_msg_node;

typedef struct offline_msg_queue {
    offline_msg_node *head;
    offline_msg_node *tail;
}offline_msg_queue;

/**
 * mailbox是一个hash表
 * 用户名为key
 * 该用户的离线消息链表为value
 */
typedef struct mail_box {
    char *username;
    offline_msg_queue *msg_queue;
    UT_hash_handle hh;
    pthread_mutex_t lock;
}mail_box;

typedef struct global_mail_manager {
    mail_box *mail_hash;
    pthread_rwlock_t lock;
}global_mail_manager;

//这是msg_task传递的参数
typedef struct task_context {
    int client_fd;
    online_user_manager *uf_manager;
    global_mail_manager *mail_manager;
    threadpool *pool;
}task_context;


#endif
