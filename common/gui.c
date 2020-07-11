/*************************************************************************
	> File Name: gui.c
	> Author: tangxiaolong 
	> Mail: 2284841184@qq.com
	> Created Time: Fri 10 Jul 2020 10:11:14 PM CST
 ************************************************************************/

#include <string.h>
#include <curses.h>

int main(int argc,char* argv[]){
    initscr();
    raw();
    noecho();
    curs_set(0);

    char* c = "Hello, World!";

    mvprintw(LINES/2,(COLS-strlen(c))/2,c);
    refresh();

    getch();
    endwin();

    return 0;

}
