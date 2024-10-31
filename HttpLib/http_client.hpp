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
        Client(std::string url, std::shared_ptr<IOContextWrapper> io_context = std::make_shared<IOContextWrapper>())
            : io_context_(io_context), socket_(*io_context->getContext()) {
			parse_url(url);
        }

		void SetUrl(const std::string& url) {
            parse_url(url);
		}

        // 请求头
        void set_header(const std::string& key, const std::string& value) {
            Headers[key] = value;
        }

        // 设置超时
        void set_timeout(std::chrono::seconds timeout) {
            timeout_ = timeout;
        }

        // 发送 GET 请求
        std::future<Response> Get(std::string path) {
			path_ = path;
            return send_request("GET");
        }

        // 发送 POST 请求
        std::future<Response> Post(const std::string& body) {
            set_header("Content-Length", std::to_string(body.size()));
            return send_request("POST");
        }

        // 发送 PUT 请求
        std::future<Response> Put(const std::string& url, const std::string& body) {
            set_header("Content-Length", std::to_string(body.size()));
            return send_request("PUT", url);
        }

        // 发送 DELETE 请求
        std::future<Response> Del() {
            return send_request("DELETE");
        }

		// 发送 OPTIONS 请求
		std::future<Response> Options() {
			return send_request("OPTIONS");
		}

		// 发送 PATCH 请求
		std::future<Response> Patch(const std::string& body) {
			set_header("Content-Length", std::to_string(body.size()));
			return send_request("PATCH",  body);
		}

        

    private:
        std::shared_ptr<IOContextWrapper> io_context_;
        asio::ip::tcp::socket socket_;
        Header Headers;
        std::chrono::seconds timeout_{ 5 }; // 默认超时为5秒
		std::string host_;
		std::string path_;
		std::string port_ = "80";

        // 发送请求的通用方法
        std::future<Response> send_request(const std::string& method, const std::string& body = "") {
            return std::async(std::launch::async, [this, method, body]() {
                try {
                    // 连接服务器
                    connect_to_server();

                    // 创建请求字符串
                    std::string request = build_request(method, body);

                    // 发送请求
                    asio::write(socket_, asio::buffer(request));

                    // 接收响应
                    Response response = read_response();

                    // 关闭连接
                    socket_.close();
                    return response;

                } catch (const std::exception& e) {
                    std::cerr << "Error: " << e.what() << std::endl;
                    return Response{ StatusCode::InternalServerError, "Internal Server Error" };
                }
                });
        }

        // 解析URL，
		// 如：http://www.example.com:8080 => www.example.com, 8080
        // 如: www.example.com:8080 => www.example.com", 8080,
		// 如：127.0.0.1:8080 => 127.0.0.1,8080
        void parse_url(const std::string& url) {
            size_t pos = url.find("http://");
            if (pos != std::string::npos) {
                pos += 7;
            } else {
				pos = 0;
			}
            size_t colon = url.find(":", pos);
			host_ = url.substr(pos, colon - pos);
            if (colon == std::string::npos) { // 默认端口 “80”
				port_ = "80";
			}
            else {
				port_ = url.substr(colon+1);
            }
        }

        // 连接到服务器
        void connect_to_server() {
            asio::ip::tcp::resolver resolver(*io_context_->getContext());
			// 如果host为ip地址，则直接连接，否则解析域名
			if (host_.find_first_of("0123456789.") != std::string::npos) {
				asio::ip::tcp::endpoint endpoint(asio::ip::make_address(host_), std::stoi(port_));
				socket_.connect(endpoint);
				return;
			}
            auto endpoints = resolver.resolve(host_, port_);
            asio::connect(socket_, endpoints.begin());
        }

        // 构建请求字符串
        std::string build_request(const std::string& method, const std::string& body) {
            std::ostringstream request;
            request << method << " " << path_ << " HTTP/1.1\r\n";
            request << "Host: " << socket_.remote_endpoint().address().to_string() << "\r\n";

            // 添加请求头
            for (const auto& [key, value] : Headers) {
                request << key << ": " << value << "\r\n";
            }

            request << "Connection: close\r\n"; // 关闭
            request << "\r\n"; // 请求头结束

            // 添加请求体（如果有）
            if (!body.empty()) {
                request << body;
            }

            return request.str();
        }

        // 读取响应
        Response read_response() {
            std::string response;

            asio::read_until(socket_, asio::dynamic_buffer(response), "\r\n");

            // 解析状态码
            std::istringstream response_stream(response);
            std::string http_version;
            int status_code;
            response_stream >> http_version >> status_code;

            // 读取头部
            Header header = parse_headers(response);

            // 读取响应体
            std::string body;
			body = asio::buffer_cast<const char*>(asio::dynamic_buffer(response).data());
            return Response{ static_cast<StatusCode>(status_code), header, body };
        }
    };

} // namespace http_asio

#endif // HTTP_CLIENT_HPP