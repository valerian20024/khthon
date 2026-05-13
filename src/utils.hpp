#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <iostream>

namespace khthon::utils {

    /**
     * @brief Transforms strings of escaped characters (ASCII codes) into
     * a printable character.
     * 
     * @example Transforms a C++ string "\\x61" into "a".
     * @example "\x01" is not converted because it is not printable.
     */
    inline std::string to_printable(const std::string& hex_string) {
        // Verify input
        if (hex_string.size() < 4 || hex_string.substr(0, 2) != "\\x") {
            std::cerr << "Error in printable_hex_value: "
                         "incorrect escaped character" 
                      << std::endl;
        }

        // Remove the escaping header: \x
        int hex_code = stoi(hex_string.substr(2), nullptr, 16);
        
        // Only change the value for printable hexadecimal characters.
        // Avoid ASCII non-printable characters and both " and \.
        if (hex_code <= 0x1f || hex_code >= 0x7f ||
            hex_code == 0x5c || hex_code == 0x22
        ) {
            return hex_string;
        }

        return std::string(1, static_cast<char>(hex_code));
    }

} // namespace khthon::utils

#endif
