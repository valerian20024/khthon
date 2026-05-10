#include "types.hpp"

#include <string>
#include <iostream>

using namespace std;

namespace Khthon
{
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
} // namespace Khthon
