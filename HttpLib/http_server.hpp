#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#include "http_types.hpp"
#include "http_response.hpp"
#include "http_request.hpp"
#include "http_asio_wrapper.hpp"
#include "http_util.hpp"

#include <asio.hpp>
#include <string>
#include <unordered_map>
#include <functional>
#include <memory>
#include <iostream>
#include <queue>
#include <regex>
#include <map>


namespace http_asio {
	class SessionPool;
	class Server;
	class Session;

    using Handler = std::function<void(const Request&, Response&)>;

    class Session : public std::enable_shared_from_this<Session> {
    public:
        Session(asio::ip::tcp::socket socket, std::shared_ptr<IOContextWrapper> io_context, 
            std::function<void(Response&)> error_handler, std::weak_ptr<SessionPool> pool)
			: socket_(std::move(socket)), io_context_(io_context), error_handler_(error_handler), pool_(pool),
            buffer_(std::make_shared<asio::streambuf>()) {}

		~Session() {
			std::cout << "Session destroyed" << std::endl;
		}

        void start() {
            read_request();
        }

        void assignSocket(asio::ip::tcp::socket socket) {
            socket_ = std::move(socket);
        }

        void setHandlerMap(const std::unordered_map<std::string, Handler>& handlers) {
            handlers_ = handlers;
        }

        void returnSession();

    private:
        asio::ip::tcp::socket socket_;
        std::shared_ptr<IOContextWrapper> io_context_;
        std::function<void(Response&)> error_handler_;
        Request request_;
        std::unordered_map<std::string, Handler> handlers_;
        std::shared_ptr<asio::streambuf> buffer_;
        std::weak_ptr<SessionPool> pool_;
        
        class Deleter
        {
        public:
            void operator() (asio::streambuf* x) {
                std::cout << "Deleter function called" << std::endl;
                delete[] x;
            }
        };

        void read_request() {
            auto self(shared_from_this());
            asio::async_read_until(socket_, *buffer_, "\r\n\r\n",
                [this, self](std::error_code ec, std::size_t length) { // 捕获 buffer
                    if (!ec) {
                        parse_request(buffer_);
                        handle_request();
                    } else if (ec == asio::error::eof) {  // 处理 End of File 情况
                        std::cerr << "Client closed connection" << std::endl;
                        returnSession();
                    } else {
                        std::cerr << "Error during async_read_until: " << ec.message() << std::endl;
                        send_error_response(StatusCode::BadRequest);
                        returnSession();  // 释放 session
                    }
                    buffer_->consume(buffer_->size());
            });
        }

        // 测试使用
        void send_response(const std::string& response) {
            asio::write(socket_, asio::buffer(response));
            socket_.close();  // 关闭连接
        }

        void parse_request(std::shared_ptr<asio::streambuf> buffer) {
            try {
                std::istream request_stream(buffer.get());
                std::string request_line;

                // 检查是否有请求行
                if (!std::getline(request_stream, request_line) || request_line.empty()) {
                    std::cerr << "Failed to read request line or request is empty" << std::endl;
                    return;
                }

                std::istringstream request_line_stream(request_line);
                std::string method, uri;
                request_line_stream >> method >> uri;

                request_.setMethod(method);
                request_.Path = uri;

                std::string header;
                while (std::getline(request_stream, header) && header != "\r") {
                    //std::cout << request_line << std::endl;
                    size_t colon_index = header.find(':');
                    if (colon_index != std::string::npos) {
                        std::string key = header.substr(0, colon_index);
                        std::string value = header.substr(colon_index + 1);
                        trim(key); trim(value);
                        request_.Headers[key] = value;
                    }
                }

                //std::cout << "Parsed request: " << method << " " << uri << std::endl;
            } catch (const std::exception& ex) {
                std::cerr << "Exception in parse_request: " << ex.what() << std::endl;
            }
        }

        void handle_request() {
            Response response;
            std::string str = methodToString(request_.Method) + ':' + request_.Path;
            if (handlers_.count(str) > 0) {
                handlers_[str](request_, response);
            } else {
                response.setStatus(StatusCode::NotFound);
                response.setContent("404 Not Found", "text/html");
            }

            send_response(response);

            //std::cout << "Handled request: " << str << std::endl;
        }

