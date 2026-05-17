#ifndef CONFIG_HPP
#define CONFIG_HPP

/**
 * This file acts as the central point where configuration variables are set.
 * Depending on the compilation type (release or debug), Khthon behaves differently.
 */

namespace khthon {
    
    /// @brief Global logging configuration for debugging.
    inline constexpr bool enable_advanced_logging =
#ifdef DEBUG
    true;
#else
    false;
#endif

    // A place to add more flags in the future...

} // namespace khthon

namespace colors {
    
    /// @brief Colors are only enabled in debug mode to not pollutate the tests output.
    inline constexpr bool enable_colors =
#ifdef DEBUG
    true;
#else
    false;
#endif

}


#endif
