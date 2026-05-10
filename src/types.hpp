#ifndef TYPES_HPP
#define TYPES_HPP

#include <string>

namespace khthon {
    
    /// @brief Represents a type in VSOP.
    class Type {
    private: 
        enum class Kind { 
            CUSTOM, 
            INT32, 
            BOOL, 
            STRING, 
            UNIT,
            DEFAULT
        };    

        Kind kind_ = Kind::DEFAULT;
        std::string custom_name_ = {};
        
    public:
        /// @brief Default constructor for Type.
        /// @note Required by Bison.
        Type() = default;

        /// @brief Constructs a new Type based on a Type::Kind.
        explicit Type(Kind k) : kind_(k), custom_name_("") { }
        
        /// @brief Constructs a new custom type based on the class name.
        explicit Type(std::string name) : 
            kind_(Kind::CUSTOM), custom_name_(std::move(name)) { }

        /// @brief Factory method constructing a new int32 type. 
        static Type Int32()   { return Type(Kind::INT32); }

        /// @brief Factory method constructing a new bool type.
        static Type Bool()    { return Type(Kind::BOOL); }

        /// @brief Factory method constructing a new string type.
        static Type String()  { return Type(Kind::STRING); }

        /// @brief Factory method constructing a new unit type.
        static Type Unit()    { return Type(Kind::UNIT); }

        /// @brief Factory method constructing a new default type.
        static Type Default() { return Type(Kind::DEFAULT); }

        /// @brief Factory method constructing Object.
        static Type Object()  { return Type("Object"); }

        /// @brief Any class in VSOP is a type. 
        bool is_custom()    const { return kind_ == Kind::CUSTOM; }

        /// @brief Primitive types are `int32`, `string`, `bool` and `unit` in VSOP.
        bool is_primitive() const { return kind_ != Kind::DEFAULT && !is_custom(); }
        bool is_int32()     const { return kind_ == Kind::INT32; }
        bool is_bool()      const { return kind_ == Kind::BOOL; }
        bool is_string()    const { return kind_ == Kind::STRING; }
        bool is_unit()      const { return kind_ == Kind::UNIT; }

        /// @brief Checks whether this `Type` is defined in VSOP.
        bool is_undefined() const { return kind_ == Kind::DEFAULT; }

        bool operator==(const Type& other) const;
        bool operator!=(const Type& other) const;

        /// @brief Returns the custom type name. An empty string if none.
        std::string custom_name() const { return custom_name_; }

        /// @brief The `Type` string representation, as seen in any valid VSOP code.
        std::string to_string() const;
    };
} // namespace khthon


#endif
