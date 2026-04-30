#ifndef GENERATION_HPP
#define GENERATION_HPP

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Type.h"

#include "driver.hpp"
#include "semantics.hpp"

namespace Khthon {

    class CodeGenOrchestrator {
    private:
        Driver& driver_;
        SemanticChecker& checker_;

        llvm::LLVMContext context_;
        std::unique_ptr<llvm::Module> module_;

        // Maps class name -> its LLVM struct type
        std::map<std::string, llvm::StructType*> class_types_;

        // Maps class name -> its vtable struct type
        std::map<std::string, llvm::StructType*> vtable_types_;

        // Maps class name -> the global vtable instance
        std::map<std::string, llvm::GlobalVariable*> vtable_instances_;

        // --- private helpers ---

        /// Pass 1: create opaque struct types for every class.
        void create_class_type(const ClassNode& node);

        /// Pass 2: fill in vtable body (empty for now).
        void finalize_vtable(const ClassNode& node);

        /// Pass 3: fill in class struct body (vtable ptr only for now).
        void finalize_class(const ClassNode& node);

        /// Pass 4: emit the vtable global constant.
        void emit_vtable_global(const ClassNode& node);

    public:
        CodeGenOrchestrator(Driver& driver, SemanticChecker& checker);

        /// Top-level entry point.
        void generate(const std::shared_ptr<ProgramNode>& root);

        /// Print the LLVM IR to the given stream.
        void print_ir(llvm::raw_ostream& out) const;
    };

} // namespace Khthon

#endif