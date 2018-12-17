#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <thread>
#include "BlockingQueue.h"
#include <cassert>

int main()
{
	const std::size_t nThreads = 5;
	std::vector<std::thread> producers(nThreads);
	std::vector<std::thread> consumers(nThreads);

	const std::size_t nTasks = 10;

	jojo::BlockingQueue<std::size_t> queue;

	printf("begin!\n");

	for (auto i = 0; i < nThreads; ++i)
	{
		producers.emplace_back([nTasks, &queue](std::size_t i)
		{
			for (auto j = 0; j < nTasks; ++j)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(500));

				printf("producer[%zu]: push %zu\n", i, i);
				queue.Push(i);
				//while (!queue.TryPush(i));
			}
		}, i);

		consumers.emplace_back([nTasks, &queue](std::size_t i)
		{
			for (auto j = 0; j < nTasks; ++j)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(100));

				const auto data = queue.Pop();
				//std::size_t data = -1;  while (!queue.TryPop(data));
				printf("consumer[%zu]: pop %zu\n", i, data);
			}
		}, i);
	}

	for (auto& producer: producers)
	{
		if (producer.joinable())
			producer.join();
	}
	for (auto& consumer: consumers)
	{
		if (consumer.joinable())
			consumer.join();
	}

	system("pause");
	assert(queue.IsEmpty());

	return 0;
}
