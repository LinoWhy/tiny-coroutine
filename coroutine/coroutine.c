#include <stdio.h>
#include <stdlib.h>
#include <sys/ucontext.h>
#include <coroutine.h>

#include "list.h"

#define STACK_SIZE (1024 * 1024)
#define DEFAULT_COROUTINE 16

enum co_state {
	NEW,
	READY,
	SUSPEND,
	RUNNING,
	DEAD
};

struct coroutine {
	co_func func;
	void * data;
	struct schedule *sch;
	enum co_state status;
	ucontext_t ctx;
	char *stack;
};

struct schedule {
	struct coroutine *running;
	struct list_head ready_list;
	struct list_head suspend_list;
	ucontext_t ctx_main;
	struct coroutine **co_list;
	int cap;
	int num;
};

struct schedule *co_scheduler_create()
{
	struct schedule *sch = calloc(1, sizeof(struct schedule));

	sch->co_list = calloc(DEFAULT_COROUTINE, sizeof(struct coroutine));
	sch->cap = DEFAULT_COROUTINE;
	sch->num = 0;
	sch->running = NULL;
	INIT_LIST_HEAD(&sch->ready_list);
	INIT_LIST_HEAD(&sch->suspend_list);

	return sch;
}

void co_scheduler_close(struct schedule *sch)
{
	free(sch->co_list);
	free(sch);
}
