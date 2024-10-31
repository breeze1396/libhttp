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

            // ����IOContextWrapperʵ�����洢�ڳ���
            for (std::size_t i = 0; i < pool_size; ++i) {
                auto context = std::make_shared<IOContextWrapper>();
                io_contexts_.push_back(context);
                threads_.emplace_back([context]() { context->run(); });
            }
        }

        ~IOContextPool() {
            stop(); // ֹͣ���� io_context
            join(); // �ȴ������߳����
        }

        // ��ȡ������һ�� io_context ʵ����������ѯ�㷨
        std::shared_ptr<asio::io_context> getNextContext() {
            std::lock_guard<std::mutex> lock(mutex_);
            auto io_context = io_contexts_[next_io_context_]->getContext();
            next_io_context_ = (next_io_context_ + 1) % io_contexts_.size();
            return io_context;
        }

        // ֹͣ�������е� io_context ʵ��
        void stop() {
            for (auto& context : io_contexts_) {
                context->stop();
            }
        }

        // �ȴ������߳����
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


    // TCP���ӹ�����
    class TCPConnection : public std::enable_shared_from_this<TCPConnection> {
    public:
        TCPConnection(std::shared_ptr<asio::io_context> io_context)
            : socket_(*io_context) {}

        asio::ip::tcp::socket& socket() {
            return socket_;
        }

        // �첽��ȡ��Ӧ��
        void read_response_line(std::function<void(const std::string&, const asio::error_code&)> callback) {
            auto self = shared_from_this();
            asio::async_read_until(socket_, asio::dynamic_buffer(buffer_), "\r\n",
                [this, self, callback](const asio::error_code& ec, std::size_t bytes_transferred) {
                    if (!ec) {
						std::string response_line(buffer_.substr(0, bytes_transferred - 2)); // ȥ��ĩβ�� \r\n
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

        // �첽д������
        void write_request(const std::string& request, std::function<void(const asio::error_code&)> callback) {
            auto self = shared_from_this();
            asio::async_write(socket_, asio::buffer(request),
                [this, self, callback](const asio::error_code& ec, std::size_t /*bytes_transferred*/) {
                    callback(ec);
                });
        }

        // ���ӵ�������
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

    // ��ʱ����
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
