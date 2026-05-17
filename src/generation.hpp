#ifndef GENERATION_HPP
#define GENERATION_HPP

/**
 * This file contains the interface of the CodeGenOrchestrator class
 * and the CodeGenVisitor. Both are responsible for code generation.
 */

#include "llvm_compatibility.hpp"
#include "config.hpp"
#include "driver.hpp"
#include "semantics.hpp"
#include "types.hpp"
#include "operators.hpp"

namespace khthon {

    /// @brief Acts as the central orchestrator for the code generation pass.
    /// 
    /// Stores all the necessary information (VTables, classes, slots in 
    /// VTables, ...) and prepares for the Visitor to emit LLVM for methods.
    class CodeGenOrchestrator {
    private:
        Driver& driver_;
        SemanticChecker& checker_;

        llvm::LLVMContext context_;
        std::unique_ptr<llvm::Module> module_;
        llvm::IRBuilder<> builder_;

        /// @brief Mapping class names to their structures (fields and vtable pointer)
        std::map<std::string, llvm::StructType*> class_structs_;

        /// @brief Mapping class names to their vtable.
        std::map<std::string, llvm::StructType*> vtable_structs_;

        /// @brief Mapping class names to their vtable globals.
        std::map<std::string, llvm::GlobalVariable*> vtable_globals_;

        /// @brief Mapping class name -> (method name -> vtable slot index)
        std::map<std::string, std::map<std::string, unsigned>> vtable_indices_;

        /// @brief Mapping class name -> (field name -> class struct slot index)
        std::map<std::string, std::map<std::string, unsigned>> field_indices_;

        // Maps mangled function name -> llvm::Function*
        // Needed so emit_vtable_global can reference already-emitted methods.
        std::map<std::string, llvm::Function*> functions_;

        /// @brief Emits declarations for Object.
        void emit_runtime_declarations();

        /// @brief Emits the C main entry point the OS expects.
        void emit_entry_point();

        /// @brief Emits ClassName___init for a single class.
        void emit_class_init(const ClassNode& node);

        /// @brief Emits ClassName___new for a single class.
        void emit_class_new(const ClassNode& node);

        /// @brief Create an opaque structure for the class structure.
        void declare_class(const ClassNode& node);

        /// @brief Create an opaque structure for the class vtable.
        void declare_vtable(const ClassNode& node);

        /// @brief Fill in the vtable body with methods signatures.
        void finalize_class_vtable(const ClassNode& node);

        void declare_class_init(const ClassNode& node);

        void declare_class_new(const ClassNode& node);

        /// @brief Emit one method of a class.
        void emit_method(const ClassNode& class_node, const MethodNode& method_node);

        /// @brief Emit all methods of a class.
        void emit_methods(const ClassNode& node);

        /// @brief Emits a thunk for an inherited method.
        void emit_thunk(
            const std::string& class_name, 
            const MethodInfo& method_info
        );

        /// @brief Fill in the class struct body.
        void finalize_class_struct(const ClassNode& node);

        /// @brief Emit the vtable global constant.
        void emit_vtable(const ClassNode& node);

        /// @brief Helper to convert from VSOP types to LLVM types.
        llvm::Type* to_llvm(const khthon::Type& t);

        /// @brief Wrapper that returns all the accessible fields of a class.
        std::vector<khthon::FieldInfo> collect_fields(
            const std::string class_name
        ) const;

        /// @brief Wrapper that returns all the accessible methods of a class.
        std::vector<khthon::MethodInfo> collect_methods(
            const std::string class_name
        ) const;

    public:
        CodeGenOrchestrator(Driver& driver, SemanticChecker& checker);

        /// @brief Passes over the AST to fetch data for the generation pass.
        /// @param root The AST root.
        void generate(const shared_ptr<ProgramNode>& root);

        /// @brief Print the LLVM IR to the given stream.
        void print_ir(llvm::raw_ostream& out) const;

        /// @brief Inserts a comment in the LLVM IR at the current insertion point.
        /// @note This is only intended for debugging purpose.
        ///       Requires to compile in debug mode.
        void comment(const std::string& text);

        /// @brief Handle to the context. 
        llvm::LLVMContext& context();

        /// @brief Handle to the module. 
        llvm::Module& module();

        /// @brief Handle to the builder. 
        llvm::IRBuilder<>& builder();

        llvm::StructType* get_class_struct(const std::string& class_name);
        
        llvm::StructType* get_vtable_struct(const std::string& class_name);
        
        unsigned get_vtable_index(
            const std::string& class_name, 
            const std::string& method_name
        );

    };

    /// @brief The code generation visitor generates LLVM code for expressions.
    class CodeGenVisitor : public MutableVisitor<llvm::Value*> {
    private:
        /// @brief Handle to driver.
        Driver& driver_;

        /// @brief Handle to code generation orchestrator.
        CodeGenOrchestrator& orchestrator_;

        /// @brief Contains bindings to LLVM values: self, method formals, 
        /// and local variables.
        std::map<std::string, llvm::Value*> named_values_ = {};

        /// @brief Emits a power operator by implementing a loop.
        llvm::Value* emit_power(llvm::Value* left, llvm::Value* right);

        /// @brief Wrapper to provide the LLVMContext handle.
        inline llvm::LLVMContext& context();
        
        /// @brief Wrapper to provide the Module handle.
        inline llvm::Module& module();

        /// @brief Wrapper to provide the IRBuilder handle.
        inline llvm::IRBuilder<>& builder();

        /// @brief Wrapper to provide the class struct of class_name. 
        inline llvm::StructType* class_struct(const std::string& class_name);

        /// @brief Wrapper to provide the vtable struct of class_name.
        inline llvm::StructType* vtable_struct(const std::string& class_name);
        
        /// @brief Wrapper to provide the vtable index of a method in a class.
        inline unsigned vtable_index(
            const std::string& class_name, 
            const std::string& method_name
        );

        /// @brief Logs a message from CodeGenVisitor.
        /// @note For debugging purpose. Only outputs when compiling in 
        /// debug mode.
        void trace(const string& message) const;

    public:
        CodeGenVisitor(Driver& d, CodeGenOrchestrator& cgo) : 
            driver_(d), orchestrator_(cgo) { 
                (void) driver_;  // not used for now.
            }

        /// @brief Binds a named llvm::Value* to its name.
        void bind(std::string name, llvm::Value* value);

        /// @brief Unbinds a named llvm::Value*.
        void unbind(std::string name);

        /// @brief Prints out the variables bindings of the visitor.
        /// @note For debugging purposes.
        void print_named_values() const;

        llvm::Value* visit(BlockExpr& node) override;
        llvm::Value* visit(IntegerLiteralExpr& node) override;
        llvm::Value* visit(StringLiteralExpr& node) override;
        llvm::Value* visit(BoolLiteralExpr& node) override;
        llvm::Value* visit(UnitLiteralExpr& node) override;
        llvm::Value* visit(IfExpr& node) override;
        llvm::Value* visit(AssignExpr& node) override;
        llvm::Value* visit(NewExpr& node) override;
        llvm::Value* visit(UnOpExpr& node) override;
        llvm::Value* visit(BinOpExpr& node) override;
        llvm::Value* visit(VariableExpr& node) override;
        llvm::Value* visit(CallExpr& node) override;
        llvm::Value* visit(SelfExpr& node) override;
        llvm::Value* visit(LetExpr& node) override;
        llvm::Value* visit(WhileExpr& node) override;       
    };

} // namespace khthon

#endif
