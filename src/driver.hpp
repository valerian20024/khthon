#ifndef _DRIVER_HPP
#define _DRIVER_HPP

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "parser.hpp"
#include "ast.hpp"

/*
? Who should have driver as reference to be able to send InternalDiagnostics ?
    Instead, create a DiagnosticHandler class that has the duty to log new diags, etc.
todo add a _ after the name
*/

// Give prototype of yylex() function, then declare it.
// Passing driver as an argument for error reporting.
#define YY_DECL Khthon::Parser::symbol_type yylex(Khthon::Driver &driver)
YY_DECL;

using namespace Khthon;

namespace Khthon {

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

    /**
     * @brief The Driver class acts as a central coordinator. 
     * 
     * It manages the input file, stores state (tokens, AST, ...) 
     * and bridges the lexer/parser.
     */
    class Driver {
    private:
        /// @brief The source file to compile.
        std::string source_file_;

        /// @brief The parser.
        Khthon::Parser *parser;

        /// @brief Stores the tokens.
        std::vector<Parser::symbol_type> tokens;  

        /// @brief Stores all the encountered errors and warnings. 
        std::vector<std::shared_ptr<Diagnostic>> diagnostics_;

        /// @brief The total number of errors encountered during compilation.
        size_t error_count_ = 0;

        /// @brief The total number of warnings encountered during compilation.
        size_t warning_count_ = 0;

        /// @brief Starts the lexer.
        void scan_begin();

        /// @brief Stops the lexer.
        void scan_end();

    public:
        /// @brief Construct a new Driver.
        /// @param source_file The file containing the source code.
        Driver(const std::string &source_file) : source_file_(source_file) {}

        /// @return The compiled source file.
        const std::string source_file() { return source_file_; }

        /// @brief Returns the default location: current_source_file: 1: 1.
        Khthon::location default_location() const;

        /// @brief The root of the AST. Used as a handle to parse the whole tree.
        std::shared_ptr<ProgramNode> ast_root;

        /// @brief Run the lexer on the source file.
        /// @return 0 if no error happened during compilation. Non-zero otherwise.
        int lex();

        /// @brief Run the lexer and the parser on the source file.
        /// @note Requires the lexer to run first.
        /// @return 0 if no error happened during compilation. Non-zero otherwise.
        int parse();

        /// @brief Checks semantics on the source file.
        /// @note Requires the lexer and the parser to run first.
        /// @return 0 if no error happened during compilation. Non-zero otherwise.
        int analyze();

        /// @brief Run the lexer, parser on the source file. Checks semantics and generate intermediate representation.
        /// @note Requires the lexer, parser, and semantic checker to run first.
        /// @return 0 if no error happened during compilation. Non-zero otherwise.
        int generate(bool make_executable = false);

        /// @brief Print all the tokens, that is the output of the lexical analysis
        void print_tokens(std::ostream& out);

        /// @brief Print the abstract syntax tree.
        /// @param annotate: print with or without annotations.
        void print_AST(bool annotate, std::ostream& out);

        /// @brief Adds a new diagnostic to the list.
        void report(std::shared_ptr<Diagnostic> diagnostic);

        /// @brief Sets up the driver to include necessary VSOP extensions.
        /// @note This method is a stub for now.
        void enable_extensions();

        /// @brief Logs an internal error.
        /// @note When debugging is activated, will directly print the error on stderr.
        void internal_error(const std::string& reason);

        /// @brief Helper function to add a new lexical note.
        void lexical_note(const location& l, const std::string& reason);

        /// @brief Helper function to add a new lexical warning.
        void lexical_warning(const location& l, const std::string& reason);

        /// @brief Helper function to add a new lexical error.
        void lexical_error(const location& l, const std::string& reason);

        /// @brief Helper function to add a new syntax note.
        void syntax_note(const location& l, const std::string& reason);

        /// @brief Helper function to add a new syntax warning.
        void syntax_warning(const location& l, const std::string& reason);

        /// @brief Helper function to add a new syntax error.
        void syntax_error(const location& l, const std::string& reason);

        /// @brief Helper function to add a new semantic note.
        void semantic_note(const location& l, const std::string& reason);

        /// @brief Helper function to add a new semantic warning.
        void semantic_warning(const location& l, const std::string& reason);

        /// @brief Helper function to add a new semantic error.
        void semantic_error(const location& l, const std::string& reason);        
        
        /// @brief Print all the diagnostics.
        void print_diagnostics() const;
    };
}

#endif
