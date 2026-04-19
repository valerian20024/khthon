#include "semantics.hpp"

#include <unordered_set>
#include <string>
#include <vector>
#include <functional>
#include <map>

using namespace std;

using Khthon::Type;

namespace Khthon {


        /*================================================++
        ||                CORE PROCEDURES                 ||
        ++================================================*/

    bool ClassManager::add_class(ClassInfo c) {
        auto [it, inserted] = class_table_.emplace(c.name(), std::move(c));
        return inserted;
    }

    bool ClassManager::class_exists(const std::string& name) const { 
        return class_table_.count(name) > 0; 
    }

    const std::optional<ClassInfo> ClassManager::get_class(const std::string& name) const {
        auto it = class_table_.find(name);
        if (it == class_table_.end())
            return std::nullopt;
        return it->second;
    }

    const optional<bool> ClassManager::is_subtype(const Type& given, const Type& compared_to) const {
        
        // Subtyping is only available for classes.
        if (!given.is_custom() || !compared_to.is_custom())
            return nullopt;

        string current = given.custom_name();
        while (true) {

            if (current == compared_to.custom_name())
                return true;
            
            if (current == "Object")
                return false;
            
            if (!class_exists(current))
                return nullopt;

            auto info = get_class(current);
            if (!info)
                return nullopt;

            current = info->parent();
        }
    }

    optional<Type> ScopeManager::lookup(const string& name) const {
        // Walk the stack from innermost to outermost scope.
        for (auto it = scope_table_.rbegin(); it != scope_table_.rend(); ++it) {
            auto found = it->find(name);

            if (found != it->end())
                return found->second;
        }
        return std::nullopt;
    }

    void SemanticChecker::analyze(const shared_ptr<ProgramNode>& root) {
        if (!root)
            driver_.internal_error("SemanticChecker::analyze(): No ast root.");

        ClassesVisitor cv = ClassesVisitor(driver_, *this);
        root->accept(cv);

        check_main();
        check_parent_classes_exist();
        check_inheritance_cycles();

        TypesVisitor tv = TypesVisitor(driver_, *this);
        root->accept(tv);

        if (enable_advanced_logging)
            print_class_table();
    }

    void SemanticChecker::check_main() const {
        
        auto main_info = get_class("Main");
        if (!main_info) {
            driver_.semantic_error(
                driver_.default_location(),  // no meaningful location
                "no 'Main' class defined"
            );
            return;
        }

        // Check main method exists
        const auto& methods = main_info->methods();
        auto main_method = methods.find("main");
        if (main_method == methods.end()) {
            driver_.semantic_error(
                main_info->location(),
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
                + return_type.to_string() + "'"
            );
        }
    }

    void SemanticChecker::check_parent_classes_exist() const {
        for (const auto& [name, info] : class_manager_.table()) {
            const string& parent = info.parent();
            
            // built-in root, always valid
            if (parent == "Object") 
                continue;
            
            if (!class_exists(parent)) {
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

        auto info = get_class(name);
        if (!info) {
            driver_.internal_error(
                "cycle_check(): unable to get class '" + name + "'"
            );
            return false;
        }

        const string& parent = info->parent();

        if (parent != "Object" && parent != "") {

            if (!class_exists(parent)) {
                driver_.internal_error(
                    "class '" + parent + "' is not part of the symbol table."
                );
                return true;
            }

            if (states[parent] == VisitState::Visiting) {
                driver_.semantic_error(
                    info->location(),
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

        for (const auto& [name, info] : class_manager_.table())
            states[name] = VisitState::Unvisited;

        for (const auto& [name, info] : class_manager_.table())
            if (states[name] == VisitState::Unvisited)
                cycle_check(name, states);
    }

    void SemanticChecker::print_class_table() const {
        for (const auto& [class_name, class_info] : class_manager_.table()) {
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

}