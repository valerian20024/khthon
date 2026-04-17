#include "semantics.hpp"

#include <string>
#include <vector>
#include <functional>
#include <map>

using namespace std;

using Khthon::Type;


    /*================================================++
    ||          TYPESVISITOR VISIT METHODS            ||
    ++================================================*/


void ClassesVisitor::visit(const ProgramNode& node) const {
    
    // Inject the built-in Object class
    Khthon::location builtin_loc;  // default location, no source file
    ClassInfo object_info("Object", "Object", builtin_loc);

    // Object's built-in methods
    object_info.add_method(MethodInfo(
        "print",
        Type("Object"),
        { FormalInfo("s", Type::String(), builtin_loc) },
        builtin_loc
    ));

    object_info.add_method(MethodInfo(
        "printBool",
        Type("Object"),
        { FormalInfo("b", Type::Bool(), builtin_loc) },
        builtin_loc
    ));

    object_info.add_method(MethodInfo(
        "printInt32",
        Type("Object"),
        { FormalInfo("i", Type::Int32(), builtin_loc) },
        builtin_loc
    ));

    object_info.add_method(MethodInfo(
        "inputLine",
        Type::String(),
        {},  // no formals
        builtin_loc
    ));

    object_info.add_method(MethodInfo(
        "inputBool",
        Type::Bool(),
        {},
        builtin_loc
    ));

    object_info.add_method(MethodInfo(
        "inputInt32",
        Type::Int32(),
        {},
        builtin_loc
    ));

    class_table_.emplace("Object", std::move(object_info));

    // Reading concrete classes of the program.
    for (const auto& c : node.classes())
        c->accept(*this);
}

void ClassesVisitor::visit(const ClassNode& node) const {
    const string& class_name = node.name();

    // Not adding duplicate classes.
    if (class_table_.count(class_name)) {
        driver_.semantic_error(
            node.location(), 
            "class '" + class_name + "' is defined more than once"
        );
        return;
    }

    ClassInfo class_info(class_name, node.parent(), node.location());

    // Fields.
    for (const auto& field : node.fields()) {
        FieldInfo field_info(
            field->name(), 
            field->type(), 
            field->location()
        );

        if (!class_info.add_field(std::move(field_info))) {
            driver_.semantic_error(
                field->location(), 
                "field '" + field->name() + "' is defined more than once"
            );
        }
    }

    // Methods.
    for (const auto& method : node.methods()) {
        // Formals.
        vector<FormalInfo> formals_infos;
        for (const auto& formal : method->formals()) {
            formals_infos.emplace_back(
                formal->name(), 
                formal->type(), 
                formal->location()
            );
        }
        
        MethodInfo method_info(
            method->name(), 
            method->type(), 
            std::move(formals_infos), 
            method->location()
        );

        if (!class_info.add_method(std::move(method_info))) {
            driver_.semantic_error(
                method->location(),
                "method '" + method->name() + "' is defined more than once"
            );
        }
    }

    class_table_.emplace(class_name, std::move(class_info));
}


    /*================================================++
    ||          SEMANTIC CHECKS PROCEDURES            ||
    ++================================================*/


void SemanticChecker::analyze(const shared_ptr<ProgramNode>& root) {
    if (!root)
        driver_.internal_error("SemanticChecker::analyze(): No ast root.");

    ClassesVisitor cv = ClassesVisitor(driver_, class_table_);
    root->accept(cv);

    check_main();
    check_parent_classes_exist();
    check_inheritance_cycles();

    TypesVisitor tv = TypesVisitor(driver_, class_table_);
    root->accept(tv);

    #ifdef DEBUG
        print_class_table();
    #endif
}

