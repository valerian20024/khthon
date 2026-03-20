#include <iostream>
#include <string>
#include <map>

#include "driver.hpp"
#include "parser.hpp"
#include "ast.hpp"

using namespace std;
using namespace Khthon;

/**
 * @brief Map a token type to a string (pretty-print token types).
 */
static const map<Parser::token_type, string> type_to_string = {
    {Parser::token::AND,                "and"},
    {Parser::token::BOOL,               "bool"},
    {Parser::token::CLASS,              "class"},
    {Parser::token::DO,                 "do"},
    {Parser::token::ELSE,               "else"},
    {Parser::token::EXTENDS,            "extends"},
    {Parser::token::FALSE,              "false"},
    {Parser::token::IF,                 "if"},
    {Parser::token::IN,                 "in"},
    {Parser::token::INT32,              "int32"},
    {Parser::token::ISNULL,             "isnull"},
    {Parser::token::LET,                "let"},
    {Parser::token::NEW,                "new"},
    {Parser::token::NOT,                "not"},
    {Parser::token::SELF,               "self"},
    {Parser::token::STRING,             "string"},
    {Parser::token::THEN,               "then"},
    {Parser::token::TRUE,               "true"},
    {Parser::token::UNIT,               "unit"},
    {Parser::token::WHILE,              "while"},

    {Parser::token::LEFT_BRACE,         "lbrace"},
    {Parser::token::RIGHT_BRACE,        "rbrace"},
    {Parser::token::LEFT_PARENTHESIS,   "lpar"},
    {Parser::token::RIGHT_PARENTHESIS,  "rpar"},
    {Parser::token::COLON,              "colon"},
    {Parser::token::SEMICOLON,          "semicolon"},
    {Parser::token::COMMA,              "comma"},
    {Parser::token::PLUS,               "plus"},
    {Parser::token::MINUS,              "minus"},
    {Parser::token::TIMES,              "times"},
    {Parser::token::DIVIDE,             "div"},
    {Parser::token::POWER,              "pow"},
    {Parser::token::DOT,                "dot"},
    {Parser::token::EQUAL,              "equal"},
    {Parser::token::LOWER,              "lower"},
    {Parser::token::LOWER_EQUAL,        "lower-equal"},
    {Parser::token::ASSIGN,             "assign"},

    {Parser::token::INTEGER_LITERAL,    "integer-literal"},
    {Parser::token::OBJECT_IDENTIFIER,  "object-identifier"},
    {Parser::token::TYPE_IDENTIFIER,    "type-identifier"},
    {Parser::token::STRING_LITERAL,     "string-literal"},
};

/**
 * @brief Print the information about a token
 *
 * @param token the token
 */
static void print_token(Parser::symbol_type token)
{
    position pos = token.location.begin;
    Parser::token_type type = (Parser::token_type)token.type_get();

    cout << pos.line << ","
         << pos.column << ","
         << type_to_string.at(type);

    // When token has a value, print it based on its type
    switch (type)
    {
        case Parser::token::INTEGER_LITERAL:
        {
            int value = token.value.as<int>();
            cout << "," << value;
            break;
        }
        case Parser::token::TYPE_IDENTIFIER:
        {
            string id = token.value.as<string>();
            cout << "," << id;
            break;
        }
        case Parser::token::OBJECT_IDENTIFIER:
        {
            string id = token.value.as<string>();
            cout << "," << id;
            break;
        }
        case Parser::token::STRING_LITERAL:
        {
            string id = token.value.as<string>();
            cout << "," << id;
            break;
        }
        default:
            break;
    }

    cout << endl;
}

int Driver::lex()
{
    scan_begin();

    int error = 0;

    while (true)
    {
        Parser::symbol_type token = yylex();

        if ((Parser::token_type)token.type_get() == Parser::token::YYEOF)
            break;

        // Push back the new token with the others, or error
        if ((Parser::token_type)token.type_get() != Parser::token::YYerror)
            tokens.push_back(token);

        else
            error = 1;
    }

    scan_end();

    return error;
}

int Driver::parse()
{
    scan_begin();

    parser = new Parser(*this);    
    int res = parser->parse();

    scan_end();

    delete parser;

    // Printing the AST
    PrintVisitor printer;
    std::string ast_dump = ast_root->accept(printer);
    cout << ast_dump << endl;

    printDiagnostics();

    if (error_count_ > 0 || warning_count_ > 0) {
        cout << "There are " << error_count_ << " errors." << endl;
        cout << "There are " << warning_count_ << " warnings." << endl;
        return 1;
    }

    return res;
}

void Driver::print_tokens()
{
    for (auto token : tokens)
        print_token(token);
}

//! old method for reporting errors
/*
void Driver::error(const Khthon::location& l, const std::string& m)
{
    const position &pos = l.begin;

    cerr << *(pos.filename) << ":"
         << pos.line << ":" 
         << pos.column << ": "
         << m
         << endl;

    error_count_++;
}
*/

std::string LexicalError::print() const {
    const position& pos = loc_.begin;

    return *pos.filename 
        + ":" 
        + std::to_string(pos.line) 
        + ":"
        + std::to_string(pos.column) 
        + ": lexical error: " 
        + reason;
}

std::string SyntaxError::print() const {
    const position& pos = loc_.begin;

    return *pos.filename 
        + ":" 
        + std::to_string(pos.line) 
        + ":"
        + std::to_string(pos.column) 
        + ": syntax error: " 
        + reason;
}

void Driver::report(std::shared_ptr<CompilerError> diagnostic) {
    if (diagnostic->level() == ErrorLevel::Error)
        error_count_++;
    if (diagnostic->level() == ErrorLevel::Warning)
        warning_count_++;

    diagnostics_.push_back(std::move(diagnostic));
}

void Driver::syntaxError(const location& l, const std::string& reason) {
    report(std::make_shared<SyntaxError>(l, reason));
}

//todo sort the errors by line and columns
void Driver::printDiagnostics() const {
    for (const auto& e : diagnostics_) {
        std::cerr << e->print() << '\n';  //todo endl?
    }
}
