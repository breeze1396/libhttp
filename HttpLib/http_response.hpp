#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP


#include "http_types.hpp"
#include <string>
#include <unordered_map>
#include <optional>
#include <sstream>


namespace http_asio {
class Response {
public:
    HttpStatusCode StatusCode = HttpStatusCode::OK; // 默认状态码为200 OK
    std::string StatusMsg = "OK";               // 默认状态消息
	Header Headers;							// 头部字段
	std::string Body;						   // 响应体


    Response() = default;
	Response(HttpStatusCode code, const std::string& message)
		: StatusCode(code), StatusMsg(message) {}
    Response(HttpStatusCode code, Header head, std::string body)
        : Headers(head), Body(body) {}



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


    // 设置重定向
    void setRedirect(const std::string& location, HttpStatusCode code = HttpStatusCode::Found) {
		StatusCode = code;
        addHeader("Location", location);
		Body = "";
    }

    // 设置内容及其类型
    void setContent(const std::string& content, const std::string& content_type) {
        Body = content;
        addHeader("Content-Type", content_type);
    }

    // 将响应转换为原始响应字符串（用于调试或发送到客户端）
    std::string toString() const {
        std::ostringstream oss;
        oss << "HTTP/1.1 " << static_cast<int>(StatusCode) << " " << StatusMsg << "\r\n";

        for (const auto& [key, value] : Headers) {
            oss << key << ": " << value << "\r\n";
        }

        oss << "\r\n" << Body;
        return oss.str();
    }

};

} // namespace http_asio

#endif // HTTP_RESPONSE_HPP