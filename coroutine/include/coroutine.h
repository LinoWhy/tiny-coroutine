#ifndef COROUTINE_H
#define COROUTINE_H

struct schedule;

typedef void (*co_func_t)(struct schedule *sch, void *data);

struct schedule *co_scheduler_create();
void co_scheduler_close(struct schedule *sch);
struct coroutine *co_get(struct schedule *sch, co_func_t func, void *data);
struct coroutine *co_find_suspend(struct schedule *sch, int key);
void co_start(struct schedule *sch, struct coroutine *co);
void co_resume(struct schedule *sch, struct coroutine *co);
void co_yield(struct schedule *sch);

#endif
