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
%type <std::shared_ptr<ProgramNode>>              program
%type <std::vector<std::shared_ptr<ClassNode>>>   class_list
%type <std::shared_ptr<ClassNode>>                class
%type <std::string>                               optional_extends
%type <Khthon::ClassMembers>                      class_body class_content
%type <std::shared_ptr<FieldNode>>                field
%type <std::shared_ptr<MethodNode>>               method
%type <Khthon::Type>                              type
%type <std::vector<std::shared_ptr<FormalNode>>>  formals
%type <std::shared_ptr<FormalNode>>               formal
%type <std::shared_ptr<BlockExpr>>                block
%type <std::shared_ptr<Expr>>                     expression
%type <std::shared_ptr<Expr>>                     literal
%type <std::shared_ptr<StringLiteralExpr>>        string_literal
%type <std::vector<std::shared_ptr<Expr>>>        expression_list
%type <std::shared_ptr<IntegerLiteralExpr>>       integer_literal
%type <std::shared_ptr<BoolLiteralExpr>>          boolean_literal
%type <std::shared_ptr<UnitLiteralExpr>>          unit_literal




// Precedence : defined in descending order
%right      ASSIGN                  // 9
%left       AND                     // 8
%right      NOT                     // 7
%nonassoc   EQUAL LOWER_EQUAL LOWER // 6
%left       PLUS MINUS              // 5
%left       TIMES DIVIDE            // 4
%right      ISNULL                  // 3. 
/*todo remove and check reduce conflicts*/
//%right      "Unary MINUS operator"// 3  // comment out inherited from ABK6 parser 
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
      $$.emplace_back(std::move($1));
    }
  | class_list class 
    {
      $$ = std::move($1);
      $$.emplace_back(std::move($2));
    }
  ;

class
  : CLASS TYPE_IDENTIFIER optional_extends class_body 
    {
      $$ = std::make_shared<ClassNode>(
          @$,
          $2,
          $3,
          std::move($4.fields),
          std::move($4.methods)
      );
    }
  ;

optional_extends
  : %empty 
    {
      $$ = "Object";  // Default parent of any class
    }
  | EXTENDS TYPE_IDENTIFIER 
    {
      $$ = $2;  // works with std::move as well
    }
  ;

class_body
  : LEFT_BRACE class_content RIGHT_BRACE 
    {
      $$ = $2;
    }
  ;

class_content
  : %empty
    {
      // Default-constructed => fields and methods of the class are set to "" 
      $$ = Khthon::ClassMembers();
    }
  | class_content field
    {
      $$ = std::move($1);
      $$.fields.push_back(std::move($2));
    }
  | class_content method
    {
      $$ = std::move($1);
      $$.methods.push_back(std::move($2));
    }
  ;

/*todo: when expr are started, add the optional init-expr here */ 
field
  : OBJECT_IDENTIFIER COLON type SEMICOLON
    {
      $$ = make_shared<FieldNode>(@$, $1, $3);
    }
  ;

/*todo missing BLOCK and FORMALS*/
method
  : OBJECT_IDENTIFIER LEFT_PARENTHESIS formals RIGHT_PARENTHESIS COLON type block 
    {
      $$ = make_shared<MethodNode>(@$, $1, $6, $3, $7);
    }
  ;

formals
  : %empty
    {

    }
  | formal
    {
      $$.push_back(std::move($1));
    }
  | formals COMMA formal 
    {
      $$ = std::move($1);
      $$.push_back(std::move($3));
    }
  ;

formal
  : OBJECT_IDENTIFIER COLON type
    {
      $$ = make_shared<FormalNode>(@$, std::move($1), std::move($3));
    }
  ;

type
  : TYPE_IDENTIFIER { $$ = Khthon::Type(std::move($1)); }
  | INT32           { $$ = Khthon::Type(Khthon::Type::Kind::INT32);  }
  | BOOL            { $$ = Khthon::Type(Khthon::Type::Kind::BOOL);   }
  | STRING          { $$ = Khthon::Type(Khthon::Type::Kind::STRING); }
  | UNIT            { $$ = Khthon::Type(Khthon::Type::Kind::UNIT);   }
  ;

block
  : LEFT_BRACE RIGHT_BRACE
    {
      $$ = std::make_shared<BlockExpr>(@$, std::vector<std::shared_ptr<Expr>>{});
    }
  | LEFT_BRACE expression_list RIGHT_BRACE
    {
      $$ = std::make_shared<BlockExpr>(@$, std::move($2));
    }
  ;


expression_list
  : expression SEMICOLON expression_list
    {
      $$ = std::move($3);
      $$.insert($$.begin(), std::move($1));
    }
  | expression SEMICOLON
    {
      $$.push_back(std::move($1));
    }
  | expression 
    {
      $$.push_back(std::move($1));
    }
  ;

expression
  : literal 
    {
      $$ = $1;
    }
  ;

literal
  : string_literal    { $$ = $1; }
  | integer_literal   { $$ = $1; }
  | boolean_literal   { $$ = $1; }
  | unit_literal      { $$ = $1; }
  ;

string_literal 
  : STRING_LITERAL { $$ = std::make_shared<StringLiteralExpr>(@$, std::move($1)); }
  ;

integer_literal
  : INTEGER_LITERAL { $$ = std::make_shared<IntegerLiteralExpr>(@$, std::move($1)); }
  ;

boolean_literal
  : TRUE    { $$ = std::make_shared<BoolLiteralExpr>(@$, true);  }
  | FALSE   { $$ = std::make_shared<BoolLiteralExpr>(@$, false); }
  ;

unit_literal
  : LEFT_PARENTHESIS RIGHT_PARENTHESIS 
    {
      $$ = std::make_shared<UnitLiteralExpr>(@$);
    }


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
