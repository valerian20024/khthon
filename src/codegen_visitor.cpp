#include "generation.hpp"

using namespace llvm;
using namespace std;


namespace Khthon
{
    CodeGenOrchestrator::CodeGenOrchestrator(
        Driver& driver, 
        SemanticChecker& checker
    ) :
        driver_(driver),
        checker_(checker),
        context_(),
        module_(std::make_unique<Module>("vsop_module", context_))
    {}

    void CodeGenOrchestrator::generate(
        const shared_ptr<ProgramNode>& root
    ) {
        // Multiple passes are required to properly gather data.
        
        // Creating opaque structure for each class.
        for (const auto& c : root->classes())
            create_class_type(*c);
        
        // Fill in each class vtable bodies.
        for (const auto& c : root->classes())
            finalize_vtable(*c);
        
        // Fill in the rest of the class structure (i.e., fields).
        for (const auto& c : root->classes())
            finalize_class(*c);
        
        // Emit the vtable globals.
        for (const auto& c : root->classes())
            emit_vtable_global(*c);
        
        // Debug: force-print the struct layout.
        class_types_["Main"]->print(llvm::errs());
        llvm::errs() << "\n";

        // todo call in the CodeGen visitor
    }

    void CodeGenOrchestrator::create_class_type(const ClassNode& node) {
        const string name = node.name();

        // Creating named but empty opaque struct types.
        StructType* class_type = StructType::create(context_, name);
        StructType* vtable_type = StructType::create(
            context_, name + "_vtable_type"
        );

        class_types_[name]  = class_type;
        vtable_types_[name] = vtable_type;
    }

    void CodeGenOrchestrator::finalize_vtable(const ClassNode& node) {
        StructType* vtable_type = vtable_types_[node.name()];

        // No methods yet: empty body.
        // Later: push one function pointer type per method here.
        vtable_type->setBody(ArrayRef<llvm::Type*>());
    }

    void CodeGenOrchestrator::finalize_class(const ClassNode& node) {
        const string name = node.name();

        StructType* class_type  = class_types_[name];
        StructType* vtable_type = vtable_types_[name];

        // For now: only the vtable pointer.
        // Later: inherited fields and own fields follow here.
        vector<llvm::Type*> fields;
        fields.push_back(vtable_type->getPointerTo());

        class_type->setBody(fields);
    }

    void CodeGenOrchestrator::emit_vtable_global(const ClassNode& node) {
        const string name = node.name();
        StructType* vtable_type = vtable_types_[name];

        // zeroinitializer is the correct constant for an empty struct.
        Constant* init = ConstantAggregateZero::get(vtable_type);

        GlobalVariable* vtable_global = new GlobalVariable(
            *module_,
            vtable_type,
            true,                                   // isConstant
            GlobalValue::InternalLinkage,
            init,
            name + "_vtable"
        );

        vtable_instances_[name] = vtable_global;
    }

    void CodeGenOrchestrator::print_ir(raw_ostream& out) const {
        module_->print(out, nullptr);
    }

} // namespace Khthon