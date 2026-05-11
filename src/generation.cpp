#include "generation.hpp"
#include "mangling.hpp"

using namespace llvm;
using namespace std;

namespace khthon {

    void CodeGenOrchestrator::emit_runtime_declarations() {
        
        // Creating Object and its vtable as opaque types.
        // This avoids a chicken and egg problem: we can use pointers to 
        // the vtable and object type before constructing them concretely.
        StructType* object_vtable = StructType::create(
            context_, mangle::vt_struct("Object")
        );
        StructType* object = StructType::create(context_, "Object");

        // We take the handle to Object to be able to use it in methods 
        // declarations. Pointers have a fixed size so it's fine even though
        // Object is is not yet built.
        llvm::Type* object_ptr = object->getPointerTo();
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
        object->setBody(object_vtable->getPointerTo());

        // Register Object type and its vtable.
        class_structs_["Object"]  = object;
        vtable_structs_["Object"] = object_vtable;

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

        declare(mangle::meth("Object", "print"),
            object_ptr, {object_ptr, i8_ptr});
        declare(mangle::meth("Object", "printBool"),  
            object_ptr, {object_ptr, i1});
        declare(mangle::meth("Object", "printInt32"), 
            object_ptr, {object_ptr, i32});
        declare(mangle::meth("Object", "inputLine"),
            i8_ptr,     {object_ptr});
        declare(mangle::meth("Object", "inputBool"),
            i1,         {object_ptr});
        declare(mangle::meth("Object", "inputInt32"), 
            i32,        {object_ptr});
        declare(mangle::ctor("Object"),
            object_ptr, {});
        declare(mangle::init("Object"),
            object_ptr, {object_ptr});

        // Declaring the vtable global as external.
        new GlobalVariable(
            *module_,
            object_vtable,
            true,      // It is constant.
            GlobalValue::ExternalLinkage,
            nullptr,  // Linker provides the initializer.
            mangle::vt_global("Object")
        );

        declare("malloc", 
            llvm::Type::getInt8PtrTy(context_), 
            {llvm::Type::getInt64Ty(context_)}
        );
    }

    void CodeGenOrchestrator::emit_entry_point() {

        // We need to create an additional main function
        // that can be called by the runtime, whose job
        // is simply to instanciate Main, call Main::main
        // and return the value it returned.

        auto* entry_signature = FunctionType::get(
            llvm::Type::getInt32Ty(context_),
            {},  // no argc/argv arguments for now
            false
        );

        auto* entry_function = Function::Create(
            entry_signature,
            GlobalValue::ExternalLinkage,
            "main",  // the real main entrypoint of the program
            *module_
        );

        BasicBlock* bb = BasicBlock::Create(context_, "entry", entry_function);
        builder_.SetInsertPoint(bb);

        // Instanciate Main class.
        auto* new_function = module_->getFunction(mangle::ctor("Main"));
        auto* main_object  = builder_.CreateCall(
            new_function,
            {},
            "main_object"
        );

        // Call main method of Main.
        auto* main_method = module_->getFunction(mangle::meth("Main", "main"));
        auto* return_value = builder_.CreateCall(
            main_method,
            {main_object},
            "ret"
        );

        // Return the value returned by Main::main.
        builder_.CreateRet(return_value);
    }

    void CodeGenOrchestrator::emit_class_init(const ClassNode& node) {
        const string class_name = node.name();
        StructType* class_struct = class_structs_.at(class_name);
        llvm::Type* class_ptr = class_struct->getPointerTo();

        // Creating this class' init method.
        auto* init_signature = FunctionType::get(class_ptr, {class_ptr}, false);
        auto* init_method = Function::Create(
            init_signature,
            GlobalValue::ExternalLinkage,
            mangle::init(class_name),
            *module_
        );

        // Only parameter of init is self.
        init_method->arg_begin()->setName("self");

        functions_[mangle::init(class_name)] = init_method;

        // Creating the entry point.
        BasicBlock* bb = BasicBlock::Create(context_, "entry", init_method);
        builder_.SetInsertPoint(bb);

        llvm::Value* self = init_method->arg_begin();

        // Store the class vtable pointer as the first index in the 
        // class structure.
        auto* vtable_global = vtable_globals_.at(class_name);
        auto* vtable_ptr = builder_.CreateStructGEP(
            class_struct,     // The class structure
            self,           // Pointer
            0,              // First index of the class structure
            "vtable_ptr"    // Name
        );

        // Storing in the code.
        builder_.CreateStore(vtable_global, vtable_ptr);

        // Return self.
        builder_.CreateRet(self);
    }

