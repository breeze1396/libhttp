#ifndef HTTP_CONTENT_HPP
#define HTTP_CONTENT_HPP

#include <asio.hpp>
#include <string>
#include <functional>
#include <iostream>
#include <vector>
#include <memory>

namespace http_asio {

    // ����ContentProvider��ChunkedContentProvider����
    using ContentProvider = std::function<void(size_t offset, size_t max_size, std::function<void(const std::string&)>)>;
    using ChunkedContentProvider = std::function<void(size_t chunk_size, std::function<void(const std::string&)>)>;

    // ContentReader�࣬���ڴ���HTTP���ݶ�ȡ�Ͷಿ������
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

        // �첽��ȡ��������
        void async_readContent(size_t max_size, std::function<void(const std::string&)> handler) {
            if (content_provider_) {
                content_provider_(current_offset_, max_size, handler);
            }
            else {
                handler(""); // ���û�������ṩ�ߣ����ؿ��ַ���
            }
        }

        // �첽�����ಿ������
        void async_parseMultipartData(std::function<void()> handler) {
            auto self = shared_from_this();
            async_readPart([this, self, handler](bool success) {
                if (success) {
                    // �ݹ���ý�����һ����
                    async_parseMultipartData(handler);
                }
                else {
                    handler(); // ������ɻ���ִ���
                }
                });
        }

        // �첽����ֿ鴫��
        void async_handleChunkedTransfer(std::function<void()> handler) {
            if (!chunked_content_provider_) {
                handler();
                return;
            }

            auto self = shared_from_this();
            size_t chunk_size = 4096; // �趨Ĭ�Ͽ��С

            async_readChunk(chunk_size, [this, self, handler](const std::string& chunk) {
                if (chunk.empty()) {
                    handler(); // �������
                    return;
                }

                processChunk(chunk);

                // ������ȡ��һ����
                async_handleChunkedTransfer(handler);
                });
        }

    private:
        // �ڲ�״̬����
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

        // �첽��ȡ�ʹ��������ֵ�����
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
                        handler(false); // ��ȡ����
                    }
                });
        }

        // �첽��ȡһ����
        void async_readChunk(size_t chunk_size, std::function<void(const std::string&)> handler) {
            auto self = shared_from_this();
            asio::async_read(socket_, asio::buffer(buffer_, chunk_size),
                [this, self, handler](const asio::error_code& ec, std::size_t bytes_transferred) {
                    if (!ec) {
                        std::string chunk(buffer_.data(), bytes_transferred);
                        handler(chunk);
                    }
                    else {
                        handler(""); // ��ȡʧ�ܣ����ؿ��ַ���
                    }
                });
        }

        // ����һ�����ݿ�
        void processChunk(const std::string& chunk) {
            // ������Ϳ�����
            std::cout << "Processing chunk of size: " << chunk.size() << std::endl;
        }

        // ����ÿ���ಿ������
        void processPart(const std::string& header, const std::string& content) {
            std::cout << "Processing part with header: " << header << std::endl;
        }

        // α���룺�ӻ�������ȡͷ����Ϣ
        std::string extractHeader() {
            return "Part header data";
        }

        // α���룺�ӻ�������ȡ������
        std::string extractBody(size_t bytes_transferred) {
            return std::string(buffer_.data(), bytes_transferred);
        }

        std::vector<char> buffer_{ 4096 }; // ��ʱ������
    };

} // namespace http_asio

#endif // HTTP_CONTENT_HPP