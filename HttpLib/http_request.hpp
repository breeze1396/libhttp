#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP



#include "http_types.hpp"
#include <string>
#include <unordered_map>
#include <optional>
#include <sstream>


namespace http_asio {

class Request {
public:
    HttpMethod Method = HttpMethod::UNKNOWN;
    std::string Path;
    Param Params;
    Header Headers;
    std::string Body;

    Request() = default;

	void setMethod(const std::string& method) {
		Method = stringToHttpMethod(method);
	}

    // 添加单个查询参数
    void addParam(const std::string& key, const std::string& value) {
        Params[key] = value;
    }

    // 获取单个查询参数值
    std::optional<std::string> getParamValue(const std::string& key) const {
        auto it = Params.find(key);
        if (it != Params.end()) {
            return it->second;
        }
        return std::nullopt;
    }


    // 添加单个头部字段
    void addHeader(const std::string& key, const std::string& value) {
        Headers[key] = value;
    }

    // 检查是否有某个头部字段
    bool hasHeader(const std::string& key) const {
        return Headers.find(key) != Headers.end();
    }

    // 获取头部字段的值
    std::optional<std::string> getHeaderValue(const std::string& key) const {
        auto it = Headers.find(key);
        if (it != Headers.end()) {
            return it->second;
        }
        return std::nullopt;
    }


    // 将请求转换为原始请求字符串（用于调试或发送到服务器）
    std::string toString() const {
        std::ostringstream oss;
        oss << methodToString(Method) << " " << Path;
        if (!Params.empty()) {
            oss << "?";
            for (const auto& [key, value] : Params) {
                oss << key << "=" << value << "&";
            }
            oss.seekp(-1, std::ios_base::end); // 移除最后一个 '&'
        }
        oss << " HTTP/1.1\r\n";

        for (const auto& [key, value] : Headers) {
            oss << key << ": " << value << "\r\n";
        }

        oss << "\r\n" << Body;
        return oss.str();
    }


    std::string methodToString(HttpMethod method) const {
        switch (method) {
        case HttpMethod::GET    : return "GET";
        case HttpMethod::POST   : return "POST";
        case HttpMethod::PUT    : return "PUT";
        case HttpMethod::DELETE : return "DELETE";
        case HttpMethod::PATCH  : return "PATCH";
        case HttpMethod::HEAD   : return "HEAD";
        case HttpMethod::OPTIONS: return "OPTIONS";
        case HttpMethod::TRACE  : return "TRACE";
        case HttpMethod::CONNECT: return "CONNECT";
        default                 : return "UNKNOWN";
        }
    }
};

} // namespace http_asio

#endif // HTTP_REQUEST_HPP