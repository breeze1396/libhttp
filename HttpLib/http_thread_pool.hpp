#ifndef HTTP_THREAD_POOL_HPP
#define HTTP_THREAD_POOL_HPP

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <stop_token>
#include <iostream>

namespace http_asio {

    class ThreadPool {
    public:
        // 构造函数：初始化线程池并启动指定数量的线程
        explicit ThreadPool(size_t num_threads)
            : stop_requested_(false) {
            for (size_t i = 0; i < num_threads; ++i) {
                threads_.emplace_back([this](std::stop_token stoken) {
                    while (!stoken.stop_requested()) {
                        std::function<void()> task;
                        {
                            // 互斥锁保护任务队列
                            std::unique_lock<std::mutex> lock(mutex_);
                            condition_.wait(lock, [this, &stoken] {
                                return stop_requested_ || !tasks_.empty();
                                });

                            if (stop_requested_ && tasks_.empty()) {
                                return;
                            }

                            task = std::move(tasks_.front());
                            tasks_.pop();
                        }
                        // 执行任务
                        task();
                    }
                    });
            }
        }

        // 禁用拷贝构造和拷贝赋值
        ThreadPool(const ThreadPool&) = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;

        // 析构函数：停止线程池并等待所有线程完成
        ~ThreadPool() {
            stop(); // 确保停止所有线程
        }

        // 提交任务到线程池，并返回一个future对象来获取结果
        template<typename Func, typename... Args>
        auto submit(Func&& func, Args&&... args)
            -> std::future<std::invoke_result_t<Func, Args...>> {
            using ReturnType = std::invoke_result_t<Func, Args...>;

            auto task = std::make_shared<std::packaged_task<ReturnType()>>(
                std::bind(std::forward<Func>(func), std::forward<Args>(args)...)
            );

            std::future<ReturnType> result = task->get_future();
            {
                std::unique_lock<std::mutex> lock(mutex_);
                if (stop_requested_) {
                    throw std::runtime_error("Submit on stopped ThreadPool");
                }
                tasks_.emplace([task]() { (*task)(); });
            }
            condition_.notify_one();
            return result;
        }

        // 停止线程池，停止所有线程并等待它们完成
        void stop() {
            {
                std::unique_lock<std::mutex> lock(mutex_);
                stop_requested_ = true;
            }
            condition_.notify_all(); // 唤醒所有线程
            for (auto& t : threads_) {
                if (t.joinable()) {
                    t.request_stop(); // 使用 std::jthread 的 stop 请求
                    t.join(); // 等待线程完成
                }
            }
        }

    private:
        std::vector<std::jthread> threads_; // 使用 std::jthread 管理线程
        std::queue<std::function<void()>> tasks_; // 任务队列
        std::mutex mutex_; // 保护任务队列
        std::condition_variable_any condition_; // 用于线程间的通知
        bool stop_requested_; // 标识线程池是否已请求停止
    };

} // namespace http_asio

#endif // HTTP_THREAD_POOL_HPP