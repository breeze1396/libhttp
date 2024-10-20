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

        // ��������ͷ
        void set_header(const std::string& key, const std::string& value) {
            Headers[key] = value;
        }

        // ���ó�ʱ
        void set_timeout(std::chrono::seconds timeout) {
            timeout_ = timeout;
        }

        // ���� GET ����
        std::future<Response> get(const std::string& url) {
            return send_request("GET", url);
        }

        // ���� POST ����
        std::future<Response> post(const std::string& url, const std::string& body) {
            set_header("Content-Length", std::to_string(body.size()));
            return send_request("POST", url, body);
        }

        // ���� PUT ����
        std::future<Response> put(const std::string& url, const std::string& body) {
            set_header("Content-Length", std::to_string(body.size()));
            return send_request("PUT", url, body);
        }

        // ���� DELETE ����
        std::future<Response> del(const std::string& url) {
            return send_request("DELETE", url);
        }

    private:
        std::shared_ptr<IOContextWrapper> io_context_;
        asio::ip::tcp::socket socket_;
        std::unordered_map<std::string, std::string> Headers;
        std::chrono::seconds timeout_{ 5 }; // Ĭ�ϳ�ʱΪ5��

        // ���������ͨ�÷���
        std::future<Response> send_request(const std::string& method, const std::string& url, const std::string& body = "") {
            return std::async(std::launch::async, [this, method, url, body]() {
                try {
                    // ����URL
                    auto [host, path] = parse_url(url);

                    // ���ӷ�����
                    connect_to_server(host);

                    // ���������ַ���
                    std::string request = build_request(method, path, body);

                    // ��������
                    asio::write(socket_, asio::buffer(request));

                    // ������Ӧ
                    Response response = read_response();

                    // �ر�����
                    socket_.close();
                    return response;

                } catch (const std::exception& e) {
                    std::cerr << "Error: " << e.what() << std::endl;
                    return Response{ HttpStatusCode::InternalServerError, "Internal Server Error" };
                }
                });
        }

        // ����URL��������������·��
        std::pair<std::string, std::string> parse_url(const std::string& url) {
            std::string host, path;
            size_t pos = url.find("://");
            if (pos != std::string::npos) {
                size_t start = pos + 3; // ����Э��
                size_t end = url.find('/', start);
                host = url.substr(start, end - start);
                path = end == std::string::npos ? "/" : url.substr(end);
            }
            return { host, path };
        }

        // ���ӵ�������
        void connect_to_server(const std::string& host) {
            asio::ip::tcp::resolver resolver(*io_context_->getContext());
            auto endpoints = resolver.resolve(host, "80"); // ʹ��HTTPĬ�϶˿�
            asio::connect(socket_, endpoints);
        }

        // ���������ַ���
        std::string build_request(const std::string& method, const std::string& path, const std::string& body) {
            std::ostringstream request;
            request << method << " " << path << " HTTP/1.1\r\n";
            request << "Host: " << socket_.remote_endpoint().address().to_string() << "\r\n";

            // �������ͷ
            for (const auto& [key, value] : Headers) {
                request << key << ": " << value << "\r\n";
            }

            request << "Connection: close\r\n"; // �ر�����
            request << "\r\n"; // ����ͷ����

            // ��������壨����У�
            if (!body.empty()) {
                request << body;
            }

            return request.str();
        }

        // ��ȡ��Ӧ
        Response read_response() {
            std::string response_line;

            std::getline(asio::buffered_read_until(socket_, asio::dynamic_buffer(response_line), "\r\n"));

            // ����״̬��
            std::istringstream response_stream(response_line);
            std::string http_version;
            int status_code;
            response_stream >> http_version >> status_code;

            // ��ȡͷ��
            Header header = parse_headers();

            // ��ȡ��Ӧ��
            std::string body;
            asio::read(socket_, asio::dynamic_buffer(body)); // ���Խ�һ������ chunked ��Ӧ

            return Response{ static_cast<HttpStatusCode>(status_code), header, body };
        }
    };

} // namespace http_asio

#endif // HTTP_CLIENT_HPP