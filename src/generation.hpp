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

        std::map<std::string, llvm::GlobalVariable*> vtable_instances_;

        /// @brief Emits declarations for Object.
        void emit_runtime_declarations();

        /// Pass 1: create opaque struct types for every class.
        void create_class_type(const ClassNode& node);

        /// Pass 2: fill in vtable body (empty for now).
        void finalize_vtable(const ClassNode& node);

        /// Pass 3: fill in class struct body (vtable ptr only for now).
        void finalize_class(const ClassNode& node);

        /// Pass 4: emit the vtable global constant.
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