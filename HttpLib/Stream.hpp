#ifndef STREAM_HPP
#define STREAM_HPP

#include <string>
#include <cstddef>

namespace bre {


class Stream {
public:
    virtual ~Stream() = default;

    virtual bool is_readable() const = 0;
    virtual bool is_writable() const = 0;

    virtual long long read(char* ptr, size_t size) = 0;
    virtual long long write(const char* ptr, size_t size) = 0;
    virtual void get_remote_ip_and_port(std::string& ip, int& port) const = 0;

    template <typename... Args>
    long long write_format(const char* fmt, const Args &... args);
    long long write(const char* ptr);
    long long write(const std::string& s);
};

inline long long Stream::write(const char* ptr) {
    return write(ptr, strlen(ptr));
}

inline long long Stream::write(const std::string& s) {
    return write(s.data(), s.size());
}

template <typename... Args>
inline long long Stream::write_format(const char* fmt, const Args &... args) {
    std::array<char, 2048> buf{};
    auto sn = snprintf(buf.data(), buf.size() - 1, fmt, args...);
    if (sn <= 0) { return sn; }

    size_t n = static_cast<size_t>(sn);

    if (n >= buf.size() - 1) {
        std::vector<char> glowable_buf(buf.size());

        while (n >= glowable_buf.size() - 1) {
            glowable_buf.resize(glowable_buf.size() * 2);
            n = static_cast<size_t>(
                snprintf(&glowable_buf[0], glowable_buf.size() - 1, fmt, args...));
        }
        return write(&glowable_buf[0], n);
    }
    else {
        return write(buf.data(), n);
    }
}

class BufferStream : public Stream {
public:
    BufferStream() = default;
    ~BufferStream() override = default;

    bool is_readable() const override;
    bool is_writable() const override;
    long long read(char* ptr, size_t size) override;
    long long write(const char* ptr, size_t size) override;
    void get_remote_ip_and_port(std::string& ip, int& port) const override;

    const std::string& get_buffer() const;

private:
    std::string buffer;
    size_t position = 0;
};


} // namespace bre

#endif // !STREAM_HPP
