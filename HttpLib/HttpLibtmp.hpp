#ifndef BRE_HTTP_LIB
#define BRE_HTTP_LIB

#include "DataSink.hpp"
#include "Stream.hpp"
#include "MultipartFormData.hpp"
#include "const.hpp"

#include "BreUtils/Log.hpp"
#include "BreUtils/TaskQueue.hpp"


#include <asio.hpp>

#include <errno.h>
#include <fcntl.h>
#include <cassert>
#include <climits>

#include <array>
#include <atomic>
#include <condition_variable>
#include <fstream>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <random>
#include <regex>
#include <string>
#include <thread>
#include <typeinfo>



namespace bre {
namespace detail {
struct ci {
    bool operator()(const std::string& s1, const std::string& s2) const {
        return std::lexicographical_compare(
            s1.begin(), s1.end(), s2.begin(), s2.end(),
            [](char c1, char c2) { return ::tolower(c1) < ::tolower(c2); });
    }
};

} // namespace detail
}

namespace bre {
    class DataSink;
	class Stream;
	struct MultipartFormData;
    class ContentReader;
    struct Request;
    struct Response;

    class Server;
	class Client;

using asio::ip::tcp;
using asio::ip::address;

using Headers = std::multimap<std::string, std::string, detail::ci>;

using Params = std::multimap<std::string, std::string>;
using Range = std::pair<long long, long long>;
using Ranges = std::vector<Range>;
using Match = std::smatch;

using Progress = std::function<bool(uint64_t current, uint64_t total)>;
using ResponseHandler = std::function<bool(const Response& response)>;

using ChunkedContentProvider = std::function<bool(size_t offset, DataSink& sink)>;

using MultipartFormDataItems = std::vector<MultipartFormData>;
using MultipartFormDataMap = std::multimap<std::string, MultipartFormData>;
using MultipartContentHeader = std::function<bool(const MultipartFormData& file)>;

using ContentProvider = std::function<bool(size_t offset, size_t length, DataSink& sink)>;
using ContentReceiver = std::function<bool(const char* data, size_t data_length)>;

class ContentReader {
public:
    using Reader = std::function<bool(ContentReceiver receiver)>;
    using MultipartReader = std::function<bool(MultipartContentHeader header,
        ContentReceiver receiver)>;

    ContentReader(Reader reader, MultipartReader multipart_reader)
        : reader_(reader), multipart_reader_(multipart_reader) {}

    bool operator()(MultipartContentHeader header,
        ContentReceiver receiver) const {
        return multipart_reader_(header, receiver);
    }

    bool operator()(ContentReceiver receiver) const { return reader_(receiver); }

    Reader reader_;
    MultipartReader multipart_reader_;
};


struct Request {
    std::string method;
    std::string path;
    Headers headers;
    std::string body;

    std::string remote_addr;
    int remote_port = -1;

    // for server
    std::string version;
    std::string target;
    Params params;
    MultipartFormDataMap files;
    Ranges ranges;
    Match matches;

    // for client
    size_t redirect_count = CPPHTTPLIB_REDIRECT_MAX_COUNT;
    ResponseHandler response_handler;
    ContentReceiver content_receiver;
    size_t content_length = 0;
    ContentProvider content_provider;
    Progress progress;

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
    const SSL* ssl;
#endif
    bool has_header(const char* key) const;
    std::string get_header_value(const char* key, size_t id = 0) const;
    size_t get_header_value_count(const char* key) const;
    void set_header(const char* key, const char* val);
    void set_header(const char* key, const std::string& val);

    bool has_param(const char* key) const;
    std::string get_param_value(const char* key, size_t id = 0) const;
    size_t get_param_value_count(const char* key) const;

    bool is_multipart_form_data() const;

    bool has_file(const char* key) const;
    MultipartFormData get_file_value(const char* key) const;

    // private members...
    size_t authorization_count_ = 0;
};

struct Response {
    std::string version;
    int status = -1;
    Headers headers;
    std::string body;

    bool has_header(const char* key) const;
    std::string get_header_value(const char* key, size_t id = 0) const;
    size_t get_header_value_count(const char* key) const;
    void set_header(const char* key, const char* val);
    void set_header(const char* key, const std::string& val);

    void set_redirect(const char* url, int status = 302);
    void set_content(const char* s, size_t n, const char* content_type);
    void set_content(std::string s, const char* content_type);

    void set_content_provider(
        size_t length, ContentProvider provider,
        std::function<void()> resource_releaser = [] {});

    void set_chunked_content_provider(
        ChunkedContentProvider provider,
        std::function<void()> resource_releaser = [] {});

