#include "ast.hpp"

/*
? Is it safe to use a for each loop instead of size_t etc. in printNode methods 
?   of FieldNode and MethodNode

*/

using namespace std;

namespace Khthon {

    string Type::to_string() const {
        switch (kind_) {
            case Type::Kind::CUSTOM:        return custom_name_; 
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
}