#include <iostream>
#include <string>
#include <map>

#include "driver.hpp"
#include "parser.hpp"
#include "ast.hpp"

using namespace std;
using namespace Khthon;

/*

? Should I make a wrapper function for outputting in color?
    E.g., take an input string and return the same string wrapper in ANSI colors
    determine if it's better to output directly with cerr << or
    creating a string then printing. 
        => Responsability of doing what to which function

*/

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

    printDiagnostics();

    // Printing the AST
    PrintVisitor printer;
    string ast_dump = ast_root->accept(printer);
    cout << ast_dump << endl;

    if (error_count_ > 0 || warning_count_ > 0)
        return 1;

    return res;
}

void Driver::print_tokens()
{
    for (auto token : tokens)
        print_token(token);
}

string LexicalDiagnostic::print() const {
    const position& pos = loc_.begin;

    return *pos.filename 
        + ":" 
        + to_string(pos.line) 
        + ":"
        + to_string(pos.column) 
        + ": lexical error: " 
        + reason;
}

string SyntaxDiagnostic::print() const {
    const position& pos = loc_.begin;

    return *pos.filename 
        + ":" 
        + to_string(pos.line)
        + ":"
        + to_string(pos.column)
        + ": syntax error: " 
        + reason;
}

void Driver::report(std::shared_ptr<Diagnostic> d) {
    if (d->level() == ErrorLevel::Error)
        error_count_++;
    if (d->level() == ErrorLevel::Warning)
        warning_count_++;

    diagnostics_.push_back(std::move(d));
}

void Driver::syntaxError(const location& l, const std::string& reason) {
    report(std::make_shared<SyntaxDiagnostic>(l, ErrorLevel::Error, reason));
}

void Driver::lexicalWarning(const location& l, const std::string& reason) {
    report(std::make_shared<SyntaxDiagnostic>(l, ErrorLevel::Warning, reason));
}


//todo sort the errors by line and columns
void Driver::printDiagnostics() const {
    for (const auto& e : diagnostics_) {
        cerr << e->print() << endl;
    }

    if (error_count_ > 0)
        cerr << "There are " << error_count_ << " errors." << endl;
    if (warning_count_ > 0)
        cerr << "There are " << warning_count_ << " warnings." << endl;
}
