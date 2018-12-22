#if 0
/******************************************
 * Example: A simple blocking HTTP client *
 ******************************************/

/* sockaddr_in */
#include <netinet/in.h>
/* socket functions */
#include <sys/socket.h>
/* gethostbyname */
#include <netdb.h>

#include <unistd.h>
#include <string.h>
#include <stdio.h>

int main(){
    const char query[] = 
        "GET / HTTP/1.0\r\n"
        "Host: www.baidu.com\r\n"
        "\r\n";
    const char hostname[] = "www.baidu.com";
    struct sockaddr_in sin;
    struct hostent *h;
    const char* cp;
    int fd;
    ssize_t n_written, remaining;
    char buf[1024];
    
    /* look up the IP address for the hostname */
    /* gethostbyname isn't threadsafe  */
    h = gethostbyname(hostname);
    if (!h) {
        fprintf(stderr, "Couldn't lookup %s: %s", hostname, hstrerror(h_errno));
        return 1;
    }
    if (h->h_addrtype != AF_INET) {
        fprintf(stderr, "No ipv6 support, sorry");
        return 2;
    }

    /* socket */
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        return 3;
    }

    /* connect to the remote host */
    sin.sin_family = AF_INET;
    sin.sin_port = htons(80);
    sin.sin_addr = *(struct in_addr*)h->h_addr;
    if (connect(fd, (struct sockaddr*)&sin, sizeof(sin))) {
        perror("connect");
        close(fd);
        return 4;
    }

    /* write the query */
    cp = query;
    remaining = strlen(query);
    while (remaining) {
        n_written = send(fd, cp, remaining, 0);
        if (n_written <= 0) {
            perror("send");
            return 5;
        }
        remaining -= n_written;
        cp += n_written;
    }

    /* get an answer back */
    while (1) {
        ssize_t result = recv(fd, buf, sizeof(buf), 0);
        if (result == 0) {
            break;
        }
        else if (result < 0) {
            perror("recv");
            close(fd);
            return 6;
        }
        fwrite(buf, 1, result, stdout);
    }

    close(fd);
    return 0;
}
#endif

#if 0
/*************************************
 *            Bad Example            *
 *************************************/
/* This won't work */
char buf[1024];
int i, n;
while (i_still_want_to_read()) {
    for (i = 0; i < n_sockets; ++ i) {
        n = recv(fd[i], buf, sizeof(buf), 0);
        if (n == 0)
            handle_close(fd[i]);
        else if (n < 0)
            handle_error(fd[i], errno);
        else
            handle_input(fd[i], buf, n);
    }
}

/* if data arrives on fd[2] first, program won't even try reading */
/* from fd[2] until the reads from fd[0] and fd[1] have gotten */
/* some data and finished */
/* you can use multithreading or with multiprocess servers */
#endif

#if 0
/*************************************
 *        Forking ROT13 server       *
 *************************************/

/* sockaddr_in */
#include <netinet/in.h>
/* socket functions */
#include <sys/socket.h>

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_LINE 16384

char rot13_char(char c){
    if ((c >= 'a' && c <= 'm') || (c >= 'A' && c <= 'M'))
        return c + 13;
    else if ((c >= 'n' && c <= 'z') || (c >= 'N' && c <= 'Z'))
        return c + 13;
    else
        return c;
}

void child(int fd){
    char outbuf[MAX_LINE+1];
    size_t outbuf_used = 0;
    ssize_t result;

    while(1){
        char ch;
        result = recv(fd, &ch, 1, 0);
        if (result == 0) {
            break;
        }else if (result) {
            perror("read");
            break;
        }

        /* 防止数据溢出缓存 */
        if (outbuf_used < sizeof(outbuf)) {
            outbuf[outbuf_used++] = rot13_char(ch);
        }

        if (ch == '\n') {
            send(fd, outbuf, outbuf_used, 0);
            outbuf_used = 0;
            continue;
        }
    }
}

void run(void){
    int listener;
    struct sockaddr_in sin;

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = 0;
    sin.sin_port = htons(40713);

    listener = socket(AF_INET, SOCK_STREAM, 0);

#ifndef WIN32
    {
        int one = 1;
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    }
#endif

    if (bind(listener, (struct sockaddr*)&sin, sizeof(sin)) < 0) {
        perror("bind");
        return;
    }

    if (listen(listener, 16) < 0) {
        perror("listen");
        return;
    }

    while (1) {
        struct sockaddr_storage ss;
        socklen_t slen = sizeof(ss);
        int fd = accept(listener, (struct sockaddr*)&ss, &slen);
        if (fd < 0) {
            perror("accept");
        } else {
            if (fork() == 0) {
                child(fd);
                exit(0);
            }
        }
    }
}

