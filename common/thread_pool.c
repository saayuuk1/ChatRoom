/*************************************************************************
	> File Name: thread_pool.c
	> Author: suyelu 
	> Mail: suyelu@126.com
	> Created Time: Thu 09 Jul 2020 02:50:28 PM CST
 ************************************************************************/

#include "head.h"
struct User *rteam, *bteam;
extern int repollfd, bepollfd;
extern pthread_mutex_t rmutex, bmutex;

void do_work(struct User *user){
    struct ChatMsg msg;
    struct ChatMsg r_msg;
    bzero(&msg, sizeof(msg));
    recv(user->fd, (void *)&msg, sizeof(msg), 0);
    strcpy(msg.name, user->name);
    if (msg.type & CHAT_WALL) {
        printf("<%s> ~ %s \n", user->name, msg.msg);
        send_all(&msg);
    } else if (msg.type & CHAT_MSG) {
        int sub = 0;
        while (msg.msg[sub] != ' ' && msg.msg[sub] != '\0') sub++;
        if (msg.msg[sub] != ' ' || msg.msg[0] != '@') {
            memset(&r_msg, 0, sizeof(r_msg));
            r_msg.type = CHAT_SYS;
            sprintf(r_msg.msg, "私聊格式错误！");
            send(user->fd, (void *)&r_msg, sizeof(r_msg), 0);
        } else {
            msg.type = CHAT_MSG;
            char name[20] = {0};
            msg.msg[sub] = '\0';
            strcpy(name, &msg.msg[1]);
            strcpy(msg.msg, &msg.msg[sub + 1]);
            printf("<%s> $ %s \n", user->name, msg.msg);
            if(send_to(name, &msg, user->fd)) send_to(user->name, &msg, user->fd);
        }

    } else if (msg.type & CHAT_FIN) {
        bzero(msg.msg, sizeof(msg.msg));
        msg.type = CHAT_SYS;
        sprintf(msg.msg, "%s 要下线了！", user->name);
        strcpy(msg.name, user->name);
        send_all(&msg);
        
        if (user->team)
            pthread_mutex_lock(&bmutex);
        else
            pthread_mutex_lock(&rmutex);
        user->online = 0;
        for (int i = 0; i < MAX; i++) {
            if (rteam[i].online && (!strcmp(user->name, rteam[i].name))) {
                rteam[i].online = 0;
                break;        
            }
            if (bteam[i].online && (!strcmp(user->name, bteam[i].name))) {
                bteam[i].online = 0;
                break;        
            }

        }
        int epollfd = user->team ? bepollfd : repollfd;
        del_event(epollfd, user);
        if (user->team)
            pthread_mutex_unlock(&bmutex);
        else
            pthread_mutex_unlock(&rmutex);
        printf(GREEN"Server Info"NONE" : %s logout!\n", user->name);
        close(user->fd);
    } else if (msg.type & CHAT_FUNC) {
        if (msg.msg[1] == '1') {
            char datas[1024] = {0};
            char tmp[1024] = {0};
            int num = 0;
            for (int i = 0; i < MAX; i++) {
                if (bteam[i].online == 1) {
                    if (num == 0)
                        sprintf(tmp, "%s", bteam[i].name);
                    else
                        sprintf(tmp, ",%s", bteam[i].name);
                    num++;
                    strcat(datas, tmp);
                }
                if (rteam[i].online == 1) {
                    if (num == 0)
                        sprintf(tmp, "%s", rteam[i].name);
                    else
                        sprintf(tmp, ",%s", rteam[i].name);
                    num++;
                    strcat(datas, tmp);
                }
            }
            sprintf(tmp, "  等 %d 位好友在线", num);
            strcat(datas, tmp);

            struct ChatMsg r_msg;
            bzero(&r_msg, sizeof(r_msg));
            strcpy(r_msg.msg, datas);
            r_msg.type = CHAT_SYS;
            send_to(user->name, &r_msg, user->fd);
        }
    }
}

void task_queue_init(struct task_queue *taskQueue, int sum, int epollfd) {
    taskQueue->sum = sum;
    taskQueue->epollfd = epollfd;
    taskQueue->team = calloc(sum, sizeof(void *));
    taskQueue->head = taskQueue->tail = 0;
    pthread_mutex_init(&taskQueue->mutex, NULL);
    pthread_cond_init(&taskQueue->cond, NULL);
}

void task_queue_push(struct task_queue *taskQueue, struct User *user) {
    pthread_mutex_lock(&taskQueue->mutex);
    taskQueue->team[taskQueue->tail] = user;
    DBG(L_GREEN"Thread Pool"NONE" : Task push %s\n", user->name);
    if (++taskQueue->tail == taskQueue->sum) {
        DBG(L_GREEN"Thread Pool"NONE" : Task Queue End\n");
        taskQueue->tail = 0;
    }
    pthread_cond_signal(&taskQueue->cond);
    pthread_mutex_unlock(&taskQueue->mutex);
}


struct User *task_queue_pop(struct task_queue *taskQueue) {
    pthread_mutex_lock(&taskQueue->mutex);
    while (taskQueue->tail == taskQueue->head) {
        DBG(L_GREEN"Thread Pool"NONE" : Task Queue Empty, Waiting For Task\n");
        pthread_cond_wait(&taskQueue->cond, &taskQueue->mutex);
    }
    struct User *user = taskQueue->team[taskQueue->head];
    DBG(L_GREEN"Thread Pool"NONE" : Task Pop %s\n", user->name);
    if (++taskQueue->head == taskQueue->sum) {
        DBG(L_GREEN"Thread Pool"NONE" : Task Queue End\n");
        taskQueue->head = 0;
    }
    pthread_mutex_unlock(&taskQueue->mutex);
    return user;
}

void *thread_run(void *arg) {
    pthread_detach(pthread_self());
    struct task_queue *taskQueue = (struct task_queue *)arg;
    while (1) {
        struct User *user = task_queue_pop(taskQueue);
        do_work(user);
    }
}

