#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

class Task
{
public:
	virtual void run()
	{
	}
};

class CSThreadPool
{
private:
	std::vector<std::thread> threads_;
	std::queue<Task*> tasks_;
	std::mutex mtx_;
	std::atomic_bool running_flag_;
	std::condition_variable cv_;
public:
	CSThreadPool(int count) :
		running_flag_(false)
	{
		for (int i = 0; i < count; i++)
		{
			threads_.emplace_back(&CSThreadPool::work, this);
		}
	}
	~CSThreadPool()
	{
		stop();
	}
	void start()
	{
		running_flag_ = true;
	}
	void stop()
	{
		cv_.notify_all();
		running_flag_ = false;
		for (auto& th : threads_)
		{
			if (th.joinable())
			{
				th.join();
			}
		}
	}
	void push_task(Task* task)
	{
		std::unique_lock<std::mutex> ulock(this->mtx_);
		this->tasks_.push(task);
		ulock.unlock();
		this->cv_.notify_one();
	}
private:
	void work()
	{
		while (running_flag_)
		{
			Task* task = nullptr;

			std::unique_lock<std::mutex> ulock(this->mtx_);
			while (this->tasks_.empty())
			{
				cv_.wait(ulock);
			}
			if (running_flag_)
			{
				task = this->tasks_.front();
				this->tasks_.pop();
			}

			if (task != nullptr)
			{
				task->run();
			}
		}
	}
};