    Response() = default;
    Response(const Response&) = default;
    Response& operator=(const Response&) = default;
    Response(Response&&) = default;
    Response& operator=(Response&&) = default;
    ~Response() {
        if (content_provider_resource_releaser_) {
            content_provider_resource_releaser_();
        }
    }

    size_t content_length_ = 0;
    ContentProvider content_provider_;
    std::function<void()> content_provider_resource_releaser_;
};

class Server {
public:
    using Handler = std::function<void(const Request&, Response&)>;
    using HandlerWithContentReader = std::function<void(
        const Request&, Response&, const ContentReader& content_reader)>;
    using Expect100ContinueHandler =
        std::function<int(const Request&, Response&)>;

    Server();

    virtual ~Server();

    virtual bool is_valid() const;

    Server& Get(const char* pattern, Handler handler);
    Server& Post(const char* pattern, Handler handler);
    Server& Post(const char* pattern, HandlerWithContentReader handler);
    Server& Put(const char* pattern, Handler handler);
    Server& Put(const char* pattern, HandlerWithContentReader handler);
    Server& Patch(const char* pattern, Handler handler);
    Server& Patch(const char* pattern, HandlerWithContentReader handler);
    Server& Delete(const char* pattern, Handler handler);
    Server& Delete(const char* pattern, HandlerWithContentReader handler);
    Server& Options(const char* pattern, Handler handler);

    [[deprecated]] bool set_base_dir(const char* dir,
        const char* mount_point = nullptr);
    bool set_mount_point(const char* mount_point, const char* dir);
    bool remove_mount_point(const char* mount_point);
    void set_file_extension_and_mimetype_mapping(const char* ext,
        const char* mime);
    void set_file_request_handler(Handler handler);

    void set_error_handler(Handler handler);

    void set_expect_100_continue_handler(Expect100ContinueHandler handler);

    void set_keep_alive_max_count(size_t count);
    void set_read_timeout(time_t sec, time_t usec);
    void set_write_timeout(time_t sec, time_t usec);
    void set_payload_max_length(size_t length);

    bool start(const char* host, int port, int socket_flags = 0);
    bool start_on_any_port(const char* host, int socket_flags = 0);
	bool start(const char* host, int port, std::function<void()> initializer,
		int socket_flags = 0);
    bool is_running() const;
    void stop();

    std::function<TaskQueue* (void)> new_task_queue;

protected:
    bool process_request(Stream& strm, bool last_connection,
        bool& connection_close,
        const std::function<void(Request&)>& setup_request);

    size_t keep_alive_max_count_;
    time_t read_timeout_sec_;
    time_t read_timeout_usec_;
    time_t write_timeout_sec_;
    time_t write_timeout_usec_;
    size_t payload_max_length_;

private:
    using Handlers = std::vector<std::pair<std::regex, Handler>>;
    using HandlersForContentReader =
        std::vector<std::pair<std::regex, HandlerWithContentReader>>;

    void init_acceptor(const char* host, int port, int socket_flags);
    void start_accepting();
    void handle_accept(std::shared_ptr<tcp::socket> new_socket, const std::error_code& error);

    bool routing(Request& req, Response& res, Stream& strm);
    bool handle_file_request(Request& req, Response& res, bool head = false);
    bool dispatch_request(Request& req, Response& res, Handlers& handlers);
    bool dispatch_request_for_content_reader(Request& req, Response& res,
        ContentReader content_reader,
        HandlersForContentReader& handlers);

    bool parse_request_line(const char* s, Request& req);
    bool write_response(Stream& strm, bool last_connection, const Request& req,
        Response& res);
    bool write_content_with_provider(Stream& strm, const Request& req,
        Response& res, const std::string& boundary,
        const std::string& content_type);
    bool read_content(Stream& strm, Request& req, Response& res);
    bool
        read_content_with_content_receiver(Stream& strm, Request& req, Response& res,
            ContentReceiver receiver,
            MultipartContentHeader multipart_header,
            ContentReceiver multipart_receiver);
    bool read_content_core(Stream& strm, Request& req, Response& res,
        ContentReceiver receiver,
        MultipartContentHeader mulitpart_header,
        ContentReceiver multipart_receiver);

    std::atomic<bool> is_running_;

    std::vector<std::pair<std::string, std::string>> base_dirs_;
    std::map<std::string, std::string> file_extension_and_mimetype_map_;
    Handler file_request_handler_;
    Handlers get_handlers_;
    Handlers post_handlers_;
    HandlersForContentReader post_handlers_for_content_reader_;
    Handlers put_handlers_;
    HandlersForContentReader put_handlers_for_content_reader_;
    Handlers patch_handlers_;
    HandlersForContentReader patch_handlers_for_content_reader_;
    Handlers delete_handlers_;
    HandlersForContentReader delete_handlers_for_content_reader_;
    Handlers options_handlers_;
    Handler error_handler_;
    Expect100ContinueHandler expect_100_continue_handler_;

