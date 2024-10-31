// 寻找bug的文件
//#ifndef HTTP_SERVER_HPP
//#define HTTP_SERVER_HPP
//
//
//#include "http_types.hpp"
//#include "http_response.hpp"
//#include "http_request.hpp"
//
//#include <asio.hpp>
//#include <string>
//#include <unordered_map>
//#include <functional>
//#include <memory>
//#include <iostream>
//#include <queue>
//#include <regex>
//#include <map>
//#include "http_asio_wrapper.hpp"
//#include "http_util.hpp"
//
//namespace http_asio {
//    class SessionPool;
//    class Server;
//    class Session;
//    using Handler = std::function<void(const Request&, Response&)>;
//
//    class Session : public std::enable_shared_from_this<Session> {
//    public:
//        //Session(asio::ip::tcp::socket socket)
//        //    : socket_(std::move(socket)), buffer_(std::make_shared<asio::streambuf>()) {}
//        Session(asio::ip::tcp::socket socket, std::shared_ptr<IOContextWrapper> io_context,
//            std::function<void(Response&)> error_handler)
//            : socket_(std::move(socket)), io_context_(io_context), error_handler_(error_handler),
//            buffer_(std::make_shared<asio::streambuf>()) {}
//
//        void start() {
//            read_request();
//        }
//
//
//        void setHandlerMap(const std::unordered_map<std::string, Handler>& handlers) {
//            handlers_ = handlers;
//        }
//
//    private:
//        asio::ip::tcp::socket socket_;
//        std::shared_ptr<IOContextWrapper> io_context_;
//        std::function<void(Response&)> error_handler_;
//        Request request_;
//        std::unordered_map<std::string, Handler> handlers_;
//        //std::shared_ptr<asio::streambuf> buffer;
//
//        std::shared_ptr<asio::streambuf> buffer_;
//        Handler handler_;
//
//        class Deleter
//        {
//        public:
//            void operator() (asio::streambuf* x) {
//                std::cout << "Deleter function called" << std::endl;
//                delete[] x;
//            }
//        };
//
//        void read_request() {
//            try {
//                // 使用同步 read_until 读取到请求头结束标志 "\r\n\r\n"
//                std::size_t length = asio::read_until(socket_, *buffer_, "\r\n\r\n");
//
//                // 打印请求内容以检查接收到的数据
//                std::istream request_stream(buffer_.get());
//                std::string request_line;
//                std::getline(request_stream, request_line);
//
//                if (!request_line.empty()) {
//                    std::cout << "Request received: " << request_line << std::endl;
//                    
//                }
//
//                // 简单响应
//                send_response("HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!");
//            }
//            catch (const asio::system_error& ec) {
//                if (ec.code() == asio::error::eof) {
//                    std::cerr << "Client closed the connection (EOF received)." << std::endl;
//                }
//                else {
//                    std::cerr << "Error during read_until: " << ec.what() << std::endl;
//                }
//                socket_.close();
//            }
//        }
//
//        void parse_request(std::shared_ptr<asio::streambuf> buffer) {
//            try {
//                std::istream request_stream(buffer.get());
//                std::string request_line;
//
//                // 检查是否有请求行
//                if (!std::getline(request_stream, request_line) || request_line.empty()) {
//                    std::cerr << "Failed to read request line or request is empty" << std::endl;
//                    return;
//                }
//
//                std::istringstream request_line_stream(request_line);
//                std::string method, uri;
//                request_line_stream >> method >> uri;
//
//                request_.setMethod(method);
//                request_.Path = uri;
//
//                std::string header;
//                while (std::getline(request_stream, header) && header != "\r") {
//                    size_t colon_index = header.find(':');
//                    if (colon_index != std::string::npos) {
//                        std::string key = header.substr(0, colon_index);
//                        std::string value = header.substr(colon_index + 1);
//                        trim(key); trim(value);
//                        request_.Headers[key] = value;
//                    }
//                }
//
//                std::cout << "Parsed request: " << method << " " << uri << std::endl;
//            }
//            catch (const std::exception& ex) {
//                std::cerr << "Exception in parse_request: " << ex.what() << std::endl;
//            }
//        }
//
//        void send_response(const std::string& response) {
//            asio::write(socket_, asio::buffer(response));
//            socket_.close();  // 关闭连接
//        }
//
//        void send_response(const Response& response) {
//            auto self(shared_from_this());
//            std::ostringstream response_stream;
//            response_stream << "HTTP/1.1 " << static_cast<int>(response.StatCde) << " " << response.StatusMsg << "\r\n";
//            for (const auto& [key, value] : response.Headers) {
//                response_stream << key << ": " << value << "\r\n";
//            }
//            response_stream << "Content-Length: " << response.Body.size() << "\r\n";
//            response_stream << "Connection: close\r\n\r\n";
//            response_stream << response.Body;
//
//            asio::async_write(socket_, asio::buffer(response_stream.str()),
//                [this, self](std::error_code ec, std::size_t /*length*/) {
//                    if (ec) {
//                        send_error_response(StatusCode::InternalServerError);
//                    }
//                });
//        }
//
//        void send_error_response(StatusCode status_code) {
//            Response response;
//            response.StatCde = status_code;
//            error_handler_(response);
//            send_response(response);
//        }
//        void handle_request() {
//            Response response;
//            std::string str = methodToString(request_.Method) + ':' + request_.Path;
//            if (handlers_.count(str) > 0) {
//                handlers_[str](request_, response);
//            }
//            else {
//                response.setStatus(StatusCode::NotFound);
//                response.setContent("404 Not Found", "text/html");
//            }
//            send_response(response);
//
//            std::cout << "Handled request: " << str << std::endl;
//        }
//
//    };
//
//
//    class Server {
//    public:
//        Server(short port, std::shared_ptr<IOContextWrapper> io_context = std::make_shared<IOContextWrapper>())
//            : acceptor_(*io_context->getContext(), asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
//            io_context_(io_context) {
//            start_accept();
//        }
// 
//
//        Server& Get(const std::string& pattern, Handler handler) {
//            handlers_["GET:" + pattern] = handler;
//            return *this;
//        }
//
//        Server& Post(const std::string& pattern, Handler handler) {
//            handlers_["POST:" + pattern] = handler;
//            return *this;
//        }
//
//        void set_error_handler(std::function<void(Response&)> handler) {
//            error_handler_ = handler;
//            //session_pool_.setError_handler(handler);
//        }
//
//        void Run() {
//            io_context_->run();
//        }
//
//    private:
//        asio::ip::tcp::acceptor acceptor_;
//        std::shared_ptr<IOContextWrapper> io_context_;
//        std::function<void(Response&)> error_handler_;
//        //SessionPool session_pool_;
//        std::unordered_map<std::string, Handler> handlers_;
//
//        void start_accept() {
//            acceptor_.async_accept([this](std::error_code ec, asio::ip::tcp::socket socket) {
//                if (!ec) {
//                    auto session = std::make_shared<Session>(std::move(socket), io_context_, error_handler_);
//                    session->setHandlerMap(handlers_);
//                    session->start();
//                }
//                start_accept();  // 继续接受下一个连接
//                });
//        }
//    };
//} // namespace http_asio
//
//#endif // HTTP_SERVER_HPP
