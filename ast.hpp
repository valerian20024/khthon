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

? Is it better to keep make_shared or use unique ptr?

? Visitor<std::string> or PrintVisitor in the methods arguments?
todo put back const std::string before to_string methods of UnOp, BinOp, Type

todo try to wrap with "[" "]" directly inside printNodeList

*/

namespace Khthon {

    // Forward declarations for Visitor. Avoid circular dependencies. 
    class ProgramNode;
    class ClassNode;
    class FieldNode;
    class MethodNode;
    class FormalNode;
    class BlockExpr;
    class StringLiteralExpr;
    class IntegerLiteralExpr;
    class BoolLiteralExpr;
    class UnitLiteralExpr;
    class IfExpr;
    class AssignExpr;
    class NewExpr;
    class UnOpExpr;
    class BinOpExpr;
    class VariableExpr;
    class CallExpr;
    class SelfExpr;
    class LetExpr;

    template <typename T> using NodeList = std::vector<std::shared_ptr<T>>;
    
    // Datastructure to hold fields and methods together when parsing a class
    struct ClassMembers {
        NodeList<FieldNode> fields;
        NodeList<MethodNode> methods;

        ClassMembers() {}
    };


    // This class holds the possible types of VSOP: both builtin and user-defined
    struct Type {
        // todo: add a DEFAULT case as in UnaryOperation
        // todo: change DEFAULT to __DEFAULT__
        enum class Kind { 
            CUSTOM, 
            INT32, 
            BOOL, 
            STRING, 
            UNIT 
        };  

        Kind kind = Kind::UNIT;
        std::string custom_name;

        // Default ctor is required by Bison
        // Constructor polymorphism allows to construct a new Type conveniently
        Type() = default;
        explicit Type(Kind k) : kind(k), custom_name("") { }
        explicit Type(std::string name) : kind(Kind::CUSTOM), custom_name(std::move(name)) { }

        std::string to_string() const;
    };


    // Class for handling unary operations
    struct UnaryOperation {
        enum class Kind { 
            NOT, 
            UMINUS, 
            ISNULL, 
            DEFAULT 
        };  // default is not part of the language

        Kind kind = Kind::DEFAULT;

        UnaryOperation() = default;
        explicit UnaryOperation(Kind k) : kind(k) { }

        std::string to_string() const;
    };


    // Class for handling binary operations
    struct BinaryOperation {
        enum class Kind { 
            EQUAL, 
            LOWER, 
            LOWER_EQUAL, 
            PLUS, 
            MINUS, 
            TIMES, 
            DIVIDE, 
            POWER, 
            AND, 
            DEFAULT 
        };  // default is not part of the language

        Kind kind = Kind::DEFAULT;

        BinaryOperation() = default;
        explicit BinaryOperation(Kind k) : kind(k) { }

        std::string to_string() const;
    };


    /*================================================++
    ||               ABSTRACT CLASSES                 ||
    ++================================================*/

    // Abstract class for visitors.
    template <typename R> class Visitor {
    public:
        virtual R visit(const ProgramNode& node) const = 0;
        virtual R visit(const ClassNode& node) const = 0;
        virtual R visit(const FieldNode& node) const = 0;
        virtual R visit(const MethodNode& node) const = 0;
        virtual R visit(const FormalNode& node) const = 0;
        virtual R visit(const BlockExpr& node) const = 0;
        virtual R visit(const StringLiteralExpr& node) const = 0;
        virtual R visit(const IntegerLiteralExpr& node) const = 0;
        virtual R visit(const BoolLiteralExpr& node) const = 0;
        virtual R visit(const UnitLiteralExpr& node) const = 0;
        virtual R visit(const IfExpr& node) const = 0;
        virtual R visit(const AssignExpr& node) const = 0;
        virtual R visit(const NewExpr& node) const = 0;
        virtual R visit(const UnOpExpr& node) const = 0;
        virtual R visit(const BinOpExpr& node) const = 0;
        virtual R visit(const VariableExpr& node) const = 0;
        virtual R visit(const CallExpr& node) const = 0;
        virtual R visit(const SelfExpr& node) const = 0;
        virtual R visit(const LetExpr& node) const = 0;

        virtual ~Visitor() = default;
    };

    // Abstract class for nodes. 
    class Node {
    private:
        Khthon::location loc_;
    public:
        Node(Khthon::location l) : loc_(l) { }
        virtual ~Node() = default;

        virtual std::string accept(Visitor<std::string> const& v) const = 0;
        
        Khthon::location location() const { return loc_; }
    };

    // Abstract class for expression nodes.
    class Expr : public Node {
    public:
        Expr(Khthon::location l) : Node(l) {}
        virtual ~Expr() = default;
        virtual std::string accept(Visitor<std::string> const& v) const = 0;
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
        std::optional<std::shared_ptr<Expr>> initializer_;
    public:
        FieldNode(
            Khthon::location l, 
            std::string n, 
            Khthon::Type t,
            std::optional<std::shared_ptr<Expr>> i = std::nullopt
        ) : 
            Node(l), 
            name_(std::move(n)), 
            type_(std::move(t)),
            initializer_(std::move(i)) 
        { }
        
