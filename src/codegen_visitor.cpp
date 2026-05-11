#include "generation.hpp"


namespace khthon {

    llvm::Value* CodeGenVisitor::visit(ProgramNode& node) {
        (void) node;
        cout << "CodeGenVisitor has visited a ProgramNode" << endl;
        return nullptr;
    }
    
    llvm::Value* CodeGenVisitor::visit(IntegerLiteralExpr& node) {
        return llvm::ConstantInt::get(
            llvm::Type::getInt32Ty(orchestrator_.context()), node.value(), true
        );
    }
    

} // namespace khthon

