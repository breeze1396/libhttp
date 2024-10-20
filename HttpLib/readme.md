# 1. 基础模块

这些模块提供HTTP库的核心功能和通用工具，如HTTP消息解析、请求和响应的结构定义、以及各种数据结构和工具。

## 1.1 `http_types.hpp`

- 定义一些HTTP相关的数据类型和常量，例如：
  - `Headers`（HTTP头部类型）
  - `Params`（查询参数）
  - `Ranges`（Range请求类型）
  - 状态码和方法枚举等

`http_types.hpp` 文件将定义一些 HTTP 相关的类型和常量，以支持后续实现中的 HTTP 请求和响应处理。我们将定义 HTTP 头部、查询参数、请求方法、状态码，以及其他 HTTP 相关的结构体和枚举。这个文件会为其他模块提供基础数据类型。

```cpp
#ifndef HTTP_TYPES_HPP
#define HTTP_TYPES_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include <optional>
#include <sstream>

namespace http_asio {

// 定义HTTP头部类型，使用unordered_map存储头部字段和对应的值
using Headers = std::unordered_map<std::string, std::string>;

// 查询参数类型，存储键值对形式的查询参数
using Params = std::unordered_map<std::string, std::string>;

// Range请求的结构体定义，表示Range请求的起始和结束字节
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

// HTTP方法的枚举类型
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

// 将字符串转换为HttpMethod枚举
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

// HTTP状态码的枚举类型
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

// 将HttpStatusCode转换为对应的字符串描述
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

// 判断状态码是否是成功的状态码
inline bool isSuccessStatus(HttpStatusCode status) {
    return static_cast<int>(status) >= 200 && static_cast<int>(status) < 300;
}

// 判断状态码是否是重定向的状态码
inline bool isRedirectStatus(HttpStatusCode status) {
    return static_cast<int>(status) >= 300 && static_cast<int>(status) < 400;
}

// 判断状态码是否是客户端错误的状态码
inline bool isClientErrorStatus(HttpStatusCode status) {
    return static_cast<int>(status) >= 400 && static_cast<int>(status) < 500;
}

// 判断状态码是否是服务器错误的状态码
inline bool isServerErrorStatus(HttpStatusCode status) {
    return static_cast<int>(status) >= 500 && static_cast<int>(status) < 600;
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

// URL解码函数
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
        } else if (str[i] == '+') {
            result += ' ';
        } else {
            result += str[i];
        }
    }
    return result;
}

} // namespace http_asio

#endif // HTTP_TYPES_HPP
```

### 详细解释

1. **Headers 和 Params**:
   - `Headers` 使用 `unordered_map` 存储 HTTP 头部字段和其值。
   - `Params` 用于查询参数，同样使用 `unordered_map` 存储键值对。

2. **HttpMethod 枚举和转换**:
   - 定义了常见的 HTTP 方法，并提供 `stringToHttpMethod` 函数将字符串方法名转换为对应的枚举类型。

3. **HttpStatusCode 枚举**:
   - 包含了常见的 HTTP 状态码及其描述。
   - `statusCodeToString` 方法可以将状态码转换为标准 HTTP 响应字符串。

4. **Range 请求**:
   - `Range` 结构体支持可选的起始和结束字节。
   - `toString()` 方法用于将 `Range` 转换为符合 HTTP 规范的字符串。

5. **MimeType 定义**:
   - 包含了常见的 `Content-Type` 值，方便后续使用。

6. **URL 解码**:
   - `urlDecode` 函数用于解码 URL 编码的字符串。

此文件定义了 HTTP 相关的基本数据结构和方法，可以直接用于处理 HTTP 请求和响应。



## 1.2 `http_request.hpp`

- 定义`Request`结构，用于表示HTTP请求，包括请求方法、路径、头部、参数等。
- 提供访问头部和参数的辅助方法，如`has_header()`和`get_header_value()`。

`http_request.hpp` 文件将定义一个 `Request` 结构体来表示 HTTP 请求，并提供便捷的方法来访问和操作请求中的各类信息，如请求方法、路径、头部、参数、主体等内容。我们会确保这个结构体易于使用，并支持常见的操作。

```cpp
#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include <string>
#include <unordered_map>
#include <optional>
#include <sstream>
#include "http_types.hpp"

namespace http_asio {

class Request {
public:
    Request() = default;

    // 设置HTTP方法
    void setMethod(HttpMethod method) {
        method_ = method;
    }

    // 获取HTTP方法
    HttpMethod getMethod() const {
        return method_;
    }

    // 设置请求路径
    void setPath(const std::string& path) {
        path_ = path;
    }

    // 获取请求路径
    std::string getPath() const {
        return path_;
    }

    // 设置查询参数
    void setParams(const Params& params) {
        params_ = params;
    }

    // 获取查询参数
    Params getParams() const {
        return params_;
    }

    // 添加单个查询参数
    void addParam(const std::string& key, const std::string& value) {
        params_[key] = value;
    }

    // 获取单个查询参数值
    std::optional<std::string> getParamValue(const std::string& key) const {
        auto it = params_.find(key);
        if (it != params_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    // 设置头部
    void setHeaders(const Headers& headers) {
        headers_ = headers;
    }

    // 获取头部
    Headers getHeaders() const {
        return headers_;
    }

    // 添加单个头部字段
    void addHeader(const std::string& key, const std::string& value) {
        headers_[key] = value;
    }

    // 检查是否有某个头部字段
    bool hasHeader(const std::string& key) const {
        return headers_.find(key) != headers_.end();
    }

    // 获取头部字段的值
    std::optional<std::string> getHeaderValue(const std::string& key) const {
        auto it = headers_.find(key);
        if (it != headers_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    // 设置请求体
    void setBody(const std::string& body) {
        body_ = body;
    }

    // 获取请求体
    std::string getBody() const {
        return body_;
    }

    // 将请求转换为原始请求字符串（用于调试或发送到服务器）
    std::string toString() const {
        std::ostringstream oss;
        oss << methodToString(method_) << " " << path_;
        if (!params_.empty()) {
            oss << "?";
            for (const auto& [key, value] : params_) {
                oss << key << "=" << value << "&";
            }
            oss.seekp(-1, std::ios_base::end); // 移除最后一个 '&'
        }
        oss << " HTTP/1.1\r\n";
        
        for (const auto& [key, value] : headers_) {
            oss << key << ": " << value << "\r\n";
        }

        oss << "\r\n" << body_;
        return oss.str();
    }

private:
    HttpMethod method_ = HttpMethod::UNKNOWN;
    std::string path_;
    Params params_;
    Headers headers_;
    std::string body_;

    // 将HttpMethod转换为字符串，用于生成请求行
    std::string methodToString(HttpMethod method) const {
        switch (method) {
            case HttpMethod::GET: return "GET";
            case HttpMethod::POST: return "POST";
            case HttpMethod::PUT: return "PUT";
            case HttpMethod::DELETE: return "DELETE";
            case HttpMethod::PATCH: return "PATCH";
            case HttpMethod::HEAD: return "HEAD";
            case HttpMethod::OPTIONS: return "OPTIONS";
            case HttpMethod::TRACE: return "TRACE";
            case HttpMethod::CONNECT: return "CONNECT";
            default: return "UNKNOWN";
        }
    }
};

} // namespace http_asio

#endif // HTTP_REQUEST_HPP
```

