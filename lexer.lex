%{
    #include <string>
    #include <stack>

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
    // Code run each time a pattern is matched.
    #define YY_USER_ACTION  loc.columns(yyleng);

    /* Code to include at the beginning of the lexer file. */
    using namespace std;
    using namespace Khthon;
    
    /**
     * @brief Transforms strings of escaped characters into
     * a printable ASCII representation. 
     * 
     * @example Transforms a "\x61" into "a".
     * @example "\x01" is not converted because it is not printable.
     */
    static std::string printable_hex_value(const std::string& hex_string);
    
    /**
     * @brief Prints the whole stack content. Used for debugging nested
     * comments start positions.
     * todo: making it static triggers [-Wunused-function] warning. Make it static without triggering a warning
     */
    void dump_stack_content(std::stack<position> s);

    /**
     * @brief This function simply converts a Bison position to a punctual location.
     */
    inline location point_location(const position& p);

    // Maintains the current location across the code.
    location loc;

    // Stack to track the nested comments number and starting positions.
    stack<position> comments_start_pos;

    // Storing string contentwhen scanning a string.
    string current_string;

    // Remember the current string's start position.
    location string_start_loc;
%}

    /*================================================++
    ||                  DEFINITIONS                   ||
    ++================================================*/

    /* Starting states */
%x COMMENT_STATE STRING_STATE

    /* Basics */
LOWERCASE_LETTER                [a-z]
UPPERCASE_LETTER                [A-Z]
LETTER                          [a-zA-Z]

WHITESPACE_NO_LF                [ \t\f\r]
SPACE                           " "
TABULATION                      \t

COMMENT_START                   "(*"
COMMENT_END                     "*)"
COMMENT_SL                      "//".*
NEWLINE                         \n

    /* Integer literals with error catchers */
HEX_PREFIX                      0[xX]
HEX_DIGIT                       [0-9a-fA-F]
DEC_DIGIT                       [0-9]

INT_LIT_HEX_E_EMPTY             {HEX_PREFIX}
    /* todo find symbols that can be next to an hex lit and add them here vvv*/
INT_LIT_HEX_E_INVALID           {HEX_PREFIX}{HEX_DIGIT}*[^0-9a-fA-F \t\n\r\f;]
INT_LIT_HEX                     {HEX_PREFIX}{HEX_DIGIT}+

INT_LIT_DEC_E_INVALID           {DEC_DIGIT}+[a-zA-Z_][a-zA-Z0-9_]*
INT_LIT_DEC                     {DEC_DIGIT}+

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

    /* Wrong keywords : must not start with uppercase letters */
KEYWORD_E_UPPERCASE             "And"|"Bool"|"Class"|"Do"|"Else"|"Extends"|"False"|"If"|"In"|"Int32"|"Isnull"|"Let"|"New"|"Not"|"Self"|"String"|"Unit"|"While"

    /* Identifiers */
TYPE_IDENTIFIER                 {UPPERCASE_LETTER}({LETTER}|{DEC_DIGIT}|_)*
OBJECT_IDENTIFIER               {LOWERCASE_LETTER}({LETTER}|{DEC_DIGIT}|_)*

    /* String and escaped characters */
