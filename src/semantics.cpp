/**
 * This file contains the implementations of the semantic analysis, 
 * excluding the visitors. It contains methods from Info classes as
 * well as the SemanticChecker implementation, which is the central
 * orchestrator for the semantic analysis phase.
 */

#include "semantics.hpp"

#include <unordered_set>
#include <string>
#include <vector>
#include <functional>
#include <map>

using namespace std;

using khthon::Type;

namespace khthon {


    /*================================================++
    ||                INFORMATIONS                    ||
    ++================================================*/

    FieldInfo::FieldInfo(
        string              name, 
        Type                type, 
        khthon::location    loc
    ) : 
        name_(std::move(name)), 
        type_(std::move(type)), 
        location_(std::move(loc)) 
    {}

    FormalInfo::FormalInfo(
        string              name,
        Type                type,
        khthon::location    loc
    ) : 
        name_(std::move(name)), 
        type_(std::move(type)), 
        location_(std::move(loc)) 
    {}

    MethodInfo::MethodInfo(
        string                  name,
        khthon::Type            return_type,
        vector<FormalInfo>      formals,
        khthon::location        loc
    ) : 
        name_(std::move(name)),
        return_type_(std::move(return_type)),
        formals_(std::move(formals)),
        location_(std::move(loc))
    {}

    bool MethodInfo::has_formals() const {
        return !formals_.empty();
    }

    int MethodInfo::formals_count() const {
        return static_cast<int>(formals_.size());
    }

    ClassInfo::ClassInfo(
        std::string         name, 
        std::string         parent, 
        khthon::location    loc
    ) : 
        name_(std::move(name)), 
        parent_(std::move(parent)), 
        location_(std::move(loc)) 
    {}

    ClassInfo ClassInfo::Dummy() {
        return ClassInfo("Dummy", "Object", khthon::location());
    }

    bool ClassInfo::add_field(FieldInfo f) {
        // Check for duplicates
        for (const auto& existing : fields_)
            if (existing.name() == f.name())
                return false;

        fields_.push_back(std::move(f));
        return true;
    }

    bool ClassInfo::add_method(MethodInfo m) {
        // Check for duplicates
        for (const auto& existing : methods_)
            if (existing.name() == m.name())
                return false;

        methods_.push_back(std::move(m));
        return true;
    }

    bool ClassInfo::has_field(const string field_name) const {
        return get_field(field_name).has_value();
    }

    bool ClassInfo::has_method(const string method_name) const {
        return get_method(method_name).has_value();
    }

    optional<FieldInfo> ClassInfo::get_field(const string name) const {
        for (const auto& f : fields_)
            if (f.name() == name)
                return f;
        return nullopt;
    }

    optional<MethodInfo> ClassInfo::get_method(const string name) const {
        for (const auto& m : methods_)
            if (m.name() == name)
                return m;
        return nullopt;
    }


