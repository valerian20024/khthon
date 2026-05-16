#ifndef _DRIVER_HPP
#define _DRIVER_HPP

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "parser.hpp"
#include "ast.hpp"
#include "diagnostics.hpp"

/**
 * This file contains the interface of the Driver class.
 * Driver is the central coordinator of the whole program.
 * For the time being, it is also responsible in managing diagnostics.
 */

// Give prototype of yylex() function, then declare it.
// Passing driver as an argument for error reporting.
#define YY_DECL khthon::Parser::symbol_type yylex(khthon::Driver &driver)
YY_DECL;

namespace khthon {

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

        /// @brief Stable pointer for Bison locations
        std::shared_ptr<std::string> filename_ptr_;

        /// @brief The parser.
        khthon::Parser *parser;

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
        Driver(const std::string &source_file);

        /// @return The compiled source file.
        const std::string source_file() { return source_file_; }

        /// @brief Returns the default location: current_source_file: 1: 1.
        khthon::location default_location() const;

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

        /// @brief Print the information about a token.
        void print_token(Parser::symbol_type token, std::ostream& out);

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
