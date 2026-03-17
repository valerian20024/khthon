%skeleton "lalr1.cc" // -*- C++ -*-
%language "c++"
%require "3.7.5"
%locations

%defines

%define api.namespace {Khthon}

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
%token <std::string>  TYPE_IDENTIFIER     "type-identifier"
%token <std::string>  OBJECT_IDENTIFIER   "object-identifier"
%token <std::string>  STRING_LITERAL      "string-literal"
%token <int>          INTEGER_LITERAL     "integer-literal"

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
%type <std::shared_ptr<IfExpr>>                   if_expr
%type <std::shared_ptr<AssignExpr>>               assign_expr
%type <std::shared_ptr<NewExpr>>                  new_expr
%type <std::shared_ptr<UnOpExpr>>                 unary_operation_expr
%type <std::shared_ptr<BinOpExpr>>                binary_operation_expr
%type <std::shared_ptr<Expr>>                     enclosed_expr
%type <std::shared_ptr<Expr>>                     variable_expr
%type <std::shared_ptr<Expr>>                     call_expr
%type <std::vector<std::shared_ptr<Expr>>>        arg_list


// Precedence : defined in descending order
// UMINUS (unary minus) is only defined to override precedence of binary minus
%right      ASSIGN                    // 9
%left       AND                       // 8
%right      NOT                       // 7
%nonassoc   LOWER LOWER_EQUAL EQUAL   // 6
%left       PLUS MINUS                // 5
%left       TIMES DIVIDE              // 4
%right      ISNULL UMINUS             // 3 

%nonassoc   IF_EXPR

%right      POWER                     // 2
%left       DOT                       // 1

%nonassoc   THEN
%nonassoc   ELSE


%%

    /*================================================++
    ||                 GRAMMAR RULE                   ||
    ++================================================*/


%start program;

