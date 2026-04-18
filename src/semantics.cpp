#include "semantics.hpp"

#include <unordered_set>
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

    TypesVisitor tv = TypesVisitor(driver_, *this);
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
    ||                 TYPESVISITOR                   ||
    ++================================================*/

bool TypesVisitor::conforms(const Type& given, const Type& expected) const {
    
    // Undefined types should never reach type checking.
    if (given.is_undefined() || expected.is_undefined()) {
        driver_.internal_error("conforms(): encountered an undefined type");
        return false;
    }

    // Primitive types conform only with themselves.
    if (given.is_primitive() || expected.is_primitive())
        return given == expected;


    // Types are custom so we check for subtyping.
    return is_subtype(given, expected);
}

bool TypesVisitor::is_subtype(const Type& given, const Type& compared_to) const {
    
    // Subtyping is only available for classes.
    if (!given.is_custom() || !compared_to.is_custom()) {
        driver_.internal_error("is_subtype(): called with non custom types");
        return false;
    }

    string current = given.custom_name();
    while (true) {

        if (current == compared_to.custom_name())
            return true;
        
        if (current == "Object")
            return false;
        
        if (!checker_.class_exists(current)) {
            driver_.internal_error(
                "is_subtype(): class '" + current + "'not found in class table"
            );
            return false;
        }

        current = checker_.get_class(current).parent();
    }
}

Type TypesVisitor::ancestor(const Type& t1, const Type& t2) const {

    // This method only applies to custom types.
    if (!t1.is_custom() || !t2.is_custom()) {
        driver_.internal_error("ancestor(): called with non custom types");
        return Type::Object();
    }

    // Types agree so their common ancestor is themselves.
    if (t1 == t2)
        return t1;
    
    // Collect the full ancestry chain of t1 into an ordered list.
    vector<string> ancestors;
    string current = t1.custom_name();

    while (true) {
        ancestors.push_back(current);
        if (current == "Object")
            break;

        if (!checker_.class_exists(current)) {
            driver_.internal_error(
                "ancestor(): class '" + current + "' not found in class table."
            );
            return Type::Object();
        }

        current = checker_.get_class(current).parent();
    }

    // Walk up t2's ancestors and return the first class found in t1's ancestors
    unordered_set<string>ancestors_set(
        ancestors.begin(), ancestors.end());

    current = t2.custom_name();
    while (true) {

        if (ancestors_set.count(current))
            return Type(current);

        if (current == "Object")
            return Type("Object");  // Fallback

        if (!checker_.class_exists(current)) {
            driver_.internal_error(
                "ancestor(): class '" + current + "' not found in class table."
            );
            return Type::Object();
        }

        current = checker_.get_class(current).parent();
    }
}

bool TypesVisitor::check_unop_operand(
    const UnaryOperation& operation,
    const Type& t_operand
) const {
    
    // If the given operand is one of those expected by the operation.
    for (const auto& expected : operation.valid_operand_types()) {
        if (conforms(t_operand, expected))
            return true;
    }
    return false;
}

bool TypesVisitor::check_binop_operands(
    const BinaryOperation& op,
    const Type& t_left,
    const Type& t_right
) const {

    // Handling the special case of equality.
    if (op.is_equality())
        return (t_left == t_right) || (t_left.is_custom() && t_right.is_custom());

    // If the given operands are among the pairs expected by the operation.
    for (const auto& [expected_left, expected_right] : op.valid_operand_types()) {
        if (conforms(t_left, expected_left) && conforms(t_right, expected_right))
            return true;
    }
    return false;
}

void TypesVisitor::visit(ProgramNode& node) {
    cout << "TypesVisitor::visit(ProgramNode" << endl;
    for (const auto& c : node.classes())
        c->accept(*this);
}

void TypesVisitor::visit(ClassNode& node) {
    cout << "TypesVisitor::visit(ClassNode" << endl;
    
    current_class_name_ = node.name();
    // Have fresh scope for new class
    
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
    
    // Guardian must have type bool.
    if (!conforms(node.guardian()->type(), Type::Bool())) {
        driver_.semantic_error(
            node.location(),
            "condition of 'if' must be of type 'bool', found '"
            + node.guardian()->type().to_string()
            + "'"
        );
    }

    const Type consequent_type = node.consequent()->type();
    const Type alternative_type = node.alternative().has_value()
        ? node.alternative().value()->type()
        : Type::Unit();
    
    // If at least one branch is unit, the node is unit.
    if (consequent_type.is_unit() || alternative_type.is_unit()) {
        node.set_type(Type::Unit());

    // If branches have class types, the node has the type of their ancestor.
    } else if (consequent_type.is_custom() && alternative_type.is_custom()) {
        node.set_type(ancestor(consequent_type, alternative_type));

    // If branches have non-unit, primitive types.
    } else if (consequent_type == alternative_type) {
        node.set_type(consequent_type);

    } else {
        driver_.semantic_error(
            node.location(),
            "branches of 'if' have incompatible types '"
            + consequent_type.to_string()
            + "' and '"
            + alternative_type.to_string()
            + "'"
        );

        node.set_type(Type::Object());  // error recovery
    }
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

    const Type& t_operand = node.operand()->type();

    if(!check_unop_operand(node.operation(), t_operand)) {
        driver_.semantic_error(
            node.location(),
            "operator '" + node.operation().to_string()
            + "' cannot be applied to type '"
            + t_operand.to_string() + "'"
        );

        node.set_type(Type::Object());
        return;
    }

    node.set_type(node.operation().result_type());
}

void TypesVisitor::visit(BinOpExpr& node) { 
    cout << "TypesVisitor::visit(BinOpExpr" << endl;

    node.left()->accept(*this);
    node.right()->accept(*this);  
    
    const Type& t_left  = node.left()->type();
    const Type& t_right = node.right()->type();

    if (!check_binop_operands(node.operation(), t_left, t_right)) {
        driver_.semantic_error(
            node.location(),
            "operator '" + node.operation().to_string()
            + "' cannot be applied to types '"
            + t_left.to_string() + "' and '"
            + t_right.to_string() + "'"
        );

        node.set_type(Type::Object());
        return;
    }

    node.set_type(node.operation().result_type());
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
    
    node.condition()->accept(*this);
    node.body()->accept(*this);

    if (!conforms(node.condition()->type(), Type::Bool())) {
        driver_.semantic_error(
            node.location(),
            "condition of 'while' must be of type 'bool', found '"
            + node.condition()->type().to_string()
            + "'"
        );
    }

    if (node.body()->type().is_undefined())
        driver_.internal_error("visiting WhileExpr found an undefined type");

    node.set_type(Type::Unit());
}
