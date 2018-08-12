#pragma once

// TODO: Rewrite this into a struct and make it cleaner.

#define NUM_THREADS 1

int32 worker_thread(void *ptr);

#define MAX_NUM_WORK 124
uint32 num_work_orders = 0;
uint32 next_work_order = 0;
Work work_queue[MAX_NUM_WORK];

SDL_mutex *queue_lock;
SDL_cond *queue_cond;

SDL_Thread *threads[NUM_THREADS];
void create_sdl_threads()
{
	queue_lock = SDL_CreateMutex();
	queue_cond = SDL_CreateCond();
	for (uint32 i = 0; i < NUM_THREADS; i++)
	{
		threads[i] = SDL_CreateThread(worker_thread, "Worker", (void *)0);
	}
}

void delete_sdl_threads()
{
	int32 _;
	for (uint32 i = 0; i < NUM_THREADS; i++)
	{
		SDL_WaitThread(threads[i], &_);
	}
	SDL_DestroyCond(queue_cond);
	SDL_DestroyMutex(queue_lock);
}

int32 send_work(Work work)
{
	SDL_LockMutex(queue_lock);
	if (num_work_orders == MAX_NUM_WORK)
		return 0; // We can't take more work right now.

	uint32 index = num_work_orders + next_work_order;
	index %= MAX_NUM_WORK;
	work_queue[index] = work;
	num_work_orders++;
	SDL_CondSignal(queue_cond);
	SDL_UnlockMutex(queue_lock);
	return 1;
}

static int32 worker_thread(void *_)
{
	while (running)
	{
		SDL_LockMutex(queue_lock);
		while (num_work_orders == 0)
		{
			SDL_CondWait(queue_cond, queue_lock);
		}
		Work order = work_queue[next_work_order++];
		next_work_order %= MAX_NUM_WORK;
		num_work_orders--;
		SDL_UnlockMutex(queue_lock);

		order.func(order.args);
	}
	return 1;
}
