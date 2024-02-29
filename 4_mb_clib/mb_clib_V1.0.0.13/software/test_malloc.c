#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>

static pthread_mutex_t lock;/*锁count*/
static int count = 0;/*计数器, malloc: count++, free: count--*/

void * test_malloc(size_t size)
{
	void * new_mem;

	assert(size != 0);
	new_mem = calloc(1,size);
	if (new_mem)
	{/*注意当malloc成功时, 才能操作count*/
		pthread_mutex_lock(&lock);
		++count;
		//printf("malloc count = %d\n", count);
		pthread_mutex_unlock(&lock);
	}
	return new_mem;
}

void test_free(void * ptr)
{
	assert(ptr != NULL);/*当free(NULL)时, 说明程序中有不合理的地方, 直接退出*/
	free(ptr);
	pthread_mutex_lock(&lock);
	//printf("free count = %d\n", count);
	--count;
	pthread_mutex_unlock(&lock);
}

void mem_leak_check_result(void)
{
	int temp;
	pthread_mutex_lock(&lock);
	temp = count;
	pthread_mutex_unlock(&lock);
	if (temp > 0)
	{
		printf("memery leak! count=%d\n", count);
	}
	else if (temp == 0)
	{
		printf("no memery leak!\n");
	}
	else
	{
		printf("pigs might fly count = %d!\n", count);
	}
}

int mem_leak_check_init(void)
{
	if (pthread_mutex_init(&lock, NULL) != 0)
	{
		return -1;
	}

	return 0;
}

void mem_leak_check_destroy(void)
{
	pthread_mutex_destroy(&lock);
}