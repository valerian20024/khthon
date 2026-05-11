#ifndef _DIAGNOSTICS_HPP
#define _DIAGNOSTICS_HPP

#include "parser.hpp"

namespace khthon
{
    enum class ErrorLevel {
        Error,
        Warning,
        Note
    };

    class Diagnostic {
    protected:
        location loc_;
        ErrorLevel level_;
        std::string reason_;

        Diagnostic(
            location location = khthon::location(),
            ErrorLevel level = ErrorLevel::Error,
            std::string reason = ""
        ) : 
            loc_(location),
            level_(level),
            reason_(reason)
        {}

        /// @brief Cleanly formats the filename, line and column of the Diagnostic.
        /// @return The location associated to this Diagnostic.
        std::string format_location() const;

    public:
        virtual ~Diagnostic() = default;
        
        /// @brief Compiles all the diagnostic useful informations.
        /// @return The Diagnostic's informations.
        virtual std::string to_string() const = 0;

        /// @brief Formats the header for a Diagnostic.
        /// @return The formatted header.
        const std::string   header() const;
        const location&     loc() const { return loc_; }
        const ErrorLevel&   level() const { return level_; };
    };

    class LexicalDiagnostic : public Diagnostic {
    public:
        LexicalDiagnostic(location l, ErrorLevel e, std::string reason) : 
            Diagnostic(l, e, reason) { }
    
        std::string to_string() const override;
    };

    class SyntaxDiagnostic : public Diagnostic {
    public:
        SyntaxDiagnostic(location l, ErrorLevel e, std::string reason) : 
            Diagnostic(l, e, reason) { }
    
        std::string to_string() const override;
    };
    
    class SemanticDiagnostic : public Diagnostic {
    public:
        SemanticDiagnostic(location l, ErrorLevel e, std::string reason) : 
            Diagnostic(l, e, reason) { }
    
        std::string to_string() const override;
    };
    
} // namespace khthon

#endif
