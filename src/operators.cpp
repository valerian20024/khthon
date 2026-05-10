#include "operators.hpp"

using namespace std;

namespace Khthon
{
    vector<Type> UnaryOperation::valid_operand_types() const {
        switch (kind_) {
            case Kind::NOT:
                return { Type::Bool() };

            case Kind::UMINUS:
                return { Type::Int32() };
            
            case Kind::ISNULL:
                return { Type::Object() };  // sentinel value

            default: {
                cerr << "valid_operand_type(): Unknown kind." << endl;
                return { Type::Default() };
            }
        }
    }

    Type UnaryOperation::result_type() const {
        switch (kind_) {
            case Kind::NOT:
                return Type::Bool();

            case Kind::UMINUS:
                return Type::Int32();
            
            case Kind::ISNULL:
                return Type::Bool();

            default: {
                cerr << "result_type(): Unknown kind." << endl;
                return Type::Default();
            }
        }
    }


    string UnaryOperation::to_string() const {
        switch (kind_) {
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

    bool BinaryOperation::is_arithmetic() const {
        return (kind_ == Kind::PLUS) 
            || (kind_ == Kind::MINUS)
            || (kind_ == Kind::TIMES)
            || (kind_ == Kind::DIVIDE)
            || (kind_ == Kind::POWER);
    }
    
    bool BinaryOperation::is_comparison() const {
        return (kind_ == Kind::LOWER) || (kind_ == Kind::LOWER_EQUAL);
    }
    
    bool BinaryOperation::is_equality() const {
        return kind_ == Kind::EQUAL;
    }

    bool BinaryOperation::is_logical() const {
        return kind_ == Kind::AND;
    }

    std::vector<BinaryOperation::TypePair> 
    BinaryOperation::valid_operand_types() const {
        switch (kind_) {

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
                    {Type::Object(), Type::Object()},  // sentinel value
                };

            default: {
                cerr << "valid_operand_types(): unknown BinOp kind." << endl;
                return {};
            }
        }
    }

    Type BinaryOperation::result_type() const {
        switch (kind_) {
            case Kind::PLUS:
            case Kind::MINUS:
            case Kind::TIMES:
            case Kind::DIVIDE:
            case Kind::POWER:   
                return Type::Int32();

            case Kind::LOWER:
            case Kind::LOWER_EQUAL:
            case Kind::AND:
            case Kind::EQUAL:   
                return Type::Bool();

            default: {
                cerr << "result_type(): Unknown kind." << endl;
                return Type::Default();
            }
        }
    }

    string BinaryOperation::to_string() const {
        switch (kind_) {
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
} // namespace Khthon
