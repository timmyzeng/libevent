#if 0
/***********************************************
 * Example: Preferring edge-triggered backends *
 ***********************************************/
#include <event2/event.h>

int main(){
    struct event_config *cfg;
    struct event_base *base;

    /* 创建event_config */
    cfg = event_config_new();
    /* 不使用select */
    event_config_avoid_method(cfg, "select");
    /* 使用边沿触发 */
    event_config_require_features(cfg, EV_FEATURE_ET);
    /* 使用event_config创建event_base */
    base = event_base_new_with_config(cfg);
    /* 释放event_config创建的event_base */
    event_config_free(cfg);
    return 0;
}
#endif

#if 0
/****************************************
 * Example: Avoiding priority-inversion *
 ****************************************/
#include <event2/event.h>

int main(){
    struct event_config* cfg;
    struct event_base* base;

    cfg = event_config_new();
    if (!cfg) {
        /* 处理错误 */
    }
    /* 让events运行在两个级别中，期望优先级低的进行更少次的
     * 函数回调，优先级高的events不超过100 mesc 或 5 次回调
     * 中会被调用一次 */
    struct timeval msec_100 = {0, 100*100};
    event_config_set_max_dispatch_interval(cfg, &msec_100, 5, 1);
    base = event_base_new_with_config(cfg);
    if (!base) {
        /* 处理错误 */
    }
    event_base_priority_init(base, 2);

    return 0;
}
#endif

#if 0
/******************************************
 * Example: event_get_supported_methods() *
 ******************************************/
#include <event2/event.h>

int main(){
    int i;
    const char** methods = event_get_supported_methods();
    printf("Starting Libevent %s, Avaliable methods are: \n",
           event_get_version());
    for (i=0; methods[i] != NULL; ++i) {
        printf("    %s\n", methods[i]);
    }
    return 0;
}
#endif

#if 0
/**************************************
 * Example: event_base_get_methods()  *
 *          event_base_get_features() *
 **************************************/
#include <event2/event.h>
#include <stdio.h>
#include <stdlib.h>

int main(){
    struct event_base* base;
    enum event_method_feature f;

    base = event_base_new();
    if (!base) {
        puts("Couldn't get an event_base!");
    } else {
        printf("Using Libevent with backend method %s.\n",
               event_base_get_method(base));
        f = event_base_get_features(base);
        if ((f & EV_FEATURE_ET))
            printf("    Edeg-triggered events are supported.\n");
        if ((f & EV_FEATURE_O1))
            printf("    O(1) events notification is supported.\n");
        if ((f & EV_FEATURE_FDS))
            printf("    ALL FD types aevents re supported.\n");
        puts(" ");
    }
    return 0;
}
#endif

#if 0
/***************************
 * Example: event_reinit() *
 ***************************/
struct event_base* base = event_base_new();
/* ... */
if (fork()) {
    continue_running_parent(base);
    /* ... */
} else {
    event_reinit(base);
    continue_running_child(base);
    /* ... */
}
#endif