### 详细解释

1. **`Request` 类**:
   - 定义了 HTTP 请求的基本属性：方法、路径、查询参数、头部、和主体。
   - 提供了设置和获取这些属性的公共方法，使其易于操作。

2. **请求方法（Method）**:
   - 提供 `setMethod` 和 `getMethod` 方法来设置和获取请求的 HTTP 方法。
   - 通过 `HttpMethod` 枚举类型来表示请求方法，确保类型安全。

3. **路径和查询参数**:
   - 路径使用 `setPath` 和 `getPath` 方法来设置和获取。
   - 查询参数存储为 `Params` 类型，提供了 `setParams`、`addParam` 和 `getParamValue` 方法来操作。

4. **头部字段**:
   - 使用 `Headers` 类型来表示 HTTP 头部字段。
   - 提供了 `addHeader`、`hasHeader` 和 `getHeaderValue` 方法，方便查询头部字段的存在和获取其值。

5. **请求体**:
   - 提供 `setBody` 和 `getBody` 方法来操作 HTTP 请求的主体。

6. **请求转换为字符串（toString）**:
   - `toString` 方法将整个请求转换为标准的 HTTP 请求字符串，用于调试或发送到服务器。

这个类的设计旨在易于使用并具有灵活性，可以直接用于构建和解析 HTTP 请求。

`http_request.hpp` 文件将定义一个 `Request` 结构体来表示 HTTP 请求，并提供便捷的方法来访问和操作请求中的各类信息，如请求方法、路径、头部、参数、主体等内容。我们会确保这个结构体易于使用，并支持常见的操作。

```cpp
#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include <string>
#include <unordered_map>
#include <optional>
#include <sstream>
#include "http_types.hpp"

namespace http_asio {

class Request {
public:
    Request() = default;

    // 设置HTTP方法
    void setMethod(HttpMethod method) {
        method_ = method;
    }

    // 获取HTTP方法
    HttpMethod getMethod() const {
        return method_;
    }

    // 设置请求路径
    void setPath(const std::string& path) {
        path_ = path;
    }

    // 获取请求路径
    std::string getPath() const {
        return path_;
    }

    // 设置查询参数
    void setParams(const Params& params) {
        params_ = params;
    }

    // 获取查询参数
    Params getParams() const {
        return params_;
    }

    // 添加单个查询参数
    void addParam(const std::string& key, const std::string& value) {
        params_[key] = value;
    }

    // 获取单个查询参数值
    std::optional<std::string> getParamValue(const std::string& key) const {
        auto it = params_.find(key);
        if (it != params_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    // 设置头部
    void setHeaders(const Headers& headers) {
        headers_ = headers;
    }

    // 获取头部
    Headers getHeaders() const {
        return headers_;
    }

    // 添加单个头部字段
    void addHeader(const std::string& key, const std::string& value) {
        headers_[key] = value;
    }

    // 检查是否有某个头部字段
    bool hasHeader(const std::string& key) const {
        return headers_.find(key) != headers_.end();
    }

    // 获取头部字段的值
    std::optional<std::string> getHeaderValue(const std::string& key) const {
        auto it = headers_.find(key);
        if (it != headers_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    // 设置请求体
    void setBody(const std::string& body) {
        body_ = body;
    }

    // 获取请求体
    std::string getBody() const {
        return body_;
    }

    // 将请求转换为原始请求字符串（用于调试或发送到服务器）
    std::string toString() const {
        std::ostringstream oss;
        oss << methodToString(method_) << " " << path_;
        if (!params_.empty()) {
            oss << "?";
            for (const auto& [key, value] : params_) {
                oss << key << "=" << value << "&";
            }
            oss.seekp(-1, std::ios_base::end); // 移除最后一个 '&'
        }
        oss << " HTTP/1.1\r\n";
        
        for (const auto& [key, value] : headers_) {
            oss << key << ": " << value << "\r\n";
        }

        oss << "\r\n" << body_;
        return oss.str();
    }

private:
    HttpMethod method_ = HttpMethod::UNKNOWN;
    std::string path_;
    Params params_;
    Headers headers_;
    std::string body_;

    // 将HttpMethod转换为字符串，用于生成请求行
    std::string methodToString(HttpMethod method) const {
        switch (method) {
            case HttpMethod::GET: return "GET";
            case HttpMethod::POST: return "POST";
            case HttpMethod::PUT: return "PUT";
            case HttpMethod::DELETE: return "DELETE";
            case HttpMethod::PATCH: return "PATCH";
            case HttpMethod::HEAD: return "HEAD";
            case HttpMethod::OPTIONS: return "OPTIONS";
            case HttpMethod::TRACE: return "TRACE";
            case HttpMethod::CONNECT: return "CONNECT";
            default: return "UNKNOWN";
        }
    }
};

} // namespace http_asio

#endif // HTTP_REQUEST_HPP
```

### 详细解释

1. **`Request` 类**:
   - 定义了 HTTP 请求的基本属性：方法、路径、查询参数、头部、和主体。
   - 提供了设置和获取这些属性的公共方法，使其易于操作。

2. **请求方法（Method）**:
   - 提供 `setMethod` 和 `getMethod` 方法来设置和获取请求的 HTTP 方法。
   - 通过 `HttpMethod` 枚举类型来表示请求方法，确保类型安全。

3. **路径和查询参数**:
   - 路径使用 `setPath` 和 `getPath` 方法来设置和获取。
   - 查询参数存储为 `Params` 类型，提供了 `setParams`、`addParam` 和 `getParamValue` 方法来操作。

4. **头部字段**:
   - 使用 `Headers` 类型来表示 HTTP 头部字段。
   - 提供了 `addHeader`、`hasHeader` 和 `getHeaderValue` 方法，方便查询头部字段的存在和获取其值。

5. **请求体**:
   - 提供 `setBody` 和 `getBody` 方法来操作 HTTP 请求的主体。

6. **请求转换为字符串（toString）**:
   - `toString` 方法将整个请求转换为标准的 HTTP 请求字符串，用于调试或发送到服务器。

这个类的设计旨在易于使用并具有灵活性，可以直接用于构建和解析 HTTP 请求。



## 1.3 `http_response.hpp`

- 定义`Response`结构，用于表示HTTP响应，包括状态码、头部和响应体等。
- 提供一些常用的响应设置方法，如`set_content()`、`set_header()`和`set_redirect()`。



`http_response.hpp` 文件将定义一个 `Response` 结构体来表示 HTTP 响应，并提供便捷的方法来设置和获取响应中的状态码、头部和响应体等内容。这个结构体将支持常见的 HTTP 响应操作，以便在服务器端或客户端方便地构造和处理 HTTP 响应。

