#ifndef VISITORS_HPP
#define VISITORS_HPP

#include <string>
#include <vector>
#include <memory>

using namespace std;

namespace khthon {

    class Node;
    class Expr;
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

    template <typename T> using NodeList = std::vector<std::shared_ptr<T>>;

    /**
     * @brief Abstract class for visitors.
     * @note While the visit methods are declared const member functions, the 
     * classes extending Visitor can still implement them to have side
     * effects. E.g., by modifying a reference to a variable owned by a
     * different class.
     */
    template <typename R> class Visitor {
    public:
        virtual R visit(const ProgramNode&) const           { return R{}; }
        virtual R visit(const ClassNode&) const             { return R{}; }
        virtual R visit(const FieldNode&) const             { return R{}; }
        virtual R visit(const MethodNode&) const            { return R{}; }
        virtual R visit(const FormalNode&) const            { return R{}; }
        virtual R visit(const BlockExpr&) const             { return R{}; }
        virtual R visit(const StringLiteralExpr&) const     { return R{}; }
        virtual R visit(const IntegerLiteralExpr&) const    { return R{}; }
        virtual R visit(const BoolLiteralExpr&) const       { return R{}; }
        virtual R visit(const UnitLiteralExpr&) const       { return R{}; }
        virtual R visit(const IfExpr&) const                { return R{}; }
        virtual R visit(const AssignExpr&) const            { return R{}; }
        virtual R visit(const NewExpr&) const               { return R{}; }
        virtual R visit(const UnOpExpr&) const              { return R{}; }
        virtual R visit(const BinOpExpr&) const             { return R{}; }
        virtual R visit(const VariableExpr&) const          { return R{}; }
        virtual R visit(const CallExpr&) const              { return R{}; }
        virtual R visit(const SelfExpr&) const              { return R{}; }
        virtual R visit(const LetExpr&) const               { return R{}; }
        virtual R visit(const WhileExpr&) const             { return R{}; }

        virtual ~Visitor() = default;
    };

    /**
     * @brief Base class for visitors that are allowed to mutate the AST
     *        (e.g. type annotation during semantic analysis).
     * 
     * This is deliberately separate from Visitor<T> so we keep the original
     * visitor purely const-correct.
     */
    template <typename R> class MutableVisitor {
    public:
        virtual R visit(ProgramNode&)           { return R{}; }
        virtual R visit(ClassNode&)             { return R{}; }
        virtual R visit(FieldNode&)             { return R{}; }
        virtual R visit(MethodNode&)            { return R{}; }
        virtual R visit(FormalNode&)            { return R{}; }
        virtual R visit(BlockExpr&)             { return R{}; }
        virtual R visit(StringLiteralExpr&)     { return R{}; }
        virtual R visit(IntegerLiteralExpr&)    { return R{}; }
        virtual R visit(BoolLiteralExpr&)       { return R{}; }
        virtual R visit(UnitLiteralExpr&)       { return R{}; }
        virtual R visit(IfExpr&)                { return R{}; }
        virtual R visit(AssignExpr&)            { return R{}; }
        virtual R visit(NewExpr&)               { return R{}; }
        virtual R visit(UnOpExpr&)              { return R{}; }
        virtual R visit(BinOpExpr&)             { return R{}; }
        virtual R visit(VariableExpr&)          { return R{}; }
        virtual R visit(CallExpr&)              { return R{}; }
        virtual R visit(SelfExpr&)              { return R{}; }
        virtual R visit(LetExpr&)               { return R{}; }
        virtual R visit(WhileExpr&)             { return R{}; }

        virtual ~MutableVisitor() = default;
    };

