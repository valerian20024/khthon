#ifndef _DIAGNOSTICS_HPP
#define _DIAGNOSTICS_HPP

#include "parser.hpp"

namespace Khthon
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

        Diagnostic(
            location location = Khthon::location(),
            ErrorLevel level = ErrorLevel::Error
        ) : 
            loc_(std::move(location)), 
            level_(std::move(level)) 
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
        const std::string header() const;
        const location& loc() const { return loc_; }
        const ErrorLevel& level() const { return level_; };
    };

    class LexicalDiagnostic : public Diagnostic {
    private:
        std::string reason_;
    public:
        LexicalDiagnostic(
            location l,
            ErrorLevel e,
            std::string r
        ) : 
            Diagnostic(l, e),
            reason_(std::move(r))
        {}
    
        std::string to_string() const override;
    };

    class SyntaxDiagnostic : public Diagnostic {
    private:
        std::string reason_;
    public:
        SyntaxDiagnostic(
            location l,
            ErrorLevel e,
            std::string r
        ) : 
            Diagnostic(l, e),
            reason_(std::move(r))
        {}
    
        std::string to_string() const override;
    };
    
    class SemanticDiagnostic : public Diagnostic {
    private:
        std::string reason_;
    public:
        SemanticDiagnostic(
            location l,
            ErrorLevel e,
            std::string r
        ) : 
            Diagnostic(l, e),
            reason_(std::move(r))
        {}
    
        std::string to_string() const override;
    };
    
} // namespace Khthon

#endif