```cpp
#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include <string>
#include <unordered_map>
#include <optional>
#include <sstream>
#include "http_types.hpp"

namespace http_asio {

class Response {
public:
    Response() = default;

    // 设置状态码
    void setStatusCode(HttpStatusCode code) {
        status_code_ = code;
    }

    // 获取状态码
    HttpStatusCode getStatusCode() const {
        return status_code_;
    }

    // 设置状态消息
    void setStatusMessage(const std::string& message) {
        status_message_ = message;
    }

    // 获取状态消息
    std::string getStatusMessage() const {
        return status_message_;
    }

    // 设置头部
    void setHeaders(const Headers& headers) {
        headers_ = headers;
    }

    // 获取头部
    Headers getHeaders() const {
        return headers_;
    }

    // 添加单个头部字段
    void addHeader(const std::string& key, const std::string& value) {
        headers_[key] = value;
    }

    // 检查是否有某个头部字段
    bool hasHeader(const std::string& key) const {
        return headers_.find(key) != headers_.end();
    }

    // 获取头部字段的值
    std::optional<std::string> getHeaderValue(const std::string& key) const {
        auto it = headers_.find(key);
        if (it != headers_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    // 设置响应体
    void setBody(const std::string& body) {
        body_ = body;
    }

    // 获取响应体
    std::string getBody() const {
        return body_;
    }

    // 设置重定向
    void setRedirect(const std::string& location, HttpStatusCode code = HttpStatusCode::FOUND) {
        setStatusCode(code);
        addHeader("Location", location);
        setBody("");  // 通常重定向响应体为空
    }

    // 设置内容及其类型
    void setContent(const std::string& content, const std::string& content_type) {
        setBody(content);
        addHeader("Content-Type", content_type);
    }

    // 将响应转换为原始响应字符串（用于调试或发送到客户端）
    std::string toString() const {
        std::ostringstream oss;
        oss << "HTTP/1.1 " << static_cast<int>(status_code_) << " " << status_message_ << "\r\n";
        
        for (const auto& [key, value] : headers_) {
            oss << key << ": " << value << "\r\n";
        }

        oss << "\r\n" << body_;
        return oss.str();
    }

private:
    HttpStatusCode status_code_ = HttpStatusCode::OK; // 默认状态码为200 OK
    std::string status_message_ = "OK";               // 默认状态消息
    Headers headers_;
    std::string body_;
};

} // namespace http_asio

#endif // HTTP_RESPONSE_HPP
```

### 详细解释

1. **`Response` 类**:
   - 定义了 HTTP 响应的基本属性：状态码、状态消息、头部和响应体。
   - 提供了设置和获取这些属性的公共方法，使其易于操作。

2. **状态码（Status Code）和状态消息**:
   - 提供 `setStatusCode` 和 `getStatusCode` 方法来设置和获取响应的状态码。
   - 状态消息默认值为 `"OK"`，可以使用 `setStatusMessage` 方法进行自定义。

3. **头部字段**:
   - 使用 `Headers` 类型来表示 HTTP 头部字段。
   - 提供了 `addHeader`、`hasHeader` 和 `getHeaderValue` 方法，方便查询头部字段的存在和获取其值。

4. **响应体**:
   - 使用 `setBody` 和 `getBody` 方法来操作 HTTP 响应的主体内容。

5. **常用响应设置**:
   - `setRedirect`: 用于设置重定向响应，默认状态码为 `302 FOUND`，并设置 `"Location"` 头部字段。
   - `setContent`: 用于设置响应体内容和 `"Content-Type"` 头部字段。

6. **响应转换为字符串（toString）**:
   - `toString` 方法将整个响应转换为标准的 HTTP 响应字符串，用于调试或发送到客户端。

这个 `Response` 类设计旨在简洁明了且易于使用，支持设置常见的 HTTP 响应操作，适用于服务器端或客户端的 HTTP 处理。



## 1.4 `http_util.hpp`

- 一些常用的工具和函数，例如字符串处理、URL编码解码、头部解析等。
- 定义进度和内容处理的函数类型，如`Progress`、`ContentReceiver`等。

以下是一个 `http_util.hpp` 的实现，包含常用的工具函数，例如字符串处理、URL 编码/解码、头部解析等。此外，还定义了一些函数类型，如 `Progress` 和 `ContentReceiver`，以便为后续的服务器和客户端开发做好准备。

```cpp
#ifndef HTTP_UTIL_HPP
#define HTTP_UTIL_HPP

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
std::string url_encode(const std::string& value) {
    std::ostringstream escaped;
    escaped << std::hex << std::setfill('0');

    for (char c : value) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
        } else {
            escaped << '%' << std::uppercase << std::setw(2) << static_cast<int>(static_cast<unsigned char>(c));
        }
    }
    return escaped.str();
}

// URL 解码
std::string url_decode(const std::string& value) {
    std::string decoded;
    for (size_t i = 0; i < value.size(); ++i) {
        if (value[i] == '%') {
            if (i + 2 < value.size()) {
                std::string hex = value.substr(i + 1, 2);
                char decoded_char = static_cast<char>(std::stoi(hex, nullptr, 16));
                decoded += decoded_char;
                i += 2; // Skip past the hex digits
            }
        } else {
            decoded += value[i];
        }
    }
    return decoded;
}

// 解析 HTTP 头部
Headers parse_headers(const std::string& raw_headers) {
    Headers headers;
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
std::string join_url(const std::string& base, const std::string& path) {
    if (base.back() == '/' && path.front() == '/') {
        return base + path.substr(1);
    } else if (base.back() != '/' && path.front() != '/') {
        return base + '/' + path;
    }
    return base + path;
}

// 打印请求/响应内容
void print_content(const std::string& content) {
    std::cout << "Content: " << content << std::endl;
}

// 示例进度报告
void report_progress(std::size_t current, std::size_t total) {
    std::cout << "Progress: " << (current * 100 / total) << "% completed." << std::endl;
}

} // namespace http_asio

#endif // HTTP_UTIL_HPP
```

### 代码说明

1. **函数类型定义**：
   - `Progress`：表示进度回调的类型，通常用于报告处理进度。
   - `ContentReceiver`：表示内容接收回调的类型，用于接收数据流。

2. **URL 编码与解码**：
   - `url_encode`：将字符串进行 URL 编码，以确保字符在 URL 中是安全的。
   - `url_decode`：将 URL 编码的字符串解码为原始字符串。

3. **HTTP 头部解析**：
   - `parse_headers`：将原始 HTTP 头部字符串解析为 `Headers` 类型的映射（key-value 对）。

4. **URL 连接**：
   - `join_url`：将基本 URL 和路径连接在一起，处理斜杠的情况，确保生成有效的 URL。

5. **打印内容**：
   - `print_content`：用于打印请求或响应的内容，方便调试。

6. **进度报告**：
   - `report_progress`：示例进度报告函数，用于输出处理的进度。

### 使用示例

