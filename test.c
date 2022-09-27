#include <coroutine.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#define VERBOSE

#ifdef VERBOSE
#define LOG(format, ...) \
	printf("[%s %d]: " format, __func__, __LINE__, ##__VA_ARGS__)
#else
#define LOG(format, ...)
#endif

void counting(struct schedule *sch, void *data)
{
	int i;
	int count = *(int *)data;

	LOG("%d, get number %d\n", __LINE__, count);

	printf("Ready to print number till %d\n", count);

	for (i = 0; i < count; i++) {
		// take break
		if (i != 0 && !(i % 10)) {
			printf("\ni %d, take a break\n", i);
			co_yield(sch);
		}

		printf("%d ", i);
	}

	printf("\nFinished counting!\n");
}

void do_loop(struct schedule *sch)
{
	char *line = malloc(256);
	size_t len = 0;
	int num;
	struct coroutine *co;

	while ((num = getline(&line, &len, stdin))) {
		// remove trailing newline
		line[--num] = '\0';

		LOG("%s\n",line);

		if (!strcmp(line, "q"))
			break;

		int count = atoi(line);
		if (count) {
			LOG("get number %d\n", count);

			co = co_find_suspend(sch, count);
			if (co) {
				LOG("find suspended with key %d\n", count);
				co_resume(sch, co);
			} else {
				co = co_get(sch, counting, (void *)&count);
				co_start(sch, co);
			}
		}

	}

	free(line);
}

int main (int argc, char *argv[])
{
	struct schedule *sch = co_scheduler_create();

	do_loop(sch);

	co_scheduler_close(sch);

	return 0;
}

