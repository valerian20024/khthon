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
    class FieldNode;
    class MethodNode;
    
    // Datastructure to hold fields and methods together when parsing a class
    struct ClassMembers {
        NodeList<FieldNode> fields;
        NodeList<MethodNode> methods;
    };

    /*================================================++
    ||               ABSTRACT CLASSES                 ||
    ++================================================*/

    // Abstract class for visitors.
    template <typename R> class Visitor {
    public:
        // Pure virtual methods and virtual destructor (rule of zero)
        virtual R visit(const ProgramNode& node) = 0;
        virtual R visit(const ClassNode& node) = 0;
        virtual R visit(const FieldNode& node) = 0;
        virtual R visit(const MethodNode& node) = 0;
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

    /*================================================++
    ||                CONCRETE NODES                  ||
    ++================================================*/

    class ProgramNode : public Node {
    private:
        NodeList<ClassNode> classes_;
    public:
        // Constructor
        ProgramNode(Khthon::location l, NodeList<ClassNode> cs) : Node(l), classes_(std::move(cs)) { }

        // Getter
        const NodeList<ClassNode>& classes() const { return classes_; }
        
        // Accept visitor
        std::string accept(Visitor<std::string>& v) const override { return v.visit(*this); }
    };


    class ClassNode : public Node {
    private:
        std::string name_;
        std::string parent_;
        NodeList<FieldNode> fields_;
        NodeList<MethodNode> methods_;
    public:
        // Constructor
        ClassNode(Khthon::location l) : Node(l) { }
        ClassNode(Khthon::location l, std::string n, std::string p) : 
            Node(l), name_(std::move(n)), parent_(std::move(p)) { }
        ClassNode(
            Khthon::location l, 
            std::string n, 
            std::string p,
            NodeList<FieldNode> fs,
            NodeList<MethodNode> ms
        ) : 
            Node(l), name_(std::move(n)), parent_(std::move(p)), 
            fields_(std::move(fs)), methods_(std::move(ms)) { }

        // Getters
        const std::string& name() const { return name_; }
        const std::string& parent() const { return parent_; }
        const NodeList<FieldNode>& fields() const { return fields_; }
        const NodeList<MethodNode>& methods() const { return methods_; }

        // Accept visitors
        std::string accept(Visitor<std::string>& v) const override { 
            return v.visit(*this); 
        }
    };

    class FieldNode : public Node {
    private:
        std::string name_;
        std::string type_;
    public:
        // Constructor
        FieldNode(Khthon::location l) : Node(l) { }
        FieldNode(Khthon::location l, std::string n, std::string t) : 
            Node(l), name_(std::move(n)), type_(std::move(t)) { }
        // Getters
        const std::string& name() const { return name_; }
        const std::string& type() const { return type_; }

        std::string accept(Visitor<std::string>& v) const override {
            return v.visit(*this);
        }
    };

    //! dummy class copy pasted from FieldNode
    class MethodNode : public Node {
    private:
        std::string name_;
        std::string type_;
    public:
        // Constructor
        MethodNode(Khthon::location l) : Node(l) { }
        MethodNode(Khthon::location l, std::string n, std::string t) : 
            Node(l), name_(std::move(n)), type_(std::move(t)) { }
        // Getters
        const std::string& name() const { return name_; }
        const std::string& type() const { return type_; }

        std::string accept(Visitor<std::string>& v) const override {
            return v.visit(*this);
        }
    };

    /*================================================++
    ||               CONCRETE VISITORS                ||
    ++================================================*/

    class PrintVisitor : public Visitor<std::string> {
    private:
        // For handling both fields and methods.
        std::string printList(const NodeList<FieldNode>& items) const {
            (void) items;
            return "LIST OF FIELDS";
        }

        std::string printList(const NodeList<MethodNode>& items) const {
            (void) items;
            return "LIST OF METHODS";
        }

    public:
        std::string visit(const ProgramNode& node) override {
            std::string result;
            const auto& classes = node.classes();

            // Chaining visit to each of the classes
            for (size_t i = 0; i < classes.size(); ++i) {
                if (i > 0)
                    result += ", ";
                result += classes[i]->accept(*this);
            }
            return result;
        }

        std::string visit(const ClassNode& node) override {
            return "Class(" + node.name() + ", " + node.parent() + ", [" +
                printList(node.fields()) + "], [" + printList(node.methods()) + "])";
        }

        std::string visit(const FieldNode& node) override {
            (void) node;
            return "super FIELD ici";
        }

        std::string visit(const MethodNode& node) override {
            (void) node;
            return "super METHOD ici";
        }
    };
}

#endif