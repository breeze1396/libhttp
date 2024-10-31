#ifndef HTTP_ASIO_WRAPPER_HPP
#define HTTP_ASIO_WRAPPER_HPP

#include <asio.hpp>
#include <memory>
#include <string>
#include <functional>
#include <iostream>
#include <thread>
#include <fstream>
namespace http_asio {

    class IOContextWrapper {
    public:
        IOContextWrapper() : io_context_(std::make_shared<asio::io_context>()) {}

        std::shared_ptr<asio::io_context> getContext() {
            return io_context_;
        }

        void run() {
            io_context_->run();
        }

        void stop() {
            io_context_->stop();
        }

		~IOContextWrapper() {
			stop();
			std::cout << "IOContextWrapper destroyed" << std::endl;
		}

    private:
        std::shared_ptr<asio::io_context> io_context_;
    };


    class IOContextPool {
    public:
        explicit IOContextPool(std::size_t pool_size)
            : next_io_context_(0) {
            if (pool_size == 0) {
                throw std::runtime_error("Pool size must be greater than 0");
            }

            // 创建IOContextWrapper实例并存储在池中
            for (std::size_t i = 0; i < pool_size; ++i) {
                auto context = std::make_shared<IOContextWrapper>();
                io_contexts_.push_back(context);
                threads_.emplace_back([context]() { context->run(); });
            }
        }

        ~IOContextPool() {
            stop(); // 停止所有 io_context
            join(); // 等待所有线程完成
        }

        // 获取池中下一个 io_context 实例，采用轮询算法
        std::shared_ptr<asio::io_context> getNextContext() {
            std::lock_guard<std::mutex> lock(mutex_);
            auto io_context = io_contexts_[next_io_context_]->getContext();
            next_io_context_ = (next_io_context_ + 1) % io_contexts_.size();
            return io_context;
        }

        // 停止池中所有的 io_context 实例
        void stop() {
            for (auto& context : io_contexts_) {
                context->stop();
            }
        }

        // 等待所有线程完成
        void join() {
            for (auto& thread : threads_) {
                if (thread.joinable()) {
                    thread.join();
                }
            }
        }

    private:
        std::vector<std::shared_ptr<IOContextWrapper>> io_contexts_;
        std::vector<std::thread> threads_;
        std::size_t next_io_context_;
        std::mutex mutex_;
    };


    // TCP连接管理类
    class TCPConnection : public std::enable_shared_from_this<TCPConnection> {
    public:
        TCPConnection(std::shared_ptr<asio::io_context> io_context)
            : socket_(*io_context) {}

        asio::ip::tcp::socket& socket() {
            return socket_;
        }

        // 异步读取响应行
        void read_response_line(std::function<void(const std::string&, const asio::error_code&)> callback) {
            auto self = shared_from_this();
            asio::async_read_until(socket_, asio::dynamic_buffer(buffer_), "\r\n",
                [this, self, callback](const asio::error_code& ec, std::size_t bytes_transferred) {
                    if (!ec) {
						std::string response_line(buffer_.substr(0, bytes_transferred - 2)); // 去掉末尾的 \r\n
						//std::istream response_stream(response_line.data());
                        //std::string response_line;
                        //std::getline(response_stream, response_line);
                        callback(response_line, ec);
                    }
                    else {
                        callback("", ec);
                    }
                });
        }

        // 异步写入请求
        void write_request(const std::string& request, std::function<void(const asio::error_code&)> callback) {
            auto self = shared_from_this();
            asio::async_write(socket_, asio::buffer(request),
                [this, self, callback](const asio::error_code& ec, std::size_t /*bytes_transferred*/) {
                    callback(ec);
                });
        }

        // 连接到服务器
        void connect(const asio::ip::tcp::endpoint& endpoint, std::function<void(const asio::error_code&)> callback) {
            auto self = shared_from_this();
            socket_.async_connect(endpoint,
                [this, self, callback](const asio::error_code& ec) {
                    callback(ec);
                });
        }

    private:
        asio::ip::tcp::socket socket_;
        std::string buffer_;
    };

    // 定时器类
    class TimerWrapper {
    public:
        TimerWrapper(std::shared_ptr<asio::io_context> io_context)
            : timer_(*io_context) {}

        void start_timer(std::chrono::seconds duration, std::function<void(const asio::error_code&)> callback) {
            timer_.expires_after(duration);
            timer_.async_wait(callback);
        }

        void cancel_timer() {
            timer_.cancel();
        }

    private:
        asio::steady_timer timer_;
    };

} // namespace http_asio

#endif // HTTP_ASIO_WRAPPER_HPP