    void CodeGenOrchestrator::emit_class_new(const ClassNode& node) {
        const string class_name = node.name();
        const string class_constructor = mangle::ctor(class_name);

        StructType* class_struct = class_structs_.at(class_name);
        llvm::Type* class_ptr = class_struct->getPointerTo();

        // Creating the class constructor.
        auto* signature = FunctionType::get(class_ptr, {}, false);
        auto* constructor = Function::Create(
            signature,
            GlobalValue::ExternalLinkage,
            class_constructor,
            *module_
        );

        functions_[class_constructor] = constructor;

        // Create entry point.
        BasicBlock* bb = BasicBlock::Create(context_, "entry", constructor);
        builder_.SetInsertPoint(bb);

        // Compute sizeof(ClassName) using the standard GEP trick.
        // GEP a null pointer by 1 element, then ptrtoint — gives the byte size.
        auto* null_ptr = ConstantPointerNull::get(cast<PointerType>(class_ptr));
        
        auto* size_ptr = builder_.CreateConstGEP1_32(
            class_struct, 
            null_ptr, 
            1, 
            "size_ptr"
        );

        auto* size = builder_.CreatePtrToInt(
            size_ptr, 
            llvm::Type::getInt64Ty(context_), 
            "size"
        );

        // Call malloc.
        auto* malloc_fn = module_->getFunction("malloc");
        auto* raw_mem = builder_.CreateCall(malloc_fn, {size}, "raw_mem");

        // Cast the raw memory returned by malloc to this class.
        auto* obj = builder_.CreateBitCast(raw_mem, class_ptr, "obj");

        // Call the init method.
        auto* init_fn = module_->getFunction(mangle::init(class_name));
        builder_.CreateCall(init_fn, {obj});

        // Return the initialized object.
        builder_.CreateRet(obj);
    }

    void CodeGenOrchestrator::create_class_struct(const ClassNode& node) {
        const string class_name = node.name();

        StructType* class_struct = StructType::create(context_, class_name);

        class_structs_[class_name] = class_struct;
    }

    void CodeGenOrchestrator::create_class_vtable(const ClassNode& node) {
        const string class_name = node.name();

        StructType* vtable_struct = StructType::create(
            context_, mangle::vt_struct(class_name)
        );

        vtable_structs_[class_name] = vtable_struct;
    }

    void CodeGenOrchestrator::finalize_class_vtable(const ClassNode& node) {
        const string class_name = node.name();
        StructType* vtable_struct = vtable_structs_[class_name];
        StructType* class_struct  = class_structs_[class_name];

        // Store methods signatures recording their slot index in the vtable.
        vector<llvm::Type*> slot_types;
        unsigned slot_index = 0;

        for (const auto& method : collect_methods(class_name)) {
            const string method_name = method.name();

            // Method parameters.
            vector<llvm::Type*> params;
            params.push_back(class_struct->getPointerTo());  // self

            for (const auto& formal : method.formals())
                params.push_back(to_llvm(formal.type()));

            // Method return type.
            llvm::Type* ret = to_llvm(method.return_type());

            slot_types.push_back(
                FunctionType::get(ret, params, false)->getPointerTo()
            );

            // Record the slot index for this method.
            vtable_indices_[class_name][method_name] = slot_index++;
        }

        vtable_struct->setBody(slot_types);
    }

    void CodeGenOrchestrator::emit_method(
        const ClassNode& class_node,
        const MethodNode& method_node
    ) {
        const string class_name  = class_node.name();
        const string method_name = method_node.name();
        const string mangled = mangle::meth(class_name, method_name);

        StructType* class_struct = class_structs_[class_name];

        // Build parameter types.
        vector<llvm::Type*> param_types;

        param_types.push_back(class_struct->getPointerTo());  // self
        
        for (const auto& formal : method_node.formals())
            param_types.push_back(to_llvm(formal->type()));  // formals
        
        llvm::Type* return_type = to_llvm(method_node.type());  // return type

        // Create the method in the module.
        auto* method_signature = FunctionType::get(
            return_type, 
            param_types, 
            false
        );

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
        builder_.SetInsertPoint(bb);

        //! Stub: return the default value for the return type.
        //! Later this will be replaced by visiting the method body AST.
        if (return_type->isVoidTy()) {
            builder_.CreateRetVoid();
        } else if (return_type->isIntegerTy()) {
            // Covers both int32 (i32) and bool (i1).
            builder_.CreateRet(ConstantInt::get(return_type, 0));  // value of 0
        } else {
            // Pointer types (string, custom classes): return null for now.
            builder_.CreateRet(ConstantPointerNull::get(
                cast<PointerType>(return_type)
            ));
        }
        //! end of stub

        // Register the function for later use.
        functions_[mangled] = method;
    }

