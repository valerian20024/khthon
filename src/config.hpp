#ifndef CONFIG_HPP
#define CONFIG_HPP

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


#endif
