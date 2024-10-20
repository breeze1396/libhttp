
#ifndef HTTP_UTIL_HPP
#define HTTP_UTIL_HPP

#include "http_types.hpp"
#include <string>
#include <sstream>
#include <unordered_map>
#include <functional>
#include <iomanip>
#include <cctype>
#include <algorithm>
#include <stdexcept>
#include <iostream>


namespace http_asio {

    // 定义进度和内容处理的函数类型
    using Progress = std::function<void(std::size_t, std::size_t)>;
    using ContentReceiver = std::function<void(const std::string&)>;

    // URL 编码
    inline std::string url_encode(const std::string& value) {
        std::ostringstream escaped;
        escaped << std::hex << std::setfill('0');

        for (char c : value) {
            if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
                escaped << c;
            }
            else {
                escaped << '%' << std::uppercase << std::setw(2) << static_cast<int>(static_cast<unsigned char>(c));
            }
        }
        return escaped.str();
    }

    // URL 解码
    inline std::string url_decode(const std::string& value) {
        std::string decoded;
        for (size_t i = 0; i < value.size(); ++i) {
            if (value[i] == '%') {
                if (i + 2 < value.size()) {
                    std::string hex = value.substr(i + 1, 2);
                    char decoded_char = static_cast<char>(std::stoi(hex, nullptr, 16));
                    decoded += decoded_char;
                    i += 2; // Skip past the hex digits
                }
            }
            else {
                decoded += value[i];
            }
        }
        return decoded;
    }

    // 解析 HTTP 头部
    inline Header parse_headers(const std::string& raw_headers) {
        Header headers;
        std::istringstream stream(raw_headers);
        std::string line;

        while (std::getline(stream, line) && !line.empty()) {
            size_t separator = line.find(':');
            if (separator != std::string::npos) {
                std::string key = line.substr(0, separator);
                std::string value = line.substr(separator + 1);
                // 去除键和值的空白字符
                key.erase(std::remove_if(key.begin(), key.end(), isspace), key.end());
                value.erase(std::remove_if(value.begin(), value.end(), isspace), value.end());
                headers[key] = value;
            }
        }
        return headers;
    }

    // 连接两个 URL
    inline std::string join_url(const std::string& base, const std::string& path) {
        if (base.back() == '/' && path.front() == '/') {
            return base + path.substr(1);
        }
        else if (base.back() != '/' && path.front() != '/') {
            return base + '/' + path;
        }
        return base + path;
    }

    // 打印请求/响应内容
    inline void print_content(const std::string& content) {
        std::cout << "Content: " << content << std::endl;
    }

    // 示例进度报告
    inline void report_progress(std::size_t current, std::size_t total) {
        std::cout << "Progress: " << (current * 100 / total) << "% completed." << std::endl;
    }

	inline void trim(std::string& str) {
		str.erase(0, str.find_first_not_of(" \t\n\r"));
		str.erase(str.find_last_not_of(" \t\n\r") + 1);
	}

} // namespace http_asio

#endif // HTTP_UTIL_HPP