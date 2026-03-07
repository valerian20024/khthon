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
    using namespace Khthon;
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

// Tell Bison the semantic type of non-terminals. Uses variant support.
%type <std::shared_ptr<ProgramNode>> program
%type <std::vector<std::shared_ptr<ClassNode>>> class_list
%type <std::shared_ptr<ClassNode>> class

// Precedence : defined in descending order
%right      ASSIGN                  // 9
%left       AND                     // 8
%right      NOT                     // 7
%nonassoc   EQUAL LOWER_EQUAL LOWER // 6
%left       PLUS MINUS              // 5
%left       TIMES DIVIDE            // 4
%right      ISNULL                  // 3. 
//%right      "Unary MINUS operator"// 3
%right      POWER                   // 2
%left       DOT                     // 1

%%

    /*================================================++
    ||                 GRAMMAR RULE                   ||
    ++================================================*/


%start program;

program 
    : class_list 
      {
        $$ = std::make_shared<ProgramNode>(@$, $1);  // std::move($1) seems to work as well
        driver.ast_root = $$;
      }
    ;

class_list
    : class 
      {
        std::cerr << "base case – creating new vector, size will be 1" << endl;
        $$.emplace_back(std::move($1));
      }
    | class_list class 
      {
        std::cerr << "recursive case – previous size = " << $1.size() << endl;
        //$$ = std::move($1);
        //$$.emplace_back(std::move($2));
      }
    ;

class
    : CLASS TYPE_IDENTIFIER optional_extends class_body 
      {
        $$ = std::make_shared<ClassNode>(@$);
      }
    ;

optional_extends
    : %empty 
      {

      }
    | EXTENDS TYPE_IDENTIFIER 
      {

      }
    ;

class_body
    : LEFT_BRACE RIGHT_BRACE {

    }
    ;

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