void SemanticChecker::check_main() const {
    // Looking for a Main class.
    auto main_class = class_table_.find("Main");
    if (main_class == class_table_.end()) {
        //todo  Initialize location to filename: 1: 1
        //todo  Maybe create a helper in driver?
        driver_.semantic_error(
            Khthon::location(),  // no meaningful location
            "no 'Main' class defined"
        );
        return;
    }

    const ClassInfo& main_info = main_class->second;

    // Check main method exists
    const auto& methods = main_info.methods();
    auto main_method = methods.find("main");
    if (main_method == methods.end()) {
        driver_.semantic_error(
            main_info.location(),
            "class 'Main' has no 'main' method"
        );
        return;
    }

    const MethodInfo& method_info = main_method->second;

    // Check main takes no formals
    if (!method_info.formals().empty()) {
        driver_.semantic_error(
            method_info.location(),
            "method 'main' must take no arguments"
        );
    }

    // Check main returns int32
    const Type& return_type = method_info.return_type();
    if (!return_type.is_int32()) {
        driver_.semantic_error(
            method_info.location(),
            "method 'main' must return 'int32', found '" 
            + return_type.to_string() 
            + "'"
        );
    }
}

void SemanticChecker::check_parent_classes_exist() const {
    for (const auto& [name, info] : class_table_) {
        const string& parent = info.parent();
        
        // built-in root, always valid
        if (parent == "Object") 
            continue;
        
        if (!class_table_.count(parent)) {
            driver_.semantic_error(
                info.location(),
                "class '" + name + "' extends unknown class '" + parent + "'"
            );
        }
    }
}

bool SemanticChecker::cycle_check(
    const string& name,
    map<string, VisitState>& states
) const {

    states[name] = VisitState::Visiting;

    const string& parent = class_table_.at(name).parent();

    if (parent != "Object" && parent != "") {

        if (class_table_.find(parent) == class_table_.end()) {
            driver_.internal_error("class '" + parent + "' is not part of the symbol table.");
            return true;
        }

        if (states[parent] == VisitState::Visiting) {
            driver_.semantic_error(
                class_table_.at(name).location(),
                "inheritance cycle detected involving class '" + name + "'"
            );
            return false;
        }

        if (states[parent] == VisitState::Unvisited)
            if (!cycle_check(parent, states))
                return false;

        // VisitState::Visited means already fully explored, safe to skip
    }

    states[name] = VisitState::Visited;
    return true;
}

void SemanticChecker::check_inheritance_cycles() {

    map<string, VisitState> states;
    for (const auto& [name, info] : class_table_)
        states[name] = VisitState::Unvisited;

    for (const auto& [name, info] : class_table_)
        if (states[name] == VisitState::Unvisited)
            cycle_check(name, states);
}

void SemanticChecker::print_class_table() const {
    for (const auto& [class_name, class_info] : class_table_) {
        // Class and inheritance.
        cout << "-------------------------\n"
            << "Class: " << class_name 
            << " extends " << class_info.parent() << "\n";

        // Fields.
        cout << "  Fields:\n";
        if (class_info.fields().empty()) {
            cout << "    (none)\n";
        } else {
            for (const auto& [field_name, field_info] : class_info.fields()) {
                cout << "    " 
                    << field_info.name() 
                    << " : " 
                    << field_info.type().to_string() << "\n";
            }
        }

        // Methods.
        cout << "  Methods:\n";
        if (class_info.methods().empty()) {/*  */
            cout << "    (none)\n";
        } else {
            for (const auto& [method_name, method_info] : class_info.methods()) {
                cout << "    " 
                    << method_info.name()
                    << "(";
                
                // Formals.
                const auto& formals = method_info.formals();
                for (size_t i = 0; i < formals.size(); ++i) {
                    if (i > 0) cout << ", ";
                    cout << formals[i].name() 
                        << " : " 
                        << formals[i].type().to_string();
                }

                cout << ") : " << method_info.return_type().to_string() << "\n";
            }
        }
        cout << "-------------------------\n" << endl;
    }
}


    /*================================================++
    ||          TYPESVISITOR VISIT METHODS            ||
    ++================================================*/


void TypesVisitor::visit(ProgramNode& node) {
    cout << "TypesVisitor::visit(ProgramNode" << endl;
    for (const auto& c : node.classes())
        c->accept(*this);
}

