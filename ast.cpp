#include "ast.hpp"

/*
? Is it safe to use a for each loop instead of size_t etc. in printNode methods 
?   of FieldNode and MethodNode

*/

using namespace std;

namespace Khthon {

    /*================================================++
    ||                  STRUCTURES                    ||
    ++================================================*/

    string Type::to_string() const {
        string result;
        switch (kind) {
            case Type::Kind::CUSTOM:            return custom_name; 
            case Type::Kind::INT32:             return "int32";
            case Type::Kind::BOOL:              return "bool";
            case Type::Kind::STRING:            return "string";
            case Type::Kind::UNIT:              return "unit";
            case Type::Kind::__DEFAULT__:       return "DEFAULT_TYPE";
        }
        return "Unknown type kind";
    }

    string UnaryOperation::to_string() const {
        switch (kind) {
            case UnaryOperation::Kind::NOT:         return "not";
            case UnaryOperation::Kind::UMINUS:      return "-";
            case UnaryOperation::Kind::ISNULL:      return "isnull";
            case UnaryOperation::Kind::__DEFAULT__: return "DEFAULT_UNOP";
        }
        return "Unknown unary operation";
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
            case BinaryOperation::Kind::__DEFAULT__:    return "DEFAULT_BINOP";
        }
        return "Unknown binary operation";
    }

    /*================================================++
    ||                    NODES                       ||
    ++================================================*/

    shared_ptr<MethodNode> 
    MethodNode::makeDummy(Khthon::location loc, string name) {

        shared_ptr<FormalNode> dummy_formal = make_shared<FormalNode>(
            loc, 
            "DEFAULT_FORMAL", 
            Khthon::Type()
        );

        vector<shared_ptr<FormalNode>> dummy_formals;
        dummy_formals.push_back(dummy_formal);

        shared_ptr<BlockExpr> dummy_body = make_shared<BlockExpr>(
            loc, 
            vector<shared_ptr<Expr>>{}
        );

        return make_shared<MethodNode>(
            loc,
            std::move(name),
            Khthon::Type(),
            dummy_formals,
            dummy_body
        );
    }

    /*================================================++
    ||                  VISITORS                      ||
    ++================================================*/

    template<typename T> string 
    PrintVisitor::print(const NodeList<T>& items) const {
            std::string result = "[";
            for (size_t i = 0; i < items.size(); ++i) {
                if (i > 0) 
                    result += ", ";
                result += items[i]->accept(*this);
            }
            return result + "]";
    }

    string PrintVisitor::visit(const ProgramNode& node) const {
        return print(node.classes());
    }

    string PrintVisitor::visit(const ClassNode& node) const {
        return "Class(" 
            + node.name() 
            + ", " 
            + node.parent() 
            + ", \n\t" 
            + print(node.fields()) 
            + ", \n\t" 
            + print(node.methods()) 
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
            + print(node.formals())
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
        return print(node.expressions());
    }

    string PrintVisitor::visit(const StringLiteralExpr& node) const {
        return node.value();
    }

    string PrintVisitor::visit(const IntegerLiteralExpr& node) const {
        return to_string(node.value());
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
            + print(node.args())
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