int main(){
    run();
    return 0;
}
#endif

#if 0
/*****************************************
 * Bad Example: busy-polling all sockets *
 *****************************************/
int i, n;
char buf[1024];
for (i=0; i < n_sockets; ++i)
    fcntl(fd[i], F_SETFL, O_NONBLOCK);

while (i_still_want_to_read()) {
    for (i=0; i < n_sockets; ++i) {
        n = recv(fd[i], buf, sizeof(buf), 0);
        if (n == 0) {
            handle_close(fd[i]);
        } else if (n < 0) {
            if (errno == EAGAIN)
                ;
            else
                handle_error(fd[i], errno);
        } else {
            handle_input(fd[i], buf, n);
        }
    }
}
/* first: 这里会进行CPU轮询 */
/* second: 就算只有一两个socket有数据也会对所有的socket进行内核调用 */
#endif

#if 0
/****************************************
 * Example: select()-based ROT13 server *
 ****************************************/
/* sockaddr_in */
#include <netinet/in.h>
/* socket functions */
#include <sys/socket.h>
/* fcntl */
#include <fcntl.h>
/* select */
#include <sys/select.h>

#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define MAX_LINE 16384

char rot13_char(char c){
    if ((c >= 'a' && c <= 'm') || (c >= 'A' && c <= 'M'))
        return c + 13;
    else if ((c >= 'n' && c <= 'z') || (c >= 'N' && c <= 'Z'))
        return c + 13;
    else
        return c;
}

struct fd_state {
    char buffer[MAX_LINE];
    size_t buffer_used;

    int writing;
    size_t n_written;
    size_t write_upto;
};

struct fd_state* alloc_fd_state(void) {
    struct fd_state* state = (struct fd_state *)malloc(sizeof(struct fd_state));
    if (!state)
        return NULL;
    state->buffer_used = state->n_written = state->writing = state->write_upto = 0;
    return state;
}

void free_fd_state(struct fd_state* state) {
    free(state);
}

void make_nonblocking(int fd) {
    fcntl(fd, F_SETFL, O_NONBLOCK);
}

int do_read(int fd, struct fd_state* state) {
    char buf[1024];
    int i;
    ssize_t result;
    while (1) {
        result = recv(fd, buf, sizeof(buf), 0);
        if (result <=0) 
            break;

        for (i=0; i < result; ++i) {
            if (state->buffer_used < sizeof(state->buffer))
                state->buffer[state->buffer_used++] = rot13_char(buf[i]);
            if (buf[i] == '\n') {
                state->writing = 1;
                state->write_upto = state->buffer_used;
            }
        }
    }
    
    if (result == 0) {
        return 1;
    } else if (result < 0) {
        if (errno == EAGAIN)
            return 0;
        return -1;
    }

    return 0;
}

int do_write(int fd, struct fd_state* state) {
    while (state->n_written < state->write_upto) {
        ssize_t result = send(fd, state->buffer + state->n_written,
                              state->write_upto - state->n_written, 0);
        if (result < 0) {
            if (errno == EAGAIN)
                return 0;
            return -1;
        }
        assert(result != 0);

        state->n_written += result;
    }

    if (state->n_written == state->buffer_used)
        state->n_written = state->write_upto = state->buffer_used = 0;

    state->writing = 0;

    return 0;
}

void run(void) {
    int listener;
    struct fd_state* state[FD_SETSIZE];
    struct sockaddr_in sin;
    int i, maxfd;
    fd_set readset, writeset, exset;

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = 0;
    sin.sin_port = htons(40713);

    for (i=0; i < FD_SETSIZE; ++i) {
        state[i] = NULL;
    }

    listener = socket(AF_INET, SOCK_STREAM, 0);
    make_nonblocking(listener);

#ifndef WIN32
    {
        int one = 1;
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    }
#endif

    if (bind(listener, (struct sockaddr*)&sin, sizeof(sin)) < 0) {
        perror("bind");
        return;
    }

    FD_ZERO(&readset);
    FD_ZERO(&writeset);
    FD_ZERO(&exset);

    while (1) {
        maxfd = listener;

        FD_ZERO(&readset);
        FD_ZERO(&writeset);
        FD_ZERO(&exset);

        FD_SET(listener, &readset);

        for (i=0; i < FD_SETSIZE; ++i) {
            if (state[i]) {
                if (i > maxfd)
                    maxfd = i;
                FD_SET(i, &readset);
                if (state[i]->writing) {
                    FD_SET(i, &writeset);
                }
            }
        }

        if (select(maxfd+1, &readset, &writeset, &exset, NULL) < 0) {
            perror("select");
            return;
        }

        if (FD_ISSET(listener, &readset)) {
            struct sockaddr_storage ss;
            socklen_t slen = sizeof(ss);
            int fd = accept(listener, (struct sockaddr*)&ss, &slen);
            if (fd < 0) {
                perror("accept");
            } else if (fd > FD_SETSIZE) {
                close(fd);
            } else {
                make_nonblocking(fd);
                state[fd] = alloc_fd_state();
                assert(state[fd]);
            }
        }

        for (i=0; i < maxfd+1; ++i) {
            int r = 0;
            if (i == listener)
                continue;

            if (FD_ISSET(i, &readset)) {
                r = do_read(i, state[i]);
            }
            if (r == 0 && FD_ISSET(i, &writeset)) {
                r = do_write(i, state[i]);
            }
            if (r) {
                free_fd_state(state[i]);
                state[i] = NULL;
                close(i);
            }
        }
    }
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);

    run();
    return 0;
}
#endif

