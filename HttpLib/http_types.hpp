#ifndef HTTP_TYPES_HPP
#define HTTP_TYPES_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include <optional>
#include <sstream>

namespace http_asio {

    // 定义HTTP头部类型，使用unordered_map存储头部字段和对应的值
    using Header = std::unordered_map<std::string, std::string>;

    // 查询参数类型，存储键值对形式的查询参数
    using Param = std::unordered_map<std::string, std::string>;

    enum class HttpMethod {
        GET,
        POST,
        PUT,
        DEL,
        PATCH,
        HEAD,
        OPTIONS,
        TRACE,
        CONNECT,
        UNKNOWN
    };

    // 将字符串转换为HttpMethod枚举
    inline HttpMethod stringToHttpMethod(const std::string& method) {
        if (method == "GET")     return HttpMethod::GET;
        if (method == "POST")    return HttpMethod::POST;
        if (method == "PUT")     return HttpMethod::PUT;
        if (method == "DELETE")  return HttpMethod::DEL;
        if (method == "PATCH")   return HttpMethod::PATCH;
        if (method == "HEAD")    return HttpMethod::HEAD;
        if (method == "OPTIONS") return HttpMethod::OPTIONS;
        if (method == "TRACE")   return HttpMethod::TRACE;
        if (method == "CONNECT") return HttpMethod::CONNECT;
        return HttpMethod::UNKNOWN;
    }

    std::string methodToString(HttpMethod method) {
        switch (method) {
        case HttpMethod::GET: return "GET";
        case HttpMethod::POST: return "POST";
        case HttpMethod::PUT: return "PUT";
        case HttpMethod::DEL: return "DELETE";
        case HttpMethod::PATCH: return "PATCH";
        case HttpMethod::HEAD: return "HEAD";
        case HttpMethod::OPTIONS: return "OPTIONS";
        case HttpMethod::TRACE: return "TRACE";
        case HttpMethod::CONNECT: return "CONNECT";
        default: return "UNKNOWN";
        }
    }

    // HTTP状态码的枚举类型
    enum class StatusCode {
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

    // 将HttpStatusCode转换为对应的字符串描述
    inline std::string statusCodeToString(StatusCode status) {
        switch (status) {
        case StatusCode::Continue: return "100 Continue";
        case StatusCode::SwitchingProtocols: return "101 Switching Protocols";
        case StatusCode::OK: return "200 OK";
        case StatusCode::Created: return "201 Created";
        case StatusCode::Accepted: return "202 Accepted";
        case StatusCode::NoContent: return "204 No Content";
        case StatusCode::MovedPermanently: return "301 Moved Permanently";
        case StatusCode::Found: return "302 Found";
        case StatusCode::NotModified: return "304 Not Modified";
        case StatusCode::BadRequest: return "400 Bad Request";
        case StatusCode::Unauthorized: return "401 Unauthorized";
        case StatusCode::Forbidden: return "403 Forbidden";
        case StatusCode::NotFound: return "404 Not Found";
        case StatusCode::MethodNotAllowed: return "405 Method Not Allowed";
        case StatusCode::RequestTimeout: return "408 Request Timeout";
        case StatusCode::Conflict: return "409 Conflict";
        case StatusCode::Gone: return "410 Gone";
        case StatusCode::PayloadTooLarge: return "413 Payload Too Large";
        case StatusCode::URITooLong: return "414 URI Too Long";
        case StatusCode::UnsupportedMediaType: return "415 Unsupported Media Type";
        case StatusCode::RangeNotSatisfiable: return "416 Range Not Satisfiable";
        case StatusCode::InternalServerError: return "500 Internal Server Error";
        case StatusCode::NotImplemented: return "501 Not Implemented";
        case StatusCode::BadGateway: return "502 Bad Gateway";
        case StatusCode::ServiceUnavailable: return "503 Service Unavailable";
        case StatusCode::GatewayTimeout: return "504 Gateway Timeout";
        case StatusCode::HTTPVersionNotSupported: return "505 HTTP Version Not Supported";
        default: return "0 Unknown";
        }
    }



    // 常见的Content-Type定义
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

} // namespace http_asio

#endif // HTTP_TYPES_HPP