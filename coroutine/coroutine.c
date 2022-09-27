#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>
#include <coroutine.h>
#include <list.h>

#define STACK_SIZE (1024 * 1024)
#define DEFAULT_COROUTINE 16

enum co_state {
	READY,
	SUSPEND,
	RUNNING,
	DEAD
};

struct coroutine {
	co_func_t		func;	// user function
	void			*data;	// argument for user function
	struct schedule	*sch;	// which schedule
	struct list_head list;
	enum co_state	status;
	int				key;	// used to identify coroutine
	ucontext_t		ctx;
	char			*stack;	// stack pointer
};

struct schedule {
	struct coroutine *running;		// the running coroutine
	struct list_head ready_list;
	struct list_head suspend_list;
	ucontext_t ctx_main;			// context for main coroutine
	struct coroutine **co_list;		// list of coroutines
	int cap;						// capacity of co_list
	int num;						// available number of co_list
};

struct schedule *co_scheduler_create()
{
	struct schedule *sch = calloc(1, sizeof(struct schedule));

	sch->co_list = calloc(DEFAULT_COROUTINE, sizeof(struct coroutine *));
	sch->cap = DEFAULT_COROUTINE;
	sch->num = 0;
	sch->running = NULL;
	INIT_LIST_HEAD(&sch->ready_list);
	INIT_LIST_HEAD(&sch->suspend_list);

	return sch;
}

void co_scheduler_close(struct schedule *sch)
{
	struct coroutine *co;

	for (int i = 0; i < sch->cap; i++) {
		co = sch->co_list[i];

		if (co) {
			free(co->stack);
			free(co);
		}
	}

	free(sch->co_list);
	free(sch);
}

struct coroutine *_co_find_free(struct schedule *sch)
{
	struct coroutine *co;

	for (int i = 0; i < sch->cap; i++) {
		co = sch->co_list[i];

		if (co && co->status == DEAD)
			return co;
	}

	return NULL;
}

struct coroutine *_co_create(struct schedule *sch)
{
	struct coroutine *co;

	if (sch->num >= sch->cap) {
		// TODO: resize schedule
	}

	for (int i = 0; i < sch->cap; i++) {
		co = sch->co_list[i];

		if (!co) {
			co = calloc(1, sizeof(*co));
			co->stack = malloc(STACK_SIZE);

			sch->co_list[i] = co;
			sch->num++;
			return co;
		}
	}

	return NULL;
}

/*
 * Get one free coroutine or create a new one.
 * Fill the user function & arguments.
 */
struct coroutine *co_get(struct schedule *sch, co_func_t func, void *data)
{
	assert(sch);

	struct coroutine *co;

	co = _co_find_free(sch);
	if (!co)
		co = _co_create(sch);
	if (!co) {
		printf("Failed to get coroutine\n");
		return NULL;
	}

	co->func = func;
	co->data = data;
	co->sch = sch;
	co->status = READY;
	co->key = *(int *)data;
	INIT_LIST_HEAD(&co->list);

	return co;
}

/*
 * Find suspended coroutine according to key.
 */
struct coroutine *co_find_suspend(struct schedule *sch, int key)
{
	struct coroutine *co, *b;

	list_for_each_entry_safe(co, b, &sch->suspend_list, list) {
		if (co->key == key)
			return co;
	}

	return NULL;
}

/*
 * Free the coroutine by just changing its status.
 */
void _co_free(struct coroutine *co)
{
	co->status = DEAD;
}

/*
 * Wrap the user function inside the coroutine
 */
void co_wrapper(struct schedule *sch)
{
	struct coroutine *co = sch->running;

	co->func(sch, co->data);

	_co_free(co);
}

/*
 * Start the newly created coroutine.
 */
void co_start(struct schedule *sch, struct coroutine *co)
{
	assert(sch);
	assert(co);
	assert(co->status == READY);

	getcontext(&co->ctx);
	co->ctx.uc_stack.ss_sp = co->stack;
	co->ctx.uc_stack.ss_size = STACK_SIZE;
	co->ctx.uc_link = &sch->ctx_main;

	sch->running = co;
	co->status = RUNNING;

	makecontext(&co->ctx, (void (*)(void))co_wrapper, 1, sch);
	swapcontext(&sch->ctx_main, &co->ctx);
}

/*
 * Resume the suspended coroutine.
 */
void co_resume(struct schedule *sch, struct coroutine *co)
{
	assert(sch);
	assert(co);
	assert(co->status == SUSPEND);

	sch->running = co;
	co->status = RUNNING;
	list_del_init(&co->list);

	swapcontext(&sch->ctx_main, &co->ctx);
}

/*
 * Suspend the current coroutine
 */
void co_yield(struct schedule *sch)
{
	assert(sch);

	struct coroutine *co = sch->running;

	sch->running = NULL;
	co->status = SUSPEND;
	list_move(&co->list, &sch->suspend_list);

	swapcontext(&co->ctx, &sch->ctx_main);
}
