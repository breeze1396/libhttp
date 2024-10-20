#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#include "http_types.hpp"
#include "http_response.hpp"
#include "http_util.hpp"
#include "http_request.hpp"

#include <asio.hpp>
#include <string>
#include <unordered_map>
#include <functional>
#include <memory>
#include <iostream>


namespace http_asio {

    class Session : public std::enable_shared_from_this<Session> {
    public:
        Session(asio::ip::tcp::socket socket, std::shared_ptr<IOContextWrapper> io_context, std::function<void(Response&)> error_handler)
            : socket_(std::move(socket)), io_context_(io_context), error_handler_(error_handler) {}

        void start() {
            read_request();
        }

    private:
        asio::ip::tcp::socket socket_;
        std::shared_ptr<IOContextWrapper> io_context_;
        std::function<void(Response&)> error_handler_;
        Request request_;

        void read_request() {
            auto self(shared_from_this());
            asio::async_read_until(socket_, asio::dynamic_buffer(buffer_), "\r\n\r\n",
                [this, self](std::error_code ec, std::size_t length) {
                    if (!ec) {
                        parse_request();
                        handle_request();
                    }
                    else {
                        send_error_response(HttpStatusCode::BadRequest);
                    }
                });
        }

        void parse_request() {
            std::istream request_stream(&buffer_);
            std::string request_line;
            std::getline(request_stream, request_line);
            // 解析请求行
            std::istringstream request_line_stream(request_line);
            std::string method, uri;
            request_line_stream >> method >> uri;

            request_.setMethod(method);
            request_.Path = uri;

            // 解析头部
            std::string header;
            while (std::getline(request_stream, header) && header != "\r") {
                size_t colon_index = header.find(':');
                if (colon_index != std::string::npos) {
                    std::string key = header.substr(0, colon_index);
                    std::string value = header.substr(colon_index + 1);
                    trim(key);      trim(value);
                    request_.Headers[key] = value;
                }
            }
        }

        void handle_request() {
            Response response;
            // 这里可以添加路由处理逻辑
            if (request_.Method == HttpMethod::GET) {
                handle_get(response);
            }
            else if (request_.Method == HttpMethod::POST) {
                handle_post(response);
            }
            else {
                send_error_response(HttpStatusCode::MethodNotAllowed);
                return;
            }
            send_response(response);
        }

        void handle_get(Response& response) {
            // 处理GET请求

        }

        void handle_post(Response& response) {

        }

		void handle_delete(Response& response) {

		}

		void handle_put(Response& response) {

		}

		void handle_options(Response& response) {

		}

        void send_response(const Response& response) {
            auto self(shared_from_this());
            std::ostringstream response_stream;
            response_stream << "HTTP/1.1 " << static_cast<int>(response.StatusCode) << " " << response.StatusMsg << "\r\n";
            for (const auto& [key, value] : response.Headers) {
                response_stream << key << ": " << value << "\r\n";
            }
            response_stream << "Content-Length: " << response.Body.size() << "\r\n";
            response_stream << "Connection: close\r\n\r\n";
            response_stream << response.Body;

            asio::async_write(socket_, asio::buffer(response_stream.str()),
                [this, self](std::error_code ec, std::size_t /*length*/) {
                    if (ec) {
                        send_error_response(HttpStatusCode::InternalServerError);
                    }
                });
        }

        void send_error_response(HttpStatusCode status_code) {
            Response response;
            response.StatusCode = status_code;
            error_handler_(response);
            send_response(response);
        }
    };

    class Server {
    public:
        Server(short port, std::shared_ptr<IOContextWrapper> io_context)
            : acceptor_(*io_context->getContext(), asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)), io_context_(io_context) {
            start_accept();
        }

        void set_error_handler(std::function<void(Response&)> handler) {
            error_handler_ = handler;
        }

    private:
        asio::ip::tcp::acceptor acceptor_;
        std::shared_ptr<IOContextWrapper> io_context_;
        std::function<void(Response&)> error_handler_;

        void start_accept() {
            acceptor_.async_accept([this](std::error_code ec, asio::ip::tcp::socket socket) {
                if (!ec) {
                    std::make_shared<Session>(std::move(socket), io_context_, error_handler_)->start();
                }
                start_accept();
                });
        }
    };

} // namespace http_asio

#endif // HTTP_SERVER_HPP