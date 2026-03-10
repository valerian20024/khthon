#ifndef AST_HPP
#define AST_HPP

#include <memory>       // for smart pointers
#include <string>       // for std::string
#include <vector>       // for std::vector
#include <optional>     // for std::optional

#include "location.hh"  // for Bison location 

/*
Notes about the whole file

? Should I put back every scope ?
?   that is : Khthon::Type for example

? Should I try to cut the file into .cpp and .hpp
!   Warning: it'll certainly break stuff. But in the end it would be
!   more legible. Keep constructors here and put methods there?

? Is it safe to use a for each loop instead of size_t etc. in printNode methods 
?   of FieldNode and MethodNode

? Is it better to keep make_shared or use unique ptr?
*/

namespace Khthon {

    // Forward declarations for Visitor. Avoid circular dependencies. 
    class ProgramNode;
    class ClassNode;
    class FieldNode;
    class MethodNode;

    template <typename T> using NodeList = std::vector<std::shared_ptr<T>>;
    
    // Datastructure to hold fields and methods together when parsing a class
    struct ClassMembers {
        NodeList<FieldNode> fields;
        NodeList<MethodNode> methods;

        ClassMembers() {}
    };

    // This class holds the possible types of VSOP: both builtin and user-defined
    struct Type {
        enum class Kind { CUSTOM, INT32, BOOL, STRING, UNIT };

        Kind kind = Kind::UNIT;
        std::string custom_name;

        // Default ctor is required by Bison
        Type() = default;

        // Constructor polymorphism allows to construct a new Type conveniently
        explicit Type(Kind k) : kind(k), custom_name("") { }
        explicit Type(std::string name) : kind(Kind::CUSTOM), custom_name(std::move(name)) { }
    };

    /*================================================++
    ||               ABSTRACT CLASSES                 ||
    ++================================================*/

    // Abstract class for visitors.
    template <typename R> class Visitor {
    public:
        virtual R visit(const ProgramNode& node) const = 0;
        virtual R visit(const ClassNode& node) const   = 0;
        virtual R visit(const FieldNode& node) const   = 0;
        virtual R visit(const MethodNode& node) const  = 0;
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

        virtual std::string accept(Visitor<std::string> const& v) const = 0;
        
        Khthon::location location() const { return loc_; }
    };

    /*================================================++
    ||                CONCRETE NODES                  ||
    ++================================================*/

    class ProgramNode : public Node {
    private:
        NodeList<ClassNode> classes_;
    public:
        ProgramNode(
            Khthon::location l, 
            NodeList<ClassNode> cs
        ) : 
            Node(l), 
            classes_(std::move(cs)) 
        { }

        std::string accept(Visitor<std::string> const& v) const override { return v.visit(*this); }
        
        const NodeList<ClassNode>& classes() const { return classes_; }        
    };


    class ClassNode : public Node {
    private:
        std::string name_;
        std::string parent_;
        NodeList<FieldNode> fields_;
        NodeList<MethodNode> methods_;
    public:
        ClassNode(
            Khthon::location l, 
            std::string n, 
            std::string p,
            NodeList<FieldNode> fs,
            NodeList<MethodNode> ms
        ) : 
            Node(l), 
            name_(std::move(n)), 
            parent_(std::move(p)), 
            fields_(std::move(fs)), 
            methods_(std::move(ms)) 
        { }

        std::string accept(Visitor<std::string> const& v) const override { return v.visit(*this); }

        const std::string& name() const { return name_; }
        const std::string& parent() const { return parent_; }
        const NodeList<FieldNode>& fields() const { return fields_; }
        const NodeList<MethodNode>& methods() const { return methods_; }
    };

    class FieldNode : public Node {
    private:
        std::string name_;
        Type type_;
    public:
        FieldNode(
            Khthon::location l, 
            std::string n, 
            Khthon::Type t
        ) :
            Node(l), 
            name_(std::move(n)), 
            type_(std::move(t)) 
        { }
        
        std::string accept(Visitor<std::string> const& v) const override { return v.visit(*this); }

        const std::string& name() const { return name_; }
        const Type& type() const { return type_; }
    };

    class MethodNode : public Node {
    private:
        std::string name_;
        Type type_;
    public:
        MethodNode(
            Khthon::location l, 
            std::string n, 
            Khthon::Type t
        ) : 
            Node(l), 
            name_(std::move(n)), 
            type_(std::move(t)) 
        { }
        
        std::string accept(Visitor<std::string> const& v) const override { return v.visit(*this); }

        const std::string& name() const { return name_; }
        const Type& type() const { return type_; }
    };

    /*================================================++
    ||               CONCRETE VISITORS                ||
    ++================================================*/

    class PrintVisitor : public Visitor<std::string> {
    private:
        std::string printNodeList(const NodeList<FieldNode>& items) const;
        std::string printNodeList(const NodeList<MethodNode>& items) const;
    public:
        std::string visit(const ProgramNode& node) const override;
        std::string visit(const ClassNode& node) const override;
        std::string visit(const FieldNode& node) const override;
        std::string visit(const MethodNode& node) const override;
    };
}

#endif