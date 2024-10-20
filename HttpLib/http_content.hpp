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
            }
            else {
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
                }
                else {
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
                    }
                    else {
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
                    }
                    else {
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

        std::vector<char> buffer_{ 4096 }; // 临时缓冲区
    };

} // namespace http_asio

#endif // HTTP_CONTENT_HPP