    /*================================================++
    ||                  CLASS MANAGER                 ||
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

    const optional<bool> ClassManager::is_subtype(
        const Type& given, 
        const Type& compared_to
    ) const {
        
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

    Type ClassManager::ancestor(
        const Type& t1, 
        const Type& t2
    ) const {
        
        // Collect the full ancestry chain of t1 into an ordered list.
        vector<string> ancestors;
        
        string current = t1.custom_name();
        while (true) {
            ancestors.push_back(current);
            if (current == "Object")
                break;

            current = get_class(current)->parent(); 
        }

        // Walk up t2's ancestors and return the first class found in t1's ancestors
        unordered_set<string>ancestors_set(
            ancestors.begin(), 
            ancestors.end()
        );

        current = t2.custom_name();
        while (true) {
            if (ancestors_set.count(current))
                return Type(current);

            if (current == "Object")
                return Type("Object");  // Fallback

            current = get_class(current)->parent(); 
        }
    }

    bool SemanticChecker::has_field(
        const std::string& field_name, 
        const std::string& class_name
    ) const {
        return lookup_field(field_name, class_name).has_value();
    }

    bool SemanticChecker::has_method(
        const std::string& method_name,
        const std::string& class_name
    ) const {
        return lookup_method(method_name, class_name).has_value();
    }

    optional<FieldInfo> ClassManager::lookup_field(
        const string& field_name,
        const string& class_name
    ) const {

        string candidate = class_name;
        while (!candidate.empty()) {
            auto info = get_class(candidate);
            if (!info)
                return nullopt;

            const auto& fields = info->fields();
            auto it = fields.begin();
            while (it != fields.end()) {
                if (it->name() == field_name)
                    return *it;
                ++it;
            }

            // Stop after Object, but still check it first.
            if (candidate == "Object")
                break;

            candidate = info->parent();
        }

        return nullopt;
    }

    optional<MethodInfo> ClassManager::lookup_method(
        const string& method_name,
        const string& class_name
    ) const {
        string candidate = class_name;
        while (!candidate.empty()) {
            auto info = get_class(candidate);
            if (!info)
                return nullopt;

            const auto& methods = info->methods();
            auto it = methods.begin();
            while (it != methods.end()) {
                if (it->name() == method_name)
                    return *it;
                ++it;
            }

            // Stop after Object, but check its methods first.
            if (candidate == "Object")
                break;

            candidate = info->parent();
        }
        return nullopt;
    }
    
    vector<FieldInfo> ClassManager::collect_fields(
        const string class_name
    ) const {
        // Collect ancestry chain from child to root.
        vector<ClassInfo> ancestry;
        string current = class_name;

        while (true) {
            auto info_opt = get_class(current);
            if (!info_opt) 
                break;

            ancestry.push_back(info_opt.value());

            if (current == "Object") 
                break;

            current = info_opt->parent();
        }

        // Walk from root to child so fields are in base-first order.
        vector<FieldInfo> result;
        for (auto it = ancestry.rbegin(); it != ancestry.rend(); ++it) {
            const auto& fields = it->fields();
            result.insert(result.end(), fields.begin(), fields.end());
        }

        return result;
    }
    
    vector<MethodInfo> ClassManager::collect_methods(
        const string class_name
    ) const {
        // Collect ancestry chain from child to root.
        vector<ClassInfo> ancestry;
        string current = class_name;

        while (true) {
            auto info_opt = get_class(current);
            if (!info_opt)
                break;

            ancestry.push_back(info_opt.value());

            if (current == "Object")
                break;

            current = info_opt->parent();
        }

        // Walk from root to child, respecting overrides.
        vector<MethodInfo> result;
        map<string, size_t> slot_map;  // method name -> index in result

        for (auto it = ancestry.rbegin(); it != ancestry.rend(); ++it) {
            for (const auto& method : it->methods()) {
                
                auto existing = slot_map.find(method.name());
                if (existing != slot_map.end()) {
                    // Override: replace at the existing slot, preserving the index.
                    result[existing->second] = method;
                } else {
                    // New method: assign the next available slot.
                    slot_map[method.name()] = result.size();
                    result.push_back(method);
                }
            }
        }

        return result;
    }
    
    
    /*================================================++
    ||                 SCOPE MANAGER                  ||
    ++================================================*/

    
    void ScopeManager::push_scope() { 
        scope_table_.push_back({}); 
    }
    
    void ScopeManager::pop_scope() { 
        scope_table_.pop_back(); 
    }
    
