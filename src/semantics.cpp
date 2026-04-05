#include "semantics.hpp"

#include <string>

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

    ClassInfo& info = class_table_[name];
    info.parent = node.parent();
    info.location = node.location();

    //todo collect fields and methods
}


bool SemanticChecker::analyze(const shared_ptr<ProgramNode>& root) {
    if (!root)
        return false;
    
    ClassesVisitor cv = ClassesVisitor(driver_, class_table_);
    root->accept(cv);

    return true;
}
