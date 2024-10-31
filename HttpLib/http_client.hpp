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

        // ����ͷ
        void set_header(const std::string& key, const std::string& value) {
            Headers[key] = value;
        }

        // ���ó�ʱ
        void set_timeout(std::chrono::seconds timeout) {
            timeout_ = timeout;
        }

        // ���� GET ����
        std::future<Response> Get(std::string path) {
			path_ = path;
            return send_request("GET");
        }

        // ���� POST ����
        std::future<Response> Post(const std::string& body) {
            set_header("Content-Length", std::to_string(body.size()));
            return send_request("POST");
        }

        // ���� PUT ����
        std::future<Response> Put(const std::string& url, const std::string& body) {
            set_header("Content-Length", std::to_string(body.size()));
            return send_request("PUT", url);
        }

        // ���� DELETE ����
        std::future<Response> Del() {
            return send_request("DELETE");
        }

		// ���� OPTIONS ����
		std::future<Response> Options() {
			return send_request("OPTIONS");
		}

		// ���� PATCH ����
		std::future<Response> Patch(const std::string& body) {
			set_header("Content-Length", std::to_string(body.size()));
			return send_request("PATCH",  body);
		}

        

    private:
        std::shared_ptr<IOContextWrapper> io_context_;
        asio::ip::tcp::socket socket_;
        Header Headers;
        std::chrono::seconds timeout_{ 5 }; // Ĭ�ϳ�ʱΪ5��
		std::string host_;
		std::string path_;
		std::string port_ = "80";

        // ���������ͨ�÷���
        std::future<Response> send_request(const std::string& method, const std::string& body = "") {
            return std::async(std::launch::async, [this, method, body]() {
                try {
                    // ���ӷ�����
                    connect_to_server();

                    // ���������ַ���
                    std::string request = build_request(method, body);

                    // ��������
                    asio::write(socket_, asio::buffer(request));

                    // ������Ӧ
                    Response response = read_response();

                    // �ر�����
                    socket_.close();
                    return response;

                } catch (const std::exception& e) {
                    std::cerr << "Error: " << e.what() << std::endl;
                    return Response{ StatusCode::InternalServerError, "Internal Server Error" };
                }
                });
        }

        // ����URL��
		// �磺http://www.example.com:8080 => www.example.com, 8080
        // ��: www.example.com:8080 => www.example.com", 8080,
		// �磺127.0.0.1:8080 => 127.0.0.1,8080
        void parse_url(const std::string& url) {
            size_t pos = url.find("http://");
            if (pos != std::string::npos) {
                pos += 7;
            } else {
				pos = 0;
			}
            size_t colon = url.find(":", pos);
			host_ = url.substr(pos, colon - pos);
            if (colon == std::string::npos) { // Ĭ�϶˿� ��80��
				port_ = "80";
			}
            else {
				port_ = url.substr(colon+1);
            }
        }

        // ���ӵ�������
        void connect_to_server() {
            asio::ip::tcp::resolver resolver(*io_context_->getContext());
			// ���hostΪip��ַ����ֱ�����ӣ������������
			if (host_.find_first_of("0123456789.") != std::string::npos) {
				asio::ip::tcp::endpoint endpoint(asio::ip::make_address(host_), std::stoi(port_));
				socket_.connect(endpoint);
				return;
			}
            auto endpoints = resolver.resolve(host_, port_);
            asio::connect(socket_, endpoints.begin());
        }

        // ���������ַ���
        std::string build_request(const std::string& method, const std::string& body) {
            std::ostringstream request;
            request << method << " " << path_ << " HTTP/1.1\r\n";
            request << "Host: " << socket_.remote_endpoint().address().to_string() << "\r\n";

            // �������ͷ
            for (const auto& [key, value] : Headers) {
                request << key << ": " << value << "\r\n";
            }

            request << "Connection: close\r\n"; // �ر�
            request << "\r\n"; // ����ͷ����

            // ��������壨����У�
            if (!body.empty()) {
                request << body;
            }

            return request.str();
        }

        // ��ȡ��Ӧ
        Response read_response() {
            std::string response;

            asio::read_until(socket_, asio::dynamic_buffer(response), "\r\n");

            // ����״̬��
            std::istringstream response_stream(response);
            std::string http_version;
            int status_code;
            response_stream >> http_version >> status_code;

            // ��ȡͷ��
            Header header = parse_headers(response);

            // ��ȡ��Ӧ��
            std::string body;
			body = asio::buffer_cast<const char*>(asio::dynamic_buffer(response).data());
            return Response{ static_cast<StatusCode>(status_code), header, body };
        }
    };

} // namespace http_asio

#endif // HTTP_CLIENT_HPP