    void ScopeManager::add_binding(const std::string& name, const Type& type) {
        scope_table_.back()[name] = type;
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


    /*================================================++
    ||               SEMANTIC CHECKER                 ||
    ++================================================*/


    void SemanticChecker::check_main() const {
        
        const auto& main_info = class_manager_.get_class("Main");
        if (!main_info) {
            driver_.semantic_error(
                driver_.default_location(),  // no meaningful location
                "no 'Main' class defined"
            );
            return;
        }

        const auto& main_method = main_info->get_method("main");
        if (!main_method) {
            driver_.semantic_error(
                driver_.default_location(),
                "class 'Main' has no 'main' method"
            );
            return;
        }

        if (main_method->has_formals()) {
            driver_.semantic_error(
                main_method->location(),
                "method 'main' must take no arguments"
            );
            return;
        }

        const Type& return_type = main_method->return_type();
        
        if (!return_type.is_int32()) {
            driver_.semantic_error(
                main_method->location(),
                "method 'main' must return 'int32', found '" 
                + return_type.to_string() + "'"
            );
            return;
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

        auto info = class_manager_.get_class(name);
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
                << "Class: "    << class_name 
                << " extends "  << class_info.parent() << "\n";

            // Fields.
            cout << "  Fields:\n";
            if (class_info.fields().empty()) {
                cout << "    (none)\n";
            } else {
                for (const auto& field : class_info.fields()) {
                    cout << "    " 
                        << field.name() 
                        << " : " 
                        << field.type().to_string() << "\n";
                }
            }

            // Methods.
            cout << "  Methods:\n";
            if (class_info.methods().empty()) {
                cout << "    (none)\n";
            } else {
                for (const auto& method_info : class_info.methods()) {
                    // Method name.
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

    void SemanticChecker::push_scope() { 
        scope_manager_.push_scope(); 
    }

    void SemanticChecker::pop_scope() { 
        scope_manager_.pop_scope(); 
    }
    
    void SemanticChecker::add_binding(
        const std::string& name, 
        const Type& t
    ) { 
        scope_manager_.add_binding(name, t); 
    }
    
    bool SemanticChecker::add_class(ClassInfo c) { 
        return class_manager_.add_class(c); 
    }        

    bool SemanticChecker::class_exists(const std::string& name) const { 
        return class_manager_.class_exists(name); 
    }

    ClassInfo SemanticChecker::get_class(const std::string& name) const { 
        auto info = class_manager_.get_class(name); 
        if (!info) {
            driver_.internal_error("get_class(): class not found in the table");
            return ClassInfo::Dummy();
        }
        return info.value();
    }

    bool SemanticChecker::is_subtype(
        const Type& given, 
        const Type& compared_to
    ) const {
        auto result = class_manager_.is_subtype(given, compared_to);
        if (!result) {
            driver_.internal_error("is_subtype(): class not found in table");
            return false;
        }
        return result.value();
    }

    Type SemanticChecker::ancestor(const Type& t1, const Type& t2) const {
        
        // This method only applies to custom types.
        if (!t1.is_custom() || !t2.is_custom()) {
            driver_.internal_error("ancestor(): called with non custom types");
            return Type::Object();
        }

        if (   !class_exists(t1.custom_name()) 
            || !class_exists(t2.custom_name())
        ) {
            driver_.internal_error("ancestor(): unknown custom types.");
            return Type::Object();
        }
        
        // Types agree so their common ancestor is themselves.
        if (t1 == t2)
            return t1;

        return class_manager_.ancestor(t1, t2);
    }

    optional<Type> SemanticChecker::resolve(
        const std::string& name, 
        const std::string& current_class
    ) const {
        // Looking in the local scope.
        auto local = scope_manager_.lookup(name);
        if (local)
            return local;

        // Looking for fields in the class hierarchy.
        auto field = class_manager_.lookup_field(name, current_class);
        if (!field)
            return nullopt;
            
        return field->type();
    }

    optional<FieldInfo> SemanticChecker::lookup_field(
        const string& name,
        const string& class_name
    ) const {
        return class_manager_.lookup_field(name, class_name);
    }

    optional<MethodInfo> SemanticChecker::lookup_method(
        const string& name,
        const string& class_name
    ) const {
        return class_manager_.lookup_method(name, class_name);
    }
}
