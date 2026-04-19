#ifndef SEMANTICS_HPP
#define SEMANTICS_HPP

#include "ast.hpp"
#include "driver.hpp"

#include <map>
#include <string>

namespace Khthon {

    inline constexpr bool enable_advanced_logging =
#ifdef DEBUG
    true;
#else
    false;
#endif


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

        /// @return false if a field with that name already exists. true otherwise.
        bool add_field(FieldInfo f) {
            auto [it, inserted] = fields_.emplace(f.name(), std::move(f));
            return inserted;
        }

        /// @return false if a method with that name already exists. true otherwise.
        bool add_method(MethodInfo m) {
            auto [it, inserted] = methods_.emplace(m.name(), std::move(m));
            return inserted;
        }

        const std::string name() const                              { return name_; }
        const std::string& parent() const                           { return parent_; }
        const Khthon::location& location() const                    { return location_; }
        const std::map<std::string, FieldInfo>& fields() const      { return fields_; }
        const std::map<std::string, MethodInfo>& methods() const    { return methods_; }
    };
 

    /*================================================++
    ||                     CORE                       ||
    ++================================================*/

    /**
     * @brief Manages the scope symbol table.
     */
    class ScopeManager {
    public:
        using ScopeSymbolTable = std::vector<std::map<std::string, Type>>;

    private:
        ScopeSymbolTable scope_table_;

    public:
        void push_scope() { 
            scope_table_.push_back({}); 
        }
        
        void pop_scope() { 
            scope_table_.pop_back(); 
        }
        
        void add_binding(const std::string& name, const Type& type) {
            scope_table_.back()[name] = type;
        }

        /** @brief Looks for a identifier's type.
         *
         * It will first look for variables in local scope, then upward, up
         * to global scope. This allows to implement shadowing.
         * 
         * @return The identifier's type, or nullopt if not found.
         */
        std::optional<Type> lookup(const std::string& name) const;
    };

    class ClassManager {
    public:
        using ClassSymbolTable = std::map<std::string, ClassInfo>;

    private:
        ClassSymbolTable class_table_;

    public:
        /// @brief Tries to insert new class information into the table.
        /// @return false if a class with that name already exists.
        bool add_class(ClassInfo c);
        
        /// @brief Whether a class with this name already exists in the table.
        bool class_exists(const std::string& name) const;
        
        /// @brief Tries to get the information of class with name `name`.
        /// @return `nullopt` if not found. 
        const std::optional<ClassInfo> get_class(const std::string& name) const;

        /// @return The whole classes symbol table.
        const ClassSymbolTable& table() const { return class_table_; }

        /// @brief Checks whether `given` is a subtype of `compared_to` using class inheritance.
        /// @return `nullopt` if an error occured.
        const std::optional<bool> is_subtype(const Type& given, const Type& compared_to) const;
        
    };

    /** 
     * @brief Main orchestrator of the semantic analysis.
     */
    class SemanticChecker {
    private:
        Driver& driver_;
        
        ClassManager class_manager_;
        ScopeManager scope_manager_;

        //todo bulky to have this here
        enum class VisitState { Unvisited, Visiting, Visited };

        /// @brief Helper function implementing depth-first search for finding cycles.
        bool cycle_check(const std::string& name, 
            std::map<std::string, VisitState>& states) const;
        
        void check_main() const;

        void check_parent_classes_exist() const;
        
        void check_inheritance_cycles();
        
        /// @brief Debugging purpose.
        void print_class_table() const;

    public:
        explicit SemanticChecker(Driver& driver) : driver_(driver) { }

        /// @brief Orchestrator for semantic analysis checks.
        void analyze(const std::shared_ptr<ProgramNode>& root);

        // Scope management is delegated to ScopeManager
        
        void push_scope() { scope_manager_.push_scope(); }

        void pop_scope() { scope_manager_.pop_scope(); }
        
        void add_binding(const std::string& name, const Type& t) { 
            scope_manager_.add_binding(name, t); 
        }

        // Class Management is delegated to ClassManager

        //? ClassInfo& c ?
        bool add_class(ClassInfo c) {
            return class_manager_.add_class(c);
        }        

        bool class_exists(const std::string& name) const { 
            return class_manager_.class_exists(name);
        }

        std::optional<ClassInfo> get_class(const std::string& name) const { 
            return class_manager_.get_class(name); 
        }

        bool is_subtype(const Type& given, const Type& compared_to) const {
            auto result = class_manager_.is_subtype(given, compared_to);
            if (!result) {
                driver_.internal_error("is_subtype(): class not found in table");
                return false;
            }
            return result.value();
        }

        std::optional<Type> resolve(
            const std::string& name, 
            const std::string& current_class
        ) const;
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
        SemanticChecker& checker_;

    public:
        explicit ClassesVisitor(
            Driver& d, 
            SemanticChecker& sc
        ) : 
            driver_(d), 
            checker_(sc) 
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
        // Handle to main driver.
        Driver& driver_;

        // Handle to the classes symbol table.
        SemanticChecker& checker_;

        std::string current_class_name_;  // For handling 'self' keyword.

        /// @brief Checks the `actual` type conforms to the one `expected`.
        bool conforms(const Type& actual, const Type& expected) const;

        /// @brief Finds the least common ancestor of `t1` and `t2`
        Type ancestor(const Type& t1, const Type& t2) const;

        /// @brief Checks whether the operand conforms to the ones expected by 
        /// the unary operator.
        bool check_unop_operand(
            const UnaryOperation& operation,
            const Type& t_operand
        ) const;

        /// @brief Checks whether the two operands conform to the ones expected by 
        /// the binary operator.
        bool check_binop_operands(
            const BinaryOperation& op,
            const Type& t_left,
            const Type& t_right
        ) const;

        /// @brief Prints a tracing message when debugging is enabled.
        void trace(const std::string& message) const { 
            if (Khthon::enable_advanced_logging)
                std::cout << message << std::endl;
        }

    public:
        explicit TypesVisitor(
            Driver& d, 
            SemanticChecker& sc
        ) : 
            driver_(d), 
            checker_(sc) 
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
