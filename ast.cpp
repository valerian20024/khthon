#include "ast.hpp"

/*
Notes about the whole file

? Is it safe to use a for each loop instead of size_t etc. in printNode methods 
?   of FieldNode and MethodNode

todo change std::string to string

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

    string UnaryOperation::to_string() const {
        switch (kind) {
            case UnaryOperation::Kind::NOT:         return "not";
            case UnaryOperation::Kind::UMINUS:      return "-";
            case UnaryOperation::Kind::ISNULL:      return "isnull";
            default: 
                return "DEFAULT";
        }
        return "unknown unary op";
    }

    string PrintVisitor::visit(const ProgramNode& node) const {
        string result = "";
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
        std::string res = "Field(" 
            + node.name() 
            + ", " 
            + node.type().to_string() 
            + ")";
        
        if (node.has_init())
            res += ", " + node.initializer().value()->accept(*this);
        
        return res;
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
        return printNodeList(node.expressions());
    }

    string PrintVisitor::visit(const StringLiteralExpr& node) const {
        return node.value();
    }

    std::string PrintVisitor::visit(const IntegerLiteralExpr& node) const {
        return std::to_string(node.value());
    }

    std::string PrintVisitor::visit(const BoolLiteralExpr& node) const {
        return node.value() ? "true" : "false";
    }

    std::string PrintVisitor::visit(const UnitLiteralExpr& node) const {
        (void) node;  /*placeholder to not trigger warning*/
        return "()";
    }

    std::string PrintVisitor::visit(const IfExpr& node) const {
        std::string s = "If(" 
            + node.guardian()->accept(*this)
            + ", " 
            + node.consequent()->accept(*this);
        
        if (node.alternative().has_value())
            s += ", " + node.alternative().value()->accept(*this);

        s += ")";
        return s;
    }

    std::string PrintVisitor::visit(const AssignExpr& node) const {
    return "Assign("
         + node.name()
         + ", "
         + node.value()->accept(*this)
         + ")";
    }

    std::string PrintVisitor::visit(const NewExpr& node) const {
        return "New("
            + node.identifier()
            + ")";
    }

    std::string PrintVisitor::visit(const UnOpExpr& node) const {
        string op = node.operation().to_string();

        return "UnOp("
            + op
            + ", "
            + node.operand()->accept(*this)
            + ")";
    }

}