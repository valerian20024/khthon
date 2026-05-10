#ifndef AST_HPP
#define AST_HPP

#include <memory>
#include <string>
#include <vector>
#include <optional>

#include "location.hh"
#include "visitors.hpp"
#include "types.hpp"

/*
? Should I put back every scope ?
?   that is : Khthon::Type for example

? Is it better to keep make_shared or use unique ptr?

? Merging both UnaryOp and BinaryOp into Operator?

? add driver reference in classes to be able to log internal errors?
*/

namespace Khthon {

    // Forward declarations for visitors.

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
    class WhileExpr;

    template <typename R> class Visitor;
    template <typename R> class MutableVisitor;
    class PrintVisitor;

    using StringVisitor         = Visitor<std::string>;
    using VoidVisitor           = Visitor<void>;
    using MutableStringVisitor  = MutableVisitor<std::string>;
    using MutableVoidVisitor    = MutableVisitor<void>;

    template <typename T> using NodeList = std::vector<std::shared_ptr<T>>;

    /// @brief Datastructure to hold fields and methods together when 
    /// parsing a class.

    struct ClassMembers {
        NodeList<FieldNode> fields;
        NodeList<MethodNode> methods;

        ClassMembers() {}
    };

    /// @brief Represents an unary operation in VSOP.
    class UnaryOperation {
    private:
        enum class Kind {
            NOT,
            UMINUS,
            ISNULL,
            DEFAULT
        };
    
        Kind kind_ = Kind::DEFAULT;

    public:
        UnaryOperation() = default;
        
        explicit UnaryOperation(Kind k) : kind_(k) { }

        static UnaryOperation Not()         { return UnaryOperation(Kind::NOT); }
        static UnaryOperation UnaryMinus()  { return UnaryOperation(Kind::UMINUS); }
        static UnaryOperation IsNull()      { return UnaryOperation(Kind::ISNULL); }
        static UnaryOperation Default()     { return UnaryOperation(Kind::DEFAULT); }

        bool is_not()           const { return kind_ == Kind::NOT; }
        bool is_unary_minus()   const { return kind_ == Kind::UMINUS; }
        bool is_isnull()        const { return kind_ == Kind::ISNULL; }

        /// @brief Returns true for unary minus.
        bool is_arithmetic() const  { return kind_ == Kind::UMINUS; }

        /// @brief Returns true for the not and isnull operators.
        bool is_logical() const     { return (kind_ == Kind::NOT) || (kind_ == Kind::ISNULL); }

        /// @brief Checks whether this operation is defined in VSOP.
        bool is_undefined()     const { return kind_ == Kind::DEFAULT; }

        /// @brief Returns the type this unary operator operand can be.
        std::vector<Type> valid_operand_types() const;

        /// @brief Returns the type this unary operator should result in.
        Type result_type() const;

        /// @brief The `UnaryOperation` string representation, as seen in any valid VSOP code.
        std::string to_string() const;
    };


    /// @brief Represents a binary operation in VSOP.
    class BinaryOperation {
    private:
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
        };
    
        Kind kind_ = Kind::DEFAULT;

    public:
        using TypePair = std::pair<Type, Type>;

        BinaryOperation() = default;

        explicit BinaryOperation(Kind k) : kind_(k) { }

        // Factory methods.

        static BinaryOperation Equal()        { return BinaryOperation(Kind::EQUAL); }
        static BinaryOperation Lower()        { return BinaryOperation(Kind::LOWER); }
        static BinaryOperation LowerEqual()   { return BinaryOperation(Kind::LOWER_EQUAL); }
        static BinaryOperation Plus()         { return BinaryOperation(Kind::PLUS); }
        static BinaryOperation Minus()        { return BinaryOperation(Kind::MINUS); }
        static BinaryOperation Times()        { return BinaryOperation(Kind::TIMES); }
        static BinaryOperation Divide()       { return BinaryOperation(Kind::DIVIDE); }
        static BinaryOperation Power()        { return BinaryOperation(Kind::POWER); }
        static BinaryOperation And()          { return BinaryOperation(Kind::AND); }
        static BinaryOperation Default()      { return BinaryOperation(Kind::DEFAULT); }

        /// @brief Returns true for +, -, *, /, ^
        bool is_arithmetic() const;

        /// @brief Returns true only for 'and'
        bool is_logical() const;

        /// @brief Returns true for < and <=
        bool is_comparison() const;

        /// @brief Returns true only for =
        bool is_equality() const;

        /// @brief Checks whether this operation is defined in VSOP.
        bool is_undefined()     const { return kind_ == Kind::DEFAULT; }

