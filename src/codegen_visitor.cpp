#include "generation.hpp"
#include "colors.hpp"

using namespace std;
using namespace colors;
using namespace llvm;

namespace khthon {

    void CodeGenVisitor::bind(string name, Value* value) {
        named_values_[name] = value;
    }
        
    void CodeGenVisitor::unbind(string name) {
        named_values_.erase(name);
    }

    void CodeGenVisitor::print_named_values() const {
        if (enable_advanced_logging) {    
            cout << as_note("CodeGenVisitor's named values: \n");

            for (auto it = named_values_.begin(); it != named_values_.end(); it++) {
                cout << "  " << it->first 
                    << "  " << it->second << endl;
            }
        }
    }

    inline LLVMContext& CodeGenVisitor::context() {
        return orchestrator_.context();
    }

    inline Module& CodeGenVisitor::module() {
        return orchestrator_.module();
    }

    inline IRBuilder<>& CodeGenVisitor::builder() {
        return orchestrator_.builder();
    }

    void CodeGenVisitor::trace(const string& message) const { 
        if (enable_advanced_logging)
            cout << "[CodeGenVisitor] " << message << endl;
    }

    Value* CodeGenVisitor::visit(ProgramNode& node) {
        trace("visited a ProgramNode");
        (void) node;
        return nullptr;
    }

    Value* CodeGenVisitor::visit(ClassNode& node) {
        trace("visited a ClassNode");
        (void) node;
        return nullptr;
    }

    Value* CodeGenVisitor::visit(FormalNode& node) {
        trace("visited a FormalNode");
        (void) node;
        return nullptr;
    }

    Value* CodeGenVisitor::visit(FieldNode& node) {
        trace("visited a FieldNode");
        (void) node;
        return nullptr;
    }
    
    Value* CodeGenVisitor::visit(MethodNode& node) {
        trace("visited a MethodNode");
        (void) node;
        return nullptr;
    }

    Value* CodeGenVisitor::visit(BlockExpr& node) {
        trace("visited a BlockExpr");

        if (node.is_empty())
            return Constant::getNullValue(llvm::Type::getVoidTy(context()));
        
        Value* last = nullptr;
        for (auto& expr : node.expressions())
            last = expr->accept(*this);
        return last;
    }

    Value* CodeGenVisitor::visit(IntegerLiteralExpr& node) {
        trace("visited a IntegerLiteralExpr");

        return ConstantInt::get(
            llvm::Type::getInt32Ty(context()), 
            node.value(), 
            true  // Signed integer.
        );
    }

    Value* CodeGenVisitor::visit(StringLiteralExpr& node) {
        trace("visited a StringLiteralExpr");
        (void) node;
        return nullptr;
    }

    Value* CodeGenVisitor::visit(BoolLiteralExpr& node) {
        trace("visited a BoolLiteralExpr");
        (void) node;
        return nullptr;
    }

    Value* CodeGenVisitor::visit(UnitLiteralExpr& node) {
        trace("visited a UnitLiteralExpr");
        (void) node;
        return nullptr;
    }

    Value* CodeGenVisitor::visit(IfExpr& node) {
        trace("visited a IfExpr");
        (void) node;
        return nullptr;
    }

    Value* CodeGenVisitor::visit(AssignExpr& node) {
        trace("visited a AssignExpr");
        (void) node;
        return nullptr;
    }

    Value* CodeGenVisitor::visit(NewExpr& node) {
        trace("visited a NewExpr");
        (void) node;
        return nullptr;
    }

    Value* CodeGenVisitor::visit(UnOpExpr& node) {
        trace("visited a UnOpExpr");
        (void) node;
        return nullptr;
    }
    
    Value* CodeGenVisitor::visit(BinOpExpr& node) {
        trace("visited a BinOpExpr");
        (void) node;
        return nullptr;
    }
    
    Value* CodeGenVisitor::visit(VariableExpr& node) {
        trace("visited a VariableExpr");
        (void) node;
        return nullptr;
    }
    
    Value* CodeGenVisitor::visit(CallExpr& node) {
        trace("visited a CallExpr");
        (void) node;
        return nullptr;
    }
    
    Value* CodeGenVisitor::visit(SelfExpr& node) {
        trace("visited a SelfExpr");

        //? return self in named_values_
        //? something like return named_values_.at("self");

        (void) node;
        return nullptr;
    }
    
    Value* CodeGenVisitor::visit(LetExpr& node) {
        trace("visited a LetExpr");
        (void) node;
        return nullptr;
    }
    
    Value* CodeGenVisitor::visit(WhileExpr& node) {
        trace("visited a WhileExpr");
        (void) node;
        return nullptr;
    }
    
} // namespace khthon

