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

        throw std::runtime_error("Unknown type kind in Type::to_string()");
    }

    //todo: redundant function here, should factorize
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

    string PrintVisitor::printNodeList(const NodeList<Expr>& items) const {
        std::string result;
        for (size_t i = 0; i < items.size(); ++i) {
            if (i > 0) {
                result += ", ";
            }
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
                result += ",\n";
            result += classes[i]->accept(*this);
        }
        return result;
    }

    string PrintVisitor::visit(const ClassNode& node) const {
        return "Class(" 
            + node.name() 
            + ", " 
            + node.parent() 
            + ", \n\t[" 
            + printNodeList(node.fields()) 
            + "], \n\t[" 
            + printNodeList(node.methods()) 
            + "])";
    }

    string PrintVisitor::visit(const FieldNode& node) const {
        //todo need to handle fields with init expr
        //todo make a templated version
        return "Field(" 
            + node.name() 
            + ", " 
            + node.type().to_string() 
            + ")";
    }

    string PrintVisitor::visit(const MethodNode& node) const {
        return "Method("
            + node.name()
            + ", ["
            + printNodeList(node.formals())
            + "], "
            + node.type().to_string()
            + ", \n\t["
            + node.body()->accept(*this)
            + "])";
    }

    string PrintVisitor::visit(const FormalNode& node) const {
        return node.name() 
            + " : "
            + node.type().to_string();
    }

    string PrintVisitor::visit(const BlockExpr& node) const {
        return "BLOCKK([" + printNodeList(node.expressions()) + "])";    
    }

    string PrintVisitor::visit(const StringLiteralExpr& node) const {
        (void) node;
        return "STRING LITERAL VISITED";
    }
}