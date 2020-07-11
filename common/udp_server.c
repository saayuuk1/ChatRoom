/*************************************************************************
	> File Name: udp_server.c
	> Author: suyelu 
	> Mail: suyelu@126.com
	> Created Time: Thu 09 Jul 2020 11:15:39 AM CST
 ************************************************************************/

#include "head.h"

extern struct User *rteam, *bteam;
extern int repollfd, bepollfd;

int socket_create_udp(int port) {
    int server_listen;
    if ((server_listen = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        return -1;
    }
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = INADDR_ANY;

    int opt = 1;
    setsockopt(server_listen, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    make_non_block(server_listen);
    
    if (bind(server_listen, (struct sockaddr *)&server, sizeof(server)) < 0) {
        return -1;
    }
    return server_listen;
}

void send_all(struct ChatMsg *msg) {
    for (int i = 0; i < MAX; i++) {
        if (bteam[i].online == 1) {
            send(bteam[i].fd, (void *)msg, sizeof(struct ChatMsg), 0);
        }
        if (rteam[i].online == 1) {
            send(rteam[i].fd, (void *)msg, sizeof(struct ChatMsg), 0);
        }
    }
}

int send_to(char *to, struct ChatMsg *msg, int fd) {
    int flag = 0;
    for (int i = 0; i < MAX; i++) {
        if (rteam[i].online && (!strcmp(to, rteam[i].name))) {
            send(rteam[i].fd, msg, sizeof(struct ChatMsg), 0);
            flag = 1;
            break;
        }
        if (bteam[i].online && (!strcmp(to, bteam[i].name))) {
            send(bteam[i].fd, msg, sizeof(struct ChatMsg), 0);
            flag = 1;
            break;
        }
    }
    if (!flag) {
        memset(msg->msg, 0, sizeof(msg->msg));
        sprintf(msg->msg, "用户 %s 不在线，或用户名错误！", to);
        msg->type = CHAT_SYS;
        send(fd, msg, sizeof(struct ChatMsg), 0);
        return 0;
    }
    return 1;
}
