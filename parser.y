%skeleton "lalr1.cc" // -*- C++ -*-
%language "c++"
%require "3.7.5"
%locations

%defines

// Put parser inside a namespace
%define api.namespace {Khthon}

// Give the name of the parser class
%define api.parser.class {Parser}

// Force the token kind enum (used by the lexer) and the symbol kind enum
// (used by the parser) to use the same value for the tokens.
// (e.g. '+' will be represented by the same integer value in both enums.)
%define api.token.raw

// Tokens contain their type, value and location
// Also allow to use the make_TOKEN functions
%define api.token.constructor

// Allow to use C++ objects as semantic values
%define api.value.type variant

// Add some assertions.
%define parse.assert

// C++ code put inside parser.hpp
%code requires {
    #include <string>
    #include <memory>
    #include <optional>
    
    #include "ast.hpp"

    namespace Khthon
    {
        class Driver;
    }
}

// Add an argument to the parser constructor
%parse-param {Khthon::Driver &driver}

// C++ code put inside parser.cpp
%code {
    #include "driver.hpp"
    #include "ast.hpp"
    
    using namespace std;
}

    /*================================================++
    ||                 DEFINITIONS                    ||
    ++================================================*/

%token
    AND                             "and"
    BOOL                            "bool"
    CLASS                           "class"
    DO                              "do"
    ELSE                            "else"
    EXTENDS                         "extends"
    FALSE                           "false"
    IF                              "if"
    IN                              "in"
    INT32                           "int32"
    ISNULL                          "isnull"
    LET                             "let"
    NEW                             "new"
    NOT                             "not"
    SELF                            "self"
    STRING                          "string"
    THEN                            "then"
    TRUE                            "true"
    UNIT                            "unit"
    WHILE                           "while"

    LEFT_BRACE                      "{"
    RIGHT_BRACE                     "}"
    LEFT_PARENTHESIS                "("
    RIGHT_PARENTHESIS               ")"
    COLON                           ":"
    SEMICOLON                       ";"
    COMMA                           ","
    PLUS                            "+"
    MINUS                           "-"
    TIMES                           "*"
    DIVIDE                          "/"
    POWER                           "^"
    DOT                             "."
    EQUAL                           "="
    LOWER                           "<"
    LOWER_EQUAL                     "<="
    ASSIGN                          "<-"
;

// For some symbols, need to store a value
%token <std::string> TYPE_IDENTIFIER "type-identifier"
%token <std::string> OBJECT_IDENTIFIER "object-identifier"
%token <std::string> STRING_LITERAL "string-literal"
%token <int> INTEGER_LITERAL "integer-literal"


%type <std::shared_ptr<ProgramNode>> program
%type <std::vector<std::shared_ptr<Khthon::ClassNode>>> class_list
//%type <std::shared_ptr<ClassNode>> class

// Precedence
%left "+" "-"; // Could also do: %left PLUS MINUS
%left "*" "/";

%%

    /*================================================++
    ||                 GRAMMAR RULE                   ||
    ++================================================*/


%start program;

program: 
    class_list {
        $$ = std::make_shared<ProgramNode>(@$);
        driver.ast_root = $$;
    }
    ;

class_list:
    %empty

%%

    /*================================================++
    ||                  USER CODE                     ||
    ++================================================*/

void Khthon::Parser::error(const location_type& l, const std::string& m)
{
    const position &pos = l.begin;

    cerr << *(pos.filename) << ":"
         << pos.line << ":" 
         << pos.column << ": "
         << m
         << endl;
}
