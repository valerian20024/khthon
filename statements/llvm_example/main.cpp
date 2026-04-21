#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Type.h"

#include <map>
#include <vector>

using namespace std;
using namespace llvm;

/* NOTE: As shown in this example, generating llvm code using the C++ API
 * requires to use a lot of classes. There exist a class in the C++ API, called
 * IRBuilder, which groups together many useful methods. For instance, it allows
 * to retrieve easily the different llvm types, to add new instructions, etc.
 * You can choose to use it or not.
 * More info: https://llvm.org/doxygen/classllvm_1_1IRBuilderBase.html
 */

// Used to generate the LLVM module and the types
LLVMContext *context;

// Return a LLVM type corresponding to the one given in argument.
// EG. get_type("int") -> i32
static Type *get_type(string type)
{
    if (type == "int")
        return Type::getInt32Ty(*context);
    else if (type == "bool")
        return Type::getInt1Ty(*context);
    else if (type == "void")
        return Type::getVoidTy(*context);
    else
        exit(1);
}

// Return a LLVM constant i32 with the same value than the argument.
static Constant *get_int(int value)
{
    return ConstantInt::get(
        Type::getInt32Ty(*context), // Type of the int
        value,                      // Its value
        true);                      // It is signed (can have negative values)
}

// Return a LLVM constant i1 with the same value than the argument.
static Constant *get_bool(bool value)
{
    return ConstantInt::get(
        Type::getInt1Ty(*context), // Type of the int
        value ? 1 : 0);            // Its value
}

