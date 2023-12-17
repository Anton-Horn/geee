#include "job_system.h"

namespace ec {

	void JobSystem::create(uint32_t threadCount)
	{
		m_threads.resize(threadCount);
		for (uint32_t i = 0; i < threadCount; i++) {
			m_threads[i] = std::thread(&JobSystem::threadLoop, this);
		}
	}

	void JobSystem::destroy()
	{
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_shouldStop = true;
		}

		m_waitJob.notify_all();

		for (std::thread& thread : m_threads) {
			thread.join();
		}

	}

	void JobSystem::queueJob(const std::function<void()>& task)
	{
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_jobs.push(task);
		}
		m_waitJob.notify_one();

	}

	bool JobSystem::busy()
	{
		
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_occupiedWorkers;
		
	}

	void JobSystem::waitIdle()
	{

		while (true)
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			
			if (!m_occupiedWorkers) return;
		}

	}

	void JobSystem::threadLoop()
	{

		while (true) {

			std::unique_lock<std::mutex> lock(m_mutex);
			m_waitJob.wait(lock, [this] {
				return !m_jobs.empty() || m_shouldStop;
			});

			if (m_shouldStop) {
				return;
			}
			std::function<void()> task = m_jobs.front();
			m_jobs.pop();
			m_occupiedWorkers++;

			lock.unlock();		
			task();

			std::lock_guard<std::mutex> lock1(m_mutex);
			m_occupiedWorkers--;

		}

	}

}
