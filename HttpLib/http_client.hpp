#ifndef HTTP_CLIENT_HPP
#define HTTP_CLIENT_HPP

#include "http_types.hpp"
#include "http_util.hpp"
#include "http_response.hpp"
#include "http_asio_wrapper.hpp"

#include <asio.hpp>
#include <string>
#include <unordered_map>
#include <memory>
#include <iostream>
#include <future>



namespace http_asio {

    class Client {
    public:
        Client(std::shared_ptr<IOContextWrapper> io_context)
            : io_context_(io_context), socket_(*io_context->getContext()) {}

        // 设置请求头
        void set_header(const std::string& key, const std::string& value) {
            Headers[key] = value;
        }

        // 设置超时
        void set_timeout(std::chrono::seconds timeout) {
            timeout_ = timeout;
        }

        // 发送 GET 请求
        std::future<Response> get(const std::string& url) {
            return send_request("GET", url);
        }

        // 发送 POST 请求
        std::future<Response> post(const std::string& url, const std::string& body) {
            set_header("Content-Length", std::to_string(body.size()));
            return send_request("POST", url, body);
        }

        // 发送 PUT 请求
        std::future<Response> put(const std::string& url, const std::string& body) {
            set_header("Content-Length", std::to_string(body.size()));
            return send_request("PUT", url, body);
        }

        // 发送 DELETE 请求
        std::future<Response> del(const std::string& url) {
            return send_request("DELETE", url);
        }

    private:
        std::shared_ptr<IOContextWrapper> io_context_;
        asio::ip::tcp::socket socket_;
        std::unordered_map<std::string, std::string> Headers;
        std::chrono::seconds timeout_{ 5 }; // 默认超时为5秒

        // 发送请求的通用方法
        std::future<Response> send_request(const std::string& method, const std::string& url, const std::string& body = "") {
            return std::async(std::launch::async, [this, method, url, body]() {
                try {
                    // 解析URL
                    auto [host, path] = parse_url(url);

                    // 连接服务器
                    connect_to_server(host);

                    // 创建请求字符串
                    std::string request = build_request(method, path, body);

                    // 发送请求
                    asio::write(socket_, asio::buffer(request));

                    // 接收响应
                    Response response = read_response();

                    // 关闭连接
                    socket_.close();
                    return response;

                } catch (const std::exception& e) {
                    std::cerr << "Error: " << e.what() << std::endl;
                    return Response{ HttpStatusCode::InternalServerError, "Internal Server Error" };
                }
                });
        }

        // 解析URL，返回主机名和路径
        std::pair<std::string, std::string> parse_url(const std::string& url) {
            std::string host, path;
            size_t pos = url.find("://");
            if (pos != std::string::npos) {
                size_t start = pos + 3; // 跳过协议
                size_t end = url.find('/', start);
                host = url.substr(start, end - start);
                path = end == std::string::npos ? "/" : url.substr(end);
            }
            return { host, path };
        }

        // 连接到服务器
        void connect_to_server(const std::string& host) {
            asio::ip::tcp::resolver resolver(*io_context_->getContext());
            auto endpoints = resolver.resolve(host, "80"); // 使用HTTP默认端口
            asio::connect(socket_, endpoints);
        }

        // 构建请求字符串
        std::string build_request(const std::string& method, const std::string& path, const std::string& body) {
            std::ostringstream request;
            request << method << " " << path << " HTTP/1.1\r\n";
            request << "Host: " << socket_.remote_endpoint().address().to_string() << "\r\n";

            // 添加请求头
            for (const auto& [key, value] : Headers) {
                request << key << ": " << value << "\r\n";
            }

            request << "Connection: close\r\n"; // 关闭连接
            request << "\r\n"; // 请求头结束

            // 添加请求体（如果有）
            if (!body.empty()) {
                request << body;
            }

            return request.str();
        }

        // 读取响应
        Response read_response() {
            std::string response_line;

            std::getline(asio::buffered_read_until(socket_, asio::dynamic_buffer(response_line), "\r\n"));

            // 解析状态码
            std::istringstream response_stream(response_line);
            std::string http_version;
            int status_code;
            response_stream >> http_version >> status_code;

            // 读取头部
            Header header = parse_headers();

            // 读取响应体
            std::string body;
            asio::read(socket_, asio::dynamic_buffer(body)); // 可以进一步处理 chunked 响应

            return Response{ static_cast<HttpStatusCode>(status_code), header, body };
        }
    };

} // namespace http_asio

#endif // HTTP_CLIENT_HPP