int main(int argc, char const *argv[])
{
    context = new LLVMContext();
    Module *module = new Module("test", *context);

    /****************** Declare 'malloc' function ******************/
    // void *malloc(size_t size);

    // Signature
    FunctionType *malloc_sign = FunctionType::get(
        Type::getInt8Ty(*context)->getPointerTo(), // The return type
        {Type::getInt64Ty(*context)},              // The arguments
        false);                                    // No variable number of arguments

    // Declare the function
    Function *malloc_function = Function::Create(
        malloc_sign,                  // The signature
        GlobalValue::ExternalLinkage, // The linkage: external, malloc is implemented elsewhere
        "malloc",                     // The name
        module);                      // The LLVM module

    /****************** Define the 'MyClass' structure ******************/
    // Declare structure
    string class_name = "MyClass";
    StructType *class_type = StructType::create(*context, class_name);

    // Declare vtable
    string vtable_name = "struct." + class_name + "VTable";
    StructType *vtable_type = StructType::create(*context, vtable_name);

    /************** Fields **************/
    vector<Type *> class_fields;

    // First field is the pointer towards the vtable
    class_fields.push_back(vtable_type->getPointerTo());

    // Insert the other fields ('integer' and 'boolean')
    class_fields.push_back(get_type("int"));
    class_fields.push_back(get_type("bool"));

    // Link the class to its fields
    class_type->setBody(class_fields);

    /************** Methods **************/
    vector<Type *> methods_types;
    vector<Constant *> methods;
    map<string, FunctionType *> methods_signatures;
    map<string, Function *> methods_functions;

    /******** Method Add ********/
    // void add(int a, int b)
    vector<Type *> method_arguments;

    // First argument is always 'self'
    Type *class_type_ptr = class_type->getPointerTo();
    method_arguments.push_back(class_type_ptr);

    // Other arguments: 'a' and 'b'
    method_arguments.push_back(get_type("int"));
    method_arguments.push_back(get_type("int"));

    // Signature: void (MyClass*, int, int)
    FunctionType *method_type = FunctionType::get(
        get_type("void"), // Return type
        method_arguments, // List of arguments types
        false);           // No variable number of arguments

    methods_types.push_back(method_type->getPointerTo());
    methods_signatures["add"] = method_type;

    // Create the actual function that will implement the method
    Function *method_function = Function::Create(
        method_type,                  // The signature
        GlobalValue::ExternalLinkage, // The linkage (not important here)
        "add",                        // The name
        module);                      // The LLVM module

    // Setting the name of the arguments
    int i = 0;
    for (auto &arg : method_function->args())
    {
        if (i == 0)
            arg.setName("self");

        else if (i == 1)
            arg.setName("a");

        else
            arg.setName("b");

        i++;
    }

    methods_functions["add"] = method_function;
    methods.push_back(method_function);

    // Set the body of the vtable, i.e. pointers with the methods types
    vtable_type->setBody(methods_types);

    // Up to now, we have declare the structure representing the class,
    // and the structure representing the vtable.
    // The vtable is common to all the objects of a given class, it must
    // thus be created as a unique global variable, towards which
    // each object will pointer.

    /************** Defining the vtable **************/
    // Create a constant
    Constant *vtable_const = ConstantStruct::get(
        vtable_type, // Type of the constant structure
        methods);    // Values to give to the different fields

    // Assign the constant to a global variable
    GlobalVariable *vtable = new GlobalVariable(
        *module,                      // The LLVM module
        vtable_type,                  // The type of the constant
        true,                         // It is constant
        GlobalValue::InternalLinkage, // The linkage
        vtable_const,                 // The constant value
        class_name + "_vtable");      // The name of the variable

    /************** New and Init functions **************/
    // Declare function 'new': allocate memory for a new object and return it.
    method_type = FunctionType::get(
        class_type->getPointerTo(), // The return type
        {},                         // The arguments
        false);                     // No variable number of arguments

    Function *new_function = Function::Create(
        method_type,                  // The signature
        GlobalValue::ExternalLinkage, // The linkage
        "new_" + class_name,          // The name
        module);                      // The LLVM module

    // Declare function 'init': initialize an object.
    method_type = FunctionType::get(
        class_type->getPointerTo(),   // The return type
        {class_type->getPointerTo()}, // The arguments
        false);                       // No variable number of arguments

    Function *init_function = Function::Create(
        method_type,                  // The signature
        GlobalValue::ExternalLinkage, // The linkage
        "init_" + class_name,         // The name
        module);                      // The LLVM module

    init_function->arg_begin()->setName("self");

    /****************** Implement the functions ******************/
    // Up to now, the functions are only declared, they must still be implemented.

    /******** Implement the 'new' function ********/
    // First create an entry point.
    BasicBlock *new_entry = BasicBlock::Create(
        *context,      // The LLVM context
        "entry",       // The label of the block
        new_function); // The function in which should be inserted the block

    // To allocate a structure, needs to know its size.
    // Use the trick shown in the slides
    GetElementPtrInst *ptr_size = GetElementPtrInst::Create(
        class_type,                                           // The pointed type
        ConstantPointerNull::get(class_type->getPointerTo()), // The address
        {get_int(1)},                                         // The index of the element we want
        "",                                                   // Name of the LLVM variable (not fixed here)
        new_entry);                                           // The block in which the instruction will be inserted

    CastInst *bytes_size = CastInst::CreatePointerCast(
        ptr_size,                   // The value to cast
        Type::getInt64Ty(*context), // The casting type
        "",                         // Name of the LLVM variable (not fixed here)
        new_entry);                 // The block in which the instruction will be inserted

    // Now that we have the size, we can call malloc:
    Value *struct_ptr = CallInst::Create(
        malloc_function, // Name of the function to call
        {bytes_size},    // Arguments
        "",              // Name of the LLVM variable (not fixed here)
        new_entry);      // The block in which the instruction will be inserted

    // Malloc returns a i8 pointer, we have to cast it as a pointer towards our structure
    Value *cast_struct_ptr = CastInst::CreatePointerCast(
        struct_ptr,                 // The value to cast
        class_type->getPointerTo(), // The casting type
        "",                         // Name of the LLVM variable (not fixed here)
        new_entry);                 // The block in which the instruction will be inserted

    // The new object needs to be initialized, thus we call the 'init' function
    Value *initialized_struct = CallInst::Create(
        init_function,     // Name of the function to call
        {cast_struct_ptr}, // Arguments
        "",                // Name of the LLVM variable (not fixed here)
        new_entry);        // The block in which the instruction will be inserted

    // The last thing to do is to return the initialized object:
    ReturnInst::Create(
        *context,           // The LLVM context
        initialized_struct, // The value to return
        new_entry);         // The block in which the instruction will be inserted

    /******** Implement the 'init' function ********/
    // First create an entry point.
    BasicBlock *init_entry = BasicBlock::Create(
        *context,       // The LLVM context
        "entry",        // The label of the block
        init_function); // The function in which should be inserted the block

    // The 'init' function must initialized all the fields correctly.
    // The first field is the vtable, the second is 'integer' and the third 'boolean'.

    // First, need to get 'self', the first argument:
    Value *arg_self = &(*(init_function->arg_begin()));

    // Then get the address of the first field:
    Value *vtable_ptr = GetElementPtrInst::Create(
        class_type,   // The pointed type
        arg_self,     // The address
        {get_int(0),  // First element "of the array" (no array here, only one element, but it is required)
         get_int(0)}, // First field
        "",           // Name of the LLVM variable (not fixed here)
        init_entry);  // The block in which the instruction will be inserted

    // Then store a pointer towards the vtable at this address:
    new StoreInst(
        vtable,      // The value to store
        vtable_ptr,  // The address where the value has to be stored
        init_entry); // The block in which the instruction will be inserted

    // We can then initialize the two other fields.
    // Let's put '10' in 'integer' and 'true' in boolean.

    // Load address of 'integer':
    Value *integer_ptr = GetElementPtrInst::Create(
        class_type,   // The pointed type
        arg_self,     // The address
        {get_int(0),  // First element "of the array" (no array here, only one element, but it is required)
         get_int(1)}, // Second field
        "",           // Name of the LLVM variable (not fixed here)
        init_entry);  // The block in which the instruction will be inserted

    // Store '10' in it:
    new StoreInst(
        get_int(10), // The value to store
        integer_ptr, // The address where the value has to be stored
        init_entry); // The block in which the instruction will be inserted

    // Load address of 'boolean':
    Value *boolean_ptr = GetElementPtrInst::Create(
        class_type,   // The pointed type
        arg_self,     // The address
        {get_int(0),  // First element "of the array" (no array here, only one element, but it is required)
         get_int(2)}, // Third field
        "",           // Name of the LLVM variable (not fixed here)
        init_entry);  // The block in which the instruction will be inserted

    // Store 'true' in it:
    new StoreInst(
        get_bool(true), // The value to store
        boolean_ptr,    // The address where the value has to be stored
        init_entry);    // The block in which the instruction will be inserted

    // All the fields have been initialized, the object can be returned:
    ReturnInst::Create(
        *context,    // The LLVM context
        arg_self,    // The value to return
        init_entry); // The block in which the instruction will be inserted

    /******** Implement the 'add' method ********/
    // First retrieve the function:
    Function *add_function = methods_functions["add"];

    // Create the entry block:
    BasicBlock *add_entry = BasicBlock::Create(
        *context,      // The LLVM context
        "entry",       // The label of the block
        add_function); // The function in which should be inserted the block

    // Then implement the method.

    // Get the arguments
    Value *arg_a, *arg_b;
    i = 0;
    for (auto &arg : method_function->args())
    {
        if (i == 0)
            arg_self = &arg;

        else if (i == 1)
            arg_a = &arg;

        else
            arg_b = &arg;

        i++;
    }

    // Load the value of 'integer':
    integer_ptr = GetElementPtrInst::Create(
        class_type,   // The pointed type
        arg_self,     // The address
        {get_int(0),  // First element "of the array" (no array here, only one element, but it is required)
         get_int(1)}, // Second field
        "",           // Name of the LLVM variable (not fixed here)
        add_entry);   // The block in which the instruction will be inserted

    Value *integer_val = new LoadInst(
        get_type("int"), // The type of the value to load
        integer_ptr,     // The address of the value to load
        "",              // Name of the LLVM variable (not fixed here)
        add_entry);      // The block in which the instruction will be inserted

    // Add 'integer' and 'a':
    Value *sum = BinaryOperator::Create(
        Instruction::Add, // The binary operator
        integer_val,      // The left operand
        arg_a,            // The right operand
        "",               // Name of the LLVM variable (not fixed here)
        add_entry);       // The block in which the instruction will be inserted

    // Add 'sum' and 'b':
    sum = BinaryOperator::Create(
        Instruction::Add, // The binary operator
        sum,              // The left operand
        arg_b,            // The right operand
        "",               // Name of the LLVM variable (not fixed here)
        add_entry);       // The block in which the instruction will be inserted

    // Store the final sum in 'integer':
    new StoreInst(
        sum,         // The value to store
        integer_ptr, // The address where the value has to be stored
        add_entry);  // The block in which the instruction will be inserted

    // Finally, return
    ReturnInst::Create(
        *context,   // The LLVM context
        add_entry); // The block in which the instruction will be inserted

    /****************** Implement the 'main' function ******************/
    // The last thing to do is to implement the 'main' function, the entry point of the program.

    // Create its signature:
    FunctionType *main_sign = FunctionType::get(
        get_type("int"), // The return type
        {},              // The arguments
        false);          // No variable number of arguments

    // Create the function
    Function *main_func = Function::Create(
        main_sign,                    // The signature
        GlobalValue::ExternalLinkage, // The linkage: 'main' will be called from outside
        "main",                       // The name
        module);                      // The LLVM module

    // Create the entry point
    BasicBlock *main_entry = BasicBlock::Create(
        *context,   // The LLVM context
        "entry",    // The label of the block
        main_func); // The function in which should be inserted the block

    // The 'main' function do three things:
    // - Create new object
    // - Call the 'add' method
    // - Return '0'

    // First, call the new function:
    Value *object = CallInst::Create(
        new_function, // Name of the function to call
        {},           // Arguments
        "",           // Name of the LLVM variable (not fixed here)
        main_entry);  // The block in which the instruction will be inserted

    // Then get the vtable:
    vtable_ptr = GetElementPtrInst::Create(
        class_type,   // The pointed type
        object,       // The address
        {get_int(0),  // First element "of the array" (no array here, only one element, but it is required)
         get_int(0)}, // First field
        "",           // Name of the LLVM variable (not fixed here)
        main_entry);  // The block in which the instruction will be inserted

    Value *vtable_val = new LoadInst(
        vtable_type->getPointerTo(), // The type of the value to load
        vtable_ptr,                  // The address of the value to load
        "",                          // Name of the LLVM variable (not fixed here)
        main_entry);                 // The block in which the instruction will be inserted

    // Load the address of the 'add' method:
    Value *add_ptr = GetElementPtrInst::Create(
        vtable_type,  // The pointed type
        vtable_val,   // The address
        {get_int(0),  // First element "of the array" (no array here, only one element, but it is required)
         get_int(0)}, // First field
        "",           // Name of the LLVM variable (not fixed here)
        main_entry);  // The block in which the instruction will be inserted

    Value *add_val = new LoadInst(
        methods_signatures.at("add")->getPointerTo(), // The type of the value to load
        add_ptr,                                      // The address of the value to load
        "",                                           // Name of the LLVM variable (not fixed here)
        main_entry);                                  // The block in which the instruction will be inserted

    // Call 'add':
    CallInst::Create(
        methods_signatures["add"], // The signature
        add_val,                   // The function
        {object,
         get_int(1),
         get_int(2)}, // The arguments
        "",           // Name of the LLVM variable (not fixed here)
        main_entry    // The block in which the instruction will be inserted
    );

    // Return '0':
    ReturnInst::Create(
        *context,   // The LLVM context
        get_int(0), // The value to return
        main_entry  // The block in which the instruction will be inserted
    );

    module->print(outs(), nullptr);
}
