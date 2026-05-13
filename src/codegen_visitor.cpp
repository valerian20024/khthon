#include "generation.hpp"
#include "colors.hpp"
#include "utils.hpp"

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

    inline StructType* CodeGenVisitor::class_struct(const string& class_name) {
        return orchestrator_.get_class_struct(class_name);
    }

    inline StructType* CodeGenVisitor::vtable_struct(const string& class_name) {
        return orchestrator_.get_vtable_struct(class_name);
    }

    inline unsigned CodeGenVisitor::vtable_index(
        const std::string& class_name, 
        const std::string& method_name
    ) {
        return orchestrator_.get_vtable_index(class_name, method_name);
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

        string raw = node.value();
        string decoded = utils::decode(raw);
        
        // Adds the null terminator automatically.
        return builder().CreateGlobalStringPtr(decoded, ".str");
    }

    Value* CodeGenVisitor::visit(BoolLiteralExpr& node) {
        trace("visited a BoolLiteralExpr");
        
        return ConstantInt::get(
            llvm::Type::getInt1Ty(context()), 
            node.value() ? 1 : 0, 
            false  // Unsigned
        );
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

        Value* left  = node.left()->accept(*this);
        Value* right = node.right()->accept(*this);

        const BinaryOperation& op = node.operation();


        switch (op.kind()) {
            case BinaryOperation::Kind::PLUS:
                return builder().CreateAdd(left, right, "add");
            case BinaryOperation::Kind::MINUS:
                return builder().CreateSub(left, right, "sub");
            case BinaryOperation::Kind::TIMES:
                return builder().CreateMul(left, right, "mul");
            case BinaryOperation::Kind::DIVIDE:
                return builder().CreateSDiv(left, right, "div");

            case BinaryOperation::Kind::POWER: {
                // Grab the current function we're inserting into.
                llvm::Function* current_fn = builder().GetInsertBlock()->getParent();

                // Create the four basic blocks.
                llvm::BasicBlock* bb_init = builder().GetInsertBlock();
                llvm::BasicBlock* bb_cond = llvm::BasicBlock::Create(context(), "pow_cond", current_fn);
                llvm::BasicBlock* bb_body = llvm::BasicBlock::Create(context(), "pow_body", current_fn);
                llvm::BasicBlock* bb_exit = llvm::BasicBlock::Create(context(), "pow_exit", current_fn);

                llvm::Type* i32 = llvm::Type::getInt32Ty(context());

                // Jump from current block into the loop condition.
                builder().CreateBr(bb_cond);

                // --- pow_cond ---
                // PHI nodes pick up values from the two incoming edges:
                // the first iteration (from bb_init) and back-edges (from bb_body).
                builder().SetInsertPoint(bb_cond);
                llvm::PHINode* result_phi = builder().CreatePHI(i32, 2, "result");
                llvm::PHINode* exp_phi    = builder().CreatePHI(i32, 2, "exp");

                // Seed values from entry: result starts at 1, exp starts at `right`.
                result_phi->addIncoming(llvm::ConstantInt::get(i32, 1), bb_init);
                exp_phi->addIncoming(right, bb_init);

                // Loop condition: exp > 0
                llvm::Value* cond = builder().CreateICmpSGT(
                    exp_phi, 
                    llvm::ConstantInt::get(i32, 0), 
                    "exp_gt_0"
                );
                builder().CreateCondBr(cond, bb_body, bb_exit);

                // --- pow_body ---
                builder().SetInsertPoint(bb_body);

                llvm::Value* new_result = builder().CreateMul(result_phi, left,  "new_result");
                llvm::Value* new_exp    = builder().CreateSub(exp_phi,    
                    llvm::ConstantInt::get(i32, 1), "new_exp"
                );

                // Back-edge: feed updated values back into the PHI nodes.
                result_phi->addIncoming(new_result, bb_body);
                exp_phi->addIncoming(new_exp,    bb_body);

                builder().CreateBr(bb_cond);

                // --- pow_exit ---
                builder().SetInsertPoint(bb_exit);
                return result_phi;  // The last value of result_phi is our answer.
            }
            
            case BinaryOperation::Kind::AND:
                return builder().CreateAnd(left, right, "and");

            case BinaryOperation::Kind::LOWER:
                return builder().CreateICmpSLT(left, right, "lt");
            case BinaryOperation::Kind::LOWER_EQUAL:
                return builder().CreateICmpSLE(left, right, "le");

            case BinaryOperation::Kind::EQUAL: {
                const Type& t = node.left()->type();

                if (t.is_int32() || t.is_bool())
                    return builder().CreateICmpEQ(left, right, "eq");

                // Strings and class instances: pointer equality
                if (t.is_string() || t.is_custom())
                    return builder().CreateICmpEQ(left, right, "ptr_eq");
            }

            default:
                driver_.internal_error("visit(BinOp): Unhandled binary operator.");
                return nullptr;
        }
    }
    
    Value* CodeGenVisitor::visit(VariableExpr& node) {
        trace("visited a VariableExpr");
        (void) node;
        return nullptr;
    }
    
    Value* CodeGenVisitor::visit(CallExpr& node) {
        trace("visited a CallExpr");

        // Evaluating receiver and arguments.
        Value* receiver = node.receiver()->accept(*this);
        vector<Value*> args;
        for (const auto& arg : node.args())
            args.push_back(arg->accept(*this));

        const string receiver_name = node.receiver()->type().custom_name();
        const string method_name = node.name();

        // Load the vtable pointer from slot 0 of the receiver struct
        auto* receiver_struct = class_struct(receiver_name);
        auto* receiver_vtable = vtable_struct(receiver_name);

        // Computes the address where the VTable is stored.
        Value* vtable_ptr_addr = builder().CreateStructGEP(
            receiver_struct, 
            receiver, 
            0, 
            "vtable_ptr_addr"
        );

        // Reading the address.
        LoadInst* vtable_ptr = builder().CreateLoad(
            receiver_vtable->getPointerTo(), 
            vtable_ptr_addr, 
            "vtable"
        );

        // Get the vtable slot index for this method.
        unsigned slot = vtable_index(receiver_name, method_name);

        // Load the function pointer from the vtable slot.
        Value* fn_ptr_addr = builder().CreateStructGEP(
            receiver_vtable, 
            vtable_ptr, 
            slot, 
            "fn_ptr_addr"
        );

        // e.g. %Object* (%Main*, i8*)*
        llvm::Type* fn_ptr_type = receiver_vtable->getElementType(slot);  
        LoadInst* fn_ptr = builder().CreateLoad(
            fn_ptr_type, 
            fn_ptr_addr, 
            "fn_ptr"
        );

        // The function pointer expects a specific self type (e.g. %Main*).
        // Extract it and bitcast receiver if needed.
        FunctionType* fn_type = cast<FunctionType>(
            cast<PointerType>(fn_ptr_type)->getElementType()
        );

        // Casting.
        Value* casted_self = builder().CreateBitCast(
            receiver, 
            fn_type->getParamType(0),
            "self_cast"
        );

        // Assemble the argument list: "self" first, then the rest.
        vector<Value*> call_args;
        call_args.push_back(casted_self);
        for (auto* arg : args)
            call_args.push_back(arg);

        return builder().CreateCall(fn_type, fn_ptr, call_args, "call");
    }
    
    Value* CodeGenVisitor::visit(SelfExpr& node) {
        trace("visited a SelfExpr");
        (void) node;
        return named_values_.at("self");
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