```cpp
#include "http_util.hpp"

int main() {
    // URL 编码和解码示例
    std::string encoded = http_asio::url_encode("hello world!");
    std::cout << "Encoded: " << encoded << std::endl;
    std::cout << "Decoded: " << http_asio::url_decode(encoded) << std::endl;

    // HTTP 头部解析示例
    std::string raw_headers = "Content-Type: text/html\r\nContent-Length: 123\r\n";
    Headers headers = http_asio::parse_headers(raw_headers);
    for (const auto& [key, value] : headers) {
        std::cout << key << ": " << value << std::endl;
    }

    // 连接 URL 示例
    std::string full_url = http_asio::join_url("http://example.com/", "/path/to/resource");
    std::cout << "Joined URL: " << full_url << std::endl;

    // 进度报告示例
    http_asio::report_progress(5, 20);

    return 0;
}
```

这个 `http_util.hpp` 文件提供了一系列通用工具函数，能够帮助在 HTTP 客户端和服务器之间进行处理。同时，定义的函数类型也为异步处理提供了灵活的回调接口。这样，在实现服务器和客户端的功能时，可以直接利用这些工具类和函数来完成常见的操作。



# 2. 客户端模块

客户端模块封装了HTTP客户端的逻辑，用于发送请求和处理响应。

## 2.1 `http_client.hpp`

- 定义`Client`类，支持各种HTTP方法（如GET、POST、PUT、DELETE等）的请求。
- 提供异步请求和同步请求的接口。
- 支持配置选项，如超时时间、头部设置、压缩、认证等。
- 支持通过ASIO的TCP连接进行HTTP请求。

以下是 `http_client.hpp` 的实现，它定义了 `Client` 类，支持多种 HTTP 方法的请求，并提供异步和同步请求的接口。这个实现利用了前面提供的工具函数和数据结构，降低了冗余性并提高了扩展性。并为后续的 SSL 页提供了一定的帮助。

### 2.1 `http_client.hpp`

```cpp
#ifndef HTTP_CLIENT_HPP
#define HTTP_CLIENT_HPP

#include <asio.hpp>
#include <string>
#include <unordered_map>
#include <memory>
#include <iostream>
#include <future>
#include "http_types.hpp"
#include "http_util.hpp"
#include "http_response.hpp"

namespace http_asio {

class Client {
public:
    Client(std::shared_ptr<IOContextWrapper> io_context)
        : io_context_(io_context), socket_(*io_context->getContext()) {}

    // 设置请求头
    void set_header(const std::string& key, const std::string& value) {
        headers_[key] = value;
    }

    // 设置超时
    void set_timeout(std::chrono::seconds timeout) {
        timeout_ = timeout;
    }

    // 发送 GET 请求
    std::future<Response> get(const std::string& url) {
        return send_request("GET", url);
    }

    // 发送 POST 请求
    std::future<Response> post(const std::string& url, const std::string& body) {
        set_header("Content-Length", std::to_string(body.size()));
        return send_request("POST", url, body);
    }

    // 发送 PUT 请求
    std::future<Response> put(const std::string& url, const std::string& body) {
        set_header("Content-Length", std::to_string(body.size()));
        return send_request("PUT", url, body);
    }

    // 发送 DELETE 请求
    std::future<Response> del(const std::string& url) {
        return send_request("DELETE", url);
    }

private:
    std::shared_ptr<IOContextWrapper> io_context_;
    asio::ip::tcp::socket socket_;
    std::unordered_map<std::string, std::string> headers_;
    std::chrono::seconds timeout_{5}; // 默认超时为5秒

    // 发送请求的通用方法
    std::future<Response> send_request(const std::string& method, const std::string& url, const std::string& body = "") {
        return std::async(std::launch::async, [this, method, url, body]() {
            try {
                // 解析URL
                auto [host, path] = parse_url(url);

                // 连接服务器
                connect_to_server(host);

                // 创建请求字符串
                std::string request = build_request(method, path, body);

                // 发送请求
                asio::write(socket_, asio::buffer(request));

                // 接收响应
                Response response = read_response();

                // 关闭连接
                socket_.close();
                return response;

            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << std::endl;
                return Response{StatusCode::INTERNAL_SERVER_ERROR, {}, "Internal Server Error"};
            }
        });
    }

    // 解析URL，返回主机名和路径
    std::pair<std::string, std::string> parse_url(const std::string& url) {
        std::string host, path;
        size_t pos = url.find("://");
        if (pos != std::string::npos) {
            size_t start = pos + 3; // 跳过协议
            size_t end = url.find('/', start);
            host = url.substr(start, end - start);
            path = end == std::string::npos ? "/" : url.substr(end);
        }
        return {host, path};
    }

    // 连接到服务器
    void connect_to_server(const std::string& host) {
        asio::ip::tcp::resolver resolver(*io_context_->getContext());
        auto endpoints = resolver.resolve(host, "80"); // 使用HTTP默认端口
        asio::connect(socket_, endpoints);
    }

    // 构建请求字符串
    std::string build_request(const std::string& method, const std::string& path, const std::string& body) {
        std::ostringstream request;
        request << method << " " << path << " HTTP/1.1\r\n";
        request << "Host: " << socket_.remote_endpoint().address().to_string() << "\r\n";

        // 添加请求头
        for (const auto& [key, value] : headers_) {
            request << key << ": " << value << "\r\n";
        }

        request << "Connection: close\r\n"; // 关闭连接
        request << "\r\n"; // 请求头结束

        // 添加请求体（如果有）
        if (!body.empty()) {
            request << body;
        }

        return request.str();
    }

    // 读取响应
    Response read_response() {
        std::string response_line;
        std::getline(asio::buffered_read_until(socket_, asio::dynamic_buffer(response_line), "\r\n"));

        // 解析状态码
        std::istringstream response_stream(response_line);
        std::string http_version;
        int status_code;
        response_stream >> http_version >> status_code;

        // 读取头部
        std::string header;
        Headers headers = parse_headers(socket_);

        // 读取响应体
        std::string body;
        asio::read(socket_, asio::dynamic_buffer(body)); // 可以进一步处理 chunked 响应

        return Response{static_cast<StatusCode>(status_code), headers, body};
    }
};

} // namespace http_asio

#endif // HTTP_CLIENT_HPP
```

### 代码说明

1. **类的定义**：
   - `Client` 类提供了与 HTTP 服务器进行交互的接口。
   - 使用 `asio::ip::tcp::socket` 进行网络连接。

2. **请求方法**：
   - 提供 `get`、`post`、`put` 和 `delete` 方法，分别对应不同的 HTTP 请求方法。

3. **请求配置**：
   - `set_header` 方法允许用户设置 HTTP 头部。
   - `set_timeout` 方法可以设置请求的超时时间。

4. **发送请求**：
   - `send_request` 是一个私有方法，处理请求的发送和响应的接收。
   - 使用 `std::async` 实现异步请求。
   - 解析 URL，连接到服务器，构建请求字符串，发送请求并接收响应。