    std::shared_ptr<asio::io_context> io_context_;
    std::shared_ptr<asio::ip::tcp::acceptor> acceptor_;
};

class Client {
public:
    explicit Client(const std::string& host);

    explicit Client(const std::string& host, int port);

    explicit Client(const std::string& host, int port,
        const std::string& client_cert_path,
        const std::string& client_key_path);

    virtual ~Client();

    virtual bool is_valid() const;

    std::shared_ptr<Response> Get(const char* path);

    std::shared_ptr<Response> Get(const char* path, const Headers& headers);

    std::shared_ptr<Response> Get(const char* path, Progress progress);

    std::shared_ptr<Response> Get(const char* path, const Headers& headers,
        Progress progress);

    std::shared_ptr<Response> Get(const char* path,
        ContentReceiver content_receiver);

    std::shared_ptr<Response> Get(const char* path, const Headers& headers,
        ContentReceiver content_receiver);

    std::shared_ptr<Response>
        Get(const char* path, ContentReceiver content_receiver, Progress progress);

    std::shared_ptr<Response> Get(const char* path, const Headers& headers,
        ContentReceiver content_receiver,
        Progress progress);

    std::shared_ptr<Response> Get(const char* path, const Headers& headers,
        ResponseHandler response_handler,
        ContentReceiver content_receiver);

    std::shared_ptr<Response> Get(const char* path, const Headers& headers,
        ResponseHandler response_handler,
        ContentReceiver content_receiver,
        Progress progress);

    std::shared_ptr<Response> Head(const char* path);

    std::shared_ptr<Response> Head(const char* path, const Headers& headers);

    std::shared_ptr<Response> Post(const char* path);

    std::shared_ptr<Response> Post(const char* path, const std::string& body,
        const char* content_type);

    std::shared_ptr<Response> Post(const char* path, const Headers& headers,
        const std::string& body,
        const char* content_type);

    std::shared_ptr<Response> Post(const char* path, size_t content_length,
        ContentProvider content_provider,
        const char* content_type);

    std::shared_ptr<Response> Post(const char* path, const Headers& headers,
        size_t content_length,
        ContentProvider content_provider,
        const char* content_type);

    std::shared_ptr<Response> Post(const char* path, const Params& params);

    std::shared_ptr<Response> Post(const char* path, const Headers& headers,
        const Params& params);

    std::shared_ptr<Response> Post(const char* path,
        const MultipartFormDataItems& items);

    std::shared_ptr<Response> Post(const char* path, const Headers& headers,
        const MultipartFormDataItems& items);

    std::shared_ptr<Response> Put(const char* path);

    std::shared_ptr<Response> Put(const char* path, const std::string& body,
        const char* content_type);

    std::shared_ptr<Response> Put(const char* path, const Headers& headers,
        const std::string& body,
        const char* content_type);

    std::shared_ptr<Response> Put(const char* path, size_t content_length,
        ContentProvider content_provider,
        const char* content_type);

    std::shared_ptr<Response> Put(const char* path, const Headers& headers,
        size_t content_length,
        ContentProvider content_provider,
        const char* content_type);

    std::shared_ptr<Response> Put(const char* path, const Params& params);

    std::shared_ptr<Response> Put(const char* path, const Headers& headers,
        const Params& params);

    std::shared_ptr<Response> Patch(const char* path, const std::string& body,
        const char* content_type);

    std::shared_ptr<Response> Patch(const char* path, const Headers& headers,
        const std::string& body,
        const char* content_type);

    std::shared_ptr<Response> Patch(const char* path, size_t content_length,
        ContentProvider content_provider,
        const char* content_type);

    std::shared_ptr<Response> Patch(const char* path, const Headers& headers,
        size_t content_length,
        ContentProvider content_provider,
        const char* content_type);

    std::shared_ptr<Response> Delete(const char* path);

    std::shared_ptr<Response> Delete(const char* path, const std::string& body,
        const char* content_type);

    std::shared_ptr<Response> Delete(const char* path, const Headers& headers);

    std::shared_ptr<Response> Delete(const char* path, const Headers& headers,
        const std::string& body,
        const char* content_type);

    std::shared_ptr<Response> Options(const char* path);

    std::shared_ptr<Response> Options(const char* path, const Headers& headers);

    bool send(const Request& req, Response& res);

    bool send(const std::vector<Request>& requests,
        std::vector<Response>& responses);

    void stop();

    void set_timeout_sec(time_t timeout_sec);

    void set_read_timeout(time_t sec, time_t usec);

    void set_write_timeout(time_t sec, time_t usec);

    void set_keep_alive_max_count(size_t count);