    /**
     * @brief Specializaton of MutableVisitor for the type void.
     * @note On older compilers (e.g., clang++ 11), return void{} is seen
     * as an error. This allows to fix it.
     */
    template <> class MutableVisitor<void> {
    public:
        virtual void visit(ProgramNode&)        { return; }
        virtual void visit(ClassNode&)          { return; }
        virtual void visit(FieldNode&)          { return; }
        virtual void visit(MethodNode&)         { return; }
        virtual void visit(FormalNode&)         { return; }
        virtual void visit(BlockExpr&)          { return; }
        virtual void visit(StringLiteralExpr&)  { return; }
        virtual void visit(IntegerLiteralExpr&) { return; }
        virtual void visit(BoolLiteralExpr&)    { return; }
        virtual void visit(UnitLiteralExpr&)    { return; }
        virtual void visit(IfExpr&)             { return; }
        virtual void visit(AssignExpr&)         { return; }
        virtual void visit(NewExpr&)            { return; }
        virtual void visit(UnOpExpr&)           { return; }
        virtual void visit(BinOpExpr&)          { return; }
        virtual void visit(VariableExpr&)       { return; }
        virtual void visit(CallExpr&)           { return; }
        virtual void visit(SelfExpr&)           { return; }
        virtual void visit(LetExpr&)            { return; }
        virtual void visit(WhileExpr&)          { return; }
    };

    /**
     * @brief Specializaton of Visitor for the type void.
     * @note On older compilers (e.g., clang++ 11), return void{} is seen
     * as an error. This allows to fix it.
     */
    template <> class Visitor<void> {
    public:
        virtual void visit(const ProgramNode&) const        { return; }
        virtual void visit(const ClassNode&) const          { return; }
        virtual void visit(const FieldNode&) const          { return; }
        virtual void visit(const MethodNode&) const         { return; }
        virtual void visit(const FormalNode&) const         { return; }
        virtual void visit(const BlockExpr&) const          { return; }
        virtual void visit(const StringLiteralExpr&) const  { return; }
        virtual void visit(const IntegerLiteralExpr&) const { return; }
        virtual void visit(const BoolLiteralExpr&) const    { return; }
        virtual void visit(const UnitLiteralExpr&) const    { return; }
        virtual void visit(const IfExpr&) const             { return; }
        virtual void visit(const AssignExpr&) const         { return; }
        virtual void visit(const NewExpr&) const            { return; }
        virtual void visit(const UnOpExpr&) const           { return; }
        virtual void visit(const BinOpExpr&) const          { return; }
        virtual void visit(const VariableExpr&) const       { return; }
        virtual void visit(const CallExpr&) const           { return; }
        virtual void visit(const SelfExpr&) const           { return; }
        virtual void visit(const LetExpr&) const            { return; }
        virtual void visit(const WhileExpr&) const          { return; }
    };

    /**
     * @brief Visitor responsible for printing an Abstract Syntax Tree.
     */
    class PrintVisitor : public Visitor<std::string> {
    private:
        bool annotate_;
        
        /// @brief Turns a node list into a string, ready for printing.
        template<typename T> std::string stringify(
            const NodeList<T>& items) const;
        
        /// @brief If needed, returns an annotated version of an expression node.
        std::string annotate(std::string annotation, const Expr& node) const;

    public:
        explicit PrintVisitor(bool annotate = false) : annotate_(annotate) {}

        std::string visit(const ProgramNode&) const override;
        std::string visit(const ClassNode&) const override;
        std::string visit(const FieldNode&) const override;
        std::string visit(const MethodNode&) const override;
        std::string visit(const FormalNode&) const override;
        std::string visit(const BlockExpr&) const override;
        std::string visit(const StringLiteralExpr&) const override;
        std::string visit(const IntegerLiteralExpr&) const override;
        std::string visit(const BoolLiteralExpr&) const override;
        std::string visit(const UnitLiteralExpr&) const override;
        std::string visit(const IfExpr&) const override;
        std::string visit(const AssignExpr&) const override;
        std::string visit(const NewExpr&) const override;
        std::string visit(const UnOpExpr&) const override;
        std::string visit(const BinOpExpr&) const override;
        std::string visit(const VariableExpr&) const override;
        std::string visit(const CallExpr&) const override;
        std::string visit(const SelfExpr&) const override;
        std::string visit(const LetExpr&) const override;
        std::string visit(const WhileExpr&) const override;
    };
}

#endif