STRING_START                    "\""
STRING_END                      "\""
STRING_BREAK                    \\\n({SPACE}|{TABULATION})*
STRING_NORMAL_CHARACTERS        [^"\\\n]+

ESCAPED_HEX                     \\x{HEX_DIGIT}{HEX_DIGIT}
ESCAPED_NEWLINE                 \\n
ESCAPED_TABULATION              \\t
ESCAPED_BACKSPACE               \\b
ESCAPED_RETURN                  \\r
ESCAPED_QUOTES                  \\\"
ESCAPED_BACKSLASH               \\\\

ESCAPED_E                       \\.

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

%%
%{
    /* 
    Code runs each time yylex is called.
    Prepares location for new token.
    It moves loc.begin to where loc.end currently is 
    */
    loc.step();
%}

    /*================================================++
    ||                    RULES                       ||
    ++================================================*/

    /*-------------------------+
    |         INITIAL          |
    +-------------------------*/

<INITIAL>{
    {WHITESPACE_NO_LF}+ {loc.step();}

    {NEWLINE}+ {
        loc.lines(yyleng); 
        loc.step();
    }

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

    {KEYWORD_E_UPPERCASE} {
        driver.syntaxError(loc, "\'" + std::string(yytext) + "\' keyword should start with a lowercase letter");
        return Parser::make_YYerror(loc);
    }

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
    
    /* Identifiers */
    {TYPE_IDENTIFIER}       return Parser::make_TYPE_IDENTIFIER(yytext, loc);     
    {OBJECT_IDENTIFIER}     return Parser::make_OBJECT_IDENTIFIER(yytext, loc);

    /*todo think about out range errors and the likes*/
    {INT_LIT_HEX_E_EMPTY} {
        driver.lexicalError(loc, std::string(yytext) + " is an incomplete hexadecimal literal (missing digits)");
        return Parser::make_YYerror(loc);
    }

    {INT_LIT_HEX_E_INVALID} {
        driver.lexicalError(loc, std::string(yytext) + " is not a valid hexadecimal literal");
        return Parser::make_YYerror(loc);
    }

    {INT_LIT_HEX} {
        int val = stoi(yytext, nullptr, 16);
        return Parser::make_INTEGER_LITERAL(val, loc);
    }

    {INT_LIT_DEC_E_INVALID} {
        driver.lexicalError(loc, std::string(yytext) + " is not a valid integer literal");
        return Parser::make_YYerror(loc);
    }

    {INT_LIT_DEC} {
        int val = stoi(yytext, nullptr, 10);
        return Parser::make_INTEGER_LITERAL(val, loc);
    }

    {STRING_START} {
        string_start_loc = loc;
        
        current_string.clear();
        BEGIN(STRING_STATE);
    }

    {COMMENT_SL} { /* eats up to a newline */ }

    {COMMENT_START} {
        comments_start_pos.push(loc.begin);
        BEGIN(COMMENT_STATE);
    }

    {COMMENT_END} {
        driver.lexicalError(loc, "closing an unmatched opening comment");
        return Parser::make_YYerror(loc);
    }

    /* Invalid characters */
    . {
        driver.lexicalError(loc, "invalid character: " + string(yytext));
        return Parser::make_YYerror(loc);
    }
}

    /*-------------------------+
    |          STRING          |
    +-------------------------*/

<STRING_STATE>{
    {STRING_END} {
        string token_value = "\"" + current_string + "\"";
        location start = string_start_loc;

        current_string.clear();
        BEGIN(INITIAL);
        return Parser::make_STRING_LITERAL(token_value, start);
    }

    {STRING_BREAK} {
        int whitespace_count = yyleng - 2;
        loc.step();
        loc.lines(1);
        loc.columns(whitespace_count);
    }

    {STRING_NORMAL_CHARACTERS} {current_string.append(yytext, yyleng);}

    {ESCAPED_BACKSPACE}     {current_string += "\\x08";}
    {ESCAPED_TABULATION}    {current_string += "\\x09";}
    {ESCAPED_NEWLINE}       {current_string += "\\x0a";}
    {ESCAPED_RETURN}        {current_string += "\\x0d";}
    {ESCAPED_QUOTES}        {current_string += "\\x22";}
    {ESCAPED_BACKSLASH}     {current_string += "\\x5c";}
    {ESCAPED_HEX}           {current_string += printable_hex_value(yytext);}

    {ESCAPED_E} {
        position tmp = loc.begin;
        loc.begin = loc.end;
        loc.begin.column -= yyleng;

        driver.lexicalError(loc, "Unknown escaped sequence.");

        current_string += yytext[1];  // Ignore the \ and add the character to the string

        loc.begin = tmp;

        return Parser::make_YYerror(loc);
    }

    {NEWLINE} {
        driver.lexicalError(point_location(loc.end - 1), "Cannot have a newline inside a string.");
        return Parser::make_YYerror(loc);
    }
    
    <<EOF>> {
        driver.lexicalError(point_location(string_start_loc.begin), "Cannot have EOF inside an unclosed string.");
        BEGIN(INITIAL);
        return Parser::make_YYerror(loc);
    }

    . {
        driver.lexicalError(string_start_loc, "Catched an invalid char in string.");
    }
}

    /*-------------------------+
    |         COMMENT          |
    +-------------------------*/

<COMMENT_STATE>{
    {COMMENT_START} {comments_start_pos.push(loc.begin);}

    {COMMENT_END} {
        comments_start_pos.pop();
        if (comments_start_pos.empty())
            BEGIN(INITIAL);
    }

    {NEWLINE}+ {
        loc.lines(yyleng);
        loc.step();
    }

    . {
        loc.step();
    }

    <<EOF>> {
        if (!comments_start_pos.empty()) {
            driver.lexicalError(point_location(comments_start_pos.top()), "Unmatched comment.");
            BEGIN(INITIAL);
            return Parser::make_YYerror(loc);
        }

        driver.lexicalError(point_location(comments_start_pos.top()), "EOF cannot happen inside an unclosed comment");
        BEGIN(INITIAL);
        return Parser::make_YYerror(loc);
    }
}

    /* 
    Serves as a default case when EOF errors occur in the other
    states. This catches the EOF and allows the lexer to stop.
    */
    <<EOF>>     return Parser::make_YYEOF(loc);
%%

    /*================================================++
    ||                  USER CODE                     ||
    ++================================================*/

void Driver::scan_begin() {
    loc.initialize(&source_file);

    // When no file is provided, defaults to stdin
    if (source_file.empty() || source_file == "-")
        yyin = stdin;
    else if (!(yyin = fopen(source_file.c_str(), "r"))) {
        cerr << "cannot open " << source_file << ": " << strerror(errno) << '\n';
        exit(EXIT_FAILURE);
    }
}

void Driver::scan_end() {
    fclose(yyin);
}

static string printable_hex_value(const string& hex_string) {
    // Verify input
    if (hex_string.size() < 4 || hex_string.substr(0, 2) != "\\x") {
        cerr << "error in printable_hex_value: "
                "incorrect escaped character" 
                << endl;
    }

    // Remove the escaping header ("\x")
    int hex_code = stoi(hex_string.substr(2), nullptr, 16);

    /* 
    Only change the value for printable hexadecimal characters.
    Avoid ASCII non-printable characters and both " and \
    */
    if (hex_code <= 0x1f || hex_code >= 0x7f ||
        hex_code == 0x5c || hex_code == 0x22) {
        return hex_string;
    }

    return string(1, static_cast<char>(hex_code));
}

void dump_stack_content(std::stack<position> s) {
    std::cout << "--- Unclosed comments stack (top) ---" << std::endl;
    while (!s.empty()) {
        std::cout << s.top() << std::endl;
        s.pop();
    }
    std::cout << "-----------------------------------------------" << std::endl;
}

inline location point_location(const position& p) {
    return location(p, p);
}