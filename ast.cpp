#include "ast.hpp"

/*
Notes about the whole file

? Is it safe to use a for each loop instead of size_t etc. in printNode methods 
?   of FieldNode and MethodNode

? should make a map for eg UnOp and BinOp : Kind -> str

*/

using std::string;

namespace Khthon {

    /*================================================++
    ||                  STRUCTURES                    ||
    ++================================================*/

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
        return "!> Unknown unary op";
    }

    string BinaryOperation::to_string() const {
        switch (kind) {
            case BinaryOperation::Kind::EQUAL:          return "=";
            case BinaryOperation::Kind::LOWER:          return "<";
            case BinaryOperation::Kind::LOWER_EQUAL:    return "<=";
            case BinaryOperation::Kind::PLUS:           return "+";
            case BinaryOperation::Kind::MINUS:          return "-";
            case BinaryOperation::Kind::TIMES:          return "*";
            case BinaryOperation::Kind::DIVIDE:         return "/";
            case BinaryOperation::Kind::POWER:          return "^";
            case BinaryOperation::Kind::AND:            return "and";
            default: 
                return "DEFAULT";
        }
        return "!> Unknown binary op";
    }

    /*================================================++
    ||                    NODES                       ||
    ++================================================*/

    std::shared_ptr<MethodNode> MethodNode::makeDummy(
        Khthon::location loc, 
        string name, 
        std::shared_ptr<Expr> body
    ) {

        std::shared_ptr<FormalNode> formal = std::make_shared<FormalNode>(loc, "test1", Khthon::Type());
        std::vector<std::shared_ptr<FormalNode>> formals;

        formals.push_back(formal);

        return std::make_shared<MethodNode>(
            loc,
            std::move(name),
            Khthon::Type(Khthon::Type::Kind::UNIT),
            formals,  // no formals
            std::move(body)
        );
    }


    /*================================================++
    ||                  VISITORS                      ||
    ++================================================*/

    string PrintVisitor::visit(const ProgramNode& node) const {
        return printNodeList(node.classes());
    }

    string PrintVisitor::visit(const ClassNode& node) const {
        return "Class(" 
            + node.name() 
            + ", " 
            + node.parent() 
            + ", \n\t" 
            + printNodeList(node.fields()) 
            + ", \n\t" 
            + printNodeList(node.methods()) 
            + ")";
    }

    string PrintVisitor::visit(const FieldNode& node) const {
        string res = "Field(" 
            + node.name() 
            + ", " 
            + node.type().to_string();
        
        if (node.has_init())
            res += ", " + node.initializer().value()->accept(*this);
        
        res += ")";

        return res;
    }

    string PrintVisitor::visit(const MethodNode& node) const {
        return "Method("
            + node.name()
            + ", "
            + printNodeList(node.formals())
            + ", "
            + node.type().to_string()
            + ", \n\t"
            + node.body()->accept(*this)
            + ")";
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

    string PrintVisitor::visit(const IntegerLiteralExpr& node) const {
        return std::to_string(node.value());
    }

    string PrintVisitor::visit(const BoolLiteralExpr& node) const {
        return node.value() ? "true" : "false";
    }

    string PrintVisitor::visit(const UnitLiteralExpr&) const {
        return "()";
    }

    string PrintVisitor::visit(const IfExpr& node) const {
        string s = "If(" 
            + node.guardian()->accept(*this)
            + ", " 
            + node.consequent()->accept(*this);
        
        if (node.alternative().has_value())
            s += ", " + node.alternative().value()->accept(*this);

        s += ")";
        return s;
    }

    string PrintVisitor::visit(const AssignExpr& node) const {
        return "Assign("
            + node.name()
            + ", "
            + node.value()->accept(*this)
            + ")";
    }

    string PrintVisitor::visit(const NewExpr& node) const {
        return "New("
            + node.identifier()
            + ")";
    }

    /*todo refactor without op */
    string PrintVisitor::visit(const UnOpExpr& node) const {
        string op = node.operation().to_string();

        return "UnOp("
            + op
            + ", "
            + node.operand()->accept(*this)
            + ")";
    }

    string PrintVisitor::visit(const BinOpExpr& node) const {
        string op = node.operation().to_string();

        return "BinOp("
            + op
            + ", "
            + node.left()->accept(*this)
            + ", "
            + node.right()->accept(*this)
            + ")";
    }

    string PrintVisitor::visit(const VariableExpr& node) const {
        return node.identifier();
    }

    string PrintVisitor::visit(const CallExpr& node) const {
        return "Call("
            + node.receiver()->accept(*this)
            + ", "
            + node.name()
            + ", "
            + printNodeList(node.args())
            + ")";
    }

    string PrintVisitor::visit(const SelfExpr&) const {
        return "self";
    }

    string PrintVisitor::visit(const LetExpr& node) const {
        string s = "Let("
            + node.name()
            + ", "
            + node.type().to_string();

        if (node.has_initializer())
            s += ", " + node.initializer().value()->accept(*this);

        s += ", "
            + node.scope()->accept(*this)
            + ")";
        return s;
    }

    string PrintVisitor::visit(const WhileExpr& node) const {
        return "While("
            + node.condition()->accept(*this)
            + ", "
            + node.body()->accept(*this)
            + ")";
    }
}