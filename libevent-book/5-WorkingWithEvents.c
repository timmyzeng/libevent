#if 0
/***************
 * event_new() *
 ***************/
#include <event2/event.h>
#include <stdio.h>

void cb_func(evutil_socket_t fd, short what, void* arg){
    const char* data = (const char*)arg;
    printf("Got an event on socket %d:%s%s%s%s [%s]",
           (int)fd,
           (what&EV_TIMEOUT) ? " timeout" : "",
           (what&EV_READ) ? " read" : "",
           (what&EV_WRITE) ? " write" : "",
           (what&EV_SIGNAL) ? " signal" : "",
           data);
}

void main_loop(evutil_socket_t fd1, evutil_socket_t fd2){
    struct event* ev1, *ev2;
    struct timeval five_seconds = {5, 0};
    struct event_base* base = event_base_new();

    ev1 = event_new(base, fd1, EV_TIMEOUT | EV_READ | EV_PERSIST,
                    cb_func, (char*)"Reading event");

    ev2 = event_new(base, fd2, EV_WRITE | EV_PERSIST,
                    cb_func, (char*)"Writing event");

    event_add(ev1, &five_seconds);
    event_add(ev2, NULL);
    event_base_dispatch(base);
}
#endif

#if 0
/**********************
 * event_self_cbarg() *
 ***********************/
#include <event2/event.h>
#include <stdio.h>

static int n_calls = 0;
void cb_func(evutil_socket_t fd, short what, void* arg){
    struct event* me = (struct event*)arg;
    printf("cb_func called %d times so far. \n", ++n_calls);

    if (n_calls > 100)
        event_del(me);
}

void run(struct event_base* base){
    struct timeval one_sec = {1, 0};
    struct event* ev;

    ev = event_new(base, -1, EV_PERSIST, cb_func, event_self_cbarg());
    event_add(ev, &one_sec);
    event_base_dispatch(base);
}
#endif

#if 0
#endif
/******************
 * event_assign() *
 ******************/
