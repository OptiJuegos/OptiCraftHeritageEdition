#include "ThreadedFileIOBase.h"

#include <algorithm>
#include <cstddef>
#include <chrono>

#include "IThreadedFileIO.h"

ThreadedFileIOBase ThreadedFileIOBase::threadedIOInstance;

ThreadedFileIOBase::ThreadedFileIOBase() :
	writeQueuedCounter(0), savedIOCounter(0), isThreadWaiting(false), stopping(false),
	worker(&ThreadedFileIOBase::run, this)
{
}

ThreadedFileIOBase::~ThreadedFileIOBase()
{
	{
		std::lock_guard<std::mutex> guard(queueMutex);
		stopping = true;
	}
	queueCondition.notify_all();
	finishCondition.notify_all();
	if (worker.joinable())
		worker.join();
}

void ThreadedFileIOBase::run()
{
	for (;;)
	{
		{
			std::unique_lock<std::mutex> lock(queueMutex);
			queueCondition.wait_for(lock, std::chrono::milliseconds(25), [this]
			{
				return stopping || !threadedIOQueue.empty();
			});
			if (stopping)
				return;
		}
		processQueue();
	}
}

void ThreadedFileIOBase::processQueue()
{
	std::size_t index = 0;
	while (true)
	{
		IThreadedFileIO *task = nullptr;
		bool waiting = false;
		{
			std::lock_guard<std::mutex> guard(queueMutex);
			if (stopping || index >= threadedIOQueue.size())
				return;
			task = threadedIOQueue[index];
			waiting = isThreadWaiting;
		}

		const bool hasMore = task != nullptr && task->writeNextIO();
		{
			std::lock_guard<std::mutex> guard(queueMutex);
			if (index < threadedIOQueue.size() && threadedIOQueue[index] == task)
			{
				if (!hasMore)
				{
					// queueIO() may race with the transition from writeNextIO()
					// returning false to this removal.  Preserve the task when a
					// producer requested it again during that window; otherwise the
					// newly queued disk write could be left pending forever.
					if (requeueRequested.erase(task) != 0)
					{
						++index;
					}
					else
					{
						threadedIOQueue.erase(threadedIOQueue.begin() + (std::ptrdiff_t)index);
						++savedIOCounter;
						finishCondition.notify_all();
					}
				}
				else
				{
					// Any request made while the task was actively processing is
					// already visible to the task's own pending-work queue.
					requeueRequested.erase(task);
					++index;
				}
			}
		}

		if (!waiting)
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		else
			std::this_thread::yield();
	}
}

void ThreadedFileIOBase::queueIO(IThreadedFileIO *task)
{
	if (task == nullptr)
		return;

	{
		std::lock_guard<std::mutex> guard(queueMutex);
		if (std::find(threadedIOQueue.begin(), threadedIOQueue.end(), task) != threadedIOQueue.end())
		{
			requeueRequested.insert(task);
			return;
		}
		++writeQueuedCounter;
		threadedIOQueue.push_back(task);
	}
	queueCondition.notify_one();
}

void ThreadedFileIOBase::waitForFinish()
{
	std::unique_lock<std::mutex> lock(queueMutex);
	isThreadWaiting = true;
	queueCondition.notify_one();
	finishCondition.wait(lock, [this]
	{
		return stopping || writeQueuedCounter == savedIOCounter;
	});
	isThreadWaiting = false;
}
