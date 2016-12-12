#include "TaskScheduler.h"
#include <iostream>

TaskScheduler::TaskScheduler()
{
	std::lock_guard<std::mutex> lck(m_mDataMutex);

	//Allocate storage (little bit reduntant with default constructors, but still good practice
	m_UnassignedQueueIndices = std::queue<int>();
	m_ActiveQueues = std::unordered_map<int, unsigned int>();
	m_QueuedTasks = std::queue<Task>();

	for (int i = 0; i < MAX_QUEUE_INDICIES; ++i)
	{
		m_UnassignedQueueIndices.push(i);
	}

	//Initiate Worker Threads
	m_IsTerminating = false;
	for (int i = 0; i < NUM_WORKER_THREADS; ++i)
	{
		m_WorkerThreads[i] = std::thread(&TaskScheduler::ThreadWorkLoop, this);
	}
}

TaskScheduler::~TaskScheduler()
{
	//Set Terminating Flag
	{
		std::lock_guard<std::mutex> lck(m_mDataMutex);
		m_IsTerminating = true;
	}

	//Inform all worker threads that the program is closing
	m_cvTaskReadyForProcessing.notify_all();

	//Wait for all worker threads to exit
	for (int i = 0; i < NUM_WORKER_THREADS; ++i)
	{
		m_WorkerThreads[i].join();
	}
}

int  TaskScheduler::BeginNewTaskQueue()
{
	if (m_UnassignedQueueIndices.size() == 0)
	{
		std::cout << "Task Scheduler Error: Unable to obtain free Task Queue Index.";
		return -1;
	}

	std::lock_guard<std::mutex> lck(m_mDataMutex);

	int idx = m_UnassignedQueueIndices.front();
	m_UnassignedQueueIndices.pop();

	m_ActiveQueues[idx] = 0;

	return idx;
}

void TaskScheduler::PostTaskToQueue(int queue_idx, const std::function<void()>& task)
{
	if (queue_idx == -1)
	{
		std::cout << "Task Scheduler Error: Invalid Queue Index parsed as parameter";
		return;
	}

	{
		std::lock_guard<std::mutex> lck(m_mDataMutex);

		Task t;
		t.queue_idx = queue_idx;
		t.task_function = task;
		t.task_callback = []{};

		m_ActiveQueues[queue_idx]++;

		m_QueuedTasks.push(t);
	}

	m_cvTaskReadyForProcessing.notify_one();
}

void TaskScheduler::WaitForTaskQueueToComplete(int queue_idx)
{
	if (queue_idx == -1)
	{
		std::cout << "Task Scheduler Error: Invalid Queue Index parsed as parameter";
		return;
	}

	std::unique_lock<std::mutex> lck(m_mDataMutex);
	m_cvTaskCompleted.wait(lck, [&]{return (m_ActiveQueues[queue_idx] == 0); });

	m_UnassignedQueueIndices.push(queue_idx);
}

void TaskScheduler::ThreadWorkLoop()
{
	std::unique_lock<std::mutex> lck(m_mDataMutex);
	lck.unlock();
	Task cTask;
	while (true)
	{
		lck.lock();
		//Check if either the program is terminating or more work is already availible
		m_cvTaskReadyForProcessing.wait(lck, [&]{return (m_IsTerminating || !m_QueuedTasks.empty()); });

		//Once notified, or the confition already holds true, check again if the thread should terminate or if it has work todo.
		if (m_IsTerminating)
		{
			return;
		}
		else
		{
			cTask = m_QueuedTasks.front();
			m_QueuedTasks.pop();
		}

		lck.unlock();


		//Complete the task
		cTask.task_function();

		//Check to see if any callbacks are required upon completeion
		cTask.task_callback();

		//Finally notify queue that it has completed the work
		if (cTask.queue_idx != -1)
		{
			lck.lock();
			m_ActiveQueues[cTask.queue_idx]--;
			lck.unlock();
			m_cvTaskCompleted.notify_all();
		}
	}
}