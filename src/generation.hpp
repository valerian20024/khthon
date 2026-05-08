#ifndef GENERATION_HPP
#define GENERATION_HPP

#include "llvm_compatibility.hpp"
#include "driver.hpp"
#include "semantics.hpp"

namespace Khthon {

    class CodeGenOrchestrator {
    private:
        Driver& driver_;
        SemanticChecker& checker_;

        llvm::LLVMContext context_;
        std::unique_ptr<llvm::Module> module_;
        llvm::IRBuilder<> builder_;

        /// @brief Mapping class names to their structures (fields and vtable pointer)
        std::map<std::string, llvm::StructType*> class_types_;

        /// @brief Mapping class names to their vtable.
        std::map<std::string, llvm::StructType*> vtable_types_;

        /// @brief Mapping class names to their vtable globals.
        std::map<std::string, llvm::GlobalVariable*> vtable_globals_;

        /// @brief Mapping class name -> (method name -> vtable slot index)
        std::map<std::string, std::map<std::string, unsigned>> vtable_indices_;

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
        void create_class_struct(const ClassNode& node);

        /// @brief Create an opaque structure for the class vtable.
        void create_class_vtable(const ClassNode& node);

        /// @brief Fill in the vtable body with methods signatures.
        void finalize_class_vtable(const ClassNode& node);

        /// @brief Emit one method of a class.
        void emit_method(const ClassNode& class_node, const MethodNode& method_node);

        /// @brief Emit all methods of a class.
        void emit_methods(const ClassNode& node);

        /// @brief Fill in the class struct body.
        void finalize_class_struct(const ClassNode& node);

        /// @brief Emit the vtable global constant.
        void emit_vtable(const ClassNode& node);

        /// @brief Helper to convert from VSOP types to LLVM types.
        llvm::Type* to_llvm(const Khthon::Type& t);


    public:
        CodeGenOrchestrator(Driver& driver, SemanticChecker& checker);

        /// @brief Passes over the AST to fetch data for the generation pass.
        /// @param root The AST root.
        void generate(const std::shared_ptr<ProgramNode>& root);

        /// Print the LLVM IR to the given stream.
        void print_ir(llvm::raw_ostream& out) const;
    };

} // namespace Khthon

#endif