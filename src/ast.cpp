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
        switch (kind) {
            case Type::Kind::CUSTOM:        return custom_name; 
            case Type::Kind::INT32:         return "int32";
            case Type::Kind::BOOL:          return "bool";
            case Type::Kind::STRING:        return "string";
            case Type::Kind::UNIT:          return "unit";
            case Type::Kind::DEFAULT:       return "DEFAULT_TYPE";
            default:
                return "Unknown type kind";
        }
    }

    string UnaryOperation::to_string() const {
        switch (kind) {
            case UnaryOperation::Kind::NOT:         return "not";
            case UnaryOperation::Kind::UMINUS:      return "-";
            case UnaryOperation::Kind::ISNULL:      return "isnull";
            case UnaryOperation::Kind::DEFAULT:     return "DEFAULT_UNOP";
            default:    
                return "Unknown unary operation";
        }
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
            case BinaryOperation::Kind::DEFAULT:        return "DEFAULT_BINOP";
            default: 
                return "Unknown binary operation";
        }
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
    PrintVisitor::stringify(const NodeList<T>& items) const {
            string result = "[";
            for (size_t i = 0; i < items.size(); ++i) {
                if (i > 0) 
                    result += ", ";
                result += items[i]->accept(*this);
            }
            return result + "]";
    }

    string PrintVisitor::annotate(string s, const Expr& node) const {
        if (annotate_)
            s += " : " + node.type().to_string();
        return s;
    }

    string PrintVisitor::visit(const ProgramNode& node) const {
        return stringify(node.classes());
    }

    string PrintVisitor::visit(const ClassNode& node) const {
        return "Class(" 
            + node.name() 
            + ", " 
            + node.parent() 
            + ", \n\t" 
            + stringify(node.fields()) 
            + ", \n\t" 
            + stringify(node.methods()) 
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
            + stringify(node.formals())
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
        return annotate(stringify(node.expressions()), node);
    }

    string PrintVisitor::visit(const StringLiteralExpr& node) const {
        return annotate(node.value(), node);
    }

    string PrintVisitor::visit(const IntegerLiteralExpr& node) const {
        return annotate(to_string(node.value()), node);
    }

    string PrintVisitor::visit(const BoolLiteralExpr& node) const {
        string value = node.value() ? "true" : "false";
        return annotate(value, node);
    }

    string PrintVisitor::visit(const UnitLiteralExpr& node) const {
        return annotate("()", node);
    }

    string PrintVisitor::visit(const IfExpr& node) const {
        string res = "If(" 
            + node.guardian()->accept(*this)
            + ", " 
            + node.consequent()->accept(*this);
        
        if (node.alternative().has_value())
            res += ", " + node.alternative().value()->accept(*this);
        res += ")";

        return annotate(res, node);
    }

    string PrintVisitor::visit(const AssignExpr& node) const {
        string res = "Assign("
            + node.name()
            + ", "
            + node.value()->accept(*this)
            + ")";

        return annotate(res, node);
    }

    string PrintVisitor::visit(const NewExpr& node) const {
        string res = "New("
            + node.identifier()
            + ")";
        
        return annotate(res, node);
    }

    string PrintVisitor::visit(const UnOpExpr& node) const {
        string res = "UnOp("
            + node.operation().to_string()
            + ", "
            + node.operand()->accept(*this)
            + ")";
        
        return annotate(res, node);
    }

    string PrintVisitor::visit(const BinOpExpr& node) const {
        string res = "BinOp("
            + node.operation().to_string()
            + ", "
            + node.left()->accept(*this)
            + ", "
            + node.right()->accept(*this)
            + ")";
        
        return annotate(res, node);
    }

    string PrintVisitor::visit(const VariableExpr& node) const {
        return annotate(node.identifier(), node);
    }

    string PrintVisitor::visit(const CallExpr& node) const {
        string res = "Call("
            + node.receiver()->accept(*this)
            + ", "
            + node.name()
            + ", "
            + stringify(node.args())
            + ")";
        
        return annotate(res, node);
    }

    string PrintVisitor::visit(const SelfExpr& node) const {
        return annotate("self", node);
    }

    string PrintVisitor::visit(const LetExpr& node) const {
        string res = "Let("
            + node.name()
            + ", "
            + node.type().to_string();

        if (node.has_initializer())
            res += ", " + node.initializer().value()->accept(*this);

        res += ", "
            + node.scope()->accept(*this)
            + ")";

        return annotate(res, node);
    }

    string PrintVisitor::visit(const WhileExpr& node) const {
        string res = "While("
            + node.condition()->accept(*this)
            + ", "
            + node.body()->accept(*this)
            + ")";
        
        return annotate(res, node);
    }
}