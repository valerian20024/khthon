// Adapted from https://www.gnu.org/software/bison/manual/html_node/A-Complete-C_002b_002b-Example.html

#include <iostream>
#include <string>

#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/Host.h"
#include "llvm/IR/LLVMContext.h"

#include "driver.hpp"

using namespace std;

enum class Mode {
    LEX,
    PARSE,
    ANALYZE,
    GENERATE,
    EXTEND
};

static const map<string, Mode> flag_to_mode = {
    {"-l", Mode::LEX},
    {"-p", Mode::PARSE},
    {"-c", Mode::ANALYZE},
    {"-i", Mode::GENERATE},
    {"-e", Mode::EXTEND}
};

int main(int argc, char const *argv[]) {

    Mode mode = Mode::GENERATE;
    string source_file;

    // LLVM Verification Code
    cout << "Verifying LLVM connection..." << endl;
    llvm::InitializeAllTargetInfos();
    string targetTriple = llvm::sys::getDefaultTargetTriple();
    cout << "LLVM Default Target Triple: " << targetTriple << endl;
    
    // Create a dummy context to test IR headers
    llvm::LLVMContext context;
    cout << "LLVM Context initialized successfully." << endl;

    if (argc < 2 || argc > 3) {
        cerr << "Usage: " 
             << argv[0]
             << " [-l|-p|-c|-i|-e] <source_file>" 
             << endl;
        return -1;
    }

    if (argc == 3 && flag_to_mode.count(argv[1]) == 0) {
        cerr << "Invalid mode: " 
             << argv[1] 
             << endl;
        return -1;
    }

    if (argc == 2) {
        source_file = argv[1];
    } else {
        mode = flag_to_mode.at(argv[1]);
        source_file = argv[2];
    }

    Khthon::Driver driver = Khthon::Driver(source_file);

    switch (mode) {
    case Mode::LEX:
        return driver.lex();

    case Mode::PARSE:
        return driver.parse();
    
    case Mode::ANALYZE:
        return driver.analyze();

    case Mode::GENERATE:
        return driver.generate();
    
    case Mode::EXTEND:
        cout << "Extended VSOP mode is not yet supported." << endl;
        return 0;
    
    default:
        return -1;
    }
}
