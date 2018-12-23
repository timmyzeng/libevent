/************************** 
 *        设定定时器       *
 ***************************/
#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/event_compat.h>

#include <stdio.h>

struct event ev;
struct timeval tv;

void time_cb(int fd, short event, void *argc){
    printf("timer wakeup\n");
    event_add(&ev, &tv);
}

int main(){
    /* 初始化libevent库 */
    struct event_base* base = event_init();

    tv.tv_sec = 10;
    tv.tv_usec = 0;

    /* 设置回调函数和关注的事件 */
    evtimer_set(&ev, time_cb, NULL);
    /* 添加事件 */
    event_add(&ev, &tv);
    /* 进入无限循环，等待就绪事件并执行事件处理 */
    event_base_dispatch(base);
}