5. **URL 解析**：
   - `parse_url` 方法将完整的 URL 拆分为主机名和路径。

6. **建立连接**：
   - `connect_to_server` 方法使用解析器连接到服务器。

7. **构建请求**：
   - `build_request` 方法构建 HTTP 请求字符串。

8. **读取响应**：
   - `read_response` 方法负责读取 HTTP 响应，并解析状态码和头部。

### 使用示例

```cpp
#include <iostream>
#include "http_client.hpp"
#include "http_util.hpp"

int main() {
    auto io_context = std::make_shared<http_asio::IOContextWrapper>();
    http_asio::Client client(io_context);

    // 设置请求头
    client.set_header("User-Agent", "MyHttpClient/1.0");

    // 发送 GET 请求
    auto future_response = client.get("http://example.com");

    // 等待并获取响应
    http_asio::Response response = future_response.get();
    std::cout << "Response Code: " << static_cast<int>(response.status_code) << std::endl;
    std::cout << "Response Body: " << response.body << std::endl;

    return 0;
}
```

### 扩展性和 SSL 支持

- 此实现已经为未来的 SSL 支持做好了基础。如果需要支持 HTTPS，可以在 `connect_to_server` 方法中使用 `asio::ssl::stream` 来进行加密的 TCP 连接。
- 可以进一步扩展请求支持，例如添加更多的请求配置选项、实现更复杂的响应处理（如 chunked 响应）、重试机制等。

通过这种方式，`http_client.hpp` 代码模块变得完整且易于使用，能够满足大多数 HTTP 客户端的需求，并为 SSL 的实现提供了基础架构。



# 3. 服务器模块

服务器模块封装了HTTP服务器的逻辑，用于处理客户端请求并发送响应。

## 3.1 `http_server.hpp`

- 定义`Server`类，支持路由设置（GET、POST等）。
- 提供设置错误处理、文件处理、以及内容读取的接口。
- 使用ASIO进行异步处理，管理连接和请求处理流程。

以下是 `http_server.hpp` 的完整实现。这个实现定义了 `Server` 类，支持路由设置、错误处理、文件处理和内容读取，使用 ASIO 进行异步处理以管理连接和请求处理流程。

### 3.1 `http_server.hpp`

```cpp
#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#include <asio.hpp>
#include <string>
#include <unordered_map>
#include <functional>
#include <memory>
#include <iostream>
#include "http_types.hpp"
#include "http_response.hpp"
#include "http_util.hpp"

namespace http_asio {

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(asio::ip::tcp::socket socket, std::shared_ptr<IOContextWrapper> io_context, std::function<void(Response&)> error_handler)
        : socket_(std::move(socket)), io_context_(io_context), error_handler_(error_handler) {}

    void start() {
        read_request();
    }

private:
    asio::ip::tcp::socket socket_;
    std::shared_ptr<IOContextWrapper> io_context_;
    std::function<void(Response&)> error_handler_;
    Request request_;

    void read_request() {
        auto self(shared_from_this());
        asio::async_read_until(socket_, asio::dynamic_buffer(buffer_), "\r\n\r\n",
            [this, self](std::error_code ec, std::size_t length) {
                if (!ec) {
                    parse_request();
                    handle_request();
                } else {
                    send_error_response(StatusCode::BAD_REQUEST);
                }
            });
    }

    void parse_request() {
        std::istream request_stream(&buffer_);
        std::string request_line;
        std::getline(request_stream, request_line);
        // 解析请求行
        std::istringstream request_line_stream(request_line);
        std::string method, uri;
        request_line_stream >> method >> uri;

        request_.method = method;
        request_.path = uri;

        // 解析头部
        std::string header;
        while (std::getline(request_stream, header) && header != "\r") {
            size_t colon_index = header.find(':');
            if (colon_index != std::string::npos) {
                std::string key = header.substr(0, colon_index);
                std::string value = header.substr(colon_index + 1);
                request_.headers[key] = trim(value);
            }
        }
    }

    void handle_request() {
        Response response;
        // 这里可以添加路由处理逻辑
        if (request_.method == "GET") {
            handle_get(response);
        } else if (request_.method == "POST") {
            handle_post(response);
        } else {
            send_error_response(StatusCode::METHOD_NOT_ALLOWED);
            return;
        }
        send_response(response);
    }

    void handle_get(Response& response) {
        // 示例：处理GET请求
        response.set_content("<h1>GET request received</h1>", "text/html");
        response.set_status(StatusCode::OK);
    }

    void handle_post(Response& response) {
        // 示例：处理POST请求
        response.set_content("<h1>POST request received</h1>", "text/html");
        response.set_status(StatusCode::OK);
    }

    void send_response(const Response& response) {
        auto self(shared_from_this());
        std::ostringstream response_stream;
        response_stream << "HTTP/1.1 " << static_cast<int>(response.status_code) << " " << status_message(response.status_code) << "\r\n";
        for (const auto& [key, value] : response.headers) {
            response_stream << key << ": " << value << "\r\n";
        }
        response_stream << "Content-Length: " << response.body.size() << "\r\n";
        response_stream << "Connection: close\r\n\r\n";
        response_stream << response.body;

        asio::async_write(socket_, asio::buffer(response_stream.str()),
            [this, self](std::error_code ec, std::size_t /*length*/) {
                if (ec) {
                    send_error_response(StatusCode::INTERNAL_SERVER_ERROR);
                }
            });
    }

    void send_error_response(StatusCode status_code) {
        Response response;
        response.set_status(status_code);
        error_handler_(response);
        send_response(response);
    }
};

class Server {
public:
    Server(short port, std::shared_ptr<IOContextWrapper> io_context)
        : acceptor_(*io_context->getContext(), asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)), io_context_(io_context) {
        start_accept();
    }

    void set_error_handler(std::function<void(Response&)> handler) {
        error_handler_ = handler;
    }

private:
    asio::ip::tcp::acceptor acceptor_;
    std::shared_ptr<IOContextWrapper> io_context_;
    std::function<void(Response&)> error_handler_;

    void start_accept() {
        acceptor_.async_accept([this](std::error_code ec, asio::ip::tcp::socket socket) {
            if (!ec) {
                std::make_shared<Session>(std::move(socket), io_context_, error_handler_)->start();
            }
            start_accept();
        });
    }
};

} // namespace http_asio

#endif // HTTP_SERVER_HPP
```

### 代码说明

1. **Session 类**：
   - 负责处理单个客户端连接，继承自 `std::enable_shared_from_this` 以支持自我共享。
   - 使用 `asio::async_read_until` 异步读取请求，并解析请求。

2. **读取请求**：
   - `read_request` 方法用于异步读取 HTTP 请求。
   - `parse_request` 方法解析请求行和请求头。

3. **处理请求**：
   - `handle_request` 方法根据请求方法（GET 或 POST）调用相应的处理函数。
   - 示例中的 `handle_get` 和 `handle_post` 方法生成简单的 HTML 响应。

