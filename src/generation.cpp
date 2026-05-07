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
            auto* method_signature = llvm::FunctionType::get(
                ret, 
                params, 
                false
            );
            
            // Inserting the declaration into the module.
            Function::Create(
                method_signature,
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
        new GlobalVariable(
            *module_,  // LLVM automatically registers the GlobalVariable into the module
            object_vtable,
            true,      // It is constant.
            GlobalValue::ExternalLinkage,
            nullptr,  // Linker provides the initializer.
            "Object___vtable"
        );

        declare(
            "malloc",
            llvm::Type::getInt8PtrTy(context_),
            {llvm::Type::getInt64Ty(context_)}
        );
    }

    void CodeGenOrchestrator::emit_entry_point() {
        auto* main_type = FunctionType::get(
            llvm::Type::getInt32Ty(context_),
            {},  // no argc/argv arguments for now
            false
        );

        auto* entry = Function::Create(
            main_type,
            GlobalValue::ExternalLinkage,
            "main",
            *module_
        );

        BasicBlock* bb = BasicBlock::Create(
            context_,
            "entry",
            entry
        );
        IRBuilder<> builder(bb);

        auto* new_function = module_->getFunction("Main___new");
        auto* main_object  = builder.CreateCall(
            new_function,
            {},
            "main_object"
        );

        auto* main_function = module_->getFunction("Main__main");
        auto* return_value = builder.CreateCall(
            main_function,
            {main_object},
            "ret"
        );

        builder.CreateRet(return_value);
    }

    void CodeGenOrchestrator::emit_class_init(const ClassNode& node) {
        const string class_name = node.name();
        StructType* class_type = class_types_.at(class_name);
        llvm::Type* class_ptr = class_type->getPointerTo();

        // Creating this class' init method.
        auto* init_signature = FunctionType::get(class_ptr, {class_ptr}, false);
        auto* init_method = Function::Create(
            init_signature,
            GlobalValue::ExternalLinkage,
            class_name + "___init",
            *module_
        );

        // Only parameter of init is self.
        init_method->arg_begin()->setName("self");

        functions_[class_name + "___init"] = init_method;

        // Creating the entry point.
        BasicBlock* bb = BasicBlock::Create(context_, "entry", init_method);
        IRBuilder<> builder(bb);

        llvm::Value* self = init_method->arg_begin();

        // Store the class vtable pointer as the first index in the 
        // class structure.
        auto* vtable_global = vtable_globals_.at(class_name);
        auto* vtable_ptr = builder.CreateStructGEP(
            class_type,     // The class structure
            self,           // Pointer
            0,              // First index of the class structure
            "vtable_ptr"    // Name
        );

        // Storing in the code.
        builder.CreateStore(vtable_global, vtable_ptr);

        // Return self.
        builder.CreateRet(self);
    }

    void CodeGenOrchestrator::emit_class_new(const ClassNode& node) {
        const string class_name = node.name();
        StructType* class_type = class_types_.at(class_name);
        llvm::Type* class_ptr = class_type->getPointerTo();

        // Signature: %ClassName* @ClassName___new()
        auto* fn_type = FunctionType::get(class_ptr, {}, false);
        auto* fn = Function::Create(
            fn_type,
            GlobalValue::ExternalLinkage,
            class_name + "___new",
            *module_
        );

        functions_[class_name + "___new"] = fn;

        // Create entry point.
        BasicBlock* bb = BasicBlock::Create(context_, "entry", fn);
        IRBuilder<> builder(bb);

        // Compute sizeof(ClassName) using the standard GEP trick.
        // GEP a null pointer by 1 element, then ptrtoint — gives the byte size.
        auto* null_ptr = ConstantPointerNull::get(cast<PointerType>(class_ptr));
        
        auto* size_ptr = builder.CreateConstGEP1_32(
            class_type, 
            null_ptr, 
            1, 
            "size_ptr"
        );

        auto* size = builder.CreatePtrToInt(
            size_ptr, 
            llvm::Type::getInt64Ty(context_), 
            "size"
        );

        // Call malloc.
        auto* malloc_fn = module_->getFunction("malloc");
        auto* raw_mem = builder.CreateCall(malloc_fn, {size}, "raw_mem");

        // Cast the raw memory returned by malloc to this class.
        auto* obj = builder.CreateBitCast(raw_mem, class_ptr, "obj");

        // Call the init method.
        auto* init_fn = module_->getFunction(class_name + "___init");
        builder.CreateCall(init_fn, {obj});

        // Return the initialized object.
        builder.CreateRet(obj);
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

    void CodeGenOrchestrator::emit_method(
        const ClassNode& class_node,
        const MethodNode& method_node
    ) {
        const string class_name  = class_node.name();
        const string method_name = method_node.name();
        const string mangled = class_name + "__" + method_name;

        StructType* class_type = class_types_[class_name];

        // Build parameter types.
        vector<llvm::Type*> param_types;

        param_types.push_back(class_type->getPointerTo());  // self
        
        for (const auto& formal : method_node.formals())
            param_types.push_back(llvm_type(formal->type()));  // formals
        
        llvm::Type* return_type = llvm_type(method_node.type());  // return type

        // Create the method in the module.
        auto* method_signature = FunctionType::get(return_type, param_types, false);
        auto* method = Function::Create(
            method_signature,
            GlobalValue::ExternalLinkage,
            mangled,
            *module_
        );

        // Name the parameters for readability in the IR.
        auto arg_it = method->arg_begin();
        arg_it->setName("self");
        ++arg_it;
        for (const auto& formal : method_node.formals()) {
            arg_it->setName(formal->name());
            ++arg_it;
        }

        // Create the entry basic block and a stub body.
        BasicBlock* bb = BasicBlock::Create(context_, "entry", method);
        IRBuilder<> builder(bb);

        //! Stub: return the default value for the return type.
        //! Later this will be replaced by visiting the method body AST.
        if (return_type->isVoidTy()) {
            builder.CreateRetVoid();
        } else if (return_type->isIntegerTy()) {
            // Covers both int32 (i32) and bool (i1).
            builder.CreateRet(ConstantInt::get(return_type, 3));  // value of 0
        } else {
            // Pointer types (string, custom classes): return null for now.
            builder.CreateRet(ConstantPointerNull::get(
                cast<PointerType>(return_type)
            ));
        }

        // Register the function for later use.
        functions_[mangled] = method;
    }

    void CodeGenOrchestrator::emit_methods(const ClassNode& node) {
        for (const auto& method : node.methods())
            emit_method(node, *method);
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
        const string class_name = node.name();
        StructType* vtable_type = vtable_types_[class_name];

        // We have to get the methods associated to this class and
        // create a vtable constant.
        vector<Constant*> methods;
        for (const auto& method : node.methods()) {
            const string mangled = class_name + "__" + method->name();
            methods.push_back(functions_.at(mangled));
        }

        Constant* vtable_const = methods.empty()
            ? ConstantAggregateZero::get(vtable_type)     // no methods
            : ConstantStruct::get(vtable_type, methods);  // real pointers

        auto* vtable_global = new GlobalVariable(
            *module_,
            vtable_type,
            true,                           // isConstant
            GlobalValue::InternalLinkage,
            vtable_const,                   // initializer
            class_name + "_vtable"
        );

        vtable_globals_[class_name] = vtable_global;
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
        
        // Methods must be emitted before vtable globals, so that
        // globals can reference real function pointers.
        for (const auto& c : root->classes())
            emit_methods(*c);
        
        for (const auto& c : root->classes())
            emit_vtable_global(*c);
        
        for (const auto& c : root->classes())
            emit_class_init(*c);
        
        for (const auto& c : root->classes()) 
            emit_class_new(*c);
        


        // todo call in the CodeGen visitor

        // Lastly we emit the entrypoint.
        emit_entry_point();
    }

    void CodeGenOrchestrator::print_ir(raw_ostream& out) const {
        module_->print(out, nullptr);
    }

} // namespace Khthon


