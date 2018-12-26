#if 0
/*************************************
 * Example: Log messages in Libevent *
**************************************/

#include <event2/event.h>
#include <stdio.h>

/* 1 */
static void discard_cb(int severity, const char* msg){
    /* This callback does nothing */
}

/* Turn off all logging from Libevent */
void suppress_logging(void){
    event_set_log_callback(discard_cb);
}

/* 2 */
static FILE* logfile = NULL;
static void write_to_file_cb(int severity, const char* msg){
    const char* s;
    if (!logfile)
        return;
    switch (severity) {
        case _EVENT_LOG_DEBUG: s = "debug";break;
        case _EVENT_LOG_MSG: s = "msg";break;
        case _EVENT_LOG_WARN: s = "warn";break;
        case _EVENT_LOG_ERR: s = "error";break;
        default: s = "?";break;
    }
    fprintf(logfile, "[%s] %s\n", s, msg);
}

/* Redirect all Libevent log messages to the C stdio file 'f'. */
void set_logfile(FILE* f){
    logfile = f;
    event_set_log_callback(write_to_file_cb);
}
#endif

#if 0
/******************************
 * Example: Memory management *
*******************************/
/* Here’s a simple example that replaces Libevent’s allocation  */
/* functions with variants that count the total number  */
/* of bytes that are allocated. */

#include <event2/event.h>
#include <sys/types.h>
#include <stdlib.h>

/* This union's purpose is to be as big as the largest of all the */
/* types it contains */
union alignment {
    size_t sz;
    void* ptr;
    double dbl;
};
/* We need to make sure that everything we return is on the right */
/* alignment to hold anything, including a double.  */
#define ALIGNMENT sizeof(union alignment)

/* We need to do this cast-to-char* trick on our pointers to adjust */
/* them; doing arithmetic on a void* is not standard. */
#define OUTPTR(ptr) (((char*)ptr)+ALIGNMENT)
#define INPTR(ptr) (((char*)ptr)-ALIGNMENT)

static size_t total_allocated = 0;
static void* replacement_malloc(size_t sz){
    void* chunk = malloc(sz + ALIGNMENT);
    if (!chunk)
        return chunk;
    total_allocated += sz;
    *(size_t*)chunk = sz;
    return OUTPTR(chunk);
}

static void* replacement_realloc(void* ptr, size_t sz){
    size_t old_size = 0;
    if (ptr) {
        ptr = INPTR(ptr);
        old_size = *(size_t*)ptr;
    }
    ptr = realloc(ptr, sz + ALIGNMENT);
    if (!ptr)
        return NULL;
    *(size_t*)ptr = sz;
    total_allocated = total_allocated - old_size + sz;
    return OUTPTR(ptr);
}

static void replacement_free(void* ptr){
    ptr = INPTR(ptr);
    total_allocated -= *(size_t*)ptr;
    free(ptr);
}

void start_counting_bytes(void){
    event_set_mem_functions(replacement_malloc,
                            replacement_realloc,
                            replacement_free);
}
#endif

#if 0
/**********************************
 * Example: Debugging event usage *
***********************************/
#include <event2/event.h>
#include <event2/event_struct.h>

#include <stdlib.h>

void cb(evutil_socket_t fd, short what, void* ptr){
    struct event* ev = (struct event*)ptr;

    if (ev)
        event_debug_unassign(ev);
}

void mainloop(evutil_socket_t fdl, evutil_socket_t fd2, int debug_mode){
    struct event_base* base;
    struct event event_on_stack, *event_on_heap;

    if (debug_mode)
        event_enable_debug_mode();

    base = event_base_new();

    event_on_heap = event_new(base, fdl, EV_READ, cb, NULL);
    event_assign(&event_on_stack, base, fd2, EV_READ, cb, &event_on_stack);

    event_add(event_on_heap, NULL);
    event_add(&event_on_stack, NULL);

    event_base_dispatch(base);

    event_free(event_on_heap);
    event_base_free(base);
}
#endif

#if 0
/********************************
 * Example: Compile-time checks *
*********************************/
#include <event2/event.h>

#if !defined(LIBEVENT_VERSION_NUMBER) || LIBEVENT_VERSION_NUMBER < 0x02000100
#error "this version of Libevent is not supported; Get 2.0.1-alpha or later."
#endif

int make_sandwitch(void){
# if LIBEVENT_VERSION_NUMBER >= 0x06000500
    evutil_me_a_sandwich();
    return 0;
#else
    return -1;
#endif
}
#endif

#if 0
#endif
/****************************
 * Example: Run-time checks *
*****************************/
#include <event2/event.h>
#include <string.h>

int check_for_old_version(void){
    const char* v = event_get_version();
    if (!strncmp(v, "0.", 2) ||
        !strncmp(v, "1.1", 3) ||
        !strncmp(v, "1.2", 3) ||
        !strncmp(v, "1.3", 3)){
        printf("version is very old\n");
        return -1;
    } else {
        printf("Runing with Libevent version %s\n", v);
        return 0;
    }
}

int check_version_match(void){
    ev_uint32_t v_compile, v_run;
    v_compile = LIBEVENT_VERSION_NUMBER;
    v_run = event_get_version_number();
    if ((v_compile & 0xffff0000) != (v_run & 0xffff0000)) {
        printf("Running with a Libevent version (%s) very different from the"
               "one we were built with (%s). \n", event_get_version(), LIBEVENT_VERSION);
        return -1;
    }
    return 0;
}

int main(){
    check_for_old_version();
    check_version_match();
}
