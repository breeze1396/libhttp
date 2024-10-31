#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include "http_types.hpp"
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <optional>
#include <sstream>

namespace http_asio {

    class Response {
    public:
        StatusCode StatCde = StatusCode::OK;        // 默认状态码为200 OK
        std::string StatusMsg = "OK";               // 默认状态消息
        Header Headers;                             // 头部字段
        std::string Body;                           // 响应体

        Response() = default;
        Response(StatusCode code, const std::string& message)
            : StatCde(code), StatusMsg(message) {}
        Response(StatusCode code, Header head, std::string body)
            : StatCde(code), Headers(head), Body(body) {}

        // 添加单个头部字段
        void SetHeader(const std::string& key, const std::string& value) {
            Headers[key] = value;
        }

        // 检查是否存在某个头部字段
        bool HasHeader(const std::string& key) const {
            return Headers.find(key) != Headers.end();
        }

        // 获取头部字段的值
        std::optional<std::string> GetHeaderValue(const std::string& key) const {
            auto it = Headers.find(key);
            return it != Headers.end() ? std::make_optional(it->second) : std::nullopt;
        }

        // 设置重定向
        void SetRedirect(const std::string& location, StatusCode code = StatusCode::Found) {
            StatCde = code;
            SetHeader("Location", location);
            Body.clear(); // 重定向通常不需要主体内容
        }

        // 设置响应内容及内容类型
        void SetContent(const std::string& content, const std::string& content_type) {
            Body = content;
            SetHeader("Content-Type", content_type);
            SetHeader("Content-Length", std::to_string(content.size()));
        }

        // 设置状态码和状态消息
        void SetStatus(StatusCode code, const std::string& message = "") {
            StatCde = code;
            StatusMsg = message;
        }

        // 添加Cookie
        void addCookie(const std::string& name, const std::string& value,
            const std::optional<std::string>& path = std::nullopt,
            const std::optional<std::string>& domain = std::nullopt,
            bool http_only = true, bool secure = false) {
            std::ostringstream cookie;
            cookie << name << "=" << value;
            if (path) cookie << "; Path=" << *path;
            if (domain) cookie << "; Domain=" << *domain;
            if (http_only) cookie << "; HttpOnly";
            if (secure) cookie << "; Secure";
            SetHeader("Set-Cookie", cookie.str());
        }

        // 清除某个头部字段
        void removeHeader(const std::string& key) {
            Headers.erase(key);
        }

        // 设置允许跨域请求
        void enableCORS(const std::string& origin = "*", const std::string& methods = "GET, POST, PUT, DELETE") {
            SetHeader("Access-Control-Allow-Origin", origin);
            SetHeader("Access-Control-Allow-Methods", methods);
        }

        // 设置压缩内容 接口
        void setCompressedContent(const std::string& compressed_data, const std::string& compression_type) {
            Body = compressed_data;
            SetHeader("Content-Encoding", compression_type);
            SetHeader("Content-Length", std::to_string(compressed_data.size()));
        }

        // 将响应转换为原始响应字符串
        std::string toString() const {
            std::ostringstream oss;
            oss << "HTTP/1.1 " << static_cast<int>(StatCde) << " " << StatusMsg << "\r\n";

            for (const auto& [key, value] : Headers) {
                oss << key << ": " << value << "\r\n";
            }

            oss << "\r\n" << Body;
            return oss.str();
        }

    private:
        std::string formatHeaderValue(const std::string& value) const {
            std::string formatted_value = value;
            // todo: 实现对value的清理或转义逻辑
            return formatted_value;
        }

        size_t contentLength_ = 0;
        ContentProvider contentProvider_;
        std::function<void()> contentProviderResourceReleaser_;
    };

} // namespace http_asio

#endif // HTTP_RESPONSE_HPP
