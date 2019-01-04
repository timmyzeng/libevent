#if 0
/**********************************
 * Example: Shut down immediately *
 **********************************/

#include <event2/event.h>

void cb(int sock, short what, void* arg){
    struct event_base* base = (struct event_base*)arg;
    event_base_loopbreak(base);
}

void main_loop(struct event_base* base, evutil_socket_t watchdog_fd){
    struct event* watchdog_event;

    watchdog_event = event_new(base, watchdog_fd, EV_READ, cb, base);
    event_add(watchdog_event, NULL);
    event_base_dispatch(base);
}
#endif

#if 0
/*********************************************************
 * Example: Run an event loop for 10 seconds, then exit. *
 *********************************************************/

#include <event2/event.h>

void run_base_with_ticks(struct event_base* base){
    struct timeval ten_sec;

    ten_sec.tv_sec = 10;
    ten_sec.tv_usec = 0;

    while (1) {
        event_base_loopexit(base, &ten_sec);

        event_base_dispatch(base);
        puts("Tick");
    }
}
#endif