#if 0
/***************************************************
 * Example: A low-level ROT13 server with Libevent *
 * Using event2/event.h
 ***************************************************/
/* sockaddr_in */
#include <netinet/in.h>
/* socket functions */
#include <sys/socket.h>
/* fcntl */
#include <fcntl.h>

/* Libevent */
#include <event2/event.h>

#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define MAX_LINE 16384

void do_read(evutil_socket_t fd, short events, void *arg);
void do_write(evutil_socket_t fd, short events, void *arg);

char rot13_char(char c){
    if ((c >= 'a' && c <= 'm') || (c >= 'A' && c <= 'M'))
        return c + 13;
    else if ((c >= 'n' && c <= 'z') || (c >= 'N' && c <= 'Z'))
        return c + 13;
    else
        return c;
}

struct fd_state {
    char buffer[MAX_LINE];
    size_t buffer_used;

    size_t n_written;
    size_t write_upto;

    struct event* read_event;
    struct event* write_event;
};

struct fd_state* alloc_fd_state(struct event_base* base, evutil_socket_t fd){
    struct fd_state* state = (struct fd_state*)malloc(sizeof(struct fd_state));
    if (!state)
        return NULL;
    state->read_event = event_new(base, fd, EV_READ|EV_PERSIST, do_read, state);
    if (!state->read_event) {
        free(state);
        return NULL;
    }

    state->write_event = event_new(base, fd, EV_WRITE|EV_PERSIST, do_write, state);
    if (!state->write_event) {
        event_free(state->read_event);
        free(state);
        return NULL;
    }

    state->buffer_used = state->n_written = state->write_upto = 0;

    assert(state->write_event);
    return state;
}

void free_fd_state(struct fd_state* state){
    event_free(state->read_event);
    event_free(state->write_event);
    free(state);
}

void do_read(evutil_socket_t fd, short events, void* arg){
    struct fd_state* state =(struct fd_state*)arg;
    char buf[1024];
    int i;
    ssize_t result;
    while (1) {
        assert(state->write_event);
        result = recv(fd, buf, sizeof(buf), 0);
        if (result <= 0)
            break;

        for (i=0; i < result; ++i) {
            if (state->buffer_used < sizeof(state->buffer))
                state->buffer[state->buffer_used++] = rot13_char(buf[i]);
            if (buf[i] == '\n') {
                assert(state->write_event);
                event_add(state->write_event, NULL);
                state->write_upto = state->buffer_used;
            }
        }
    }

    if (result == 0) {
        free_fd_state(state);
    } else if (result < 0) {
        if (errno == EAGAIN)
            return;
        perror("recv");
        free_fd_state(state);
    }
}

void do_write(evutil_socket_t fd, short events, void* arg){
    struct fd_state* state = (struct fd_state*)arg;
    
    while (state->n_written < state->write_upto) {
        ssize_t result = send(fd, state->buffer + state->n_written,
                              state->write_upto - state->n_written, 0);
        if (result < 0) {
            if (errno == EAGAIN)
                return;
            free_fd_state(state);
            return;
        }
        assert(result != 0);

        state->n_written += result;
    }

    if (state->n_written == state->buffer_used)
        state->n_written = state->write_upto = state->buffer_used = 1;

    event_del(state->write_event);
}

void do_accept(evutil_socket_t listener, short event, void *arg){
    struct event_base* base = (struct event_base*) arg;
    struct sockaddr_storage ss;
    socklen_t slen = sizeof(ss);
    int fd = accept(listener, (struct sockaddr*)&ss, &slen);
    if (fd < 0) {
        perror("accept");
    } else if (fd > FD_SETSIZE) {
        close(fd);
    } else {
        struct fd_state* state;
        evutil_make_socket_nonblocking(fd);
        state = alloc_fd_state(base, fd);
        assert(state);
        assert(state->write_event);
        event_add(state->read_event, NULL);
    }
}

