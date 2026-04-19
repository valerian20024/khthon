#include "semantics.hpp"

using namespace std;
using namespace Khthon;

namespace Khthon {

    void TypesVisitor::trace(const std::string& message) const { 
        if (Khthon::enable_advanced_logging)
            std::cout << message << std::endl;
    }

    bool TypesVisitor::conforms(const Type& given, const Type& expected) const {
        
        // Undefined types should never reach type checking.
        if (given.is_undefined() || expected.is_undefined()) {
            driver_.internal_error("conforms(): encountered an undefined type");
            return false;
        }

        // Primitive types conform only with themselves.
        if (given.is_primitive() || expected.is_primitive())
            return given == expected;


        // Types are custom so we check for subtyping.
        //return is_subtype(given, expected);

        return checker_.is_subtype(given, expected);
    }

    Type TypesVisitor::ancestor(const Type& t1, const Type& t2) const {

        // This method only applies to custom types.
        if (!t1.is_custom() || !t2.is_custom()) {
            driver_.internal_error("ancestor(): called with non custom types");
            return Type::Object();
        }

        // Types agree so their common ancestor is themselves.
        if (t1 == t2)
            return t1;
        
        // Collect the full ancestry chain of t1 into an ordered list.
        vector<string> ancestors;
        
        string current = t1.custom_name();
        while (true) {

            ancestors.push_back(current);
            if (current == "Object")
                break;

            auto info = checker_.get_class(current);
            if (!info) {
                driver_.internal_error(
                    "ancestor(): unable to find '" + current + "'"
                );
                return Type::Object();
            } 

            current = info->parent();
        }

        // Walk up t2's ancestors and return the first class found in t1's ancestors
        unordered_set<string>ancestors_set(
            ancestors.begin(), ancestors.end());

        current = t2.custom_name();
        while (true) {

            if (ancestors_set.count(current))
                return Type(current);

            if (current == "Object")
                return Type("Object");  // Fallback

            auto info = checker_.get_class(current);
            if (!info) {
                driver_.internal_error(
                    "ancestor(): unable to find '" + current + "'"
                );
                return Type::Object();
            }

            current = info->parent();
        }
    }

    bool TypesVisitor::check_unop_operand(
        const UnaryOperation& operation,
        const Type& t_operand
    ) const {
        
        // If the given operand is one of those expected by the operation.
        for (const auto& expected : operation.valid_operand_types()) {
            if (conforms(t_operand, expected))
                return true;
        }
        return false;
    }

    bool TypesVisitor::check_binop_operands(
        const BinaryOperation& op,
        const Type& t_left,
        const Type& t_right
    ) const {

        // Handling the special case of equality.
        if (op.is_equality())
            return (t_left == t_right) || (t_left.is_custom() && t_right.is_custom());

        // If the given operands are among the pairs expected by the operation.
        for (const auto& [expected_left, expected_right] : op.valid_operand_types()) {
            if (conforms(t_left, expected_left) && conforms(t_right, expected_right))
                return true;
        }
        return false;
    }

    void TypesVisitor::visit(ProgramNode& node) {
        trace("TypesVisitor visits ProgramNode");

        for (const auto& c : node.classes())
            c->accept(*this);
    }

    void TypesVisitor::visit(ClassNode& node) {
        trace("TypesVisitor visits ClassNode");
        
        current_class_name_ = node.name();
        // Have fresh scope for new class
        
        for (const auto& f : node.fields())
            f->accept(*this);

        for (const auto& m : node.methods())
            m->accept(*this);
    }

    void TypesVisitor::visit(MethodNode& node) {
        trace("TypesVisitor visits MethodNode");

        // New scope

        //? visiting formals?

        node.body()->accept(*this);
        
        // Pop scope 
    }

    void TypesVisitor::visit(FormalNode& node) {
        trace("TypesVisitor visits FormalNode");

        (void) node;
        return;
    }

    void TypesVisitor::visit(FieldNode& node) {
        trace("TypesVisitor visits FieldNode");

        if (node.has_init())
            node.initializer().value()->accept(*this);
    }

    void TypesVisitor::visit(BlockExpr& node) {
        trace("TypesVisitor visits BlockExpr");

        // Empty blocks yield unit.
        if (node.is_empty()) {
            node.set_type(Type::Unit());
            return;
        }

        for (const auto& e : node.expressions())
            e->accept(*this);
        
        // The type of a block is the type of its last expression.
        const auto& last_expression = node.last_expression();
        node.set_type(last_expression->type());
    }

    void TypesVisitor::visit(StringLiteralExpr& node) { 
        trace("TypesVisitor visits StringLiteralExpr");

        node.set_type(Type::String());
    }

    void TypesVisitor::visit(IntegerLiteralExpr& node) { 
        trace("TypesVisitor visits IntegerLiteralExpr");

        node.set_type(Type::Int32());
    }

