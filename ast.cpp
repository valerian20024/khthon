#include "ast.hpp"

/*
Notes about the whole file

? Is it safe to use a for each loop instead of size_t etc. in printNode methods 
?   of FieldNode and MethodNode

*/
namespace Khthon {

    // For handling both fields and methods.
    std::string PrintVisitor::printNodeList(const NodeList<FieldNode>& items) const {
        std::string result;
        for (size_t i = 0; i < items.size(); ++i) {
            if (i > 0)
                result += ", ";
            result += items[i]->accept(*this);
        }
        return result;
    }

    std::string PrintVisitor::printNodeList(const NodeList<MethodNode>& items) const {
        std::string result;
        for (size_t i = 0; i < items.size(); ++i) {
            if (i > 0)
                result += ", ";
            result += items[i]->accept(*this);
        }
        return result;
    }

    std::string PrintVisitor::visit(const ProgramNode& node) const {
        std::string result;
        const auto& classes = node.classes();

        // Chaining visit to each of the classes
        for (size_t i = 0; i < classes.size(); ++i) {
            if (i > 0)
                result += ", ";
            result += classes[i]->accept(*this);
        }
        return result;
    }

    std::string PrintVisitor::visit(const ClassNode& node) const {
        return "Class(" + node.name() + ", " + node.parent() + ", [" +
            printNodeList(node.fields()) + "], [" + printNodeList(node.methods()) + "])";
    }

    std::string PrintVisitor::visit(const FieldNode& node) const {
        //! the following is only if no default init expr
        //todo need to add a case with init expr
        std::string type;
        switch (node.type().kind) {
            case Type::Kind::CUSTOM: type = node.type().custom_name; break;
            case Type::Kind::INT32:  type = "int32";    break;
            case Type::Kind::BOOL:   type = "bool";     break;
            case Type::Kind::STRING: type = "string";   break;
            case Type::Kind::UNIT:   type = "unit";     break;
        }
        return "Field(" + node.name() + ", " + type + ")";
    }

    std::string PrintVisitor::visit(const MethodNode& node) const {
        //! for now it's been copied from field but afterwards we need to 
        //! add formals and stuff so it'll be different
        std::string type;
        switch (node.type().kind) {
            case Type::Kind::CUSTOM: type = node.type().custom_name; break;
            case Type::Kind::INT32:  type = "int32";    break;
            case Type::Kind::BOOL:   type = "bool";     break;
            case Type::Kind::STRING: type = "string";   break;
            case Type::Kind::UNIT:   type = "unit";     break;
        }
        return "Method(" + node.name() + ", blablaformals, " + type + ", blabla block)";
    }
}