    void CodeGenOrchestrator::emit_methods(const ClassNode& node) {
        for (const auto& method : node.methods())
            emit_method(node, *method);
    }

    void CodeGenOrchestrator::finalize_class_struct(const ClassNode& node) {
        const string class_name = node.name();
        StructType* class_struct  = class_structs_[class_name];
        StructType* vtable_struct = vtable_structs_[class_name];

        // The fields we will fill into the class struct.
        vector<llvm::Type*> fields;

        // Slot 0 is the vtable pointer.
        fields.push_back(vtable_struct->getPointerTo());
        unsigned slot = 1;

        // Walk up the class hierarchy to collect fields.
        for (const auto& info : collect_fields(class_name)) {
            field_indices_[class_name][info.name()] = slot++;
            fields.push_back(to_llvm(info.type()));
        }

        class_struct->setBody(fields);
    }

    void CodeGenOrchestrator::emit_vtable(const ClassNode& node) {
        const string class_name = node.name();
        StructType* vtable_struct = vtable_structs_[class_name];

        // We have to get the methods associated to this class and
        // create a vtable constant.
        vector<Constant*> methods;
        for (const auto& method : node.methods()) {
            methods.push_back(
                functions_.at(mangle::meth(class_name, method->name()))
            );
        }

        Constant* vtable_const = methods.empty()
            ? ConstantAggregateZero::get(vtable_struct)     // No methods.
            : ConstantStruct::get(vtable_struct, methods);  // Real pointers.

        auto* vtable_global = new GlobalVariable(
            *module_,
            vtable_struct,
            true,                   // It is constant.
            GlobalValue::InternalLinkage,
            vtable_const,           // Provide a constant initializer.
            mangle::vt_global(class_name)
        );

        vtable_globals_[class_name] = vtable_global;
    }

    llvm::Type* CodeGenOrchestrator::to_llvm(const khthon::Type& t) {
        if (t.is_int32())   return llvm::Type::getInt32Ty(context_);
        if (t.is_bool())    return llvm::Type::getInt1Ty(context_);
        if (t.is_unit())    return llvm::Type::getVoidTy(context_);
        if (t.is_string())  return llvm::Type::getInt8PtrTy(context_);
        if (t.is_custom())  return class_structs_.at(t.custom_name())->getPointerTo();

        driver_.internal_error("to_llvm(): unknown type " + t.to_string());

        return llvm::Type::getVoidTy(context_);
    }

    vector<FieldInfo> CodeGenOrchestrator::collect_fields(
        const string class_name
    ) const {
        return checker_.class_manager().collect_fields(class_name);
    }

    vector<MethodInfo> CodeGenOrchestrator::collect_methods(
        const string class_name
    ) const {
        return checker_.class_manager().collect_methods(class_name);
    }


    CodeGenOrchestrator::CodeGenOrchestrator(
        Driver& driver, 
        SemanticChecker& checker
    ) :
        driver_(driver),
        checker_(checker),
        context_(),
        module_(std::make_unique<Module>("", context_)),
        builder_(context_)
    {
        module_->setModuleIdentifier("VSOP Module");
        module_->setSourceFileName(driver_.source_file());
        module_->setTargetTriple(llvm::sys::getDefaultTargetTriple());
    }

    void CodeGenOrchestrator::generate(const shared_ptr<ProgramNode>& root) {
        emit_runtime_declarations();
        
        for (const auto& c : root->classes())
            create_class_vtable(*c);

        for (const auto& c : root->classes())
            create_class_struct(*c);
        
        for (const auto& c : root->classes())
            finalize_class_vtable(*c);
        
        for (const auto& c : root->classes())
            finalize_class_struct(*c);
        
        // Methods must be emitted before vtable globals, so that
        // globals can reference real function pointers.
        for (const auto& c : root->classes())
            emit_methods(*c);
        
        for (const auto& c : root->classes())
            emit_vtable(*c);
        
        for (const auto& c : root->classes())
            emit_class_init(*c);
        
        for (const auto& c : root->classes()) 
            emit_class_new(*c);

        // Lastly we emit the entrypoint.
        emit_entry_point();
    }

    void CodeGenOrchestrator::print_ir(raw_ostream& out) const {
        module_->print(out, nullptr);
    }

} // namespace khthon