void run(void) {
    evutil_socket_t listener;
    struct sockaddr_in sin;
    struct event_base* base;
    struct event* listener_event;

    base = event_base_new();
    if (!base)
        return;

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = 0;
    sin.sin_port = htons(40713);

    listener = socket(AF_INET, SOCK_STREAM, 0);
    evutil_make_socket_nonblocking(listener);

#ifndef WIN32
    {
        int one = 1;
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    }
#endif

    if (bind(listener, (struct sockaddr*)&sin, sizeof(sin)) < 0) {
        perror("bind");
        return;
    }

    if (listen(listener, 16) < 0) {
        perror("listen");
        return;
    }

    listener_event = event_new(base, listener, EV_READ | EV_PERSIST, do_accept, (void*)base);
    event_add(listener_event, NULL);

    event_base_dispatch(base);
}

int main(){
    setvbuf(stdout, NULL, _IONBF, 0);

    run();
    return 0;
}
#endif

#if 1
/*************************************************
 * Example: A simpler ROT13 server with Libevent *
 * Using bufferevents
 *************************************************/
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define MAX_LINE 16384

void do_read(evutil_socket_t fd, short events, void *arg);
void do_write(evutil_socket_t fd, short events, void *arg);

char rot13_char(char c){
    if ((c >= 'a' && c <= 'm') || (c >= 'A' && c <= 'M'))
        return c + 13;
    else if ((c >= 'n' && c <= 'z') || (c >= 'N' && c <= 'Z'))
        return c + 13;
    else
        return c;
}

void readcb(struct bufferevent* bev, void *ctx){
    struct evbuffer* input, *output;
    char *line;
    size_t n;
    int i;
    input = bufferevent_get_input(bev);
    output = bufferevent_get_output(bev);

    while ((line = evbuffer_readln(input, &n, EVBUFFER_EOL_LF))) {
        for (i=0; i < (int)n; ++i)
            line[i] = rot13_char(line[i]);
        evbuffer_add(output, line, n);
        evbuffer_add(output, "\n", 1);
        free(line);
    }

    if (evbuffer_get_length(input) >= MAX_LINE) {
        char buf[1024];
        while (evbuffer_get_length(input)) {
            int n = evbuffer_remove(input, buf, sizeof(buf));
            for (i=0; i < n; ++i)
                buf[i] = rot13_char(buf[i]);
            evbuffer_add(output, buf, n);
        }
        evbuffer_add(output, "\n", 1);
    }
}

void errorcb(struct bufferevent* bev, short error, void *ctx){
    if (error & BEV_EVENT_EOF) {
        /* connection has been closed, do any clean up here */
    } else if (error & BEV_EVENT_ERROR) {
        /* check errno to see what error occurred */
    } else if (error & BEV_EVENT_TIMEOUT) {
        /* must be a timeout event handle, handle it */
    }
    bufferevent_free(bev);
}

void do_accept(evutil_socket_t listener, short event, void* arg){
    struct event_base* base = (struct event_base*)arg;
    struct sockaddr_storage ss;
    socklen_t slen = sizeof(ss);
    int fd = accept(listener, (struct sockaddr*)&ss, &slen);
    if (fd < 0) {
        perror("accept");
    } else if (fd > FD_SETSIZE) {
        close(fd);
    } else {
        struct bufferevent* bev;
        evutil_make_socket_nonblocking(fd);
        bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
        bufferevent_setcb(bev, readcb, NULL, errorcb, NULL);
        bufferevent_setwatermark(bev, EV_READ, 0, MAX_LINE);
        bufferevent_enable(bev, EV_READ | EV_WRITE);
    }
}


void run(void) {
    evutil_socket_t listener;
    struct sockaddr_in sin;
    struct event_base* base;
    struct event* listener_event;

    base = event_base_new();
    if (!base)
        return;

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = 0;
    sin.sin_port = htons(40713);

    listener = socket(AF_INET, SOCK_STREAM, 0);
    evutil_make_socket_nonblocking(listener);

#ifndef WIN32
    {
        int one = 1;
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    }
#endif

    if (bind(listener, (struct sockaddr*)&sin, sizeof(sin)) < 0) {
        perror("bind");
        return;
    }

    if (listen(listener, 16) < 0) {
        perror("listen");
        return;
    }

    listener_event = event_new(base, listener, EV_READ | EV_PERSIST, do_accept, (void*)base);
    event_add(listener_event, NULL);

    event_base_dispatch(base);
}

int main(){
    setvbuf(stdout, NULL, _IONBF, 0);

    run();
    return 0;
}
#endif
