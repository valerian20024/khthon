#ifndef _DRIVER_HPP
#define _DRIVER_HPP

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "parser.hpp"
#include "ast.hpp"

// Give prototype of yylex() function, then declare it.
#define YY_DECL Khthon::Parser::symbol_type yylex()
YY_DECL;

using namespace Khthon;

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
            location location,
            ErrorLevel level
        ) : 
            loc_(std::move(location)), 
            level_(std::move(level)) 
        {}

    public:
        virtual ~Diagnostic() = default;
        
        virtual std::string print() const = 0;

        const location& loc() const { return loc_; }
        const ErrorLevel& level() const { return level_; };
    };


    class LexicalDiagnostic : public Diagnostic {
    private:
        std::string reason;
    public:
        LexicalDiagnostic(
            location l,
            ErrorLevel e,
            std::string r
        ) : 
            Diagnostic(l, e),
            reason(std::move(r))
        {}
    
        std::string print() const override;
    };

    class SyntaxDiagnostic : public Diagnostic {
    private:
        std::string reason;
    public:
        SyntaxDiagnostic(
            location l,
            ErrorLevel e,
            std::string r
        ) : 
            Diagnostic(l, e),
            reason(std::move(r))
        {}
    
        std::string print() const override;
    };


    /*
    The Driver class acts as a central coordinator. 
    It manages the input file, stores state (like variables for the calculator),
    and bridges the lexer/parser 
    */
    class Driver
    {
    public:
        /**
         * @brief Construct a new Driver.
         *
         * @param _source_file The file containing the source code.
         */
        Driver(const std::string &_source_file) : source_file(_source_file) {}

        /**
         * @brief Get the source file.
         *
         * @return const std::string& The source file.
         */
        const std::string &get_source_file() { return source_file; }

        /**
         * @brief Add a new integer variable.
         *
         * @param name The name of the variable.
         * @param value The value of the variable.
         */
        void add_variable(std::string name, int value) { variables[name] = value; }

        /**
         * @brief Check if a variable exists.
         *
         * @param name The name of the variable.
         *
         * @return true If the variable exists.
         * @return false If the variable does not exist.
         */
        bool has_variable(std::string name) { return variables.count(name); }

        /**
         * @brief Get the interger value of a variable.
         *
         * @param name The name of the variable.
         *
         * @return int The value of the variable.
         */
        int get_variable(std::string name) { return variables.at(name); }

        /**
         * @brief Run the lexer on the source file.
         *
         * @return int 0 if no lexical error.
         */
        int lex();

        /**
         * @brief Run the parser on the source file and compute the result.
         *
         * @return int 0 if no syntax or lexical error.
         */
        int parse();

        /**
         * @brief Print all the tokens, that is the output of the lexical analysis
         */
        void print_tokens();

        /**
         * @brief The result of the computation.
         */
        int result;

        /**
         * @brief The root of the AST. Used as a handle to parse the whole tree.
         */
        std::shared_ptr<ProgramNode> ast_root;

        /**
         * @brief Adds a new diagnostic to the list.
         */
        void report(std::shared_ptr<Diagnostic> diagnostic);

        /**
         * @brief Helper function to add a new lexical note.
         */
        void lexicalNote(const location& l, const std::string& reason);

        /**
         * @brief Helper function to add a new lexical warning.
         */
        void lexicalWarning(const location& l, const std::string& reason);

        /**
         * @brief Helper function to add a new lexical error.
         */
        void lexicalError(const location& l, const std::string& reason);

        /**
         * @brief Helper function to add a new syntax note.
         */
        void syntaxNote(const location& l, const std::string& reason);

        /**
         * @brief Helper function to add a new syntax warning.
         */
        void syntaxWarning(const location& l, const std::string& reason);

        /**
         * @brief Helper function to add a new syntax error.
         */
        void syntaxError(const location& l, const std::string& reason);
        
        /**
         * @brief Prints all the diagnostics.
         */
        void printDiagnostics() const;

    private:
        /**
         * @brief The source file.
         */
        std::string source_file;

        /**
         * @brief The parser.
         */
        Khthon::Parser *parser;

        /**
         * @brief Store the variables (names + values).
         */
        std::map<std::string, int> variables;

        /**
         * @brief Stores the tokens.
         */
        std::vector<Parser::symbol_type> tokens;  //todo add a _ after the name

        /**
         * @brief Stores all the encountered errors and warnings. 
         */
        std::vector<std::shared_ptr<Diagnostic>> diagnostics_;

        /**
         * @brief The total number of errors encountered during compilation.
         */
        size_t error_count_ = 0;

        /**
         * @brief The total number of warnings encountered during compilation.
         */
        size_t warning_count_ = 0;

        /**
         * @brief Start the lexer.
         */
        void scan_begin();

        /**
         * @brief Stop the lexer.
         */
        void scan_end();
    };
}

#endif
