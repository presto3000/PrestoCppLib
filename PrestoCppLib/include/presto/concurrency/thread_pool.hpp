#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future>
#include <atomic>

namespace presto::concurrency {

    class ThreadPool {
    public:
        explicit ThreadPool(size_t threads)
            : stop(false)
        {
            for (size_t i = 0; i < threads; ++i) {
                workers.emplace_back([this] {
                    for (;;)
                    {
                        std::function<void()> task;

                        {
                            std::unique_lock<std::mutex> lock(this->mutex);
                            this->cv.wait(lock, [this] {
                                return this->stop || !this->tasks.empty();
                                });

                            if (this->stop && this->tasks.empty())
                                return;

                            task = std::move(this->tasks.front());
                            this->tasks.pop();
                        }

                        task();
                    }
                    });
            }
        }

        ~ThreadPool()
        {
            {
                std::unique_lock<std::mutex> lock(mutex);
                stop = true;
            }

            cv.notify_all();

            for (std::thread& worker : workers)
                worker.join();
        }
		// function + args -> packaged_task -> future created -> task executed by worker thread -> future gets result
		// return a future that will hold the result of the task once it's executed
        template<class F, class... Args>
        auto enqueue(F&& f, Args&&... args) -> std::future<decltype(f(args...))>
        {
            using return_type = decltype(f(args...));

			// Wrap function into a packaged_task to get a future for the result
			// Turns f(5,10) for example into f() that can be called later by the worker thread
            // function + promise of result
			// std::shared_ptr<std::packaged_task<return_type()>> 
            auto task = std::make_shared<std::packaged_task<return_type()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...)
            );
			// handle to get the result from the future
            std::future<return_type> res = task->get_future();

            {
                std::unique_lock<std::mutex> lock(mutex);

                if (stop)
                    throw std::runtime_error("enqueue on stopped ThreadPool");
				// converting the packaged_task into a void() function
                // that can be stored in the queue and called by the worker thread
                tasks.emplace([task = std::move(task)]() mutable {
                    (*task)(); 
                    });
            }

            cv.notify_one();
            return res;
        }

    private:
        std::vector<std::thread> workers;
        std::queue<std::function<void()>> tasks;

        std::mutex mutex;
        std::condition_variable cv;
        bool stop;
    };
}