#ifndef GENERATION_HPP
#define GENERATION_HPP

// LLVM headers contain a lot of warnings when compiling.
// Omitting them to focus on Khthon's warnings.

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter" 
#pragma GCC diagnostic ignored "-Wunused-variable" 

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/Support/raw_ostream.h"
// In old versions of LLVM. Used to find LLVM target triple.
#include "llvm/Support/Host.h"  

#pragma GCC diagnostic pop

#include "driver.hpp"
#include "semantics.hpp"

namespace Khthon {

    class CodeGenOrchestrator {
    private:
        Driver& driver_;
        SemanticChecker& checker_;

        llvm::LLVMContext context_;
        std::unique_ptr<llvm::Module> module_;

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

        /// @brief Creates opaque struct types for every class.
        void create_class_type(const ClassNode& node);

        /// @brief Fill in the vtable body with methods signatures.
        void finalize_vtable(const ClassNode& node);

        /// Pass 5: emit a stub body for one method.
        void emit_method(const ClassNode& class_node, const MethodNode& method_node);

        /// Pass 5 (driver): emit all methods of a class.
        void emit_methods(const ClassNode& node);

        /// @brief Fill in the class struct body.
        void finalize_class(const ClassNode& node);

        /// @brief Emit the vtable global constant.
        void emit_vtable_global(const ClassNode& node);

        /// @brief Helper to convert from VSOP types to LLVM types.
        llvm::Type* llvm_type(const Khthon::Type& t);


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