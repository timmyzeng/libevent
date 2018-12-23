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
