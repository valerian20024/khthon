/* This flex/bison example is provided to you as a starting point for your
 * assignment. You are free to use its code in your project.
 *
 * This example implements a simple calculator. You can use the '-l' flag to
 * list all the tokens found in the source file, and the '-p' flag (or no flag)
 * to parse the file and to compute the result.
 *
 * Also, if you have any suggestions for improvements, please let us know.
 */

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

// C++ code put inside header file
%code requires {
    #include <string>

    namespace Khthon
    {
        class Driver;
    }
}

// Add an argument to the parser constructor
%parse-param {Khthon::Driver &driver}


%code {
    #include "driver.hpp"

    using namespace std;
}

// Token and symbols definitions
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



// Precedence
%left "+" "-"; // Could also do: %left PLUS MINUS
%left "*" "/";

%%
// Grammar rules

%start unit;
unit: CLASS assignments { }
    | assignments  { }

assignments:
    %empty                      {}



/*Generated */
/*
%type <std::shared_ptr<ClassNode>> class
%type <std::vector<std::shared_ptr<MethodNode>>> methods  // List type
%type <std::shared_ptr<MethodNode>> method
%type <std::vector<Formal>> formals formal_list  // Formal is struct {std::string name, type;}
%type <std::optional<std::string>> opt_extends  // For optional "extends"



// Optional extends: Returns string if present, empty optional if not
opt_extends: "extends" TYPE_IDENTIFIER { $$ = $2; }
           | %empty { $$ = std::nullopt; }  // No extends

// Class rule (regular: name; optional: parent; lists: fields/methods assumed similar)
class: "class" TYPE_IDENTIFIER opt_extends "{" methods "}" ";" {
    auto node = std::make_shared<ClassNode>($2, $3.value_or("Object"));  // Regular + optional
    node->methods = $5;  // List set directly (assuming constructor or public setter)
    node->loc = @$;  // Set location
    $$ = node;
}

// Methods list (recursive for lists)
methods: methods method { $1.push_back($2); $$ = $1; }  // Append to list
       | %empty { $$ = {}; }  // Empty vector

// Method rule (regular: name/retType; list: formals; regular: body as BlockNode, assume $7 is shared_ptr<BlockNode>)
method: OBJECT_IDENTIFIER "(" formals ")" ":" TYPE_IDENTIFIER block {
    auto node = std::make_shared<MethodNode>($1, $6, $7);  // Regular members
    node->formals = $3;  // List
    node->loc = @$;
    $$ = node;
}

// Formals list (similar recursive)
formals: formal_list { $$ = $1; }
       | %empty { $$ = {}; }

formal_list: formal_list "," formal { $1.push_back($3); $$ = $1; }
           | formal { $$ = {$1}; }  // Start list with one

// Single formal (struct)
formal: OBJECT_IDENTIFIER ":" TYPE_IDENTIFIER {
    $$ = Formal{$1, $3};  // Simple struct creation
}
*/
%%
// User code
void Khthon::Parser::error(const location_type& l, const std::string& m)
{
    const position &pos = l.begin;

    cerr << *(pos.filename) << ":"
         << pos.line << ":" 
         << pos.column << ": "
         << m
         << endl;
}
