#ifndef OPERATORS_HPP
#define OPERATORS_HPP

#include "types.hpp"

#include <vector>
#include <iostream>

namespace khthon {

    /// @brief Represents an unary operation in VSOP.
    class UnaryOperation {
    private:
        enum class Kind {
            NOT,
            UMINUS,
            ISNULL,
            DEFAULT
        };
    
        Kind kind_ = Kind::DEFAULT;

    public:
        UnaryOperation() = default;
        
        explicit UnaryOperation(Kind k) : kind_(k) { }

        static UnaryOperation Not()         { return UnaryOperation(Kind::NOT); }
        static UnaryOperation UnaryMinus()  { return UnaryOperation(Kind::UMINUS); }
        static UnaryOperation IsNull()      { return UnaryOperation(Kind::ISNULL); }
        static UnaryOperation Default()     { return UnaryOperation(Kind::DEFAULT); }

        bool is_not()           const { return kind_ == Kind::NOT; }
        bool is_unary_minus()   const { return kind_ == Kind::UMINUS; }
        bool is_isnull()        const { return kind_ == Kind::ISNULL; }

        /// @brief Returns true for unary minus.
        bool is_arithmetic() const  { return kind_ == Kind::UMINUS; }

        /// @brief Returns true for the not and isnull operators.
        bool is_logical() const     { return (kind_ == Kind::NOT) || (kind_ == Kind::ISNULL); }

        /// @brief Checks whether this operation is defined in VSOP.
        bool is_undefined()     const { return kind_ == Kind::DEFAULT; }

        /// @brief Returns the type this unary operator operand can be.
        std::vector<Type> valid_operand_types() const;

        /// @brief Returns the type this unary operator should result in.
        Type result_type() const;

        /// @brief The `UnaryOperation` string representation, as seen in any valid VSOP code.
        std::string to_string() const;
    };


    /// @brief Represents a binary operation in VSOP.
    class BinaryOperation {
    private:
        enum class Kind { 
            EQUAL, 
            LOWER, 
            LOWER_EQUAL, 
            PLUS, 
            MINUS, 
            TIMES, 
            DIVIDE, 
            POWER, 
            AND, 
            DEFAULT
        };
    
        Kind kind_ = Kind::DEFAULT;

    public:
        using TypePair = std::pair<Type, Type>;

        BinaryOperation() = default;

        explicit BinaryOperation(Kind k) : kind_(k) { }

        // Factory methods.

        static BinaryOperation Equal()        { return BinaryOperation(Kind::EQUAL); }
        static BinaryOperation Lower()        { return BinaryOperation(Kind::LOWER); }
        static BinaryOperation LowerEqual()   { return BinaryOperation(Kind::LOWER_EQUAL); }
        static BinaryOperation Plus()         { return BinaryOperation(Kind::PLUS); }
        static BinaryOperation Minus()        { return BinaryOperation(Kind::MINUS); }
        static BinaryOperation Times()        { return BinaryOperation(Kind::TIMES); }
        static BinaryOperation Divide()       { return BinaryOperation(Kind::DIVIDE); }
        static BinaryOperation Power()        { return BinaryOperation(Kind::POWER); }
        static BinaryOperation And()          { return BinaryOperation(Kind::AND); }
        static BinaryOperation Default()      { return BinaryOperation(Kind::DEFAULT); }

        /// @brief Returns true for +, -, *, /, ^
        bool is_arithmetic() const;

        /// @brief Returns true only for 'and'
        bool is_logical() const;

        /// @brief Returns true for < and <=
        bool is_comparison() const;

        /// @brief Returns true only for =
        bool is_equality() const;

        /// @brief Checks whether this operation is defined in VSOP.
        bool is_undefined()     const { return kind_ == Kind::DEFAULT; }

        /// @brief Returns the types this binary operator operands can be.
        /// @note The vector of pairs allows for future asymmetric and overloaded operators.
        std::vector<TypePair> valid_operand_types() const;

        /// @brief Returns the type this binary operator should result in.
        Type result_type() const;

        /// @brief The `BinaryOperation` string representation, as seen in any valid VSOP code.
        std::string to_string() const;
    };
} // namespace khthon


#endif
