#include "generation.hpp"
#include "colors.hpp"

using namespace std;
using namespace colors;

namespace khthon {


    void CodeGenVisitor::bind(std::string name, llvm::Value* value) {
        named_values_[name] = value;
    }
        
    void CodeGenVisitor::unbind(std::string name) {
        named_values_.erase(name);
    }

    void CodeGenVisitor::print_named_values() const {
        cout << as_note("CodeGenVisitor's named values: \n");
        for (auto it = named_values_.begin(); it != named_values_.end(); it++) {
            cout << "  " << it->first 
                 << "  " << it->second << endl;
        }
    }

    
    

    

    

    llvm::Value* CodeGenVisitor::visit(ProgramNode& node) {
        (void) node;
        cout << as_error("CodeGenVisitor has visited a ProgramNode") << endl;
        return nullptr;
    }

    llvm::Value* CodeGenVisitor::visit(ClassNode& node) {
        (void) node;
        return nullptr;
    }

    llvm::Value* CodeGenVisitor::visit(FormalNode& node) {
        (void) node;
        return nullptr;
    }

    llvm::Value* CodeGenVisitor::visit(FieldNode& node) {
        (void) node;
        return nullptr;
    }
    
    llvm::Value* CodeGenVisitor::visit(MethodNode& node) {

        (void) node;

        cout << as_error("CodeGenVisitor has visited a MethodNode") << endl;
        return llvm::ConstantInt::get(
            llvm::Type::getInt32Ty(orchestrator_.context()), 42, true
        );
    }

    llvm::Value* CodeGenVisitor::visit(BlockExpr& node) {
        (void) node;

        cout << as_error("CodeGenVisitor has visited a BlockExpr") << endl;
        
        //orchestrator_.builder().CreateRet(ConstantInt::get(return_type, 0));

        return llvm::ConstantInt::get(
            llvm::Type::getInt32Ty(orchestrator_.context()), 42, true
        );
    }

    llvm::Value* CodeGenVisitor::visit(IntegerLiteralExpr& node) {
        cout << as_error("CodeGenVisitor has visited a IntegerLiterlaNode") << endl;
        return llvm::ConstantInt::get(
            llvm::Type::getInt32Ty(orchestrator_.context()), node.value(), true
        );
    }

    llvm::Value* CodeGenVisitor::visit(StringLiteralExpr& node) {
        (void) node;
        return nullptr;
    }

    llvm::Value* CodeGenVisitor::visit(BoolLiteralExpr& node) {
        (void) node;
        return nullptr;
    }

    llvm::Value* CodeGenVisitor::visit(UnitLiteralExpr& node) {
        (void) node;
        return nullptr;
    }

    llvm::Value* CodeGenVisitor::visit(IfExpr& node) {
        (void) node;
        return nullptr;
    }

    llvm::Value* CodeGenVisitor::visit(AssignExpr& node) {
        (void) node;
        return nullptr;
    }

    llvm::Value* CodeGenVisitor::visit(NewExpr& node) {
        (void) node;
        return nullptr;
    }

    llvm::Value* CodeGenVisitor::visit(UnOpExpr& node) {
        (void) node;
        return nullptr;
    }
    
    llvm::Value* CodeGenVisitor::visit(BinOpExpr& node) {
        (void) node;
        return nullptr;
    }
    
    llvm::Value* CodeGenVisitor::visit(VariableExpr& node) {
        (void) node;
        return nullptr;
    }
    
    llvm::Value* CodeGenVisitor::visit(CallExpr& node) {
        (void) node;
        return nullptr;
    }
    
    llvm::Value* CodeGenVisitor::visit(SelfExpr& node) {
        (void) node;
        return nullptr;
    }
    
    llvm::Value* CodeGenVisitor::visit(LetExpr& node) {
        (void) node;
        return nullptr;
    }
    
    llvm::Value* CodeGenVisitor::visit(WhileExpr& node) {
        (void) node;
        return nullptr;
    }
    
} // namespace khthon

