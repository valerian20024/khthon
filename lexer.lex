    /* This flex/bison example is provided to you as a starting point for your
     * assignment. You are free to use its code in your project.
     *
     * This example implements a simple calculator. You can use the '-l' flag to
     * list all the tokens found in the source file, and the '-p' flag (or no flag)
     * to parse the file and to compute the result.
     *
     * Also, if you have any suggestions for improvements, please let us know.
     */

%{
    /* Includes */
    #include <string>

    #include "parser.hpp"
    #include "driver.hpp"
%}

    /* Flex options
     * - noyywrap: yylex will not call yywrap() function
     * - nounput: do not generate yyunput() function
     * - noinput: do not generate yyinput() function
     * - batch: tell Flex that the lexer will not often be used interactively
     */
%option noyywrap nounput noinput batch

%{
    /* Code to include at the beginning of the lexer file. */
    using namespace std;
    using namespace VSOP;

    // Create a new NUMBER token from the value s.
    Parser::symbol_type make_NUMBER(const string &s,
                                    const location &loc);

    // Print an lexical error message.
    static void print_error(const position &pos,
                            const string &m);

    // Code run each time a pattern is matched.
    #define YY_USER_ACTION  loc.columns(yyleng);

    // Global variable used to maintain the current location.
    location loc;
%}

    /* Definitions */
id    [a-zA-Z][a-zA-Z_0-9]*
int   [0-9]+
blank [ \t\r]

LOWERCASE_LETTER                [a-z]
UPPERCASE_LETTER                [A-Z]
LETTER                          [a-zA-Z]

BINARY_DIGIT                    [01]
DECIMAL_DIGIT                   [0-9]
HEXADECIMAL_DIGIT               [0-9a-fA-F]

WHITESPACE                      [ \t\n\f\r]
WHITESPACE_NO_LF                [ \t\f\r]

COMMENT_START                   "(*"
COMMENT_END                     "*)"
COMMENT_SL                      "//".*  /*todo check this*/

INTEGER_LITERAL_DECIMAL         {DECIMAL_DIGIT}+
INTEGER_LITERAL_HEXADECIMAL     0x{HEXADECIMAL_DIGIT}+

    /* Keywords */
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

TYPE_IDENTIFIER                 {UPPERCASE_LETTER}({LETTER}|{DECIMAL_DIGIT}|_)*

OBJECT_IDENTIFIER               {LOWERCASE_LETTER}({LETTER}|{DECIMAL_DIGIT}|_)*

STRING_START                    "\""
STRING_END                      "\""
STRING_BREAK                    \\\n({WHITESPACE_NO_LF})*
ESCAPED_HEX                     \\x{HEXADECIMAL_DIGIT}{HEXADECIMAL_DIGIT}
ESCAPED_N                       \\n
ESCAPED_T                       \\t
ESCAPED_B                       \\b
ESCAPED_R                       \\r
ESCAPED_QUOTES                  \\\"
ESCAPED_BACKSLASH               \\\\

    /* Operators */
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

/*todo understand Loup's error catcher*/
OTHER			[^a-zA-Z0-9\t\n\r\f*"{}():;,-/\^.=<]


%%
%{
    // Code run each time yylex is called.
    // Prepare location for new token
    loc.step();
%}

    /* White spaces */
{blank}+    loc.step();
\n+         loc.lines(yyleng); loc.step();


    /* Keywords */
{AND}                       return Parser::make_AND(loc);
{BOOL}                      return Parser::make_BOOL(loc);
{CLASS}                     return Parser::make_CLASS(loc);
{DO}                        return Parser::make_DO(loc);
{ELSE}                      return Parser::make_ELSE(loc);
{EXTENDS}                   return Parser::make_EXTENDS(loc);
{FALSE}                     return Parser::make_FALSE(loc);
{IF}                        return Parser::make_IF(loc);
{IN}                        return Parser::make_IN(loc);
{INT32}                     return Parser::make_INT32(loc);
{ISNULL}                    return Parser::make_ISNULL(loc);
{LET}                       return Parser::make_LET(loc);
{NEW}                       return Parser::make_NEW(loc);
{NOT}                       return Parser::make_NOT(loc);
{SELF}                      return Parser::make_SELF(loc);
{STRING}                    return Parser::make_STRING(loc);
{THEN}                      return Parser::make_THEN(loc);
{TRUE}                      return Parser::make_TRUE(loc);
{UNIT}                      return Parser::make_UNIT(loc);
{WHILE}                     return Parser::make_WHILE(loc);

    /* Operators */
{LEFT_BRACE}                return Parser::make_LEFT_BRACE(loc);
{RIGHT_BRACE}               return Parser::make_RIGHT_BRACE(loc);
{LEFT_PARENTHESIS}          return Parser::make_LEFT_PARENTHESIS(loc);
{RIGHT_PARENTHESIS}         return Parser::make_RIGHT_PARENTHESIS(loc);
{COLON}                     return Parser::make_COLON(loc);
{SEMICOLON}                 return Parser::make_SEMICOLON(loc);
{COMMA}                     return Parser::make_COMMA(loc);
{PLUS}                      return Parser::make_PLUS(loc);
{MINUS}                     return Parser::make_MINUS(loc);
{TIMES}                     return Parser::make_TIMES(loc);
{DIVIDE}                    return Parser::make_DIVIDE(loc);
{POWER}                     return Parser::make_POWER(loc);
{DOT}                       return Parser::make_DOT(loc);
{EQUAL}                     return Parser::make_EQUAL(loc);
{LOWER}                     return Parser::make_LOWER(loc);
{LOWER_EQUAL}               return Parser::make_LOWER_EQUAL(loc);
{ASSIGN}                    return Parser::make_ASSIGN(loc);


{INTEGER_LITERAL_DECIMAL}  {
    cout << "debug dec " << yytext << endl;
    int val = stoi(yytext, nullptr, 10);
    return Parser::make_INTEGER_LITERAL(val, loc);
}

{INTEGER_LITERAL_HEXADECIMAL}  {
    cout << "debug hex " << yytext << endl;
    int val = stoi(yytext, nullptr, 16);
    return Parser::make_INTEGER_LITERAL(val, loc);
}


{TYPE_IDENTIFIER}   return Parser::make_TYPE_IDENTIFIER(yytext, loc);     

    /* Invalid characters */
.           {
                print_error(loc.begin, "invalid character: " + string(yytext));
                return Parser::make_YYerror(loc);
}
    
    /* End of file */
<<EOF>>     return Parser::make_YYEOF(loc);
%%

/*
Parser::symbol_type make_NUMBER(const string &s,
                                const location& loc)
{
    int n = stoi(s);

    return Parser::make_NUMBER(n, loc);
}
*/

static void print_error(const position &pos, const string &m)
{
    cerr << *(pos.filename) << ":"
         << pos.line << ":"
         << pos.column << ":"
         << " lexical error: "
         << m
         << endl;
}

void Driver::scan_begin()
{
    loc.initialize(&source_file);

    // When no file is provided, defaults to stdin
    if (source_file.empty() || source_file == "-")
        yyin = stdin;
    else if (!(yyin = fopen(source_file.c_str(), "r")))
    {
        cerr << "cannot open " << source_file << ": " << strerror(errno) << '\n';
        exit(EXIT_FAILURE);
    }
}

void Driver::scan_end()
{
    fclose(yyin);
}
