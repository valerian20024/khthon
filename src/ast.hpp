#ifndef AST_HPP
#define AST_HPP

#include <memory>
#include <string>
#include <vector>
#include <optional>

#include "location.hh"
#include "visitors.hpp"
#include "types.hpp"
#include "operators.hpp"
#include "llvm_compatibility.hpp"

/**
 * This file contains the interface for the Abstract Syntax Tree nodes.
 */

namespace khthon {

    template <typename T> using NodeList = std::vector<std::shared_ptr<T>>;

    /// @brief Datastructure to hold fields and methods together when 
    /// parsing a class.
    struct ClassMembers {
        NodeList<FieldNode> fields;
        NodeList<MethodNode> methods;
    };

    /// @brief Abstract class for nodes. 
    class Node {
    private:
        khthon::location loc_;
    public:
        Node(khthon::location l) : loc_(l) { }
        virtual ~Node() = default;

        virtual std::string     accept(const Visitor<std::string>&) const = 0;
        virtual void            accept(const Visitor<void>&) const = 0;
        virtual void            accept(MutableVisitor<void>&) = 0;
        virtual llvm::Value*    accept(MutableVisitor<llvm::Value*>&) = 0;

        khthon::location location() const { return loc_; }
    };

    /// @brief Abstract class for expression nodes.
    class Expr : public Node {
    protected:
        khthon::Type type_ = Type();

    public:
        Expr(khthon::location l) : Node(l) { }
        virtual ~Expr() = default;

        virtual std::string     accept(const Visitor<std::string>&) const = 0;
        virtual void            accept(const Visitor<void>&) const = 0;
        virtual void            accept(MutableVisitor<void>&) = 0;
        virtual llvm::Value*    accept(MutableVisitor<llvm::Value*>&) = 0;

        /// @brief Returns the type of this expression. 
        const khthon::Type& type() const { return type_; }

        /// @brief Set type of the node.
        /// @warning Mutates the node even if declared as const.
        void set_type(const khthon::Type& t) { type_ = t; }
    };


    class ProgramNode : public Node {
    private:
        NodeList<ClassNode> classes_;
    public:
        ProgramNode(khthon::location l, NodeList<ClassNode> cs) : 
            Node(l), classes_(std::move(cs)) { }

        std::string accept(Visitor<std::string> const& visitor) const override { 
            return visitor.visit(*this); 
        }
        void accept(Visitor<void> const& visitor) const override { 
            visitor.visit(*this); 
        }
        void accept(MutableVisitor<void>& visitor) override {
            visitor.visit(*this);
        }
        llvm::Value* accept(MutableVisitor<llvm::Value*>& visitor) override {
            return visitor.visit(*this);
        }

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
            khthon::location l, 
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

        std::string accept(Visitor<std::string> const& visitor) const override { 
            return visitor.visit(*this); 
        }
        void accept(Visitor<void> const& visitor) const override { 
            visitor.visit(*this); 
        }
        void accept(MutableVisitor<void>& visitor) override {
            visitor.visit(*this);
        }
        llvm::Value* accept(MutableVisitor<llvm::Value*>& visitor) override {
            return visitor.visit(*this);
        }

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
            khthon::location l, 
            std::string n, 
            khthon::Type t,
            std::optional<std::shared_ptr<Expr>> i = std::nullopt
        ) : 
            Node(l), 
            name_(std::move(n)), 
            type_(std::move(t)),
            initializer_(std::move(i)) 
        { }
        
        std::string accept(Visitor<std::string> const& visitor) const override { 
            return visitor.visit(*this); 
        }
        void accept(Visitor<void> const& visitor) const override { 
            visitor.visit(*this); 
        }
        void accept(MutableVisitor<void>& visitor) override {
            visitor.visit(*this);
        }
        llvm::Value* accept(MutableVisitor<llvm::Value*>& visitor) override {
            return visitor.visit(*this);
        }

