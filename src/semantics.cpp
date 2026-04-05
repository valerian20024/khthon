#include "semantics.hpp"

void ClassesVisitor::visit(const ProgramNode& node) const {
    for (const auto& c : node.classes())
        c->accept(*this);
}

void ClassesVisitor::visit(const ClassNode& node) const {
    const std::string& name = node.name();

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
