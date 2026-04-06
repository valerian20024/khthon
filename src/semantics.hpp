#ifndef SEMANTICS_HPP
#define SEMANTICS_HPP

#include "ast.hpp"
#include "driver.hpp"

namespace Khthon {


    /*================================================++
    ||                  STRUCTURES                    ||
    ++================================================*/


    class FieldInfo {
    private:
        std::string name_;
        Khthon::Type type_;
        Khthon::location location_;

    public:
        FieldInfo(
            std::string name, 
            Khthon::Type type, 
            Khthon::location loc
        ) : 
            name_(std::move(name)), 
            type_(std::move(type)), 
            location_(std::move(loc)) 
        {}

        // Eventual Type inference
        //void set_type(const Khthon::Type& t) { type_ = t; }

        const std::string& name() const { return name_; }
        const Khthon::Type& type() const { return type_; }
        const Khthon::location& location() const { return location_; }
    };

    class FormalInfo {
    private:
        std::string name_;
        Khthon::Type type_;
        Khthon::location location_;

    public:
        FormalInfo(
            std::string n, 
            Khthon::Type t,
            Khthon::location l
        ) : 
            name_(std::move(n)), 
            type_(std::move(t)),
            location_(std::move(l))
        {}
        
        const std::string& name() const { return name_; }
        const Khthon::Type& type() const { return type_; }
        const Khthon::location& location() const { return location_; }
    };

   class MethodInfo {
    private:
        std::string name_;
        Khthon::Type return_type_;
        std::vector<FormalInfo> formals_;
        Khthon::location location_;

    public:
        MethodInfo(
            std::string name,
            Khthon::Type return_type,
            std::vector<FormalInfo> formals,
            Khthon::location loc
        ) : 
            name_(std::move(name)),
            return_type_(std::move(return_type)),
            formals_(std::move(formals)),
            location_(std::move(loc))
        {}

        const std::string& name() const { return name_; }
        const Khthon::Type& return_type() const { return return_type_; }
        const std::vector<FormalInfo>& formals() const { return formals_; }
        const Khthon::location& location() const { return location_; }
    };

    class ClassInfo {
    private:
        std::string name_;
        std::string parent_;
        Khthon::location location_;
        std::map<std::string, FieldInfo> fields_;
        std::map<std::string, MethodInfo> methods_;

    public:
        ClassInfo(
            std::string name, 
            std::string parent, 
            Khthon::location loc
        ) : 
            name_(std::move(name)), 
            parent_(std::move(parent)), 
            location_(std::move(loc)) 
        {}

        // Returns false if a field with that name already exists
        bool add_field(FieldInfo f) {
            auto [it, inserted] = fields_.emplace(f.name(), std::move(f));
            return inserted;
        }

        bool add_method(MethodInfo m) {
            auto [it, inserted] = methods_.emplace(m.name(), std::move(m));
            return inserted;
        }

        const std::map<std::string, FieldInfo>& fields() const { return fields_; }
        const std::map<std::string, MethodInfo>& methods() const { return methods_; }
        const std::string& parent() const { return parent_; }
        const Khthon::location& location() const { return location_; }
    };
 

    /*================================================++
    ||                     CORE                       ||
    ++================================================*/


    // Orchestrator for diverse passes of semantic analysis.
    class SemanticChecker {
    private:
        Driver& driver_;
        std::map<std::string, ClassInfo> class_table_;

        // Pass 2
        void check_main() const;
        //void check_parent_classes_exist() const;

        // Pass 3
        //void check_inheritance_cycles() const;
        //std::vector<std::string> get_ancestors(const std::string& class_name) const;

        // Pass 4
        //void check_method_overriding() const;
        //void check_field_shadowing() const;

        /// @brief Debugging purpose.
        void print_class_table() const;

    public:
        explicit SemanticChecker(Driver& driver) : driver_(driver) {}

        /// @brief Orchestrator for semantic analysis checks.
        bool analyze(const std::shared_ptr<ProgramNode>& root);
    };


    /*================================================++
    ||                   VISITORS                     ||
    ++================================================*/


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

        // Modifying SemanticChecker state.
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
