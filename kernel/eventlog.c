#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "spinlock.h"

static struct log_controller {
    struct spinlock lock;
    uint64 interrupts;
    uint64 switches;
    uint64 syscalls;
    int initialized;
} controller;

void init_log_controller() {
    initlock(&controller.lock, "log controller lock");
    controller.interrupts = 1;
    controller.switches = 1;
    controller.syscalls = 1;
    controller.initialized = 1;
}

#define SET_LOGGING_EVENT(event_type) \
void set_ ## event_type ## _logging(int ticks) { \
    acquire(&controller.lock); \
    if (ticks == 0) \
        controller.event_type = 1; \
    if (ticks == 1) \
        controller.event_type = 0; \
    else \
        controller.event_type = uptime() + ticks; \
    release(&controller.lock); \
} \
\
int event_type ## _logged() { \
    acquire(&controller.lock); \
    int logged = (controller.initialized == 1 && (controller.event_type >= uptime() + 1 || controller.event_type == 0)); \
    release(&controller.lock); \
    return logged; \
}

SET_LOGGING_EVENT(interrupts)
SET_LOGGING_EVENT(switches)
SET_LOGGING_EVENT(syscalls)
    