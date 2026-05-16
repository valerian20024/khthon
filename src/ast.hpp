#ifndef AST_HPP
#define AST_HPP

/**
 * This file contains the interface for the Abstract Syntax Tree nodes.
 */

#include <memory>
#include <string>
#include <vector>
#include <optional>

#include "location.hh"
#include "visitors.hpp"
#include "types.hpp"
#include "operators.hpp"
#include "llvm_compatibility.hpp"

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
        /// @warning Mutates the node.
        void set_type(const khthon::Type& t) { type_ = t; }
    };


    /// @brief Node relating to a program.
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

        /// @return Handle to the classes.
        const NodeList<ClassNode>& classes() const { return classes_; }
    };

    /// @brief Node relating to a class.
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

        /// @return The class name.
        const std::string& name() const { return name_; }

        /// @return The parent class name.
        const std::string& parent() const { return parent_; }

        /// @return Handle to the fields.
        const NodeList<FieldNode>& fields() const { return fields_; }

        /// @return Handle to the methods.
        const NodeList<MethodNode>& methods() const { return methods_; }
    };

    /// @brief Node relating to a field.
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

        /// @return The field name.
        const std::string& name() const { return name_; }

        /// @return The field type.
        const Type& type() const { return type_; }

        /// @return Whether the field has an initializer.
        bool has_init() const { return initializer_.has_value(); }

        /// @return The initializer for this field.
        const auto& initializer() const { return initializer_; }
    };

    /// @brief Node relating to a method.
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

        /// @return The method name.
        const std::string& name() const { return name_; }

        /// @return The method return type.
        const Type& type() const { return type_; }

        /// @return Handle to the formals.
        const NodeList<FormalNode>& formals() const { return formals_; }

        /// @return Handle to the body of the method.
        const std::shared_ptr<Expr>& body() const { return body_; }

        /// @brief Creates a dummy node with no valuable information.
        static std::shared_ptr<MethodNode> makeDummy(
            khthon::location location, 
            std::string name
        );
    };

    /// @brief Node relating to a method's formal.
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

        /// @return The formal name.
        const std::string& name() const { return name_; }

        /// @return The formal class.
        const Type& type() const { return type_; }
    };

    /// @brief Node relating to a block.
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

        /// @return Whether this block contains expressions or not.
        bool is_empty() const { return expressions_.empty(); }
        
        /// @return Handle to the block's expressions.
        const NodeList<Expr>& expressions() const { return expressions_; }

        /// @return Handle to the block's last expression. 
        const std::shared_ptr<Expr>& last_expression() const {return expressions_.back(); }
    };

    /// @brief Node relating to a string literal.
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

        /// @return The literal value.
        std::string value() const { return value_; }
    };

    /// @brief Node relating to an integer literal.
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

        /// @return The literal value.
        int value() const { return value_; }
    };

    /// @brief Node relating to a boolean literal.
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
        
        /// @return The literal value.
        bool value() const { return value_; }
    };
    
    /// @brief Node relating to a unit literal.
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
    
    /// @brief Node relating to a conditional expression.
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

        /// @return Handle to the Expr in the condition.
        const auto& guardian() const { return guardian_; }

        /// @return Handle to the Expr in the "then" case.
        const auto& consequent() const { return consequent_; }

        /// @return Handle to the Expr in the "else" case.
        const auto& alternative() const { return alternative_; }
    };

    /// @brief Node relating to an assignment expression.
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

        /// @return The name of the assignment.
        const std::string& name() const { return name_; }

        /// @return The value of the assignment.
        const auto& value() const { return value_; }
    };

    /// @brief Node relating to an instanciation expression.
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

        /// @return The name of the identifier.
        const std::string& identifier() const { return identifier_; }
    };

    /// @brief Node relating to an unary operation.
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
        
        /// @return The unary operation.
        const UnaryOperation& operation() const { return operation_; }

        /// @return The only operand.
        const auto& operand() const { return operand_; }
    };

    /// @brief Node relating to a binary operation.
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
        
        /// @return The binary operation.
        const BinaryOperation& operation() const { return operation_; }

        /// @return The first operand.
        const auto& left() const { return left_; }

        /// @return The second operand.
        const auto& right() const { return right_; }
    };

    /// @brief Node relating to a variable.
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
        
        /// @return The identifier name.
        const std::string& identifier() const { return identifier_; }
    };

    /// @brief Node relating to a call expression.
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
        
        /// @return Handle to the callee.
        const auto& receiver() const { return receiver_; }

        /// @return The name of the method called.
        const std::string& name() const { return method_name_; }

        /// @return Handle to the arguments of the method.
        const auto& args() const { return arguments_; }
    };

    /// @brief Node relating to self.
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

    /// @brief Node relating to a local variable definition.
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
        
        /// @return Name of the variable.
        const std::string& name() const { return name_; }

        /// @return The type of the variable.
        const Type& type() const { return type_; }

        /// @return Whether it has an initializer.
        bool has_initializer() const { return initializer_.has_value(); }

        /// @return Handle to the initializer.
        const auto& initializer() const { return initializer_; }

        /// @return Handle to the expression in which the variable lives.
        const auto& scope() const { return scope_; }
    };

    /// @brief Node relating to a loop.
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
        
        /// @return Handle to the expression representing the condition.
        const auto& condition() const { return condition_; }

        /// @return Handle to the expression representing the body.
        const auto& body() const { return body_; }        
    };   
}

#endif
