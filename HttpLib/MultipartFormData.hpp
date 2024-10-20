#ifndef MULTIPART_FORM_DATA_HPP
#define MULTIPART_FORM_DATA_HPP
#include <functional>
#include <string>

namespace bre {

struct MultipartFormData {
    std::string name;
    std::string content;
    std::string filename;
    std::string content_type;
};

} // namespace bre
#endif // !MULTIPART_FORM_DATA_HPP

