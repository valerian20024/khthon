#include "semantics.hpp"

#include <string>
#include <vector>

using namespace std;

// todo Add Object class first, as it is predefined.
void ClassesVisitor::visit(const ProgramNode& node) const {
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

bool SemanticChecker::analyze(const shared_ptr<ProgramNode>& root) {
    if (!root)
        return false;
    
    ClassesVisitor cv = ClassesVisitor(driver_, class_table_);
    root->accept(cv);

    check_main();

    print_class_table();

    return true;
}

void SemanticChecker::check_main() const {
    
    auto list = class_table_.find("List");

    auto main_class = class_table_.find("Main");
    if (main_class == class_table_.end()) {

        //Khthon::location loc;
        //loc.initialize(nullptr, 1, 1);
        driver_.semantic_error(
            Khthon::location(),  // no meaningful location
            "no 'Main' class defined"
        );
        return;
    }
    
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
        if (class_info.methods().empty()) {
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
