#include "ast.hpp"

/*
Notes about the whole file

? Is it safe to use a for each loop instead of size_t etc. in printNode methods 
?   of FieldNode and MethodNode

? using std::string would be easier to read

todo add some spaces or lf in the AST dump for legibility

*/

using std::string;

namespace Khthon {

    string Type::to_string() const {
        string result;
        switch (kind) {
            case Type::Kind::CUSTOM: return custom_name; 
            case Type::Kind::INT32:  return "int32";
            case Type::Kind::BOOL:   return "bool";
            case Type::Kind::STRING: return "string";
            case Type::Kind::UNIT:   return "unit";
        }

        throw std::runtime_error("Unkwnow type kind in Type::to_string()");
    }

    // For handling both fields and methods.
    string PrintVisitor::printNodeList(const NodeList<FieldNode>& items) const {
        string result;
        for (size_t i = 0; i < items.size(); ++i) {
            if (i > 0)
                result += ", ";
            result += items[i]->accept(*this);
        }
        return result;
    }

    string PrintVisitor::printNodeList(const NodeList<MethodNode>& items) const {
        string result;
        for (size_t i = 0; i < items.size(); ++i) {
            if (i > 0)
                result += ", ";
            result += items[i]->accept(*this);
        }
        return result;
    }

    string PrintVisitor::printNodeList(const NodeList<FormalNode>& items) const {
        string result;
        for (size_t i = 0; i < items.size(); ++i) {
            if (i > 0)
                result += ", ";
            result += items[i]->accept(*this);
        }
        return result;
    }

    string PrintVisitor::visit(const ProgramNode& node) const {
        string result;
        const auto& classes = node.classes();

        // Chaining visit to each of the classes
        for (size_t i = 0; i < classes.size(); ++i) {
            if (i > 0)
                result += ", ";
            result += classes[i]->accept(*this);
        }
        return result;
    }

    string PrintVisitor::visit(const ClassNode& node) const {
        return "Class(" 
            + node.name() 
            + ", " 
            + node.parent() 
            + ", [" 
            + printNodeList(node.fields()) 
            + "], [" 
            + printNodeList(node.methods()) 
            + "])";
    }

    string PrintVisitor::visit(const FieldNode& node) const {
        //! the following is only if no default init expr
        //todo need to add a case with init expr
        string type;
        switch (node.type().kind) {
            case Type::Kind::CUSTOM: type = node.type().custom_name; break;
            case Type::Kind::INT32:  type = "int32";    break;
            case Type::Kind::BOOL:   type = "bool";     break;
            case Type::Kind::STRING: type = "string";   break;
            case Type::Kind::UNIT:   type = "unit";     break;
        }
        return "Field(" + node.name() + ", " + type + ")";
    }

    string PrintVisitor::visit(const MethodNode& node) const {
        string type;
        switch (node.type().kind) {
            case Type::Kind::CUSTOM: type = node.type().custom_name; break;
            case Type::Kind::INT32:  type = "int32";    break;
            case Type::Kind::BOOL:   type = "bool";     break;
            case Type::Kind::STRING: type = "string";   break;
            case Type::Kind::UNIT:   type = "unit";     break;
        }
        return "Method(" 
            + node.name() 
            + ", " 
            + printNodeList(node.formals()) 
            + type
            + "blabla block)";
    }

    string PrintVisitor::visit(const FormalNode& node) const {
        string type;
        switch (node.type().kind) {
            case Type::Kind::CUSTOM: type = node.type().custom_name; break;
            case Type::Kind::INT32:  type = "int32";    break;
            case Type::Kind::BOOL:   type = "bool";     break;
            case Type::Kind::STRING: type = "string";   break;
            case Type::Kind::UNIT:   type = "unit";     break;
        }
        return node.name() 
            + " : "
            + type;
    }
}