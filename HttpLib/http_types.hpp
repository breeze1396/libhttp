#ifndef HTTP_TYPES_HPP
#define HTTP_TYPES_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include <optional>
#include <sstream>

namespace http_asio {

    // ����HTTPͷ�����ͣ�ʹ��unordered_map�洢ͷ���ֶκͶ�Ӧ��ֵ
    using Header = std::unordered_map<std::string, std::string>;

    // ��ѯ�������ͣ��洢��ֵ����ʽ�Ĳ�ѯ����
    using Param = std::unordered_map<std::string, std::string>;

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

    // HTTP������ö������
    enum class HttpMethod {
        GET,
        POST,
        PUT,
        DELETE,
        PATCH,
        HEAD,
        OPTIONS,
        TRACE,
        CONNECT,
        UNKNOWN
    };

    // ���ַ���ת��ΪHttpMethodö��
    inline HttpMethod stringToHttpMethod(const std::string& method) {
        if (method == "GET") return HttpMethod::GET;
        if (method == "POST") return HttpMethod::POST;
        if (method == "PUT") return HttpMethod::PUT;
        if (method == "DELETE") return HttpMethod::DELETE;
        if (method == "PATCH") return HttpMethod::PATCH;
        if (method == "HEAD") return HttpMethod::HEAD;
        if (method == "OPTIONS") return HttpMethod::OPTIONS;
        if (method == "TRACE") return HttpMethod::TRACE;
        if (method == "CONNECT") return HttpMethod::CONNECT;
        return HttpMethod::UNKNOWN;
    }

    // HTTP״̬���ö������
    enum class HttpStatusCode {
        Continue = 100,
        SwitchingProtocols = 101,
        OK = 200,
        Created = 201,
        Accepted = 202,
        NoContent = 204,
        MovedPermanently = 301,
        Found = 302,
        NotModified = 304,
        BadRequest = 400,
        Unauthorized = 401,
        Forbidden = 403,
        NotFound = 404,
        MethodNotAllowed = 405,
        RequestTimeout = 408,
        Conflict = 409,
        Gone = 410,
        PayloadTooLarge = 413,
        URITooLong = 414,
        UnsupportedMediaType = 415,
        RangeNotSatisfiable = 416,
        InternalServerError = 500,
        NotImplemented = 501,
        BadGateway = 502,
        ServiceUnavailable = 503,
        GatewayTimeout = 504,
        HTTPVersionNotSupported = 505,
        Unknown = 0
    };

    // ��HttpStatusCodeת��Ϊ��Ӧ���ַ�������
    inline std::string statusCodeToString(HttpStatusCode status) {
        switch (status) {
        case HttpStatusCode::Continue: return "100 Continue";
        case HttpStatusCode::SwitchingProtocols: return "101 Switching Protocols";
        case HttpStatusCode::OK: return "200 OK";
        case HttpStatusCode::Created: return "201 Created";
        case HttpStatusCode::Accepted: return "202 Accepted";
        case HttpStatusCode::NoContent: return "204 No Content";
        case HttpStatusCode::MovedPermanently: return "301 Moved Permanently";
        case HttpStatusCode::Found: return "302 Found";
        case HttpStatusCode::NotModified: return "304 Not Modified";
        case HttpStatusCode::BadRequest: return "400 Bad Request";
        case HttpStatusCode::Unauthorized: return "401 Unauthorized";
        case HttpStatusCode::Forbidden: return "403 Forbidden";
        case HttpStatusCode::NotFound: return "404 Not Found";
        case HttpStatusCode::MethodNotAllowed: return "405 Method Not Allowed";
        case HttpStatusCode::RequestTimeout: return "408 Request Timeout";
        case HttpStatusCode::Conflict: return "409 Conflict";
        case HttpStatusCode::Gone: return "410 Gone";
        case HttpStatusCode::PayloadTooLarge: return "413 Payload Too Large";
        case HttpStatusCode::URITooLong: return "414 URI Too Long";
        case HttpStatusCode::UnsupportedMediaType: return "415 Unsupported Media Type";
        case HttpStatusCode::RangeNotSatisfiable: return "416 Range Not Satisfiable";
        case HttpStatusCode::InternalServerError: return "500 Internal Server Error";
        case HttpStatusCode::NotImplemented: return "501 Not Implemented";
        case HttpStatusCode::BadGateway: return "502 Bad Gateway";
        case HttpStatusCode::ServiceUnavailable: return "503 Service Unavailable";
        case HttpStatusCode::GatewayTimeout: return "504 Gateway Timeout";
        case HttpStatusCode::HTTPVersionNotSupported: return "505 HTTP Version Not Supported";
        default: return "0 Unknown";
        }
    }

    // �ж�״̬���Ƿ��ǳɹ���״̬��
    inline bool isSuccessStatus(HttpStatusCode status) {
        return static_cast<int>(status) >= 200 && static_cast<int>(status) < 300;
    }

    // �ж�״̬���Ƿ����ض����״̬��
    inline bool isRedirectStatus(HttpStatusCode status) {
        return static_cast<int>(status) >= 300 && static_cast<int>(status) < 400;
    }

    // �ж�״̬���Ƿ��ǿͻ��˴����״̬��
    inline bool isClientErrorStatus(HttpStatusCode status) {
        return static_cast<int>(status) >= 400 && static_cast<int>(status) < 500;
    }

    // �ж�״̬���Ƿ��Ƿ����������״̬��
    inline bool isServerErrorStatus(HttpStatusCode status) {
        return static_cast<int>(status) >= 500 && static_cast<int>(status) < 600;
    }

    // ������Content-Type����
    namespace MimeType {
        const std::string TEXT_HTML = "text/html";
        const std::string TEXT_PLAIN = "text/plain";
        const std::string APPLICATION_JSON = "application/json";
        const std::string APPLICATION_XML = "application/xml";
        const std::string MULTIPART_FORM_DATA = "multipart/form-data";
        const std::string APPLICATION_OCTET_STREAM = "application/octet-stream";
        const std::string IMAGE_PNG = "image/png";
        const std::string IMAGE_JPEG = "image/jpeg";
    }

    // URL���뺯��
    inline std::string urlDecode(const std::string& str) {
        std::string result;
        result.reserve(str.size());
        for (size_t i = 0; i < str.size(); ++i) {
            if (str[i] == '%' && i + 2 < str.size()) {
                int value = 0;
                std::istringstream iss(str.substr(i + 1, 2));
                if (iss >> std::hex >> value) {
                    result += static_cast<char>(value);
                    i += 2;
                }
            }
            else if (str[i] == '+') {
                result += ' ';
            }
            else {
                result += str[i];
            }
        }
        return result;
    }

} // namespace http_asio

#endif // HTTP_TYPES_HPP