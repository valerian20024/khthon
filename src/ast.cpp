#include "ast.hpp"

/*
? Is it safe to use a for each loop instead of size_t etc. in printNode methods 
?   of FieldNode and MethodNode

*/

using namespace std;

namespace Khthon {

    bool Type::operator==(const Type& other) const {
        if (kind_ != other.kind_)
            return false;
        
        if (kind_ == Kind::CUSTOM)
            return custom_name_ == other.custom_name_;

        return true;
    }

    bool Type::operator!=(const Type& other) const {
        return !(*this == other);
    }

    string Type::to_string() const {
        switch (kind_) {
            case Kind::CUSTOM:        return custom_name_; 
            case Kind::INT32:         return "int32";
            case Kind::BOOL:          return "bool";
            case Kind::STRING:        return "string";
            case Kind::UNIT:          return "unit";
            case Kind::DEFAULT:       return "DEFAULT_TYPE";
            
            default: {
                cerr << "Unknown type kind" << endl;
                return "";
            }
        }
    }

    string UnaryOperation::to_string() const {
        switch (kind) {
            case UnaryOperation::Kind::NOT:         return "not";
            case UnaryOperation::Kind::UMINUS:      return "-";
            case UnaryOperation::Kind::ISNULL:      return "isnull";
            case UnaryOperation::Kind::DEFAULT:     return "DEFAULT_UNOP";
            
            default: {
                cerr << "to_string(): Unknown unary operation." << endl;
                return "";
            }
        }
    }


    std::vector<BinaryOperation::TypePair> 
    BinaryOperation::valid_operand_types() const {
        switch (kind) {

            case Kind::PLUS:
            case Kind::MINUS:
            case Kind::TIMES:
            case Kind::DIVIDE:
            case Kind::POWER:
                return { {Type::Int32(), Type::Int32()} };

            case Kind::LOWER:
            case Kind::LOWER_EQUAL:
                return { {Type::Int32(), Type::Int32()} };

            case Kind::AND:
                return { {Type::Bool(), Type::Bool()} };

            // Defined on all primitive pairs, and any custom type pair.
            // The custom case is represented with a sentinel Type::Object().
            case Kind::EQUAL:
                return {
                    {Type::Int32(),  Type::Int32()},
                    {Type::Bool(),   Type::Bool()},
                    {Type::String(), Type::String()},
                    {Type::Object(), Type::Object()},
                };

            default: {
                cerr << "valid_operand_types(): unknown BinOp kind." << endl;
                return {};
            }
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
            
            default: {
                cerr << "to_string(): Unknown binary operation." << endl;
                return "";
            }
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