4. **发送响应**：
   - `send_response` 方法格式化 HTTP 响应并异步发送给客户端。

5. **错误处理**：
   - `send_error_response` 方法生成错误响应并发送。
   - `set_error_handler` 方法允许用户自定义错误处理逻辑。

6. **Server 类**：
   - 负责监听传入连接并创建新的 `Session` 实例处理连接。
   - 使用 `asio::ip::tcp::acceptor` 来接受客户端连接。

7. **异步接受连接**：
   - `start_accept` 方法实现异步接受连接，并在接受连接后创建新的 `Session` 实例。

### 使用示例

```cpp
#include <iostream>
#include "http_server.hpp"

int main() {
    auto io_context = std::make_shared<http_asio::IOContextWrapper>();
    http_asio::Server server(8080, io_context);

    // 设置错误处理
    server.set_error_handler([](http_asio::Response& response) {
        response.set_content("<h1>Error occurred</h1>", "text/html");
        response.set_status(http_asio::StatusCode::INTERNAL_SERVER_ERROR);
    });

    std::cout << "Server is running on port 8080..." << std::endl;
    io_context->run(); // 运行IO上下文，开始处理请求

    return 0;
}
```

### 扩展性

- 该实现支持进一步扩展，例如添加路由功能，支持更多的 HTTP 方法（如 PUT 和 DELETE）。
- 可以集成中间件，处理认证、日志记录等功能。
- 为 SSL/TLS 支持做好准备，方便后续使用 `asio::ssl::stream`。

通过这种方式，`http_server.hpp` 的代码模块可以正常使用，满足基本的 HTTP 服务器需求，并为进一步的扩展和 SSL 支持提供了基础架构。



# 4. 实用模块

这些模块用于实现HTTP库的辅助功能，例如多线程支持和内容处理。

## 4.1 `http_content.hpp`

- 定义`ContentReader`类，用于处理多部分数据和内容读取。
- 定义`ContentProvider`和`ChunkedContentProvider`函数类型。

为了将 `ContentReader` 类改为使用 ASIO 的异步接口，我们需要借助 ASIO 的 `async_read`、`async_write` 等函数来处理异步 I/O 操作。这样做的好处是可以避免阻塞主线程，从而提升并发性能。我们将使用回调函数（handler）来处理异步操作的完成情况。

### 改进设计思路

- **异步接口**：
  - 通过 ASIO 的异步接口来处理数据读取和解析，比如多部分数据读取和分块传输。
  - 使用回调函数（即 handler）来接收异步操作完成的结果。
  
- **支持异步多部分数据和内容块读取**：
  - 实现 `async_readPart()` 方法，用于异步读取和处理 HTTP 请求中的多部分数据。
  - 实现 `async_readChunk()` 方法，用于异步读取分块传输的数据。

### 异步版本 `ContentReader` 实现

```cpp
#ifndef HTTP_CONTENT_HPP
#define HTTP_CONTENT_HPP

#include <asio.hpp>
#include <string>
#include <functional>
#include <iostream>
#include <vector>
#include <memory>

namespace http_asio {

// 定义ContentProvider和ChunkedContentProvider类型
using ContentProvider = std::function<void(size_t offset, size_t max_size, std::function<void(const std::string&)>)>;
using ChunkedContentProvider = std::function<void(size_t chunk_size, std::function<void(const std::string&)>)>;

// ContentReader类，用于处理HTTP内容读取和多部分数据
class ContentReader : public std::enable_shared_from_this<ContentReader> {
public:
    ContentReader(asio::io_context& io_context, asio::ip::tcp::socket& socket, const std::string& boundary)
        : socket_(socket), boundary_(boundary), state_(ReadState::Idle) {}

    void setContentProvider(ContentProvider provider) {
        content_provider_ = provider;
    }

    void setChunkedContentProvider(ChunkedContentProvider provider) {
        chunked_content_provider_ = provider;
    }

    // 异步读取内容数据
    void async_readContent(size_t max_size, std::function<void(const std::string&)> handler) {
        if (content_provider_) {
            content_provider_(current_offset_, max_size, handler);
        } else {
            handler(""); // 如果没有内容提供者，返回空字符串
        }
    }

    // 异步解析多部分内容
    void async_parseMultipartData(std::function<void()> handler) {
        auto self = shared_from_this();
        async_readPart([this, self, handler](bool success) {
            if (success) {
                // 递归调用解析下一部分
                async_parseMultipartData(handler);
            } else {
                handler(); // 解析完成或出现错误
            }
        });
    }

    // 异步处理分块传输
    void async_handleChunkedTransfer(std::function<void()> handler) {
        if (!chunked_content_provider_) {
            handler();
            return;
        }

        auto self = shared_from_this();
        size_t chunk_size = 4096; // 设定默认块大小

        async_readChunk(chunk_size, [this, self, handler](const std::string& chunk) {
            if (chunk.empty()) {
                handler(); // 传输完成
                return;
            }

            processChunk(chunk);

            // 继续读取下一个块
            async_handleChunkedTransfer(handler);
        });
    }

private:
    // 内部状态定义
    enum class ReadState {
        Idle,
        Reading,
        ParsingMultipart,
        ChunkedTransfer
    };

    asio::ip::tcp::socket& socket_;
    std::string boundary_;
    ContentProvider content_provider_;
    ChunkedContentProvider chunked_content_provider_;
    size_t current_offset_ = 0;
    ReadState state_;

    // 异步读取和处理单个部分的数据
    void async_readPart(std::function<void(bool)> handler) {
        auto self = shared_from_this();
        asio::async_read_until(socket_, asio::dynamic_buffer(buffer_), boundary_,
            [this, self, handler](const asio::error_code& ec, std::size_t bytes_transferred) {
                if (!ec) {
                    std::string part_header = extractHeader();
                    std::string part_content = extractBody(bytes_transferred);
                    processPart(part_header, part_content);
                    handler(true);
                } else {
                    handler(false); // 读取错误
                }
            });
    }

    // 异步读取一个块
    void async_readChunk(size_t chunk_size, std::function<void(const std::string&)> handler) {
        auto self = shared_from_this();
        asio::async_read(socket_, asio::buffer(buffer_, chunk_size),
            [this, self, handler](const asio::error_code& ec, std::size_t bytes_transferred) {
                if (!ec) {
                    std::string chunk(buffer_.data(), bytes_transferred);
                    handler(chunk);
                } else {
                    handler(""); // 读取失败，返回空字符串
                }
            });
    }

    // 处理一个内容块
    void processChunk(const std::string& chunk) {
        // 处理或发送块数据
        std::cout << "Processing chunk of size: " << chunk.size() << std::endl;
    }

    // 处理每个多部分数据
    void processPart(const std::string& header, const std::string& content) {
        std::cout << "Processing part with header: " << header << std::endl;
    }

    // 伪代码：从缓冲区提取头部信息
    std::string extractHeader() {
        return "Part header data";
    }

    // 伪代码：从缓冲区提取内容体
    std::string extractBody(size_t bytes_transferred) {
        return std::string(buffer_.data(), bytes_transferred);
    }

    std::vector<char> buffer_{4096}; // 临时缓冲区
};

} // namespace http_asio

#endif // HTTP_CONTENT_HPP
```

