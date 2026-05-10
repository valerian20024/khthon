#ifndef MANGLING_HPP
#define MANGLING_HPP

/**
 * This file defines the various ways of mangling names when generating code.
 */

#include <string>

namespace mangle {

    /// @brief Formats to `class_name__method_name`.
    std::string meth(const std::string& class_name, const std::string& method_name) {
        return class_name + "__" + method_name;
    }

    /// @brief Formats to `class_name___new`.
    std::string ctor(const std::string& class_name) {
        return class_name + "___new";
    }

    /// @brief Formats to `class_name___init`.
    std::string init(const std::string& class_name) {
        return class_name + "___init";
    }

    /// @brief Formats to `class_name___vtable`.
    std::string vt_global(const std::string& class_name) {
        return class_name + "___vtable";
    }

    /// @brief Formats to `class_name___VTable`.
    std::string vt_struct(const std::string& class_name) {
        return class_name + "___VTable";
    }

} // namespace mangle

#endif
