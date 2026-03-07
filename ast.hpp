#ifndef AST_HPP
#define AST_HPP

#include <memory>       // for smart pointers
#include <string>       // for std::string
#include <vector>       // for std::vector
#include <optional>     // for std::optional

#include "location.hh"  // for Bison location 

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
        Khthon::location loc_;
    public:
        // Constructor and destructor
        Node(Khthon::location l) : loc_(l) { }
        virtual ~Node() = default;
        // Accept visitors
        virtual std::string accept(Visitor<std::string>& v) const = 0;
        // Getters
        Khthon::location location() const { return loc_; }
    };

    class ProgramNode : public Node {
    private:
        NodeList<ClassNode> classes_;
    public:
        // Constructor
        ProgramNode(Khthon::location l, NodeList<ClassNode> cs) : Node(l), classes_(std::move(cs)) { }

        //! Dummy constructor for testing
        ProgramNode(Khthon::location l) : Node(l) {}
        // Getter
        const NodeList<ClassNode>& classes() const { return classes_; }
        // Accept visitor
        std::string accept(Visitor<std::string>& v) const override { return v.visit(*this); }
    };

    class ClassNode : public Node {
    private:
        std::string name_;
        std::string parent_;
        NodeList<Node> fields_;
        NodeList<Node> methods_;
    public:
        // Constructor
        ClassNode(Khthon::location l) : Node(l) { }
        ClassNode(Khthon::location l, std::string n, std::string p = "Object") : Node(l), name_(std::move(n)), parent_(std::move(p)) { }

        // Getters
        const std::string& name() const { return name_; }
        const std::string& parent() const { return parent_; }
        const NodeList<Node>& fields() const { return fields_; }
        const NodeList<Node>& methods() const { return methods_; }

        // Accept visitors
        std::string accept(Visitor<std::string>& v) const override { return v.visit(*this); }
    };

    class PrintVisitor : public Visitor<std::string> {
    private:
        std::string printList(const NodeList<Node>& items) const {
            (void) items;
            return "HEY IM A PLACEHOLDER (for lists)!!";
        }

    public:
        // Visiting ProgramNode
        std::string visit(const ProgramNode& node) override {
            std::string result;
            const auto& classes = node.classes();

            /* debug */
            std::cout << "=> Print visitor has seen a ProgramNode" << std::endl;

            for (size_t i = 0; i < classes.size(); ++i) {
                if (i > 0)
                    result += ", ";
                result += classes[i]->accept(*this);
            }
            return result;
        }

        // Visiting ClassNode
        std::string visit(const ClassNode& node) override {
            return "Class(" + node.name() + ", " + node.parent() + ", [" +
                printList(node.fields()) + "], [" + printList(node.methods()) + "])";
        }
        
    };
}

#endif