        /// @brief Returns the types this binary operator operands can be.
        /// @note The vector of pairs allows for future asymmetric and overloaded operators.
        std::vector<TypePair> valid_operand_types() const;

        /// @brief Returns the type this binary operator should result in.
        Type result_type() const;

        /// @brief The `BinaryOperation` string representation, as seen in any valid VSOP code.
        std::string to_string() const;
    };


    /*================================================++
    ||                   AST NODES                    ||
    ++================================================*/


    /// @brief Abstract class for nodes. 
    class Node {
    private:
        Khthon::location loc_;
    public:
        Node(
            Khthon::location l
        ) : 
            loc_(l) 
        { }
        virtual ~Node() = default;

        virtual std::string accept(const StringVisitor&) const = 0;
        virtual void accept(MutableVoidVisitor&) = 0;
        virtual void accept(const VoidVisitor&) const = 0;

        Khthon::location location() const { return loc_; }
    };

    /// @brief Abstract class for expression nodes.
    class Expr : public Node {
    protected:
        Khthon::Type type_ = Type();

    public:
        Expr(
            Khthon::location l
        ) : 
            Node(l) 
        {}
        virtual ~Expr() = default;

        virtual std::string accept(const StringVisitor&) const = 0;
        virtual void accept(MutableVoidVisitor&) = 0;
        virtual void accept(const VoidVisitor&) const = 0;

        const Khthon::Type& type() const { return type_; }

        /// @brief Set type of the node.
        /// @warning Mutates the node even if declared as const.
        void set_type(const Khthon::Type& t) { type_ = t; }
    };


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

        std::string accept(StringVisitor const& visitor) const override { 
            return visitor.visit(*this); 
        }
        void accept(VoidVisitor const& visitor) const override { 
            visitor.visit(*this); 
        }
        void accept(MutableVoidVisitor& visitor) override {
            visitor.visit(*this);
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

        std::string accept(StringVisitor const& visitor) const override { 
            return visitor.visit(*this); 
        }
        void accept(VoidVisitor const& visitor) const override { 
            visitor.visit(*this); 
        }
        void accept(MutableVoidVisitor& visitor) override {
            visitor.visit(*this);
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
        
        std::string accept(StringVisitor const& visitor) const override { 
            return visitor.visit(*this); 
        }
        void accept(VoidVisitor const& visitor) const override { 
            visitor.visit(*this); 
        }
        void accept(MutableVoidVisitor& visitor) override {
            visitor.visit(*this);
        }

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
        
        std::string accept(StringVisitor const& visitor) const override { 
            return visitor.visit(*this); 
        }
        void accept(VoidVisitor const& visitor) const override { 
            visitor.visit(*this); 
        }
        void accept(MutableVoidVisitor& visitor) override {
            visitor.visit(*this);
        }

        const std::string& name() const             { return name_; }
        const Type& type() const                    { return type_; }
        const NodeList<FormalNode>& formals() const { return formals_; }
        const std::shared_ptr<Expr>& body() const   { return body_; }

        /// @brief Creating a dummy node to fill the tree and find more errors.
        static std::shared_ptr<MethodNode> makeDummy(
            Khthon::location location, 
            std::string name
        );
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

        std::string accept(StringVisitor const& visitor) const override { 
            return visitor.visit(*this); 
        }
        void accept(VoidVisitor const& visitor) const override { 
            visitor.visit(*this); 
        }
        void accept(MutableVoidVisitor& visitor) override {
            visitor.visit(*this);
        }

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

        std::string accept(StringVisitor const& visitor) const override { 
            return visitor.visit(*this); 
        }
        void accept(VoidVisitor const& visitor) const override { 
            visitor.visit(*this); 
        }
        void accept(MutableVoidVisitor& visitor) override {
            visitor.visit(*this);
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
        StringLiteralExpr(
            Khthon::location l, 
            std::string v
        ) : 
            Expr(l), 
            value_(std::move(v)) 
        {}

        std::string accept(StringVisitor const& visitor) const override { 
            return visitor.visit(*this); 
        }
        void accept(VoidVisitor const& visitor) const override { 
            visitor.visit(*this); 
        }
        void accept(MutableVoidVisitor& visitor) override {
            visitor.visit(*this);
        }
        
        std::string value() const { return value_; }
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

        std::string accept(StringVisitor const& visitor) const override { 
            return visitor.visit(*this); 
        }
        void accept(VoidVisitor const& visitor) const override { 
            visitor.visit(*this); 
        }
        void accept(MutableVoidVisitor& visitor) override {
            visitor.visit(*this);
        }

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

        std::string accept(StringVisitor const& visitor) const override { 
            return visitor.visit(*this); 
        }
        void accept(VoidVisitor const& visitor) const override { 
            visitor.visit(*this); 
        }
        void accept(MutableVoidVisitor& visitor) override {
            visitor.visit(*this);
        }
        
        bool value() const { return value_; }
    };
    
    
    class UnitLiteralExpr : public Expr {
    public:
        explicit UnitLiteralExpr(Khthon::location l) : Expr(l) {}

        std::string accept(StringVisitor const& v) const override { 
            return v.visit(*this); 
        }
        void accept(VoidVisitor const& visitor) const override { 
            visitor.visit(*this); 
        }
        void accept(MutableVoidVisitor& visitor) override {
            visitor.visit(*this);
        }
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

        std::string accept(StringVisitor const& visitor) const override { 
            return visitor.visit(*this); 
        }
        void accept(VoidVisitor const& visitor) const override { 
            visitor.visit(*this); 
        }
        void accept(MutableVoidVisitor& visitor) override {
            visitor.visit(*this);
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
            Khthon::location l,
            std::string n,
            std::shared_ptr<Expr> v
        ) : 
            Expr(l),
            name_(n),
            value_(v)
        {}

        std::string accept(StringVisitor const& visitor) const override { 
            return visitor.visit(*this); 
        }
        void accept(VoidVisitor const& visitor) const override { 
            visitor.visit(*this); 
        }
        void accept(MutableVoidVisitor& visitor) override {
            visitor.visit(*this);
        }

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

        std::string accept(StringVisitor const& visitor) const override { 
            return visitor.visit(*this); 
        }
        void accept(VoidVisitor const& visitor) const override { 
            visitor.visit(*this); 
        }
        void accept(MutableVoidVisitor& visitor) override {
            visitor.visit(*this);
        }

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

        std::string accept(StringVisitor const& visitor) const override { 
            return visitor.visit(*this); 
        }
        void accept(VoidVisitor const& visitor) const override { 
            visitor.visit(*this); 
        }
        void accept(MutableVoidVisitor& visitor) override {
            visitor.visit(*this);
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

        std::string accept(StringVisitor const& visitor) const override { 
            return visitor.visit(*this); 
        }
        void accept(VoidVisitor const& visitor) const override { 
            visitor.visit(*this); 
        }
        void accept(MutableVoidVisitor& visitor) override {
            visitor.visit(*this);
        }
        
        const BinaryOperation& operation() const    { return operation_; }
        const auto& left() const                    { return left_; }
        const auto& right() const                   { return right_; }
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

        std::string accept(StringVisitor const& visitor) const override { 
            return visitor.visit(*this); 
        }
        void accept(VoidVisitor const& visitor) const override { 
            visitor.visit(*this); 
        }
        void accept(MutableVoidVisitor& visitor) override {
            visitor.visit(*this);
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

        std::string accept(StringVisitor const& visitor) const override { 
            return visitor.visit(*this); 
        }
        void accept(VoidVisitor const& visitor) const override { 
            visitor.visit(*this); 
        }
        void accept(MutableVoidVisitor& visitor) override {
            visitor.visit(*this);
        }
        
        const auto& receiver() const    { return receiver_; }
        const std::string& name() const { return method_name_; }
        const auto& args() const        { return arguments_; }
    };


    class SelfExpr : public Expr {
    public:
        explicit SelfExpr(Khthon::location l) : Expr(l) {}

        std::string accept(StringVisitor const& v) const override { 
            return v.visit(*this); 
        }
        void accept(VoidVisitor const& visitor) const override { 
            visitor.visit(*this); 
        }
        void accept(MutableVoidVisitor& visitor) override {
            visitor.visit(*this);
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

        std::string accept(StringVisitor const& visitor) const override { 
            return visitor.visit(*this); 
        }
        void accept(VoidVisitor const& visitor) const override { 
            visitor.visit(*this); 
        }
        void accept(MutableVoidVisitor& visitor) override {
            visitor.visit(*this);
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
            Khthon::location l,
            std::shared_ptr<Expr> c,
            std::shared_ptr<Expr> b
        ) :
            Expr(l),
            condition_(c),
            body_(b)
        {}

        std::string accept(StringVisitor const& visitor) const override { 
            return visitor.visit(*this); 
        }
        void accept(VoidVisitor const& visitor) const override { 
            visitor.visit(*this); 
        }
        void accept(MutableVoidVisitor& visitor) override {
            visitor.visit(*this);
        }
        
        const auto& condition() const { return condition_; }
        const auto& body() const { return body_; }        
    };   
}

#endif
