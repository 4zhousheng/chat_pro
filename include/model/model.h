#ifndef MODEL_H
#define MODEL_H
#define MAX_NAME_LEN 20
#define MAX_CONTEXT_LEN 1024
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

struct message{
    struct msg_header header;
    char origin_user[MAX_NAME_LEN];
    char target_user[MAX_NAME_LEN];
    char msg_context[MAX_CONTEXT_LEN];
};

//用户名和socketfd的map映射方便找到聊天对象
struct username_to_fd_map{
    char username[MAX_NAME_LEN];//key
    int socket_fd;//value
    UT_hash_handle hh; //句柄
};




#endif