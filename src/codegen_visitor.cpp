#include "generation.hpp"

namespace Khthon
{
    CodeGenOrchestrator::CodeGenOrchestrator(
        Driver& driver, 
        SemanticChecker& checker
    ) :
        driver_(driver),
        checker_(checker),
        context_(),
        module_(std::make_unique<llvm::Module>("vsop_module", context_))  // vsopc_module is just a label
    {}

    /// @brief Passes over the AST to fetch data for the generation pass.
    /// @param root is the AST root.
    void CodeGenOrchestrator::generate(
        const std::shared_ptr<ProgramNode>& root
    ) {
        // Pass 1 — create opaque types for every class
        for (const auto& c : root->classes())
            create_class_type(*c);
        
        // Pass 2 — set vtable bodies (empty for now)
        for (const auto& c : root->classes())
            finalize_vtable(*c);

        // Pass 3 — set class struct bodies
        for (const auto& c : root->classes())
            finalize_class(*c);
        
        // Pass 4 — emit vtable globals
        for (const auto& c : root->classes())
            emit_vtable_global(*c);
        
    }

    // ---------------------------------------------------------------
    // Pass 1: create opaque struct types
    // ---------------------------------------------------------------

    void CodeGenOrchestrator::create_class_type(const ClassNode& node) {
        const std::string& name = node.name();

        // Create named but empty (opaque) struct types.
        // setBody() is called later in passes 2 and 3.
        llvm::StructType* class_ty  = 
            llvm::StructType::create(context_, name);

        llvm::StructType* vtable_ty = 
            llvm::StructType::create(context_, name + "_vtable_type");

        class_types_[name]  = class_ty;
        vtable_types_[name] = vtable_ty;
    }

    // ---------------------------------------------------------------
    // Pass 2: fill in vtable body
    // ---------------------------------------------------------------

    void CodeGenOrchestrator::finalize_vtable(const ClassNode& node) {
        llvm::StructType* vtable_ty = vtable_types_[node.name()];

        // No methods yet: empty body.
        // Later: push one function pointer type per method here.
        vtable_ty->setBody({});
    }

    // ---------------------------------------------------------------
    // Pass 3: fill in class struct body
    // ---------------------------------------------------------------

    void CodeGenOrchestrator::finalize_class(const ClassNode& node) {
        const std::string& name = node.name();

        llvm::StructType* class_ty  = class_types_[name];
        llvm::StructType* vtable_ty = vtable_types_[name];

        // For now: only the vtable pointer.
        // Later: inherited fields and own fields follow here.
        std::vector<llvm::Type*> fields;
        fields.push_back(vtable_ty->getPointerTo());

        class_ty->setBody(fields);
    }

    // ---------------------------------------------------------------
    // Pass 4: emit the vtable global
    // ---------------------------------------------------------------

    void CodeGenOrchestrator::emit_vtable_global(const ClassNode& node) {
        const std::string& name = node.name();
        llvm::StructType* vtable_ty = vtable_types_[name];

        // zeroinitializer is the correct constant for an empty struct.
        llvm::Constant* init = 
            llvm::ConstantAggregateZero::get(vtable_ty);

        llvm::GlobalVariable* vtable_global = new llvm::GlobalVariable(
            *module_,
            vtable_ty,
            true,                                   // isConstant
            llvm::GlobalValue::InternalLinkage,
            init,
            name + "_vtable"
        );

        vtable_instances_[name] = vtable_global;
    }


} // namespace Khthon