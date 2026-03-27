#ifndef SEMANTICS_HPP
#define SEMANTICS_HPP

#include "ast.hpp"
#include "driver.hpp"

namespace Khthon {

    struct MethodInfo {
        Khthon::Type return_type;

    };

    struct FieldInfo {
        Khthon::Type type;
    };

    struct ClassInfo {
        std::string parent;
        Khthon::location location;
        std::map<std::string, FieldInfo> fields;
        std::map<std::string, MethodInfo> methods;
    };
 

    class SemanticChecker {
    private:
        Driver& driver_;
        std::map<std::string, ClassInfo> class_table_;

        // Private methods to perform various checks

    public:
        explicit SemanticChecker(Driver& d) : driver_(d) {}
        bool analyze(const std::shared_ptr<ProgramNode>& root);
    };

    class ClassesVisitor : public Visitor<void> {
    private:
        Driver& driver_;
        std::map<std::string, ClassInfo>& class_table_;

    public:
        explicit ClassesVisitor(
            Driver& d, 
            std::map<std::string, ClassInfo>& table
        ) : 
            driver_(d), 
            class_table_(table) 
        {}

        void visit(const ProgramNode& node) const override;
        void visit(const ClassNode& node) const override;

    };

    class TypesVisitor : public Visitor<void> {
        // scope stack
        // visit methods for all the expr nodes
        //? access to driver
        // Pass-2 entry points

    };
}

#endif