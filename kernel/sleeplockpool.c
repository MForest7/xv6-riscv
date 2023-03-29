#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "sleeplock.h"

#define SLEEPLOCK_POOL_SIZE 512

enum lock_state { FREE, USING, PROCESSING };

struct sleeplock pool[SLEEPLOCK_POOL_SIZE];
enum lock_state state[SLEEPLOCK_POOL_SIZE];
int free_locks[SLEEPLOCK_POOL_SIZE];
int head;
struct spinlock pool_lock;

void initsleeplockpool() {
    head = 0;

    initlock(&pool_lock, "sleeplock pool lock");

    for (int i = 0; i < SLEEPLOCK_POOL_SIZE; i++) {
        initsleeplock(&pool[i], "pool");
        free_locks[i] = i;
        state[i] = FREE;
    }
}

int initpoollock(int* ld) {
    acquire(&pool_lock);
    if (head == SLEEPLOCK_POOL_SIZE)
        return -2; // no locks available

    *ld = free_locks[head++];
    state[*ld] = USING;
    release(&pool_lock);
    return 0;
}

static int check_ld_is_valid(int ld) {
    if (ld < 0 || SLEEPLOCK_POOL_SIZE <= ld) {
        return -1; // index is out of bounds
    }
    if (state[ld] == FREE) {
        return -3; // uninitialized lock
    }
    return 0;
}

static int get_lock_ownership(int ld) {
    while (1) {
        acquire(&pool_lock);

        int error = check_ld_is_valid(ld); 
        if (error) {
            release(&pool_lock);
            return error;
        }

        if (state[ld] != PROCESSING) {
            state[ld] = PROCESSING;
            release(&pool_lock);
            return 0;
        }
        release(&pool_lock);
    }
}

static void end_lock_ownership(int ld, enum lock_state new_state) {
    acquire(&pool_lock);
    state[ld] = new_state;
    release(&pool_lock);
}

int acquirepoollock(int ld) {
    int error;
    if ((error = get_lock_ownership(ld)) != 0)
        return error;
    
    acquiresleep(&pool[ld]);

    end_lock_ownership(ld, USING);
    return 0;
}

int releasepoollock(int ld) {
    int error;
    if ((error = get_lock_ownership(ld)) != 0)
        return error;
    
    releasesleep(&pool[ld]);

    end_lock_ownership(ld, USING);
    return 0;
}

int holdingpoollock(int ld) {
    int error;
    if ((error = get_lock_ownership(ld)) != 0)
        return error;
    
    int res = holdingsleep(&pool[ld]);

    end_lock_ownership(ld, USING);
    return res;
}

int deletepoollock(int ld) {
    int error;
    if ((error = get_lock_ownership(ld)) != 0)
        return error;

    head--;
    free_locks[head] = ld;

    end_lock_ownership(ld, FREE);
    return 0;
}