        std::string accept(Visitor<std::string> const& v) const override { return v.visit(*this); }

        const std::string& name() const { return name_; }
        const Type& type() const { return type_; }
        bool has_init() const { return initializer_.has_value(); }
        const auto& initializer() const { return initializer_; }

    };


    class MethodNode : public Node {
    private:
        std::string name_;
        Type type_;
        NodeList<FormalNode> formals_;
        std::shared_ptr<Expr> body_;
    public:
        MethodNode(
            Khthon::location l, 
            std::string n, 
            Khthon::Type t,
            NodeList<FormalNode> fs,
            std::shared_ptr<Expr> b
        ) : 
            Node(l), 
            name_(std::move(n)), 
            type_(std::move(t)),
            formals_(std::move(fs)),
            body_(std::move(b))
        { }
        
        std::string accept(Visitor<std::string> const& v) const override { return v.visit(*this); }

        const std::string& name() const { return name_; }
        const Type& type() const { return type_; }
        const NodeList<FormalNode>& formals() const { return formals_; }
        const std::shared_ptr<Expr>& body() const { return body_; }
    };


    class FormalNode : public Node {
    private:
        std::string name_;
        Type type_;
    public:
        FormalNode(
            Khthon::location l,
            std::string n,
            Type t
        ) : 
            Node(l),
            name_(std::move(n)),
            type_(std::move(t))
        { }

        std::string accept(Visitor<std::string> const& v) const override { return v.visit(*this); }

        const std::string& name() const { return name_; }
        const Type& type() const { return type_; }
    };


    class BlockExpr : public Expr {
    private:
        NodeList<Expr> expressions_;
    public:
        BlockExpr(
            Khthon::location l, 
            NodeList<Expr> es
        ) : 
            Expr(l), 
            expressions_(std::move(es)) 
        {}

        const NodeList<Expr>& expressions() const { return expressions_; }
        
        std::string accept(Visitor<std::string> const& v) const override { return v.visit(*this); }
    };


    class StringLiteralExpr : public Expr {
    private:
        std::string value_;
    public:
        StringLiteralExpr(
            Khthon::location l, 
            std::string v
        ) : 
            Expr(l), 
            value_(std::move(v)) 
        {}

        const std::string& value() const { return value_; }

        std::string accept(Visitor<std::string> const& v) const override { return v.visit(*this); }
    };


    class IntegerLiteralExpr : public Expr {
    private:
        int value_;
    public:
        IntegerLiteralExpr(
            Khthon::location l, 
            int val
        ) : 
            Expr(l), 
            value_(val) 
        {}

        std::string accept(Visitor<std::string> const& v) const override { return v.visit(*this); }

        int value() const { return value_; }
    };


    class BoolLiteralExpr : public Expr {
    private:
        bool value_;
    public:
        BoolLiteralExpr(
            Khthon::location l, 
            bool val
        ) : 
            Expr(l), 
            value_(val) 
        {}

        std::string accept(Visitor<std::string> const& v) const override { return v.visit(*this); }

        bool value() const { return value_; }
    };
    
    
    class UnitLiteralExpr : public Expr {
    public:
        explicit UnitLiteralExpr(Khthon::location l) : Expr(l) {}

        std::string accept(Visitor<std::string> const& v) const override { return v.visit(*this); }
    };
    

    class IfExpr : public Expr {
    private:
        std::shared_ptr<Expr> guardian_;
        std::shared_ptr<Expr> consequent_;
        std::optional<std::shared_ptr<Expr>> alternative_;

    public:
        IfExpr(
            Khthon::location l,
            std::shared_ptr<Expr> g,
            std::shared_ptr<Expr> c,
            std::optional<std::shared_ptr<Expr>> a = std::nullopt
        ) : 
            Expr(l),
            guardian_(std::move(g)),
            consequent_(std::move(c)),
            alternative_(std::move(a))
        {}

        std::string accept(Visitor<std::string> const& v) const override { return v.visit(*this); }

        const auto& guardian() const { return guardian_; }
        const auto& consequent() const { return consequent_; }
        const auto& alternative() const { return alternative_; }
    };


    class AssignExpr : public Expr {
    private:
        std::string name_;
        std::shared_ptr<Expr> value_;
    public:
        AssignExpr(
            Khthon::location l,
            std::string n,
            std::shared_ptr<Expr> v
        ) : 
            Expr(l),
            name_(n),
            value_(v)
        {}

        std::string accept(Visitor<std::string> const& v) const override { return v.visit(*this); }
        
        const std::string& name() const { return name_; }
        const auto& value() const { return value_; }
    };


    class NewExpr : public Expr {
    private:
        std::string identifier_;
    public:
        NewExpr(
            Khthon::location l, 
            std::string i
        ) :
            Expr(l),
            identifier_(i)
        {}

