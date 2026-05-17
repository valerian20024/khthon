#ifndef UTILS_HPP
#define UTILS_HPP

/**
 * This file contains definition of utility functions, that may be used from
 * anywhere in the program, but do not belong to a particular place. 
 * For now, it is mainly populated with functions transforming different
 * string representations in VSOP.
 */

#include <string>
#include <iostream>

/// @brief Contains utility functions not belonging to a particular place. 
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

    /**
     * @brief Decodes a string like "string\x0a" to an actual "string\\n".
    */
    inline std::string decode(std::string& raw) {

        // Strip surrounding quotes: "hello" -> hello
        raw = raw.substr(1, raw.size() - 2);
        
        // Decode VSOP escape sequences
        std::string decoded;
        decoded.reserve(raw.size());

        for (size_t i = 0; i < raw.size(); ++i) {
            if (raw[i] == '\\' && i + 1 < raw.size()) {
                switch (raw[i+1]) {
                    case 'n':  decoded += '\n'; i++; break;
                    case 't':  decoded += '\t'; i++; break;
                    case 'r':  decoded += '\r'; i++; break;
                    case '"':  decoded += '"';  i++; break;
                    case '\\': decoded += '\\'; i++; break;
                    case 'x': {
                        // \xNN hex escape
                        if (i + 3 < raw.size()) {
                            std::string hex = raw.substr(i+2, 2);
                            decoded += (char) std::stoi(hex, nullptr, 16);
                            i += 3;
                        }
                        break;
                    }
                    default: decoded += raw[i]; break;
                }
            } else {
                decoded += raw[i];
            }
        }
        return decoded;
    }
    
} // namespace khthon::utils

#endif
