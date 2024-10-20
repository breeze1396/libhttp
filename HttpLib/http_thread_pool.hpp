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
        // ���캯������ʼ���̳߳ز�����ָ���������߳�
        explicit ThreadPool(size_t num_threads)
            : stop_requested_(false) {
            for (size_t i = 0; i < num_threads; ++i) {
                threads_.emplace_back([this](std::stop_token stoken) {
                    while (!stoken.stop_requested()) {
                        std::function<void()> task;
                        {
                            // �����������������
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
                        // ִ������
                        task();
                    }
                    });
            }
        }

        // ���ÿ�������Ϳ�����ֵ
        ThreadPool(const ThreadPool&) = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;

        // ����������ֹͣ�̳߳ز��ȴ������߳����
        ~ThreadPool() {
            stop(); // ȷ��ֹͣ�����߳�
        }

        // �ύ�����̳߳أ�������һ��future��������ȡ���
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

        // ֹͣ�̳߳أ�ֹͣ�����̲߳��ȴ��������
        void stop() {
            {
                std::unique_lock<std::mutex> lock(mutex_);
                stop_requested_ = true;
            }
            condition_.notify_all(); // ���������߳�
            for (auto& t : threads_) {
                if (t.joinable()) {
                    t.request_stop(); // ʹ�� std::jthread �� stop ����
                    t.join(); // �ȴ��߳����
                }
            }
        }

    private:
        std::vector<std::jthread> threads_; // ʹ�� std::jthread �����߳�
        std::queue<std::function<void()>> tasks_; // �������
        std::mutex mutex_; // �����������
        std::condition_variable_any condition_; // �����̼߳��֪ͨ
        bool stop_requested_; // ��ʶ�̳߳��Ƿ�������ֹͣ
    };

} // namespace http_asio

#endif // HTTP_THREAD_POOL_HPP