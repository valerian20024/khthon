#include "semantics.hpp"

#include <string>
#include <vector>

using namespace std;

void ClassesVisitor::visit(const ProgramNode& node) const {
    for (const auto& c : node.classes())
        c->accept(*this);
}

void ClassesVisitor::visit(const ClassNode& node) const {
    const string& name = node.name();

    // Not updating the symbol table if duplicates.
    if (class_table_.count(name)) {
        driver_.semantic_error(
            node.location(), 
            "class '" + name + "' is defined more than once"
        );
        return;
    }

    ClassInfo info(node.name(), node.parent(), node.location());

    for (const auto& f : node.fields()) {
        FieldInfo fi(f->name(), f->type(), f->location());

        if (!info.add_field(std::move(fi))) {
            driver_.semantic_error(
                f->location(), 
                "field '" + f->name() + "' is defined more than once"
            );
        }
    }

    // build FormalInfo list from m->formals()
    
    for (const auto& m : node.methods()) {
        
        vector<FormalInfo> formals;

        
        MethodInfo mi(m->name(), m->type(), std::move(formals), m->location());

        if (!info.add_method(std::move(mi))) {
            driver_.semantic_error(
                m->location(),
                "method '" + m->name() + "' is defined more than once"
            );
        }
    }

    class_table_.emplace(node.name(), std::move(info));
}

bool SemanticChecker::analyze(const shared_ptr<ProgramNode>& root) {
    if (!root)
        return false;
    
    ClassesVisitor cv = ClassesVisitor(driver_, class_table_);
    root->accept(cv);

    return true;
}