program 
  : class_list 
    {
      $$ = std::make_shared<ProgramNode>(@$, $1);
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
      $$ = $2;
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

field
  : OBJECT_IDENTIFIER COLON type SEMICOLON
    {
      $$ = make_shared<FieldNode>(@$, $1, $3);
    }
  | OBJECT_IDENTIFIER COLON type ASSIGN expression SEMICOLON
    {
      $$ = make_shared<FieldNode>(@$, $1, $3, $5);
    }
  ;

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
      $$.insert($$.begin(), std::move($1));  // prepending the new expression
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
  : literal                   { $$ = $1; }
  | if_expr                   { $$ = $1; }
  | assign_expr               { $$ = $1; }
  | new_expr                  { $$ = $1; }
  | unary_operation_expr      { $$ = $1; }
  | binary_operation_expr     { $$ = $1; }
  | enclosed_expr             { $$ = $1; }
  | variable_expr             { $$ = $1; }
  | call_expr                 { $$ = $1; }
  ;

literal
  : string_literal            { $$ = $1; }
  | integer_literal           { $$ = $1; }
  | boolean_literal           { $$ = $1; }
  | unit_literal              { $$ = $1; }
  ;

string_literal 
  : STRING_LITERAL 
    { 
      $$ = std::make_shared<StringLiteralExpr>(@$, std::move($1)); 
    }
  ;

integer_literal
  : INTEGER_LITERAL 
    { 
      $$ = std::make_shared<IntegerLiteralExpr>(@$, std::move($1)); 
    }
  ;

boolean_literal
  : TRUE    
    { 
      $$ = std::make_shared<BoolLiteralExpr>(@$, true);  
    }
  | FALSE   
    { 
      $$ = std::make_shared<BoolLiteralExpr>(@$, false); 
    }
  ;

unit_literal
  : LEFT_PARENTHESIS RIGHT_PARENTHESIS 
    { 
      $$ = std::make_shared<UnitLiteralExpr>(@$); 
    }
  ;

if_expr
  : IF expression THEN expression
    {
      $$ = std::make_shared<IfExpr>(@$, std::move($2), std::move($4));
    }
  | IF expression THEN expression ELSE expression
    {
      $$ = std::make_shared<IfExpr>(@$, std::move($2), std::move($4), std::move($6));
    }
  ;

assign_expr
  : OBJECT_IDENTIFIER ASSIGN expression 
    { 
      $$ = make_shared<AssignExpr>(@$, std::move($1), std::move($3)); 
    }
  ;

new_expr
  : NEW TYPE_IDENTIFIER 
    { 
      $$ = make_shared<NewExpr>(@$, std::move($2)); 
    }
  ;

unary_operation_expr
  : NOT expression
    {
      $$ = make_shared<UnOpExpr>(
        @$, UnaryOperation(UnaryOperation::Kind::NOT), std::move($2)
      );
    }
  | MINUS expression
    {
      $$ = make_shared<UnOpExpr>(
        @$, UnaryOperation(UnaryOperation::Kind::UMINUS), std::move($2)
      );
    }
  | ISNULL expression
    {
      $$ = make_shared<UnOpExpr>(
        @$, UnaryOperation(UnaryOperation::Kind::ISNULL), std::move($2)
      );
    }
  ;

binary_operation_expr
  : expression EQUAL expression
    {
      $$ = make_shared<BinOpExpr>(
        @$, BinaryOperation(BinaryOperation::Kind::EQUAL), std::move($1), std::move($3)
      );
    }
  | expression LOWER expression
    {
      $$ = make_shared<BinOpExpr>(
        @$, BinaryOperation(BinaryOperation::Kind::LOWER), std::move($1), std::move($3)
      );
    }
  | expression LOWER_EQUAL expression
    {
      $$ = make_shared<BinOpExpr>(
        @$, BinaryOperation(BinaryOperation::Kind::LOWER_EQUAL), std::move($1), std::move($3)
      );
    }
  | expression PLUS expression
    {
      $$ = make_shared<BinOpExpr>(
        @$, BinaryOperation(BinaryOperation::Kind::PLUS), std::move($1), std::move($3)
      );
    }
  | expression MINUS expression
    {
      $$ = make_shared<BinOpExpr>(
        @$, BinaryOperation(BinaryOperation::Kind::MINUS), std::move($1), std::move($3)
      );
    }
  | expression TIMES expression
    {
      $$ = make_shared<BinOpExpr>(
        @$, BinaryOperation(BinaryOperation::Kind::TIMES), std::move($1), std::move($3)
      );
    }
  | expression DIVIDE expression
    {
      $$ = make_shared<BinOpExpr>(
        @$, BinaryOperation(BinaryOperation::Kind::DIVIDE), std::move($1), std::move($3)
      );
    }
  | expression POWER expression
    {
      $$ = make_shared<BinOpExpr>(
        @$, BinaryOperation(BinaryOperation::Kind::POWER), std::move($1), std::move($3)
      );
    }
  | expression AND expression
    {
      $$ = make_shared<BinOpExpr>(
        @$, BinaryOperation(BinaryOperation::Kind::AND), std::move($1), std::move($3)
      );
    }
  ;

enclosed_expr
  : LEFT_PARENTHESIS expression RIGHT_PARENTHESIS
    {
      $$ = std::move($2);
    }
  ;

/*todo  there must be something funny going on with the 'self' keyword*/
/*todo  to be checked during semantics*/
variable_expr
  : OBJECT_IDENTIFIER
    {
      $$ = make_shared<VariableExpr>(@$, std::move($1));
    }
  | SELF
    {
      $$ = make_shared<SelfExpr>(@$);
    }
  ;

call_expr
  : expression DOT OBJECT_IDENTIFIER LEFT_PARENTHESIS arg_list RIGHT_PARENTHESIS
      {
        $$ = std::make_shared<CallExpr>(@$, std::move($1), std::move($3), std::move($5));
      }
    | OBJECT_IDENTIFIER LEFT_PARENTHESIS arg_list RIGHT_PARENTHESIS
      {
        auto self = std::make_shared<SelfExpr>(@$);
        $$ = std::make_shared<CallExpr>(@$, std::move(self), std::move($1), std::move($3));
      }
    ;

arg_list
    : %empty
      { 

      }
    | expression
      { 
        $$.push_back(std::move($1)); 
      }
    | arg_list COMMA expression
      { 
        $$ = std::move($1); 
        $$.push_back(std::move($3)); 
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
