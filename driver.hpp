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


    class CompilerError {
    protected:
        location loc_;

    public:
    //todo make it protected. Only child classes should be able to create it.
        CompilerError(location l) : loc_(std::move(l)) {}  
        virtual ~CompilerError() = default;

        virtual ErrorLevel level() const = 0;
        virtual std::string print() const = 0;

        const location& loc() const { return loc_; }
    };


    class LexicalError : public CompilerError {
    private:
        std::string reason;
    public:
        LexicalError(
            location l,
            std::string r
        ) : 
            CompilerError(l),
            reason(std::move(r))
        {}
    
        ErrorLevel level() const override { return ErrorLevel::Error; }
        std::string print() const override;
    };

    class SyntaxError : public CompilerError {
    private:
        std::string reason;
    public:
        SyntaxError(
            location l,
            std::string r
        ) : 
            CompilerError(l),
            reason(std::move(r))
        {}
    
        ErrorLevel level() const override { return ErrorLevel::Error; }
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
         * @brief Prints syntax errors.
         */
        void error(const Khthon::location& l, const std::string& m);

        /**
         * @brief The result of the computation.
         */
        int result;

        /**
         * @brief The root of the AST. Used as a handle to parse the whole tree.
         */
        std::shared_ptr<ProgramNode> ast_root;

        /**
         * @brief The total number of errors encountered during compilation.
         */
        //todo private ? unsigned ?
        int error_count = 0;

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
         * @brief Store the tokens.
         */
        std::vector<Parser::symbol_type> tokens;

        

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
