#include <iostream>
#include <string>
#include <map>

#include "driver.hpp"
#include "parser.hpp"
#include "ast.hpp"
#include "colors.hpp"
#include "semantics.hpp"

using namespace std;
using namespace Khthon;
using namespace colors;

/*

*/

/// @brief Map a token type to its string representation.
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


int Driver::lex() {
    scan_begin();

    //todo use the new error mecanism instead of the one of the old calc 
    int res = 0;

    while (true)
    {
        Parser::symbol_type token = yylex(*this);

        if ((Parser::token_type)token.type_get() == Parser::token::YYEOF)
            break;

        // Push back the new token with the others, or error
        if ((Parser::token_type)token.type_get() != Parser::token::YYerror)
            tokens.push_back(token);

        else
            res = 1;
    }

    scan_end();
    
    print_diagnostics();

    // All correct tokens must be printed, whatever the error.
    print_tokens(cout);

    bool has_error = error_count_ > 0 || warning_count_ > 0;
    
    return has_error ? 1 : res;
}

int Driver::parse() {
    scan_begin();

    parser = new Parser(*this);    
    int parse_result = parser->parse();
    if (parse_result) 
        internal_error(
            "Driver::parse(): Bison parse returned with:" 
            + to_string(parse_result)
        );
    
    scan_end();
    delete parser;

    bool has_error = error_count_ > 0 || warning_count_ > 0;
    if (has_error) {
        print_diagnostics();
        print_AST(false, cerr);
    } else {
        print_AST(false, cout);
    }

    return has_error;
}

int Driver::analyze() {
    scan_begin();
    parser = new Parser(*this);    
    int parse_result = parser->parse();
    
    if (parse_result) {
        internal_error(
            "Driver::analyze(): Bison parse returned with:" 
            + to_string(parse_result)
        );
    }
    
    scan_end();
    delete parser;

    SemanticChecker checker = SemanticChecker(*this);
    checker.analyze(ast_root);

    bool has_error = error_count_ > 0 || warning_count_ > 0;
    if (has_error) {
        print_diagnostics();
        print_AST(true, cerr);
    } else {
        print_AST(true, cout);
    }

    return has_error;
}

int Driver::generate() {
    return 0;
}

/**
 * @brief Print the information about a token
 *
 * @param token the token
 * todo make it a method of Driver
 */
static void print_token(Parser::symbol_type token, std::ostream& out) {
    position pos = token.location.begin;
    Parser::token_type type = (Parser::token_type)token.type_get();

    out << pos.line << ","
         << pos.column << ","
         << type_to_string.at(type);

    // When token has a value, print it based on its type
    switch (type) {
        case Parser::token::INTEGER_LITERAL: {
            int value = token.value.as<int>();
            out << "," << value;
            break;
        }
        case Parser::token::TYPE_IDENTIFIER: {
            string id = token.value.as<string>();
            out << "," << id;
            break;
        }
        case Parser::token::OBJECT_IDENTIFIER: {
            string id = token.value.as<string>();
            out << "," << id;
            break;
        }
        case Parser::token::STRING_LITERAL: {
            string id = token.value.as<string>();
            out << "," << id;
            break;
        }
        default:
            break;
    }
    out << endl;
}


void Driver::print_tokens(std::ostream& out) {    
    for (auto token : tokens)
        print_token(token, out);
}

void Driver::print_AST(bool annotate, std::ostream& out) {
    PrintVisitor printer(annotate);
    
    if (ast_root) {
        string ast_dump = ast_root->accept(printer);
        out << ast_dump << endl;
    } else {
        internal_error(
            "The ast_root is a null pointer." 
            "Unable to print the ast dump."
        );
    }
}

void Driver::internal_error(const std::string& reason) {
    // Only output internal errors when debugging.
    #ifdef DEBUG
        cerr << internal_banner() << reason << endl;
    #else 
        (void) reason;
    #endif
}

string LexicalDiagnostic::to_string() const {
    return format_location()
        + ": lexical error: \n"
        + header()
        + bold(reason_);
}

string SyntaxDiagnostic::to_string() const {
    return format_location()
        + ": syntax error: \n" 
        + header()
        + bold(reason_);
}

string SemanticDiagnostic::to_string() const {
    return format_location()
        + ": semantic error: \n"
        + header()
        + bold(reason_);
}

string GenerationDiagnostic::to_string() const {
    return format_location()
        + ": generation error: \n"
        + header()
        + bold(reason_);
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

void Driver::semantic_note(const location& l, const std::string& reason) {
    report(make_shared<SemanticDiagnostic>(l, ErrorLevel::Note, reason));
}

void Driver::semantic_warning(const location& l, const std::string& reason) {
    report(make_shared<SemanticDiagnostic>(l, ErrorLevel::Warning, reason));
}

void Driver::semantic_error(const location& l, const std::string& reason) {
    report(make_shared<SemanticDiagnostic>(l, ErrorLevel::Error, reason));
}

void Driver::generation_note(const location& l, const std::string& reason) {
    report(make_shared<GenerationDiagnostic>(l, ErrorLevel::Note, reason));
}

void Driver::generation_warning(const location& l, const std::string& reason) {
    report(make_shared<GenerationDiagnostic>(l, ErrorLevel::Warning, reason));
}

void Driver::generation_error(const location& l, const std::string& reason) {
    report(make_shared<GenerationDiagnostic>(l, ErrorLevel::Error, reason));
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

string Diagnostic::format_location() const {
    const position& pos = loc_.begin;
    string filename = (pos.filename && !pos.filename->empty())
        ? *pos.filename
        : "<unknown>";

    return filename
        + ":"
        + std::to_string(pos.line)
        + ":"
        + std::to_string(pos.column);
}

const string Diagnostic::header() const {
    switch (level_) {
    case ErrorLevel::Error:     return as_error("Error: ");
    case ErrorLevel::Warning:   return as_warning("Warning: ");
    case ErrorLevel::Note:      return as_note("Note: ");
    default:
        cerr << "Error in Diagnostic::header(): No such ErrorLevel.";
        return "";
    }
}
