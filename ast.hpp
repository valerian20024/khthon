#ifndef AST_HPP
#define AST_HPP

#include <memory>       // for smart pointers
#include <string>       // for std::string
#include <vector>       // for std::vector
#include <optional>     // for std::optional
#include "parser.hpp"   // for Bison location

namespace Khthon {

    template <typename T> using NodeList = std::vector<std::shared_ptr<T>>;

    // Forward declarations for Visitor. Avoid circular dependencies. 
    class ProgramNode;
    class ClassNode;

    // Abstract class for visitors.
    template <typename R> class Visitor {
    public:
        // Pure virtual methods and virtual destructor (rule of zero)
        virtual R visit(const ProgramNode& node) = 0;
        virtual R visit(const ClassNode& node) = 0;
        virtual ~Visitor() = default;
    };

    // Abstract class for nodes. 
    class Node {
    private:
        location loc_;
    public:
        virtual void accept(Visitor<std::string>& v) const = 0;
        virtual ~Node() = default;
        location location() const { return loc_; }
    };

    class ProgramNode : public Node {
    private:
        NodeList<ClassNode> classes_;
    public:
        // Constructor
        ProgramNode(NodeList<ClassNode> cs) : classes_(std::move(cs)) { }
        // Getter
        const NodeList<ClassNode>& classes() const { return classes_; }
        // Accept visitor
        void accept(Visitor<std::string>& v) const override { v.visit(*this); }
    };

    class ClassNode : public Node {
    private:
        std::string name;
    };

}

#endif