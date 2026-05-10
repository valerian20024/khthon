#include "diagnostics.hpp"
#include "colors.hpp"

using namespace std;
using namespace colors;

namespace khthon {
    
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
} // namespace khthon
