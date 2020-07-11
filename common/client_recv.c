/*************************************************************************
	> File Name: client_recv.c
	> Author: tangxiaolong 
	> Mail: 2284841184@qq.com
	> Created Time: Fri 10 Jul 2020 03:01:18 PM CST
 ************************************************************************/

#include "head.h"

extern int sockfd;
extern WINDOW *message_win, *message_sub,  *info_win, *info_sub, *input_win, *input_sub;

void *do_recv(void *arg) {
    while (1) {
        while(1){
            struct ChatMsg msg;
            bzero(&msg, sizeof(msg));
            recv(sockfd, (void*)&msg, sizeof(struct ChatMsg), 0);
            show_message(message_sub, &msg, 0);
                
        }
    }
}
