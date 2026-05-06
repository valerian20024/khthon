#include "generation.hpp"

using namespace llvm;
using namespace std;

namespace Khthon
{
    void CodeGenOrchestrator::emit_runtime_declarations() {
        
        // Creating Object and its vtable as opaque types.
        // This avoids a chicken and egg problem: we can use pointers to 
        // the vtable and object type before constructing them concretely.
        StructType* object_vtable = StructType::create(context_, "ObjectVTable");
        StructType* object_type = StructType::create(context_, "Object");

        // We take the handle to Object to be able to use it in methods 
        // declarations. Pointers have a fixed size so it's fine even though
        // Object is is not yet built.
        llvm::Type* object_ptr = object_type->getPointerTo();
        llvm::Type* i8_ptr     = llvm::Type::getInt8PtrTy(context_);
        llvm::Type* i1         = llvm::Type::getInt1Ty(context_);
        llvm::Type* i32        = llvm::Type::getInt32Ty(context_);

        // Populating Object's vtable.
        vector<llvm::Type*> vtable_methods = {
            FunctionType::get(object_ptr, {object_ptr, i8_ptr}, false)->getPointerTo(),  // print
            FunctionType::get(object_ptr, {object_ptr, i1},     false)->getPointerTo(),  // printBool
            FunctionType::get(object_ptr, {object_ptr, i32},    false)->getPointerTo(),  // printInt32
            FunctionType::get(i8_ptr,     {object_ptr},         false)->getPointerTo(),  // inputLine
            FunctionType::get(i1,         {object_ptr},         false)->getPointerTo(),  // intputBool
            FunctionType::get(i32,        {object_ptr},         false)->getPointerTo(),  // inputInt32
        };
        object_vtable->setBody(vtable_methods);

        // Object contains only a pointer to its vtable.
        object_type->setBody(object_vtable->getPointerTo());

        // Register Object type and its vtable.
        class_types_["Object"]  = object_type;
        vtable_types_["Object"] = object_vtable;

        // Declaring each method as external.
        auto declare = [&](
            const string&       name,
            llvm::Type*         ret,
            vector<llvm::Type*> params
        ) {
            // Creating the function signature.
            auto* method_type = llvm::FunctionType::get(ret, params, false);
            
            // Inserting the declaration into the module.
            Function::Create(
                method_type,
                GlobalValue::ExternalLinkage, 
                name,
                *module_
            );
        };

        declare("Object__print",      object_ptr, {object_ptr, i8_ptr});
        declare("Object__printBool",  object_ptr, {object_ptr, i1});
        declare("Object__printInt32", object_ptr, {object_ptr, i32});
        declare("Object__inputLine",  i8_ptr,     {object_ptr});
        declare("Object__inputBool",  i1,         {object_ptr});
        declare("Object__inputInt32", i32,        {object_ptr});
        declare("Object___new",       object_ptr, {});
        declare("Object___init",      object_ptr, {object_ptr});

        // Declaring the vtable global as external.
        //? why new here?
        new GlobalVariable(
            *module_,
            object_vtable,
            true,                           // It is constant.
            GlobalValue::ExternalLinkage,
            nullptr,                        // Linker provides the initializer.
            "Object___vtable"
        );
    }

    void CodeGenOrchestrator::create_class_type(const ClassNode& node) {
        const string name = node.name();

        // Creating class type and its vtable as opaque struct types.
        StructType* class_type = StructType::create(context_, name);
        StructType* vtable_type = StructType::create(
            context_, name + "_vtable_type"
        );

        // Inserting them into our data structs.
        class_types_[name]  = class_type;
        vtable_types_[name] = vtable_type;
    }

    void CodeGenOrchestrator::finalize_vtable(const ClassNode& node) {
        const string class_name = node.name();
        StructType* vtable_type = vtable_types_[node.name()];
        StructType* class_type = class_types_[class_name];

        // Stores the methods signatures.
        vector<llvm::Type*> slot_types;
        
        // Record the slot index of each method in the vtable.
        unsigned slot_index = 0;

        for (const auto& method : node.methods()) {
            const string method_name = method->name();

            // Stores the types of the method's parameters.
            vector<llvm::Type*> method_parameters;

            // The first parameter is always 'self', a pointer to the class.
            method_parameters.push_back(class_type->getPointerTo());

            for (const auto& formal : method->formals())
                method_parameters.push_back(llvm_type(formal->type()));

            // The return type.
            llvm::Type* return_type = llvm_type(method->type());

            // Create the LLVM construct for this method.
            llvm::Type* method_ptr_type = FunctionType::get(
                return_type, 
                method_parameters, 
                false               // is not vararg
            )->getPointerTo();

            slot_types.push_back(method_ptr_type);

            // Record the slot index for this method.
            vtable_indices_[class_name][method_name] = slot_index++;
        }

        vtable_type->setBody(slot_types);
    }

    void CodeGenOrchestrator::finalize_class(const ClassNode& node) {
        const string name = node.name();

        StructType* class_type  = class_types_[name];
        StructType* vtable_type = vtable_types_[name];

        //! For now: only the vtable pointer.
        //! Later: inherited fields and own fields follow here.
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
            true,                           // isConstant
            GlobalValue::InternalLinkage,
            init,                           // initializer
            name + "_vtable"
        );

        vtable_globals_[name] = vtable_global;
    }

    llvm::Type* CodeGenOrchestrator::llvm_type(const Khthon::Type& t) {
        if (t.is_int32())   return llvm::Type::getInt32Ty(context_);
        if (t.is_bool())    return llvm::Type::getInt1Ty(context_);
        if (t.is_unit())    return llvm::Type::getVoidTy(context_);
        if (t.is_string())  return llvm::Type::getInt8PtrTy(context_);
        if (t.is_custom())  return class_types_.at(t.custom_name())->getPointerTo();

        driver_.internal_error("llvm_type(): unknown type " + t.to_string());

        return llvm::Type::getVoidTy(context_);
    }

    CodeGenOrchestrator::CodeGenOrchestrator(
        Driver& driver, 
        SemanticChecker& checker
    ) :
        driver_(driver),
        checker_(checker),
        context_(),
        module_(std::make_unique<Module>("vsop_module", context_))
    {
        // Setting up manually target triple to not trigger an LLVM warning
        // telling it has overriden it to some value.
        module_->setTargetTriple(llvm::sys::getDefaultTargetTriple());
    }

    void CodeGenOrchestrator::generate(
        const shared_ptr<ProgramNode>& root
    ) {
        // First we emit the Object class content.
        emit_runtime_declarations();
        
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
        
        //! Debug: force-print the struct layout.
        class_types_["Main"]->print(llvm::errs());
        llvm::errs() << "\n";

        // todo call in the CodeGen visitor
    }

    void CodeGenOrchestrator::print_ir(raw_ostream& out) const {
        module_->print(out, nullptr);
    }

} // namespace Khthon


