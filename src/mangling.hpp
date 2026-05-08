#ifndef MANGLING_HPP
#define MANGLING_HPP

/**
 * This file defines the various ways of mangling names when generating code.
 */

#include <string>

namespace Khthon::Mangle {

    /// @brief Formats to `class_name__method_name`.
    std::string method(const std::string& class_name, const std::string& method_name) {
        return class_name + "__" + method_name;
    }

    /// @brief Formats to `class_name___new`.
    std::string constructor(const std::string& class_name) {
        return class_name + "___new";
    }

    /// @brief Formats to `class_name___init`.
    std::string initilizer(const std::string& class_name) {
        return class_name + "___init";
    }

    /// @brief Formats to `class_name___vtable`.
    std::string vtable_global(const std::string& class_name) {
        return class_name + "___vtable";
    }

    /// @brief Formats to `class_name___vtable_type`.
    std::string vtable_type(const std::string& class_name) {
        return class_name + "___vtable_type";
    }

} // namespace Khthon::Mangle

#endif
