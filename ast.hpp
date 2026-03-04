#ifndef AST_HPP
#define AST_HPP

#include <memory>       // for smart pointers
#include <string>       // for std::string
#include <vector>       // for std::vector
#include <optional>     // for std::optional
#include "parser.hpp"   // for Bison location

// Forward declarations for Visitor. Avoid circular dependencies. 
class ProgramNode;
class ClassNode;

template <typename R> class Visitor {
public:
    // Pure virtual methods and virtual destructor (rule of zero)
    virtual R visit(const ProgramNode& node) = 0;
    virtual R visit(const ClassNode& node) = 0;
    virtual ~Visitor() = default;
};

class Node {
private:
    Khthon::location loc;
public:
    virtual void accept(Visitor<std::string>& v) const = 0;
    virtual ~Node() = default;
};



#endif  // AST_HPP