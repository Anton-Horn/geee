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

	void JobSystem::queueJob(const std::function<void()>& task, bool shouldWait)
	{
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_jobs.push(std::make_tuple(task, shouldWait));
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

			std::tuple<std::function<void()>, bool> task = m_jobs.front();

			m_jobs.pop();
			if (std::get<1>(task))
			m_occupiedWorkers++;

			lock.unlock();		
			std::get<0>(task)();

			if (std::get<1>(task)) {
				std::lock_guard<std::mutex> lock1(m_mutex);
				m_occupiedWorkers--;
			}

		}

	}

}
