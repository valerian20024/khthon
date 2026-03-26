// Adapted from https://www.gnu.org/software/bison/manual/html_node/A-Complete-C_002b_002b-Example.html

#include <iostream>
#include <string>

#include "driver.hpp"

using namespace std;

enum class Mode
{
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

    //todo change default to code generation
    Mode mode = Mode::PARSE;
    string source_file;

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
        return -1;

    case Mode::GENERATE:
        return -1;
    
    case Mode::EXTEND:
        cout << "Extended VSOP Mode is not yet supported." << endl;
        return 0;
    
    default:
        return -1;
    }
}