    void TypesVisitor::visit(BoolLiteralExpr& node) { 
        trace("TypesVisitor visits BoolLiteralExpr");

        node.set_type(Type::Bool());
    }

    void TypesVisitor::visit(UnitLiteralExpr& node) { 
        trace("TypesVisitor visits UnitLiteralExpr");

        node.set_type(Type::Unit());
    }

    void TypesVisitor::visit(IfExpr& node) { 
        trace("TypesVisitor visits IfExpr");

        node.guardian()->accept(*this);
        node.consequent()->accept(*this);
        
        if (node.alternative().has_value())
            node.alternative().value()->accept(*this);
        
        // Guardian must have type bool.
        if (!conforms(node.guardian()->type(), Type::Bool())) {
            driver_.semantic_error(
                node.location(),
                "condition of 'if' must be of type 'bool', found '"
                + node.guardian()->type().to_string()
                + "'"
            );
        }

        const Type consequent_type = node.consequent()->type();
        const Type alternative_type = node.alternative().has_value()
            ? node.alternative().value()->type()
            : Type::Unit();
        
        // If at least one branch is unit, the node is unit.
        if (consequent_type.is_unit() || alternative_type.is_unit()) {
            node.set_type(Type::Unit());

        // If branches have class types, the node has the type of their ancestor.
        } else if (consequent_type.is_custom() && alternative_type.is_custom()) {
            node.set_type(ancestor(consequent_type, alternative_type));

        // If branches have non-unit, primitive types.
        } else if (consequent_type == alternative_type) {
            node.set_type(consequent_type);

        } else {
            driver_.semantic_error(
                node.location(),
                "branches of 'if' have incompatible types '"
                + consequent_type.to_string()
                + "' and '"
                + alternative_type.to_string()
                + "'"
            );

            node.set_type(Type::Object());  // error recovery
        }
    }

    void TypesVisitor::visit(AssignExpr& node) {
        trace("TypesVisitor visits AssignExpr");

        node.value()->accept(*this);
    }

    void TypesVisitor::visit(NewExpr& node) {
        trace("TypesVisitor visits NewExpr");

        (void) node;
        return;
    }

    void TypesVisitor::visit(UnOpExpr& node) { 
        trace("TypesVisitor visits UnOpExpr");

        node.operand()->accept(*this);

        const Type& t_operand = node.operand()->type();

        if(!check_unop_operand(node.operation(), t_operand)) {
            driver_.semantic_error(
                node.location(),
                "operator '" + node.operation().to_string()
                + "' cannot be applied to type '"
                + t_operand.to_string() + "'"
            );

            node.set_type(Type::Object());
            return;
        }

        node.set_type(node.operation().result_type());
    }

    void TypesVisitor::visit(BinOpExpr& node) { 
        trace("TypesVisitor visits BinOpExpr");

        node.left()->accept(*this);
        node.right()->accept(*this);  
        
        const Type& t_left  = node.left()->type();
        const Type& t_right = node.right()->type();

        if (!check_binop_operands(node.operation(), t_left, t_right)) {
            driver_.semantic_error(
                node.location(),
                "operator '" + node.operation().to_string()
                + "' cannot be applied to types '"
                + t_left.to_string() + "' and '"
                + t_right.to_string() + "'"
            );

            node.set_type(Type::Object());
            return;
        }

        node.set_type(node.operation().result_type());
    }

    void TypesVisitor::visit(VariableExpr& node) { 
        trace("TypesVisitor visits VariableExpr");

        (void) node;
        return;
    }

    void TypesVisitor::visit(CallExpr& node) { 
        trace("TypesVisitor visits CallExpr");

        node.receiver()->accept(*this);
        for (const auto& arg : node.args())
            arg->accept(*this);
        
        // Do something with dispatching
    }

    void TypesVisitor::visit(SelfExpr& node) { 
        trace("TypesVisitor visits SelfExpr");

        (void) node;
        return;
    }

    void TypesVisitor::visit(LetExpr& node) { 
        trace("TypesVisitor visits LetExpr");

        // New scope

        if (node.has_initializer())
            node.initializer().value()->accept(*this);

        node.scope()->accept(*this);
        
        // Pop scope
    }

    void TypesVisitor::visit(WhileExpr& node) { 
        trace("TypesVisitor visits WhileExpr");
        
        node.condition()->accept(*this);
        node.body()->accept(*this);

        if (!conforms(node.condition()->type(), Type::Bool())) {
            driver_.semantic_error(
                node.location(),
                "condition of 'while' must be of type 'bool', found '"
                + node.condition()->type().to_string()
                + "'"
            );
        }

        if (node.body()->type().is_undefined())
            driver_.internal_error("visiting WhileExpr found an undefined type");

        node.set_type(Type::Unit());
    }
    
} // namespace Khthon
