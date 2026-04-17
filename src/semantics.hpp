#ifndef SEMANTICS_HPP
#define SEMANTICS_HPP

#include "ast.hpp"
#include "driver.hpp"

#include <map>
#include <string>

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

        enum class VisitState { Unvisited, Visiting, Visited };

        /// @brief Helper function implementing depth-first search for finding cycles.
        bool cycle_check(const std::string& name, 
            std::map<std::string, VisitState>& states) const;

        // Pass 2
        void check_main() const;
        void check_parent_classes_exist() const;
        void check_inheritance_cycles();

        
        /// @brief Debugging purpose.
        void print_class_table() const;

    public:
        explicit SemanticChecker(Driver& driver) : driver_(driver) {}

        /// @brief Orchestrator for semantic analysis checks.
        void analyze(const std::shared_ptr<ProgramNode>& root);
    };


    /*================================================++
    ||                   VISITORS                     ||
    ++================================================*/

    /**
     * @brief Visitor gathering class informations in the symboltable.
     * 
     * @note Also performs basic duplicate checks, which lead to an error.
     */
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

        /// @warning Mutates SemanticChecker state.
        void visit(const ClassNode& node) const override;

    };

    /***
     * @brief Visitor performing type checking and annotating the AST.
     */
    class TypesVisitor : public MutableVisitor<void> {
    private:
        Driver& driver_;

        const std::map<std::string, ClassInfo>& class_table_;

        std::string current_class_name_;  // For handling 'self' keyword.
        
        std::vector<std::map<std::string, Type>> scope_stack_;

        bool conforms(const Type& actual, const Type& expected) const;

        Type ancestor(const Type& t1, const Type& t2) const;

        


    public:
        explicit TypesVisitor(
            Driver& d, 
            const std::map<std::string, ClassInfo>& class_table
        ) : 
            driver_(d), 
            class_table_(class_table) 
        {}

        void visit(ProgramNode& node) override;
        void visit(ClassNode& node) override;
        void visit(MethodNode& node) override;
        void visit(FormalNode& node) override;
        void visit(FieldNode& node) override;
        void visit(BlockExpr& node) override;
        void visit(StringLiteralExpr& node) override;
        void visit(IntegerLiteralExpr& node) override;
        void visit(BoolLiteralExpr& node) override;
        void visit(UnitLiteralExpr& node) override;
        void visit(IfExpr& node) override;
        void visit(AssignExpr& node) override;
        void visit(NewExpr& node) override;
        void visit(UnOpExpr& node) override;
        void visit(BinOpExpr& node) override;
        void visit(VariableExpr& node) override;
        void visit(CallExpr& node) override;
        void visit(SelfExpr& node) override;
        void visit(LetExpr& node) override;
        void visit(WhileExpr& node) override;
    };
}

#endif