        void send_response(const Response& response) {
            auto self(shared_from_this());

            auto response_data = std::make_shared<std::string>();
            std::ostringstream response_stream;
            response_stream << "HTTP/1.1 " << static_cast<int>(response.StatCde) << " " << response.StatusMsg << "\r\n";
            for (const auto& [key, value] : response.Headers) {
                response_stream << key << ": " << value << "\r\n";
            }
            response_stream << "Content-Length: " << response.Body.size() << "\r\n";
            response_stream << "Connection: close\r\n\r\n";
            response_stream << response.Body;

            *response_data = response_stream.str();  // 保存到 shared_ptr 中

            asio::async_write(socket_, asio::buffer(*response_data),
                [this, self, response_data](std::error_code ec, std::size_t length) {
                    if (ec) {
                        send_error_response(StatusCode::InternalServerError);
                    }

                    returnSession();
                });
        }

        void send_error_response(StatusCode status_code) {
            Response response;
            response.StatCde = status_code;
            error_handler_(response);
            send_response(response);
        }
    };

    class SessionPool : public std::enable_shared_from_this<SessionPool> {
    public:
        SessionPool(std::shared_ptr<IOContextWrapper> io_context, std::function<void(Response&)> error_handler)
            : io_context_(io_context), error_handler_(error_handler) {}

		~SessionPool() {
			std::lock_guard<std::mutex> lock(mutex_);
			while (!idle_sessions_.empty()) {
				idle_sessions_.pop();
			}
			std::cout << "SessionPool destroyed" << std::endl;
		}

        std::shared_ptr<Session> getSession(asio::ip::tcp::socket socket) {
            std::unique_lock<std::mutex> lock(mutex_);
            if (!idle_sessions_.empty()) {
                std::shared_ptr<Session> session = idle_sessions_.front();
				session->assignSocket(std::move(socket));
                idle_sessions_.pop();
                return session;
            }
            return std::make_shared<Session>(std::move(socket), io_context_, error_handler_, shared_from_this());
        }

        void returnSession(std::shared_ptr<Session> session) {
            std::lock_guard<std::mutex> lock(mutex_);
            idle_sessions_.push(session);
        }

		void setError_handler(std::function<void(Response&)> handler) {
			error_handler_ = handler;
		}

    private:
        std::shared_ptr<IOContextWrapper> io_context_;
        std::function<void(Response&)> error_handler_;
        std::queue<std::shared_ptr<Session>> idle_sessions_;
        std::mutex mutex_;
    };

    class Server {
    public:
        Server(short port, std::shared_ptr<IOContextWrapper> io_context = std::make_shared<IOContextWrapper>())
            : acceptor_(*io_context->getContext(), asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
            io_context_(io_context), session_pool_(std::make_shared<SessionPool>(io_context_, Server::errorHandlerFunc)) {
            start_accept();
        }

		~Server() {
			std::cout << "Server destroyed" << std::endl;
		}

        Server& Get(const std::string& pattern, Handler handler) {
            handlers_["GET:" + pattern] = handler;
            return *this;
        }

        Server& Post(const std::string& pattern, Handler handler) {
            handlers_["POST:" + pattern] = handler;
            return *this;
        }

        void set_error_handler(std::function<void(Response&)> handler) {
            error_handler_ = handler;
            session_pool_->setError_handler(handler);
        }

		void Run() {
			io_context_->run();
		}

    private:
        asio::ip::tcp::acceptor acceptor_;
        std::shared_ptr<IOContextWrapper> io_context_;
        std::function<void(Response&)> error_handler_;
        std::shared_ptr<SessionPool> session_pool_;
        std::unordered_map<std::string, Handler> handlers_;

        void start_accept() {
            acceptor_.async_accept([this](std::error_code ec, asio::ip::tcp::socket socket) {
                if (!ec) {
                    auto session = session_pool_->getSession(std::move(socket));
                    //auto session = std::make_shared<Session>(std::move(socket), io_context_, error_handler_);
                    session->setHandlerMap(handlers_);
                    session->start();
				} else {
					std::cerr << "Error during async_accept: " << ec.message() << std::endl;
				}
                start_accept();
            });
        }

		 static void errorHandlerFunc(Response& response) {
			response.setContent("Internal Server Error", "text/html");
			response.setStatus(StatusCode::InternalServerError);
		}
    };




    inline void Session::returnSession() {
        if (auto pool = pool_.lock()) {
            socket_.close();
            pool->returnSession(shared_from_this());
        }
    }
} // namespace http_asio

#endif // HTTP_SERVER_HPP
