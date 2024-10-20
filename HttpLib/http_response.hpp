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
    HttpStatusCode StatusCode = HttpStatusCode::OK; // Ĭ��״̬��Ϊ200 OK
    std::string StatusMsg = "OK";               // Ĭ��״̬��Ϣ
	Header Headers;							// ͷ���ֶ�
	std::string Body;						   // ��Ӧ��


    Response() = default;
	Response(HttpStatusCode code, const std::string& message)
		: StatusCode(code), StatusMsg(message) {}
    Response(HttpStatusCode code, Header head, std::string body)
        : Headers(head), Body(body) {}



    // ��ӵ���ͷ���ֶ�
    void addHeader(const std::string& key, const std::string& value) {
        Headers[key] = value;
    }

    // ����Ƿ���ĳ��ͷ���ֶ�
    bool hasHeader(const std::string& key) const {
        return Headers.find(key) != Headers.end();
    }

    // ��ȡͷ���ֶε�ֵ
    std::optional<std::string> getHeaderValue(const std::string& key) const {
        auto it = Headers.find(key);
        if (it != Headers.end()) {
            return it->second;
        }
        return std::nullopt;
    }


    // �����ض���
    void setRedirect(const std::string& location, HttpStatusCode code = HttpStatusCode::Found) {
		StatusCode = code;
        addHeader("Location", location);
		Body = "";
    }

    // �������ݼ�������
    void setContent(const std::string& content, const std::string& content_type) {
        Body = content;
        addHeader("Content-Type", content_type);
    }

    // ����Ӧת��Ϊԭʼ��Ӧ�ַ��������ڵ��Ի��͵��ͻ��ˣ�
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