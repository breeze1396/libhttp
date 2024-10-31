#include "http_server.hpp"
#include "temp.h"
#include "EasyTest.hpp"
#include "Json.hpp"
#include "http_request.hpp"
#include "http_response.hpp"
#include "http_client.hpp"
#include <asio.hpp>
#include <thread>
#include <chrono>
#include <iostream>
#include <asio/ssl.hpp>
using namespace std;
using namespace asio::ip;


void testServer() {
	http_asio::Server server(8080);
	server.Get("/", [](const http_asio::Request& req, http_asio::Response& res) {
        auto t = std::chrono::system_clock::now().time_since_epoch().count();
        res.setContent("Hello World! " + std::to_string(t), "text/plain");
    });

    server.Post("/", [](const http_asio::Request& req, http_asio::Response& res) {
		auto t = std::chrono::system_clock::now().time_since_epoch().count();
		res.setContent("Hello World! " + std::to_string(t), "text/plain");
    });

    server.Run();
}

std::vector<std::thread> vec_threads;
void testSerPerformance() {
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 16; i++) {
		int id = i;
        vec_threads.emplace_back([id]() {
            try
            {
                asio::io_context ioc;
                
                std::string send_data = "GET / HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: close\r\n\r\n";
                int i = 0;
                while (i < 1000) {
                    tcp::socket sock(ioc);

                    tcp::endpoint remote_ep(address::from_string("127.0.0.1"), 8080);
                   

                    //sock.set_option(asio::socket_base::reuse_address(true));
                    sock.connect(remote_ep);
                    sock.set_option(asio::ip::tcp::socket::reuse_address(true));
                    asio::write(sock, asio::buffer(send_data, send_data.size()));

                    asio::streambuf response_buf;
                    asio::read_until(sock, response_buf, "\r\n\r\n");

                    std::istream response_stream(&response_buf);
                    std::string header_line;
                    while (std::getline(response_stream, header_line) && header_line != "\r");
                    

                    std::ostringstream body_stream;
                    body_stream << &response_buf;
                    //std::cout << "Body content: " << body_stream.str() << std::endl;

                    if (response_stream.rdbuf()->in_avail() > 0) {
                        //std::cout << response_stream.rdbuf(); 
                    }
                    i++;
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
            }
            catch (std::exception& e) {
                std::cerr << id <<" Exception: " << e.what() << std::endl;
            }
            });
        //std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    for (auto& t : vec_threads) {
        t.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);
    std::cout << "Time spent: " << duration.count() << " seconds." << std::endl;
}

void testUseClient() {
    http_asio::Client client("127.0.0.1:8080");
    auto t = client.Get("/");

	t.wait_for(std::chrono::seconds(10));
	if (t.valid()) {
		auto res = t.get();
		std::cout << "Response: " << res.Body << std::endl;
	}
	
}

void handle_post(const httplib::Request& req, httplib::Response& res) {
    // 获取请求体数据
    std::string body = req.body;

    // 在服务器端打印请求体数据
    std::cout << "Received POST request with body: " << body << std::endl;

    // 设置响应内容
    res.set_content("POST request received", "application/json");
}
int server() {
    httplib::Server svr;

    svr.Get("/", [](const httplib::Request& req, httplib::Response& res) {
        res.set_content("Hello, World!", "text/plain");
        });

    svr.Get("/about", [](const httplib::Request& req, httplib::Response& res) {
        res.set_content("About page", "text/plain");
        });

    svr.Post("/api/users", handle_post);

    std::cout << "Server started on port 8080" << std::endl;
    svr.listen("localhost", 8080);

    return 0;
}
int client() {
    httplib::Client cli("localhost", 8080);

    // 发送GET请求
    auto res = cli.Get("/");
    if (res && res->status == 200) {
        std::cout << res->body << std::endl;
    }

    res = cli.Get("/about");
    if (res && res->status == 200) {
        std::cout << res->body << std::endl;
    }

    // 发送POST请求
    httplib::Headers headers = {
        { "Content-Type", "application/json" }  // 设置请求头 MIME类型
    };

    std::string body = R"({"name": "John", "age": 30})";  // 请求体数据

    auto res2 = cli.Post("/api/users", headers, body, "application/json");

    if (res2 && res2->status == 200) {
        std::cout << "Request successful: " << res2->body << std::endl;
    }
    else {
        std::cout << "Request failed!" << std::endl;
    }

    return 0;
}
void test1() {
    // 启动服务器
    std::thread ser(server);
    std::thread cli(client);
    cli.join();
    ser.join();
}



void testAsioSsl() {
    using namespace asio;
	io_context service;
    typedef ssl::stream<ip::tcp::socket> ssl_socket;
    ssl::context ctx(ssl::context::sslv23);
    ctx.set_default_verify_paths();
    // 打开一个到指定主机的SSL socket

    ssl_socket sock(service, ctx);
    ip::tcp::resolver resolver(service);
    std::string host = "www.baidu.com";
    ip::tcp::resolver::query query(host, "https");
    connect(sock.lowest_layer(), resolver.resolve(query));
    // SSL 握手
    sock.set_verify_mode(ssl::verify_none);
    sock.set_verify_callback(ssl::rfc2818_verification(host));
    sock.handshake(ssl_socket::client);
    std::string req = "GET /index.html HTTP/2.0\r\nHost: " + host + "\r\nAccept: */*\r\nConnection: close\r\n\r\n";
    write(sock, buffer(req.c_str(), req.length()));
    char buff[512];
    asio::error_code ec;
    while (!ec) {
        int bytes = read(sock, buffer(buff), ec);
        std::cout << std::string(buff, bytes);
    }

}

int main() {
	try {
		//test1();
		//testSerPerformance();
        //testAsioSsl();
        http_asio::Response r;
		r.hasHeader("Content-Type");
	}
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    //std::thread ser(testServer);
 //   std::thread cli(testUseClient);
 //   cli.join();
    //testUseClient();

    //ser.join();
    getchar();
	return 0;
}

