#ifndef DATA_SINK_HPP
#define DATA_SINK_HPP

#include <functional>
#include <iostream>

namespace bre {

class DataSink {
public:
    DataSink() : os(&sb_), sb_(*this) {}

    DataSink(const DataSink&) = delete;
    DataSink& operator=(const DataSink&) = delete;
    DataSink(DataSink&&) = delete;
    DataSink& operator=(DataSink&&) = delete;

    std::function<void(const char* data, size_t data_len)> write;
    std::function<void()> done;
    std::function<bool()> is_writable;
    std::ostream os;

private:
    class data_sink_streambuf : public std::streambuf {
    public:
        data_sink_streambuf(DataSink& sink) : sink_(sink) {}

    protected:
        std::streamsize xsputn(const char* s, std::streamsize n) {
            sink_.write(s, static_cast<size_t>(n));
            return n;
        }

    private:
        DataSink& sink_;
    };

    data_sink_streambuf sb_;
};
} // namespace bre
#endif // !DATA_SINK_HPP