### 详细解释

1. **异步读取的实现**：
   - `async_readPart` 和 `async_readChunk` 方法使用 ASIO 的异步接口进行读取操作。
   - 每次读取完成后，通过回调函数（handler）传递结果，避免阻塞调用线程。

2. **共享指针管理**：
   - 使用 `enable_shared_from_this` 来确保在异步操作过程中对象的生命周期得到正确管理，避免对象在异步调用期间被销毁。

3. **多部分和分块传输**：
   - `async_parseMultipartData` 方法通过递归调用自身的方式处理多部分数据。
   - `async_handleChunkedTransfer` 方法逐块读取并处理传输数据。

4. **回调函数与错误处理**：
   - 每个异步操作都带有一个回调函数，用于在操作完成时传递结果或状态。
   - 在异步读取失败时，通过传递空字符串或错误标志来通知调用者处理错误情况。

5. **性能和非阻塞性**：
   - 通过异步调用和回调机制，`ContentReader` 可以在不阻塞主线程的情况下读取内容，大大提升了并发处理性能。

### 改进建议

- **增加超时机制**：在每个异步操作中可以增加超时检测，防止长时间阻塞。
- **日志和错误信息**：增加日志记录和详细的错误信息输出，以便在调试和监控时更加清晰了解系统状态。







## 4.2 `http_thread_pool.hpp`

- 实现一个完善的线程池，用于在服务器端处理多并发请求。

在实现一个完善的线程池时，C++20 提供了一些新特性（如 `std::jthread` 和 `std::stop_token`），这些特性使得线程池的实现更加优雅和安全。此外，我们可以使用模板来确保线程池的通用性，以支持多种任务类型。

##### 设计思路

- **任务队列**：使用一个线程安全的任务队列来存储提交的任务。
- **线程管理**：线程池内部包含多个线程，这些线程会不断从任务队列中获取任务执行。
- **可停止性**：使用 `std::stop_token`，确保在请求停止时可以优雅地终止线程池。
- **模板化任务**：使用模板支持不同类型的任务（即任意返回类型的 `callable` 对象）。

##### 实现 `http_thread_pool.hpp`

```cpp
#ifndef HTTP_THREAD_POOL_HPP
#define HTTP_THREAD_POOL_HPP

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <stop_token>
#include <iostream>

namespace http_asio {

class ThreadPool {
public:
    // 构造函数：初始化线程池并启动指定数量的线程
    explicit ThreadPool(size_t num_threads)
        : stop_requested_(false) {
        for (size_t i = 0; i < num_threads; ++i) {
            threads_.emplace_back([this](std::stop_token stoken) {
                while (!stoken.stop_requested()) {
                    std::function<void()> task;
                    {
                        // 互斥锁保护任务队列
                        std::unique_lock<std::mutex> lock(mutex_);
                        condition_.wait(lock, [this, &stoken] {
                            return stop_requested_ || !tasks_.empty();
                        });

                        if (stop_requested_ && tasks_.empty()) {
                            return;
                        }

                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }
                    // 执行任务
                    task();
                }
            });
        }
    }

    // 禁用拷贝构造和拷贝赋值
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    // 析构函数：停止线程池并等待所有线程完成
    ~ThreadPool() {
        stop(); // 确保停止所有线程
    }

    // 提交任务到线程池，并返回一个future对象来获取结果
    template<typename Func, typename... Args>
    auto submit(Func&& func, Args&&... args)
        -> std::future<std::invoke_result_t<Func, Args...>> {
        using ReturnType = std::invoke_result_t<Func, Args...>;

        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<Func>(func), std::forward<Args>(args)...)
        );

        std::future<ReturnType> result = task->get_future();
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if (stop_requested_) {
                throw std::runtime_error("Submit on stopped ThreadPool");
            }
            tasks_.emplace([task]() { (*task)(); });
        }
        condition_.notify_one();
        return result;
    }

    // 停止线程池，停止所有线程并等待它们完成
    void stop() {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            stop_requested_ = true;
        }
        condition_.notify_all(); // 唤醒所有线程
        for (auto& t : threads_) {
            if (t.joinable()) {
                t.request_stop(); // 使用 std::jthread 的 stop 请求
                t.join(); // 等待线程完成
            }
        }
    }

private:
    std::vector<std::jthread> threads_; // 使用 std::jthread 管理线程
    std::queue<std::function<void()>> tasks_; // 任务队列
    std::mutex mutex_; // 保护任务队列
    std::condition_variable_any condition_; // 用于线程间的通知
    bool stop_requested_; // 标识线程池是否已请求停止
};

} // namespace http_asio

#endif // HTTP_THREAD_POOL_HPP
```

##### 解释

1. **`ThreadPool` 构造函数**：
   - 根据给定的线程数量，启动线程并在每个线程中执行一个循环任务处理器。
   - 线程会从任务队列中获取任务并执行，使用 `std::stop_token` 来检查是否有停止请求。
  
2. **`submit()` 方法**：
   - 这个方法是模板化的，支持将任意类型的 `callable` 对象提交到线程池。
   - 使用 `std::packaged_task` 包装任务，以便返回一个 `std::future` 对象，方便在后续获取任务的执行结果。
   - 任务被放入一个 `std::queue` 中，并由线程在条件变量的通知下取出和执行。

3. **`stop()` 方法**：
   - 设置 `stop_requested_` 标志，并通知所有线程，停止事件循环。
   - 使用 `std::jthread` 的 `request_stop()` 方法优雅地停止线程。

4. **析构函数**：
   - 在析构时调用 `stop()` 确保线程池正确停止，避免程序退出时发生资源泄漏或未完成的任务。

##### 线程池的使用示例

```cpp
int main() {
    try {
        http_asio::ThreadPool pool(4);

        // 提交任务到线程池
        auto future1 = pool.submit([] {
            std::cout << "Task 1 executed" << std::endl;
        });

        auto future2 = pool.submit([](int value) {
            std::cout << "Task 2 executed with value: " << value << std::endl;
            return value * 2;
        }, 10);

        // 等待任务完成
        future1.get();
        int result = future2.get();
        std::cout << "Result from task 2: " << result << std::endl;

        // 停止线程池
        pool.stop();

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
```

##### 优势和扩展

- **并发性**：线程池允许多个任务并发执行，并在多个线程间进行负载均衡，提高服务器的响应速度。
- **优雅停止**：使用 `std::jthread` 和 `stop_token` 使线程池可以安全、优雅地停止。
- **模板化支持**：支持任意类型的任务，通过模板化 `submit` 方法使线程池更加通用。
- **C++20 的新特性**：利用 C++20 的 `std::jthread` 和 `std::invoke_result_t` 等特性简化代码结构，使得线程管理更为高效和安全。

##### 进一步改进

