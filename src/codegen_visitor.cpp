#include "generation.hpp"
#include "colors.hpp"

using namespace std;
using namespace colors;

namespace khthon {

    llvm::Value* CodeGenVisitor::visit(ProgramNode& node) {
        (void) node;
        //cout << as_error("CodeGenVisitor has visited a ProgramNode") << endl;
        return nullptr;
    }
    
    llvm::Value* CodeGenVisitor::visit(IntegerLiteralExpr& node) {
        cout << as_error("CodeGenVisitor has visited a IntegerLiterlaNode") << endl;
        return llvm::ConstantInt::get(
            llvm::Type::getInt32Ty(orchestrator_.context()), node.value(), true
        );
    }

    llvm::Value* CodeGenVisitor::visit(MethodNode& node) {
        cout << as_error("CodeGenVisitor has visited a MethodNode") << endl;
        return llvm::ConstantInt::get(
            llvm::Type::getInt32Ty(orchestrator_.context()), 42, true
        );
    }

    llvm::Value* CodeGenVisitor::visit(BlockExpr& node) {
        //cout << as_error("CodeGenVisitor has visited a BlockExpr") << endl;
        
        return llvm::ConstantInt::get(
            llvm::Type::getInt32Ty(orchestrator_.context()), 42, true
        );
    }
    

} // namespace khthon

