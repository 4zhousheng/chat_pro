#include "server/gateway/gw_connection.h"

#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include "logic/logic_service.h"
#include "model/model.h"
#include "server/core/thpool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <asm-generic/errno-base.h>
#include <sys/epoll.h>

#define PORT 8080
#define MAX_EVENTS 10

void start_server(){
    int socket_fd,new_socket,nfds,epoll_fd;//nfds用来计数已经就绪态的描述符
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    // char buffer[BUFFER_SIZE] = {0};

    // username_to_fd_map* uf_map = NULL;//ufmap是为了通过username能够找到对应fd发送请求,现在他被淘汰了
    online_user_manager *uf_manager = init_online_user_manager();//ufmanager管理器是为了保障线程安全使用读写锁

    //这是离线消息管理器
    global_mail_manager *manager = malloc(sizeof(global_mail_manager));
    //初始化这个
    pthread_rwlock_init(&manager->lock,NULL);
    struct epoll_event ev,events[MAX_EVENTS];//引入epoll

    threadpool logic_pool = thpool_init(8);//准备一个线程池处理计算逻辑

    //new socket
    if((socket_fd = socket(AF_INET,SOCK_STREAM,0)) < 0){
        perror("SOCK FAILED");
        exit(EXIT_FAILURE);
    }
    int opt = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
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

    //create epoll example
    if((epoll_fd = epoll_create(1)) < 0){
        perror("EPOLL CREATE FAILED");
        exit(EXIT_FAILURE);
    }
    //register event listener to listen socket_fd
    ev.events = EPOLLIN;
    ev.data.fd = socket_fd;
    if(epoll_ctl(epoll_fd,EPOLL_CTL_ADD,socket_fd,&ev) < 0){
        perror("EPOLL CTL ADD: LISTEN SOCKET FD FAILED");
        exit(EXIT_FAILURE);
    }
    while(1){
        nfds = epoll_wait(epoll_fd,events,MAX_EVENTS,-1);
        if(nfds < 0) {
            if (errno == EINTR) {
                continue; // debugger 或信号中断都自动继续
            }
            perror("EPOLL WAIT FAILED");
            exit(EXIT_FAILURE);
        }
        for(int n = 0; n <nfds ; n++){
            if(events[n].data.fd == socket_fd){//创建新连接
                if((new_socket = accept(socket_fd,(struct sockaddr *)&address,(socklen_t*)&addrlen)) < 0){
                    perror("ACCEPT FAILED");
                    exit(EXIT_FAILURE);
                }
                int flags = fcntl(new_socket, F_GETFL, 0);
                fcntl(new_socket, F_SETFL, flags | O_NONBLOCK);

                ev.events = EPOLLIN | EPOLLET;//新socket fd为边缘触发
                ev.data.fd = new_socket;
                
                if(epoll_ctl(epoll_fd,EPOLL_CTL_ADD,new_socket,&ev) < 0){
                    perror("EPOLL CTL ADD : NEW SOCKET FAILURE");
                    exit(EXIT_FAILURE);
                }

            }else{//对已存在的连接发起请求
                task_context* context = (task_context*)malloc(sizeof(task_context));
                context->client_fd = events[n].data.fd;
                context->uf_manager = uf_manager;
                context->pool = &logic_pool;
                context->mail_manager = manager;
                int ret = thpool_add_work(logic_pool,handle_msg_task,context);
                if (ret < 0) {
                    perror("thpool_add_work failed");
                    exit(EXIT_FAILURE);
                }
                // char buffer[BUFFER_SIZE] = {0};
                // recv(events[n].data.fd,buffer,BUFFER_SIZE,0);
                // message msg = deserialization(buffer);
                // switch(msg.header.type){
                //     case MSG_TYPE_LOGIN_REQUEST:
                //         // add_mapping(msg.origin_user,events[n].data.fd,&uf_map);
                //         manager_add_user(&uf_manager,msg.origin_user,events[n].data.fd);//使用加了读写锁的user-socketfd的hash管理在线状态
                //         printf("用户%s登录成功,socket fd:%d\n",msg.origin_user,events[n].data.fd);
                //         fflush(stdout);
                //         break;
                //     case MSG_TYPE_CHAT_MESSAGE:
                //         // struct username_to_fd_map* entry = find_mapping(msg.target_user,uf_map);老版本采用最原始的无线程安全的hash结构
                //         // old_socket = entry->socket_fd;
                //         old_socket = manager_get_user_socket(&uf_manager,msg.target_user);//old socket是已存在的socket的意思，差点忘了
                //         send(old_socket,buffer,BUFFER_SIZE,0);
                //         printf("用户%s向%s发送消息,接收用户fd: %d\n",msg.origin_user,msg.target_user,old_socket);
                //         fflush(stdout);
                //         break;
                // }
            }
        }
    }
    close(socket_fd);

}