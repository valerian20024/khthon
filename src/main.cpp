// Adapted from https://www.gnu.org/software/bison/manual/html_node/A-Complete-C_002b_002b-Example.html

#include <iostream>
#include <string>

#include "driver.hpp"

using namespace std;

enum class Action {
    LEX,
    PARSE,
    ANALYZE,
    GENERATE,
    NATIVE
};

int main(int argc, char const *argv[]) {

    Action action = Action::NATIVE;
    bool extended_VSOP = false;
    string source_file = {};

    if (argc < 2 || argc > 4) {
        cerr << "Usage: " 
             << argv[0]
             << " [-e] [-l|-p|-c|-i] <source_file>"
             << endl;
        return -1;
    }
    
    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];

        // Handling flags arguments.
        if (arg[0] == '-' && arg.length() == 2) {
            switch (arg[1]) {
                case 'e':   extended_VSOP = true;           break;
                case 'l':   action = Action::LEX;           break;
                case 'p':   action = Action::PARSE;         break;
                case 'c':   action = Action::ANALYZE;       break;
                case 'i':   action = Action::GENERATE;      break;
                default: {
                    cerr << "Unknown flag." << endl;
                    return -1;
                }
            }
        }
        
        // Handling other arguments, i.e., source file name.
        else {
            source_file = arg;
        }
    }

    if (source_file.empty()) {
        cerr << "Error: No source file specified." << endl;
        return -1;
    }

    Khthon::Driver driver(source_file);

    if (extended_VSOP)
        driver.enable_extensions();

    switch (action) {
        case Action::LEX:       return driver.lex();
        case Action::PARSE:     return driver.parse();
        case Action::ANALYZE:   return driver.analyze();
        case Action::GENERATE:  return driver.generate();
        case Action::NATIVE:    return driver.generate(true);
        default: 
            return -1;
    }
}