void TypesVisitor::visit(ClassNode& node) {
    cout << "TypesVisitor::visit(ClassNode" << endl;
    
    current_class_name_ = node.name();
    scope_stack_.clear();  // fresh scope for new class
    
    for (const auto& f : node.fields())
        f->accept(*this);

    for (const auto& m : node.methods())
        m->accept(*this);
}

void TypesVisitor::visit(MethodNode& node) {
    // New scope
    cout << "TypesVisitor::visit(MethodNode& node" << endl;

    //? visiting formals?

    node.body()->accept(*this);
    
    // Pop scope 
}

void TypesVisitor::visit(FormalNode& node) {
    cout << "TypesVisitor::visit(FormalNode" << endl;
    (void) node;
    return;
}

void TypesVisitor::visit(FieldNode& node) {
    cout << "TypesVisitor::visit(FieldNode" << endl;
    if (node.has_init())
        node.initializer().value()->accept(*this);
}

void TypesVisitor::visit(BlockExpr& node) {
    //? New scope
    cout << "TypesVisitor::visit(BlockExpr& node" << endl;

    for (const auto& e : node.expressions())
        e->accept(*this);
    
    // Set type of last expression

    //? Pop scope
}

void TypesVisitor::visit(StringLiteralExpr& node) { 
    cout << "TypesVisitor::visit(StringLiteralExpr" << endl;
    node.set_type(Type::String());
}

void TypesVisitor::visit(IntegerLiteralExpr& node) { 
    cout << "TypesVisitor::visit(IntegerLiteralExpr" << endl;
    node.set_type(Type::Int32());
}

void TypesVisitor::visit(BoolLiteralExpr& node) { 
    cout << "TypesVisitor::visit(BoolLiteralExpr" << endl;
    node.set_type(Type::Bool());
}

void TypesVisitor::visit(UnitLiteralExpr& node) { 
    cout << "TypesVisitor::visit(UnitLiteralExpr" << endl;
    node.set_type(Type::Unit());
}

void TypesVisitor::visit(IfExpr& node) { 
    cout << "TypesVisitor::visit(IfExpr" << endl;
    node.guardian()->accept(*this);
    node.consequent()->accept(*this);
    
    if (node.alternative().has_value())
        node.alternative().value()->accept(*this);
}

void TypesVisitor::visit(AssignExpr& node) {
    cout << "TypesVisitor::visit(AssignExpr" << endl;
    node.value()->accept(*this);
}

void TypesVisitor::visit(NewExpr& node) {
    cout << "TypesVisitor::visit(NewExpr" << endl;
    (void) node;
    return;
}

void TypesVisitor::visit(UnOpExpr& node) { 
    cout << "TypesVisitor::visit(UnOpExpr" << endl;
    node.operand()->accept(*this);
}

void TypesVisitor::visit(BinOpExpr& node) { 
    cout << "TypesVisitor::visit(BinOpExpr" << endl;
    node.left()->accept(*this);
    node.right()->accept(*this);  
    // compare types based on the operation
}

void TypesVisitor::visit(VariableExpr& node) { 
    cout << "TypesVisitor::visit(VariableExpr" << endl;
    (void) node;
    return;
}

void TypesVisitor::visit(CallExpr& node) { 
    cout << "TypesVisitor::visit(CallExpr" << endl;
    node.receiver()->accept(*this);
    for (const auto& arg : node.args())
        arg->accept(*this);
    
    // Do something with dispatching
}

void TypesVisitor::visit(SelfExpr& node) { 
    cout << "TypesVisitor::visit(SelfExpr" << endl;
    (void) node;
    return;
}

void TypesVisitor::visit(LetExpr& node) { 
    cout << "TypesVisitor::visit(LetExpr" << endl;
    // New scope

    if (node.has_initializer())
        node.initializer().value()->accept(*this);

    node.scope()->accept(*this);
    
    // Pop scope
}

void TypesVisitor::visit(WhileExpr& node) { 
    cout << "TypesVisitor::visit(WhileExpr" << endl;
    //? New scope
    node.condition()->accept(*this);  // must be bool
    node.body()->accept(*this);  // can be anything
    // set type of while to unit
    //? Pop scope
}
