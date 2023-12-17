#pragma once
#include <thread>
#include <functional>
#include <vector>
#include <queue>

#include "core.h"

namespace ec {

	class JobSystem {

	public:


		JobSystem() = default; 
		~JobSystem() = default; 

		JobSystem(const JobSystem&) = delete; 
		JobSystem(const JobSystem&&) = delete; 

		JobSystem& operator=(const JobSystem&) = delete; 
		JobSystem& operator=(const JobSystem&&) = delete;

		void create(uint32_t threadCount);
		void destroy();

		void queueJob(const std::function<void()>& task);
		bool busy();
		void waitIdle();

	private:

		void threadLoop();

		std::vector<std::thread> m_threads;

		std::mutex m_mutex;
		std::queue<std::function<void()>> m_jobs;

		std::condition_variable m_waitJob;
		

		bool m_shouldStop = false;
		uint32_t m_occupiedWorkers = 0;

	};

}