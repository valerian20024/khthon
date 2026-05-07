#ifndef MANGLING_HPP
#define MANGLING_HPP

#include <string>

namespace Khthon::Mangle {
    std::string method(const std::string& class_name, const std::string& method_name) {
        return class_name + "__" + method_name;
    }

    std::string constructor(const std::string& class_name) {
        return class_name + "___new";
    }

    std::string initilizer(const std::string& class_name) {
        return class_name + "___init";
    }

    std::string vtable_global(const std::string& class_name) {
        return class_name + "___vtable";
    }

    std::string vtable_type(const std::string& class_name) {
        return class_name + "___vtable_type";
    }



} // namespace Khthon::Mangle

#endif