        const std::string& name() const     { return name_; }
        const Type& type() const            { return type_; }
        bool has_init() const               { return initializer_.has_value(); }
        const auto& initializer() const     { return initializer_; }
    };


    class MethodNode : public Node {
    private:
        std::string name_;
        Type type_;
        NodeList<FormalNode> formals_;
        std::shared_ptr<Expr> body_;
    public:
        MethodNode(
            khthon::location l, 
            std::string n, 
            khthon::Type t,
            NodeList<FormalNode> fs,
            std::shared_ptr<Expr> b
        ) : 
            Node(l), 
            name_(std::move(n)), 
            type_(std::move(t)),
            formals_(std::move(fs)),
            body_(std::move(b))
        { }
        
        std::string accept(Visitor<std::string> const& visitor) const override { 
            return visitor.visit(*this); 
        }
        void accept(Visitor<void> const& visitor) const override { 
            visitor.visit(*this); 
        }
        void accept(MutableVisitor<void>& visitor) override {
            visitor.visit(*this);
        }
        llvm::Value* accept(MutableVisitor<llvm::Value*>& visitor) override {
            return visitor.visit(*this);
        }

        const std::string& name() const             { return name_; }
        const Type& type() const                    { return type_; }
        const NodeList<FormalNode>& formals() const { return formals_; }
        const std::shared_ptr<Expr>& body() const   { return body_; }

        /// @brief Creating a dummy node to fill the tree and find more errors.
        static std::shared_ptr<MethodNode> makeDummy(
            khthon::location location, 
            std::string name
        );
    };


    class FormalNode : public Node {
    private:
        std::string name_;
        Type type_;
    public:
        FormalNode(
            khthon::location l,
            std::string n,
            Type t
        ) : 
            Node(l),
            name_(std::move(n)),
            type_(std::move(t))
        { }

        std::string accept(Visitor<std::string> const& visitor) const override { 
            return visitor.visit(*this); 
        }
        void accept(Visitor<void> const& visitor) const override { 
            visitor.visit(*this); 
        }
        void accept(MutableVisitor<void>& visitor) override {
            visitor.visit(*this);
        }
        llvm::Value* accept(MutableVisitor<llvm::Value*>& visitor) override {
            return visitor.visit(*this);
        }

        const std::string& name() const { return name_; }
        const Type& type() const        { return type_; }
    };


    class BlockExpr : public Expr {
    private:
        NodeList<Expr> expressions_;
    public:
        BlockExpr(khthon::location l, NodeList<Expr> es) : 
            Expr(l), expressions_(std::move(es)) { }

        std::string accept(Visitor<std::string> const& visitor) const override { 
            return visitor.visit(*this); 
        }
        void accept(Visitor<void> const& visitor) const override { 
            visitor.visit(*this); 
        }
        void accept(MutableVisitor<void>& visitor) override {
            visitor.visit(*this);
        }
        llvm::Value* accept(MutableVisitor<llvm::Value*>& visitor) override {
            return visitor.visit(*this);
        }

        /// @brief Whether this block contains expressions or not.
        bool is_empty() const { return expressions_.empty(); }
        
        /// @brief Return the expressions of this block.
        const NodeList<Expr>& expressions() const { return expressions_; }

        /// @brief Get the last expression of this block.
        const std::shared_ptr<Expr>& last_expression() const {return expressions_.back(); }
        
    };


    class StringLiteralExpr : public Expr {
    private:
        std::string value_;
    public:
        StringLiteralExpr(khthon::location l, std::string v) : 
            Expr(l), value_(std::move(v)) { }

        std::string accept(Visitor<std::string> const& visitor) const override { 
            return visitor.visit(*this); 
        }
        void accept(Visitor<void> const& visitor) const override { 
            visitor.visit(*this); 
        }
        void accept(MutableVisitor<void>& visitor) override {
            visitor.visit(*this);
        }
        llvm::Value* accept(MutableVisitor<llvm::Value*>& visitor) override {
            return visitor.visit(*this);
        }

        
        std::string value() const { return value_; }
    };


    class IntegerLiteralExpr : public Expr {
    private:
        int value_;
    public:
        IntegerLiteralExpr(khthon::location l, int val) : 
            Expr(l), value_(val) { }

        std::string accept(Visitor<std::string> const& visitor) const override { 
            return visitor.visit(*this); 
        }
        void accept(Visitor<void> const& visitor) const override { 
            visitor.visit(*this); 
        }
        void accept(MutableVisitor<void>& visitor) override {
            visitor.visit(*this);
        }
        llvm::Value* accept(MutableVisitor<llvm::Value*>& visitor) override {
            return visitor.visit(*this);
        }

        int value() const { return value_; }
    };


    class BoolLiteralExpr : public Expr {
    private:
        bool value_;
    public:
        BoolLiteralExpr(khthon::location l, bool val) : 
            Expr(l), value_(val) { }

        std::string accept(Visitor<std::string> const& visitor) const override { 
            return visitor.visit(*this); 
        }
        void accept(Visitor<void> const& visitor) const override { 
            visitor.visit(*this); 
        }
        void accept(MutableVisitor<void>& visitor) override {
            visitor.visit(*this);
        }
        llvm::Value* accept(MutableVisitor<llvm::Value*>& visitor) override {
            return visitor.visit(*this);
        }
        
        bool value() const { return value_; }
    };
    
    
    class UnitLiteralExpr : public Expr {
    public:
        explicit UnitLiteralExpr(khthon::location l) : Expr(l) {}

        std::string accept(Visitor<std::string> const& v) const override { 
            return v.visit(*this); 
        }
        void accept(Visitor<void> const& visitor) const override { 
            visitor.visit(*this); 
        }
        void accept(MutableVisitor<void>& visitor) override {
            visitor.visit(*this);
        }
        llvm::Value* accept(MutableVisitor<llvm::Value*>& visitor) override {
            return visitor.visit(*this);
        }
    };
    

    class IfExpr : public Expr {
    private:
        std::shared_ptr<Expr> guardian_;
        std::shared_ptr<Expr> consequent_;
        std::optional<std::shared_ptr<Expr>> alternative_;

    public:
        IfExpr(
            khthon::location l,
            std::shared_ptr<Expr> g,
            std::shared_ptr<Expr> c,
            std::optional<std::shared_ptr<Expr>> a = std::nullopt
        ) : 
            Expr(l),
            guardian_(std::move(g)),
            consequent_(std::move(c)),
            alternative_(std::move(a))
        {}

        std::string accept(Visitor<std::string> const& visitor) const override { 
            return visitor.visit(*this); 
        }
        void accept(Visitor<void> const& visitor) const override { 
            visitor.visit(*this); 
        }
        void accept(MutableVisitor<void>& visitor) override {
            visitor.visit(*this);
        }
        llvm::Value* accept(MutableVisitor<llvm::Value*>& visitor) override {
            return visitor.visit(*this);
        }

        const auto& guardian() const    { return guardian_; }
        const auto& consequent() const  { return consequent_; }
        const auto& alternative() const { return alternative_; }
    };


    class AssignExpr : public Expr {
    private:
        std::string name_;
        std::shared_ptr<Expr> value_;
    public:
        AssignExpr(
            khthon::location l,
            std::string n,
            std::shared_ptr<Expr> v
        ) : 
            Expr(l),
            name_(n),
            value_(v)
        {}

        std::string accept(Visitor<std::string> const& visitor) const override { 
            return visitor.visit(*this); 
        }
        void accept(Visitor<void> const& visitor) const override { 
            visitor.visit(*this); 
        }
        void accept(MutableVisitor<void>& visitor) override {
            visitor.visit(*this);
        }
        llvm::Value* accept(MutableVisitor<llvm::Value*>& visitor) override {
            return visitor.visit(*this);
        }

        const std::string& name() const { return name_; }
        const auto& value() const { return value_; }
    };


    class NewExpr : public Expr {
    private:
        std::string identifier_;
    public:
        NewExpr(khthon::location l, std::string i) :
            Expr(l), identifier_(i) { }

        std::string accept(Visitor<std::string> const& visitor) const override { 
            return visitor.visit(*this); 
        }
        void accept(Visitor<void> const& visitor) const override { 
            visitor.visit(*this); 
        }
        void accept(MutableVisitor<void>& visitor) override {
            visitor.visit(*this);
        }
        llvm::Value* accept(MutableVisitor<llvm::Value*>& visitor) override {
            return visitor.visit(*this);
        }

        const std::string& identifier() const { return identifier_; }
    };

    
    class UnOpExpr : public Expr {
    private:
        UnaryOperation operation_;
        std::shared_ptr<Expr> operand_;
    public:
        UnOpExpr(
            khthon::location l,
            UnaryOperation operation,
            std::shared_ptr<Expr> operand
        ) :
            Expr(l),
            operation_(operation),
            operand_(operand)
        {}

        std::string accept(Visitor<std::string> const& visitor) const override { 
            return visitor.visit(*this); 
        }
        void accept(Visitor<void> const& visitor) const override { 
            visitor.visit(*this); 
        }
        void accept(MutableVisitor<void>& visitor) override {
            visitor.visit(*this);
        }
        llvm::Value* accept(MutableVisitor<llvm::Value*>& visitor) override {
            return visitor.visit(*this);
        }
        
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
            khthon::location l,
            BinaryOperation operation,
            std::shared_ptr<Expr> left,
            std::shared_ptr<Expr> right
        ) :
            Expr(l),
            operation_(operation),
            left_(left),
            right_(right)
        {}

        std::string accept(Visitor<std::string> const& visitor) const override { 
            return visitor.visit(*this); 
        }
        void accept(Visitor<void> const& visitor) const override { 
            visitor.visit(*this); 
        }
        void accept(MutableVisitor<void>& visitor) override {
            visitor.visit(*this);
        }
        llvm::Value* accept(MutableVisitor<llvm::Value*>& visitor) override {
            return visitor.visit(*this);
        }
        
        const BinaryOperation& operation() const    { return operation_; }
        const auto& left() const                    { return left_; }
        const auto& right() const                   { return right_; }
    };


    class VariableExpr : public Expr {
    private:
        std::string identifier_;
    public:
        VariableExpr(khthon::location l, std::string i) :
            Expr(l), identifier_(i) { }

        std::string accept(Visitor<std::string> const& visitor) const override { 
            return visitor.visit(*this); 
        }
        void accept(Visitor<void> const& visitor) const override { 
            visitor.visit(*this); 
        }
        void accept(MutableVisitor<void>& visitor) override {
            visitor.visit(*this);
        }
        llvm::Value* accept(MutableVisitor<llvm::Value*>& visitor) override {
            return visitor.visit(*this);
        }
        
        const std::string& identifier() const { return identifier_; }
    };


    class CallExpr : public Expr {
    private:
        std::shared_ptr<Expr> receiver_;
        std::string method_name_;
        std::vector<std::shared_ptr<Expr>> arguments_;

    public:
        CallExpr(
            khthon::location l,
            std::shared_ptr<Expr> r,
            std::string m,
            std::vector<std::shared_ptr<Expr>> as
        ) : 
            Expr(l),
            receiver_(std::move(r)),
            method_name_(std::move(m)),
            arguments_(std::move(as))
        {}

        std::string accept(Visitor<std::string> const& visitor) const override { 
            return visitor.visit(*this); 
        }
        void accept(Visitor<void> const& visitor) const override { 
            visitor.visit(*this); 
        }
        void accept(MutableVisitor<void>& visitor) override {
            visitor.visit(*this);
        }
        llvm::Value* accept(MutableVisitor<llvm::Value*>& visitor) override {
            return visitor.visit(*this);
        }
        
        const auto& receiver() const    { return receiver_; }
        const std::string& name() const { return method_name_; }
        const auto& args() const        { return arguments_; }
    };


    class SelfExpr : public Expr {
    public:
        explicit SelfExpr(khthon::location l) : Expr(l) { }

        std::string accept(Visitor<std::string> const& v) const override { 
            return v.visit(*this); 
        }
        void accept(Visitor<void> const& visitor) const override { 
            visitor.visit(*this); 
        }
        void accept(MutableVisitor<void>& visitor) override {
            visitor.visit(*this);
        }
        llvm::Value* accept(MutableVisitor<llvm::Value*>& visitor) override {
            return visitor.visit(*this);
        }
    };


    class LetExpr : public Expr {
    private:
        std::string name_;
        Type type_;
        std::shared_ptr<Expr> scope_;
        std::optional<std::shared_ptr<Expr>> initializer_;

    public:
        LetExpr(
            khthon::location l,
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

        std::string accept(Visitor<std::string> const& visitor) const override { 
            return visitor.visit(*this); 
        }
        void accept(Visitor<void> const& visitor) const override { 
            visitor.visit(*this); 
        }
        void accept(MutableVisitor<void>& visitor) override {
            visitor.visit(*this);
        }
        llvm::Value* accept(MutableVisitor<llvm::Value*>& visitor) override {
            return visitor.visit(*this);
        }
        
        const std::string& name() const { return name_; }
        const Type& type() const        { return type_; }
        bool has_initializer() const    { return initializer_.has_value(); }
        const auto& initializer() const { return initializer_; }
        const auto& scope() const       { return scope_; }
    };


    class WhileExpr : public Expr {
    private:
        std::shared_ptr<Expr> condition_;
        std::shared_ptr<Expr> body_;

    public:
        WhileExpr(
            khthon::location l,
            std::shared_ptr<Expr> c,
            std::shared_ptr<Expr> b
        ) :
            Expr(l),
            condition_(c),
            body_(b)
        {}

        std::string accept(Visitor<std::string> const& visitor) const override { 
            return visitor.visit(*this); 
        }
        void accept(Visitor<void> const& visitor) const override { 
            visitor.visit(*this); 
        }
        void accept(MutableVisitor<void>& visitor) override {
            visitor.visit(*this);
        }
        llvm::Value* accept(MutableVisitor<llvm::Value*>& visitor) override {
            return visitor.visit(*this);
        }
        
        const auto& condition() const { return condition_; }
        const auto& body() const { return body_; }        
    };   
}

#endif
