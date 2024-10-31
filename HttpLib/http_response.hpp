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
        StatusCode StatCde = StatusCode::OK;        // Ĭ��״̬��Ϊ200 OK
        std::string StatusMsg = "OK";               // Ĭ��״̬��Ϣ
        Header Headers;                             // ͷ���ֶ�
        std::string Body;                           // ��Ӧ��

        Response() = default;
        Response(StatusCode code, const std::string& message)
            : StatCde(code), StatusMsg(message) {}
        Response(StatusCode code, Header head, std::string body)
            : StatCde(code), Headers(head), Body(body) {}

        // ��ӵ���ͷ���ֶ�
        void SetHeader(const std::string& key, const std::string& value) {
            Headers[key] = value;
        }

        // ����Ƿ����ĳ��ͷ���ֶ�
        bool HasHeader(const std::string& key) const {
            return Headers.find(key) != Headers.end();
        }

        // ��ȡͷ���ֶε�ֵ
        std::optional<std::string> GetHeaderValue(const std::string& key) const {
            auto it = Headers.find(key);
            return it != Headers.end() ? std::make_optional(it->second) : std::nullopt;
        }

        // �����ض���
        void SetRedirect(const std::string& location, StatusCode code = StatusCode::Found) {
            StatCde = code;
            SetHeader("Location", location);
            Body.clear(); // �ض���ͨ������Ҫ��������
        }

        // ������Ӧ���ݼ���������
        void SetContent(const std::string& content, const std::string& content_type) {
            Body = content;
            SetHeader("Content-Type", content_type);
            SetHeader("Content-Length", std::to_string(content.size()));
        }

        // ����״̬���״̬��Ϣ
        void SetStatus(StatusCode code, const std::string& message = "") {
            StatCde = code;
            StatusMsg = message;
        }

        // ���Cookie
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

        // ���ĳ��ͷ���ֶ�
        void removeHeader(const std::string& key) {
            Headers.erase(key);
        }

        // ���������������
        void enableCORS(const std::string& origin = "*", const std::string& methods = "GET, POST, PUT, DELETE") {
            SetHeader("Access-Control-Allow-Origin", origin);
            SetHeader("Access-Control-Allow-Methods", methods);
        }

        // ����ѹ������ �ӿ�
        void setCompressedContent(const std::string& compressed_data, const std::string& compression_type) {
            Body = compressed_data;
            SetHeader("Content-Encoding", compression_type);
            SetHeader("Content-Length", std::to_string(compressed_data.size()));
        }

        // ����Ӧת��Ϊԭʼ��Ӧ�ַ���
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
            // todo: ʵ�ֶ�value�������ת���߼�
            return formatted_value;
        }

        size_t contentLength_ = 0;
        ContentProvider contentProvider_;
        std::function<void()> contentProviderResourceReleaser_;
    };

} // namespace http_asio

#endif // HTTP_RESPONSE_HPP
