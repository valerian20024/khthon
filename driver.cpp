#include <iostream>
#include <string>
#include <map>

#include "driver.hpp"
#include "parser.hpp"
#include "ast.hpp"
#include "colors.hpp"

using namespace std;
using namespace Khthon;
using namespace colors;

/*

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
static void print_token(Parser::symbol_type token) {
    position pos = token.location.begin;
    Parser::token_type type = (Parser::token_type)token.type_get();

    cout << pos.line << ","
         << pos.column << ","
         << type_to_string.at(type);

    // When token has a value, print it based on its type
    switch (type) {
        case Parser::token::INTEGER_LITERAL: {
            int value = token.value.as<int>();
            cout << "," << value;
            break;
        }
        case Parser::token::TYPE_IDENTIFIER: {
            string id = token.value.as<string>();
            cout << "," << id;
            break;
        }
        case Parser::token::OBJECT_IDENTIFIER: {
            string id = token.value.as<string>();
            cout << "," << id;
            break;
        }
        case Parser::token::STRING_LITERAL: {
            string id = token.value.as<string>();
            cout << "," << id;
            break;
        }
        default:
            break;
    }
    cout << endl;
}

int Driver::lex() {
    scan_begin();

    //todo use the new error mecanism instead of the one of the old calc 
    int error = 0;

    while (true)
    {
        Parser::symbol_type token = yylex(*this);

        if ((Parser::token_type)token.type_get() == Parser::token::YYEOF)
            break;

        // Push back the new token with the others, or error
        if ((Parser::token_type)token.type_get() != Parser::token::YYerror)
            tokens.push_back(token);

        else
            error = 1;
    }

    scan_end();

    print_diagnostics();

    // Always printing tokens even if there is an error
    print_tokens();

    return error;
}

int Driver::parse() {
    scan_begin();

    parser = new Parser(*this);    
    int res = parser->parse();

    scan_end();

    delete parser;

    print_diagnostics();

    print_AST(false);

    if (error_count_ > 0 || warning_count_ > 0)
        return 1;

    return res;
}

int Driver::analyze() {
    return 0;
}

int Driver::generate() {
    return 0;
}

void Driver::print_tokens() {
    for (auto token : tokens)
        print_token(token);
}

void Driver::print_AST(bool annotate) {
    PrintVisitor printer(annotate);
    string ast_dump = ast_root->accept(printer);
    cout << ast_dump << endl;
}

string LexicalDiagnostic::to_string() const {
    const position& pos = loc_.begin;

    return *pos.filename 
        + ":" 
        + std::to_string(pos.line) 
        + ":"
        + std::to_string(pos.column) 
        + ": lexical error: " 
        + "\n"
        + header()
        + bold(reason);
}

string SyntaxDiagnostic::to_string() const {
    const position& pos = loc_.begin;

    return *pos.filename 
        + ":" 
        + std::to_string(pos.line)
        + ":"
        + std::to_string(pos.column)
        + ": syntax error: " 
        + "\n"
        + header()
        + bold(reason);
}

void Driver::report(std::shared_ptr<Diagnostic> d) {
    if (d->level() == ErrorLevel::Error)
        error_count_++;
    if (d->level() == ErrorLevel::Warning)
        warning_count_++;

    diagnostics_.push_back(std::move(d));
}

void Driver::lexical_note(const location& l, const std::string& reason) {
    report(make_shared<SyntaxDiagnostic>(l, ErrorLevel::Note, reason));
}

void Driver::lexical_warning(const location& l, const std::string& reason) {
    report(make_shared<SyntaxDiagnostic>(l, ErrorLevel::Warning, reason));
}

void Driver::lexical_error(const location& l, const std::string& reason) {
    report(make_shared<LexicalDiagnostic>(l, ErrorLevel::Error, reason));
}

void Driver::syntax_note(const location& l, const std::string& reason) {
    report(make_shared<SyntaxDiagnostic>(l, ErrorLevel::Note, reason));
}

void Driver::syntax_warning(const location& l, const std::string& reason) {
    report(make_shared<SyntaxDiagnostic>(l, ErrorLevel::Warning, reason));
}

void Driver::syntax_error(const location& l, const std::string& reason) {
    report(make_shared<SyntaxDiagnostic>(l, ErrorLevel::Error, reason));
}

//todo sort the errors by line and columns
//todo Then also errors, warnings, notes for a same line
void Driver::print_diagnostics() const {
    for (const auto& d : diagnostics_)
        cerr << d->to_string() << endl;

    if (error_count_ > 0)
        cerr << underlined(to_string(error_count_) + " error(s).") << endl;
    if (warning_count_ > 0)
        cerr << underlined(to_string(warning_count_) + " warning(s)") << endl;
}


const string Diagnostic::header() const {
    //todo Refactor. Use the switch default statement to return an error string
    //todo Simply return in each case.
    string header = "";

    switch (level_) {
    case ErrorLevel::Error:
        header = as_error("Error: ");
        break;
    case ErrorLevel::Warning:
        header = as_warning("Warning: ");
        break;
    case ErrorLevel::Note:
        header = as_note("Note: ");        
    default:
        break;
    }

    return header;
}