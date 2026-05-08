#include <filesystem>
#include <iostream>
#include <map>
#include <string>

#include "driver.hpp"
#include "parser.hpp"
#include "ast.hpp"
#include "colors.hpp"
#include "semantics.hpp"
#include "generation.hpp"
#include "diagnostics.hpp"

using namespace std;
using namespace Khthon;
using namespace colors;

/*

*/

namespace Khthon {

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

    Khthon::location Driver::default_location() const {
        Khthon::location loc;

        auto filename_ptr = std::make_shared<string>(source_file_.empty() 
            ? "<unknown>"
            : source_file_);

        loc.begin.filename = filename_ptr.get();
        loc.end.filename   = filename_ptr.get();
    
        loc.begin.line   = 1;
        loc.begin.column = 1;
        loc.end.line     = 1;
        loc.end.column   = 1;

        return loc;
    }

    static string build_executable_name(const string& source_file) {
        std::filesystem::path p(source_file);
        // stem() gives the filename without extension: "bar.vsop" -> "bar"
        // parent_path() gives the directory: "foo/"
        // Combining them gives "foo/bar"
        return (p.parent_path() / p.stem()).string();
    }


    int Driver::lex() {
        int res = 0;

        scan_begin();

        while (true) {
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

        int error = parser->parse();
        if (error) {
            internal_error(
                "Driver::parse(): Bison parse returned with:" 
                + to_string(error)
            );
        }
        
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

        int error = parser->parse();
        if (error) {
            internal_error(
                "Driver::analyze(): Bison parse returned with:" 
                + to_string(error)
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

    int Driver::generate(bool make_executable) {
        scan_begin();
        parser = new Parser(*this);

        int error = parser->parse();
        if (error) {
            internal_error(
                "Driver::analyze(): Bison parse returned with:" 
                + to_string(error)
            );
        }

        scan_end();
        delete parser;

        // Semantic analysis must run first.
        // Codegen assumes the AST is clean.
        SemanticChecker checker(*this);
        checker.analyze(ast_root);

        if (error_count_ > 0) {
            print_diagnostics();
            return 1;
        }

        CodeGenOrchestrator codegen(*this, checker);
        codegen.generate(ast_root);

        bool has_error = error_count_ > 0 || warning_count_ > 0;
        if (has_error) {
            internal_error("There should not be any error left in IR.");
            print_diagnostics();
            return 1;
        }

        // Only output IR.
        if (!make_executable) {
            codegen.print_ir(llvm::outs());
            return 0;
        }

        // Making an executable file.
        
        string ir_file  = "build/output.ll";
        string exe_file = build_executable_name(source_file_);

        cout << exe_file << endl;

        error_code ec;
        llvm::raw_fd_ostream ir_stream(ir_file, ec);
        if (ec) {
            internal_error(
                "generate(): could not open IR temp file: " + ec.message()
            );
            return 1;
        }
        codegen.print_ir(ir_stream);
        ir_stream.flush();

        // RUNTIME_PATH is a macro added at compile time by the Makefile.
        string cmd = "clang -o " 
            + exe_file + " "
            + ir_file + " "
            + RUNTIME_PATH;

        // Executing the command in a shell.
        int status = system(cmd.c_str());
        int exit_code = WEXITSTATUS(status);

        if (exit_code != 0) {
            internal_error(
                "generate(): clang exited with code " 
                + std::to_string(exit_code)
            );
            return 1;
        }

        return 0;
    }

    /**
     * @brief Print the information about a token
     *
     * @param token the token
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

    void Driver::internal_error(const string& reason) {
        if (enable_advanced_logging)
            cerr << internal_error_banner() << reason << endl;
    }

    

    void Driver::report(std::shared_ptr<Diagnostic> d) {
        if (d->level() == ErrorLevel::Error)
            error_count_++;
        if (d->level() == ErrorLevel::Warning)
            warning_count_++;

        diagnostics_.push_back(std::move(d));
    }

    /// @brief Enables Extended VSOP features.
    /// @warning This method is a stub for now.
    void Driver::enable_extensions() {
        return;
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

    //todo sort the errors by line and columns
    //todo Then also errors, warnings, notes for a same line
    //todo Only if not in debug mode. Keep the original order in debug mode
    void Driver::print_diagnostics() const {
        for (const auto& d : diagnostics_)
            cerr << d->to_string() << endl;

        if (error_count_ > 0)
            cerr << underlined(to_string(error_count_) + " error(s).") << endl;
        if (warning_count_ > 0)
            cerr << underlined(to_string(warning_count_) + " warning(s)") << endl;
    }
}