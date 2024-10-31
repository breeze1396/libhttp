
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

    // ������Ⱥ����ݴ���ĺ�������
    using Progress = std::function<void(std::size_t, std::size_t)>;
    using ContentReceiver = std::function<void(const std::string&)>;

    // �ж�״̬��
    // �ɹ�
    inline bool isSuccessStatus(StatusCode status) {
        return static_cast<int>(status) >= 200 && static_cast<int>(status) < 300;
    }

    // �ض���
    inline bool isRedirectStatus(StatusCode status) {
        return static_cast<int>(status) >= 300 && static_cast<int>(status) < 400;
    }

    // �ͻ��˴���
    inline bool isClientErrorStatus(StatusCode status) {
        return static_cast<int>(status) >= 400 && static_cast<int>(status) < 500;
    }

    // ����������
    inline bool isServerErrorStatus(StatusCode status) {
        return static_cast<int>(status) >= 500 && static_cast<int>(status) < 600;
    }

    // URL ����
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

    // URL ����
    inline std::string url_decode(const std::string& value) {
        std::string decoded;
        for (size_t i = 0; i < value.size(); ++i) {
            if (value[i] == '%') {
                if (i + 2 < value.size()) {
                    std::string hex = value.substr(i + 1, 2);
                    char decoded_char = static_cast<char>(std::stoi(hex, nullptr, 16));
                    decoded += decoded_char;
                    i += 2;
                }
            }
            else {
                decoded += value[i];
            }
        }
        return decoded;
    }

    // ���� HTTP ͷ��
    inline Header parse_headers(const std::string& raw_headers) {
        Header headers;
        std::istringstream stream(raw_headers);
        std::string line;

        while (std::getline(stream, line) && !line.empty()) {
            size_t separator = line.find(':');
            if (separator != std::string::npos) {
                std::string key = line.substr(0, separator);
                std::string value = line.substr(separator + 1);
                // ȥ������ֵ�Ŀհ��ַ�
                key.erase(std::remove_if(key.begin(), key.end(), isspace), key.end());
                value.erase(std::remove_if(value.begin(), value.end(), isspace), value.end());
                headers[key] = value;
            }
        }
        return headers;
    }

    // �������� URL
    inline std::string join_url(const std::string& base, const std::string& path) {
        if (base.back() == '/' && path.front() == '/') {
            return base + path.substr(1);
        }
        else if (base.back() != '/' && path.front() != '/') {
            return base + '/' + path;
        }
        return base + path;
    }

    // ��ӡ����/��Ӧ����
    inline void print_content(const std::string& content) {
        std::cout << "Content: " << content << std::endl;
    }

    // ʾ�����ȱ���
    inline void report_progress(std::size_t current, std::size_t total) {
        std::cout << "Progress: " << (current * 100 / total) << "% completed." << std::endl;
    }

	inline void trim(std::string& str) {
		str.erase(0, str.find_first_not_of(" \t\n\r"));
		str.erase(str.find_last_not_of(" \t\n\r") + 1);
	}

    // Range����Ľṹ�嶨�壬��ʾRange�������ʼ�ͽ����ֽ�
    struct Range {
        std::optional<size_t> start;
        std::optional<size_t> end;

        std::string toString() const {
            std::ostringstream oss;
            oss << "bytes=";
            if (start) oss << *start;
            oss << "-";
            if (end) oss << *end;
            return oss.str();
        }
    };

} // namespace http_asio

#endif // HTTP_UTIL_HPP