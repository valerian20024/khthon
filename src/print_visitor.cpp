#include "ast.hpp"
#include "colors.hpp"
#include "visitors.hpp"

using namespace std;
using namespace colors;

namespace Khthon {

    template<typename T> string 
    PrintVisitor::stringify(const NodeList<T>& items) const {
            string result = "[";
            for (size_t i = 0; i < items.size(); ++i) {
                if (i > 0) 
                    result += ",\n ";
                result += items[i]->accept(*this);
            }
            return result + "]";
    }

    string PrintVisitor::annotate(string s, const Expr& node) const {
        if (annotate_)
            s += " : " + as_note(node.type().to_string());
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
            + ", \n\t"
            + stringify(node.formals())
            + ", "
            + node.type().to_string()
            + ", "
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