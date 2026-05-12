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

    Value* CodeGenVisitor::visit(ProgramNode& node) {
        (void) node;
        cout << as_error("CodeGenVisitor has visited a ProgramNode") << endl;
        return nullptr;
    }

    Value* CodeGenVisitor::visit(ClassNode& node) {
        (void) node;
        return nullptr;
    }

    Value* CodeGenVisitor::visit(FormalNode& node) {
        (void) node;
        return nullptr;
    }

    Value* CodeGenVisitor::visit(FieldNode& node) {
        (void) node;
        return nullptr;
    }
    
    Value* CodeGenVisitor::visit(MethodNode& node) {
        (void) node;
        return nullptr;
    }

    Value* CodeGenVisitor::visit(BlockExpr& node) {
        if (node.is_empty())
            return Constant::getNullValue(llvm::Type::getVoidTy(context()));

        cout << as_error("CodeGenVisitor has visited a BlockExpr") << endl;
        
        //orchestrator_.builder().CreateRet(ConstantInt::get(return_type, 0));

        return ConstantInt::get(
            llvm::Type::getInt32Ty(orchestrator_.context()), 42, true
        );
    }

    Value* CodeGenVisitor::visit(IntegerLiteralExpr& node) {
        cout << as_error("CodeGenVisitor has visited a IntegerLiterlaNode") << endl;
        return ConstantInt::get(
            llvm::Type::getInt32Ty(orchestrator_.context()), node.value(), true
        );
    }

    Value* CodeGenVisitor::visit(StringLiteralExpr& node) {
        (void) node;
        return nullptr;
    }

    Value* CodeGenVisitor::visit(BoolLiteralExpr& node) {
        (void) node;
        return nullptr;
    }

    Value* CodeGenVisitor::visit(UnitLiteralExpr& node) {
        (void) node;
        return nullptr;
    }

    Value* CodeGenVisitor::visit(IfExpr& node) {
        (void) node;
        return nullptr;
    }

    Value* CodeGenVisitor::visit(AssignExpr& node) {
        (void) node;
        return nullptr;
    }

    Value* CodeGenVisitor::visit(NewExpr& node) {
        (void) node;
        return nullptr;
    }

    Value* CodeGenVisitor::visit(UnOpExpr& node) {
        (void) node;
        return nullptr;
    }
    
    Value* CodeGenVisitor::visit(BinOpExpr& node) {
        (void) node;
        return nullptr;
    }
    
    Value* CodeGenVisitor::visit(VariableExpr& node) {
        (void) node;
        return nullptr;
    }
    
    Value* CodeGenVisitor::visit(CallExpr& node) {
        (void) node;
        return nullptr;
    }
    
    Value* CodeGenVisitor::visit(SelfExpr& node) {
        (void) node;
        return nullptr;
    }
    
    Value* CodeGenVisitor::visit(LetExpr& node) {
        (void) node;
        return nullptr;
    }
    
    Value* CodeGenVisitor::visit(WhileExpr& node) {
        (void) node;
        return nullptr;
    }
    
} // namespace khthon

