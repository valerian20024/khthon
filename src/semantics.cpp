#include "semantics.hpp"

#include <string>
#include <vector>

using namespace std;

void ClassesVisitor::visit(const ProgramNode& node) const {
    for (const auto& c : node.classes())
        c->accept(*this);
}

void ClassesVisitor::visit(const ClassNode& node) const {
    const string& class_name = node.name();

    // Not updating the symbol table if duplicates.
    if (class_table_.count(class_name)) {
        driver_.semantic_error(
            node.location(), 
            "class '" + class_name + "' is defined more than once"
        );
        return;
    }

    ClassInfo info(class_name, node.parent(), node.location());

    for (const auto& field : node.fields()) {
        FieldInfo field_info(
            field->name(), 
            field->type(), 
            field->location()
        );

        if (!info.add_field(std::move(field_info))) {
            driver_.semantic_error(
                field->location(), 
                "field '" + field->name() + "' is defined more than once"
            );
        }
    }

    // build FormalInfo list from m->formals()
    for (const auto& method : node.methods()) {
        
        vector<FormalInfo> formals_infos;
        for (const auto& formal : method->formals())
            formals_infos.emplace_back(
                formal->name(), 
                formal->type(), 
                formal->location()
            );
        
        MethodInfo method_info(
            method->name(), 
            method->type(), 
            std::move(formals_infos), 
            method->location()
        );

        if (!info.add_method(std::move(method_info))) {
            driver_.semantic_error(
                method->location(),
                "method '" + method->name() + "' is defined more than once"
            );
        }
    }

    class_table_.emplace(class_name, std::move(info));
}

bool SemanticChecker::analyze(const shared_ptr<ProgramNode>& root) {
    if (!root)
        return false;
    
    ClassesVisitor cv = ClassesVisitor(driver_, class_table_);
    root->accept(cv);

    print_class_table();

    return true;
}

void SemanticChecker::print_class_table() const {
    for (const auto& [class_name, class_info] : class_table_) {
        
        cout << "┌─ Class: " << class_name 
             << " extends " << class_info.parent() << "\n";

        // Fields
        cout << "│  Fields:\n";
        if (class_info.fields().empty()) {
            cout << "│    (none)\n";
        } else {
            for (const auto& [field_name, field_info] : class_info.fields()) {
                cout << "│    " 
                     << field_info.name() 
                     << " : " 
                     << field_info.type().to_string() 
                     << "\n";
            }
        }

        // Methods
        cout << "│  Methods:\n";
        if (class_info.methods().empty()) {
            cout << "│    (none)\n";
        } else {
            for (const auto& [method_name, method_info] : class_info.methods()) {
                cout << "│    " 
                     << method_info.name()
                     << "(";
                
                // Formals
                const auto& formals = method_info.formals();
                for (size_t i = 0; i < formals.size(); ++i) {
                    if (i > 0) cout << ", ";
                    cout << formals[i].name() 
                         << " : " 
                         << formals[i].type().to_string();
                }

                cout << ") : " 
                     << method_info.return_type().to_string() 
                     << "\n";
            }
        }

        cout << "└─────────────────────────\n";
    }
}
