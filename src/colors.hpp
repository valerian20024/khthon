#include <string>

/// @brief Contains methods printing in the terminal.
namespace colors {

    constexpr bool enable_colors =
#ifdef DEBUG
    true;   // colors ON when debugging
#else
    false;  // colors OFF when submitting 
#endif

    /// @brief Applies ANSI color codes `code` to `text`.
    /// @note Automatically resets colors at the end.
    /// @note Only does so when DEBUG is set. Otherwise returns the plain text.
    inline std::string with(const std::string code, const std::string& text) {
        if constexpr (enable_colors)
            return code + text + "\033[0m";
        else
            return text;
    }

    inline std::string bold(const std::string& s)       { return with("\033[1m", s); }
    inline std::string underlined(const std::string& s) { return with("\033[4m", s); }

    inline std::string red(const std::string& s)        { return with("\033[31m", s); }
    inline std::string yellow(const std::string& s)     { return with("\033[33m", s); }
    inline std::string cyan(const std::string& s)       { return with("\033[36m", s); }
    inline std::string white(const std::string& s)      { return with("\033[37m", s); }
    inline std::string bright_red(const std::string& s) { return with("\033[91m", s); }

    inline std::string as_error(const std::string& s)   { return bright_red(bold(s)); }
    inline std::string as_warning(const std::string& s) { return yellow(bold(s)); }
    inline std::string as_note(const std::string& s)    { return cyan(bold(s)); }

    inline std::string internal_error_banner() {
        return colors::bright_red("\n"
                "╔══════════════════════════════════════════════════════════════╗\n"
                "║                    INTERNAL COMPILER ERROR                   ║\n"
                "╚══════════════════════════════════════════════════════════════╝\n"
        );
    }
}