    void set_basic_auth(const char* username, const char* password);

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
    void set_digest_auth(const char* username, const char* password);
#endif

    void set_follow_location(bool on);

    void set_compress(bool on);

    void set_interface(const char* intf);

    void set_proxy(const char* host, int port);

    void set_proxy_basic_auth(const char* username, const char* password);

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
    void set_proxy_digest_auth(const char* username, const char* password);
#endif

protected:
    bool process_request(Stream& strm, const Request& req, Response& res,
        bool last_connection, bool& connection_close);


    const std::string host_;
    const int port_;
    const std::string host_and_port_;

    // Settings
    std::string client_cert_path_;
    std::string client_key_path_;

    time_t timeout_sec_ = 300;
    time_t read_timeout_sec_ = CPPHTTPLIB_READ_TIMEOUT_SECOND;
    time_t read_timeout_usec_ = CPPHTTPLIB_READ_TIMEOUT_USECOND;
    time_t write_timeout_sec_ = CPPHTTPLIB_WRITE_TIMEOUT_SECOND;
    time_t write_timeout_usec_ = CPPHTTPLIB_WRITE_TIMEOUT_USECOND;

    size_t keep_alive_max_count_ = CPPHTTPLIB_KEEPALIVE_MAX_COUNT;

    std::string basic_auth_username_;
    std::string basic_auth_password_;
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
    std::string digest_auth_username_;
    std::string digest_auth_password_;
#endif

    bool follow_location_ = false;

    bool compress_ = false;

    std::string interface_;

    std::string proxy_host_;
    int proxy_port_;

    std::string proxy_basic_auth_username_;
    std::string proxy_basic_auth_password_;
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
    std::string proxy_digest_auth_username_;
    std::string proxy_digest_auth_password_;
#endif


    void copy_settings(const Client& rhs) {
        client_cert_path_ = rhs.client_cert_path_;
        client_key_path_ = rhs.client_key_path_;
        timeout_sec_ = rhs.timeout_sec_;
        read_timeout_sec_ = rhs.read_timeout_sec_;
        read_timeout_usec_ = rhs.read_timeout_usec_;
        write_timeout_sec_ = rhs.write_timeout_sec_;
        write_timeout_usec_ = rhs.write_timeout_usec_;
        keep_alive_max_count_ = rhs.keep_alive_max_count_;
        basic_auth_username_ = rhs.basic_auth_username_;
        basic_auth_password_ = rhs.basic_auth_password_;
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
        digest_auth_username_ = rhs.digest_auth_username_;
        digest_auth_password_ = rhs.digest_auth_password_;
#endif
        follow_location_ = rhs.follow_location_;
        compress_ = rhs.compress_;
        interface_ = rhs.interface_;
        proxy_host_ = rhs.proxy_host_;
        proxy_port_ = rhs.proxy_port_;
        proxy_basic_auth_username_ = rhs.proxy_basic_auth_username_;
        proxy_basic_auth_password_ = rhs.proxy_basic_auth_password_;
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
        proxy_digest_auth_username_ = rhs.proxy_digest_auth_username_;
        proxy_digest_auth_password_ = rhs.proxy_digest_auth_password_;
#endif
    }

private:
    bool read_response_line(Stream& strm, Response& res);
    bool write_request(Stream& strm, const Request& req, bool last_connection);
    bool redirect(const Request& req, Response& res);
    bool handle_request(Stream& strm, const Request& req, Response& res,
        bool last_connection, bool& connection_close);
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
	void init_ssl_context();
#endif

    std::shared_ptr<Response> send_with_content_provider(
        const char* method, const char* path, const Headers& headers,
        const std::string& body, size_t content_length,
        ContentProvider content_provider, const char* content_type);

    template<typename Func>
    void async_process_and_close_socket(Func&& func) {
        if (socket_) {
            func(*socket_);
            // 设置定时器来关闭连接
            timer_.expires_after(std::chrono::seconds(1));
            timer_.async_wait([this](const std::error_code& ec) {
                if (!ec) {
                    socket_.reset(); // 关闭并释放 socket
                }
                });
        }
    }

    virtual bool is_ssl() const;
private:
    void init_connection(const char* host, int port);
    void start_connection();
    void handle_connect(const std::error_code& error);
    void handle_write(const std::error_code& error);
    void handle_read(Response& res, const std::error_code& error);

    std::shared_ptr<asio::io_context> io_context_;
    std::shared_ptr<tcp::resolver> resolver_;
    std::shared_ptr<tcp::resolver::results_type> endpoints_;
    std::shared_ptr<tcp::socket> socket_;
    asio::steady_timer timer_;
};































}; // namespace bre
#endif // !BRE_HTTP_LIB