        std::string accept(Visitor<std::string> const& v) const override { return v.visit(*this); }

        const std::string& identifier() const { return identifier_; }
    };

    
    class UnOpExpr : public Expr {
    private:
        UnaryOperation operation_;
        std::shared_ptr<Expr> operand_;
    public:
        UnOpExpr(
            Khthon::location l,
            UnaryOperation operation,
            std::shared_ptr<Expr> operand
        ) :
            Expr(l),
            operation_(operation),
            operand_(operand)
        {}

        std::string accept(Visitor<std::string> const& v) const override { return v.visit(*this); }
        
        const UnaryOperation& operation() const { return operation_; }
        const auto& operand() const { return operand_; }
    };


    class BinOpExpr : public Expr {
    private:
        BinaryOperation operation_;
        std::shared_ptr<Expr> left_;
        std::shared_ptr<Expr> right_;

    public:
        BinOpExpr(
            Khthon::location l,
            BinaryOperation operation,
            std::shared_ptr<Expr> left,
            std::shared_ptr<Expr> right
        ) :
            Expr(l),
            operation_(operation),
            left_(left),
            right_(right)
        {}

        std::string accept(Visitor<std::string> const& v) const override { return v.visit(*this); }
        
        const BinaryOperation& operation() const { return operation_; }
        const auto& left() const { return left_; }
        const auto& right() const { return right_; }
    };


    class VariableExpr : public Expr {
    private:
        std::string identifier_;
    public:
        VariableExpr(
            Khthon::location l, 
            std::string i
        ) :
            Expr(l),
            identifier_(i)
        {}

        std::string accept(Visitor<std::string> const& v) const override { return v.visit(*this); }

        const std::string& identifier() const { return identifier_; }
    };


    class CallExpr : public Expr {
    private:
        std::shared_ptr<Expr> receiver_;
        std::string method_name_;
        std::vector<std::shared_ptr<Expr>> arguments_;

    public:
        CallExpr(
            Khthon::location l,
            std::shared_ptr<Expr> r,
            std::string m,
            std::vector<std::shared_ptr<Expr>> as
        ) : 
            Expr(l),
            receiver_(std::move(r)),
            method_name_(std::move(m)),
            arguments_(std::move(as))
        {}

        std::string accept(Visitor<std::string> const& v) const override { return v.visit(*this); }

        const auto& receiver() const { return receiver_; }
        const std::string& name() const { return method_name_; }
        const auto& args() const { return arguments_; }
    };


    class SelfExpr : public Expr {
    public:
        explicit SelfExpr(Khthon::location l) : Expr(l) {}

        std::string accept(Visitor<std::string> const& v) const override { return v.visit(*this); }
    };

    class LetExpr : public Expr {
    private:
        std::string name_;
        Type type_;
        std::shared_ptr<Expr> scope_;
        std::optional<std::shared_ptr<Expr>> initializer_;

    public:
        LetExpr(
            Khthon::location l,
            std::string n,
            Type t,
            std::shared_ptr<Expr> s,
            std::optional<std::shared_ptr<Expr>> i = std::nullopt
        ) : 
            Expr(l),
            name_(std::move(n)),
            type_(std::move(t)),
            scope_(std::move(s)),
            initializer_(std::move(i))
        {}

        std::string accept(Visitor<std::string> const& v) const override { return v.visit(*this); }

        const std::string& name() const { return name_; }
        const Type& type() const { return type_; }
        bool has_initializer() const { return initializer_.has_value(); }
        const auto& initializer() const { return initializer_; }
        const auto& scope() const { return scope_; }
    };


    /*================================================++
    ||               CONCRETE VISITORS                ||
    ++================================================*/

    class PrintVisitor : public Visitor<std::string> {
    private:
        template<typename T> std::string printNodeList(const NodeList<T>& items) const {
            std::string result = "";
            for (size_t i = 0; i < items.size(); ++i) {
                if (i > 0) 
                    result += ", ";
                result += items[i]->accept(*this);
            }
            return result;
        }

    public:
        std::string visit(const ProgramNode& node) const override;
        std::string visit(const ClassNode& node) const override;
        std::string visit(const FieldNode& node) const override;
        std::string visit(const MethodNode& node) const override;
        std::string visit(const FormalNode& node) const override;
        std::string visit(const BlockExpr& node) const override;
        std::string visit(const StringLiteralExpr& node) const override;
        std::string visit(const IntegerLiteralExpr& node) const override;
        std::string visit(const BoolLiteralExpr& node) const override;
        std::string visit(const UnitLiteralExpr&) const override;
        std::string visit(const IfExpr& node) const override;
        std::string visit(const AssignExpr& node) const override;
        std::string visit(const NewExpr& node) const override;
        std::string visit(const UnOpExpr& node) const override;
        std::string visit(const BinOpExpr& node) const override;
        std::string visit(const VariableExpr& node) const override;
        std::string visit(const CallExpr& node) const override;
        std::string visit(const SelfExpr&) const override;
        std::string visit(const LetExpr&) const override;
    };
}

#endif