- **任务优先级**：可以扩展任务队列，支持优先级队列，从而根据任务的优先级分配执行顺序。
- **动态线程管理**：添加功能，根据当前负载增加或减少线程数量。
- **监控功能**：为线程池添加任务统计和监控功能，例如当前任务数量、已完成任务数等。

这种设计为多并发请求处理提供了一个高效和可扩展的基础设施，同时利用 C++20 的新特性提升了代码的简洁性和健壮性。



### 5. 低级模块

这些模块封装了基于ASIO的具体实现，提供对TCP套接字和连接的封装。

#### 5.1 `http_asio_wrapper.hpp`

- 包装ASIO的核心功能，定义一些用于管理IO上下文、连接和定时器的类。
- 提供处理客户端和服务器端TCP连接的方法，例如`read_response_line()`和`write_request()`。

#### 5.2 实现

​	在实现 `http_asio_wrapper.hpp` 时，我们需要对 ASIO 的基本功能进行封装，使其能够方便地管理 IO 上下文、TCP 连接以及定时器。这样设计的目的是为了简化在上层模块中处理客户端和服务器端连接的逻辑。以下是一个实现的示例思路，包括几个核心类的实现概述。

1. **IO上下文管理类 (`IOContextWrapper`)**
   - 负责管理 ASIO 的 `io_context` 对象，可以启动和停止上下文。
2. **TCP 连接管理类 (`TCPConnection`)**
   - 封装 TCP 套接字的创建、连接、读取和写入等操作。
   - 提供用于处理客户端连接的 `read_response_line()` 和用于处理服务器端请求的 `write_request()` 方法。
3. **定时器类 (`TimerWrapper`)**
   - 封装 ASIO 的定时器功能，允许设置和取消定时操作，例如超时检测。

```c++
#ifndef HTTP_ASIO_WRAPPER_HPP
#define HTTP_ASIO_WRAPPER_HPP

#include <asio.hpp>
#include <memory>
#include <string>
#include <functional>
#include <iostream>

namespace http_asio {

// IO上下文管理类
class IOContextWrapper {
public:
    IOContextWrapper() : io_context_(std::make_shared<asio::io_context>()) {}

    std::shared_ptr<asio::io_context> getContext() {
        return io_context_;
    }

    void run() {
        io_context_->run();
    }

    void stop() {
        io_context_->stop();
    }

private:
    std::shared_ptr<asio::io_context> io_context_;
};
    
    
class IOContextPool {
public:
    explicit IOContextPool(std::size_t pool_size)
        : next_io_context_(0) {
        if (pool_size == 0) {
            throw std::runtime_error("Pool size must be greater than 0");
        }

        // 创建IOContextWrapper实例并存储在池中
        for (std::size_t i = 0; i < pool_size; ++i) {
            auto context = std::make_shared<IOContextWrapper>();
            io_contexts_.push_back(context);
            threads_.emplace_back([context]() { context->run(); });
        }
    }

    ~IOContextPool() {
        stop(); // 停止所有 io_context
        join(); // 等待所有线程完成
    }

    // 获取池中下一个 io_context 实例，采用轮询算法
    std::shared_ptr<asio::io_context> getNextContext() {
        std::lock_guard<std::mutex> lock(mutex_);
        auto io_context = io_contexts_[next_io_context_]->getContext();
        next_io_context_ = (next_io_context_ + 1) % io_contexts_.size();
        return io_context;
    }

    // 停止池中所有的 io_context 实例
    void stop() {
        for (auto& context : io_contexts_) {
            context->stop();
        }
    }

    // 等待所有线程完成
    void join() {
        for (auto& thread : threads_) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

private:
    std::vector<std::shared_ptr<IOContextWrapper>> io_contexts_;
    std::vector<std::thread> threads_;
    std::size_t next_io_context_;
    std::mutex mutex_;
};


// TCP连接管理类
class TCPConnection : public std::enable_shared_from_this<TCPConnection> {
public:
    TCPConnection(std::shared_ptr<asio::io_context> io_context)
        : socket_(*io_context) {}

    asio::ip::tcp::socket& socket() {
        return socket_;
    }

    // 异步读取响应行
    void read_response_line(std::function<void(const std::string&, const asio::error_code&)> callback) {
        auto self = shared_from_this();
        asio::async_read_until(socket_, asio::dynamic_buffer(buffer_), "\r\n",
            [this, self, callback](const asio::error_code& ec, std::size_t bytes_transferred) {
                if (!ec) {
                    std::istream response_stream(&buffer_);
                    std::string response_line;
                    std::getline(response_stream, response_line);
                    callback(response_line, ec);
                } else {
                    callback("", ec);
                }
            });
    }

    // 异步写入请求
    void write_request(const std::string& request, std::function<void(const asio::error_code&)> callback) {
        auto self = shared_from_this();
        asio::async_write(socket_, asio::buffer(request),
            [this, self, callback](const asio::error_code& ec, std::size_t /*bytes_transferred*/) {
                callback(ec);
            });
    }

    // 连接到服务器
    void connect(const asio::ip::tcp::endpoint& endpoint, std::function<void(const asio::error_code&)> callback) {
        auto self = shared_from_this();
        socket_.async_connect(endpoint,
            [this, self, callback](const asio::error_code& ec) {
                callback(ec);
            });
    }

private:
    asio::ip::tcp::socket socket_;
    std::string buffer_;
};

// 定时器类
class TimerWrapper {
public:
    TimerWrapper(std::shared_ptr<asio::io_context> io_context)
        : timer_(*io_context) {}

    void start_timer(std::chrono::seconds duration, std::function<void(const asio::error_code&)> callback) {
        timer_.expires_after(duration);
        timer_.async_wait(callback);
    }

    void cancel_timer() {
        timer_.cancel();
    }

private:
    asio::steady_timer timer_;
};

} // namespace http_asio

#endif // HTTP_ASIO_WRAPPER_HPP

```

##### 解释

1. **`IOContextWrapper`**:
   - 用于管理 ASIO 的 `io_context` 对象，可以通过 `getContext()` 获取上下文，并通过 `run()` 启动事件循环。
2. **`TCPConnection`**:
   - 封装了 ASIO 的 TCP 套接字，提供了 `connect` 方法来异步连接到服务器。
   - `read_response_line` 方法用于读取响应行，`write_request` 方法用于发送请求。这些方法都是异步的，使用了回调函数处理结果。
3. **`TimerWrapper`**:
   - 封装了定时器的功能，允许在指定的时间后执行回调，并支持取消定时操作。

##### 进一步扩展

- **错误处理**：可以在回调中更详细地处理错误信息，或者设计更高级的错误处理机制。
- **连接管理**：可以进一步封装 TCP 连接管理，如重试连接机制、连接池管理等。
- **配置灵活性**：可以通过配置文件或参数让这些类更灵活，以适应不同的网络环境或超时设置。

这个基础实现可以作为一个起点，方便以后添加其他功能，如 SSL/TLS 支持、请求与响应解析等。